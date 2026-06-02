# functions_table_gen.awk
#
# Converts the line-oriented `functions.yaml` disposition table into
# a C++ initializer list fragment (`functions_table.inc`) the
# `Functions` loader includes inside its static
# `absl::flat_hash_map<std::string, FnEntry>` literal.
#
# The YAML grammar is the same shape as
# `node_dispositions_table_gen.awk`:
#
#   <bq_function>: <disposition> [duckdb_name=<name>]
#                                [plan=<plan>] [status=planned]
#
# Where:
#   * blank lines and `#`-comment lines are ignored
#   * trailing `# ...` inline comments are stripped
#   * `<disposition>` is one of `duckdb_native`, `duckdb_rewrite`,
#     `duckdb_udf`, `semantic_executor`, `control_op`, `unsupported`
#     (must match `backend/engine/disposition.h`'s `Disposition` enum)
#   * `duckdb_name=<NAME>` is mandatory for `duckdb_native` /
#     `duckdb_rewrite` rows (the transpiler emits `<NAME>(<args>)`)
#     and forbidden for the other dispositions (no DuckDB function
#     name to dispatch to)
#   * `plan=<plan-file>` is mandatory for `unsupported` rows; the
#     convention is to point at `specialized-feature-policy.plan.md`
#   * `status=planned` is optional and only valid alongside
#     dispositions whose runtime emit does not yet exist
#     (`duckdb_udf`, `semantic_executor`, `control_op`)
#
# Anything else aborts the build with a non-zero exit so a malformed
# YAML edit lands as a Bazel error rather than as a silent skip at
# run time.
#
# Same awk-only design rationale as `node_dispositions_table_gen.awk`:
# every developer host already has POSIX awk via the GoogleSQL
# hermetic toolchain's coreutils set, so we avoid a srcs_version
# dependency on the Python toolchain.

BEGIN {
    FS = ":"
    # Map of `kFoo` C++ enum spellings keyed by the YAML disposition
    # string. Keep in lock-step with `backend/engine/disposition.h`'s
    # `Disposition` enum and `DispositionToString`.
    kind["duckdb_native"]     = "kDuckdbNative"
    kind["duckdb_rewrite"]    = "kDuckdbRewrite"
    kind["duckdb_udf"]        = "kDuckdbUdf"
    kind["semantic_executor"] = "kSemanticExecutor"
    kind["control_op"]        = "kControlOp"
    kind["local_stub"]        = "kLocalStub"
    kind["unsupported"]       = "kUnsupported"

    # Dispositions whose runtime emit does not yet exist; only these
    # may carry `status=planned`. `local_stub` rows are eligible too
    # because the stub handler for some families (notably JS UDF
    # registration, which depends on plan 13's deferred UDF body
    # storage) may not be in place yet.
    plannable["duckdb_udf"]        = 1
    plannable["semantic_executor"] = 1
    plannable["control_op"]        = 1
    plannable["local_stub"]        = 1

    # Dispositions that emit a `<duckdb_name>(<args>)` call. Only
    # these may carry (and MUST carry) `duckdb_name=...`.
    # `duckdb_udf` rows carry the registered DuckDB UDF / macro name
    # in the same `duckdb_name=` field when the row is ready
    # (status != planned); the transpiler emits the call identically
    # to a `duckdb_native` row, and the UDF body (owned by
    # `backend/engine/duckdb/udf/`) closes the BigQuery semantic gap.
    # A `status=planned` `duckdb_udf` row still must NOT carry a
    # `duckdb_name` (the wrapper is not yet installed).
    needs_duckdb_name["duckdb_native"]  = 1
    needs_duckdb_name["duckdb_rewrite"] = 1
    udf_emit["duckdb_udf"]              = 1
}

/^[[:space:]]*$/ { next }
/^[[:space:]]*#/ { next }

