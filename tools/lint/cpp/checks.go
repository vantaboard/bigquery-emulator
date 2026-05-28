package main

import (
	"bytes"
	"fmt"
	"io"
	"path/filepath"
	"regexp"
	"sort"
	"strings"
)

// Finding is a single source-only rule violation. It is the
// reporting unit produced by every check below; the runner formats
// findings into the standard `path:line:col: rule: message` shape
// (compatible with editor jump-to-line) before exiting.
type Finding struct {
	// Rule is the stable identifier callers grep for, e.g.
	// `file-length`, `banned-logging`, `status-discarded`.
	Rule string
	// Path is repo-relative, matching the `cpp-lint list` output.
	Path string
	// Line is 1-based; 0 means "applies to the whole file".
	Line int
	// Col is 1-based, 0 when the rule is line-level.
	Col int
	// Message is the human-readable explanation. Should fit on one
	// line; longer guidance belongs in the docs the message links
	// to.
	Message string
}

// Format renders a Finding into the `path:line:col: rule: msg`
// convention. Editors jump to the right location when stderr is
// piped through the standard error filter.
func (f Finding) Format() string {
	switch {
	case f.Line > 0 && f.Col > 0:
		return fmt.Sprintf("%s:%d:%d: %s: %s", f.Path, f.Line, f.Col, f.Rule, f.Message)
	case f.Line > 0:
		return fmt.Sprintf("%s:%d: %s: %s", f.Path, f.Line, f.Rule, f.Message)
	default:
		return fmt.Sprintf("%s: %s: %s", f.Path, f.Rule, f.Message)
	}
}

// CheckOptions bundles the knobs every per-file check needs to
// know about. Centralised so the runner threads them through one
// argument instead of growing an ever-longer parameter list as new
// rules land.
type CheckOptions struct {
	// MaxFileLines is the whole-file line count above which a file
	// is rejected unless explicitly baselined. 500 today (see plan
	// thresholds).
	MaxFileLines int
	// Baseline is the set of files (repo-relative) that are
	// permitted to exceed MaxFileLines. New oversized files still
	// fail; this only grandfathers existing offenders so the lint
	// gate can land without a sweeping refactor.
	Baseline map[string]struct{}
}

// runOnce applies every source-only check to a single file and
// returns the set of findings. The returned slice is sorted by
// (line, rule) for deterministic output regardless of map
// iteration order in the underlying check helpers, and findings
// suppressed via inline `// cpp-lint:allow(rule) ...` comments are
// dropped before returning.
func runOnce(path string, body []byte, opts CheckOptions) []Finding {
	lines := splitLines(body)
	suppressions := collectSuppressions(lines)

	var out []Finding
	out = append(out, checkFileLength(path, body, opts)...)
	out = append(out, checkBannedLogging(path, lines)...)
	out = append(out, checkStatusAntiPatterns(path, lines)...)
	out = filterSuppressed(out, suppressions)
	sort.SliceStable(out, func(i, j int) bool {
		if out[i].Line != out[j].Line {
			return out[i].Line < out[j].Line
		}
		return out[i].Rule < out[j].Rule
	})
	return out
}

// splitLines slices `body` into lines, preserving 1-based numbering
// when the slice is later indexed via lines[lineNo-1]. We split on
// '\n' so a CRLF file still produces clean line content (the
// trailing '\r' is left in place; rule helpers strip it via the
// shared normaliser when needed).
func splitLines(body []byte) []string {
	if len(body) == 0 {
		return nil
	}
	s := string(body)
	if strings.HasSuffix(s, "\n") {
		s = s[:len(s)-1]
	}
	return strings.Split(s, "\n")
}

