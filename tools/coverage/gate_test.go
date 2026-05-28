package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// writeJSON serialises a Summary to disk so each gate test can stage
// its own current/baseline files in a temp directory without manual
// JSON literal management.
func writeJSON(t *testing.T, dir, name string, s Summary) string {
	t.Helper()
	path := filepath.Join(dir, name)
	buf, err := json.Marshal(&s)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}
	if err := os.WriteFile(path, buf, 0o600); err != nil {
		t.Fatalf("write %s: %v", path, err)
	}
	return path
}

// TestRunGate_Pass documents the success case: current matches
// baseline exactly, so every field reports "ok" and the binary exits
// 0.
func TestRunGate_Pass(t *testing.T) {
	dir := t.TempDir()
	cur := writeJSON(t, dir, "current.json", Summary{Total: 75, Go: 80, CPP: 70})
	base := writeJSON(t, dir, "baseline.json", Summary{Total: 75, Go: 80, CPP: 70})

	var stdout, stderr bytes.Buffer
	err := run([]string{cmdGate, "--current=" + cur, "--baseline=" + base}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("err = %v, want nil\nstderr=%s", err, stderr.String())
	}
	if !strings.Contains(stdout.String(), "| ok |") {
		t.Errorf("stdout should contain 'ok' rows; got:\n%s", stdout.String())
	}
}

// TestRunGate_RegressionTotal pins the canonical failure case: total
// dropped 2pp on a 1pp tolerance, the binary returns errRegression
// and the markdown row carries the FAIL marker so the workflow's
// step-summary is human-readable.
func TestRunGate_RegressionTotal(t *testing.T) {
	dir := t.TempDir()
	cur := writeJSON(t, dir, "current.json", Summary{Total: 73, Go: 80, CPP: 70})
	base := writeJSON(t, dir, "baseline.json", Summary{Total: 75, Go: 80, CPP: 70})

	var stdout, stderr bytes.Buffer
	err := run([]string{cmdGate, "--current=" + cur, "--baseline=" + base}, &stdout, &stderr)
	if !errors.Is(err, errRegression) {
		t.Fatalf("err = %v, want errRegression", err)
	}
	if !strings.Contains(stdout.String(), "FAIL") {
		t.Errorf("stdout should mention FAIL; got:\n%s", stdout.String())
	}
}

// TestRunGate_RegressionTolerated documents that a regression within
// --tolerance is still a pass: a half-point dip is normal noise on
// these suites and should not block CI.
func TestRunGate_RegressionTolerated(t *testing.T) {
	dir := t.TempDir()
	cur := writeJSON(t, dir, "current.json", Summary{Total: 74.5, Go: 80, CPP: 70})
	base := writeJSON(t, dir, "baseline.json", Summary{Total: 75, Go: 80, CPP: 70})

	var stdout, stderr bytes.Buffer
	err := run([]string{
		cmdGate,
		"--current=" + cur,
		"--baseline=" + base,
		"--tolerance=1.0",
	}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("err = %v, want nil", err)
	}
}

// TestRunGate_Floor exercises the absolute-floor branch independently
// of tolerance: when --floor is set above the current total, the gate
// fails even if the regression itself is within tolerance.
func TestRunGate_Floor(t *testing.T) {
	dir := t.TempDir()
	cur := writeJSON(t, dir, "current.json", Summary{Total: 30, Go: 30, CPP: 30})
	base := writeJSON(t, dir, "baseline.json", Summary{Total: 30, Go: 30, CPP: 30})

	var stdout, stderr bytes.Buffer
	err := run([]string{
		cmdGate,
		"--current=" + cur,
		"--baseline=" + base,
		"--floor=50",
	}, &stdout, &stderr)
	if !errors.Is(err, errRegression) {
		t.Fatalf("err = %v, want errRegression", err)
	}
	if !strings.Contains(stdout.String(), "below floor 50") {
		t.Errorf("stdout should mention floor failure; got:\n%s", stdout.String())
	}
}

// TestRunGate_BaselineMissing pins the bootstrap behaviour: when
// gh-pages has not been published yet, the gate prints a warning,
// emits a table with "skipped (missing data)" rows, and exits 0 so
// the first PR after the rollout can land.
func TestRunGate_BaselineMissing(t *testing.T) {
	dir := t.TempDir()
	cur := writeJSON(t, dir, "current.json", Summary{Total: 75, Go: 80, CPP: 70})
	missing := filepath.Join(dir, "no-such-file.json")

	var stdout, stderr bytes.Buffer
	err := run([]string{cmdGate, "--current=" + cur, "--baseline=" + missing}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("err = %v, want nil (bootstrap)", err)
	}
	if !strings.Contains(stderr.String(), "treating as bootstrap") {
		t.Errorf("stderr should mention bootstrap; got:\n%s", stderr.String())
	}
	if !strings.Contains(stdout.String(), "Baseline missing") {
		t.Errorf("stdout should mention missing baseline; got:\n%s", stdout.String())
	}
}

// TestRunGate_MissingFlagSkipped pins the half-suite case: when the
// current run only has a Go percentage (CPP came back as
// missingFlag), the CPP row must be skipped rather than treated as a
// catastrophic regression to 0%.
func TestRunGate_MissingFlagSkipped(t *testing.T) {
	dir := t.TempDir()
	cur := writeJSON(t, dir, "current.json", Summary{Total: 80, Go: 80, CPP: missingFlag})
	base := writeJSON(t, dir, "baseline.json", Summary{Total: 75, Go: 70, CPP: 60})

	var stdout, stderr bytes.Buffer
	err := run([]string{cmdGate, "--current=" + cur, "--baseline=" + base}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("err = %v, want nil", err)
	}
	if !strings.Contains(stdout.String(), "skipped (missing data)") {
		t.Errorf("stdout should mention skipped row; got:\n%s", stdout.String())
	}
}
