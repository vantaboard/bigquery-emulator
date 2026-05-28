package main

import (
	"strings"
	"testing"
)

// TestParseLCOVReader_Fixture pins the canonical LCOV fixture: three
// SF records, ten DA lines, six of them executed. Also exercises the
// optional MD5 checksum trailing field that geninfo emits with
// --checksum (the third record's DA lines).
func TestParseLCOVReader_Fixture(t *testing.T) {
	const input = `TN:
SF:/work/engine/foo.cc
DA:1,5
DA:2,0
DA:3,3
DA:4,1
LF:4
LH:3
end_of_record
TN:
SF:/work/engine/bar.cc
DA:10,0
DA:11,0
DA:12,2
LF:3
LH:1
end_of_record
TN:
SF:/work/engine/baz.cc
DA:1,7,abc123
DA:2,0,deadbe
DA:3,4
LF:3
LH:2
end_of_record
`
	h, total, err := parseLCOVReader(strings.NewReader(input))
	if err != nil {
		t.Fatalf("parseLCOVReader: %v", err)
	}
	if h != 6 {
		t.Errorf("hits = %d, want 6", h)
	}
	if total != 10 {
		t.Errorf("total = %d, want 10", total)
	}
}

// TestParseLCOVReader_IgnoresOtherRecords documents that the parser
// counts only DA: records, even when other record types (FN, BRDA,
// LF/LH summaries) are present.
func TestParseLCOVReader_IgnoresOtherRecords(t *testing.T) {
	const input = `TN:foo
SF:/x.cc
FN:1,doStuff
FNDA:0,doStuff
BRDA:1,0,0,-
DA:1,0
DA:2,3
BRDA:2,0,0,1
LF:2
LH:1
end_of_record
`
	h, total, err := parseLCOVReader(strings.NewReader(input))
	if err != nil {
		t.Fatalf("parseLCOVReader: %v", err)
	}
	if h != 1 || total != 2 {
		t.Errorf("hits=%d total=%d, want 1/2", h, total)
	}
}

// TestParseLCOVReader_BadDA documents that a malformed DA record
// causes a line-numbered error rather than silent truncation.
func TestParseLCOVReader_BadDA(t *testing.T) {
	const input = "DA:notanumber\n"
	_, _, err := parseLCOVReader(strings.NewReader(input))
	if err == nil {
		t.Fatal("parseLCOVReader: want error, got nil")
	}
	if !strings.Contains(err.Error(), "line 1") {
		t.Errorf("error %q should mention line 1", err)
	}
}

// TestParseLCOVFile_Fixture covers the file-backed entry point
// against the checked-in fixture so a regression in path handling
// surfaces as a real test failure.
func TestParseLCOVFile_Fixture(t *testing.T) {
	h, total, err := parseLCOVFile("testdata/coverage.dat")
	if err != nil {
		t.Fatalf("parseLCOVFile: %v", err)
	}
	if h != 6 || total != 10 {
		t.Errorf("hits=%d total=%d, want 6/10", h, total)
	}
}
