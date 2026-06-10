package runner

import (
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"
)

// TestKnownRouteNamesMatchesDispositionEnum pins the Go-side
// canonical vocabulary against `backend/engine/disposition.h`. The
// C++ enum is the source of truth: if a Disposition entry lands
// without a matching `Route*` const here, the conformance harness
// silently fails to validate the new route name in
// `expected.route` and you get unbounded drift. This test catches
// the second half of that drift (Go-side missing); the first half
// (C++ rejecting an unknown string) is enforced by
// `tools/check_disposition_parity`.
func TestKnownRouteNamesMatchesDispositionEnum(t *testing.T) {
	want := []string{
		RouteDuckDBNative,
		RouteDuckDBRewrite,
		RouteDuckDBUDF,
		RouteSemanticExecutor,
		RouteControlOp,
		RouteLocalStub,
		RouteUnsupported,
	}
	got := KnownRouteNames()
	if len(got) != len(want) {
		t.Fatalf("KnownRouteNames=%v, want %v", got, want)
	}
	for i, w := range want {
		if got[i] != w {
			t.Fatalf("KnownRouteNames[%d]=%q, want %q", i, got[i], w)
		}
	}
}

// TestKnownRouteNamesMatchesDispositionHeader is a build-time
// parity check between the Go and C++ route vocabularies: parse
// `backend/engine/disposition.h` and assert the resulting set of
// `kFoo` enum entries (lowercased + snake-cased) matches the Go-side
// `KnownRouteNames()` letter-for-letter.
//
// This catches the "C++ enum gained a new entry but the Go-side
// const list was never updated" regression. The cheaper sibling test
// `TestKnownRouteNamesMatchesDispositionEnum` pins the explicit list
// against itself, but does NOT detect that scenario because the C++
// header is not in its inputs.
//
// The C++ side has its own parity gate
// (`tools/check_disposition_parity` covers YAML <-> SHAPE_TRACKER);
// this test closes the third edge of the triangle (Go <-> C++).
func TestKnownRouteNamesMatchesDispositionHeader(t *testing.T) {
	root, err := repoRootForRouteTest()
	if err != nil {
		t.Fatalf("repoRoot: %v", err)
	}
	hdrPath := filepath.Join(root, "backend", "engine", "disposition.h")
	hdr, err := os.ReadFile(hdrPath) //nolint:gosec // test input fixed by repoRoot()
	if err != nil {
		t.Fatalf("read %s: %v", hdrPath, err)
	}
	parsed, err := parseDispositionEnumValues(string(hdr))
	if err != nil {
		t.Fatalf("parse disposition.h: %v", err)
	}
	got := KnownRouteNames()
	if len(parsed) != len(got) {
		t.Fatalf("parsed %d enum entries from disposition.h (%v) but "+
			"KnownRouteNames returned %d (%v)",
			len(parsed), parsed, len(got), got)
	}
	// Compare as ordered lists: the C++ enum declaration order is
	// the priority order (see `route_classifier.cc`'s priority
	// table) and `knownRoutes` declares the same order, so any
	// disagreement at index i is a real ordering bug.
	for i, w := range parsed {
		if got[i] != w {
			t.Fatalf("disposition.h index %d => %q but KnownRouteNames[%d] = %q "+
				"(parsed=%v, knownRoutes=%v)",
				i, w, i, got[i], parsed, got)
		}
	}
}

// dispositionEnumRe extracts the body of `enum class Disposition {
// ... };` from `disposition.h`. The matcher tolerates the canonical
// `enum class Disposition {` form (the upstream header) but stops at
// the closing brace before `;` so trailing `inline constexpr int
// kDispositionCount = 7;` lines do not contaminate the match.
var dispositionEnumRe = regexp.MustCompile(
	`(?s)enum\s+class\s+Disposition\s*\{([^}]*)\}`)

// kEntryRe matches a single `kFoo` enum entry, optionally followed
// by `,`. The leading `^` is line-anchored against the multiline
// block so a stray `kFoo` inside a comment does not match.
var kEntryRe = regexp.MustCompile(`(?m)^\s*(k[A-Za-z0-9]+)\s*(?:,|$)`)

// parseDispositionEnumValues returns the enum entries (in source
// order, lowercased + snake-cased) declared inside `enum class
// Disposition` in `src`. Comment lines starting with `//` are
// skipped before the entry-matcher runs.
func parseDispositionEnumValues(src string) ([]string, error) {
	body := dispositionEnumRe.FindStringSubmatch(src)
	if body == nil {
		return nil, errMissingDispositionEnum
	}
	// Strip per-line `//` comments so a doc-comment that re-says
	// `kFoo` inside the enum body cannot fool the matcher.
	lines := strings.Split(body[1], "\n")
	for i, line := range lines {
		if before, _, ok := strings.Cut(line, "//"); ok {
			lines[i] = before
		}
	}
	cleaned := strings.Join(lines, "\n")
	matches := kEntryRe.FindAllStringSubmatch(cleaned, -1)
	if len(matches) == 0 {
		return nil, errEmptyDispositionEnum
	}
	out := make([]string, 0, len(matches))
	for _, m := range matches {
		out = append(out, dispositionKToSnake(m[1]))
	}
	return out, nil
}

