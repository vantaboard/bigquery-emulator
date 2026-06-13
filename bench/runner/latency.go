package runner

import "time"

const minRatioMS int64 = 1

// LatencyP50MS returns the primary BigQuery server-side latency (execution p50).
// Falls back to total client wall-clock for legacy baselines.
func (b BaselineCase) LatencyP50MS() int64 {
	if b.ExecutionP50MS > 0 {
		return b.ExecutionP50MS
	}
	return b.TotalP50MS
}

// LatencyP50ForRatio returns a safe BQ denominator for ratio math (minimum 1ms).
func (b BaselineCase) LatencyP50ForRatio() int64 {
	ms := b.LatencyP50MS()
	if ms <= 0 {
		return minRatioMS
	}
	return ms
}

// CompareLatencyP50 returns the emulator latency used for baseline comparison.
// Prefers server-side total_engine; falls back to HTTP wall-clock.
func (r CaseResult) CompareLatencyP50() time.Duration {
	if r.EngineP50 > 0 {
		return r.EngineP50
	}
	return r.Latency.P50
}

// CompareLatencyMSForRatio returns emulator latency in milliseconds with a 1ms floor.
func (r CaseResult) CompareLatencyMSForRatio() int64 {
	ms := r.CompareLatencyP50().Milliseconds()
	if ms <= 0 {
		return minRatioMS
	}
	return ms
}

// EngineP50FromPhases extracts total_engine p50 from aggregated phase stats.
func EngineP50FromPhases(phases PhaseStats) time.Duration {
	if phases == nil {
		return 0
	}
	if stats, ok := phases["total_engine"]; ok {
		return stats.P50
	}
	return 0
}
