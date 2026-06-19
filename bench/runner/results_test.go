package runner

import (
	"testing"
	"time"
)

func TestMergeReport_PreservesOtherCases(t *testing.T) {
	t.Parallel()
	existing := RunReport{
		Timestamp: time.Unix(0, 0),
		Targets:   []TargetName{TargetEmulator, TargetGoccy},
		Results: []CaseResult{
			{CaseName: "keep_me", Target: TargetEmulator, Outcome: OutcomeOK, Latency: LatencyStats{P50: 5 * time.Millisecond}},
			{CaseName: "keep_me", Target: TargetGoccy, Outcome: OutcomeOK, Latency: LatencyStats{P50: 9 * time.Millisecond}},
			{CaseName: "create_view_100k", Target: TargetEmulator, Outcome: OutcomeError, Error: "old failure"},
			{CaseName: "create_view_100k", Target: TargetGoccy, Outcome: OutcomeError, Error: "old failure"},
		},
	}
	fresh := RunReport{
		Timestamp: time.Unix(100, 0),
		Targets:   []TargetName{TargetEmulator, TargetGoccy},
		Results: []CaseResult{
			{CaseName: "create_view_100k", Target: TargetEmulator, Outcome: OutcomeOK, Latency: LatencyStats{P50: 12 * time.Millisecond}},
			{CaseName: "create_view_100k", Target: TargetGoccy, Outcome: OutcomeOK, Latency: LatencyStats{P50: 20 * time.Millisecond}},
		},
	}

	merged := MergeReport(existing, fresh)
	if len(merged.Results) != 4 {
		t.Fatalf("merged rows = %d, want 4", len(merged.Results))
	}
	for _, r := range merged.Results {
		switch r.CaseName + "/" + string(r.Target) {
		case "keep_me/emulator":
			if r.Latency.P50 != 5*time.Millisecond {
				t.Fatalf("keep_me/emulator altered: %+v", r)
			}
		case "keep_me/goccy":
			if r.Latency.P50 != 9*time.Millisecond {
				t.Fatalf("keep_me/goccy altered: %+v", r)
			}
		case "create_view_100k/emulator":
			if r.Outcome != OutcomeOK || r.Latency.P50 != 12*time.Millisecond {
				t.Fatalf("create_view_100k/emulator not updated: %+v", r)
			}
		case "create_view_100k/goccy":
			if r.Outcome != OutcomeOK || r.Latency.P50 != 20*time.Millisecond {
				t.Fatalf("create_view_100k/goccy not updated: %+v", r)
			}
		default:
			t.Fatalf("unexpected row: %+v", r)
		}
	}
	if !merged.Timestamp.Equal(fresh.Timestamp) {
		t.Fatalf("Timestamp = %v, want %v", merged.Timestamp, fresh.Timestamp)
	}
}
