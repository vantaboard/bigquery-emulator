package main

import (
	"bytes"
	"os"
	"os/exec"
	"path/filepath"
	"reflect"
	"runtime"
	"slices"
	"sort"
	"strings"
	"testing"
)

// TestFilterFirstParty pins the ownership boundary that every C++
// lint task relies on. The fixture intentionally mixes paths from
// the explicitly-allowed include roots, paths under
// `firstPartyExcludePrefixes`, paths in `firstPartyExcludePaths`,
// and a wide selection of vendored / generated / cached trees that
// must always be rejected. If any future edit accidentally widens
// the boundary, this table catches it.
func TestFilterFirstParty(t *testing.T) {
	in := []string{
		// First party — must pass.
		SentinelEngine,
		"backend/engine/engine.h",
		"backend/engine/duckdb/duckdb_engine.cc",
		"backend/storage/duckdb/duckdb_storage.cc",
		SentinelEmulatorMain,
		SentinelEmulatorVersionH,
		SentinelEmulatorVersionT,
		"frontend/handlers/query.cc",
		"frontend/handlers/query.h",
		"frontend/server/server.cc",
		SentinelSmoke,
		"tools/googlesql-prebuilt/smoke/smoke_wrappers.cc",

		// Genrule outputs — explicit exclude.
		"binaries/emulator_main/version.cc",

		// Templates — under exclude prefix.
		"tools/googlesql-prebuilt/templates/BUILD.bazel",
		"tools/googlesql-prebuilt/templates/MODULE.bazel.tmpl",

		// Wrong extension under a first-party root.
		"backend/engine/BUILD.bazel",
		"backend/engine/README.md",
		"binaries/emulator_main/version_gen.sh",

		// Vendored / cached / generated trees that must NEVER leak.
		"third_party/duckdb/duckdb.h",
		"third_party/python-bigquery-tests/sample.cc",
		"vendor/some/upstream.cc",
		"node_modules/foo/bar.cc",
		"gen/proto/emulator.pb.cc",
		"bazel-out/k8-fastbuild/bin/something.cc",
		"bazel-bin/binaries/emulator_main/main.cc",
		"bazel-testlogs/something/test.log",
		".cache/googlesql-prebuilt/header.h",
		"logs/task-thirdparty.log",

		// Adjacent C++ that would be tempting to pull in but lives
		// under tools/ subtrees we don't own.
		"tools/coverage/main.go",
	}
	want := []string{
		"backend/engine/duckdb/duckdb_engine.cc",
		SentinelEngine,
		"backend/engine/engine.h",
		"backend/storage/duckdb/duckdb_storage.cc",
		SentinelEmulatorMain,
		SentinelEmulatorVersionH,
		SentinelEmulatorVersionT,
		"frontend/handlers/query.cc",
		"frontend/handlers/query.h",
		"frontend/server/server.cc",
		SentinelSmoke,
		"tools/googlesql-prebuilt/smoke/smoke_wrappers.cc",
	}
	got := filterFirstParty(in)
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("filterFirstParty mismatch:\n got: %v\nwant: %v", got, want)
	}
}

// TestFilterFirstParty_NormalisesSeparators makes sure inputs that
// carry backslash separators (a future Windows host or a pasted
// path) are still routed through the include filter. The lint
// stack only ever runs on linux today, but the helper should not
// care.
func TestFilterFirstParty_NormalisesSeparators(t *testing.T) {
	got := filterFirstParty([]string{
		`backend\engine\engine.cc`,
	})
	want := []string{SentinelEngine}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("backslash normalisation mismatch:\n got: %v\nwant: %v", got, want)
	}
}

// TestFilterFirstParty_DoesNotOverlapDirectoryPrefix guards the
// `prefix + '/'` pattern. A directory called `backend2/` must not
// match the `backend/` include root.
func TestFilterFirstParty_DoesNotOverlapDirectoryPrefix(t *testing.T) {
	got := filterFirstParty([]string{
		"backend2/engine.cc",
		SentinelEngine,
		"binaries2/emulator_main/main.cc",
	})
	want := []string{SentinelEngine}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("prefix overlap leaked:\n got: %v\nwant: %v", got, want)
	}
}

// TestIsTestFile pins the test-file convention so check rules that
// loosen for tests stay aligned with the lint runner.
func TestIsTestFile(t *testing.T) {
	cases := []struct {
		path string
		want bool
	}{
		{SentinelEngine, false},
		{"backend/engine/engine_test.cc", true},
		{"frontend/handlers/storage_read_test.cc", true},
		{SentinelEmulatorVersionH, false},
		{SentinelEmulatorVersionT, true},
		{"tools/lint/cpp/foo_test.cpp", true},
	}
	for _, c := range cases {
		if got := IsTestFile(c.path); got != c.want {
			t.Errorf("IsTestFile(%q) = %v, want %v", c.path, got, c.want)
		}
	}
}

