package main

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

const sampleLog = `=== lint:cpp:tidy-parallel started ===

========== backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc ==========
backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc:122:25: error: function 'EmitAggregateScan' has cognitive complexity of 91 (threshold 25) [readability-function-cognitive-complexity,-warnings-as-errors]
FAILED: backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc

========== backend/catalog/googlesql_catalog.cc ==========
backend/catalog/googlesql_catalog.cc:213:32: error: function 'FindTable' has cognitive complexity of 35 (threshold 25) [readability-function-cognitive-complexity,-warnings-as-errors]
FAILED: backend/catalog/googlesql_catalog.cc

========== backend/engine/semantic/value.cc ==========
backend/engine/semantic/value.cc:100:5: error: do not use 'else' after 'return' [readability-else-after-return,-warnings-as-errors]
FAILED: backend/engine/semantic/value.cc
`

const interleavedLog = `========== backend/catalog/googlesql_catalog.cc ==========
========== backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc ==========
backend/catalog/googlesql_catalog.cc:213:32: error: function 'FindTable' has cognitive complexity of 35 (threshold 25) [readability-function-cognitive-complexity,-warnings-as-errors]
backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc:122:25: error: function 'EmitAggregateScan' has cognitive complexity of 91 (threshold 25) [readability-function-cognitive-complexity,-warnings-as-errors]
FAILED: backend/catalog/googlesql_catalog.cc
FAILED: backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc
`

func TestParseTidyLogInterleaved(t *testing.T) {
	findings, failed, _ := parseTidyLog(strings.NewReader(interleavedLog))
	if len(failed) != 2 {
		t.Fatalf("failed files=%d want 2", len(failed))
	}
	summaries := summarizeByFile(findings, failed)
	if len(summaries) != 2 {
		t.Fatalf("summaries=%d want 2", len(summaries))
	}
	byPath := make(map[string]fileSummary)
	for _, s := range summaries {
		byPath[s.File] = s
	}
	if byPath["backend/catalog/googlesql_catalog.cc"].WorstComplexity != 35 {
		t.Errorf("catalog complexity=%d want 35", byPath["backend/catalog/googlesql_catalog.cc"].WorstComplexity)
	}
	if byPath["backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc"].WorstComplexity != 91 {
		t.Errorf(
			"aggregate complexity=%d want 91",
			byPath["backend/engine/duckdb/transpiler/transpiler_emit_aggregate.cc"].WorstComplexity,
		)
	}
}

func TestParseTidyLog(t *testing.T) {
	findings, failed, blocks := parseTidyLog(strings.NewReader(sampleLog))
	if blocks != 3 {
		t.Fatalf("blocks=%d want 3", blocks)
	}
	if len(failed) != 3 {
		t.Fatalf("failed files=%d want 3", len(failed))
	}
	if len(findings) != 3 {
		t.Fatalf("findings=%d want 3", len(findings))
	}
	if findings[0].ComplexityScore != 91 || findings[0].Symbol != "EmitAggregateScan" {
		t.Errorf("first finding: %+v", findings[0])
	}
	if findings[2].Check != "readability-else-after-return" {
		t.Errorf("third check=%q", findings[2].Check)
	}
}

func TestRunParseTidyLog(t *testing.T) {
	dir := t.TempDir()
	logPath := filepath.Join(dir, "test.log")
	csvPath := filepath.Join(dir, "out.csv")
	mdPath := filepath.Join(dir, "triage.md")
	if err := os.WriteFile(logPath, []byte(sampleLog), 0o644); err != nil {
		t.Fatal(err)
	}
	var stdout bytes.Buffer
	if err := runParseTidyLog([]string{
		"-log", logPath,
		"-csv", csvPath,
		"-markdown", mdPath,
	}, &stdout, os.Stderr); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(csvPath); err != nil {
		t.Fatalf("csv: %v", err)
	}
	md, err := os.ReadFile(mdPath)
	if err != nil {
		t.Fatal(err)
	}
	if !strings.Contains(string(md), "EmitAggregateScan") {
		t.Errorf("triage doc missing EmitAggregateScan")
	}
	if !strings.Contains(string(md), "readability-else-after-return") {
		t.Errorf("triage doc missing non-complexity check")
	}
}
