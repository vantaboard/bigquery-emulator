// Command coverage is the BigQuery emulator's self-hosted Codecov
// replacement. It ingests the two coverage artifacts produced by CI
// (Go `coverage.out` from `go test -coverprofile=...`, and the
// aggregated LCOV `.dat` from `bazel coverage --combined_report=lcov`),
// then emits the three pieces the gh-pages pipeline needs:
//
//  1. `summarize` writes a JSON summary with the overall percentage
//     plus per-flag (go, cpp) percentages.
//  2. `badge`     writes a shields.io endpoint JSON for one field of
//     that summary so the README badges can be rendered
//     dynamically from gh-pages without any external SaaS.
//  3. `gate`      compares the current summary against the baseline
//     published by the last `main` build and exits
//     non-zero if any tracked percentage regressed beyond
//     the configured tolerance or fell below an absolute
//     floor.
//
// The binary has no external dependencies on purpose: it ships as part
// of the repo, runs in any environment that has a Go toolchain, and is
// trivial for contributors to reproduce locally via `task coverage:*`.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
)

// Subcommand names. Declared as constants so the dispatch switch in
// run() and the table-driven tests stay aligned (and so the goconst
// linter does not complain about the strings repeating across files).
const (
	cmdSummarize = "summarize"
	cmdBadge     = "badge"
	cmdGate      = "gate"
)

// missingMessage is the literal rendered for missing-data percentages
// across the badge and gate subcommands. Centralised because the
// shields.io endpoint and the markdown step-summary share the same
// "absent value" convention.
const missingMessage = "n/a"

// run is the testable entry point. It dispatches on the subcommand
// (first positional argument) and returns an error instead of calling
// os.Exit so tests can exercise the full code path with table-driven
// fixtures without managing process lifetimes.
func run(args []string, stdout, stderr io.Writer) error {
	if len(args) < 1 {
		usage(stderr)
		return errUsage
	}
	cmd, rest := args[0], args[1:]
	switch cmd {
	case cmdSummarize:
		return runSummarize(rest, stdout, stderr)
	case cmdBadge:
		return runBadge(rest, stdout, stderr)
	case cmdGate:
		return runGate(rest, stdout, stderr)
	case "-h", "--help", "help":
		usage(stdout)
		return nil
	default:
		_, _ = fmt.Fprintf(stderr, "coverage: unknown subcommand %q\n\n", cmd)
		usage(stderr)
		return errUsage
	}
}

// errUsage signals that the caller passed an unrecognized or malformed
// invocation. main() translates it to exit code 2 (matching the Go
// `flag` package's convention) so wrappers can distinguish "you used
// it wrong" from a real failure.
var errUsage = errors.New("usage error")

func usage(w io.Writer) {
	_, _ = fmt.Fprint(w, `coverage - aggregate Go + C++ coverage for the self-hosted gh-pages pipeline.

Subcommands:
  summarize  Combine a Go coverage profile and/or an LCOV file into summary.json.
  badge      Emit a shields.io endpoint JSON for one field of summary.json.
  gate       Compare current summary to the baseline and fail on regression.

Run "coverage <subcommand> -h" for per-subcommand flags.
`)
}

// flagSet builds a FlagSet that prints its usage to stderr and stops
// on the first error. Centralizing this keeps every subcommand
// behaving the same way (and keeps tests from being polluted by
// stdlib's default ExitOnError behaviour).
func flagSet(name string, stderr io.Writer) *flag.FlagSet {
	fs := flag.NewFlagSet(name, flag.ContinueOnError)
	fs.SetOutput(stderr)
	return fs
}

func main() {
	if err := run(os.Args[1:], os.Stdout, os.Stderr); err != nil {
		if errors.Is(err, errUsage) {
			os.Exit(2)
		}
		_, _ = fmt.Fprintf(os.Stderr, "coverage: %v\n", err)
		os.Exit(1)
	}
}
