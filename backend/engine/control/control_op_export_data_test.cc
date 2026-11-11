// EXPORT DATA tests for gs:// destinations. The exporter uploads through
// the storage emulator's JSON API (`STORAGE_EMULATOR_HOST`), so these tests
// stand up a minimal in-process HTTP server that records each request and
// accepts media uploads, in place of fake-gcs-server.

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/control/control_op_executor.h"
#include "backend/engine/control/gcs_uri_resolver.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace {

namespace fs = std::filesystem;

struct RecordedRequest {
  std::string method{};
  std::string target{};
  std::string body{};
};

// FakeGcsServer answers every POST with 200 and every other method with 404,
// recording the request line and body. One connection at a time is enough:
// the exporter shells out to curl once per object.
class FakeGcsServer {
 public:
  FakeGcsServer() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (fd_ < 0 ||
        ::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
        ::listen(fd_, 8) != 0) {
      return;
    }
    socklen_t len = sizeof(addr);
    ::getsockname(fd_, reinterpret_cast<sockaddr*>(&addr), &len);
    port_ = ntohs(addr.sin_port);
    thread_ = std::thread([this] { Serve(); });
  }

  ~FakeGcsServer() {
    if (fd_ >= 0) {
      ::shutdown(fd_, SHUT_RDWR);
      ::close(fd_);
    }
    if (thread_.joinable()) thread_.join();
  }

  int port() const {
    return port_;
  }

  std::vector<RecordedRequest> requests() {
    std::lock_guard<std::mutex> lock(mu_);
    return requests_;
  }

 private:
  void Serve() {
    while (true) {
      const int conn = ::accept(fd_, nullptr, nullptr);
      if (conn < 0) return;
      Handle(conn);
      ::close(conn);
    }
  }

  void Handle(int conn) {
    std::string buf;
    char chunk[4096];
    size_t header_end = std::string::npos;
    while (header_end == std::string::npos) {
      const ssize_t n = ::recv(conn, chunk, sizeof(chunk), 0);
      if (n <= 0) return;
      buf.append(chunk, static_cast<size_t>(n));
      header_end = buf.find("\r\n\r\n");
    }
    const std::string headers = buf.substr(0, header_end);
    std::string body = buf.substr(header_end + 4);
    RecordedRequest req;
    const size_t sp1 = headers.find(' ');
    const size_t sp2 = headers.find(' ', sp1 + 1);
    req.method = headers.substr(0, sp1);
    req.target = headers.substr(sp1 + 1, sp2 - sp1 - 1);
    const std::string lower = absl::AsciiStrToLower(headers);
    size_t content_length = 0;
    if (size_t at = lower.find("content-length:"); at != std::string::npos) {
      const size_t eol = lower.find("\r\n", at);
      absl::string_view value = absl::StripAsciiWhitespace(
          absl::string_view(lower).substr(at + 15, eol - at - 15));
      (void)absl::SimpleAtoi(value, &content_length);
    }
    if (absl::StrContains(lower, "expect: 100-continue")) {
      const std::string cont = "HTTP/1.1 100 Continue\r\n\r\n";
      (void)::send(conn, cont.data(), cont.size(), 0);
    }
    while (body.size() < content_length) {
      const ssize_t n = ::recv(conn, chunk, sizeof(chunk), 0);
      if (n <= 0) break;
      body.append(chunk, static_cast<size_t>(n));
    }
    req.body = std::move(body);
    const bool ok = req.method == "POST";
    {
      std::lock_guard<std::mutex> lock(mu_);
      requests_.push_back(std::move(req));
    }
    const std::string reply =
        ok ? "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
             "Content-Length: 2\r\nConnection: close\r\n\r\n{}"
           : "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n"
             "Connection: close\r\n\r\n";
    (void)::send(conn, reply.data(), reply.size(), 0);
  }

  int fd_ = -1;
  int port_ = 0;
  std::thread thread_{};
  std::mutex mu_{};
  std::vector<RecordedRequest> requests_{};
};

::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::AnalyzerOptions options(MakeLanguageOptions());
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

class ControlOpExportDataTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_GT(server_.port(), 0) << "fake GCS server failed to listen";
    const char* prior = std::getenv("STORAGE_EMULATOR_HOST");
    if (prior != nullptr) prior_storage_host_ = prior;
    ::setenv("STORAGE_EMULATOR_HOST",
             absl::StrCat("http://127.0.0.1:", server_.port()).c_str(),
             /*overwrite=*/1);
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-control-op-export-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    executor_ = std::make_unique<ControlOpExecutor>(storage_.get());
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    CreatePeopleTable();
  }

  void TearDown() override {
    executor_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    if (prior_storage_host_.has_value()) {
      ::setenv("STORAGE_EMULATOR_HOST", prior_storage_host_->c_str(), 1);
    } else {
      ::unsetenv("STORAGE_EMULATOR_HOST");
    }
  }

  void CreatePeopleTable() {
    schema::TableSchema bq_schema;
    schema::ColumnSchema id;
    id.name = "id";
    id.type = schema::ColumnType::kInt64;
    id.mode = schema::ColumnMode::kRequired;
    bq_schema.columns.push_back(id);
    schema::ColumnSchema name;
    name.name = "name";
    name.type = schema::ColumnType::kString;
    name.mode = schema::ColumnMode::kNullable;
    bq_schema.columns.push_back(name);
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "people"}, bq_schema).ok());
    std::vector<storage::Row> rows = {
        storage::Row{{storage::Value::Int64(1), storage::Value::String("ada")}},
        storage::Row{
            {storage::Value::Int64(2), storage::Value::String("linus")}},
    };
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "people"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  absl::Status RunDdl(absl::string_view sql) {
    ::googlesql::TypeFactory catalog_types;
    catalog::GoogleSqlCatalog catalog(
        "proj-test", storage_.get(), &catalog_types, MakeLanguageOptions());
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    ::googlesql::TypeFactory type_factory;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status analyze = ::googlesql::AnalyzeStatement(
        sql, options, &catalog, &type_factory, &output);
    if (!analyze.ok()) return analyze;
    QueryRequest req;
    req.project_id = "proj-test";
    req.sql = std::string(sql);
    return executor_->ExecuteDdl(req, *output->resolved_statement(), &catalog);
  }

  FakeGcsServer server_{};
  std::optional<std::string> prior_storage_host_{};
  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<ControlOpExecutor> executor_{};
};

TEST(ExportShardUriTest, ReplacesTheWildcardWithTheFirstShardNumber) {
  auto uri = internal::ExportShardUri("gs://b/prefix/part-*.parquet");
  ASSERT_TRUE(uri.ok()) << uri.status();
  EXPECT_EQ(*uri, "gs://b/prefix/part-000000000000.parquet");
}

TEST(ExportShardUriTest, LeavesAUriWithoutAWildcardUnchanged) {
  auto uri = internal::ExportShardUri("gs://b/prefix/out.csv");
  ASSERT_TRUE(uri.ok()) << uri.status();
  EXPECT_EQ(*uri, "gs://b/prefix/out.csv");
}

TEST(ExportShardUriTest, RejectsMoreThanOneWildcard) {
  auto uri = internal::ExportShardUri("gs://b/*/part-*.parquet");
  EXPECT_EQ(uri.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(ControlOpExportDataTest, ExportToGcsUploadsWithoutFetchingTheTarget) {
  // Regression: the destination was resolved with `LocalPathFromUri`, which
  // downloads gs:// objects into the cache first; a new export target does
  // not exist yet, so the export failed before writing anything.
  absl::Status s = RunDdl(
      "EXPORT DATA OPTIONS(uri='gs://exports/people/out.csv', format='CSV') "
      "AS SELECT id, name FROM ds.people ORDER BY id");
  ASSERT_TRUE(s.ok()) << s;

  std::vector<RecordedRequest> reqs = server_.requests();
  ASSERT_EQ(reqs.size(), 1u);
  EXPECT_EQ(reqs[0].method, "POST");
  EXPECT_EQ(reqs[0].target,
            "/upload/storage/v1/b/exports/o?uploadType=media&name=people%2Fout."
            "csv");
  EXPECT_EQ(reqs[0].body, "id,name\n1,ada\n2,linus\n");
}

TEST_F(ControlOpExportDataTest, ExportWildcardUploadsTheFirstShard) {
  absl::Status s = RunDdl(
      "EXPORT DATA OPTIONS(uri='gs://exports/delta/part-*.parquet', "
      "format='PARQUET', overwrite=true) "
      "AS SELECT * FROM ds.people");
  ASSERT_TRUE(s.ok()) << s;

  std::vector<RecordedRequest> reqs = server_.requests();
  ASSERT_EQ(reqs.size(), 1u);
  EXPECT_EQ(reqs[0].method, "POST");
  EXPECT_EQ(reqs[0].target,
            "/upload/storage/v1/b/exports/o?uploadType=media&name=delta%2Fpart-"
            "000000000000.parquet");
  ASSERT_GE(reqs[0].body.size(), 8u);
  EXPECT_EQ(reqs[0].body.substr(0, 4), "PAR1");
  EXPECT_EQ(reqs[0].body.substr(reqs[0].body.size() - 4), "PAR1");
}

}  // namespace
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
