// Command check-stale-terms scans the worktree for retired DuckDB-
// only / transpiler-shape-roadmap terminology and fails the build
// when a hit is not on the allowlist.
//
// The linter exists to keep `docs/ENGINE_POLICY.md`'s sweep
// honest: the canonical seven-route disposition vocabulary lives in
// `backend/engine/disposition.h` (and is documented in
// `docs/ENGINE_POLICY.md`), and the prose / code comments in this
// repo MUST stay aligned with it.
//
// Stale terms (case-sensitive unless noted):
//
//   - `DuckDB-only` (and `DuckDB-only-`) -- replace with
//     `local-only execution` or `local coordinator with DuckDB
//     storage`.
//   - `transpiler shape roadmap` -- replace with
//     `local-execution roadmap`.
//   - `FallbackEngine` -- drop or mark historical (the runtime
//     fallback bridge was removed; the route classifier replaces
//     it).
//   - `ReferenceImpl` -- drop or mark historical (the
//     `googlesql::reference_impl::Evaluator`-backed engine was
//     removed alongside `FallbackEngine`).
//   - `\bskiplist\b` -- replace with the matching route disposition
//     (`unsupported` for specialized families, `semantic_executor`
//     for BigQuery-exact functions, ...). The function-disposition
//     migration that owned `kSkiplist` shipped via plan 1
//     (`docs/ENGINE_POLICY.md`).
//   - `\bnot_started\b` -- replace with the matching route
//     disposition (`(planned)` annotation in SHAPE_TRACKER.md is
//     the current way to flag a pending row).
//   - `all shapes become transpiler work` (and equivalents) --
//     remove. The whole-tree-becomes-transpiler-rows framing is
//     the silent-approximation hazard `docs/ENGINE_POLICY.md`
//     replaced.
//   - `every shape lowers to DuckDB SQL` (and equivalents) --
//     remove (same reason).
//
// The allowlist file lives at `.github/lint/stale-terms-allowlist.txt`
// and is one path per line, optionally with `:N-M` suffix to allowlist
// only line range N..M of the file (so a `History` section can keep
// the historical term while the rest of the file stays linted).
//
// Exit codes:
//
//	0  no findings
//	1  findings (the report names every offending file:line)
//	2  usage / IO error
//
// The CLI runs from the repo root by convention (via
// `task lint:stale-terms`); the `--root` flag overrides it for
// tests.
package main

import (
	"bufio"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
)

const (
	defaultAllowlistRel = ".github/lint/stale-terms-allowlist.txt"

	// File-extension whitelist. The linter only scans documentation,
	// build glue, and source-comment files; the
	// `.git`, `bazel-*`, `node_modules`, and `third_party` trees
	// are skipped wholesale via skipDir.
	scanExtensions = ".md,.go,.h,.cc,.proto,.yaml,.yml,.bazel,.bzl,.txt"
)

var (
	errUsage    = errors.New("usage error")
	errFindings = errors.New("stale-term findings")
)

// staleTerm is a single banned phrase + its compiled matcher. The
// matcher is `regexp.Regexp`-backed so we can express word
// boundaries (`\bskiplist\b`) without a separate token-list pass.
type staleTerm struct {
	// label is the human-readable name we surface in findings (e.g.
	// `"DuckDB-only"`). Used verbatim in error messages.
	label string
	// re is the compiled regex pattern. Plain literal terms are
	// wrapped with `regexp.QuoteMeta`; word-boundary terms ship the
	// boundary anchors explicitly.
	re *regexp.Regexp
}

// staleTerms is the closed vocabulary the linter enforces. Adding a
// new entry here requires editing the allowlist for any historical
// section that intentionally retains the term. See the file header
// for the full per-term policy.
var staleTerms = []staleTerm{
	{label: "DuckDB-only", re: regexp.MustCompile(`DuckDB-only`)},
	{
		label: "transpiler shape roadmap",
		re:    regexp.MustCompile(`transpiler shape roadmap`),
	},
	{label: "FallbackEngine", re: regexp.MustCompile(`FallbackEngine`)},
	{label: "ReferenceImpl", re: regexp.MustCompile(`ReferenceImpl`)},
	{label: "skiplist", re: regexp.MustCompile(`\bskiplist\b`)},
	{label: "not_started", re: regexp.MustCompile(`\bnot_started\b`)},
	{
		label: "all shapes become transpiler work",
		re:    regexp.MustCompile(`all shapes become transpiler work`),
	},
	{
		label: "every shape lowers to DuckDB SQL",
		re:    regexp.MustCompile(`every shape lowers to DuckDB SQL`),
	},
}

// allowlistEntry describes one allowlisted hit. Either a whole file
// (`lo == hi == 0`) or an inclusive line range [lo, hi] within the
// file. The plan calls out a per-line-range form so a `History`
// subsection can be allowlisted without the rest of the file
// getting a free pass.
type allowlistEntry struct {
	path string
	lo   int
	hi   int
}