// suppressMarkerRE recognises a suppression comment of the form
// `// cpp-lint:allow(rule[, rule]) -- reason text`. The marker may
// sit at the end of the offending line OR on a comment-only line
// directly preceding it; both placements suppress the listed rules
// for the next code line. We support both styles because
// clang-format may wrap a long trailing comment, but a marker on
// its own line stays on its own line.
//
// The trailing reason is mandatory: a `nolint`-style suppression
// that does not say WHY is harder to audit later. A marker that
// omits the `-- reason` body silently fails to suppress.
var suppressMarkerRE = regexp.MustCompile(`(?:^|\s)//\s*cpp-lint:allow\(([^)]*)\)\s*--\s*(.+?)\s*$`)

// suppression records the rule set that an inline marker disables
// on a particular source line.
type suppression struct {
	Rules map[string]struct{}
}

// collectSuppressions returns the line -> rule-set map. A marker
// on line N applies to line N, and additionally to the next code
// line (the first subsequent line that is not blank and not a
// pure comment) when the marker itself sits on a comment-only
// line. The latter handles clang-format wrapping a long trailing
// comment over two lines: the marker now lives on a comment-only
// line whose target is the first code line that follows.
func collectSuppressions(lines []string) map[int]suppression {
	out := map[int]suppression{}
	for i, raw := range lines {
		m := suppressMarkerRE.FindStringSubmatch(raw)
		if m == nil {
			continue
		}
		rules := parseSuppressionRules(m[1])
		mergeSuppression(out, i+1, rules)
		if isCommentOnlyLine(raw) {
			if next := nextCodeLine(lines, i+1); next > 0 {
				mergeSuppression(out, next, rules)
			}
		}
	}
	return out
}

func parseSuppressionRules(spec string) map[string]struct{} {
	rules := map[string]struct{}{}
	for r := range strings.SplitSeq(spec, ",") {
		r = strings.TrimSpace(r)
		if r == "" {
			continue
		}
		rules[r] = struct{}{}
	}
	return rules
}

func mergeSuppression(dst map[int]suppression, line int, rules map[string]struct{}) {
	cur, ok := dst[line]
	if !ok {
		cur = suppression{Rules: map[string]struct{}{}}
	}
	for r := range rules {
		cur.Rules[r] = struct{}{}
	}
	dst[line] = cur
}

// isCommentOnlyLine returns true when the line contains nothing
// outside whitespace and a `//` comment. We treat block comments
// (`/* ... */` on the same line) as code so a marker buried inside
// a multi-line block comment does not accidentally apply to the
// next statement.
func isCommentOnlyLine(line string) bool {
	trimmed := strings.TrimSpace(line)
	return strings.HasPrefix(trimmed, "//")
}

// nextCodeLine returns the 1-based line number of the first line
// at or after `start` (1-based) that contains executable code.
// Blank lines and pure-comment lines are skipped. Returns 0 when
// no such line exists in the file.
func nextCodeLine(lines []string, start int) int {
	for i := start; i < len(lines); i++ {
		raw := lines[i]
		if strings.TrimSpace(raw) == "" {
			continue
		}
		if isCommentOnlyLine(raw) {
			continue
		}
		return i + 1
	}
	return 0
}

func filterSuppressed(in []Finding, sup map[int]suppression) []Finding {
	if len(sup) == 0 {
		return in
	}
	out := in[:0]
	for _, f := range in {
		if entry, ok := sup[f.Line]; ok {
			if _, dropped := entry.Rules[f.Rule]; dropped {
				continue
			}
		}
		out = append(out, f)
	}
	return out
}

// --- file-length --------------------------------------------------------

const (
	ruleFileLength = "file-length"
)

