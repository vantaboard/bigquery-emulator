package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// TestRunAgainstRepoFiles is the canonical lint-gate test: pointed
// at the actual worktree, the linter must report no findings (every
// hit is either rewritten in plan 17's sweep or covered by the
// allowlist). A regression that introduces a stale term outside the
// allowlist fails this test (and CI's `task lint:stale-terms`).
func TestRunAgainstRepoFiles(t *testing.T) {
	root, err := repoRoot()
	if err != nil {
		t.Fatalf("repoRoot: %v", err)
	}
	var stdout, stderr bytes.Buffer
	err = run([]string{"--root=" + root}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("run returned %v\nstdout: %s\nstderr: %s",
			err, stdout.String(), stderr.String())
	}
	if !strings.Contains(stdout.String(), "check-stale-terms: ok") {
		t.Errorf("stdout missing OK marker: %q", stdout.String())
	}
}

// TestRunDetectsStaleTerm stages a temp repo with a known-bad term
// in a file that's NOT on the allowlist, and asserts the linter
// surfaces the offending hit and exits non-zero.
func TestRunDetectsStaleTerm(t *testing.T) {
	dir := t.TempDir()
	// Path under the simulated repo root with a plain `DuckDB-only`
	// hit. The file lives at the worktree root so the allowlist
	// path-form matches simply.
	bad := filepath.Join(dir, "STALE.md")
	if err := os.WriteFile(bad,
		[]byte("# stale\nThis emulator is DuckDB-only.\n"),
		0o600); err != nil {
		t.Fatalf("write %s: %v", bad, err)
	}
	allow := filepath.Join(dir, "allow.txt")
	if err := os.WriteFile(allow, []byte("# empty allowlist\n"),
		0o600); err != nil {
		t.Fatalf("write allowlist: %v", err)
	}
	var stdout, stderr bytes.Buffer
	err := run([]string{"--root=" + dir, "--allowlist=" + allow},
		&stdout, &stderr)
	if err == nil {
		t.Fatalf("expected non-nil error, got nil. stderr=%q",
			stderr.String())
	}
	if !strings.Contains(stderr.String(), "DuckDB-only") {
		t.Errorf("stderr missing the offending term: %q", stderr.String())
	}
	if !strings.Contains(stderr.String(), "STALE.md:2:") {
		t.Errorf("stderr missing the file:line position: %q",
			stderr.String())
	}
}

// TestRunHonorsRangeAllowlist stages a temp repo where a stale term
// lives inside a `History` section the allowlist explicitly covers
// by line range. The linter must NOT surface that hit.
func TestRunHonorsRangeAllowlist(t *testing.T) {
	dir := t.TempDir()
	// Two `DuckDB-only` hits: line 2 (outside the allowlist range)
	// and line 5 (inside [4-6]).
	doc := filepath.Join(dir, "DOC.md")
	if err := os.WriteFile(doc, []byte(""+
		"# active\n"+
		"This still says DuckDB-only and should fail.\n"+
		"\n"+
		"## History\n"+
		"The DuckDB-only era pre-dated the multi-strategy coordinator.\n"+
		"It is fine to keep this paragraph as historical context.\n"),
		0o600); err != nil {
		t.Fatalf("write %s: %v", doc, err)
	}
	allow := filepath.Join(dir, "allow.txt")
	if err := os.WriteFile(allow, []byte("DOC.md:4-6\n"), 0o600); err != nil {
		t.Fatalf("write allowlist: %v", err)
	}
	var stdout, stderr bytes.Buffer
	err := run([]string{"--root=" + dir, "--allowlist=" + allow},
		&stdout, &stderr)
	if err == nil {
		t.Fatalf("expected non-nil error, got nil. stderr=%q",
			stderr.String())
	}
	// The line-2 hit must surface (outside the allowlist range).
	if !strings.Contains(stderr.String(), "DOC.md:2:") {
		t.Errorf("stderr missing line-2 hit: %q", stderr.String())
	}
	// The line-5 hit must NOT surface (inside [4-6]).
	if strings.Contains(stderr.String(), "DOC.md:5:") {
		t.Errorf("stderr should not flag the line-5 hit: %q",
			stderr.String())
	}
}

// TestRunHonorsWholeFileAllowlist stages a temp repo where a stale
// term appears in a file that is whole-file allowlisted. The linter
// must NOT surface any hit from that file.
func TestRunHonorsWholeFileAllowlist(t *testing.T) {
	dir := t.TempDir()
	doc := filepath.Join(dir, "HISTORY.md")
	if err := os.WriteFile(doc,
		[]byte("This file documents the FallbackEngine era.\n"),
		0o600); err != nil {
		t.Fatalf("write %s: %v", doc, err)
	}
	allow := filepath.Join(dir, "allow.txt")
	if err := os.WriteFile(allow, []byte("HISTORY.md\n"), 0o600); err != nil {
		t.Fatalf("write allowlist: %v", err)
	}
	var stdout, stderr bytes.Buffer
	err := run([]string{"--root=" + dir, "--allowlist=" + allow},
		&stdout, &stderr)
	if err != nil {
		t.Fatalf("run returned %v\nstderr: %s", err, stderr.String())
	}
	if !strings.Contains(stdout.String(), "check-stale-terms: ok") {
		t.Errorf("stdout missing OK marker: %q", stdout.String())
	}
}

// repoRoot walks upward from the test binary's CWD until it finds a
// `go.mod` file (the repo root). Used by TestRunAgainstRepoFiles to
// resolve the canonical worktree.
func repoRoot() (string, error) {
	dir, err := os.Getwd()
	if err != nil {
		return "", err
	}
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", os.ErrNotExist
		}
		dir = parent
	}
}
