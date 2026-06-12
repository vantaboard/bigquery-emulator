package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/bench/runner"
)

func main() {
	var (
		targetFlag   = flag.String("target", "emulator", "emulator, goccy, bigquery, or all")
		casesDir     = flag.String("cases", defaultCasesDir(), "directory of benchmark YAML cases")
		baselinePath = flag.String("baseline", defaultBaselinePath(), "path to bigquery.json baseline")
		capture      = flag.Bool("capture", false, "capture baseline (requires --target=bigquery)")
		compare      = flag.Bool("compare", false, "compare emulator results to committed baseline")
		jsonOut      = flag.String("json-out", "", "write machine-readable results JSON")
		project      = flag.String("project", os.Getenv("BENCH_BQ_PROJECT"), "BigQuery project for capture")
		goccyImage   = flag.String("goccy-image", "", "docker image for goccy emulator")
		caseFilter   = flag.String("case", "", "run a single case by name")
		engineBin    = flag.String("engine-binary", "", "path to emulator_main")
		skipGoccy    = flag.Bool(
			"skip-goccy",
			os.Getenv("BENCH_SKIP_GOCCY") == "1",
			"skip goccy target when --target=all",
		)
		quiet = flag.Bool("quiet", false, "suppress per-case progress logging on stderr")
	)
	flag.Parse()
	if err := run(context.Background(), config{
		target:       *targetFlag,
		casesDir:     *casesDir,
		baselinePath: *baselinePath,
		capture:      *capture,
		compare:      *compare,
		jsonOut:      *jsonOut,
		project:      *project,
		goccyImage:   *goccyImage,
		caseFilter:   *caseFilter,
		engineBin:    *engineBin,
		skipGoccy:    *skipGoccy,
		quiet:        *quiet,
	}); err != nil {
		fmt.Fprintf(os.Stderr, "bench: %v\n", err)
		os.Exit(1)
	}
}

type config struct {
	target       string
	casesDir     string
	baselinePath string
	capture      bool
	compare      bool
	jsonOut      string
	project      string
	goccyImage   string
	caseFilter   string
	engineBin    string
	skipGoccy    bool
	quiet        bool
}

func run(ctx context.Context, cfg config) error {
	opts := runner.TargetOptions{
		EngineBinary: cfg.engineBin,
		GoccyImage:   cfg.goccyImage,
		BQProject:    cfg.project,
	}
	targets, err := resolveTargets(cfg.target, opts, cfg.skipGoccy)
	if err != nil {
		return err
	}
	baseline := loadBaseline(cfg)
	progress := func(format string, args ...any) {
		_, _ = fmt.Fprintf(os.Stderr, "%s bench: %s\n",
			time.Now().Format("15:04:05"), fmt.Sprintf(format, args...))
	}
	if cfg.quiet {
		progress = nil
	}
	report, err := runner.Run(ctx, runner.RunOptions{
		CasesDir:   cfg.casesDir,
		CaseFilter: cfg.caseFilter,
		Targets:    targets,
		Baseline:   baseline,
		Compare:    cfg.compare,
		Progress:   progress,
	})
	if err != nil {
		return err
	}
	if err := writeRunOutputs(cfg, targets, report, baseline); err != nil {
		return err
	}
	return enforceCompareGate(cfg, report)
}

func loadBaseline(cfg config) *runner.BaselineFile {
	if !cfg.compare && !cfg.capture {
		return nil
	}
	loaded, loadErr := runner.LoadBaseline(cfg.baselinePath)
	if loadErr != nil && cfg.compare {
		return &runner.BaselineFile{Cases: map[string]runner.BaselineCase{}}
	}
	if loadErr == nil {
		return &loaded
	}
	return nil
}

func writeRunOutputs(
	cfg config,
	targets []runner.Target,
	report runner.RunReport,
	baseline *runner.BaselineFile,
) error {
	if cfg.goccyImage != "" {
		report.GoccyImage = cfg.goccyImage
	} else if !cfg.skipGoccy && containsTarget(targets, runner.TargetGoccy) {
		report.GoccyImage = runner.ImageTag(runner.DefaultGoccyImage())
	}
	runner.PrintTextReport(os.Stdout, report, baseline)
	if cfg.jsonOut != "" {
		if err := runner.SaveReport(cfg.jsonOut, report); err != nil {
			return err
		}
	}
	if !cfg.capture {
		return nil
	}
	if cfg.project == "" {
		return errors.New("--project or BENCH_BQ_PROJECT required for capture")
	}
	b := runner.BuildBaselineFromResults(cfg.project, report.Results)
	if err := runner.SaveBaseline(cfg.baselinePath, b); err != nil {
		return err
	}
	_, _ = fmt.Fprintf(os.Stdout, "wrote baseline %s (%d cases)\n", cfg.baselinePath, len(b.Cases))
	return nil
}

func enforceCompareGate(cfg config, report runner.RunReport) error {
	if !cfg.compare {
		return nil
	}
	fail := 0
	for _, r := range report.Results {
		if r.Target == runner.TargetEmulator && r.Pass != nil && !*r.Pass {
			fail++
		}
	}
	if fail > 0 {
		return fmt.Errorf("%d emulator case(s) failed compare gate", fail)
	}
	return nil
}

func resolveTargets(name string, opts runner.TargetOptions, skipGoccy bool) ([]runner.Target, error) {
	name = strings.ToLower(strings.TrimSpace(name))
	switch name {
	case "emulator":
		return []runner.Target{runner.NewEmulatorTarget(opts)}, nil
	case "goccy":
		return []runner.Target{runner.NewGoccyTarget(opts)}, nil
	case "bigquery", "bq":
		return []runner.Target{runner.NewBigQueryTarget(opts)}, nil
	case "all":
		var out []runner.Target
		out = append(out, runner.NewEmulatorTarget(opts))
		if !skipGoccy {
			out = append(out, runner.NewGoccyTarget(opts))
		}
		return out, nil
	case "compare":
		return []runner.Target{runner.NewEmulatorTarget(opts)}, nil
	default:
		return nil, fmt.Errorf("unknown target %q", name)
	}
}

func containsTarget(targets []runner.Target, name runner.TargetName) bool {
	for _, t := range targets {
		if t.Name() == name {
			return true
		}
	}
	return false
}

func defaultCasesDir() string {
	root, err := repoRoot()
	if err != nil {
		return "bench/cases"
	}
	return filepath.Join(root, "bench", "cases")
}

func defaultBaselinePath() string {
	root, err := repoRoot()
	if err != nil {
		return "bench/baselines/bigquery.json"
	}
	return filepath.Join(root, "bench", "baselines", "bigquery.json")
}

func repoRoot() (string, error) {
	wd, err := os.Getwd()
	if err != nil {
		return "", err
	}
	dir := wd
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", fmt.Errorf("go.mod not found from %s", wd)
		}
		dir = parent
	}
}
