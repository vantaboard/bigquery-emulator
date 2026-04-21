package connection

import "testing"

func TestPoolBoundsFromResources(t *testing.T) {
	min, max := PoolBoundsFromResources(DefaultQueryWorkersForPool)
	if min < 1 || max < min {
		t.Fatalf("min=%d max=%d", min, max)
	}
	if max > poolMaxCeiling {
		t.Fatalf("max %d above ceiling", max)
	}
}

func TestParsePoolEnvClamps(t *testing.T) {
	t.Run("no_env", func(t *testing.T) {
		t.Setenv("BQ_EMULATOR_POOL_MIN", "")
		t.Setenv("BQ_EMULATOR_POOL_MAX", "")
		min, max := ParsePoolEnvClamps(10, 100)
		if min != 10 || max != 100 {
			t.Fatalf("got min=%d max=%d", min, max)
		}
	})
	t.Run("clamps", func(t *testing.T) {
		t.Setenv("BQ_EMULATOR_POOL_MIN", "20")
		t.Setenv("BQ_EMULATOR_POOL_MAX", "50")
		min, max := ParsePoolEnvClamps(10, 100)
		if min != 20 || max != 50 {
			t.Fatalf("got min=%d max=%d", min, max)
		}
	})
}

func TestPoolConfigFromEnvFixed(t *testing.T) {
	t.Setenv("BQ_EMULATOR_POOL_SIZE", "7")
	t.Setenv("BQ_EMULATOR_POOL_AUTOSCALE", "0")

	cfg := PoolConfigFromEnv()
	if cfg.FixedSize != 7 || cfg.PoolMin != 7 || cfg.PoolMaxHard != 7 || cfg.AutoscaleLoop {
		t.Fatalf("%+v", cfg)
	}
}