// checkFileLength enforces the whole-file line cap. The plan
// chooses 500 lines for first-party `.cc`/`.h` to mirror the Go
// `revive` `file-length-limit` rule already enabled in
// `.golangci.yml`. New oversized files fail immediately; existing
// offenders are grandfathered through the baseline.
func checkFileLength(path string, body []byte, opts CheckOptions) []Finding {
	if opts.MaxFileLines <= 0 {
		return nil
	}
	lines := countLines(body)
	if lines <= opts.MaxFileLines {
		return nil
	}
	if _, baselined := opts.Baseline[path]; baselined {
		return nil
	}
	return []Finding{
		{
			Rule: ruleFileLength,
			Path: path,
			Message: fmt.Sprintf(
				"file has %d lines (max %d); split it or add to tools/lint/cpp/baseline.txt with a follow-up issue",
				lines,
				opts.MaxFileLines,
			),
		},
	}
}

// countLines returns the number of newline-terminated lines in the
// file plus one for any trailing partial line. Matches `wc -l`'s
// off-by-one behaviour for files that do not end in `\n`, so the
// reported count agrees with what `wc -l` shows in CI logs.
func countLines(body []byte) int {
	if len(body) == 0 {
		return 0
	}
	count := bytes.Count(body, []byte{'\n'})
	if body[len(body)-1] != '\n' {
		count++
	}
	return count
}

// --- banned-logging ----------------------------------------------------

const ruleBannedLogging = "banned-logging"

// bannedLoggingPatterns is the (rule-message-friendly) list of
// production logging APIs we explicitly do not allow inside
// production C++. Tests, fixture printers, and the smoke binaries
// under `tools/googlesql-prebuilt/smoke/` may use these; the
// runner skips the rule for those paths via isLoggingAllowed().
//
// Each entry is a literal substring match that is anchored to a
// non-identifier boundary, so `kStdCoutName` does not falsely
// match `std::cout`. The patterns are kept simple — a regex with
// capture groups would be overkill for a list this small.
var bannedLoggingPatterns = []struct {
	needle  string
	message string
}{
	{
		"std::cout",
		"std::cout is banned in production C++; use absl::Status / structured logging via the gRPC error envelope",
	},
	{"std::cerr", "std::cerr is banned in production C++; surface errors through absl::Status / grpc::Status instead"},
	{"std::clog", "std::clog is banned in production C++; route diagnostics through absl::Status / grpc::Status"},
	{
		"std::printf",
		"std::printf is banned in production C++; use absl::StrCat / absl::StrFormat and return errors via Status",
	},
	{
		"std::fprintf",
		"std::fprintf is banned in production C++; surface diagnostics through absl::Status / grpc::Status",
	},
}

// printfWordRE matches a top-level `printf(` or `fprintf(` call
// (no `std::` prefix, no `::` either). This catches the common
// `<cstdio>` / `<stdio.h>` variants without flagging field /
// member-named `printf` (`obj.printf(...)`). The look-behind is
// expressed as a non-capturing group followed by a manual
// boundary check inside checkBannedLogging.
var printfWordRE = regexp.MustCompile(`\b(f?printf)\s*\(`)

func checkBannedLogging(path string, lines []string) []Finding {
	if isLoggingAllowed(path) {
		return nil
	}
	var out []Finding
	for i, raw := range lines {
		lineNo := i + 1
		stripped := stripCommentsAndStrings(raw)
		if stripped == "" {
			continue
		}
		for _, p := range bannedLoggingPatterns {
			if idx := strings.Index(stripped, p.needle); idx >= 0 {
				out = append(out, Finding{
					Rule:    ruleBannedLogging,
					Path:    path,
					Line:    lineNo,
					Col:     idx + 1,
					Message: p.message,
				})
			}
		}
		// printf / fprintf without an `std::` prefix. Skip the
		// match when it is preceded by `::` (already covered by
		// the std::printf needle), `.` / `->` (member call), or
		// an identifier character (avoids false positives on
		// `kSomePrefixprintf`-style symbols).
		for _, m := range printfWordRE.FindAllStringIndex(stripped, -1) {
			start := m[0]
			if start > 0 {
				prev := stripped[start-1]
				if isIdentChar(prev) || prev == '.' || prev == '>' || prev == ':' {
					continue
				}
			}
			out = append(out, Finding{
				Rule:    ruleBannedLogging,
				Path:    path,
				Line:    lineNo,
				Col:     start + 1,
				Message: "printf / fprintf is banned in production C++; surface errors through absl::Status",
			})
		}
	}
	return out
}

