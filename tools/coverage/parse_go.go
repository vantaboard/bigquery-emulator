package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
)

// parseGoFile opens a Go coverage profile and returns (hitStatements,
// totalStatements). The Go profile format is documented at
// https://pkg.go.dev/golang.org/x/tools/cover and consists of:
//
//   - an optional first line `mode: (set|count|atomic)`
//   - one record per covered statement block:
//     `<file>:<startLine>.<startCol>,<endLine>.<endCol> <numStmts> <count>`
//
// We sum numStmts as totalStatements and the same numStmts whenever
// count > 0 as hitStatements. Mirrors what `go tool cover -func` prints
// on its `total:` line without shelling out to it (and without a
// dependency on `golang.org/x/tools/cover`).
func parseGoFile(path string) (hits, total int64, err error) {
	//nolint:gosec // CLI tool; reading caller-supplied paths is the point.
	f, err := os.Open(path)
	if err != nil {
		return 0, 0, fmt.Errorf("open: %w", err)
	}
	defer f.Close() //nolint:errcheck // read-only; close errors are not actionable
	return parseGoReader(f)
}

// parseGoReader is the buffered-reader entry point so tests can drive
// the parser without touching the filesystem.
func parseGoReader(r io.Reader) (hits, total int64, err error) {
	scanner := bufio.NewScanner(r)
	// Go coverage profiles for very large monorepos can exceed bufio's
	// default 64 KiB line cap when a single statement spans a wide
	// generated file; lift the cap to 1 MiB so we don't silently
	// truncate.
	scanner.Buffer(make([]byte, 0, 64*1024), 1024*1024)

	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		if strings.HasPrefix(line, "mode:") {
			continue
		}
		stmts, count, perr := parseGoLine(line)
		if perr != nil {
			return 0, 0, fmt.Errorf("line %d: %w", lineNo, perr)
		}
		total += stmts
		if count > 0 {
			hits += stmts
		}
	}
	if err := scanner.Err(); err != nil {
		return 0, 0, fmt.Errorf("scan: %w", err)
	}
	return hits, total, nil
}

// parseGoLine splits the trailing two numeric fields off a coverage
// record and returns (numStatements, count). The leading
// `file:start.col,end.col` slug is ignored because the per-file
// breakdown is only relevant for `go tool cover -html`, which the
// publishing workflow runs separately.
func parseGoLine(line string) (stmts, count int64, err error) {
	fields := strings.Fields(line)
	const requiredFields = 3
	if len(fields) < requiredFields {
		return 0, 0, fmt.Errorf("unexpected field count %d in %q", len(fields), line)
	}
	stmts, err = strconv.ParseInt(fields[len(fields)-2], 10, 64)
	if err != nil {
		return 0, 0, fmt.Errorf("parse stmts %q: %w", fields[len(fields)-2], err)
	}
	count, err = strconv.ParseInt(fields[len(fields)-1], 10, 64)
	if err != nil {
		return 0, 0, fmt.Errorf("parse count %q: %w", fields[len(fields)-1], err)
	}
	return stmts, count, nil
}
