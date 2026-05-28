package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
)

// parseLCOVFile opens an LCOV "tracefile" (the .dat that
// `bazel coverage --combined_report=lcov` deposits at
// bazel-out/_coverage/_coverage_report.dat) and returns total
// (hitLines, totalLines) across every `SF:` record. We sum the
// per-line `DA:<line>,<count>` records directly rather than trusting
// the summary `LH:` / `LF:` totals, because some toolchains emit `LH`
// without `LF` (or vice versa) when a file has no executable lines.
//
// The LCOV format is described in
// https://manpages.debian.org/testing/lcov/geninfo.1.en.html under
// "TRACEFILE FORMAT". For our purposes we only care about three
// record types:
//
//	SF:<absolute path>            // start of a source file record
//	DA:<line>,<count>             // one DA per executable line
//	end_of_record                 // end of a source file record
//
// Anything else (TN, FN, BRDA, ...) is ignored.
func parseLCOVFile(path string) (hits, total int64, err error) {
	//nolint:gosec // CLI tool; reading caller-supplied paths is the point.
	f, err := os.Open(path)
	if err != nil {
		return 0, 0, fmt.Errorf("open: %w", err)
	}
	defer f.Close() //nolint:errcheck // read-only; close errors are not actionable
	return parseLCOVReader(f)
}

// parseLCOVReader is the buffered-reader entry point so tests can
// drive the parser without touching the filesystem.
func parseLCOVReader(r io.Reader) (hits, total int64, err error) {
	scanner := bufio.NewScanner(r)
	// Same generous line-length bump as the Go parser: combined LCOV
	// reports from large C++ trees can occasionally exceed 64 KiB on
	// pathological `BRDA:` lines, and we'd rather take the memory
	// than silently miscount.
	scanner.Buffer(make([]byte, 0, 64*1024), 1024*1024)

	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if !strings.HasPrefix(line, "DA:") {
			continue
		}
		count, perr := parseDARecord(line)
		if perr != nil {
			return 0, 0, fmt.Errorf("line %d: %w", lineNo, perr)
		}
		total++
		if count > 0 {
			hits++
		}
	}
	if err := scanner.Err(); err != nil {
		return 0, 0, fmt.Errorf("scan: %w", err)
	}
	return hits, total, nil
}

// parseDARecord pulls the execution count off a `DA:<line>,<count>[,<checksum>]`
// line. The optional MD5 checksum field that geninfo emits when
// `--checksum` is enabled is tolerated and ignored.
func parseDARecord(line string) (count int64, err error) {
	rest := strings.TrimPrefix(line, "DA:")
	parts := strings.Split(rest, ",")
	const minFields = 2 // line,count (checksum optional)
	if len(parts) < minFields {
		return 0, fmt.Errorf("malformed DA record %q", line)
	}
	count, err = strconv.ParseInt(parts[1], 10, 64)
	if err != nil {
		return 0, fmt.Errorf("parse count %q in %q: %w", parts[1], line, err)
	}
	return count, nil
}
