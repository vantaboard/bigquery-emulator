// Binary runner is the conformance harness's CLI. It loads YAML
// fixtures, iterates the engine × storage profile matrix, and emits
// PASS / FAIL records (or a JSON report). See `conformance/README.md`
// for the fixture schema and worked examples; this file is just flag
// parsing and exit-code wiring.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"strings"
	"syscall"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
)

// stringSliceFlag is a repeatable flag value, so `--profile memory
// --profile duckdb` accumulates into one slice rather than overwriting.
type stringSliceFlag []string

func (s *stringSliceFlag) String() string { return strings.Join(*s, ",") }
func (s *stringSliceFlag) Set(v string) error {
	if v == "" {
		return nil
	}
	*s = append(*s, v)
	return nil
}

func main() {
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, "runner:", err)
		os.Exit(2)
	}
}

func run() error {
	fs := flag.NewFlagSet("runner", flag.ContinueOnError)
	fs.SetOutput(os.Stderr)

	var (
		fixtures        = fs.String("fixtures", "conformance/fixtures", "directory or file containing fixture YAML")
		engineBinary    = fs.String("engine-binary", "./bin/emulator_main", "path to emulator_main; mutually exclusive with --connect")
		connect         = fs.String("connect", "", "HOST:PORT of an already-running engine to dial instead of spawning emulator_main")
		updateBaselines = fs.Bool("update-baselines", false, "overwrite each fixture's expected: block with the captured response (bootstrap mode)")
		output          = fs.String("output", "text", "output format: text or json")
		profiles        stringSliceFlag
		showHelp        = fs.Bool("help", false, "print usage and exit")
	)
	fs.Var(&profiles, "profile", "restrict the matrix to one profile (repeatable). Default: all known profiles")

	fs.Usage = func() {
		fmt.Fprintln(fs.Output(), `Usage: runner [flags]

Run the BigQuery emulator conformance fixtures and diff against
expected rows or errors. By default the runner spawns its own
emulator_main subprocess per fixture × profile; --connect HOST:PORT
reaches an already-running gateway (used by CI).

Flags:`)
		fs.PrintDefaults()
		fmt.Fprintln(fs.Output(), `
Profiles:
  memory   reference_impl engine + memory storage  (default in matrix)
  duckdb   duckdb engine + duckdb storage + fallback (default in matrix)

Exit codes:
  0   every fixture × profile PASSed
  1   at least one fixture × profile FAILed
  2   runner-internal error (bad YAML, can't start engine, etc)

See conformance/README.md for the fixture schema and JSON output
shape.`)
	}

	if err := fs.Parse(os.Args[1:]); err != nil {
		if errors.Is(err, flag.ErrHelp) {
			return nil
		}
		return err
	}
	if *showHelp {
		fs.Usage()
		return nil
	}
	if *engineBinary != "" && *connect != "" {
		// Flag default is `./bin/emulator_main`; only treat it as
		// user-supplied when --connect is the empty default. The
		// CLI lets the user pick either path explicitly.
		if *engineBinary != "./bin/emulator_main" {
			return errors.New("--engine-binary and --connect are mutually exclusive")
		}
		*engineBinary = ""
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// SIGINT/SIGTERM cancel the runner's context so the harness can
	// SIGINT every emulator subprocess it spawned. The runner
	// returns its in-progress Report so the caller still sees what
	// PASSed before the cancel.
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-sigCh
		cancel()
	}()
	defer signal.Stop(sigCh)

	report, err := runner.Run(ctx, runner.Options{
		FixturesPath: *fixtures,
		Harness: runner.HarnessOptions{
			EngineBinary:   *engineBinary,
			ConnectAddress: *connect,
			EngineStdout:   os.Stderr,
			EngineStderr:   os.Stderr,
		},
		Profiles:        []string(profiles),
		UpdateBaselines: *updateBaselines,
		Output:          *output,
		Out:             os.Stdout,
		Err:             os.Stderr,
	})
	if err != nil {
		return err
	}
	if code := report.ExitCode(); code != 0 {
		os.Exit(code)
	}
	return nil
}
