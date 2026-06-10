// Unit tests for the gateway_main version-print helper. We do not
// fork the binary (the cost of a `go build` per test run is not worth
// it; the helper is a pure function over package-level vars and is
// trivially testable in-process). The integration check that the
// linker-injected values actually surface is performed by the
// `task gateway:build` + `./bin/gateway_main --version` smoke.

package main

import (
	"bytes"
	"runtime"
	"strings"
	"testing"
)

func TestPrintVersion_DefaultsHonored(t *testing.T) {
	t.Helper()

	// Snapshot the package-level vars so we can mutate them safely
	// inside the test without leaking state to other tests in the
	// package. `version`, `commit`, `date` are intentionally `var`
	// (not `const`) so the goreleaser ldflag injection works; the
	// same mutability is what lets us drive them from the test.
	origVersion, origCommit, origDate := version, commit, date
	t.Cleanup(func() {
		version, commit, date = origVersion, origCommit, origDate
	})

	version = "1.2.3"
	commit = "abcdef0"
	date = "2026-05-25T22:18:00Z"

	var buf bytes.Buffer
	printVersion(&buf)
	out := buf.String()

	for _, want := range []string{
		"bigquery-emulator-gateway version 1.2.3",
		"commit:  abcdef0",
		"built:   2026-05-25T22:18:00Z",
		// Runtime introspection is part of the contract -- a future
		// edit that drops `runtime.Version()` would break operators
		// who scrape this output to diagnose toolchain drift.
		"go:      " + runtime.Version(),
		"os/arch: " + runtime.GOOS + "/" + runtime.GOARCH,
	} {
		if !strings.Contains(out, want) {
			t.Errorf("printVersion output missing %q\nfull output:\n%s",
				want, out)
		}
	}
}

func TestPrintVersion_UnsetDefaults(t *testing.T) {
	// With no ldflag injection (plain `go build`), the package
	// defaults must surface verbatim. Operators reading
	// `--version` on a `go run`'d build need to be able to tell that
	// the binary is unstamped (i.e. not a release artifact) at a
	// glance.
	origVersion, origCommit, origDate := version, commit, date
	t.Cleanup(func() {
		version, commit, date = origVersion, origCommit, origDate
	})
	version, commit, date = "dev", "none", "unknown"

	var buf bytes.Buffer
	printVersion(&buf)
	out := buf.String()

	for _, want := range []string{
		"bigquery-emulator-gateway version dev",
		"commit:  none",
		"built:   unknown",
	} {
		if !strings.Contains(out, want) {
			t.Errorf("printVersion default output missing %q\nfull output:\n%s",
				want, out)
		}
	}
}
