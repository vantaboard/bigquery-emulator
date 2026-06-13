package runner

import (
	"math"
	"slices"
	"time"
)

// LatencyStats summarizes repeated latency samples (post-warmup).
type LatencyStats struct {
	Min time.Duration `json:"min"`
	P50 time.Duration `json:"p50"`
	P90 time.Duration `json:"p90"`
	Max time.Duration `json:"max"`
	N   int           `json:"n"`
}

// PhaseStats summarizes per-phase timings across iterations.
type PhaseStats map[string]LatencyStats

// ComputeLatencyStats returns percentiles for samples after warmup.
func ComputeLatencyStats(samples []time.Duration, warmup int) LatencyStats {
	if len(samples) == 0 {
		return LatencyStats{}
	}
	start := warmup
	if start >= len(samples) {
		start = len(samples) - 1
	}
	if start < 0 {
		start = 0
	}
	used := append([]time.Duration(nil), samples[start:]...)
	slices.Sort(used)
	return LatencyStats{
		Min: used[0],
		P50: percentile(used, 0.50),
		P90: percentile(used, 0.90),
		Max: used[len(used)-1],
		N:   len(used),
	}
}

// ComputePhaseStats aggregates phase timings (microseconds) across iterations.
func ComputePhaseStats(iterations []map[string]int64, warmup int) PhaseStats {
	if len(iterations) == 0 {
		return nil
	}
	start := warmup
	if start >= len(iterations) {
		start = len(iterations) - 1
	}
	names := map[string]struct{}{}
	for i := start; i < len(iterations); i++ {
		for k := range iterations[i] {
			names[k] = struct{}{}
		}
	}
	out := make(PhaseStats, len(names))
	for name := range names {
		var samples []time.Duration
		for i := start; i < len(iterations); i++ {
			if us, ok := iterations[i][name]; ok {
				samples = append(samples, time.Duration(us)*time.Microsecond)
			}
		}
		out[name] = ComputeLatencyStats(samples, 0)
	}
	return out
}

// ComputeInt64P50 returns the p50 of int64 samples after warmup.
func ComputeInt64P50(samples []int64, warmup int) int64 {
	if len(samples) == 0 {
		return 0
	}
	start := warmup
	if start >= len(samples) {
		start = len(samples) - 1
	}
	if start < 0 {
		start = 0
	}
	used := append([]int64(nil), samples[start:]...)
	slices.Sort(used)
	idx := max(int(math.Round(0.50*float64(len(used)-1))), 0)
	if idx >= len(used) {
		idx = len(used) - 1
	}
	return used[idx]
}

func percentile(sorted []time.Duration, p float64) time.Duration {
	if len(sorted) == 0 {
		return 0
	}
	if len(sorted) == 1 {
		return sorted[0]
	}
	idx := max(int(math.Round(p*float64(len(sorted)-1))), 0)
	if idx >= len(sorted) {
		idx = len(sorted) - 1
	}
	return sorted[idx]
}
