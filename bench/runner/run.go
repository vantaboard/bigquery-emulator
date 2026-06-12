package runner

import (
	"context"
	"fmt"
	"os"
	"strings"
	"time"
)

// RunOptions configures a benchmark execution.
type RunOptions struct {
	CasesDir   string
	CaseFilter string
	Targets    []Target
	Timeout    time.Duration
	Baseline   *BaselineFile
	Compare    bool
	// Progress receives human-readable progress lines as the run
	// advances (target startup, per-case setup, per-iteration
	// completions). nil disables progress output.
	Progress func(format string, args ...any)
}

func (o RunOptions) logf(format string, args ...any) {
	if o.Progress != nil {
		o.Progress(format, args...)
	}
}

// Run executes all cases against the configured targets.
func Run(ctx context.Context, opts RunOptions) (RunReport, error) {
	cases, err := LoadCases(opts.CasesDir)
	if err != nil {
		return RunReport{}, err
	}
	if opts.CaseFilter != "" {
		filtered := cases[:0]
		for _, c := range cases {
			if c.Name == opts.CaseFilter {
				filtered = append(filtered, c)
			}
		}
		if len(filtered) == 0 {
			return RunReport{}, fmt.Errorf("case %q not found", opts.CaseFilter)
		}
		cases = filtered
	}
	timeout := opts.Timeout
	if timeout <= 0 {
		timeout = time.Duration(defaultTimeoutMS) * time.Millisecond
	}

	report := RunReport{
		Timestamp: time.Now().UTC(),
		CommitSHA: os.Getenv("GITHUB_SHA"),
		Host:      hostname(),
		Targets:   targetNames(opts.Targets),
	}

	for _, target := range opts.Targets {
		opts.logf("starting target %s...", target.Name())
		startBegan := time.Now()
		if err := target.Start(ctx); err != nil {
			return report, fmt.Errorf("start %s: %w", target.Name(), err)
		}
		opts.logf("target %s ready in %s", target.Name(), time.Since(startBegan).Round(time.Millisecond))
		defer func(t Target) { _ = t.Cleanup(ctx) }(target)
	}

	for ci, c := range cases {
		dataset := datasetForCase(c.Name)
		for _, target := range opts.Targets {
			opts.logf("[%d/%d] %s on %s: setup...", ci+1, len(cases), c.Name, target.Name())
			cr, err := runCase(ctx, opts, target, c, dataset, timeout)
			if err != nil {
				return report, err
			}
			switch cr.Outcome {
			case OutcomeOK:
				opts.logf("[%d/%d] %s on %s: done (p50 %s, %d rows)",
					ci+1, len(cases), c.Name, target.Name(),
					cr.Latency.P50.Round(time.Millisecond), cr.RowCount)
			default:
				opts.logf("[%d/%d] %s on %s: %s (%s)",
					ci+1, len(cases), c.Name, target.Name(), cr.Outcome, cr.Error)
			}
			if opts.Compare && opts.Baseline != nil && target.Name() == TargetEmulator {
				if base, ok := opts.Baseline.Cases[c.Name]; ok {
					pass, reason := CompareToBaseline(c, base, cr)
					cr.Pass = &pass
					cr.CompareReason = reason
					cr.BQTotalP50MS = base.TotalP50MS
					if base.TotalP50MS > 0 && cr.Latency.P50 > 0 {
						cr.Ratio = float64(cr.Latency.P50.Milliseconds()) / float64(base.TotalP50MS)
					}
				} else {
					pass := false
					cr.Pass = &pass
					cr.CompareReason = "no baseline for case"
				}
			}
			if opts.Baseline != nil && cr.Outcome == OutcomeOK && cr.ResultHash != "" {
				if base, ok := opts.Baseline.Cases[c.Name]; ok && base.ResultHash != "" &&
					base.ResultHash != cr.ResultHash {
					cr.Outcome = OutcomeWrongResult
					if target.Name() == TargetEmulator && cr.Pass != nil {
						pass := false
						cr.Pass = &pass
						cr.CompareReason = "result hash mismatch vs baseline"
					}
				}
			}
			report.Results = append(report.Results, cr)
		}
	}
	return report, nil
}

func runCase(ctx context.Context, opts RunOptions, target Target, c Case, dataset string, timeout time.Duration) (CaseResult, error) {
	project := c.ProjectID
	if bt, ok := target.(*BigQueryTarget); ok {
		project = bt.ProjectID()
	}
	dsRef := datasetRef(target.Name(), project, dataset)
	setupBegan := time.Now()
	if err := target.SetupCase(ctx, c, dsRef); err != nil {
		return CaseResult{
			CaseName:    c.Name,
			Target:      target.Name(),
			ContentHash: c.ContentHash,
			Outcome:     OutcomeError,
			Error:       err.Error(),
		}, nil
	}
	opts.logf("    %s on %s: setup done in %s, running %d iterations...",
		c.Name, target.Name(), time.Since(setupBegan).Round(time.Millisecond), c.Iterations)
	_, query := c.Substitute(dsRef, c.ProjectID)

	var samples []time.Duration
	var execSamples []time.Duration
	var phaseIters []map[string]int64
	var last QueryResult
	var lastErr string
	outcome := OutcomeOK

	for i := 0; i < c.Iterations; i++ {
		res, err := target.RunQuery(ctx, c, query, timeout)
		last = res
		if err != nil {
			if ctx.Err() == context.DeadlineExceeded {
				outcome = OutcomeTimeout
				lastErr = "timeout"
				break
			}
			outcome = OutcomeError
			lastErr = res.Error
			if lastErr == "" {
				lastErr = err.Error()
			}
			break
		}
		label := ""
		if i < c.Warmup {
			label = " (warmup)"
		}
		opts.logf("    %s on %s: iteration %d/%d%s took %s",
			c.Name, target.Name(), i+1, c.Iterations, label,
			res.Elapsed.Round(time.Millisecond))
		samples = append(samples, res.Elapsed)
		if res.ExecutionOnly > 0 {
			execSamples = append(execSamples, res.ExecutionOnly)
		}
		if len(res.Phases) > 0 {
			phaseIters = append(phaseIters, res.Phases)
		}
	}

	cr := CaseResult{
		CaseName:       c.Name,
		Target:         target.Name(),
		ContentHash:    c.ContentHash,
		Outcome:        outcome,
		Error:          lastErr,
		Latency:        ComputeLatencyStats(samples, c.Warmup),
		Phases:         ComputePhaseStats(phaseIters, c.Warmup),
		Route:          last.Route,
		ResultHash:     last.ResultHash,
		RowCount:       last.RowCount,
		BytesProcessed: last.BytesProcessed,
	}
	if len(execSamples) > 0 {
		cr.ExecutionP50 = ComputeLatencyStats(execSamples, c.Warmup).P50
	}
	return cr, nil
}

func datasetForCase(name string) string {
	return "ds_" + strings.ReplaceAll(name, "-", "_")
}

func datasetRef(target TargetName, project, dataset string) string {
	if target == TargetBigQuery {
		return project + "." + dataset
	}
	return dataset
}

func targetNames(targets []Target) []TargetName {
	out := make([]TargetName, len(targets))
	for i, t := range targets {
		out[i] = t.Name()
	}
	return out
}

func hostname() string {
	h, err := os.Hostname()
	if err != nil {
		return ""
	}
	return h
}
