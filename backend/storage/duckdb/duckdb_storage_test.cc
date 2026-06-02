#include "backend/storage/duckdb/duckdb_storage.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace {

namespace fs = std::filesystem;

// Returns a fresh temp directory under TMPDIR (defaulting to /tmp).
// The directory is created empty and removed after the fixture
// teardown so each test starts from a known-clean state.
class DuckDBStorageTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-duckdb-storage-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  void TearDown() override {
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  fs::path data_dir_{};
};

// Two-column toy schema: an INT64 primary key and a STRING name.
// Mirrors the README round-trip example so the test reads like a
// minimal user story.
schema::TableSchema PeopleSchema() {
  schema::TableSchema s;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  id.mode = schema::ColumnMode::kRequired;
  schema::ColumnSchema name;
  name.name = "name";
  name.type = schema::ColumnType::kString;
  name.mode = schema::ColumnMode::kNullable;
  s.columns = {id, name};
  return s;
}

Row MakePerson(int64_t id, absl::string_view name) {
  Row r;
  r.cells = {Value::Int64(id), Value::String(std::string(name))};
  return r;
}

TEST_F(DuckDBStorageTest, RoundTripsHundredRowsAcrossRestart) {
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};

  // ---------------- First process: write 100 rows. -----------------
  {
    auto store_or = DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(store_or.ok()) << store_or.status();
    auto& store = **store_or;

    ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
    ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

    std::vector<Row> rows;
    rows.reserve(100);
    for (int64_t i = 0; i < 100; ++i) {
      rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
    }
    ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

    // Sanity check: in-process scan agrees with what we just wrote.
    auto iter_or = store.ScanRows(table);
    ASSERT_TRUE(iter_or.ok());
    int64_t count = 0;
    Row r;
    while (true) {
      auto has = (*iter_or)->Next(&r);
      ASSERT_TRUE(has.ok());
      if (!*has) break;
      ++count;
    }
    EXPECT_EQ(count, 100);
  }

  // The DuckDBStorage destructor closes the connection and the
  // catalog.duckdb file; reopening below mirrors a process restart.
  EXPECT_TRUE(fs::exists(data_dir_ / "catalog.duckdb"));
  EXPECT_TRUE(fs::exists(data_dir_ / "proj-1" / "ds_1" / "people.parquet"));
  EXPECT_TRUE(fs::exists(data_dir_ / "proj-1" / "ds_1" / "people.meta.json"));

  // ---------------- Second process: read 100 rows back. --------------
  {
    auto store_or = DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(store_or.ok()) << store_or.status();
    auto& store = **store_or;

    auto schema_or = store.GetSchema(table);
    ASSERT_TRUE(schema_or.ok());
    ASSERT_EQ(schema_or->columns.size(), 2u);
    EXPECT_EQ(schema_or->columns[0].name, "id");
    EXPECT_EQ(schema_or->columns[0].type, schema::ColumnType::kInt64);
    EXPECT_EQ(schema_or->columns[1].name, "name");
    EXPECT_EQ(schema_or->columns[1].type, schema::ColumnType::kString);

    auto iter_or = store.ScanRows(table);
    ASSERT_TRUE(iter_or.ok());
    std::vector<Row> scanned;
    Row r;
    while (true) {
      auto has = (*iter_or)->Next(&r);
      ASSERT_TRUE(has.ok());
      if (!*has) break;
      scanned.push_back(r);
    }
    ASSERT_EQ(scanned.size(), 100u);

    // The parquet file does not have a guaranteed order (we did not
    // ORDER BY), so build a set of seen ids and confirm we got
    // exactly 0..99 with the matching name string.
    std::vector<bool> seen(100, false);
    for (const auto& row : scanned) {
      ASSERT_EQ(row.cells.size(), 2u);
      const int64_t id = row.cells[0].int64_value();
      ASSERT_GE(id, 0);
      ASSERT_LT(id, 100);
      EXPECT_FALSE(seen[id]) << "duplicate row for id " << id;
      seen[id] = true;
      EXPECT_EQ(row.cells[1].string_value(), absl::StrCat("person-", id));
    }
    for (size_t i = 0; i < seen.size(); ++i) {
      EXPECT_TRUE(seen[i]) << "missing row for id " << i;
    }
  }
}