// dispositionKToSnake converts `kDuckdbNative` -> `duckdb_native`.
// The C++ side uses CamelCase entries with a leading `k`; the
// canonical lowercase-snake-case spelling lives in
// `disposition.cc::DispositionToString` and Mirror it here so a
// drift between the enum names and the canonical strings fails
// this test.
func dispositionKToSnake(name string) string {
	if !strings.HasPrefix(name, "k") {
		return name
	}
	body := name[1:]
	var b strings.Builder
	for i, r := range body {
		if i > 0 && r >= 'A' && r <= 'Z' {
			b.WriteByte('_')
		}
		if r >= 'A' && r <= 'Z' {
			b.WriteRune(r - 'A' + 'a')
		} else {
			b.WriteRune(r)
		}
	}
	// Special case: the C++ entries follow `kDuckdbXxx` (single
	// hump) so the naive split lands on `duckdb_xxx` which is
	// already correct. `kSemanticExecutor` -> `semantic_executor`,
	// `kControlOp` -> `control_op`, `kLocalStub` -> `local_stub`,
	// `kUnsupported` -> `unsupported`. Re-flatten any
	// `duckdb_native` accidentally split into `duck_db_native`.
	out := b.String()
	out = strings.ReplaceAll(out, "duck_db_", "duckdb_")
	return out
}

// repoRootForRouteTest walks upward from the test binary's CWD until
// it finds a `go.mod` file (the repo root). Used by
// `TestKnownRouteNamesMatchesDispositionHeader` to resolve the
// canonical worktree.
func repoRootForRouteTest() (string, error) {
	dir, err := os.Getwd()
	if err != nil {
		return "", err
	}
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", os.ErrNotExist
		}
		dir = parent
	}
}

// Sentinels for `parseDispositionEnumValues` failure modes; pulled
// out so the parser stays a pure-Go single-output function.
var (
	errMissingDispositionEnum = staticErr(
		"`enum class Disposition { ... }` not found in disposition.h")
	errEmptyDispositionEnum = staticErr(
		"`enum class Disposition { ... }` matched but contained no `kFoo` entries")
)

type staticErr string

func (e staticErr) Error() string { return string(e) }

func TestKnownRouteNamesReturnsCopy(t *testing.T) {
	a := KnownRouteNames()
	a[0] = testBogus
	b := KnownRouteNames()
	if b[0] == testBogus {
		t.Fatalf("KnownRouteNames returned shared slice; mutating one altered another")
	}
}

func TestLoadRejectsUnknownRoute(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows: []
  route: cosmic
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "not a known disposition") {
		t.Fatalf("want unknown-route error, got %v", err)
	}
}

func TestLoadRejectsUnknownRouteAllowlistEntry(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows: []
  route: duckdb_native
  route_strict: false
  route_allowlist: [duckdb_native, cosmic]
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "route_allowlist[1]") {
		t.Fatalf("want unknown-allowlist-entry error, got %v", err)
	}
}