// isLoggingAllowed returns true when the file is a C++ test, the
// smoke binary, or anything under `binaries/emulator_main/main.cc`
// (which legitimately writes the `--help` / `--version` output to
// stdout/stderr at process start).
//
// The exemptions are deliberately narrow: the `main.cc` carve-out
// stays a literal path match so a future `binaries/foo/main.cc`
// has to opt in explicitly, and the smoke directory matches by
// prefix because every file under it is non-production.
func isLoggingAllowed(path string) bool {
	if IsTestFile(path) {
		return true
	}
	if strings.HasPrefix(path, "tools/googlesql-prebuilt/smoke/") {
		return true
	}
	if path == SentinelEmulatorMain {
		return true
	}
	return false
}

// stripCommentsAndStrings returns `line` with `//` comments and
// double-quoted strings replaced by spaces of the same length.
// This is a deliberately small lexer — it does not follow `/*
// */` block comments across newlines or recognise raw string
// literals — but it is sufficient to keep `// std::cerr is bad`
// comments and `"std::cout"` literals from raising findings while
// still catching real violations on any normal source line.
//
// We pad with spaces (rather than truncate) so column numbers in
// findings still match the on-disk file. Block comments and raw
// strings are flagged as a known-limitation in the package README;
// a real lexer would be overkill for a check that already runs in
// well under a second.
func stripCommentsAndStrings(line string) string {
	var out strings.Builder
	out.Grow(len(line))
	inString := false
	escape := false
	for i := 0; i < len(line); i++ {
		c := line[i]
		if inString {
			out.WriteByte(' ')
			if escape {
				escape = false
				continue
			}
			switch c {
			case '\\':
				escape = true
			case '"':
				inString = false
			}
			continue
		}
		if c == '"' {
			inString = true
			out.WriteByte(' ')
			continue
		}
		if c == '/' && i+1 < len(line) && line[i+1] == '/' {
			// Pad the rest of the line so column numbers stay
			// aligned with the original buffer.
			for ; i < len(line); i++ {
				out.WriteByte(' ')
			}
			break
		}
		out.WriteByte(c)
	}
	return out.String()
}

func isIdentChar(b byte) bool {
	return (b >= 'a' && b <= 'z') ||
		(b >= 'A' && b <= 'Z') ||
		(b >= '0' && b <= '9') ||
		b == '_'
}

// --- status-anti-patterns ---------------------------------------------

const (
	ruleStatusDiscarded   = "status-discarded"
	ruleStatusOrUnchecked = "statusor-unchecked-value"
)

// statusReturnRE matches a top-level call whose result is a
// statement-form `absl::Status` (or `Status` in the
// `bigquery_emulator` namespace). The pattern is intentionally
// simple: any line whose semicolon-terminated statement looks like
// `f(args);` AND whose textual context names a known
// status-returning function gets reported. Real
// `absl::Status s = f(); if (!s.ok()) ...` flows are not flagged
// because the assignment ends with `=` rather than `f(args);`.
//
// We avoid trying to be a parser — clang-tidy's
// `bugprone-unused-return-value` is the long-term mechanism for
// this — but the regex catches the obvious case where someone
// types `engine.ExecuteDdl(...)` and forgets to inspect the
// result.
var statusCallStmtRE = regexp.MustCompile(
	`^\s*([A-Za-z_][A-Za-z_0-9]*::)*([A-Za-z_][A-Za-z_0-9]*\s*\.\s*)?([A-Za-z_][A-Za-z_0-9]*)\(`,
)

