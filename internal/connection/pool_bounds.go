package connection

import (
	"os"
	"runtime"
	"strconv"
	"strings"
)

// PoolConfig controls elastic or fixed connection pooling for [NewManager].
type PoolConfig struct {
	// FixedSize > 0 selects a fixed pool of that size (no autoscale loop, no elastic growth).
	FixedSize int
	PoolMin   int
	// PoolMaxHard is the upper bound for live connections and idle channel capacity.
	PoolMaxHard int
	// AutoscaleLoop enables the background [StartAutoscaleLoop] (ignored for fixed pools).
	AutoscaleLoop bool
}

// PoolConfigFromEnv builds defaults: BQ_EMULATOR_POOL_SIZE fixes the pool; otherwise elastic bounds
// from [PoolBoundsFromResources] with BQ_EMULATOR_POOL_MIN/MAX clamps and BQ_EMULATOR_POOL_AUTOSCALE.
func PoolConfigFromEnv() PoolConfig {
	if v := strings.TrimSpace(os.Getenv("BQ_EMULATOR_POOL_SIZE")); v != "" {
		if n, err := strconv.Atoi(v); err == nil && n > 0 {
			return PoolConfig{
				FixedSize:     n,
				PoolMin:       n,
				PoolMaxHard:   n,
				AutoscaleLoop: false,
			}
		}
	}
	poolMin, poolMaxHard := PoolBoundsFromResources(DefaultQueryWorkersForPool)
	poolMin, poolMaxHard = ParsePoolEnvClamps(poolMin, poolMaxHard)
	autoscale := strings.TrimSpace(os.Getenv("BQ_EMULATOR_POOL_AUTOSCALE")) != "0"
	return PoolConfig{
		FixedSize:     0,
		PoolMin:       poolMin,
		PoolMaxHard:   poolMaxHard,
		AutoscaleLoop: autoscale,
	}
}

// Defaults aligned with server/job_executor query workers + HTTP headroom intent.
const (
	DefaultQueryWorkersForPool = 4
	httpConnectionHeadroom     = 16
	cpuPerConnectionHeuristic  = 2 // add this many pool slots per CPU (beyond base)
	poolMinFloor               = 8
	poolMaxCeiling             = 512
	// bytesPerPoolConn is a rough budget for cgroup-based caps (connections * overhead).
	bytesPerPoolConn = 128 << 20 // 128 MiB per connection (conservative)
)

// PoolBoundsFromResources computes elastic pool min and hard max from CPU and optional cgroup memory.
func PoolBoundsFromResources(queryWorkers int) (poolMin, poolMaxHard int) {
	if queryWorkers <= 0 {
		queryWorkers = DefaultQueryWorkersForPool
	}
	nCPU := runtime.NumCPU()
	if nCPU < 1 {
		nCPU = 1
	}
	// Baseline: workers + static headroom + scaled CPU term (Dataform-style concurrent HTTP + async jobs).
	poolMaxHard = queryWorkers + httpConnectionHeadroom + cpuPerConnectionHeuristic*nCPU
	poolMaxHard = clampInt(poolMaxHard, poolMinFloor, poolMaxCeiling)

	if maxBytes, ok := tryCgroupMemoryLimitBytes(); ok && maxBytes > 0 {
		memCap := int(maxBytes / bytesPerPoolConn)
		if memCap < 1 {
			memCap = 1
		}
		if memCap < poolMaxHard {
			poolMaxHard = memCap
		}
	}

	poolMin = queryWorkers + 2
	if poolMin < poolMinFloor {
		poolMin = poolMinFloor
	}
	if poolMin > poolMaxHard {
		poolMin = poolMaxHard
	}
	return poolMin, poolMaxHard
}

// ParsePoolEnvClamps reads BQ_EMULATOR_POOL_MIN and BQ_EMULATOR_POOL_MAX (optional) and applies clamps.
func ParsePoolEnvClamps(poolMin, poolMaxHard int) (int, int) {
	if v := strings.TrimSpace(os.Getenv("BQ_EMULATOR_POOL_MIN")); v != "" {
		if n, err := strconv.Atoi(v); err == nil && n > 0 {
			if n > poolMin {
				poolMin = n
			}
		}
	}
	if v := strings.TrimSpace(os.Getenv("BQ_EMULATOR_POOL_MAX")); v != "" {
		if n, err := strconv.Atoi(v); err == nil && n > 0 {
			if n < poolMaxHard {
				poolMaxHard = n
			}
		}
	}
	if poolMin > poolMaxHard {
		poolMin = poolMaxHard
	}
	return poolMin, poolMaxHard
}

func clampInt(v, lo, hi int) int {
	if v < lo {
		return lo
	}
	if v > hi {
		return hi
	}
	return v
}
