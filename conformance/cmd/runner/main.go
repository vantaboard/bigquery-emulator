// Binary runner is the conformance harness's CLI. It loads YAML
// fixtures, iterates the engine x storage profile matrix, and emits
// PASS / FAIL records (or a JSON report). See `conformance/README.md`
// for the fixture schema and worked examples; this file is just flag
// parsing and exit-code wiring.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"syscall"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
)

// stringSliceFlag is a repeatable flag value, so `--profile duckdb`
// (and any future profile names) accumulate into one slice rather
// than overwriting.
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
	code, err := run()
	if err != nil {
		_, _ = fmt.Fprintln(os.Stderr, "runner:", err)
		os.Exit(2)
	}
	if code != 0 {
		os.Exit(code)
	}
}

// runnerConfig is the parsed view of the CLI flags that `run` hands
// off to the conformance runner. Pulled out of run() so the flag-
// parsing block (and the engine-binary / --connect mutual-exclusion
// rule) can live in its own helper without smuggling state through
// closures.
type runnerConfig struct {
	Fixtures        string
	EngineBinary    string
	Connect         string
	UpdateBaselines bool
	Output          string
	OutputFile      string
	Profiles        []string
	HelpExit        bool // user passed --help; main should exit 0.
}

// run drives the binary's flag parse + signal handling + runner.Run
// orchestration. Returns the exit code main should hand to os.Exit
// (so any defers in this function actually fire) plus any
// runner-internal error main should print.
func run() (int, error) {
	cfg, err := parseFlags(os.Args[1:])
	if err != nil {
		if errors.Is(err, flag.ErrHelp) {
			return 0, nil
		}
		return 0, err
	}
	if cfg.HelpExit {
		return 0, nil
	}

	runnerStdout, cleanup, err := setupOutputFile(cfg.OutputFile)
	if err != nil {
		return 0, err
	}
	if cleanup != nil {
		defer cleanup()
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
		FixturesPath: cfg.Fixtures,
		Harness: runner.HarnessOptions{
			EngineBinary:   cfg.EngineBinary,
			ConnectAddress: cfg.Connect,
			EngineStdout:   os.Stderr,
			EngineStderr:   os.Stderr,
		},
		Profiles:        cfg.Profiles,
		UpdateBaselines: cfg.UpdateBaselines,
		Output:          cfg.Output,
		Out:             runnerStdout,
		Err:             os.Stderr,
	})
	if err != nil {
		return 0, err
	}
	return report.ExitCode(), nil
}

// parseFlags wires up the CLI's flag set, applies the
// --engine-binary / --connect mutual-exclusion rule, and returns a
// runnerConfig. Returns flag.ErrHelp when the user passed --help so
// the caller can short-circuit cleanly.
func parseFlags(args []string) (runnerConfig, error) {
	fs := flag.NewFlagSet("runner", flag.ContinueOnError)
	fs.SetOutput(os.Stderr)

	var (
		fixtures     = fs.String("fixtures", "conformance/fixtures", "directory or file containing fixture YAML")
		engineBinary = fs.String(
			"engine-binary",
			"./bin/emulator_main",
			"path to emulator_main; mutually exclusive with --connect",
		)
		connect = fs.String(
			"connect",
			"",
			"HOST:PORT of an already-running engine to dial instead of spawning emulator_main",
		)
		updateBaselines = fs.Bool(
			"update-baselines",
			false,
			"overwrite each fixture's expected: block with the captured response (bootstrap mode)",
		)
		output     = fs.String("output", "text", "output format: text or json")
		outputFile = fs.String(
			"output-file",
			"",
			"if non-empty, write the rendered report to this file (atomic write) in addition to stdout",
		)
		profiles stringSliceFlag
		showHelp = fs.Bool("help", false, "print usage and exit")
	)
	fs.Var(&profiles, "profile", "restrict the matrix to one profile (repeatable). Default: all known profiles")

	fs.Usage = func() { writeUsage(fs) }

	if err := fs.Parse(args); err != nil {
		return runnerConfig{}, err
	}
	if *showHelp {
		fs.Usage()
		return runnerConfig{HelpExit: true}, nil
	}
	if *engineBinary != "" && *connect != "" {
		// Flag default is `./bin/emulator_main`; only treat it as
		// user-supplied when --connect is the empty default. The
		// CLI lets the user pick either path explicitly.
		if *engineBinary != "./bin/emulator_main" {
			return runnerConfig{}, errors.New("--engine-binary and --connect are mutually exclusive")
		}
		*engineBinary = ""
	}
	return runnerConfig{
		Fixtures:        *fixtures,
		EngineBinary:    *engineBinary,
		Connect:         *connect,
		UpdateBaselines: *updateBaselines,
		Output:          *output,
		OutputFile:      *outputFile,
		Profiles:        []string(profiles),
	}, nil
}

// writeUsage emits the runner's --help banner. Pulled out of
// parseFlags so the flag-parsing function stays under the funlen
// limit; the heredoc's prose is the bulk of its line count.
func writeUsage(fs *flag.FlagSet) {
	_, _ = fmt.Fprintln(fs.Output(), `Usage: runner [flags]

Run the BigQuery emulator conformance fixtures and diff against
expected rows or errors. By default the runner spawns its own
emulator_main subprocess per fixture x profile; --connect HOST:PORT
reaches an already-running gateway (used by CI).

Flags:`)
	fs.PrintDefaults()
	_, _ = fmt.Fprintln(fs.Output(), `
Profiles:
  duckdb   duckdb engine + duckdb storage  (only profile today)

Exit codes:
  0   every fixture x profile PASSed
  1   at least one fixture x profile FAILed
  2   runner-internal error (bad YAML, can't start engine, etc)

See conformance/README.md for the fixture schema and JSON output
shape.`)
}

// setupOutputFile honors --output-file: it opens a sibling tmp file,
// returns an io.MultiWriter that tees the runner's output into both
// stdout and the tmp file, plus a cleanup closure the caller must
// defer to atomically rename the tmp file into place. When the flag
// is empty, returns os.Stdout and a nil cleanup.
//
// We rename regardless of whether the runner returned an error or
// reported a non-zero exit code (fixture mismatch): the artifact is
// still the most useful diagnostic the workflow has on hand. Only a
// CreateTemp failure (out of disk, perm denied) short-circuits
// before any data lands.
func setupOutputFile(path string) (io.Writer, func(), error) {
	if path == "" {
		return os.Stdout, nil, nil
	}
	dir := filepath.Dir(path)
	if dir == "" {
		dir = "."
	}
	tmp, err := os.CreateTemp(dir, ".conformance-runner-*.tmp")
	if err != nil {
		return nil, nil, fmt.Errorf("create --output-file tmp: %w", err)
	}
	tmpName := tmp.Name()
	cleanup := func() {
		_ = tmp.Close()
		if err := os.Rename(tmpName, path); err != nil {
			_, _ = fmt.Fprintln(os.Stderr, "runner: rename --output-file:", err)
			_ = os.Remove(tmpName)
		}
	}
	return io.MultiWriter(os.Stdout, tmp), cleanup, nil
}
