//go:build integration

// Package main's integration test exercises the conformance runner
// end-to-end against the real `emulator_main` subprocess and the
// seed fixtures shipped under `conformance/fixtures/`. The test is
// gated by the `integration` build tag, matching the pattern in
// `gateway/e2e/` so a default `go test ./...` stays hermetic on
// machines that have not built the Bazel engine binary.
//
// Run with:
//
//	task emulator:build-engine-bazel
//	go test -tags=integration ./conformance/...
package main

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
)

// emulatorBinaryPath mirrors the locator the gateway/e2e harness
// uses: BIGQUERY_EMULATOR_BIN env override, then ./bin/emulator_main
// (staged by `task emulator:build-engine-bazel`). Returns "" when
// nothing is found, so the test can Skip rather than fail on a fresh
// checkout.
func emulatorBinaryPath(t *testing.T) string {
	t.Helper()
	if p := os.Getenv("BIGQUERY_EMULATOR_BIN"); p != "" {
		if _, err := os.Stat(p); err == nil {
			return p
		}
	}
	root, err := repoRoot()
	if err != nil {
		return ""
	}
	candidate := filepath.Join(root, "bin", "emulator_main")
	if _, err := os.Stat(candidate); err == nil {
		return candidate
	}
	return ""
}

// repoRoot walks upward from the test's CWD until it finds the
// go.mod marker.
func repoRoot() (string, error) {
	wd, err := os.Getwd()
	if err != nil {
		return "", err
	}
	dir := wd
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", errors.New("no go.mod found upstream of " + wd)
		}
		dir = parent
	}
}

// TestRunFixturesAgainstEmulatorMain executes every seed fixture
// against the real `emulator_main` on both profiles and asserts the
// matrix PASSes. This is the end-to-end smoke check: the runner
// boots the engine, runs setup, queries the gateway, and the diff
// engine reports PASS. A failure here means either the fixture is
// wrong or the engine regressed for that profile.
func TestRunFixturesAgainstEmulatorMain(t *testing.T) {
	bin := emulatorBinaryPath(t)
	if bin == "" {
		t.Skip("emulator_main not found; build with `task emulator:build-engine:bazel`")
	}

	root, err := repoRoot()
	if err != nil {
		t.Fatalf("repoRoot: %v", err)
	}
	fixtures := filepath.Join(root, "conformance", "fixtures")

	// 90s budget covers `emulator_main` cold-start (~1-2s per spawn)
	// for the 4 fixtures x 2 profiles = 8 invocations on a developer
	// laptop, with headroom for slower CI. If the suite outgrows this
	// budget as the fixture corpus grows, bump explicitly here
	// rather than papering over a hang.
	ctx, cancel := context.WithTimeout(context.Background(), 90*time.Second)
	defer cancel()

	var out, errBuf bytes.Buffer
	report, err := runner.Run(ctx, runner.Options{
		FixturesPath: fixtures,
		Harness: runner.HarnessOptions{
			EngineBinary: bin,
			EngineStderr: &errBuf,
		},
		Output: "json",
		Out:    &out,
		Err:    &errBuf,
	})
	if err != nil {
		t.Fatalf("runner.Run: %v\nstderr:\n%s", err, errBuf.String())
	}
	if report.Summary.Total == 0 {
		t.Fatalf("no fixtures executed (loaded path: %s)", fixtures)
	}
	if report.Summary.Failed > 0 {
		// Surface the per-result detail so a divergence is
		// debuggable from the test log alone.
		for _, r := range report.Results {
			if r.Status != runner.StatusFail {
				continue
			}
			t.Errorf("FAIL %s profile=%s: %s\n%s",
				r.Fixture, r.Profile, r.Message, r.Diff)
		}
		t.Fatalf("conformance matrix had %d failures (passed=%d, total=%d)",
			report.Summary.Failed, report.Summary.Passed, report.Summary.Total)
	}
	if report.SchemaVersion != runner.JSONSchemaVersion {
		t.Errorf("schema_version=%d, want %d",
			report.SchemaVersion, runner.JSONSchemaVersion)
	}

	// The JSON output is the artifact the diff CI ingests.
	// Decode it back to make sure the shape on the wire stays in
	// step with the in-memory Report.
	var decoded runner.Report
	if err := json.Unmarshal(out.Bytes(), &decoded); err != nil {
		t.Fatalf("decode JSON report: %v\nstdout:\n%s",
			err, out.String())
	}
	if decoded.Summary.Total != report.Summary.Total {
		t.Errorf("JSON summary.total=%d, want %d",
			decoded.Summary.Total, report.Summary.Total)
	}
	if decoded.Summary.Passed != report.Summary.Passed {
		t.Errorf("JSON summary.passed=%d, want %d",
			decoded.Summary.Passed, report.Summary.Passed)
	}
}
