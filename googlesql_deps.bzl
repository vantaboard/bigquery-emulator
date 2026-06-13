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

# Same pattern for TVF headers (`sql_tvf.h`, `templated_sql_tvf.h`).
GOOGLESQL_SQL_TVF_DEPS = select({
    "//:googlesql_source": [
        "@googlesql//googlesql/public:sql_tvf",
        "@googlesql//googlesql/public:templated_sql_tvf",
    ],
    "//conditions:default": [],
})

# MATCH_RECOGNIZE pattern compiler (`CompiledPattern` / `MatchPartition`).
# Prebuilt mode resolves headers via `//googlesql/resolved_ast` -> `:_all_hdrs`
# and links symbols from `:_archive`.
GOOGLESQL_MATCH_RECOGNIZE_DEPS = select({
    "//:googlesql_source": [
        "@googlesql//googlesql/public/functions/match_recognize:compiled_pattern",
    ],
    "//conditions:default": [],
})
