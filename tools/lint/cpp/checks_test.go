package main

import (
	"bytes"
	"reflect"
	"sort"
	"strings"
	"testing"
)

// TestCheckFileLength_Threshold pins the line-cap behaviour: a
// file at the cap passes, one over the cap fails.
func TestCheckFileLength_Threshold(t *testing.T) {
	exact := bytes.Repeat([]byte("x\n"), 500)
	over := bytes.Repeat([]byte("x\n"), 501)

	if got := checkFileLength("backend/foo.cc", exact, CheckOptions{MaxFileLines: 500}); len(got) != 0 {
		t.Errorf("file at cap should pass, got %v", got)
	}
	got := checkFileLength("backend/foo.cc", over, CheckOptions{MaxFileLines: 500})
	if len(got) != 1 || got[0].Rule != ruleFileLength {
		t.Fatalf("file over cap should produce 1 file-length finding, got %v", got)
	}
	if !strings.Contains(got[0].Message, "501 lines (max 500)") {
		t.Errorf("finding message missing line counts: %q", got[0].Message)
	}
}

// TestCheckFileLength_TrailingNewlineIndependence guarantees the
// line counter agrees with `wc -l` for files that lack a trailing
// newline. Without this, two files of the same logical length
// would disagree on whether they exceed the cap.
func TestCheckFileLength_TrailingNewlineIndependence(t *testing.T) {
	withNewline := bytes.Repeat([]byte("x\n"), 501)
	withoutNewline := append(bytes.Repeat([]byte("x\n"), 500), 'x')
	gotA := countLines(withNewline)
	gotB := countLines(withoutNewline)
	if gotA != 501 || gotB != 501 {
		t.Errorf("countLines mismatch: with=%d without=%d", gotA, gotB)
	}
}

// TestCheckBannedLogging_Production exercises the banned-logging
// rule against a production-style file. Comments and string
// literals must not raise findings; real calls must.
func TestCheckBannedLogging_Production(t *testing.T) {
	body := []byte(`// std::cerr in a comment is fine.
// printf in a comment is fine too.
const char* msg = "std::cout in a string literal is also fine";
void Bad() {
  std::cout << "boom";
  std::cerr << "boom";
  std::printf("boom");
  printf("boom");
  fprintf(stderr, "boom");
  formatter.printf("nested member call must NOT be flagged");
  obj->printf("arrow member call must NOT be flagged");
}
`)
	got := checkBannedLogging(SentinelEngine, splitLines(body))
	if len(got) != 5 {
		for _, f := range got {
			t.Logf("finding: %s", f.Format())
		}
		t.Fatalf("want 5 banned-logging findings, got %d", len(got))
	}
	for _, f := range got {
		if f.Rule != ruleBannedLogging {
			t.Errorf("unexpected rule %q", f.Rule)
		}
	}
}

// TestCheckBannedLogging_Tests confirms tests / smoke / main
// binaries are exempt. A `std::cerr` in `version_test.cc` must not
// be flagged because gtest fixture printers commonly use it.
func TestCheckBannedLogging_Tests(t *testing.T) {
	body := []byte(`void Test() {
  std::cerr << "diagnostic";
  printf("diagnostic");
}
`)
	for _, path := range []string{
		"backend/engine/engine_test.cc",
		SentinelEmulatorMain,
		SentinelSmoke,
	} {
		if got := checkBannedLogging(path, splitLines(body)); len(got) != 0 {
			t.Errorf("%s: unexpected findings %v", path, got)
		}
	}
}

// TestStripCommentsAndStrings keeps the lightweight lexer honest.
// The function is shared by every per-line rule so a regression
// here would silently widen the false-positive rate everywhere.
func TestStripCommentsAndStrings(t *testing.T) {
	cases := []struct {
		in, want string
	}{
		{`abc // comment`, `abc           `},
		{`x = "std::cerr"; // banned in comment`, `x =            ;                     `},
		{`std::cout << "value";`, `std::cout <<        ;`},
		{`empty()`, `empty()`},
	}
	for _, c := range cases {
		if got := stripCommentsAndStrings(c.in); got != c.want {
			t.Errorf("stripCommentsAndStrings(%q):\n got: %q\nwant: %q", c.in, got, c.want)
		}
	}
}

// TestCheckStatusAntiPatterns_Discarded covers the
// status-discarded rule. The matching function names must be
// flagged when their result is dropped, but pass when assigned.
func TestCheckStatusAntiPatterns_Discarded(t *testing.T) {
	body := []byte(`void Run() {
  storage.AppendRows(id, rows);
  absl::Status s = storage.AppendRows(id, rows);
  if (!s.ok()) return;
  engine.ExecuteDdl(req, catalog);
}
`)
	got := checkStatusAntiPatterns("backend/engine/duckdb/duckdb_executor.cc", splitLines(body))
	if len(got) != 2 {
		for _, f := range got {
			t.Logf("finding: %s", f.Format())
		}
		t.Fatalf("want 2 status-discarded findings, got %d", len(got))
	}
	for _, f := range got {
		if f.Rule != ruleStatusDiscarded {
			t.Errorf("unexpected rule %q", f.Rule)
		}
	}
}

