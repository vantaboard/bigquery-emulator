package main

import (
	"errors"
	"fmt"
	"io"
	"os"
	"strings"
)

// errRegression is returned by runGate when one or more tracked
// fields fell below the configured tolerance. It causes the binary
// to exit with status 1 so CI marks the check as failed.
var errRegression = errors.New("coverage regression")

// gateResult is the per-field outcome the gate prints to stdout. We
// always print one row per tracked flag (total, go, cpp) so the GitHub
// step-summary table is consistent across runs, even when a flag
// happens to be missing in either the current or baseline summary.
type gateResult struct {
	field       string
	current     float64
	baseline    float64
	tolerance   float64
	floor       float64
	regression  float64 // baseline - current; positive means we went down
	belowFloor  bool
	overTol     bool
	missingCurr bool
	missingBase bool
}

func (r gateResult) failed() bool {
	return r.overTol || r.belowFloor
}

// formatPct renders one percentage cell for the gate's table output.
// Missing flags collapse to "n/a" to match the badge command and keep
// the visual mapping between the two outputs obvious.
func formatPct(v float64) string {
	if v < 0 {
		return missingMessage
	}
	return fmt.Sprintf("%.1f%%", v)
}

// runGate implements `coverage gate`. It loads both summaries,
// compares each tracked field, prints a markdown-friendly table, and
// returns errRegression iff any field tripped the tolerance or floor.
//
// When --baseline points at a file that doesn't exist, the gate
// treats every field as having no baseline yet, prints a warning to
// stderr, and exits 0. That preserves the bootstrap case where
// gh-pages has not been populated yet.
//
//nolint:cyclop // straight-line flag handling; refactor would hurt readability.
func runGate(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("gate", stderr)
	currentPath := fs.String("current", "", "current summary.json path (required)")
	baselinePath := fs.String("baseline", "", "baseline summary.json path (required)")
	tol := fs.Float64("tolerance", 1.0, "max allowed regression (percentage points) on the total field")
	floor := fs.Float64("floor", 0.0, "absolute floor (percentage points) on the total field; 0 disables")
	goTol := fs.Float64("go-tolerance", 1.0, "max allowed regression on the go field")
	goFloor := fs.Float64("go-floor", 0.0, "absolute floor on the go field")
	cppTol := fs.Float64("cpp-tolerance", 1.0, "max allowed regression on the cpp field")
	cppFloor := fs.Float64("cpp-floor", 0.0, "absolute floor on the cpp field")
	if err := fs.Parse(args); err != nil {
		return err
	}
	if *currentPath == "" || *baselinePath == "" {
		_, _ = fmt.Fprintln(stderr, "gate: --current and --baseline are both required")
		fs.Usage()
		return errUsage
	}

	current, err := readSummary(*currentPath)
	if err != nil {
		return err
	}
	baseline, baselineMissing, err := loadBaseline(*baselinePath, stderr)
	if err != nil {
		return err
	}

	results := []gateResult{
		evalField("total", current.Total, baseline.Total, *tol, *floor),
		evalField("go", current.Go, baseline.Go, *goTol, *goFloor),
		evalField("cpp", current.CPP, baseline.CPP, *cppTol, *cppFloor),
	}

	writeGateTable(stdout, results, baselineMissing)

	if baselineMissing {
		return nil
	}
	for _, r := range results {
		if r.failed() {
			return errRegression
		}
	}
	return nil
}

// loadBaseline opens the baseline JSON, treating a non-existent file
// as the bootstrap case (returns a zero-valued Summary, missing=true,
// no error). Any other read or decode failure propagates up so CI
// surfaces the real problem instead of silently passing.
func loadBaseline(path string, stderr io.Writer) (*Summary, bool, error) {
	if _, err := os.Stat(path); errors.Is(err, os.ErrNotExist) {
		_, _ = fmt.Fprintf(stderr, "gate: baseline %q does not exist; treating as bootstrap (no gate).\n", path)
		return &Summary{Total: missingFlag, Go: missingFlag, CPP: missingFlag}, true, nil
	} else if err != nil {
		return nil, false, fmt.Errorf("stat baseline %q: %w", path, err)
	}
	s, err := readSummary(path)
	if err != nil {
		return nil, false, err
	}
	return s, false, nil
}

// evalField runs the comparison for a single tracked field. Missing
// inputs collapse to "no opinion": if either the current or the
// baseline value is the missing sentinel, that field cannot fail the
// gate. This protects against, e.g., the C++ Bazel suite being
// temporarily disabled and the gate suddenly demanding a 0 -> 0
// improvement on a flag with no real data.
func evalField(name string, current, baseline, tol, floor float64) gateResult {
	r := gateResult{
		field:       name,
		current:     current,
		baseline:    baseline,
		tolerance:   tol,
		floor:       floor,
		missingCurr: current < 0,
		missingBase: baseline < 0,
	}
	if r.missingCurr || r.missingBase {
		return r
	}
	r.regression = baseline - current
	if r.regression > tol {
		r.overTol = true
	}
	if floor > 0 && current < floor {
		r.belowFloor = true
	}
	return r
}

// writeGateTable prints a markdown table summarising every field, the
// per-row result, and a final pass/fail line. The output is written
// to stdout (so it slots into `$GITHUB_STEP_SUMMARY` via
// `coverage gate ... >> $GITHUB_STEP_SUMMARY`) rather than stderr.
func writeGateTable(w io.Writer, results []gateResult, baselineMissing bool) {
	var b strings.Builder
	b.WriteString("## Coverage gate\n\n")
	if baselineMissing {
		b.WriteString("_Baseline missing; no regression gate enforced for this run._\n\n")
	}
	b.WriteString("| field | current | baseline | delta | tolerance | floor | status |\n")
	b.WriteString("|-------|---------|----------|-------|-----------|-------|--------|\n")
	for _, r := range results {
		b.WriteString(formatRow(r))
	}
	_, _ = fmt.Fprint(w, b.String())
}

func formatRow(r gateResult) string {
	delta := fmt.Sprintf("%+.1f%%", -r.regression)
	if r.missingCurr || r.missingBase {
		delta = "-"
	}
	status := "ok"
	switch {
	case r.missingCurr || r.missingBase:
		status = "skipped (missing data)"
	case r.overTol:
		status = fmt.Sprintf("FAIL: regressed %.1fpp > %.1fpp tol", r.regression, r.tolerance)
	case r.belowFloor:
		status = fmt.Sprintf("FAIL: below floor %.1f%%", r.floor)
	}
	return fmt.Sprintf(
		"| %s | %s | %s | %s | %.1fpp | %.1f%% | %s |\n",
		r.field,
		formatPct(r.current),
		formatPct(r.baseline),
		delta,
		r.tolerance,
		r.floor,
		status,
	)
}
