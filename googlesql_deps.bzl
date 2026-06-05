"""Shared GoogleSQL dependency lists for first-party cc_library rules."""

# Source-mode-only labels. Prebuilt wrappers resolve `sql_function.h` and
# `templated_sql_function.h` transitively via `:function` -> `:_all_hdrs`.
GOOGLESQL_SQL_FUNCTION_DEPS = select({
    "//:googlesql_source": [
        "@googlesql//googlesql/public:sql_function",
        "@googlesql//googlesql/public:templated_sql_function",
    ],
    "//conditions:default": [],
})