// TestRunList_LiveRepo confirms the `list` subcommand returns a
// non-empty, deterministic, first-party-only result against the
// real repository. The known-tracked file
// `binaries/emulator_main/main.cc` must be present, and no path
// under `third_party/` or `bazel-*/` may leak through.
//
// The test is skipped when `git` is not on PATH so contributors who
// pre-stage offline runs do not see a spurious failure.
func TestRunList_LiveRepo(t *testing.T) {
	if _, err := exec.LookPath("git"); err != nil {
		t.Skip("git not available in PATH")
	}
	root, err := repoRoot()
	if err != nil {
		t.Fatalf("repoRoot: %v", err)
	}
	defer setRepoRoot(root)()

	var stdout, stderr bytes.Buffer
	if err := run([]string{"list"}, &stdout, &stderr); err != nil {
		t.Fatalf("run list: %v\nstderr=%s", err, stderr.String())
	}
	files := nonEmptyLines(stdout.String())
	if len(files) == 0 {
		t.Fatalf("list returned no first-party C++ files")
	}
	if !sort.StringsAreSorted(files) {
		t.Errorf("list output is not sorted: %v", files)
	}
	if !contains(files, SentinelEmulatorMain) {
		t.Errorf("expected %q in list output, got %v", SentinelEmulatorMain, files)
	}
	for _, f := range files {
		if strings.HasPrefix(f, "third_party/") {
			t.Errorf("third_party leak: %q", f)
		}
		if strings.HasPrefix(f, "bazel-") {
			t.Errorf("bazel output leak: %q", f)
		}
		if strings.HasPrefix(f, ".cache/") {
			t.Errorf("cache leak: %q", f)
		}
		if strings.HasPrefix(f, "vendor/") {
			t.Errorf("vendor leak: %q", f)
		}
	}
}

// TestRunList_NoTests pins the `--tests=false` flag, which the
// task runner uses when it wants to apply a stricter rule set to
// production code without flagging test fixtures.
func TestRunList_NoTests(t *testing.T) {
	if _, err := exec.LookPath("git"); err != nil {
		t.Skip("git not available in PATH")
	}
	root, err := repoRoot()
	if err != nil {
		t.Fatalf("repoRoot: %v", err)
	}
	defer setRepoRoot(root)()
	var stdout, stderr bytes.Buffer
	if err := run([]string{"list", "-tests=false"}, &stdout, &stderr); err != nil {
		t.Fatalf("run list -tests=false: %v\nstderr=%s", err, stderr.String())
	}
	for _, f := range nonEmptyLines(stdout.String()) {
		if IsTestFile(f) {
			t.Errorf("tests=false leaked test file: %q", f)
		}
	}
}

// TestResolveAgainstRoot confirms the helper composes repo-relative
// paths with the supplied root using OS-native separators. The
// downstream callers feed these to `os.ReadFile`, which on Windows
// would otherwise reject forward-slash paths.
func TestResolveAgainstRoot(t *testing.T) {
	got := resolveAgainstRoot("/repo", SentinelEngine)
	want := filepath.FromSlash("/repo/backend/engine/engine.cc")
	if got != want {
		t.Errorf("resolveAgainstRoot = %q, want %q", got, want)
	}
}

// TestSetRepoRoot_RoundTrips makes sure the test seam restores its
// previous value. Without this, a leaked override could make a
// later test see the wrong root and silently lint nothing.
func TestSetRepoRoot_RoundTrips(t *testing.T) {
	if got := currentRepoRoot(); got != "" {
		t.Fatalf("baseline currentRepoRoot = %q, want empty", got)
	}
	cleanup := setRepoRoot("/tmp/example")
	if got := currentRepoRoot(); got != "/tmp/example" {
		t.Fatalf("after setRepoRoot, currentRepoRoot = %q, want /tmp/example", got)
	}
	cleanup()
	if got := currentRepoRoot(); got != "" {
		t.Fatalf("after cleanup, currentRepoRoot = %q, want empty", got)
	}
}

// nonEmptyLines splits a buffer on newlines and drops trailing
// blanks. Used by tests that consume `cpp-lint list` output.
func nonEmptyLines(s string) []string {
	var out []string
	for line := range strings.SplitSeq(s, "\n") {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}
		out = append(out, line)
	}
	return out
}

func contains(haystack []string, needle string) bool {
	return slices.Contains(haystack, needle)
}

// helper kept to silence "unused import" if a platform-specific
// build tag ever drops one of the imports used elsewhere in this
// file.
var _ = func() string {
	if runtime.GOOS == "" {
		_ = os.Getenv("PATH")
	}
	return ""
}