func main() {
	if err := run(os.Args[1:], os.Stdout, os.Stderr); err != nil {
		switch {
		case errors.Is(err, errUsage):
			os.Exit(2)
		case errors.Is(err, errFindings):
			os.Exit(1)
		default:
			_, _ = fmt.Fprintf(os.Stderr, "check-stale-terms: %v\n", err)
			os.Exit(1)
		}
	}
}

// finding is one offending file:line + the phrase that triggered
// it.
type finding struct {
	path  string
	line  int
	col   int
	text  string
	label string
}

// run is the testable entry point. It scans `root`, applies the
// allowlist, and writes the diagnostics to `stdout` / `stderr`.
//
// helper would obscure the linear "parse args, read inputs, scan,
// report" flow without simplifying any branch.
//
//nolint:funlen // top-level CLI; pulling each step into its own
func run(args []string, stdout, stderr io.Writer) error {
	fs := flag.NewFlagSet("check-stale-terms", flag.ContinueOnError)
	fs.SetOutput(stderr)
	root := fs.String("root", "", "Worktree root (defaults to current "+
		"working directory).")
	allowlistPath := fs.String("allowlist", "",
		"Path to the allowlist file (defaults to "+
			defaultAllowlistRel+" under --root).")
	if err := fs.Parse(args); err != nil {
		return errUsage
	}
	if fs.NArg() > 0 {
		_, _ = fmt.Fprintf(stderr,
			"check-stale-terms: unexpected positional args: %v\n",
			fs.Args())
		return errUsage
	}

	r, ap, err := resolvePaths(*root, *allowlistPath)
	if err != nil {
		_, _ = fmt.Fprintf(stderr, "check-stale-terms: %v\n", err)
		return errUsage
	}

	allowlist, err := loadAllowlist(ap)
	if err != nil {
		return fmt.Errorf("load allowlist %s: %w", ap, err)
	}

	findings, scanned, err := scanRoot(r)
	if err != nil {
		return fmt.Errorf("scan %s: %w", r, err)
	}
	kept := filterAllowed(findings, allowlist)

	if len(kept) == 0 {
		_, _ = fmt.Fprintf(stdout,
			"check-stale-terms: ok (%d files scanned, %d hits "+
				"all on allowlist)\n",
			scanned, len(findings)-len(kept))
		return nil
	}

	for _, f := range kept {
		_, _ = fmt.Fprintf(stderr, "%s:%d:%d: stale term %q in: %s\n",
			f.path, f.line, f.col, f.label, strings.TrimSpace(f.text))
	}
	_, _ = fmt.Fprintf(stderr,
		"check-stale-terms: %d offending hit(s) (%d allowlisted) "+
			"across %d files. Add the path to %s if the term is "+
			"deliberately historical, or rewrite using the "+
			"seven-route disposition vocabulary documented in "+
			"backend/engine/disposition.h.\n",
		len(kept), len(findings)-len(kept), scanned, ap)
	return errFindings
}

// resolvePaths fills in the defaults for `--root` / `--allowlist`.
func resolvePaths(rootFlag, allowlistFlag string) (root, allow string, err error) {
	root = rootFlag
	if root == "" {
		root, err = os.Getwd()
		if err != nil {
			return "", "", fmt.Errorf("getwd: %w", err)
		}
	}
	allow = allowlistFlag
	if allow == "" {
		allow = filepath.Join(root, filepath.FromSlash(defaultAllowlistRel))
	}
	if _, statErr := os.Stat(root); statErr != nil {
		return "", "", fmt.Errorf("--root path %q is not readable: %w",
			root, statErr)
	}
	if _, statErr := os.Stat(allow); statErr != nil {
		return "", "", fmt.Errorf("--allowlist path %q is not readable: %w",
			allow, statErr)
	}
	return root, allow, nil
}

// loadAllowlist parses the path-per-line allowlist. Lines beginning
// with `#` or only whitespace are comments. A path may carry a
// `:N-M` suffix to restrict the allowlist to lines [N..M]; without
// the suffix the whole file is allowlisted.
func loadAllowlist(path string) ([]allowlistEntry, error) {
	// gosec G304: the path comes from the --allowlist CLI flag
	// (defaults to .github/lint/stale-terms-allowlist.txt under the
	// repo root). Same posture as the rest of the in-repo lint
	// tooling.
	f, err := os.Open(path) //nolint:gosec // user-controlled lint input
	if err != nil {
		return nil, err
	}
	defer func() { _ = f.Close() }()

	var out []allowlistEntry
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 0, 64*1024), 1024*1024)
	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		entry, err := parseAllowlistLine(line)
		if err != nil {
			return nil, fmt.Errorf("line %d: %w", lineNo, err)
		}
		out = append(out, entry)
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	return out, nil
}

