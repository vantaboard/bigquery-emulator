package runner

import (
	"encoding/json"
	"fmt"
	"io"
	"strings"
)

// writeTextResult prints one fixture x profile result in human-readable
// form. The format is intentionally short so a sweep over hundreds of
// fixtures stays scannable.
func writeTextResult(w io.Writer, r Result) {
	tag := string(r.Status)
	prefix := ""
	switch r.Status {
	case StatusPass:
		prefix = "PASS"
	case StatusFail:
		prefix = "FAIL"
	case StatusSkip:
		prefix = "SKIP"
	default:
		prefix = tag
	}
	fmt.Fprintf(w, "[%s] %s (profile=%s, %dms)\n",
		prefix, r.Fixture, r.Profile, r.DurationMs)
	if r.Message != "" {
		fmt.Fprintf(w, "       %s\n", r.Message)
	}
	if r.Diff != "" {
		for _, line := range strings.Split(strings.TrimRight(r.Diff, "\n"), "\n") {
			fmt.Fprintf(w, "       %s\n", line)
		}
	}
}

// writeTextSummary prints the matrix-level rollup. Mirrors
// `go test`'s `--- PASS` style so engineers reading the log don't
// have to learn a new vocabulary.
func writeTextSummary(w io.Writer, report *Report) {
	fmt.Fprintf(w, "---\n")
	fmt.Fprintf(w, "conformance: total=%d passed=%d failed=%d skipped=%d\n",
		report.Summary.Total,
		report.Summary.Passed,
		report.Summary.Failed,
		report.Summary.Skipped)
}

// writeJSONReport emits the report's machine-readable form. The
// schema is documented in `conformance/README.md` (the "JSON output
// shape" section). Plan-41 CI pivots on `schema_version`.
func writeJSONReport(w io.Writer, report *Report) error {
	enc := json.NewEncoder(w)
	enc.SetIndent("", "  ")
	return enc.Encode(report)
}