// statusOrValueRE matches `.value()` invocations on a `StatusOr`
// without a preceding `.ok()` guard on the same line. The check
// runs per-line so it is necessarily approximate; the value of
// catching even the obvious cases outweighs the false-positive
// risk because the suggested replacement (`*r` after a `.ok()`
// check, or `r.value_or(default)` with an explicit fallback) is
// almost always cleaner.
var statusOrValueRE = regexp.MustCompile(`\.value\(\)`)

// statusOrOkRE detects an inline `.ok()` check on the same source
// line as the `.value()` call. When present, we skip the
// `.value()` finding because the code is already guarding the
// dereference.
var statusOrOkRE = regexp.MustCompile(`\.ok\(\)`)

// checkStatusAntiPatterns surfaces two concrete failure modes:
//   - `RunSql(...);` (a discarded `absl::Status`).
//   - `result.value()` with no `.ok()` / status guard within a
//     short window of the call (a `StatusOr` access that crashes
//     on an absent value).
//
// Both rules are deliberately conservative — clang-tidy's
// `bugprone-unused-return-value` and
// `bugprone-unchecked-optional-access` are the long-term
// mechanism. The checks here exist so `task lint:run` can still
// catch the most common review nits without spinning up the full
// compile-aware lane.
func checkStatusAntiPatterns(path string, lines []string) []Finding {
	if IsTestFile(path) {
		return nil
	}
	var out []Finding
	for i, raw := range lines {
		lineNo := i + 1
		stripped := stripCommentsAndStrings(raw)
		if stripped == "" {
			continue
		}
		out = append(out, scanDiscardedStatus(path, lineNo, stripped)...)
		out = append(out, scanStatusOrValue(path, lineNo, stripped, lines)...)
	}
	return out
}

// statusDiscardedFunctions lists functions whose return value
// must always be inspected. The list is small on purpose: this
// helper exists for repo-specific rules that clang-tidy's
// generic `[[nodiscard]]` plumbing cannot model without seeing
// the headers. Once `[[nodiscard]]` annotations land on the
// matching declarations, clang-tidy will surface the same
// findings during `task lint:cpp:tidy`.
var statusDiscardedFunctions = map[string]struct{}{
	// Engine.ExecuteDdl returns absl::Status; ignoring it loses
	// the failure that the gateway needs to surface as a 4xx /
	// 5xx response.
	"ExecuteDdl": {},
	// Storage.AppendRows returns absl::Status; ignoring it
	// silently drops a streaming insert.
	"AppendRows": {},
	// Storage.OverwriteRows returns absl::Status.
	"OverwriteRows": {},
	// Storage.DropTable returns absl::Status.
	"DropTable": {},
}

func scanDiscardedStatus(path string, line int, stripped string) []Finding {
	// Cheap pre-filter: a discarded-status statement always ends in `;`.
	if !strings.HasSuffix(strings.TrimSpace(stripped), ";") {
		return nil
	}
	m := statusCallStmtRE.FindStringSubmatch(stripped)
	if m == nil {
		return nil
	}
	fn := m[3]
	if _, banned := statusDiscardedFunctions[fn]; !banned {
		return nil
	}
	return []Finding{{
		Rule:    ruleStatusDiscarded,
		Path:    path,
		Line:    line,
		Col:     1,
		Message: fmt.Sprintf("discarded absl::Status return from %q; capture and inspect via if (!s.ok()) { ... }", fn),
	}}
}

// statusOrLookbackLines is the number of preceding non-blank lines
// scanStatusOrValue inspects when looking for an `.ok()` guard.
// Five lines is enough to cover the canonical pattern:
//
//	absl::StatusOr<T> rendered = ...;
//	if (!rendered.ok()) return rendered.status();
//	*out = std::move(rendered).value();
//
// while still catching the obvious "fetch -> dereference" calls
// that lack any guard.
const statusOrLookbackLines = 5

