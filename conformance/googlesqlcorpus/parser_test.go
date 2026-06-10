package googlesqlcorpus

import (
	"os"
	"path/filepath"
	"testing"
)

func TestParseLogicalFunctionsFile(t *testing.T) {
	path := filepath.Join("..", "googlesql-corpus", "corpus", "logical_functions.test")
	b, err := os.ReadFile(path)
	if err != nil {
		t.Skip("vendored corpus not present:", err)
	}
	tf, err := ParseFile(path, string(b))
	if err != nil {
		t.Fatal(err)
	}
	if len(tf.Cases) < 50 {
		t.Fatalf("expected >=50 cases, got %d", len(tf.Cases))
	}
	tc := tf.Cases[0]
	if tc.SQL == "" || len(tc.Expected.Rows) == 0 {
		t.Fatalf("first case not parsed: %+v", tc)
	}
}

func TestParseExpectedBoolRow(t *testing.T) {
	exp, err := ParseExpected("ARRAY<STRUCT<BOOL>>[{true}]")
	if err != nil {
		t.Fatal(err)
	}
	if len(exp.Rows) != 1 || len(exp.Rows[0]) != 1 {
		t.Fatalf("rows: %+v", exp.Rows)
	}
	if exp.Rows[0][0] != true {
		t.Fatalf("got %v", exp.Rows[0][0])
	}
}

func TestParseExpectedNull(t *testing.T) {
	exp, err := ParseExpected("ARRAY<STRUCT<BOOL>>[{NULL}]")
	if err != nil {
		t.Fatal(err)
	}
	if exp.Rows[0][0] != nil {
		t.Fatalf("expected nil, got %v", exp.Rows[0][0])
	}
}