// parseAllowlistLine extracts an allowlistEntry from a single
// non-comment line. Forms accepted: `path/to/file.md` and
// `path/to/file.md:12-48`.
func parseAllowlistLine(line string) (allowlistEntry, error) {
	colon := strings.LastIndex(line, ":")
	if colon == -1 {
		return allowlistEntry{path: line}, nil
	}
	path := line[:colon]
	span := line[colon+1:]
	before, after, ok := strings.Cut(span, "-")
	if !ok {
		return allowlistEntry{}, fmt.Errorf(
			"%q: range form must be path:N-M, got %q", line, span)
	}
	lo, err := strconv.Atoi(before)
	if err != nil {
		return allowlistEntry{}, fmt.Errorf("%q: parse lo: %w", line, err)
	}
	hi, err := strconv.Atoi(after)
	if err != nil {
		return allowlistEntry{}, fmt.Errorf("%q: parse hi: %w", line, err)
	}
	if lo < 1 || hi < lo {
		return allowlistEntry{}, fmt.Errorf(
			"%q: range must be 1 <= lo <= hi, got %d-%d", line, lo, hi)
	}
	return allowlistEntry{path: path, lo: lo, hi: hi}, nil
}

// scanRoot walks `root`, opens each candidate file, and returns the
// raw findings (allowlist not yet applied) plus the count of files
// scanned. The walker prunes vendored / generated trees so the
// scan stays a few hundred milliseconds on a cold cache.
//
// obscure the linear flow without reducing any single decision.
//
//nolint:funlen // sequential walk + per-file scan; splitting would
func scanRoot(root string) ([]finding, int, error) {
	skipDirs := map[string]bool{
		".git":                    true,
		"bazel-bigquery-emulator": true,
		"bazel-bin":               true,
		"bazel-out":               true,
		"bazel-testlogs":          true,
		"node_modules":            true,
		"third_party":             true,
		"gen":                     true,
		"vendor":                  true,
		".cache":                  true,
	}
	exts := map[string]bool{}
	for e := range strings.SplitSeq(scanExtensions, ",") {
		exts[strings.TrimSpace(e)] = true
	}
	var findings []finding
	scanned := 0
	err := filepath.WalkDir(root, func(path string, d fs.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		base := d.Name()
		if d.IsDir() {
			if skipDirs[base] {
				return filepath.SkipDir
			}
			if strings.HasPrefix(base, "bazel-") {
				return filepath.SkipDir
			}
			return nil
		}
		// Skip the linter's own source + the allowlist file from
		// the scan: both literally embed every banned term and
		// would only ever produce self-referential findings.
		rel, _ := filepath.Rel(root, path)
		if rel == filepath.FromSlash("tools/check_stale_terms/main.go") ||
			rel == filepath.FromSlash("tools/check_stale_terms/main_test.go") ||
			rel == filepath.FromSlash(defaultAllowlistRel) {
			return nil
		}
		ext := strings.ToLower(filepath.Ext(base))
		if !exts[ext] {
			return nil
		}
		scanned++
		fileFindings, err := scanFile(path, rel)
		if err != nil {
			return err
		}
		findings = append(findings, fileFindings...)
		return nil
	})
	if err != nil {
		return nil, 0, err
	}
	sort.Slice(findings, func(i, j int) bool {
		if findings[i].path != findings[j].path {
			return findings[i].path < findings[j].path
		}
		if findings[i].line != findings[j].line {
			return findings[i].line < findings[j].line
		}
		return findings[i].label < findings[j].label
	})
	return findings, scanned, nil
}

// scanFile reads one candidate file and returns its findings. Each
// finding carries the path RELATIVE to the worktree root so the
// allowlist matches portably across check-out locations.
func scanFile(absPath, relPath string) ([]finding, error) {
	// gosec G304: absPath is produced by filepath.WalkDir from the
	// caller-supplied --root flag, same posture as
	// `tools/check_disposition_parity/main.go`.
	f, err := os.Open(absPath) //nolint:gosec // user-controlled lint input
	if err != nil {
		return nil, err
	}
	defer func() { _ = f.Close() }()

	var out []finding
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 0, 64*1024), 4*1024*1024)
	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := scanner.Text()
		for _, term := range staleTerms {
			loc := term.re.FindStringIndex(line)
			if loc == nil {
				continue
			}
			out = append(out, finding{
				path:  filepath.ToSlash(relPath),
				line:  lineNo,
				col:   loc[0] + 1,
				text:  line,
				label: term.label,
			})
		}
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	return out, nil
}

// filterAllowed drops every finding whose (path, line) is on the
// allowlist. The kept slice preserves the input ordering.
func filterAllowed(in []finding, allow []allowlistEntry) []finding {
	if len(allow) == 0 {
		return in
	}
	byPath := make(map[string][]allowlistEntry, len(allow))
	for _, e := range allow {
		byPath[filepath.ToSlash(e.path)] = append(byPath[filepath.ToSlash(e.path)], e)
	}
	var out []finding
	for _, f := range in {
		entries, ok := byPath[f.path]
		if !ok {
			out = append(out, f)
			continue
		}
		if !covers(entries, f.line) {
			out = append(out, f)
		}
	}
	return out
}

// covers reports whether any entry in `entries` covers `line`. A
// whole-file entry (lo == hi == 0) covers every line; a range entry
// covers lines [lo, hi] inclusive.
func covers(entries []allowlistEntry, line int) bool {
	for _, e := range entries {
		if e.lo == 0 && e.hi == 0 {
			return true
		}
		if line >= e.lo && line <= e.hi {
			return true
		}
	}
	return false
}
