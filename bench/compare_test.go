//go:build bench

package bench_test

import (
	"context"
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/bench/runner"
)

func TestCompareAgainstBaseline(t *testing.T) {
	if os.Getenv("BIGQUERY_EMULATOR_BIN") == "" {
		if _, err := os.Stat(filepath.Join("..", "bin", "emulator_main")); err != nil {
			t.Skip("emulator_main not built; run task emulator:build-engine:bazel")
		}
	}
	baselinePath := filepath.Join("baselines", "bigquery.json")
	baseline, err := runner.LoadBaseline(baselinePath)
	if err != nil {
		t.Skipf("no baseline at %s: %v", baselinePath, err)
	}
	target := runner.NewEmulatorTarget(runner.TargetOptions{
		EngineBinary: os.Getenv("BIGQUERY_EMULATOR_BIN"),
	})
	report, err := runner.Run(context.Background(), runner.RunOptions{
		CasesDir: filepath.Join("cases"),
		Targets:  []runner.Target{target},
		Baseline: &baseline,
		Compare:  true,
	})
	if err != nil {
		t.Fatal(err)
	}
	for _, r := range report.Results {
		if r.Target != runner.TargetEmulator {
			continue
		}
		if r.Pass != nil && !*r.Pass {
			t.Errorf("case %s failed compare: %s", r.CaseName, r.CompareReason)
		}
	}
}
