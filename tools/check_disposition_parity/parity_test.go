package main

import (
	"strings"
	"testing"
)

// String constants reused across the table-driven tests. Pulled
// out as package-level constants so golangci-lint's `goconst` does
// not flag the repeated literals as a refactor opportunity, and so
// a future spelling change touches one place.
const (
	testNodeQueryStmt   = "ResolvedQueryStmt"
	testNodeExplainStmt = "ResolvedExplainStmt"
	testNodeFoo         = "ResolvedFoo"
	testNodeGraphScan   = "ResolvedGraph*Scan"
)

func TestParseYAMLValid(t *testing.T) {
	src := `# comment line
ResolvedQueryStmt: duckdb_native
ResolvedCreateTableStmt: control_op status=planned
# blank below
` +
		"\n" +
		`ResolvedExplainStmt: unsupported
`
	rows, err := parseYAML(src)
	if err != nil {
		t.Fatalf("parseYAML: %v", err)
	}
	if got, want := len(rows), 3; got != want {
		t.Fatalf("len(rows) = %d, want %d", got, want)
	}
	if rows[0].node != testNodeQueryStmt || rows[0].disposition != routeDuckdbNative {
		t.Errorf("row[0] = %+v", rows[0])
	}
	if rows[1].node != "ResolvedCreateTableStmt" || rows[1].disposition != routeControlOp {
		t.Errorf("row[1] = %+v", rows[1])
	}
	if rows[2].node != testNodeExplainStmt || rows[2].disposition != routeUnsupported {
		t.Errorf("row[2] = %+v", rows[2])
	}
}

func TestParseYAMLRejectsUnknownDisposition(t *testing.T) {
	_, err := parseYAML(testNodeQueryStmt + ": not_a_route\n")
	if err == nil {
		t.Fatal("parseYAML accepted an unknown disposition")
	}
	if !strings.Contains(err.Error(), "unknown disposition") {
		t.Errorf("error %q does not mention `unknown disposition`", err)
	}
}

func TestParseShapeTrackerExtractsRows(t *testing.T) {
	md := `# Header text outside any table
| Node | Status | Notes |
|------|--------|-------|
| ` + "`ResolvedQueryStmt`" + ` | ` + "`duckdb_native`" + ` | landed |
| ` + "`ResolvedExplainStmt`" + ` | ` + "`unsupported`" + ` | policy |
| ` + "`ResolvedCreateTableStmt`" + ` | ` + "`control_op`" + ` | (planned) |
| ` + "`ResolvedArgumentRef`" + ` / ` + "`ResolvedArgumentDef`" + ` | ` + "`semantic_executor`" + ` | (planned) |
| ` + "`ResolvedGraph*Scan`" + ` | ` + "`unsupported`" + ` | family |
Outside the table again.
`
	rows, err := parseShapeTracker(md)
	if err != nil {
		t.Fatalf("parseShapeTracker: %v", err)
	}
	if got, want := len(rows), 5; got != want {
		t.Fatalf("len(rows) = %d, want %d (%v)", got, want, rows)
	}
	if got, want := rows[3].nodes, []string{"ResolvedArgumentRef", "ResolvedArgumentDef"}; !equalStrings(got, want) {
		t.Errorf("composite row nodes = %v, want %v", got, want)
	}
	if rows[4].nodes[0] != testNodeGraphScan {
		t.Errorf("wildcard preserved literally, got %q", rows[4].nodes[0])
	}
}

func TestExtractDispositionStripsSubsetAndPlannedSuffix(t *testing.T) {
	tests := map[string]string{
		"`duckdb_native`":            routeDuckdbNative,
		"`duckdb_native` (subset)":   routeDuckdbNative,
		"`duckdb_native` (planned)":  routeDuckdbNative,
		"`semantic_executor`":        routeSemanticExecutor,
		"  `unsupported`  ":          routeUnsupported,
		"`duckdb_native` (planned) ": routeDuckdbNative,
	}
	for input, want := range tests {
		t.Run(input, func(t *testing.T) {
			got, err := extractDisposition(input)
			if err != nil {
				t.Fatalf("extractDisposition(%q): %v", input, err)
			}
			if got != want {
				t.Errorf("got %q, want %q", got, want)
			}
		})
	}
}

func TestExtractDispositionRejectsUnknown(t *testing.T) {
	if _, err := extractDisposition("`not_a_route`"); err == nil {
		t.Fatal("extractDisposition accepted unknown route")
	}
}