{
    line = $0
    sub(/[[:space:]]*#.*$/, "", line)
    pos = index(line, ":")
    if (pos == 0) {
        printf("functions_table_gen.awk: malformed line: %s\n", $0) > "/dev/stderr"
        exit 1
    }
    key = substr(line, 1, pos - 1)
    val = substr(line, pos + 1)
    sub(/^[[:space:]]+/, "", key)
    sub(/[[:space:]]+$/, "", key)
    sub(/^[[:space:]]+/, "", val)
    sub(/[[:space:]]+$/, "", val)
    if (length(key) == 0) {
        printf("functions_table_gen.awk: empty key on line: %s\n", $0) > "/dev/stderr"
        exit 1
    }
    # Lowercase the key for case-insensitive lookup at runtime.
    key = tolower(key)

    n = split(val, parts, /[[:space:]]+/)
    if (n < 1 || length(parts[1]) == 0) {
        printf("functions_table_gen.awk: missing disposition for key %s\n", key) > "/dev/stderr"
        exit 1
    }
    disposition = parts[1]
    if (!(disposition in kind)) {
        printf("functions_table_gen.awk: unknown disposition %s for key %s (allowed: duckdb_native, duckdb_rewrite, duckdb_udf, semantic_executor, control_op, local_stub, unsupported)\n", disposition, key) > "/dev/stderr"
        exit 1
    }

    duckdb_name = ""
    plan = ""
    status = ""
    for (i = 2; i <= n; i++) {
        tok = parts[i]
        if (length(tok) == 0) continue
        eq = index(tok, "=")
        if (eq <= 1) {
            printf("functions_table_gen.awk: expected key=value token, got %s (key %s)\n", tok, key) > "/dev/stderr"
            exit 1
        }
        meta_key = substr(tok, 1, eq - 1)
        meta_val = substr(tok, eq + 1)
        if (meta_key == "duckdb_name") {
            duckdb_name = meta_val
        } else if (meta_key == "plan") {
            plan = meta_val
        } else if (meta_key == "status") {
            if (meta_val != "planned") {
                printf("functions_table_gen.awk: unknown status %s for key %s (allowed: planned)\n", meta_val, key) > "/dev/stderr"
                exit 1
            }
            status = meta_val
        } else {
            printf("functions_table_gen.awk: unknown metadata key %s for entry %s\n", meta_key, key) > "/dev/stderr"
            exit 1
        }
    }

    if ((disposition in needs_duckdb_name) && length(duckdb_name) == 0) {
        printf("functions_table_gen.awk: %s needs duckdb_name=... for disposition %s\n", key, disposition) > "/dev/stderr"
        exit 1
    }
    # `duckdb_udf` (ready, status != planned) MUST carry duckdb_name
    # (the registered macro name). `duckdb_udf` (status=planned) MUST
    # NOT carry duckdb_name (no wrapper installed yet).
    if ((disposition in udf_emit) && length(status) == 0 && length(duckdb_name) == 0) {
        printf("functions_table_gen.awk: %s is ready %s but missing duckdb_name=... (registered UDF / macro name)\n", key, disposition) > "/dev/stderr"
        exit 1
    }
    if ((disposition in udf_emit) && length(status) > 0 && length(duckdb_name) > 0) {
        printf("functions_table_gen.awk: %s is status=planned %s but carries duckdb_name=%s (the wrapper is not installed yet; drop the duckdb_name= until the UDF lands)\n", key, disposition, duckdb_name) > "/dev/stderr"
        exit 1
    }
    if (!(disposition in needs_duckdb_name) && !(disposition in udf_emit) && length(duckdb_name) > 0) {
        printf("functions_table_gen.awk: %s carries duckdb_name=%s but disposition %s does not emit a DuckDB function call\n", key, duckdb_name, disposition) > "/dev/stderr"
        exit 1
    }
    if (disposition == "unsupported" && length(plan) == 0) {
        printf("functions_table_gen.awk: %s is unsupported but has no plan= pointer (expected specialized-feature-policy.plan.md)\n", key) > "/dev/stderr"
        exit 1
    }
    # Same posture-row contract for `local_stub`: every stub
    # function row must point at the owning policy plan so a reader
    # can trace why the stub was chosen and where the documented
    # contract lives. `specialized-feature-policy.plan.md` is the
    # canonical owner.
    if (disposition == "local_stub" && length(plan) == 0) {
        printf("functions_table_gen.awk: %s is local_stub but has no plan= pointer (expected specialized-feature-policy.plan.md)\n", key) > "/dev/stderr"
        exit 1
    }
    if (length(status) > 0 && !(disposition in plannable)) {
        printf("functions_table_gen.awk: %s carries status=planned but disposition %s already has a runtime emit\n", key, disposition) > "/dev/stderr"
        exit 1
    }

    is_planned = (length(status) > 0) ? "true" : "false"
    printf("    {\"%s\", FnEntry{Disposition::%s, \"%s\", \"%s\", %s}},\n",
           key, kind[disposition], duckdb_name, plan, is_planned)
}
