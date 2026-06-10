// googlesql-corpus drives the vendored GoogleSQL compliance .test subset
// through jobs.query and diffs results with the fixture lane's typed-cell
// comparator.
package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"github.com/vantaboard/bigquery-emulator/conformance/googlesqlcorpus"
	"github.com/vantaboard/bigquery-emulator/conformance/runner"
)

func main() {
	code, err := run()
	if err != nil {
		_, _ = fmt.Fprintln(os.Stderr, "googlesql-corpus:", err)
		os.Exit(2)
	}
	if code != 0 {
		os.Exit(code)
	}
}

type cliConfig struct {
	corpusDir    string
	manifestPath string
	engineBinary string
	profile      string
	gatePinned   bool
	triage       bool
	output       string
	outputFile   string
}

func run() (int, error) {
	cfg, err := parseCLI(os.Args[1:])
	if err != nil {
		if errors.Is(err, flag.ErrHelp) {
			return 0, nil
		}
		return 0, err
	}

	manifest, err := googlesqlcorpus.LoadManifest(cfg.manifestPath)
	if err != nil && !os.IsNotExist(err) {
		return 0, err
	}
	if manifest == nil {
		manifest = &googlesqlcorpus.Manifest{}
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

	report, err := googlesqlcorpus.Run(ctx, googlesqlcorpus.Options{
		CorpusDir:  cfg.corpusDir,
		Manifest:   manifest,
		GatePinned: cfg.gatePinned && !cfg.triage,
		TriageMode: cfg.triage,
		Harness: runner.HarnessOptions{
			EngineBinary: cfg.engineBinary,
			EngineStdout: os.Stderr,
			EngineStderr: os.Stderr,
		},
		Profile: cfg.profile,
		Out:     os.Stdout,
		Err:     os.Stderr,
	})
	if err != nil {
		return 0, err
	}

	if cfg.triage {
		updateManifestFromTriage(manifest, report)
		if err := googlesqlcorpus.SaveManifest(cfg.manifestPath, manifest); err != nil {
			return 0, err
		}
	}

	if err := writeReport(report, cfg.output, cfg.outputFile); err != nil {
		return 0, err
	}

	return report.ExitCode(), nil
}

func parseCLI(args []string) (cliConfig, error) {
	fs := flag.NewFlagSet("googlesql-corpus", flag.ContinueOnError)
	cfg := cliConfig{}
	fs.StringVar(&cfg.corpusDir, "corpus", "conformance/googlesql-corpus/corpus", "directory of vendored .test files")
	fs.StringVar(
		&cfg.manifestPath,
		"manifest",
		"conformance/googlesql-corpus/manifest/pinned.json",
		"pinned-passing manifest",
	)
	fs.StringVar(&cfg.engineBinary, "engine-binary", "./bin/emulator_main", "path to emulator_main")
	fs.StringVar(&cfg.profile, "profile", "duckdb", "conformance profile")
	fs.BoolVar(&cfg.gatePinned, "gate-pinned", true, "only run cases listed in manifest pinned set")
	fs.BoolVar(&cfg.triage, "triage", false, "run all runnable cases and write triage buckets to manifest")
	fs.StringVar(&cfg.output, "output", "text", "text or json")
	fs.StringVar(&cfg.outputFile, "output-file", "", "optional report path")
	if err := fs.Parse(args); err != nil {
		return cliConfig{}, err
	}
	return cfg, nil
}

func writeReport(report *googlesqlcorpus.Report, output, outputFile string) error {
	if output != "json" && outputFile == "" {
		return nil
	}
	b, err := json.MarshalIndent(report, "", "  ")
	if err != nil {
		return err
	}
	if outputFile != "" {
		if err := os.WriteFile(
			outputFile,
			append(b, '\n'),
			0o600,
		); err != nil { //nolint:gosec // report path is CLI-controlled
			return err
		}
	}
	if output == "json" {
		fmt.Println(string(b))
	}
	return nil
}

func updateManifestFromTriage(m *googlesqlcorpus.Manifest, report *googlesqlcorpus.Report) {
	if m.Triage == nil {
		m.Triage = make(map[string]googlesqlcorpus.TriageEntry)
	}
	m.Pinned = nil
	for _, r := range report.Results {
		switch r.Status {
		case string(runner.StatusPass):
			m.Pinned = append(m.Pinned, r.ID)
			m.Triage[r.ID] = googlesqlcorpus.TriageEntry{Bucket: googlesqlcorpus.BucketPinnedPass}
		case string(runner.StatusSkip):
			m.Triage[r.ID] = googlesqlcorpus.TriageEntry{Bucket: r.Bucket, Message: r.Message}
		default:
			m.Triage[r.ID] = googlesqlcorpus.TriageEntry{Bucket: r.Bucket, Message: r.Message}
		}
	}
}