TEST_F(DuckDBStorageTest, CreateTableMaterializesEmptyParquet) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  const fs::path parquet = data_dir_ / "proj-1" / "ds_1" / "people.parquet";
  ASSERT_TRUE(fs::exists(parquet));
  EXPECT_GT(fs::file_size(parquet), 0u);

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

TEST_F(DuckDBStorageTest, DropTableRemovesParquetAndSidecar) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  const fs::path parquet = data_dir_ / "proj-1" / "ds_1" / "people.parquet";
  const fs::path sidecar = data_dir_ / "proj-1" / "ds_1" / "people.meta.json";
  ASSERT_TRUE(fs::exists(parquet));
  ASSERT_TRUE(fs::exists(sidecar));

  ASSERT_TRUE(store.DropTable(table).ok());
  EXPECT_FALSE(fs::exists(parquet));
  EXPECT_FALSE(fs::exists(sidecar));
}

TEST_F(DuckDBStorageTest, AppendRowsRejectsMisshapenBatch) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  rows.push_back(MakePerson(1, "ada"));
  Row malformed;
  malformed.cells = {Value::Int64(2)};
  rows.push_back(std::move(malformed));

  auto status = store.AppendRows(table, absl::MakeConstSpan(rows));
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);

  // Misshapen batch must not have leaked the good row at index 0.
  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

TEST_F(DuckDBStorageTest, AppendRowsAppendsAcrossMultipleBatches) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> first;
  first.push_back(MakePerson(1, "ada"));
  first.push_back(MakePerson(2, "linus"));
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(first)).ok());

  std::vector<Row> second;
  second.push_back(MakePerson(3, "grace"));
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(second)).ok());

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  std::vector<Row> scanned;
  Row r;
  while (true) {
    auto has = (*iter_or)->Next(&r);
    ASSERT_TRUE(has.ok());
    if (!*has) break;
    scanned.push_back(r);
  }
  EXPECT_EQ(scanned.size(), 3u);
}

// Schema conversion smoke tests: assert that every BigQuery type the
// plan covers maps to a non-empty DuckDB type name, and that the
// inverse mapping recovers the original kind. Container types are
// exercised via ColumnSchemaToDuckDBType so the test catches the
// nested-rendering path.
// ---------------------------------------------------------------------------
// CreateReadStream (plan 38)
// ---------------------------------------------------------------------------

namespace {

// Drains the iterator into a vector. Reused across CreateReadStream
// tests; the DuckDB backend pre-materializes the rows under the lock,
// so this loop is just a thin wrapper around Next() for symmetry with
// the memory store's test fixture.
std::vector<Row> Drain(std::unique_ptr<RowIterator> iter) {
  std::vector<Row> out;
  Row r;
  while (true) {
    auto has = iter->Next(&r);
    EXPECT_TRUE(has.ok());
    if (!has.ok() || !*has) break;
    out.push_back(r);
  }
  return out;
}

}  // namespace

TEST_F(DuckDBStorageTest, CreateReadStreamReturnsAllRowsByDefault) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  for (int64_t i = 0; i < 5; ++i) {
    rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  auto iter_or = store.CreateReadStream(table, ReadFilter{});
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 5u);
  // CreateReadStream pins the order to the parquet file_row_number,
  // which mirrors INSERT order; rows[i] == person-i.
  for (size_t i = 0; i < scanned.size(); ++i) {
    EXPECT_EQ(scanned[i].cells[0].int64_value(), static_cast<int64_t>(i));
    EXPECT_EQ(scanned[i].cells[1].string_value(), absl::StrCat("person-", i));
  }
}

TEST_F(DuckDBStorageTest, CreateReadStreamHonorsRowLimit) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  for (int64_t i = 0; i < 10; ++i) {
    rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  ReadFilter filter;
  filter.row_limit = 3;
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 3u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 0);
  EXPECT_EQ(scanned[1].cells[0].int64_value(), 1);
  EXPECT_EQ(scanned[2].cells[0].int64_value(), 2);
}

