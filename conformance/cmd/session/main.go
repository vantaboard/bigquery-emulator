// session runs stateful multi-step conformance sessions against one long-lived
// emulator process per session.
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

type sessionConfig struct {
	Sessions        string
	EngineBinary    string
	Connect         string
	Profile         string
	Output          string
	OutputFile      string
	IncludeSelfTest bool
}

func main() {
	code, err := run()
	if err != nil {
		_, _ = fmt.Fprintln(os.Stderr, "session:", err)
		os.Exit(2)
	}
	if code != 0 {
		os.Exit(code)
	}
}

func run() (int, error) {
	cfg, err := parseFlags(os.Args[1:])
	if err != nil {
		if errors.Is(err, flag.ErrHelp) {
			return 0, nil
		}
		return 0, err
	}

	out, cleanup, err := setupOutputFile(cfg.OutputFile)
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

	var profiles []string
	if strings.TrimSpace(cfg.Profile) != "" {
		profiles = []string{cfg.Profile}
	}

	report, err := runner.RunSessions(ctx, runner.SessionOptions{
		SessionsPath:    cfg.Sessions,
		IncludeSelfTest: cfg.IncludeSelfTest,
		Harness: runner.HarnessOptions{
			EngineBinary:   cfg.EngineBinary,
			ConnectAddress: cfg.Connect,
			EngineStdout:   os.Stderr,
			EngineStderr:   os.Stderr,
		},
		Profiles: profiles,
		Output:   cfg.Output,
		Out:      out,
		Err:      os.Stderr,
	})
	if err != nil {
		return 0, err
	}
	return report.ExitCode(), nil
}

func parseFlags(args []string) (sessionConfig, error) {
	fs := flag.NewFlagSet("session", flag.ContinueOnError)
	fs.SetOutput(os.Stderr)
	var cfg sessionConfig
	fs.StringVar(&cfg.Sessions, "sessions", runner.DefaultSessionsDir, "session directory or single YAML")
	fs.StringVar(&cfg.EngineBinary, "engine-binary", "./bin/emulator_main", "path to emulator_main")
	fs.StringVar(&cfg.Connect, "connect", "", "HOST:PORT of a running gateway")
	fs.StringVar(&cfg.Profile, "profile", "", "runtime profile (default: all known profiles)")
	fs.StringVar(&cfg.Output, "output", "text", "output format: text or json")
	fs.StringVar(&cfg.OutputFile, "output-file", "", "tee report to this file (atomic write)")
	fs.BoolVar(&cfg.IncludeSelfTest, "include-selftest", false, "run _-prefixed self-test session files")
	if err := fs.Parse(args); err != nil {
		return sessionConfig{}, err
	}
	if cfg.EngineBinary != "" && cfg.Connect != "" && cfg.EngineBinary != "./bin/emulator_main" {
		return sessionConfig{}, errors.New("--engine-binary and --connect are mutually exclusive")
	}
	if cfg.Connect != "" {
		cfg.EngineBinary = ""
	}
	return cfg, nil
}

func setupOutputFile(path string) (io.Writer, func(), error) {
	if path == "" {
		return os.Stdout, nil, nil
	}
	dir := filepath.Dir(path)
	if dir == "" {
		dir = "."
	}
	tmp, err := os.CreateTemp(dir, ".session-*.tmp")
	if err != nil {
		return nil, nil, fmt.Errorf("create --output-file tmp: %w", err)
	}
	tmpName := tmp.Name()
	cleanup := func() {
		_ = tmp.Close()
		if err := os.Rename(tmpName, path); err != nil {
			_, _ = fmt.Fprintln(os.Stderr, "session: rename --output-file:", err)
			_ = os.Remove(tmpName)
		}
	}
	return io.MultiWriter(os.Stdout, tmp), cleanup, nil
}
