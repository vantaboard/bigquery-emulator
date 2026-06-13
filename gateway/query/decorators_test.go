package query

import (
	"strings"
	"testing"
)

func TestLowerTableDecoratorsAbsolute(t *testing.T) {
	sql := "SELECT id FROM `proj.ds.events@1700000000000`"
	got, err := LowerTableDecorators(sql)
	if err != nil {
		t.Fatalf("LowerTableDecorators: %v", err)
	}
	want := "SELECT id FROM `proj.ds.events` FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(1700000000000)"
	if got != want {
		t.Fatalf("got %q, want %q", got, want)
	}
}

func TestLowerTableDecoratorsRejectsSystemTimeCombo(t *testing.T) {
	sql := "SELECT * FROM `ds.t@123` FOR SYSTEM_TIME AS OF CURRENT_TIMESTAMP()"
	_, err := LowerTableDecorators(sql)
	if err == nil {
		t.Fatal("expected conflict error")
	}
}

func TestPrepareEngineSQLLowersDecorators(t *testing.T) {
	sql := "SELECT 1 FROM `ds.events@0`"
	got, err := PrepareEngineSQL(false, sql, "proj", "ds")
	if err != nil {
		t.Fatalf("PrepareEngineSQL: %v", err)
	}
	if !strings.Contains(got, "FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(0)") {
		t.Fatalf("expected lowered decorator, got %q", got)
	}
}
