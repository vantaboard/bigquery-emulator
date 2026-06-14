package sourceconfig

import (
	"os"
	"testing"
)

func TestLoadDefaults(t *testing.T) {
	t.Parallel()
	c, err := Load("")
	if err != nil {
		t.Fatal(err)
	}
	if c.ResolveGCS("gs://bkt/o.csv") != ModeLocal {
		t.Fatalf("gcs default = %v", c.ResolveGCS("gs://bkt/o.csv"))
	}
	if c.ResolveGoogleSheets("abc") != ModeFixture {
		t.Fatalf("sheets default = %v", c.ResolveGoogleSheets("abc"))
	}
}

func TestLiveSheetsEnvOverride(t *testing.T) {
	t.Setenv("BIGQUERY_EMULATOR_LIVE_SHEETS", "1")
	c, err := Load("")
	if err != nil {
		t.Fatal(err)
	}
	if c.ResolveGoogleSheets("abc") != ModeLive {
		t.Fatalf("sheets mode = %v, want live", c.ResolveGoogleSheets("abc"))
	}
}

func TestExtractSheetDocID(t *testing.T) {
	t.Parallel()
	const id = "1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"
	got := ExtractSheetDocID("https://docs.google.com/spreadsheets/d/" + id + "/edit")
	if got != id {
		t.Fatalf("got %q", got)
	}
}

func TestLoadYAMLFile(t *testing.T) {
	dir := t.TempDir()
	yaml := `defaults:
  google_sheets: fixture
sources:
  "1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms":
    kind: google_sheets
    mode: fixture
`
	if err := os.WriteFile(dir+"/external_sources.yaml", []byte(yaml), 0o600); err != nil {
		t.Fatal(err)
	}
	c, err := Load(dir)
	if err != nil {
		t.Fatal(err)
	}
	if c.ResolveGoogleSheets("1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms") != ModeFixture {
		t.Fatal("expected fixture from yaml")
	}
}
