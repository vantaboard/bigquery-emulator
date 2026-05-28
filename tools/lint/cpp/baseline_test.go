package main

import (
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"testing"
)

// TestReadBaseline_ParsesEntries pins the baseline file format:
// blank lines and `#`-prefixed lines are skipped, paths are
// trimmed, backslashes are normalised. The same parser feeds both
// the `check` runner and the `baseline` regenerator so a drift
// would silently widen the grandfather list.
func TestReadBaseline_ParsesEntries(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "baseline.txt")
	const body = `# header comment
backend/foo.cc

  backend/bar.cc   # inline comment
backend\baz.cc
`
	if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
		t.Fatalf("write fixture: %v", err)
	}
	got, err := readBaseline(path)
	if err != nil {
		t.Fatalf("readBaseline: %v", err)
	}
	want := map[string]struct{}{
		"backend/foo.cc": {},
		"backend/bar.cc": {},
		"backend/baz.cc": {},
	}
	if !reflect.DeepEqual(got, want) {
		t.Errorf("readBaseline mismatch:\n got: %v\nwant: %v", got, want)
	}
}

// TestReadBaseline_MissingFile is the default-state contract:
// readBaseline must succeed with an empty set when the file does
// not exist, otherwise the very first commit landing the lint
// gate would have to ship a baseline file along with it.
func TestReadBaseline_MissingFile(t *testing.T) {
	got, err := readBaseline(filepath.Join(t.TempDir(), "absent.txt"))
	if err != nil {
		t.Fatalf("readBaseline missing: %v", err)
	}
	if len(got) != 0 {
		t.Errorf("expected empty baseline, got %v", got)
	}
}

// TestWriteBaseline_RoundTrip confirms the written file is
// readable by readBaseline and that the rendered header gives
// reviewers the context they need.
func TestWriteBaseline_RoundTrip(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "baseline.txt")
	entries := []string{"backend/a.cc", "backend/b.cc"}
	if err := writeBaseline(path, entries); err != nil {
		t.Fatalf("writeBaseline: %v", err)
	}
	contents, err := os.ReadFile(path) //nolint:gosec // test fixture
	if err != nil {
		t.Fatalf("read written baseline: %v", err)
	}
	if !strings.Contains(string(contents), "tools/lint/cpp/baseline.txt") {
		t.Errorf("missing header in written baseline:\n%s", string(contents))
	}
	got, err := readBaseline(path)
	if err != nil {
		t.Fatalf("readBaseline round-trip: %v", err)
	}
	if _, ok := got["backend/a.cc"]; !ok {
		t.Errorf("round-trip lost backend/a.cc")
	}
	if _, ok := got["backend/b.cc"]; !ok {
		t.Errorf("round-trip lost backend/b.cc")
	}
}
