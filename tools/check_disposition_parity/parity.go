package main

import (
	"fmt"
	"sort"
	"strings"
)

// Route name constants. Shared with
// `backend/engine/disposition.h`, `node_dispositions.yaml`, and
// `SHAPE_TRACKER.md`. Adding a new route means editing the enum,
// both data files, the generators, and this file in lock-step.
const (
	routeDuckdbNative     = "duckdb_native"
	routeDuckdbRewrite    = "duckdb_rewrite"
	routeDuckdbUDF        = "duckdb_udf"
	routeSemanticExecutor = "semantic_executor"
	routeControlOp        = "control_op"
	routeLocalStub        = "local_stub"
	routeUnsupported      = "unsupported"
)

// validDispositions is the closed set of route names.
var validDispositions = map[string]struct{}{
	routeDuckdbNative:     {},
	routeDuckdbRewrite:    {},
	routeDuckdbUDF:        {},
	routeSemanticExecutor: {},
	routeControlOp:        {},
	routeLocalStub:        {},
	routeUnsupported:      {},
}

// yamlRow is a single entry from node_dispositions.yaml.
type yamlRow struct {
	// node is the GoogleSQL ResolvedAST class name (no backticks,
	// case-sensitive).
	node string
	// disposition is the lowercase route name (one of
	// validDispositions).
	disposition string
	// lineNumber points back at the source YAML line for diagnostics.
	lineNumber int
}

// shapeRow is a single row from the SHAPE_TRACKER.md tables.
type shapeRow struct {
	// nodes is the list of node-kind tokens the row's `Node` cell
	// covered (composite cells like `Foo / Bar` and wildcards like
	// `ResolvedGraph*Scan` expand into multiple tokens; wildcards
	// keep their `*` and are matched against the YAML by
	// matchesWildcard below).
	nodes []string
	// disposition is the lowercase route name from the row's
	// `Status` cell (one of validDispositions). Any suffix the
	// status cell carried (e.g. `(subset)`) is stripped.
	disposition string
	// lineNumber points back at the source markdown line for
	// diagnostics.
	lineNumber int
}

// parseYAML is the line-oriented reader for node_dispositions.yaml.
//
// We re-implement a narrow YAML reader rather than pull in
// gopkg.in/yaml.v3 so the parity check inherits the same "no extra
// deps to bootstrap" property the awk-based table generator already
// has. The same grammar:
//
//	<NodeKind>: <disposition> [status=planned]
//
// applies; we only care about the `<NodeKind>` and `<disposition>`
// tokens here (the plan / status metadata is relevant to the
// generator and to runtime callers, not to parity).
func parseYAML(src string) ([]yamlRow, error) {
	var rows []yamlRow
	for lineNo, raw := range strings.Split(src, "\n") {
		line := stripInlineComment(raw)
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		before, after, ok := strings.Cut(line, ":")
		if !ok {
			return nil, fmt.Errorf("line %d: missing `:` separator: %q",
				lineNo+1, raw)
		}
		key := strings.TrimSpace(before)
		rest := strings.TrimSpace(after)
		if key == "" {
			return nil, fmt.Errorf("line %d: empty node-kind key: %q",
				lineNo+1, raw)
		}
		tokens := strings.Fields(rest)
		if len(tokens) == 0 {
			return nil, fmt.Errorf("line %d: missing disposition for %q",
				lineNo+1, key)
		}
		disp := tokens[0]
		if _, ok := validDispositions[disp]; !ok {
			return nil, fmt.Errorf(
				"line %d: unknown disposition %q for node %q",
				lineNo+1, disp, key)
		}
		rows = append(rows, yamlRow{
			node:        key,
			disposition: disp,
			lineNumber:  lineNo + 1,
		})
	}
	return rows, nil
}

// stripInlineComment removes a trailing `# ...` from a YAML line. We
// take a deliberately narrow view of "inline comment": any `#`
// preceded by whitespace (or at column 0) starts a comment. That is
// enough for the disposition tables (which never put `#` inside a
// node name or a disposition word) and keeps the parser tiny.
func stripInlineComment(s string) string {
	for i := 0; i < len(s); i++ {
		if s[i] != '#' {
			continue
		}
		if i == 0 || s[i-1] == ' ' || s[i-1] == '\t' {
			return s[:i]
		}
	}
	return s
}

