package main

import (
	"strings"
	"testing"
)

// TestParseGoReader_Fixture pins the canonical Go cover.out fixture
// behaviour: a `mode:` header, five statement records spanning two
// files, three of them executed. Hits == 7, totals == 12 (see
// testdata/coverage.out for the breakdown).
func TestParseGoReader_Fixture(t *testing.T) {
	const input = `mode: atomic
github.com/example/foo.go:10.20,12.2 2 5
github.com/example/foo.go:14.30,16.16 1 0
github.com/example/foo.go:18.5,20.10 3 1
github.com/example/bar.go:5.10,7.2 4 0
github.com/example/bar.go:9.10,11.2 2 3
`
	h, total, err := parseGoReader(strings.NewReader(input))
	if err != nil {
		t.Fatalf("parseGoReader: %v", err)
	}
	if h != 7 {
		t.Errorf("hits = %d, want 7", h)
	}
	if total != 12 {
		t.Errorf("total = %d, want 12", total)
	}
}

// TestParseGoReader_EmptyAndComments documents that an empty profile
// (mode-only or zero lines) yields (0, 0) rather than an error, so a
// suite that compiled but ran no statements still produces a valid
// summary.
func TestParseGoReader_EmptyAndComments(t *testing.T) {
	cases := map[string]string{
		"empty":     "",
		"mode only": "mode: count\n",
		"whitespace": `

mode: set


`,
	}
	for name, input := range cases {
		t.Run(name, func(t *testing.T) {
			h, total, err := parseGoReader(strings.NewReader(input))
			if err != nil {
				t.Fatalf("parseGoReader: %v", err)
			}
			if h != 0 || total != 0 {
				t.Errorf("hits=%d total=%d, want 0/0", h, total)
			}
		})
	}
}

// TestParseGoReader_BadLine documents that malformed records are
// surfaced with a line-numbered error rather than silently skipped,
// so a corrupt profile fails CI loudly.
func TestParseGoReader_BadLine(t *testing.T) {
	input := "mode: atomic\nthis is not a coverage record\n"
	_, _, err := parseGoReader(strings.NewReader(input))
	if err == nil {
		t.Fatal("parseGoReader: want error, got nil")
	}
	if !strings.Contains(err.Error(), "line 2") {
		t.Errorf("error %q should mention line 2", err)
	}
}

// TestParseGoFile_Fixture covers the file-backed entry point against
// the checked-in fixture to guard against regressions in path handling
// or buffer sizing.
func TestParseGoFile_Fixture(t *testing.T) {
	h, total, err := parseGoFile("testdata/coverage.out")
	if err != nil {
		t.Fatalf("parseGoFile: %v", err)
	}
	if h != 7 || total != 12 {
		t.Errorf("hits=%d total=%d, want 7/12", h, total)
	}
}
