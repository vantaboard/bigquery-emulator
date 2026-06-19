package runner

import (
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
)

const (
	testContentHash = "abc"
	testResultHash  = "hash"
)

func TestBaselineCase_LatencyP50MS(t *testing.T) {
	t.Parallel()
	base := BaselineCase{TotalP50MS: 1500, ExecutionP50MS: 200}
	if got := base.LatencyP50MS(); got != 200 {
		t.Fatalf("LatencyP50MS() = %d, want 200", got)
	}
	legacy := BaselineCase{TotalP50MS: 1500}
	if got := legacy.LatencyP50MS(); got != 1500 {
		t.Fatalf("legacy LatencyP50MS() = %d, want 1500", got)
	}
}

func TestBaselineCase_LatencyP50ForRatio(t *testing.T) {
	t.Parallel()
	if got := (BaselineCase{}).LatencyP50ForRatio(); got != 1 {
		t.Fatalf("zero baseline LatencyP50ForRatio() = %d, want 1", got)
	}
	if got := (BaselineCase{ExecutionP50MS: 0}).LatencyP50ForRatio(); got != 1 {
		t.Fatalf("zero execution LatencyP50ForRatio() = %d, want 1", got)
	}
}

func TestCompareToBaseline_UsesExecutionAndEngineP50(t *testing.T) {
	t.Parallel()
	c := Case{Name: "x", ContentHash: testContentHash, MaxRatio: 1.5, MaxMS: 100}
	base := BaselineCase{
		ContentHash:    testContentHash,
		ExecutionP50MS: 200,
		TotalP50MS:     1500,
		ResultHash:     testResultHash,
	}
	ok := CaseResult{
		ContentHash: testContentHash,
		Outcome:     OutcomeOK,
		EngineP50:   250 * time.Millisecond,
		Latency:     LatencyStats{P50: 5 * time.Millisecond},
		ResultHash:  testResultHash,
	}
	pass, reason := CompareToBaseline(c, base, ok)
	if !pass {
		t.Fatalf("expected pass, got %q", reason)
	}
	slow := ok
	slow.EngineP50 = 400 * time.Millisecond
	pass, reason = CompareToBaseline(c, base, slow)
	if pass {
		t.Fatalf("expected fail for slow engine p50")
	}
	if reason == "" {
		t.Fatal("expected non-empty reason")
	}
}

func TestCompareToBaseline_ZeroExecutionDoesNotPanic(t *testing.T) {
	t.Parallel()
	c := Case{Name: "x", ContentHash: testContentHash, MaxRatio: 1.5, MaxMS: 500}
	base := BaselineCase{ExecutionP50MS: 0, TotalP50MS: 0, ResultHash: testResultHash}
	cr := CaseResult{
		ContentHash: testContentHash,
		Outcome:     OutcomeOK,
		EngineP50:   2 * time.Millisecond,
		ResultHash:  testResultHash,
	}
	pass, _ := CompareToBaseline(c, base, cr)
	if !pass {
		t.Fatal("expected pass with floor denominator")
	}
}

func TestBuildBaselineFromResults_IncludesSlotMs(t *testing.T) {
	t.Parallel()
	b := BuildBaselineFromResults("proj", []CaseResult{{
		CaseName:       "select_literal",
		Target:         TargetBigQuery,
		Outcome:        OutcomeOK,
		ContentHash:    "h1",
		Latency:        LatencyStats{P50: 1200 * time.Millisecond},
		ExecutionP50:   150 * time.Millisecond,
		QueueP50:       20 * time.Millisecond,
		TotalSlotMsP50: 42,
		ResultHash:     "rh",
		RowCount:       1,
	}})
	got := b.Cases["select_literal"]
	if got.ExecutionP50MS != 150 {
		t.Fatalf("ExecutionP50MS = %d", got.ExecutionP50MS)
	}
	if got.TotalSlotMsP50 != 42 {
		t.Fatalf("TotalSlotMsP50 = %d", got.TotalSlotMsP50)
	}
	if got.QueueP50MS != 20 {
		t.Fatalf("QueueP50MS = %d", got.QueueP50MS)
	}
}

func TestMergeBaseline_PreservesExistingCases(t *testing.T) {
	t.Parallel()
	existing := BaselineFile{
		CapturedAt: time.Unix(0, 0),
		Project:    "proj",
		Cases: map[string]BaselineCase{
			testCaseKeepMe:         {ContentHash: "old-keep", ExecutionP50MS: 100, ResultHash: "rk"},
			testCaseCreateView100k: {ContentHash: "old-view", ExecutionP50MS: 999, ResultHash: "stale"},
		},
	}
	fresh := BuildBaselineFromResults("proj", []CaseResult{{
		CaseName:     testCaseCreateView100k,
		Target:       TargetBigQuery,
		Outcome:      OutcomeOK,
		ContentHash:  "new-view",
		ExecutionP50: 120 * time.Millisecond,
		ResultHash:   "fresh",
	}})

	merged := MergeBaseline(existing, fresh)
	if len(merged.Cases) != 2 {
		t.Fatalf("merged cases = %d, want 2", len(merged.Cases))
	}
	if got := merged.Cases[testCaseKeepMe]; got.ContentHash != "old-keep" || got.ExecutionP50MS != 100 {
		t.Fatalf("%s was altered: %+v", testCaseKeepMe, got)
	}
	updated := merged.Cases[testCaseCreateView100k]
	if updated.ContentHash != "new-view" || updated.ExecutionP50MS != 120 || updated.ResultHash != "fresh" {
		t.Fatalf("%s not updated: %+v", testCaseCreateView100k, updated)
	}
	if !merged.CapturedAt.Equal(fresh.CapturedAt) {
		t.Fatalf("CapturedAt = %v, want fresh %v", merged.CapturedAt, fresh.CapturedAt)
	}
}

func TestExtractBQJobMetrics(t *testing.T) {
	t.Parallel()
	start := time.Unix(0, 0).Add(2 * time.Second)
	end := start.Add(150 * time.Millisecond)
	created := start.Add(-30 * time.Millisecond)
	status := &bigquery.JobStatus{
		Statistics: &bigquery.JobStatistics{
			CreationTime:        created,
			StartTime:           start,
			EndTime:             end,
			TotalBytesProcessed: 800_000,
			Details: &bigquery.QueryStatistics{
				CacheHit:   false,
				SlotMillis: 1234,
			},
		},
	}
	m, err := extractBQJobMetrics(status)
	if err != nil {
		t.Fatal(err)
	}
	if m.execution != 150*time.Millisecond {
		t.Fatalf("execution = %s", m.execution)
	}
	if m.queue != 30*time.Millisecond {
		t.Fatalf("queue = %s", m.queue)
	}
	if m.slotMs != 1234 {
		t.Fatalf("slotMs = %d", m.slotMs)
	}
}

func TestExtractBQJobMetrics_MissingTimes(t *testing.T) {
	t.Parallel()
	_, err := extractBQJobMetrics(&bigquery.JobStatus{
		Statistics: &bigquery.JobStatistics{},
	})
	if err == nil {
		t.Fatal("expected error for missing times")
	}
}

func TestCaseResult_CompareLatencyP50(t *testing.T) {
	t.Parallel()
	cr := CaseResult{
		EngineP50: 10 * time.Millisecond,
		Latency:   LatencyStats{P50: 50 * time.Millisecond},
	}
	if got := cr.CompareLatencyP50(); got != 10*time.Millisecond {
		t.Fatalf("CompareLatencyP50 = %s", got)
	}
}