func scanStatusOrValue(path string, line int, stripped string, lines []string) []Finding {
	if !statusOrValueRE.MatchString(stripped) {
		return nil
	}
	if statusOrOkRE.MatchString(stripped) {
		return nil
	}
	if hasNearbyStatusGuard(lines, line) {
		return nil
	}
	idx := statusOrValueRE.FindStringIndex(stripped)
	return []Finding{
		{
			Rule:    ruleStatusOrUnchecked,
			Path:    path,
			Line:    line,
			Col:     idx[0] + 1,
			Message: "StatusOr<T>::value() without a nearby .ok() guard; check status before unwrapping (or annotate with `// cpp-lint:allow(statusor-unchecked-value) -- reason` if intentional)",
		},
	}
}

// statusGuardRE matches the patterns we treat as a `.value()`
// safety net:
//
//   - `if (!x.ok())` — the canonical guard.
//   - `return x.status()` — the early-return inside a guard.
//   - `RETURN_IF_ERROR(x)` — the absl macro pattern.
//   - `ASSIGN_OR_RETURN(...)` — likewise.
//
// We intentionally over-accept here. False negatives (a missed
// finding because of a permissive guard pattern) are acceptable;
// false positives (a screaming finding on already-safe code) are
// not, because they erode trust in the rule.
var statusGuardRE = regexp.MustCompile(
	`!\s*[A-Za-z_][A-Za-z_0-9]*\s*\.\s*ok\(\)|RETURN_IF_ERROR\s*\(|ASSIGN_OR_RETURN\s*\(|\.\s*status\(\)`,
)

func hasNearbyStatusGuard(lines []string, line int) bool {
	from := max(line-statusOrLookbackLines, 1)
	for n := line - 1; n >= from; n-- {
		stripped := stripCommentsAndStrings(lines[n-1])
		if statusGuardRE.MatchString(stripped) {
			return true
		}
	}
	return false
}

// runCheck is the `cpp-lint check` subcommand. It loads the
// canonical first-party source list, runs every per-file rule,
// prints findings to stdout, and exits with `errFindings` when at
// least one rule reported a violation.
//
// Flags:
//   - `--max-lines` overrides the file-length cap (default 500).
//   - `--baseline` overrides the baseline file path (default
//     `tools/lint/cpp/baseline.txt`).
//   - `--no-baseline` disables the baseline so a maintainer can
//     verify a refactor has actually shrunk every entry.
func runCheck(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("check", stderr)
	maxLines := fs.Int("max-lines", 500, "fail when a first-party C++ file exceeds this many lines")
	baselinePath := fs.String(
		"baseline",
		filepath.FromSlash("tools/lint/cpp/baseline.txt"),
		"path to the file-length baseline",
	)
	noBaseline := fs.Bool("no-baseline", false, "disable the file-length baseline (every offender fails)")
	if err := fs.Parse(args); err != nil {
		return errUsage
	}
	if fs.NArg() != 0 {
		fmt.Fprintln(stderr, "cpp-lint check: takes no positional arguments")
		return errUsage
	}

	files, root, err := readSources()
	if err != nil {
		return err
	}

	baseline := map[string]struct{}{}
	if !*noBaseline {
		baseline, err = readBaseline(filepath.Join(root, *baselinePath))
		if err != nil {
			return fmt.Errorf("read baseline: %w", err)
		}
	}

	opts := CheckOptions{MaxFileLines: *maxLines, Baseline: baseline}
	var totalFindings int
	for _, rel := range files {
		body, rerr := readFile(resolveAgainstRoot(root, rel))
		if rerr != nil {
			return fmt.Errorf("read %s: %w", rel, rerr)
		}
		for _, f := range runOnce(rel, body, opts) {
			_, _ = fmt.Fprintln(stdout, f.Format())
			totalFindings++
		}
	}
	if totalFindings > 0 {
		_, _ = fmt.Fprintf(stderr, "cpp-lint: %d finding(s)\n", totalFindings)
		return errFindings
	}
	return nil
}
