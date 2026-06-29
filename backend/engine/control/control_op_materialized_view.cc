

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace {

struct DuckDbSession {
  ::duckdb_database db = nullptr;
  ::duckdb_connection conn = nullptr;

  DuckDbSession() = default;
  DuckDbSession(const DuckDbSession&) = delete;
  DuckDbSession& operator=(const DuckDbSession&) = delete;
  DuckDbSession(DuckDbSession&& other) noexcept
      : db(std::exchange(other.db, nullptr)),
        conn(std::exchange(other.conn, nullptr)) {}
  DuckDbSession& operator=(DuckDbSession&& other) noexcept {
    if (this == &other) return *this;
    if (conn != nullptr) ::duckdb_disconnect(&conn);
    if (db != nullptr) ::duckdb_close(&db);
    db = std::exchange(other.db, nullptr);
    conn = std::exchange(other.conn, nullptr);
    return *this;
  }
  ~DuckDbSession() {
    if (conn != nullptr) ::duckdb_disconnect(&conn);
    if (db != nullptr) ::duckdb_close(&db);
  }
};

absl::StatusOr<DuckDbSession> OpenDuckDbSession() {
  DuckDbSession session;
  if (::duckdb_open(nullptr, &session.db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "control op executor: duckdb_open(in-memory) failed");
  }
  if (::duckdb_connect(session.db, &session.conn) != ::DuckDBSuccess) {
    return absl::InternalError("control op executor: duckdb_connect failed");
  }
  if (auto reg = duckdb::udf::RegisterAll(session.conn); !reg.ok()) {
    return reg;
  }
  return session;
}

absl::Status EnsureTargetSchemaExistsIfNeeded(::duckdb_connection conn,
                                              const storage::TableId& target,
                                              bool is_temp_mv) {
  if (is_temp_mv) return absl::OkStatus();
  return RunSqlNoResult(conn,
                        absl::StrCat("CREATE SCHEMA IF NOT EXISTS ",
                                     QuoteIdent(target.dataset_id)));
}

absl::Status AttachCollectorTables(::duckdb_connection conn,
                                   storage::Storage& storage,
                                   const TableScanCollector& collector) {
  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (tbl == nullptr) {
      return absl::FailedPreconditionError(
          "control op executor: null table reference in CREATE MATERIALIZED "
          "VIEW scan");
    }
    const auto* source_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (source_table == nullptr) {
      return absl::FailedPreconditionError(
          absl::StrCat("control op executor: cannot attach non-StorageTable '",
                       tbl->Name(),
                       "' for CREATE MATERIALIZED VIEW"));
    }
    absl::Status attach = AttachStorageTableAt(
        conn, &storage, *source_table, QuoteIdent(tbl->Name()));
    if (!attach.ok()) return attach;
  }
  return absl::OkStatus();
}

absl::StatusOr<std::string> BuildMaterializeSql(
    absl::string_view request_sql,
    const ::googlesql::ResolvedCreateMaterializedViewStmt* stmt,
    const storage::TableId& target,
    absl::Span<const QueryParameter> parameters) {
  if (stmt == nullptr || stmt->query() == nullptr) {
    return absl::InternalError(
        "control op executor: CREATE MATERIALIZED VIEW has null query scan");
  }
  duckdb::transpiler::Transpiler transpiler;
  std::string select_sql = transpiler.TranspileScan(stmt->query());
  if (select_sql.empty()) {
    return absl::UnimplementedError(
        "control op executor: CREATE MATERIALIZED VIEW query did not transpile "
        "to DuckDB SQL");
  }
  if (!transpiler.parameter_order().empty()) {
    auto substituted = SubstituteDuckdbParameters(
        std::move(select_sql), transpiler.parameter_order(), parameters);
    if (!substituted.ok()) return substituted.status();
    select_sql = *std::move(substituted);
  }
  const bool is_temp = absl::StrContains(request_sql, "CREATE TEMP") ||
                       absl::StrContains(request_sql, "create temp");
  const std::string dest_name =
      is_temp ? QuoteIdent(target.table_id)
              : absl::StrCat(QuoteIdent(target.dataset_id),
                             ".",
                             QuoteIdent(target.table_id));
  return absl::StrCat("CREATE ",
                      is_temp ? "OR REPLACE TEMP " : "",
                      "TABLE ",
                      dest_name,
                      " AS ",
                      select_sql);
}

}  // namespace

absl::Status RunCreateMaterializedView(
    storage::Storage& storage,
    absl::string_view project_id,
    const QueryRequest& request,
    const ::googlesql::ResolvedCreateMaterializedViewStmt* stmt,
    const ::googlesql::ResolvedStatement& root_stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "control op executor: CREATE MATERIALIZED VIEW has null resolved "
        "statement");
  }
  absl::StatusOr<storage::TableId> target = NamePathToTableId(
      stmt->name_path(), project_id, request.default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }

  absl::StatusOr<schema::TableSchema> bq_schema =
      ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  TableScanCollector collector;
  absl::Status visit_status = root_stmt.Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  absl::StatusOr<DuckDbSession> session = OpenDuckDbSession();
  if (!session.ok()) return session.status();
  ::duckdb_connection conn = session->conn;

  const bool is_temp_mv = absl::StrContains(request.sql, "CREATE TEMP") ||
                          absl::StrContains(request.sql, "create temp");
  absl::Status target_schema =
      EnsureTargetSchemaExistsIfNeeded(conn, *target, is_temp_mv);
  if (!target_schema.ok()) return target_schema;
  absl::Status attach = AttachCollectorTables(conn, storage, collector);
  if (!attach.ok()) return attach;

  if (stmt->create_mode() ==
      ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
  }

  absl::StatusOr<std::string> materialize_sql =
      BuildMaterializeSql(request.sql, stmt, *target, request.parameters);
  if (!materialize_sql.ok()) return materialize_sql.status();

  ::duckdb_result mv_result;
  if (::duckdb_query(conn, materialize_sql->c_str(), &mv_result) !=
      ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&mv_result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&mv_result);
    return absl::InvalidArgumentError(
        absl::StrCat("control op executor: DuckDB rejected materialized view: ",
                     detail,
                     " (sql=",
                     *materialize_sql,
                     ")"));
  }
  ::duckdb_destroy_result(&mv_result);

  const std::string quoted_target =
      is_temp_mv ? QuoteIdent(target->table_id)
                 : absl::StrCat(QuoteIdent(target->dataset_id),
                                ".",
                                QuoteIdent(target->table_id));
  absl::StatusOr<std::vector<storage::Row>> after_rows =
      DrainTableRows(conn, quoted_target, *bq_schema);
  if (!after_rows.ok()) return after_rows.status();

  if (is_temp_mv) {
    (void)storage.DropTable(*target);
  }

  absl::Status created = ApplyCreateMode(
      storage.CreateTable(*target, *bq_schema), stmt->create_mode());
  if (!created.ok()) return created;
  if (after_rows->empty()) return absl::OkStatus();
  return storage.AppendRows(*target, absl::MakeConstSpan(*after_rows));
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
