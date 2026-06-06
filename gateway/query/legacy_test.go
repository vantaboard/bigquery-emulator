package query

import "testing"

func TestNormalizeLegacySQLBracketProjectDatasetTable(t *testing.T) {
	sql := `SELECT word FROM [bigquery-public-data:samples.shakespeare] LIMIT 10;`
	got, err := NormalizeLegacySQL(sql, "dev", "")
	if err != nil {
		t.Fatalf("NormalizeLegacySQL: %v", err)
	}
	want := "SELECT word FROM `bigquery-public-data.samples.shakespeare` LIMIT 10;"
	if got != want {
		t.Fatalf("got %q, want %q", got, want)
	}
}

func TestPrepareEngineSQLPassthroughGoogleSQL(t *testing.T) {
	sql := "SELECT 1"
	got, err := PrepareEngineSQL(false, sql, "dev", "")
	if err != nil {
		t.Fatalf("PrepareEngineSQL: %v", err)
	}
	if got != sql {
		t.Fatalf("got %q, want %q", got, sql)
	}
}

func TestNormalizeLegacySQLBareDatasetTable(t *testing.T) {
	sql := "SELECT x FROM [ds.t] LIMIT 1"
	got, err := NormalizeLegacySQL(sql, "proj", "")
	if err != nil {
		t.Fatalf("NormalizeLegacySQL: %v", err)
	}
	want := "SELECT x FROM `proj.ds.t` LIMIT 1"
	if got != want {
		t.Fatalf("got %q, want %q", got, want)
	}
}
