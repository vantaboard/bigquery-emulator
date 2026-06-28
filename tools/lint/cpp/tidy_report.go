package main

import (
	"encoding/csv"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
)

// tidyFinding is one clang-tidy diagnostic extracted from a batch log.
type tidyFinding struct {
	BlockFile       string // first-party file block being linted
	File            string // diagnostic path (normalized when possible)
	Line            int
	Column          int
	Severity        string
	Check           string
	Symbol          string
	Message         string
	ComplexityScore int // 0 when not a complexity finding
}

const (
	fileKindTest    = "test"
	fileKindFixture = "fixture"
	dispFix         = "fix"
)

var (
	reFileBlock  = regexp.MustCompile(`^========== (.+) ==========$`)
	reFailed     = regexp.MustCompile(`^FAILED: (.+)$`)
	reProcessing = regexp.MustCompile(`^Error while processing (.+)\.$`)
	// path:line:col: severity: message [check-name,...]
	reDiagnostic = regexp.MustCompile(`^(.+?):(\d+):(\d+): (warning|error|note): (.+?)(?: \[(.+?)\])?$`)
	reComplexity = regexp.MustCompile(`function '([^']+)' has cognitive complexity of (\d+)`)
)

func runParseTidyLog(args []string, stdout, stderr io.Writer) error {
	fs := flagSet(cmdParseTidyLog, stderr)
	logPath := fs.String("log", "lint-cpp-tidy.log", "clang-tidy batch log path")
	csvPath := fs.String("csv", "lint-cpp-tidy.csv", "CSV output path")
	mdPath := fs.String("markdown", "docs/dev/cpp-lint-tidy-triage.md", "triage markdown output path")
	if err := fs.Parse(args); err != nil {
		return errUsage
	}

	f, err := os.Open(*logPath)
	if err != nil {
		return fmt.Errorf("open log: %w", err)
	}
	defer func() { _ = f.Close() }()

	findings, failedFiles, totalBlocks := parseTidyLog(f)
	if err := writeTidyCSV(*csvPath, findings); err != nil {
		return err
	}
	if err := writeTriageMarkdown(*mdPath, findings, failedFiles, totalBlocks); err != nil {
		return err
	}
	printTidySummary(stdout, findings, failedFiles, totalBlocks, *csvPath, *mdPath)
	return nil
}

func parseTidyLog(r io.Reader) ([]tidyFinding, map[string]struct{}, int) {
	content, err := io.ReadAll(r)
	if err != nil {
		return nil, nil, 0
	}
	lines := strings.Split(string(content), "\n")

	failedFiles := make(map[string]struct{})
	totalBlocks := 0
	for _, line := range lines {
		if m := reFileBlock.FindStringSubmatch(line); m != nil {
			totalBlocks++
			continue
		}
		if m := reFailed.FindStringSubmatch(line); m != nil {
			failedFiles[strings.TrimSpace(m[1])] = struct{}{}
		}
	}

	var findings []tidyFinding
	lastFailed := -1
	for i, line := range lines {
		m := reFailed.FindStringSubmatch(line)
		if m == nil {
			continue
		}
		failedPath := strings.TrimSpace(m[1])
		start := lastFailed + 1
		lastFailed = i
		findings = append(findings, findingsForFailure(lines[start:i+1], failedPath)...)
	}
	return findings, failedFiles, totalBlocks
}