// parseShapeTracker extracts the per-node disposition table rows
// from SHAPE_TRACKER.md. Only table rows whose Node cell contains
// at least one identifier matching "starts with `Resolved`" are
// considered; that filters out the explanatory header rows and the
// summary paragraphs that follow each table.
func parseShapeTracker(src string) ([]shapeRow, error) {
	var rows []shapeRow
	for lineNo, raw := range strings.Split(src, "\n") {
		line := strings.TrimSpace(raw)
		if !strings.HasPrefix(line, "|") {
			continue
		}
		// Skip the table header / separator lines (the separator
		// has `---` cells, the header has the literal "Node" label).
		if strings.Contains(line, "---") {
			continue
		}
		cells := splitMarkdownRow(line)
		if len(cells) < 2 {
			continue
		}
		nodeCell := strings.TrimSpace(cells[0])
		// The header row's first cell is `Node`, which we drop here
		// (and also any other non-Resolved-prefixed first cell).
		nodes := extractNodes(nodeCell)
		if len(nodes) == 0 {
			continue
		}
		statusCell := strings.TrimSpace(cells[1])
		disposition, err := extractDisposition(statusCell)
		if err != nil {
			return nil, fmt.Errorf("line %d: %w", lineNo+1, err)
		}
		rows = append(rows, shapeRow{
			nodes:       nodes,
			disposition: disposition,
			lineNumber:  lineNo + 1,
		})
	}
	return rows, nil
}

// splitMarkdownRow splits a `| a | b | c |` pipe-table row into its
// cells. Backticks may contain `|` characters in some markdown
// dialects; SHAPE_TRACKER.md does not exercise that case so we use
// the simple split-on-`|` approach instead of dragging in a real
// markdown parser.
func splitMarkdownRow(line string) []string {
	line = strings.TrimPrefix(line, "|")
	line = strings.TrimSuffix(line, "|")
	return strings.Split(line, "|")
}

// extractNodes pulls the per-class identifiers out of a `Node` cell.
// Supports:
//
//   - single class names (“ `ResolvedQueryStmt` “)
//   - slash-joined composite rows (“ `Foo` / `Bar` “)
//   - wildcard families (“ `ResolvedGraph*Scan` “)
//
// Everything that does not look like a class identifier (e.g. the
// header row's literal "Node" text) returns an empty slice so the
// caller skips the row.
func extractNodes(cell string) []string {
	cell = strings.ReplaceAll(cell, "\\*", "*")
	parts := strings.Split(cell, "/")
	var out []string
	for _, p := range parts {
		p = strings.TrimSpace(p)
		p = strings.Trim(p, "`")
		p = strings.TrimSpace(p)
		if p == "" {
			continue
		}
		// We only care about identifiers; anything else (the
		// header row's "Node" label, the column-separator's
		// `---`, etc.) does not start with `Resolved` and would
		// have already been filtered by the caller.
		if !strings.HasPrefix(p, "Resolved") {
			return nil
		}
		out = append(out, p)
	}
	return out
}

// extractDisposition pulls the canonical route name out of a Status
// cell. Status cells look like:
//
//	`duckdb_native`
//	`duckdb_native` (subset)
//	`duckdb_native` (planned)
//
// We strip the trailing `(subset)` / `(planned)` annotations and
// keep the first backticked word as the disposition.
func extractDisposition(cell string) (string, error) {
	open := strings.IndexByte(cell, '`')
	if open == -1 {
		return "", fmt.Errorf("status cell has no backticked disposition: %q", cell)
	}
	close := strings.IndexByte(cell[open+1:], '`')
	if close == -1 {
		return "", fmt.Errorf("unterminated backtick in status cell: %q", cell)
	}
	word := strings.TrimSpace(cell[open+1 : open+1+close])
	if _, ok := validDispositions[word]; !ok {
		return "", fmt.Errorf("unknown disposition %q in status cell: %q",
			word, cell)
	}
	return word, nil
}

