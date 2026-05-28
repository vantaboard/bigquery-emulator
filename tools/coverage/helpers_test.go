package main

import (
	"encoding/json"
	"math"
	"os"
	"testing"
)

// mustRead reads a file or fails the test. Pulled into a shared
// helper so each subcommand test reads its temp-dir output the same
// way (and so a future change to file ownership/mode only has to
// update one place).
func mustRead(t *testing.T, path string) []byte {
	t.Helper()
	b, err := os.ReadFile(path) //nolint:gosec // test fixture path comes from t.TempDir()
	if err != nil {
		t.Fatalf("read %q: %v", path, err)
	}
	return b
}

// decodeSummary reads and decodes a summary.json file or fails the
// test. The marshalling layer is tested separately; tests downstream
// of summarize just want the Summary struct.
func decodeSummary(t *testing.T, path string) *Summary {
	t.Helper()
	var s Summary
	if err := json.Unmarshal(mustRead(t, path), &s); err != nil {
		t.Fatalf("decode summary %q: %v", path, err)
	}
	return &s
}

// wantPct compares percentage floats with a small epsilon. JSON
// round-trips through string formatting, and Summary's Total/Go/CPP
// fields are already rounded to one decimal, so direct equality
// almost works — but `math.Abs(a-b) < 0.05` keeps the matcher safe
// from any IEEE noise that survives the marshal/unmarshal trip.
func wantPct(t *testing.T, name string, got, want float64) {
	t.Helper()
	if math.Abs(got-want) > 0.05 {
		t.Errorf("%s = %.3f, want %.1f", name, got, want)
	}
}
