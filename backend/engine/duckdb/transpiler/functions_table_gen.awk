# functions_table_gen.awk
#
# Converts the line-oriented `functions.yaml` disposition table into a
# C++ initializer list fragment (`functions_table.inc`) the `Functions`
# loader includes inside its static `absl::flat_hash_map<std::string,
# FnEntry>` literal.
#
# The YAML format is intentionally narrow:
#   * blank lines are ignored
#   * `#`-comment lines are ignored
#   * `<key>: <value>` lines map a BigQuery function name to either a
#     DuckDB function name (emit `<NAME>(<args>)`) or the literal
#     `skiplist` / `fallback` markers.
#   * trailing `#` inline comments are stripped.
#
# Anything else aborts the build with a non-zero exit so a malformed
# YAML edit lands as a Bazel error rather than a silent skip.

BEGIN {
    FS = ":"
}

/^[[:space:]]*$/ { next }
/^[[:space:]]*#/ { next }

{
    line = $0
    # Strip inline `# ...` comment.
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
    if (val == "skiplist") {
        printf("    {\"%s\", FnEntry{FnKind::kSkiplist, \"\"}},\n", key)
    } else if (val == "fallback") {
        printf("    {\"%s\", FnEntry{FnKind::kFallback, \"\"}},\n", key)
    } else if (length(val) > 0) {
        printf("    {\"%s\", FnEntry{FnKind::kMap, \"%s\"}},\n", key, val)
    } else {
        printf("functions_table_gen.awk: empty value for key %s\n", key) > "/dev/stderr"
        exit 1
    }
}
