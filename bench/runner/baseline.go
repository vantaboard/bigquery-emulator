package runner

import (
	"encoding/json"
	"fmt"
	"os"
	"time"
)

// BaselineFile is the committed golden BigQuery capture.
type BaselineFile struct {
	CapturedAt  time.Time               `json:"captured_at"`
	Project     string                  `json:"project"`
	ProjectHash string                  `json:"project_hash,omitempty"`
	Cases       map[string]BaselineCase `json:"cases"`
}

// BaselineCase holds golden latency and correctness for one case.
type BaselineCase struct {
	ContentHash    string `json:"content_hash"`
	TotalP50MS     int64  `json:"total_p50_ms"`
	ExecutionP50MS int64  `json:"execution_p50_ms"`
	QueueP50MS     int64  `json:"queue_p50_ms,omitempty"`
	TotalSlotMsP50 int64  `json:"total_slot_ms_p50,omitempty"`
	BytesProcessed int64  `json:"bytes_processed,omitempty"`
	ResultHash     string `json:"result_hash"`
	RowCount       int    `json:"row_count"`
}

// LoadBaseline reads bench/baselines/bigquery.json.
func LoadBaseline(path string) (BaselineFile, error) {
	raw, err := os.ReadFile(path) //nolint:gosec // baseline path is CLI-controlled
	if err != nil {
		return BaselineFile{}, err
	}
	var b BaselineFile
	if err := json.Unmarshal(raw, &b); err != nil {
		return BaselineFile{}, err
	}
	if b.Cases == nil {
		b.Cases = map[string]BaselineCase{}
	}
	return b, nil
}

// SaveBaseline writes a baseline file.
//
//nolint:gosec // baseline path and 0o644 mode are CLI-controlled benchmark artifacts.
func SaveBaseline(path string, b BaselineFile) error {
	b.CapturedAt = b.CapturedAt.UTC()
	raw, err := json.MarshalIndent(b, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(
		path,
		append(raw, '\n'),
		0o644,
	)
}

// BuildBaselineFromResults constructs a baseline from a BQ benchmark run.
func BuildBaselineFromResults(project string, results []CaseResult) BaselineFile {
	b := BaselineFile{
		CapturedAt: time.Now().UTC(),
		Project:    project,
		Cases:      map[string]BaselineCase{},
	}
	for _, r := range results {
		if r.Target != TargetBigQuery || r.Outcome != OutcomeOK {
			continue
		}
		b.Cases[r.CaseName] = BaselineCase{
			ContentHash:    r.ContentHash,
			TotalP50MS:     r.Latency.P50.Milliseconds(),
			ExecutionP50MS: r.ExecutionP50.Milliseconds(),
			QueueP50MS:     r.QueueP50.Milliseconds(),
			TotalSlotMsP50: r.TotalSlotMsP50,
			BytesProcessed: r.BytesProcessed,
			ResultHash:     r.ResultHash,
			RowCount:       r.RowCount,
		}
	}
	return b
}

// CompareToBaseline checks emulator result against golden baseline.
func CompareToBaseline(c Case, base BaselineCase, r CaseResult) (pass bool, reason string) {
	if base.ContentHash != "" && base.ContentHash != c.ContentHash {
		return false, fmt.Sprintf("stale baseline (case changed): want hash %s got %s", base.ContentHash, c.ContentHash)
	}
	if r.Outcome == OutcomeWrongResult {
		return false, "wrong result vs baseline hash"
	}
	if r.Outcome != OutcomeOK {
		return false, string(r.Outcome) + ": " + r.Error
	}
	emuLatency := r.CompareLatencyP50()
	threshold := time.Duration(c.MaxMS) * time.Millisecond
	bqMS := base.LatencyP50ForRatio()
	if bqMS > 0 {
		bq := time.Duration(bqMS) * time.Millisecond
		ratioThreshold := time.Duration(float64(bq) * c.MaxRatio)
		if ratioThreshold > threshold {
			threshold = ratioThreshold
		}
	}
	if emuLatency > threshold {
		return false, fmt.Sprintf("p50 %s > threshold %s (bq execution p50 %dms, ratio %.2f)",
			emuLatency, threshold, base.LatencyP50MS(), c.MaxRatio)
	}
	return true, ""
}