TEST_F(DuckDBStorageTest, CreateReadStreamHonorsOffsetAndLimit) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  for (int64_t i = 0; i < 10; ++i) {
    rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  ReadFilter filter;
  filter.offset = 4;
  filter.row_limit = 3;
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 3u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 4);
  EXPECT_EQ(scanned[1].cells[0].int64_value(), 5);
  EXPECT_EQ(scanned[2].cells[0].int64_value(), 6);
}

TEST_F(DuckDBStorageTest, CreateReadStreamOffsetOnlyReturnsTail) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  for (int64_t i = 0; i < 4; ++i) {
    rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  ReadFilter filter;
  filter.offset = 2;
  // No row_limit -- DuckDB receives LIMIT ALL OFFSET 2 and yields the tail.
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 2u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 2);
  EXPECT_EQ(scanned[1].cells[0].int64_value(), 3);
}

TEST_F(DuckDBStorageTest, CreateReadStreamOnMissingTableIsNotFound) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const TableId table{"proj-1", "ds_1", "ghost"};
  auto iter_or = store.CreateReadStream(table, ReadFilter{});
  ASSERT_FALSE(iter_or.ok());
  EXPECT_EQ(iter_or.status().code(), absl::StatusCode::kNotFound);
}

// ---------------------------------------------------------------------------
// row_restriction predicate pushdown (plan 39)
//
// The handler parses `<column> = <literal>` into a typed
// `EqualityPredicate` and hands it to `CreateReadStream`. The DuckDB
// backend renders the predicate as a `WHERE` clause and lets DuckDB
// push it into the parquet scan. The literal is rendered using the
// same escaping the rest of the .cc uses for INSERT, so the parser's
// quoted-string form (`'O''Reilly'`) round-trips through a literal
// rendering of the parsed unescaped value (`O'Reilly`).
// ---------------------------------------------------------------------------

TEST_F(DuckDBStorageTest, CreateReadStreamFiltersInt64Predicate) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  for (int64_t i = 0; i < 5; ++i) {
    rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  EqualityPredicate pred;
  pred.column = "id";
  pred.column_index = 0;
  pred.kind = EqualityPredicate::Kind::kInt64;
  pred.int64_value = 2;
  ReadFilter filter;
  filter.equality_predicate = pred;
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 1u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 2);
  EXPECT_EQ(scanned[0].cells[1].string_value(), "person-2");
}

TEST_F(DuckDBStorageTest, CreateReadStreamFiltersStringPredicate) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows = {
      MakePerson(1, "ada"),
      // Apostrophe in the cell exercises the literal-escape path on
      // the SQL-side WHERE renderer.
      MakePerson(2, "O'Reilly"),
      MakePerson(3, "grace"),
  };
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  EqualityPredicate pred;
  pred.column = "name";
  pred.column_index = 1;
  pred.kind = EqualityPredicate::Kind::kString;
  pred.string_value = "O'Reilly";
  ReadFilter filter;
  filter.equality_predicate = pred;
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 1u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 2);
  EXPECT_EQ(scanned[0].cells[1].string_value(), "O'Reilly");
}

TEST_F(DuckDBStorageTest, CreateReadStreamPredicateNoMatchYieldsEmpty) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows = {
      MakePerson(1, "ada"),
      MakePerson(2, "linus"),
  };
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  EqualityPredicate pred;
  pred.column = "id";
  pred.column_index = 0;
  pred.kind = EqualityPredicate::Kind::kInt64;
  pred.int64_value = 999;
  ReadFilter filter;
  filter.equality_predicate = pred;
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  EXPECT_TRUE(scanned.empty());
}

// Plan 15 (storage-read-write): selected_fields projection pushdown.
// The DuckDB backend filters the SELECT projection list down to the
// caller-supplied subset and the row decoder reads cells back in the
// projected order. Verifies both the cell count and the projected
// order: passing `[name, id]` returns rows where cells[0] is name
// and cells[1] is id, even though the table declared `[id, name]`.
TEST_F(DuckDBStorageTest, CreateReadStreamProjectsSelectedFields) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  for (int64_t i = 0; i < 3; ++i) {
    rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  ReadFilter filter;
  filter.selected_fields = {"name", "id"};
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  ASSERT_EQ(scanned.size(), 3u);
  for (size_t i = 0; i < scanned.size(); ++i) {
    ASSERT_EQ(scanned[i].cells.size(), 2u);
    EXPECT_EQ(scanned[i].cells[0].string_value(),
              absl::StrCat("person-", i));
    EXPECT_EQ(scanned[i].cells[1].int64_value(), static_cast<int64_t>(i));
  }
}

