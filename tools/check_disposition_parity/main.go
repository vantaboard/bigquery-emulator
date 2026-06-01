// Command check-disposition-parity verifies that the per-node
// disposition table the engine consumes
// (`backend/engine/duckdb/transpiler/node_dispositions.yaml`)
// agrees row-for-row with the human-readable mirror in
// `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`.
//
// The two files MUST stay in lock-step: the YAML is the
// machine-readable source of truth the engine router and the
// generated `node_dispositions_table.inc` consume; the markdown is
// the human-readable mirror referenced from ROADMAP.md and the
// per-plan docs. A drift between them means either the engine and
// the docs disagree on what route a node kind takes, or the
// `(planned)` annotation in the docs has stopped tracking reality.
//
// Usage (typical):
//
//	go run ./tools/check_disposition_parity        # check repo files
//	go run ./tools/check_disposition_parity \
//	    --yaml=path/to/node_dispositions.yaml \
//	    --shape-tracker=path/to/SHAPE_TRACKER.md
//
// Exit codes mirror the Go `flag` package convention:
//
//   - 0: tables agree
//   - 1: drift detected (the report names every offending row)
//   - 2: usage error (missing or unreadable input file)
//
// The deliberately-thin CLI surface keeps the checker drop-in
// runnable from `task lint:dispositions` (the canonical
// developer-facing entry point) and from the CI lint job, with no
// extra flags needed for the common case.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

const (
	defaultYAMLRel         = "backend/engine/duckdb/transpiler/node_dispositions.yaml"
	defaultShapeTrackerRel = "backend/engine/duckdb/transpiler/SHAPE_TRACKER.md"
)

var (
	errUsage    = errors.New("usage error")
	errFindings = errors.New("parity findings")
)

func main() {
	if err := run(os.Args[1:], os.Stdout, os.Stderr); err != nil {
		switch {
		case errors.Is(err, errUsage):
			os.Exit(2)
		case errors.Is(err, errFindings):
			os.Exit(1)
		default:
			_, _ = fmt.Fprintf(os.Stderr, "check-disposition-parity: %v\n", err)
			os.Exit(1)
		}
	}
}

// run is the testable entry point. It returns an error rather than
// calling os.Exit so tests can drive the full code path with
// table-driven fixtures.
func run(args []string, stdout, stderr io.Writer) error {
	fs := flag.NewFlagSet("check-disposition-parity", flag.ContinueOnError)
	fs.SetOutput(stderr)
	yamlPath := fs.String("yaml", "",
		"Path to node_dispositions.yaml (defaults to the repo-relative "+
			"path under the current working directory).")
	shapePath := fs.String("shape-tracker", "",
		"Path to SHAPE_TRACKER.md (defaults to the repo-relative path "+
			"under the current working directory).")
	if err := fs.Parse(args); err != nil {
		return errUsage
	}
	if fs.NArg() > 0 {
		_, _ = fmt.Fprintf(stderr,
			"check-disposition-parity: unexpected positional args: %v\n",
			fs.Args())
		return errUsage
	}

	yp, sp, err := resolvePaths(*yamlPath, *shapePath)
	if err != nil {
		_, _ = fmt.Fprintf(stderr, "check-disposition-parity: %v\n", err)
		return errUsage
	}

	// gosec G304: this CLI takes user-controlled paths by design (the
	// --yaml / --shape-tracker flags); the same posture applies as
	// the rest of the in-repo lint tooling that consumes file lists.
	yamlBytes, err := os.ReadFile(yp) //nolint:gosec // user-controlled lint input
	if err != nil {
		return fmt.Errorf("read %s: %w", yp, err)
	}
	shapeBytes, err := os.ReadFile(sp) //nolint:gosec // user-controlled lint input
	if err != nil {
		return fmt.Errorf("read %s: %w", sp, err)
	}

	yamlRows, err := parseYAML(string(yamlBytes))
	if err != nil {
		return fmt.Errorf("parse %s: %w", yp, err)
	}
	shapeRows, err := parseShapeTracker(string(shapeBytes))
	if err != nil {
		return fmt.Errorf("parse %s: %w", sp, err)
	}

	findings := compareParity(yamlRows, shapeRows)
	if len(findings) == 0 {
		_, _ = fmt.Fprintf(stdout,
			"check-disposition-parity: ok (%d YAML rows, %d SHAPE_TRACKER rows)\n",
			len(yamlRows), len(shapeRows))
		return nil
	}
	for _, f := range findings {
		_, _ = fmt.Fprintln(stderr, f)
	}
	_, _ = fmt.Fprintf(stderr,
		"check-disposition-parity: %d disagreement(s) between %s and %s\n",
		len(findings), yp, sp)
	return errFindings
}

// resolvePaths fills in the defaults when the caller did not pass
// explicit --yaml / --shape-tracker flags. The defaults are the
// well-known repo-relative paths against the current working
// directory; callers run from the repo root by convention (via
// `task lint:dispositions`).
func resolvePaths(yamlFlag, shapeFlag string) (yp, sp string, err error) {
	yp = yamlFlag
	sp = shapeFlag
	if yp == "" {
		yp = filepath.FromSlash(defaultYAMLRel)
	}
	if sp == "" {
		sp = filepath.FromSlash(defaultShapeTrackerRel)
	}
	if _, statErr := os.Stat(yp); statErr != nil {
		return "", "", fmt.Errorf("--yaml path %q is not readable: %w", yp, statErr)
	}
	if _, statErr := os.Stat(sp); statErr != nil {
		return "", "", fmt.Errorf("--shape-tracker path %q is not readable: %w", sp, statErr)
	}
	return yp, sp, nil
}
