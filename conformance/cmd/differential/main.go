// differential replays committed production-BigQuery oracles against the emulator.
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
	"syscall"

	"github.com/vantaboard/bigquery-emulator/conformance/differential"
	"github.com/vantaboard/bigquery-emulator/conformance/runner"
)

func main() {
	code, err := run()
	if err != nil {
		_, _ = fmt.Fprintln(os.Stderr, "differential:", err)
		os.Exit(2)
	}
	if code != 0 {
		os.Exit(code)
	}
}

func run() (int, error) {
	fs := flag.NewFlagSet("differential", flag.ContinueOnError)
	fs.SetOutput(os.Stderr)
	corpus := fs.String("corpus", differential.DefaultCorpusDir, "corpus directory or single YAML")
	oracleDir := fs.String("oracle-dir", differential.DefaultOracleDir, "directory of committed oracle JSON files")
	engineBinary := fs.String("engine-binary", "./bin/emulator_main", "path to emulator_main")
	connect := fs.String("connect", "", "HOST:PORT of a running gateway (mutually exclusive with --engine-binary)")
	profile := fs.String("profile", "duckdb", "runtime profile")
	output := fs.String("output", "text", "output format: text or json")
	outputFile := fs.String("output-file", "", "tee report to this file (atomic write)")
	includeSelfTest := fs.Bool("include-selftest", false, "run _-prefixed self-test corpus files")
	if err := fs.Parse(os.Args[1:]); err != nil {
		if errors.Is(err, flag.ErrHelp) {
			return 0, nil
		}
		return 0, err
	}
	if *engineBinary != "" && *connect != "" && *engineBinary != "./bin/emulator_main" {
		return 0, errors.New("--engine-binary and --connect are mutually exclusive")
	}
	if *connect != "" {
		*engineBinary = ""
	}

	out, cleanup, err := setupOutputFile(*outputFile)
	if err != nil {
		return 0, err
	}
	if cleanup != nil {
		defer cleanup()
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-sigCh
		cancel()
	}()
	defer signal.Stop(sigCh)

	report, err := differential.Run(ctx, differential.Options{
		CorpusDir:       *corpus,
		OracleDir:       *oracleDir,
		IncludeSelfTest: *includeSelfTest,
		Harness: runner.HarnessOptions{
			EngineBinary:   *engineBinary,
			ConnectAddress: *connect,
			EngineStdout:   os.Stderr,
			EngineStderr:   os.Stderr,
		},
		Profile: *profile,
		Output:  *output,
		Out:     out,
		Err:     os.Stderr,
	})
	if err != nil {
		return 0, err
	}
	return report.ExitCode(), nil
}

func setupOutputFile(path string) (io.Writer, func(), error) {
	if path == "" {
		return os.Stdout, nil, nil
	}
	dir := filepath.Dir(path)
	if dir == "" {
		dir = "."
	}
	tmp, err := os.CreateTemp(dir, ".differential-*.tmp")
	if err != nil {
		return nil, nil, fmt.Errorf("create --output-file tmp: %w", err)
	}
	tmpName := tmp.Name()
	cleanup := func() {
		_ = tmp.Close()
		if err := os.Rename(tmpName, path); err != nil {
			_, _ = fmt.Fprintln(os.Stderr, "differential: rename --output-file:", err)
			_ = os.Remove(tmpName)
		}
	}
	return io.MultiWriter(os.Stdout, tmp), cleanup, nil
}