// findingsForFailure attributes diagnostics in a FAILED window onto the failed TU.
func findingsForFailure(window []string, failedPath string) []tidyFinding {
	var out []tidyFinding
	processingTU := failedPath
	windowMentionsFailed := false
	for _, line := range window {
		if strings.Contains(line, failedPath+":") {
			windowMentionsFailed = true
		}
	}
	for _, line := range window {
		if m := reProcessing.FindStringSubmatch(line); m != nil {
			processingTU = normalizeTidyPath(m[1], failedPath)
		}
		m := reDiagnostic.FindStringSubmatch(line)
		if m == nil {
			continue
		}
		severity := m[4]
		if severity == "note" {
			continue
		}
		checkRaw := m[6]
		if checkRaw == "" {
			continue
		}
		check := strings.Split(checkRaw, ",")[0]
		check = strings.TrimPrefix(check, "-warnings-as-errors")

		rawPath := m[1]
		file := normalizeTidyPath(rawPath, processingTU)
		switch {
		case file == failedPath, strings.HasPrefix(line, failedPath+":"):
			// direct hit
		case isExternalTidyPath(rawPath) && windowMentionsFailed:
			file = failedPath
		case processingTU == failedPath:
			// e.g. missing-header errors while linting a header TU
		default:
			continue
		}

		lineNum, _ := strconv.Atoi(m[2])
		colNum, _ := strconv.Atoi(m[3])
		msg := m[5]
		symbol := ""
		complexity := 0
		if cm := reComplexity.FindStringSubmatch(msg); cm != nil {
			symbol = cm[1]
			complexity, _ = strconv.Atoi(cm[2])
		}
		out = append(out, tidyFinding{
			BlockFile:       failedPath,
			File:            file,
			Line:            lineNum,
			Column:          colNum,
			Severity:        severity,
			Check:           check,
			Symbol:          symbol,
			Message:         msg,
			ComplexityScore: complexity,
		})
	}
	return dedupeFindings(out)
}

func isExternalTidyPath(raw string) bool {
	return strings.HasPrefix(raw, "bazel-out/") ||
		strings.HasPrefix(raw, "external/") ||
		strings.Contains(raw, "/external/googlesql")
}

func dedupeFindings(in []tidyFinding) []tidyFinding {
	seen := make(map[string]struct{})
	var out []tidyFinding
	for _, f := range in {
		key := fmt.Sprintf("%s:%d:%d:%s:%s", f.File, f.Line, f.Column, f.Check, f.Message)
		if _, ok := seen[key]; ok {
			continue
		}
		seen[key] = struct{}{}
		out = append(out, f)
	}
	return out
}

// normalizeTidyPath maps clang-tidy diagnostic paths onto first-party
// repo-relative paths. External / bazel-out paths are attributed to the
// TU block file so triage stays one row per `FAILED:` entry.
func normalizeTidyPath(raw, blockFile string) string {
	path := strings.TrimSpace(raw)
	path = strings.TrimPrefix(path, "./")
	if strings.HasPrefix(path, "/") {
		if idx := strings.Index(path, "/backend/"); idx >= 0 {
			return strings.TrimPrefix(path[idx+1:], "/")
		}
		const marker = "bigquery-emulator/"
		if _, after, ok := strings.Cut(path, marker); ok {
			return after
		}
		return blockFile
	}
	if strings.HasPrefix(path, "external/") || strings.HasPrefix(path, "bazel-out/") {
		return blockFile
	}
	return path
}

func writeTidyCSV(path string, findings []tidyFinding) error {
	//nolint:gosec // Output path comes from task wrapper defaults or explicit flags.
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil && !os.IsExist(err) {
		// csv at repo root has no dir
		if filepath.Dir(path) != "." {
			return err
		}
	}
	//nolint:gosec // Output path comes from task wrapper defaults or explicit flags.
	out, err := os.Create(path)
	if err != nil {
		return fmt.Errorf("create csv: %w", err)
	}
	defer func() { _ = out.Close() }()

	w := csv.NewWriter(out)
	if err := w.Write([]string{
		"block_file", "file", "line", "column", "severity", "check", "symbol", "message", "complexity_score",
	}); err != nil {
		return err
	}
	for _, f := range findings {
		if err := w.Write([]string{
			f.BlockFile,
			f.File,
			strconv.Itoa(f.Line),
			strconv.Itoa(f.Column),
			f.Severity,
			f.Check,
			f.Symbol,
			f.Message,
			strconv.Itoa(f.ComplexityScore),
		}); err != nil {
			return err
		}
	}
	w.Flush()
	return w.Error()
}