// compareParity returns a sorted list of findings -- empty when the
// two sources agree. Findings cover three cases:
//
//  1. A SHAPE_TRACKER node has no matching YAML row.
//  2. A SHAPE_TRACKER node's disposition disagrees with the YAML row.
//  3. A YAML node has no matching SHAPE_TRACKER row.
//
// SHAPE_TRACKER wildcards (`ResolvedGraph*Scan`) match every YAML
// node whose name fits the pattern; every matched YAML row must
// share the wildcard's disposition.
func compareParity(yaml []yamlRow, shape []shapeRow) []string {
	yamlByName := make(map[string]yamlRow, len(yaml))
	for _, r := range yaml {
		yamlByName[r.node] = r
	}
	seen := make(map[string]bool, len(yaml))
	var out []string
	for _, row := range shape {
		out = append(out, walkShapeRow(row, yaml, yamlByName, seen)...)
	}
	for _, yr := range yaml {
		if seen[yr.node] {
			continue
		}
		out = append(out, fmt.Sprintf(
			"node_dispositions.yaml line %d has %s -> %s but "+
				"SHAPE_TRACKER.md has no matching row",
			yr.lineNumber, yr.node, yr.disposition))
	}
	sort.Strings(out)
	return out
}

// walkShapeRow expands one SHAPE_TRACKER row into per-token
// findings (exact + wildcard) and marks the matched YAML rows in
// `seen`. Pulled out so `compareParity` stays under the funlen
// linter cap and so the exact/wildcard branches are testable in
// isolation if a future plan grows them.
func walkShapeRow(
	row shapeRow,
	yaml []yamlRow,
	yamlByName map[string]yamlRow,
	seen map[string]bool,
) []string {
	var out []string
	for _, token := range row.nodes {
		if strings.Contains(token, "*") {
			out = append(out, expandWildcard(row, token, yaml, seen)...)
			continue
		}
		yr, ok := yamlByName[token]
		if !ok {
			out = append(out, fmt.Sprintf(
				"SHAPE_TRACKER.md line %d references %s but "+
					"node_dispositions.yaml has no matching row",
				row.lineNumber, token))
			continue
		}
		seen[yr.node] = true
		if yr.disposition != row.disposition {
			out = append(out, mismatchFinding(row, yr, token))
		}
	}
	return out
}

// expandWildcard handles the `Foo*Bar` form of a SHAPE_TRACKER row
// token: it must match at least one YAML row, and every matched
// row's disposition must agree with the wildcard row's.
func expandWildcard(
	row shapeRow,
	token string,
	yaml []yamlRow,
	seen map[string]bool,
) []string {
	matched := false
	var out []string
	for _, yr := range yaml {
		if !matchesWildcard(token, yr.node) {
			continue
		}
		matched = true
		seen[yr.node] = true
		if yr.disposition != row.disposition {
			out = append(out, mismatchFinding(row, yr, yr.node))
		}
	}
	if !matched {
		out = append(out, fmt.Sprintf(
			"SHAPE_TRACKER.md line %d references wildcard %s "+
				"but node_dispositions.yaml has no matching row",
			row.lineNumber, token))
	}
	return out
}

// mismatchFinding is the canonical disposition-disagreement
// message format. Centralised so the two call sites in
// walkShapeRow / expandWildcard cannot drift.
func mismatchFinding(row shapeRow, yr yamlRow, displayName string) string {
	return fmt.Sprintf(
		"disposition mismatch: SHAPE_TRACKER.md line %d says %s -> %s, "+
			"node_dispositions.yaml line %d says %s -> %s",
		row.lineNumber, displayName, row.disposition,
		yr.lineNumber, yr.node, yr.disposition)
}

// matchesWildcard reports whether `name` matches the glob-style
// `pattern`. Only `*` is supported (it stands for any run of
// characters); SHAPE_TRACKER's wildcards only use the `*` form
// (e.g. `ResolvedGraph*Scan`).
func matchesWildcard(pattern, name string) bool {
	if !strings.Contains(pattern, "*") {
		return pattern == name
	}
	parts := strings.Split(pattern, "*")
	if !strings.HasPrefix(name, parts[0]) {
		return false
	}
	rest := name[len(parts[0]):]
	for i := 1; i < len(parts); i++ {
		p := parts[i]
		if i == len(parts)-1 {
			return strings.HasSuffix(rest, p)
		}
		idx := strings.Index(rest, p)
		if idx == -1 {
			return false
		}
		rest = rest[idx+len(p):]
	}
	return true
}
