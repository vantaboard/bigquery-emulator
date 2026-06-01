package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// TestRunAgainstRepoFiles is the canonical lint-gate test: pointed
// at the repo's real `node_dispositions.yaml` and `SHAPE_TRACKER.md`,
// the checker must report no findings. A drift between the two
// sources fails this test (and CI's `task lint:dispositions` step).
func TestRunAgainstRepoFiles(t *testing.T) {
	root, err := repoRoot()
	if err != nil {
		t.Fatalf("repoRoot: %v", err)
	}
	yp := filepath.Join(root, defaultYAMLRel)
	sp := filepath.Join(root, defaultShapeTrackerRel)
	var stdout, stderr bytes.Buffer
	err = run([]string{"--yaml=" + yp, "--shape-tracker=" + sp}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("run returned %v\nstdout: %s\nstderr: %s",
			err, stdout.String(), stderr.String())
	}
	if !strings.Contains(stdout.String(), "check-disposition-parity: ok") {
		t.Errorf("stdout missing OK marker: %q", stdout.String())
	}
}

// TestRunDetectsDriftedFixture stages a temp pair of files where
// the YAML disposition for one shared node deliberately disagrees
// with SHAPE_TRACKER's. The checker must surface the disagreement
// and exit non-zero (errFindings). This is the regression-canary
// test the plan calls out: "a deliberately drifted row in one
// source causes the checker to fail".
func TestRunDetectsDriftedFixture(t *testing.T) {
	dir := t.TempDir()
	yp := filepath.Join(dir, "node_dispositions.yaml")
	sp := filepath.Join(dir, "SHAPE_TRACKER.md")
	if err := os.WriteFile(yp, []byte(`# fixture
ResolvedQueryStmt: semantic_executor
ResolvedExplainStmt: unsupported plan=specialized-feature-policy.plan.md
`), 0o600); err != nil {
		t.Fatalf("write yaml: %v", err)
	}
	if err := os.WriteFile(sp, []byte("| Node | Status | Notes |\n"+
		"|------|--------|-------|\n"+
		"| `ResolvedQueryStmt` | `duckdb_native` | drifted-on-purpose |\n"+
		"| `ResolvedExplainStmt` | `unsupported` | policy |\n"), 0o600); err != nil {
		t.Fatalf("write shape: %v", err)
	}
	var stdout, stderr bytes.Buffer
	err := run([]string{"--yaml=" + yp, "--shape-tracker=" + sp}, &stdout, &stderr)
	if err == nil {
		t.Fatalf("expected non-nil error, got nil. stderr=%q", stderr.String())
	}
	if !strings.Contains(stderr.String(), "disposition mismatch") {
		t.Errorf("stderr missing the drift report: %q", stderr.String())
	}
	if !strings.Contains(stderr.String(), "ResolvedQueryStmt") {
		t.Errorf("stderr does not name the drifted row: %q", stderr.String())
	}
}

// repoRoot walks up from this file's directory until it finds a
// `go.mod` -- that is the repo root by convention. Used so the
// real-file test runs regardless of how Go invoked the test (from
// the repo root or from the package directory).
func repoRoot() (string, error) {
	dir, err := os.Getwd()
	if err != nil {
		return "", err
	}
	for {
		if _, statErr := os.Stat(filepath.Join(dir, "go.mod")); statErr == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", os.ErrNotExist
		}
		dir = parent
	}
}
