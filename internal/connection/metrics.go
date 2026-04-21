package connection

import (
	"os"
	"sync/atomic"
	"time"
)

// Connection pool metrics are enabled when BQ_EMULATOR_CONN_METRICS=1 (e.g. stress tests or manual debugging).
// They measure time blocked waiting for a pooled connection vs time running the callback.
func connMetricsEnabled() bool {
	return os.Getenv("BQ_EMULATOR_CONN_METRICS") == "1"
}

var (
	connAcquireWaitNanos atomic.Uint64
	connFnHoldNanos    atomic.Uint64
	connAcquireCount   atomic.Uint64
	connAcquireCancel  atomic.Uint64
)

// ResetConnMetrics clears counters (call before a stress run).
func ResetConnMetrics() {
	connAcquireWaitNanos.Store(0)
	connFnHoldNanos.Store(0)
	connAcquireCount.Store(0)
	connAcquireCancel.Store(0)
}

// ConnMetricsSnapshot is a point-in-time view of pool usage.
type ConnMetricsSnapshot struct {
	TotalAcquireWait time.Duration
	TotalFnHold      time.Duration
	AcquireCount     uint64
	AcquireCanceled  uint64
}

// SnapshotConnMetrics returns accumulated metrics since the last [ResetConnMetrics].
func SnapshotConnMetrics() ConnMetricsSnapshot {
	return ConnMetricsSnapshot{
		TotalAcquireWait: time.Duration(connAcquireWaitNanos.Load()),
		TotalFnHold:      time.Duration(connFnHoldNanos.Load()),
		AcquireCount:     connAcquireCount.Load(),
		AcquireCanceled:  connAcquireCancel.Load(),
	}
}

func recordConnAcquireSuccess(wait time.Duration) {
	connAcquireWaitNanos.Add(uint64(wait.Nanoseconds()))
	connAcquireCount.Add(1)
}

func recordConnFnHold(d time.Duration) {
	connFnHoldNanos.Add(uint64(d.Nanoseconds()))
}

func recordConnAcquireCanceled() {
	connAcquireCancel.Add(1)
}
