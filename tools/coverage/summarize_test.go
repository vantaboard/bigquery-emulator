package main

import (
	"bytes"
	"encoding/json"
	"errors"
	"path/filepath"
	"strings"
	"testing"
)

// goFixtureFlag is the shared --go invocation pointed at the
// checked-in testdata Go cover profile. Centralised so the goconst
// linter doesn't flag it across the summarize_test/badge_test pair
// and so a fixture rename only has to touch one place.
const goFixtureFlag = "--go=testdata/coverage.out"

// TestRunSummarize_BothFlags exercises the canonical CI happy path:
// both --go and --lcov are supplied, the union percentage is computed
// across both suites, and per-flag percentages land in the output
// JSON. Numbers come from the checked-in testdata fixtures
// (Go: 7/12 = 58.3, CPP: 6/10 = 60.0, combined: 13/22 = 59.1).
func TestRunSummarize_BothFlags(t *testing.T) {
	outPath := filepath.Join(t.TempDir(), "summary.json")
	var stdout, stderr bytes.Buffer
	err := run([]string{
		cmdSummarize,
		goFixtureFlag,
		"--lcov=testdata/coverage.dat",
		"--out=" + outPath,
		"--commit=deadbeef",
	}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("run summarize: %v\nstderr=%s", err, stderr.String())
	}

	var s Summary
	if data := mustRead(t, outPath); true {
		if err := json.Unmarshal(data, &s); err != nil {
			t.Fatalf("decode summary: %v\nraw=%s", err, data)
		}
	}
	if s.Commit != "deadbeef" {
		t.Errorf("Commit=%q, want deadbeef", s.Commit)
	}
	wantPct(t, "Go", s.Go, 58.3)
	wantPct(t, "CPP", s.CPP, 60.0)
	wantPct(t, "Total", s.Total, 59.1)
	if s.Timestamp == "" {
		t.Error("Timestamp should be populated")
	}
}

// TestRunSummarize_GoOnly documents the half-suite path: when only
// --go is supplied, the CPP field is the missingFlag sentinel and the
// total matches the Go percentage exactly.
func TestRunSummarize_GoOnly(t *testing.T) {
	outPath := filepath.Join(t.TempDir(), "summary.json")
	var stdout, stderr bytes.Buffer
	err := run([]string{
		cmdSummarize,
		goFixtureFlag,
		"--out=" + outPath,
	}, &stdout, &stderr)
	if err != nil {
		t.Fatalf("run summarize: %v\nstderr=%s", err, stderr.String())
	}
	s := decodeSummary(t, outPath)
	wantPct(t, "Go", s.Go, 58.3)
	if s.CPP != missingFlag {
		t.Errorf("CPP = %v, want missingFlag (%v)", s.CPP, missingFlag)
	}
	wantPct(t, "Total", s.Total, 58.3)
}

// TestRunSummarize_NoInputs documents that running summarize with
// neither --go nor --lcov fails with the usage error (exit 2 at the
// process boundary), so a misconfigured workflow surfaces loudly
// instead of writing an empty summary.json that would trip the gate.
func TestRunSummarize_NoInputs(t *testing.T) {
	var stdout, stderr bytes.Buffer
	err := run([]string{cmdSummarize}, &stdout, &stderr)
	if !errors.Is(err, errUsage) {
		t.Fatalf("err = %v, want errUsage", err)
	}
	if !strings.Contains(stderr.String(), "at least one of") {
		t.Errorf("stderr %q should hint at required flags", stderr.String())
	}
}

// TestRunSummarize_BadGoPath documents that a missing --go file
// surfaces a wrapped error, not a panic.
func TestRunSummarize_BadGoPath(t *testing.T) {
	var stdout, stderr bytes.Buffer
	err := run([]string{cmdSummarize, "--go=does/not/exist.out"}, &stdout, &stderr)
	if err == nil {
		t.Fatal("err = nil, want error")
	}
	if !strings.Contains(err.Error(), "parse go profile") {
		t.Errorf("err %q should wrap with 'parse go profile'", err)
	}
}
