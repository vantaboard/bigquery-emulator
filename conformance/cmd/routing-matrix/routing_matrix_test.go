package main

import (
	"bytes"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

// fixturesDir returns the absolute path to `conformance/fixtures`
// from the test binary's working directory (which `go test`
// pins to the test file's directory). Hardcoding the relative
// hop keeps the test self-contained -- it does not need any
// CI-side env wiring.
func fixturesDir(t *testing.T) string {
	t.Helper()
	_, here, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller(0) failed")
	}
	// `here` is `.../conformance/cmd/routing-matrix/routing_matrix_test.go`.
	return filepath.Join(filepath.Dir(here), "..", "..", "fixtures")
}

// TestRoutingMatrixIsReproducible pins the routing matrix's
// determinism contract: two consecutive invocations of
// `buildMatrix` MUST return byte-identical output. CI uses this
// in the upload step (a non-blocking artifact whose diff between
// two runs proves the matrix is a deterministic function of the
// fixture YAMLs, not the engine binary or wall clock).
func TestRoutingMatrixIsReproducible(t *testing.T) {
	dir := fixturesDir(t)
	a, err := buildMatrix(dir)
	if err != nil {
		t.Fatalf("buildMatrix(first run): %v", err)
	}
	b, err := buildMatrix(dir)
	if err != nil {
		t.Fatalf("buildMatrix(second run): %v", err)
	}
	if a != b {
		t.Fatalf("matrix output differs between runs (first=%d bytes, second=%d bytes); "+
			"the walker must be a deterministic function of the fixture YAMLs",
			len(a), len(b))
	}
}

// TestRoutingMatrixCoversEveryFixture pins the "no silent
// skipping" contract: every loadable fixture under the normal
// walker MUST appear in the matrix exactly once. The
// `_route_drift_example/` quarantine fixture MUST NOT appear (it
// is the runner's safety-net fixture, not a real conformance row;
// see `conformance/runner/route_drift_test.go`).
func TestRoutingMatrixCoversEveryFixture(t *testing.T) {
	dir := fixturesDir(t)
	out, err := buildMatrix(dir)
	if err != nil {
		t.Fatalf("buildMatrix: %v", err)
	}
	if !strings.Contains(out, "## Per-fixture") {
		t.Fatal("matrix output missing per-fixture section")
	}
	if strings.Contains(out, "_route_drift_example") {
		t.Error("matrix included the quarantined drift fixture; " +
			"LoadDir should skip underscore-prefixed directories")
	}
	// Cheap sanity check: a representative fixture from each
	// major family appears.
	for _, want := range []string{
		"`scalar/select_one`",
		"`dml/insert_values_round_trip`",
		"`ddl_create_table_round_trip`",
		"`fastpath/scan_table_basic`",
		"`specialized/keys_new_keyset_stub`",
		"`advanced_relational/pivot`",
	} {
		if !strings.Contains(out, want) {
			t.Errorf("matrix missing expected row containing %s", want)
		}
	}
}

// TestRoutingMatrixSummaryListsCanonicalRoutes makes sure the
// per-route tally block uses the canonical
// `KnownRouteNames()` vocabulary so a reviewer can compare
// relative weights without re-sorting in their head. Empty
// route buckets are not listed; non-canonical buckets land
// AFTER the canonical block.
func TestRoutingMatrixSummaryListsCanonicalRoutes(t *testing.T) {
	dir := fixturesDir(t)
	out, err := buildMatrix(dir)
	if err != nil {
		t.Fatalf("buildMatrix: %v", err)
	}
	// At least these routes must be represented in the current
	// fixture set; if a future plan removes the last fixture of
	// any of them, refresh the assertion.
	for _, want := range []string{
		"| `duckdb_native` |",
		"| `duckdb_udf` |",
		"| `semantic_executor` |",
		"| `local_stub` |",
		"| `unsupported` |",
		"| `duckdb_rewrite` |",
	} {
		if !strings.Contains(out, want) {
			t.Errorf("summary missing %q; current fixture set should include it", want)
		}
	}
}

// TestCLIWritesOutputFile pins that the `--output-file` flag
// produces a file whose contents match stdout byte-for-byte.
// CI uses the flag to upload the matrix as a non-blocking
// artifact while still streaming the same bytes to the job log.
func TestCLIWritesOutputFile(t *testing.T) {
	dir := fixturesDir(t)
	out := filepath.Join(t.TempDir(), "matrix.md")
	var stdout, stderr bytes.Buffer
	code, err := run(
		[]string{"--fixtures", dir, "--output-file", out},
		&stdout,
		&stderr,
	)
	if err != nil {
		t.Fatalf("run: %v (stderr: %s)", err, stderr.String())
	}
	if code != 0 {
		t.Fatalf("exit code=%d, want 0 (stderr: %s)", code, stderr.String())
	}
	gotBytes, err := os.ReadFile(out)
	if err != nil {
		t.Fatalf("read %s: %v", out, err)
	}
	if string(gotBytes) != stdout.String() {
		t.Fatalf("output-file (%d bytes) differs from stdout (%d bytes)",
			len(gotBytes), stdout.Len())
	}
}
