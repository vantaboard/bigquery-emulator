package jobs

import (
	"encoding/json"
	"testing"
	"time"
)

func TestFormatDryRunBytesProcessedNonZero(t *testing.T) {
	t.Parallel()
	if got, want := FormatDryRunBytesProcessed(0), "1"; got != want {
		t.Errorf("FormatDryRunBytesProcessed(0) = %q, want %q", got, want)
	}
	if got, want := FormatDryRunBytesProcessed(8192), "8192"; got != want {
		t.Errorf("FormatDryRunBytesProcessed(8192) = %q, want %q", got, want)
	}
}

func TestApplyDryRunStatisticsSetsQuerySubobject(t *testing.T) {
	t.Parallel()
	start := time.Unix(1_700_000_000, 0).UTC()
	end := start.Add(time.Second)
	job := &Job{}
	ApplyDryRunStatistics(job, 0, start, end)

	raw, err := json.Marshal(job)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}
	var out map[string]any
	if err := json.Unmarshal(raw, &out); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	stats, _ := out["statistics"].(map[string]any)
	if stats == nil {
		t.Fatal("statistics missing")
	}
	if got, want := stats["totalBytesProcessed"], "1"; got != want {
		t.Errorf("statistics.totalBytesProcessed = %v, want %q", got, want)
	}
	query, _ := stats["query"].(map[string]any)
	if query == nil {
		t.Fatal("statistics.query missing")
	}
	if got, want := query["totalBytesProcessed"], "1"; got != want {
		t.Errorf("statistics.query.totalBytesProcessed = %v, want %q", got, want)
	}
}
