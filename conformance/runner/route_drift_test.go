package runner

import (
	"strings"
	"testing"
)

// TestLoadDirSkipsUnderscoreDirectory pins the leading-underscore
// quarantine convention `conformance/fixtures/_route_drift_example/`
// uses: `LoadDir` walks the tree but MUST NOT load fixtures
// under any directory whose basename starts with `_`. The
// convention mirrors Bazel's `_*_test.cc` quarantine pattern.
func TestLoadDirSkipsUnderscoreDirectory(t *testing.T) {
	fixtures, err := LoadDir("../fixtures")
	if err != nil {
		t.Fatalf("LoadDir(../fixtures): %v", err)
	}
	for _, f := range fixtures {
		if strings.Contains(f.Path, "/_route_drift_example/") ||
			strings.Contains(f.Path, "/_") {
			t.Errorf("LoadDir loaded quarantined fixture %s; "+
				"underscore-prefixed directories must be skipped", f.Path)
		}
	}
	if len(fixtures) == 0 {
		t.Fatal("LoadDir returned no fixtures; the walker is broken")
	}
}

// TestDriftFixtureFiresRouteMismatch is the safety-net plan 16
// requires: load the drift fixture directly (bypassing the
// walker's quarantine skip), run `routeDiff` with the ACTUAL
// route the coordinator would emit, and confirm the runner's
// mismatch diagnostic fires and names both routes.
//
// If you find yourself "fixing" this test, you broke the
// safety net. The drift fixture pins `route: duckdb_native` for
// a `SELECT 1` query the classifier ALWAYS routes to
// `semantic_executor`. The runner's job is to spot that
// disagreement; the test's job is to spot the runner regressing.
func TestDriftFixtureFiresRouteMismatch(t *testing.T) {
	const path = "../fixtures/_route_drift_example/select_one_claims_duckdb_native.yaml"
	f, err := Load(path)
	if err != nil {
		t.Fatalf("Load(%s): %v", path, err)
	}
	if f.Expected.Route != RouteDuckDBNative {
		t.Fatalf("drift fixture's expected.route=%q, want %q "+
			"(do NOT 'fix' the fixture; the mismatch is intentional)",
			f.Expected.Route, RouteDuckDBNative)
	}

	// The ACTUAL route the classifier would emit for the
	// fixture's query (`SELECT 1 AS n`). Hardcoded rather than
	// derived from a live engine because this test must work
	// against the in-process runner package alone; the C++
	// classifier's behavior for this shape is already pinned by
	// `route_classifier_test.cc::ScalarOnlySelectPromotesToSemanticExecutor`.
	actualRoute := RouteSemanticExecutor

	diff := routeDiff(f.Expected, actualRoute)
	if diff == "" {
		t.Fatal("routeDiff returned empty; the safety net is broken " +
			"(the runner must fail on the drift fixture)")
	}
	if !strings.Contains(diff, RouteDuckDBNative) {
		t.Errorf("diagnostic must name the expected route %q: %s",
			RouteDuckDBNative, diff)
	}
	if !strings.Contains(diff, RouteSemanticExecutor) {
		t.Errorf("diagnostic must name the actual route %q: %s",
			RouteSemanticExecutor, diff)
	}
}
