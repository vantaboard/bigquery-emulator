package runner

import (
	"testing"
	"time"
)

func TestComputeLatencyStats(t *testing.T) {
	samples := []time.Duration{
		10 * time.Millisecond,
		20 * time.Millisecond,
		30 * time.Millisecond,
		40 * time.Millisecond,
	}
	stats := ComputeLatencyStats(samples, 1)
	if stats.N != 3 {
		t.Fatalf("N=%d", stats.N)
	}
	if stats.Min != 20*time.Millisecond {
		t.Fatalf("min=%s", stats.Min)
	}
	if stats.P50 != 30*time.Millisecond {
		t.Fatalf("p50=%s", stats.P50)
	}
}
