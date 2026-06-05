// Command cpp-lint is the BigQuery emulator's first-party C++
// source-only lint runner.
//
// It exists to enforce three classes of rule that clang-format and
// clang-tidy do not cover well in this repo:
//
//  1. The list of files we own (vs. vendored / generated / cached
//     trees) is computed in exactly one place. Every C++ lint task
//     consumes it via `cpp-lint list`, so a path that is not first
//     party can never sneak into clang-format, clang-tidy, or
//     cppcheck.
//  2. A whole-file line-count rule (default 500 lines) that
//     clang-tidy's `readability-function-size` cannot express.
//  3. Repo-specific anti-patterns: banned production logging APIs
//     (`std::cout` / `std::cerr` / `printf` outside tests and
//     tools), and obvious `absl::Status` / `absl::StatusOr<T>`
//     misuse such as a discarded `Status` return or `.value()` on a
//     `StatusOr` without a status check first.
//
// The binary deliberately depends only on the standard library so a
// fresh checkout can run `go run ./tools/lint/cpp` (or the
// `task lint:cpp:source` wrapper) without bootstrapping anything
// extra. Subcommands and flags follow the same shape as
// `tools/coverage` so contributors recognise the layout.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
)

// Subcommand names. Centralised so the dispatch in run() and the
// table-driven tests stay aligned.
const (
	cmdList  = "list"
	cmdCheck = "check"
)

// run is the testable entry point. It returns an error instead of
// calling os.Exit so tests can drive the full code path with
// table-driven fixtures without managing process lifetime.
func run(args []string, stdout, stderr io.Writer) error {
	if len(args) < 1 {
		usage(stderr)
		return errUsage
	}
	cmd, rest := args[0], args[1:]
	switch cmd {
	case cmdList:
		return runList(rest, stdout, stderr)
	case cmdCheck:
		return runCheck(rest, stdout, stderr)
	case "-h", "--help", "help":
		usage(stdout)
		return nil
	default:
		_, _ = fmt.Fprintf(stderr, "cpp-lint: unknown subcommand %q\n\n", cmd)
		usage(stderr)
		return errUsage
	}
}

// errUsage maps to exit code 2, matching the Go `flag` package's
// convention so wrappers can distinguish "you used it wrong" from a
// real lint failure (exit 1).
var errUsage = errors.New("usage error")

// errFindings signals that the lint run completed cleanly but at
// least one rule reported a finding. Wrappers exit 1 on this so CI
// can tell rule violations apart from infrastructure failures.
var errFindings = errors.New("lint findings")

func usage(w io.Writer) {
	_, _ = fmt.Fprint(w, `cpp-lint - first-party C++ source-only lint runner.

Subcommands:
  list      Print the canonical first-party C++ source list (one path per line).
  check     Run the source-only checks (file size, banned logging, status misuse).

Run "cpp-lint <subcommand> -h" for per-subcommand flags.
`)
}

// flagSet builds a FlagSet that prints its usage to stderr and stops
// on the first error. Centralising this keeps every subcommand
// behaving the same way (and keeps tests from being polluted by the
// stdlib's default ExitOnError behaviour).
func flagSet(name string, stderr io.Writer) *flag.FlagSet {
	fs := flag.NewFlagSet(name, flag.ContinueOnError)
	fs.SetOutput(stderr)
	return fs
}

func main() {
	if err := run(os.Args[1:], os.Stdout, os.Stderr); err != nil {
		switch {
		case errors.Is(err, errUsage):
			os.Exit(2)
		case errors.Is(err, errFindings):
			os.Exit(1)
		default:
			_, _ = fmt.Fprintf(os.Stderr, "cpp-lint: %v\n", err)
			os.Exit(1)
		}
	}
}