func TestMatchesWildcard(t *testing.T) {
	tests := []struct {
		pattern, name string
		want          bool
	}{
		{testNodeGraphScan, "ResolvedGraphTableScan", true},
		{testNodeGraphScan, "ResolvedGraphScan", true},
		{testNodeGraphScan, "ResolvedGraphElementScan", true},
		{testNodeGraphScan, "ResolvedTableScan", false},
		{testNodeGraphScan, "ResolvedGraphTable", false},
		{testNodeFoo, testNodeFoo, true},
		{testNodeFoo, "ResolvedBar", false},
	}
	for _, tc := range tests {
		got := matchesWildcard(tc.pattern, tc.name)
		if got != tc.want {
			t.Errorf("matchesWildcard(%q, %q) = %v, want %v",
				tc.pattern, tc.name, got, tc.want)
		}
	}
}

func TestCompareParityClean(t *testing.T) {
	yaml := []yamlRow{
		{node: testNodeQueryStmt, disposition: routeDuckdbNative, lineNumber: 1},
		{node: testNodeExplainStmt, disposition: routeUnsupported, lineNumber: 2},
		{node: "ResolvedGraphTableScan", disposition: routeUnsupported, lineNumber: 3},
		{node: "ResolvedGraphScan", disposition: routeUnsupported, lineNumber: 4},
	}
	shape := []shapeRow{
		{nodes: []string{testNodeQueryStmt}, disposition: routeDuckdbNative, lineNumber: 10},
		{nodes: []string{testNodeExplainStmt}, disposition: routeUnsupported, lineNumber: 11},
		{nodes: []string{testNodeGraphScan}, disposition: routeUnsupported, lineNumber: 12},
	}
	got := compareParity(yaml, shape)
	if len(got) != 0 {
		t.Fatalf("unexpected findings: %v", got)
	}
}

func TestCompareParityDetectsDisagreement(t *testing.T) {
	// Deliberate drift: SHAPE_TRACKER.md says duckdb_native,
	// node_dispositions.yaml says semantic_executor. This is
	// exactly the regression the lint gate must catch -- per the
	// plan, a deliberately drifted row in one source causes the
	// checker to fail.
	yaml := []yamlRow{
		{node: testNodeQueryStmt, disposition: routeSemanticExecutor, lineNumber: 1},
	}
	shape := []shapeRow{
		{nodes: []string{testNodeQueryStmt}, disposition: routeDuckdbNative, lineNumber: 10},
	}
	got := compareParity(yaml, shape)
	if len(got) == 0 {
		t.Fatal("expected at least one finding for drifted row")
	}
	found := false
	for _, f := range got {
		if strings.Contains(f, "disposition mismatch") &&
			strings.Contains(f, testNodeQueryStmt) {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("findings do not mention the drifted row: %v", got)
	}
}

func TestCompareParityDetectsMissingYAMLRow(t *testing.T) {
	yaml := []yamlRow{
		{node: testNodeQueryStmt, disposition: routeDuckdbNative, lineNumber: 1},
	}
	shape := []shapeRow{
		{nodes: []string{testNodeQueryStmt}, disposition: routeDuckdbNative, lineNumber: 10},
		{nodes: []string{"ResolvedNewNode"}, disposition: routeDuckdbNative, lineNumber: 11},
	}
	got := compareParity(yaml, shape)
	if len(got) == 0 {
		t.Fatal("expected a finding for ResolvedNewNode missing from YAML")
	}
	hit := false
	for _, f := range got {
		if strings.Contains(f, "ResolvedNewNode") {
			hit = true
		}
	}
	if !hit {
		t.Fatalf("findings do not mention the missing YAML row: %v", got)
	}
}

func TestCompareParityDetectsOrphanYAMLRow(t *testing.T) {
	yaml := []yamlRow{
		{node: testNodeQueryStmt, disposition: routeDuckdbNative, lineNumber: 1},
		{node: "ResolvedOrphan", disposition: routeDuckdbNative, lineNumber: 2},
	}
	shape := []shapeRow{
		{nodes: []string{testNodeQueryStmt}, disposition: routeDuckdbNative, lineNumber: 10},
	}
	got := compareParity(yaml, shape)
	if len(got) == 0 {
		t.Fatal("expected a finding for ResolvedOrphan missing from SHAPE_TRACKER.md")
	}
	hit := false
	for _, f := range got {
		if strings.Contains(f, "ResolvedOrphan") &&
			strings.Contains(f, "no matching row") {
			hit = true
		}
	}
	if !hit {
		t.Fatalf("findings do not mention the orphan YAML row: %v", got)
	}
}

func equalStrings(a, b []string) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}
