package connection

import (
	"context"
	"runtime"
	"time"
)

const (
	autoscaleIntervalDefault = 15 * time.Second
	heapPressureHigh         = 0.85
	heapPressureLow          = 0.50
	raiseCooldown            = 2 * time.Minute
)

// StartAutoscaleLoop runs [Manager.RunAutoscaleTick] on a ticker until ctx is cancelled.
func StartAutoscaleLoop(ctx context.Context, m *Manager, interval time.Duration) {
	if interval <= 0 {
		interval = autoscaleIntervalDefault
	}
	go func() {
		t := time.NewTicker(interval)
		defer t.Stop()
		for {
			select {
			case <-ctx.Done():
				return
			case <-t.C:
				m.RunAutoscaleTick()
			}
		}
	}()
}

// RunAutoscaleTick adjusts the soft pool cap using heap pressure and cooldown when raising the cap.
func (m *Manager) RunAutoscaleTick() {
	if m.poolMin == m.poolMaxHard {
		return
	}
	var ms runtime.MemStats
	runtime.ReadMemStats(&ms)

	cur := int(m.currentMax.Load())
	pressure := heapPressureRatio(&ms)

	if pressure >= heapPressureHigh {
		reduction := max(1, cur/10)
		next := cur - reduction
		if next < m.poolMin {
			next = m.poolMin
		}
		if next < cur {
			m.SetCurrentMax(next)
		}
		return
	}

	if pressure > heapPressureLow {
		return
	}

	m.autosMu.Lock()
	if time.Since(m.lastRaiseAt) < raiseCooldown {
		m.autosMu.Unlock()
		return
	}
	if cur >= m.poolMaxHard {
		m.autosMu.Unlock()
		return
	}
	m.lastRaiseAt = time.Now()
	m.autosMu.Unlock()
	m.SetCurrentMax(cur + 1)
}

func heapPressureRatio(ms *runtime.MemStats) float64 {
	if ms.HeapSys == 0 {
		return 0
	}
	return float64(ms.HeapAlloc) / float64(ms.HeapSys)
}
