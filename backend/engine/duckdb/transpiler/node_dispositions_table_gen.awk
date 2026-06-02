# node_dispositions_table_gen.awk
#
# Converts the line-oriented `node_dispositions.yaml` registry into a
# C++ initializer fragment (`node_dispositions_table.inc`) that
# `node_dispositions.cc` `#include`s inside its static
# `absl::flat_hash_map<std::string, NodeDispositionEntry>` literal.
#
# The YAML grammar is intentionally narrow so the generator does not
# need a real YAML parser (which would mean adding a Python dep, or
# building a one-shot YAML library — neither pays for itself on a
# table this small):
#
#   * blank lines are ignored
#   * `#`-comment lines are ignored
#   * `<NodeKind>: <disposition> [plan=<plan>] [status=<status>]`
#     lines map a single GoogleSQL `ResolvedAST` class name to its
#     six-route disposition plus optional `plan=` and `status=`
#     metadata. Order of `key=value` tokens does not matter; unknown
#     keys abort the build.
#   * trailing `# ...` inline comments are stripped.
#
# Anything else aborts the build with a non-zero exit so a malformed
# YAML edit lands as a Bazel error rather than a silent skip at run
# time.
#
# The same awk-only design rationale as `functions_table_gen.awk`
# applies: every developer host already has POSIX awk via the
# GoogleSQL hermetic toolchain's coreutils, so we avoid a srcs_version
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
}

/^[[:space:]]*$/ { next }
/^[[:space:]]*#/ { next }

{
    line = $0
    # Strip inline `# ...` comment.
    sub(/[[:space:]]*#.*$/, "", line)
    pos = index(line, ":")
    if (pos == 0) {
        printf("node_dispositions_table_gen.awk: malformed line: %s\n", $0) > "/dev/stderr"
        exit 1
    }
    key = substr(line, 1, pos - 1)
    val = substr(line, pos + 1)
    sub(/^[[:space:]]+/, "", key)
    sub(/[[:space:]]+$/, "", key)
    sub(/^[[:space:]]+/, "", val)
    sub(/[[:space:]]+$/, "", val)
    if (length(key) == 0) {
        printf("node_dispositions_table_gen.awk: empty key on line: %s\n", $0) > "/dev/stderr"
        exit 1
    }

    n = split(val, parts, /[[:space:]]+/)
    if (n < 1 || length(parts[1]) == 0) {
        printf("node_dispositions_table_gen.awk: missing disposition for key %s\n", key) > "/dev/stderr"
        exit 1
    }
    disposition = parts[1]
    if (!(disposition in kind)) {
        printf("node_dispositions_table_gen.awk: unknown disposition %s for key %s (allowed: duckdb_native, duckdb_rewrite, duckdb_udf, semantic_executor, control_op, local_stub, unsupported)\n", disposition, key) > "/dev/stderr"
        exit 1
    }

    plan = ""
    status = ""
    for (i = 2; i <= n; i++) {
        tok = parts[i]
        if (length(tok) == 0) continue
        eq = index(tok, "=")
        if (eq <= 1) {
            printf("node_dispositions_table_gen.awk: expected key=value token, got %s (key %s)\n", tok, key) > "/dev/stderr"
            exit 1
        }
        meta_key = substr(tok, 1, eq - 1)
        meta_val = substr(tok, eq + 1)
        if (meta_key == "plan") {
            plan = meta_val
        } else if (meta_key == "status") {
            if (meta_val != "planned") {
                printf("node_dispositions_table_gen.awk: unknown status %s for key %s (allowed: planned)\n", meta_val, key) > "/dev/stderr"
                exit 1
            }
            status = meta_val
        } else {
            printf("node_dispositions_table_gen.awk: unknown metadata key %s for entry %s\n", meta_key, key) > "/dev/stderr"
            exit 1
        }
    }

    # Enforce: every `unsupported` row must carry a `plan=` pointer
    # so a reader can trace the deliberate posture. This matches the
    # plan's done-criteria: every kUnsupported row points at
    # `specialized-feature-policy.plan.md`.
    if (disposition == "unsupported" && length(plan) == 0) {
        printf("node_dispositions_table_gen.awk: %s is unsupported but has no plan= pointer (expected specialized-feature-policy.plan.md)\n", key) > "/dev/stderr"
        exit 1
    }
    # Same contract for `local_stub`: every stub row points at the
    # owning policy plan so a reader can trace the deliberate-stub
    # posture. `specialized-feature-policy.plan.md` is the canonical
    # owner; the YAML loader does not pin the value, it only
    # requires that one is set.
    if (disposition == "local_stub" && length(plan) == 0) {
        printf("node_dispositions_table_gen.awk: %s is local_stub but has no plan= pointer (expected specialized-feature-policy.plan.md)\n", key) > "/dev/stderr"
        exit 1
    }

    is_planned = (length(status) > 0) ? "true" : "false"
    printf("    {\"%s\", NodeDispositionEntry{Disposition::%s, \"%s\", %s}},\n",
           key, kind[disposition], plan, is_planned)
}
