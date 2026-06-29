// Tests for the `RowSource` adapters.
//
// `MaterializedRowSource` and `DrainRowSource` are the
// composition primitives downstream plans use to wrap fast-path
// row producers as semantic-executor inputs. The tests here pin
// the streaming contract (rows come out in order, EOF is sticky,
// schema is preserved) so future plans can rely on it without
// re-deriving the semantics from the source.

#include "backend/engine/semantic/row_source.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

schema::TableSchema MakeSingleInt64Schema() {
  schema::TableSchema s;
  schema::ColumnSchema c;
  c.name = "x";
  c.type = schema::ColumnType::kInt64;
  c.mode = schema::ColumnMode::kNullable;
  s.columns.push_back(c);
  return s;
}

storage::Row MakeIntRow(int64_t v) {
  storage::Row r;
  r.cells.push_back(storage::Value::Int64(v));
  return r;
}

TEST(MaterializedRowSourceTest, StreamsRowsAndEofIsSticky) {
  std::vector<storage::Row> rows = {MakeIntRow(1), MakeIntRow(2)};
  MaterializedRowSource src(MakeSingleInt64Schema(), std::move(rows));
  EXPECT_EQ(src.schema().columns.size(), 1u);

  storage::Row row;
  auto has = src.Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_TRUE(*has);
  EXPECT_EQ(row.cells[0].int64_value(), 1);

  has = src.Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_TRUE(*has);
  EXPECT_EQ(row.cells[0].int64_value(), 2);

  has = src.Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);

  // EOF is sticky: a second Next call after EOF must still return
  // false; the row buffer is left unchanged.
  has = src.Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

TEST(MaterializedRowSourceTest, NullRowArgumentRejected) {
  MaterializedRowSource src(MakeSingleInt64Schema(), {});
  auto has = src.Next(nullptr);
  ASSERT_FALSE(has.ok());
  EXPECT_EQ(has.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST(DrainRowSourceTest, ConsumesEverythingFromUpstream) {
  // Build an upstream MaterializedRowSource (acting as the
  // analog of a DuckDB-backed RowSource for test purposes) and
  // drain it through the adapter; the adapter must preserve
  // schema and produce the same rows.
  std::vector<storage::Row> rows = {
      MakeIntRow(10), MakeIntRow(20), MakeIntRow(30)};
  MaterializedRowSource upstream(MakeSingleInt64Schema(), std::move(rows));

  auto drained_or = DrainRowSource(upstream);
  ASSERT_TRUE(drained_or.ok()) << drained_or.status();
  auto drained = *std::move(drained_or);
  ASSERT_EQ(drained->schema().columns.size(), 1u);

  std::vector<int64_t> seen;
  storage::Row row;
  while (true) {
    auto has = drained->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    seen.push_back(row.cells[0].int64_value());
  }
  EXPECT_EQ(seen, (std::vector<int64_t>{10, 20, 30}));
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
