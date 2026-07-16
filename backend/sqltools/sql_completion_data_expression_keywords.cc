#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {
const std::vector<std::string>& ExpressionKeywordEntries() {
  static const std::vector<std::string>* kEntries =
      new std::vector<std::string>{
          "ACCESS",       "ADD",          "AFTER",
          "AGGREGATE",    "ALL",          "ALTER",
          "AND",          "ANY",          "ARRAY",
          "AS",           "ASC",          "ASSERT_ROWS_MODIFIED",
          "ASSIGNMENT",   "AT",           "BEGIN",
          "BETWEEN",      "BI_CAPACITY",  "BY",
          "CAPACITY",     "CASCADE",      "CASE",
          "CAST",         "CLONE",        "CLUSTER",
          "COLLATE",      "COLUMN",       "COLUMNS",
          "COMMIT",       "CONNECTION",   "CONSTRAINT",
          "CREATE",       "CROSS",        "CUBE",
          "CURRENT",      "DATA",         "DEFAULT",
          "DEFINE",       "DESC",         "DETERMINISTIC",
          "DISTINCT",     "DO",           "DROP",
          "ELSE",         "ELSEIF",       "END",
          "ENFORCED",     "ENUM",         "ESCAPE",
          "EXCEPT",       "EXCLUDE",      "EXECUTE",
          "EXISTS",       "EXPORT",       "EXTEND",
          "EXTERNAL",     "EXTRACT",      "FALSE",
          "FETCH",        "FILES",        "FILTER",
          "FIRST",        "FOLLOWING",    "FOR",
          "FOREIGN",      "FULL",         "FUNCTION",
          "GENERATED",    "GRANT",        "GROUP",
          "GROUPING",     "GROUPS",       "HASH",
          "IDENTITY",     "IF",           "IGNORE",
          "IMMEDIATE",    "IN",           "INCREMENT",
          "INDEX",        "INNER",        "INOUT",
          "INTERSECT",    "INTERVAL",     "INTO",
          "IS",           "KEY",          "LANGUAGE",
          "LAST",         "LATERAL",      "LEFT",
          "LIKE",         "LOAD",         "LOOKUP",
          "MATCH",        "MATCHED",      "MATCH_RECOGNIZE",
          "MATERIALIZED", "MEASURES",     "MERGE",
          "METADATA",     "MODEL",        "NATURAL",
          "NEW",          "NEXT",         "NO",
          "NOT",          "NULL",         "NULLS",
          "OF",           "OPTIONS",      "OR",
          "ORDER",        "ORGANIZATION", "OUT",
          "OUTER",        "OVER",         "OVERWRITE",
          "PARTITION",    "PARTITIONS",   "PAST",
          "PATTERN",      "PIVOT",        "POLICIES",
          "POLICY",       "PRECEDING",    "PREV",
          "PRIMARY",      "PROCEDURE",    "PROJECT",
          "PROTO",        "QUALIFY",      "RANGE",
          "RECURSIVE",    "REFERENCES",   "RENAME",
          "REPEAT",       "REPLICA",      "RESERVATION",
          "RESPECT",      "RESTRICT",     "RETURNS",
          "RIGHT",        "ROLLBACK",     "ROLLUP",
          "ROW",          "ROWS",         "SCHEMA",
          "SEARCH",       "SET",          "SETS",
          "SKIP",         "SNAPSHOT",     "SOME",
          "SOURCE",       "START",        "STRUCT",
          "TABLE",        "TABLESAMPLE",  "TARGET",
          "TEMP",         "TEMPORARY",    "THEN",
          "TO",           "TRANSACTION",  "TREAT",
          "TRUE",         "TRUNCATE",     "UNBOUNDED",
          "UNDROP",       "UNION",        "UNNEST",
          "UNPIVOT",      "UNTIL",        "USING",
          "VIEW",         "WHEN",         "WITH",
          "WITHIN",
      };
  return *kEntries;
}

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