func TestLoadRejectsAllowlistWithStrictTrue(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows: []
  route: duckdb_native
  route_allowlist: [duckdb_native]
`)
	_, err := loadBytes(body, "test.yaml")
	if err == nil || !strings.Contains(err.Error(), "route_strict=true") {
		t.Fatalf("want allowlist-with-strict error, got %v", err)
	}
}

func TestLoadAcceptsRouteStrictDefault(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows: []
  route: duckdb_native
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if got := f.Expected.RouteStrictDefault(); got != true {
		t.Fatalf("RouteStrictDefault()=%v, want true (default)", got)
	}
	if f.Expected.Route != RouteDuckDBNative {
		t.Fatalf("Route=%q, want %q", f.Expected.Route, RouteDuckDBNative)
	}
}

func TestLoadAcceptsRouteStrictFalseWithAllowlist(t *testing.T) {
	body := []byte(`name: test
query: SELECT 1
expected:
  rows: []
  route_strict: false
  route_allowlist: [duckdb_native, duckdb_rewrite]
`)
	f, err := loadBytes(body, "test.yaml")
	if err != nil {
		t.Fatalf("loadBytes: %v", err)
	}
	if f.Expected.RouteStrictDefault() {
		t.Fatalf("RouteStrictDefault()=true, want false")
	}
	if len(f.Expected.RouteAllowlist) != 2 {
		t.Fatalf("RouteAllowlist=%v, want 2 entries", f.Expected.RouteAllowlist)
	}
}

// TestRouteDiffStrictPass covers the happy-path: the response's
// `emulatorRoute` equals the pinned `expected.route`.
func TestRouteDiffStrictPass(t *testing.T) {
	got := routeDiff(Expectation{Route: RouteDuckDBNative}, RouteDuckDBNative)
	if got != "" {
		t.Fatalf("routeDiff=%q, want empty", got)
	}
}

// TestRouteDiffStrictFail covers the failure-path: the diagnostic
// MUST name both the expected and actual route so a fixture writer
// can read the mismatch without re-running the engine.
func TestRouteDiffStrictFail(t *testing.T) {
	got := routeDiff(Expectation{Route: RouteDuckDBNative}, RouteSemanticExecutor)
	if got == "" {
		t.Fatal("routeDiff is empty; want a mismatch diagnostic")
	}
	if !strings.Contains(got, RouteDuckDBNative) {
		t.Errorf("diff missing expected route %q: %s", RouteDuckDBNative, got)
	}
	if !strings.Contains(got, RouteSemanticExecutor) {
		t.Errorf("diff missing actual route %q: %s", RouteSemanticExecutor, got)
	}
}

// TestRouteDiffAllowlistPass covers the relaxed mode: actual route
// is in the allowlist.
func TestRouteDiffAllowlistPass(t *testing.T) {
	strict := false
	exp := Expectation{
		Route:          "",
		RouteStrict:    &strict,
		RouteAllowlist: []string{RouteDuckDBNative, RouteDuckDBRewrite},
	}
	if got := routeDiff(exp, RouteDuckDBRewrite); got != "" {
		t.Fatalf("routeDiff=%q, want empty (allowlist match)", got)
	}
}

// TestRouteDiffAllowlistFail covers the relaxed-mode mismatch path:
// actual route is NOT in the allowlist. The diagnostic must
// enumerate every allowed route.
func TestRouteDiffAllowlistFail(t *testing.T) {
	strict := false
	exp := Expectation{
		RouteStrict:    &strict,
		RouteAllowlist: []string{RouteDuckDBNative, RouteDuckDBRewrite},
	}
	got := routeDiff(exp, RouteSemanticExecutor)
	if got == "" {
		t.Fatal("routeDiff is empty; want a mismatch diagnostic")
	}
	if !strings.Contains(got, RouteSemanticExecutor) {
		t.Errorf("diff missing actual route: %s", got)
	}
	if !strings.Contains(got, RouteDuckDBNative) ||
		!strings.Contains(got, RouteDuckDBRewrite) {
		t.Errorf("diff missing allowlist entries: %s", got)
	}
}

// TestRouteDiffEmptyExpectationSkips covers the
// no-route-assertion path: a fixture that pre-dates the routing
// matrix (or one of the deferred Storage Read / Write fixtures
// that doesn't go through the coordinator) leaves both fields
// empty and the runner skips the check entirely.
func TestRouteDiffEmptyExpectationSkips(t *testing.T) {
	if got := routeDiff(Expectation{}, ""); got != "" {
		t.Fatalf("routeDiff=%q, want empty (no assertion)", got)
	}
	if got := routeDiff(Expectation{}, RouteSemanticExecutor); got != "" {
		t.Fatalf("routeDiff=%q, want empty (no assertion even when route differs)", got)
	}
}

// TestRouteDiffRelaxedSkipsEmptyActual covers the
// document-the-intent pattern used by error-path fixtures: the
// engine returns before `EmitTrailers` fires, so the runner sees
// an empty `emulatorRoute`; we still want the fixture YAML to
// record the planning-time route for the matrix walker. Relaxed
// mode treats an empty actual as a skip so the YAML can be
// honest about intent without failing the runner.
func TestRouteDiffRelaxedSkipsEmptyActual(t *testing.T) {
	strict := false
	exp := Expectation{
		Route:       RouteUnsupported,
		RouteStrict: &strict,
	}
	if got := routeDiff(exp, ""); got != "" {
		t.Fatalf("routeDiff=%q, want empty (relaxed mode + empty actual = skip)", got)
	}
}

// TestRouteDiffRelaxedRouteOnlyPassesOnMatch covers the
// document-the-intent pattern's positive branch: if the engine
// DOES emit a route trailer in some future hardening pass, the
// fixture writer pinning `route: unsupported` would expect that
// route. Make sure relaxed mode treats `Route` as a one-element
// allowlist (i.e. `route_allowlist` is the union with `Route`).
func TestRouteDiffRelaxedRouteOnlyPassesOnMatch(t *testing.T) {
	strict := false
	exp := Expectation{
		Route:       RouteUnsupported,
		RouteStrict: &strict,
	}
	if got := routeDiff(exp, RouteUnsupported); got != "" {
		t.Fatalf("routeDiff=%q, want empty (relaxed + matching actual)", got)
	}
	if got := routeDiff(exp, RouteDuckDBNative); got == "" {
		t.Fatal("want mismatch diagnostic when relaxed actual differs from documented route")
	}
}

// TestRouteDiffStrictMissingActual catches the case where the
// engine returned an empty `emulatorRoute` despite a strict
// fixture: this happens when the gateway hits the non-loopback
// path of the loopback middleware, or when the trailer plumbing
// regresses. The runner always assumes it's running on loopback
// (it talks to a local emulator_main); a missing route there is a
// hard fail.
func TestRouteDiffStrictMissingActual(t *testing.T) {
	got := routeDiff(Expectation{Route: RouteDuckDBNative}, "")
	if got == "" {
		t.Fatal("routeDiff is empty; want a mismatch diagnostic")
	}
	if !strings.Contains(got, RouteDuckDBNative) {
		t.Errorf("diff missing expected route: %s", got)
	}
}
