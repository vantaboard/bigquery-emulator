//go:build live_sheets

package external

import (
	"context"
	"fmt"
	"os"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

// TestLiveGoogleSheetsClassData fetches the public sample sheet and compares
// row count to the embedded fixture (30 data rows).
func TestLiveGoogleSheetsClassData(t *testing.T) {
	if os.Getenv("BIGQUERY_EMULATOR_LIVE_SHEETS") == "" {
		t.Skip("set BIGQUERY_EMULATOR_LIVE_SHEETS=1 to run live Google Sheets fetch")
	}
	t.Setenv("BIGQUERY_EMULATOR_LIVE_SHEETS", "1")
	cfg, err := sourceconfig.Load("")
	if err != nil {
		t.Fatal(err)
	}
	resolver := NewResolver(cfg)
	cfgExt := &bqtypes.ExternalDataConfiguration{
		SourceFormat: "GOOGLE_SHEETS",
		SourceURIs: []string{
			"https://docs.google.com/spreadsheets/d/" + ClassDataSheetDocID + "/edit",
		},
		Autodetect: true,
	}
	parsed, err := parseSheetsExternal(context.Background(), resolver, cfgExt, nil, 1)
	if err != nil {
		t.Fatal(err)
	}
	if len(parsed.Rows) != 30 {
		t.Fatalf("live rows = %d, want 30", len(parsed.Rows))
	}
	fixture, err := loadFixtureSheetsCSV(resolver, ClassDataSheetDocID)
	if err != nil {
		t.Fatal(err)
	}
	liveNames := firstColumnNames(parsed, "Student Name")
	fixtureNames := namesFromCSV(fixture)
	if len(liveNames) != len(fixtureNames) {
		t.Fatalf("name count live=%d fixture=%d", len(liveNames), len(fixtureNames))
	}
	for i := range liveNames {
		if liveNames[i] != fixtureNames[i] {
			t.Fatalf("row %d: live=%q fixture=%q", i, liveNames[i], fixtureNames[i])
		}
	}
}

func firstColumnNames(parsed load.ParsedRows, col string) []string {
	out := make([]string, 0, len(parsed.Rows))
	for _, row := range parsed.Rows {
		v, ok := row[col]
		if !ok {
			continue
		}
		out = append(out, fmtAnyString(v))
	}
	return out
}

func fmtAnyString(v any) string {
	return fmt.Sprint(v)
}

func namesFromCSV(data []byte) []string {
	lines := strings.Split(strings.TrimSpace(string(data)), "\n")
	if len(lines) <= 1 {
		return nil
	}
	out := make([]string, 0, len(lines)-1)
	for _, line := range lines[1:] {
		if line == "" {
			continue
		}
		out = append(out, strings.SplitN(line, ",", 2)[0])
	}
	return out
}