TEST_F(DuckDBStorageTest, CreateReadStreamRejectsUnknownSelectedField) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());
  ASSERT_TRUE(
      store.AppendRows(table, absl::MakeConstSpan({MakePerson(1, "ada")}))
          .ok());

  ReadFilter filter;
  filter.selected_fields = {"phone"};  // not on the people schema
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_FALSE(iter_or.ok());
  EXPECT_EQ(iter_or.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(DuckDBStorageTest, CreateReadStreamPredicateBeforeOffsetLimit) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  // Two pools of names; predicate keeps only the "odd" name pool.
  for (int64_t i = 0; i < 10; ++i) {
    rows.push_back(MakePerson(i, (i % 2 == 0) ? "even" : "odd"));
  }
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  EqualityPredicate pred;
  pred.column = "name";
  pred.column_index = 1;
  pred.kind = EqualityPredicate::Kind::kString;
  pred.string_value = "odd";
  ReadFilter filter;
  filter.equality_predicate = pred;
  filter.offset = 1;
  filter.row_limit = 2;
  auto iter_or = store.CreateReadStream(table, filter);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<Row> scanned = Drain(std::move(*iter_or));
  // Filtered ids: 1, 3, 5, 7, 9. offset=1, limit=2 → 3, 5.
  ASSERT_EQ(scanned.size(), 2u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 3);
  EXPECT_EQ(scanned[1].cells[0].int64_value(), 5);
}

TEST(SchemaToDuckDBType, RoundTripsAllPlanCoveredTypes) {
  struct Case {
    schema::ColumnType bq = schema::ColumnType::kUnknown;
    absl::string_view duckdb;
  };
  const Case cases[] = {
      {schema::ColumnType::kInt64, "BIGINT"},
      {schema::ColumnType::kFloat64, "DOUBLE"},
      {schema::ColumnType::kBool, "BOOLEAN"},
      {schema::ColumnType::kString, "VARCHAR"},
      {schema::ColumnType::kBytes, "BLOB"},
      {schema::ColumnType::kDate, "DATE"},
      {schema::ColumnType::kTime, "TIME"},
      {schema::ColumnType::kDatetime, "TIMESTAMP"},
      {schema::ColumnType::kTimestamp, "TIMESTAMP WITH TIME ZONE"},
      {schema::ColumnType::kNumeric, "DECIMAL(38, 9)"},
      {schema::ColumnType::kBignumeric, "DECIMAL(38, 38)"},
      {schema::ColumnType::kJson, "JSON"},
  };
  for (const auto& c : cases) {
    EXPECT_EQ(schema::ToDuckDBType(c.bq), c.duckdb)
        << "kind=" << static_cast<int>(c.bq);
    // FromDuckDBType only needs to accept the bare head; the
    // TIMESTAMP WITH TIME ZONE alias falls through the suffix
    // check inside the function and round-trips back to
    // kTimestamp.
    if (c.bq != schema::ColumnType::kBignumeric) {
      EXPECT_EQ(schema::FromDuckDBType(c.duckdb), c.bq)
          << "duckdb=" << c.duckdb;
    }
  }
}

TEST(SchemaToDuckDBType, RendersRepeatedAsList) {
  schema::ColumnSchema col;
  col.name = "tags";
  col.type = schema::ColumnType::kString;
  col.mode = schema::ColumnMode::kRepeated;
  EXPECT_EQ(schema::ColumnSchemaToDuckDBType(col), "VARCHAR[]");
}

TEST(SchemaToDuckDBType, RendersStructWithNestedFields) {
  schema::ColumnSchema col;
  col.name = "person";
  col.type = schema::ColumnType::kStruct;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  schema::ColumnSchema labels;
  labels.name = "labels";
  labels.type = schema::ColumnType::kString;
  labels.mode = schema::ColumnMode::kRepeated;
  col.fields = {id, labels};
  EXPECT_EQ(schema::ColumnSchemaToDuckDBType(col),
            "STRUCT(\"id\" BIGINT, \"labels\" VARCHAR[])");
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