// TestCheckStatusAntiPatterns_StatusOrValue covers the
// statusor-unchecked-value rule. `.value()` without a nearby
// `.ok()` / status guard is reported; a guarded access is not.
// The look-back window covers the canonical pattern of an early
// return on the previous line.
func TestCheckStatusAntiPatterns_StatusOrValue(t *testing.T) {
	body := []byte(`void Run() {
  auto x = MaybeFoo().value();
  if (s.ok()) auto y = s.value();
  absl::StatusOr<int> rendered = MaybeBar();
  if (!rendered.ok()) return rendered.status();
  *out = std::move(rendered).value();
  RETURN_IF_ERROR(other);
  *out2 = std::move(other).value();
}
`)
	got := checkStatusAntiPatterns("backend/storage/storage.cc", splitLines(body))
	if len(got) != 1 {
		for _, f := range got {
			t.Logf("finding: %s", f.Format())
		}
		t.Fatalf("want 1 statusor-unchecked-value finding, got %d", len(got))
	}
	if got[0].Rule != ruleStatusOrUnchecked {
		t.Errorf("unexpected rule %q", got[0].Rule)
	}
	if got[0].Line != 2 {
		t.Errorf("expected finding on line 2, got %d", got[0].Line)
	}
}

// TestCheckStatusAntiPatterns_TestsExempt confirms that *_test.cc
// files are not subjected to the status-discarded rule. Tests
// commonly call helpers and ASSERT-test the body separately, and
// gtest's `EXPECT_THAT(call(), IsOk())` style is the long-term
// guard there — not this lightweight regex.
func TestCheckStatusAntiPatterns_TestsExempt(t *testing.T) {
	body := []byte(`void TestBody() {
  storage.AppendRows(id, rows);
}
`)
	if got := checkStatusAntiPatterns("backend/storage/storage_test.cc", splitLines(body)); len(got) != 0 {
		t.Errorf("tests should be exempt, got %v", got)
	}
}

// TestRunOnce_Suppression covers the inline-suppression marker.
// A `// cpp-lint:allow(rule) -- reason` comment on the same line
// as the offending construct must drop the finding while leaving
// other rules intact.
func TestRunOnce_Suppression(t *testing.T) {
	body := []byte(`void Bootstrap() {
  std::fprintf(stderr, "init failure\n");  // cpp-lint:allow(banned-logging) -- pre-grpc bootstrap diagnostic
  std::fprintf(stderr, "second failure\n");
}
`)
	got := runOnce("frontend/server/server.cc", body, CheckOptions{MaxFileLines: 500})
	if len(got) != 1 {
		for _, f := range got {
			t.Logf("finding: %s", f.Format())
		}
		t.Fatalf("want 1 finding (the unsuppressed call), got %d", len(got))
	}
	if got[0].Line != 3 {
		t.Errorf("expected unsuppressed finding on line 3, got %d", got[0].Line)
	}
}

// TestRunOnce_SuppressionAboveStatement guarantees that a marker
// living on a comment-only line directly above the offending
// statement still suppresses. clang-format may wrap a long trailing
// comment to its own line, and we must not lose the marker when
// that happens.
func TestRunOnce_SuppressionAboveStatement(t *testing.T) {
	body := []byte(`void Bootstrap() {
  // cpp-lint:allow(banned-logging) -- pre-grpc bootstrap diagnostic
  std::fprintf(stderr, "init failure\n");
  std::fprintf(stderr, "second failure\n");
}
`)
	got := runOnce("frontend/server/server.cc", body, CheckOptions{MaxFileLines: 500})
	if len(got) != 1 {
		for _, f := range got {
			t.Logf("finding: %s", f.Format())
		}
		t.Fatalf("want 1 finding (the unsuppressed call), got %d", len(got))
	}
	if got[0].Line != 4 {
		t.Errorf("expected unsuppressed finding on line 4, got %d", got[0].Line)
	}
}

// TestRunOnce_SuppressionRequiresReason makes sure a marker
// missing the `-- reason` body fails closed. The regex does not
// match without a reason, so the finding still fires.
func TestRunOnce_SuppressionRequiresReason(t *testing.T) {
	body := []byte(`void Bootstrap() {
  std::fprintf(stderr, "init failure\n");  // cpp-lint:allow(banned-logging)
}
`)
	got := runOnce("frontend/server/server.cc", body, CheckOptions{MaxFileLines: 500})
	if len(got) != 1 {
		t.Fatalf("want 1 finding (no reason -> no suppression), got %d", len(got))
	}
}

// TestRunOnce_Sorted confirms findings come out in a deterministic
// (line, rule) order. Editors and the CI summary both depend on
// stable ordering so a re-run does not produce a different log.
func TestRunOnce_Sorted(t *testing.T) {
	body := bytes.Buffer{}
	for range 600 {
		body.WriteString("void f();\n")
	}
	body.WriteString(`std::cout << "x";
`)
	out := runOnce("backend/foo.cc", body.Bytes(), CheckOptions{MaxFileLines: 500})
	got := make([]int, len(out))
	for i, f := range out {
		got[i] = f.Line
	}
	want := append([]int(nil), got...)
	sort.Ints(want)
	if !reflect.DeepEqual(got, want) {
		t.Errorf("findings not sorted by line: %v", got)
	}
}
