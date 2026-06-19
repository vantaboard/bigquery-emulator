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
			{
				CaseName: testCaseKeepMe,
				Target:   TargetEmulator,
				Outcome:  OutcomeOK,
				Latency:  LatencyStats{P50: 5 * time.Millisecond},
			},
			{
				CaseName: testCaseKeepMe,
				Target:   TargetGoccy,
				Outcome:  OutcomeOK,
				Latency:  LatencyStats{P50: 9 * time.Millisecond},
			},
			{CaseName: testCaseCreateView100k, Target: TargetEmulator, Outcome: OutcomeError, Error: "old failure"},
			{CaseName: testCaseCreateView100k, Target: TargetGoccy, Outcome: OutcomeError, Error: "old failure"},
		},
	}
	fresh := RunReport{
		Timestamp: time.Unix(100, 0),
		Targets:   []TargetName{TargetEmulator, TargetGoccy},
		Results: []CaseResult{
			{
				CaseName: testCaseCreateView100k,
				Target:   TargetEmulator,
				Outcome:  OutcomeOK,
				Latency:  LatencyStats{P50: 12 * time.Millisecond},
			},
			{
				CaseName: testCaseCreateView100k,
				Target:   TargetGoccy,
				Outcome:  OutcomeOK,
				Latency:  LatencyStats{P50: 20 * time.Millisecond},
			},
		},
	}

	merged := MergeReport(existing, fresh)
	assertMergeReportResults(t, merged, fresh)
}

func assertMergeReportResults(t *testing.T, merged, fresh RunReport) {
	t.Helper()
	if len(merged.Results) != 4 {
		t.Fatalf("merged rows = %d, want 4", len(merged.Results))
	}
	for _, r := range merged.Results {
		switch r.CaseName + "/" + string(r.Target) {
		case testCaseKeepMe + "/emulator":
			if r.Latency.P50 != 5*time.Millisecond {
				t.Fatalf("%s/emulator altered: %+v", testCaseKeepMe, r)
			}
		case testCaseKeepMe + "/goccy":
			if r.Latency.P50 != 9*time.Millisecond {
				t.Fatalf("%s/goccy altered: %+v", testCaseKeepMe, r)
			}
		case testCaseCreateView100k + "/emulator":
			if r.Outcome != OutcomeOK || r.Latency.P50 != 12*time.Millisecond {
				t.Fatalf("%s/emulator not updated: %+v", testCaseCreateView100k, r)
			}
		case testCaseCreateView100k + "/goccy":
			if r.Outcome != OutcomeOK || r.Latency.P50 != 20*time.Millisecond {
				t.Fatalf("%s/goccy not updated: %+v", testCaseCreateView100k, r)
			}
		default:
			t.Fatalf("unexpected row: %+v", r)
		}
	}
	if !merged.Timestamp.Equal(fresh.Timestamp) {
		t.Fatalf("Timestamp = %v, want %v", merged.Timestamp, fresh.Timestamp)
	}
}
