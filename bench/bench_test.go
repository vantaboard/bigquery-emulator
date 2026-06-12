//go:build bench

package bench_test

import (
	"context"
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/bench/runner"
)

func BenchmarkSelectLiteral(b *testing.B) {
	runBenchmarkCase(b, "select_literal")
}

func BenchmarkAggGroupBy100k(b *testing.B) {
	runBenchmarkCase(b, "agg_group_by_100k")
}

func runBenchmarkCase(b *testing.B, name string) {
	if os.Getenv("BIGQUERY_EMULATOR_BIN") == "" {
		if _, err := os.Stat(filepath.Join("..", "bin", "emulator_main")); err != nil {
			b.Skip("emulator_main not built")
		}
	}
	c, err := runner.LoadCase(filepath.Join("cases", name+".yaml"))
	if err != nil {
		b.Fatal(err)
	}
	c.Iterations = b.N
	c.Warmup = 0
	target := runner.NewEmulatorTarget(runner.TargetOptions{
		EngineBinary: os.Getenv("BIGQUERY_EMULATOR_BIN"),
	})
	ctx := context.Background()
	if err := target.Start(ctx); err != nil {
		b.Fatal(err)
	}
	defer target.Cleanup(ctx)
	dataset := "ds_" + name
	if err := target.SetupCase(ctx, c, dataset); err != nil {
		b.Fatal(err)
	}
	_, query := c.Substitute(dataset, c.ProjectID)
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if _, err := target.RunQuery(ctx, c, query, 0); err != nil {
			b.Fatal(err)
		}
	}
}
