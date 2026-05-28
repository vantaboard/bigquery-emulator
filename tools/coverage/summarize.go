package main

import (
	"encoding/json"
	"fmt"
	"io"
	"os"
	"time"
)

// Summary is the JSON shape that lands at gh-pages:baseline.json after
// every main-branch build and at workflow_run artifact downloads for
// PR gating. Per-flag fields use -1 when no input was supplied for
// that flag so downstream consumers (badge, gate) can distinguish
// "missing" from "really zero coverage".
type Summary struct {
	// Total is the union percentage across every flag the summarize
	// command consumed; when only one flag is supplied it matches
	// that flag's percentage exactly.
	Total float64 `json:"total"`
	// Go is the percentage from the Go `coverage.out` profile.
	Go float64 `json:"go"`
	// CPP is the percentage from the Bazel/LCOV combined report.
	CPP float64 `json:"cpp"`
	// Commit is the git SHA the summary describes, if known.
	Commit string `json:"commit,omitempty"`
	// Timestamp is RFC3339 UTC.
	Timestamp string `json:"timestamp,omitempty"`
}

// missingFlag is the sentinel the consumers use to detect "no input
// for this flag was supplied". -1 is impossible for a real percentage,
// so it round-trips through JSON unambiguously.
const missingFlag = -1.0

// percentage divides hits by total guarding against zero, scales to a
// 0..100 range, and pins to one decimal place to keep the JSON file
// stable across runs that drift in the noise-floor.
func percentage(hits, total int64) float64 {
	if total <= 0 {
		return 0
	}
	pct := float64(hits) / float64(total) * 100
	return roundTenths(pct)
}

// roundTenths rounds to one decimal place so summary.json does not
// churn on minor floating-point noise. Avoids `math.Round` so the
// behaviour is obvious from the source.
func roundTenths(v float64) float64 {
	return float64(int64(v*10+0.5)) / 10
}

// runSummarize implements `coverage summarize`. Either of --go or
// --lcov may be omitted (e.g. when the lcov producer failed in CI);
// the missing flag's per-flag field is reported as `missingFlag` and
// the total is computed from the flags that did provide data.
//
//nolint:cyclop // straight-line option handling; splitting hurts readability.
func runSummarize(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("summarize", stderr)
	goPath := fs.String("go", "", "path to Go coverage profile (go test -coverprofile)")
	lcovPath := fs.String("lcov", "", "path to LCOV combined report (bazel coverage --combined_report=lcov)")
	outPath := fs.String("out", "", "output JSON path (default: stdout)")
	commit := fs.String("commit", "", "git commit SHA to record in summary.json (optional)")
	if err := fs.Parse(args); err != nil {
		return err
	}
	if *goPath == "" && *lcovPath == "" {
		_, _ = fmt.Fprintln(stderr, "summarize: at least one of --go or --lcov is required")
		fs.Usage()
		return errUsage
	}

	summary := Summary{
		Go:        missingFlag,
		CPP:       missingFlag,
		Commit:    *commit,
		Timestamp: time.Now().UTC().Format(time.RFC3339),
	}

	var goHits, goTotal, cppHits, cppTotal int64

	if *goPath != "" {
		h, t, err := parseGoFile(*goPath)
		if err != nil {
			return fmt.Errorf("parse go profile %q: %w", *goPath, err)
		}
		goHits, goTotal = h, t
		summary.Go = percentage(h, t)
	}

	if *lcovPath != "" {
		h, t, err := parseLCOVFile(*lcovPath)
		if err != nil {
			return fmt.Errorf("parse lcov file %q: %w", *lcovPath, err)
		}
		cppHits, cppTotal = h, t
		summary.CPP = percentage(h, t)
	}

	summary.Total = percentage(goHits+cppHits, goTotal+cppTotal)
	return emitSummary(&summary, *outPath, stdout)
}

// emitSummary marshals to JSON (indented, deterministic field order
// because Summary's fields are declared in the order we want) and
// writes either to a file or stdout. A trailing newline keeps the
// output friendly to `cat`.
func emitSummary(s *Summary, outPath string, stdout io.Writer) error {
	buf, err := json.MarshalIndent(s, "", "  ")
	if err != nil {
		return fmt.Errorf("marshal summary: %w", err)
	}
	buf = append(buf, '\n')

	if outPath == "" {
		_, err = stdout.Write(buf)
		return err
	}
	//nolint:gosec // 0o644 is the right mode for a CI-published JSON artifact.
	if err := os.WriteFile(outPath, buf, 0o644); err != nil {
		return fmt.Errorf("write %q: %w", outPath, err)
	}
	return nil
}

// readSummary loads a summary file previously written by
// runSummarize. Used by both the badge and gate subcommands.
func readSummary(path string) (*Summary, error) {
	//nolint:gosec // CLI tool; reading caller-supplied paths is the point.
	buf, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("read %q: %w", path, err)
	}
	var s Summary
	if err := json.Unmarshal(buf, &s); err != nil {
		return nil, fmt.Errorf("decode %q: %w", path, err)
	}
	return &s, nil
}
