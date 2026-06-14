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
	cases, err = filterCases(cases, opts.CaseFilter)
	if err != nil {
		return RunReport{}, err
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
		runCaseAcrossTargets(ctx, opts, &report, cases, ci, c, timeout)
	}
	return report, nil
}

func runCaseAcrossTargets(
	ctx context.Context,
	opts RunOptions,
	report *RunReport,
	cases []Case,
	ci int,
	c Case,
	timeout time.Duration,
) {
	dataset := datasetForCase(c.Name)
	for _, target := range opts.Targets {
		cr, run := prepareCaseRun(ctx, opts, ci, len(cases), c, target)
		if !run {
			report.Results = append(report.Results, cr)
			continue
		}
		opts.logf("[%d/%d] %s on %s: setup...", ci+1, len(cases), c.Name, target.Name())
		cr = runCase(ctx, opts, target, c, dataset, timeout)
		if gt, ok := target.(*GoccyTarget); ok && cr.Outcome == OutcomeError {
			_ = gt.EnsureReady(ctx)
		}
		logCaseResult(opts, ci+1, len(cases), c, target, cr)
		cr = enrichWithBaseline(opts, target, c, cr)
		report.Results = append(report.Results, cr)
	}
}

// prepareCaseRun handles skip and goccy health checks. The bool is
// false when the caller should append cr and continue without running.
func prepareCaseRun(
	ctx context.Context,
	opts RunOptions,
	index, total int,
	c Case,
	target Target,
) (CaseResult, bool) {
	if skipped, reason := c.SkippedFor(target.Name()); skipped {
		cr := CaseResult{
			CaseName:    c.Name,
			Target:      target.Name(),
			ContentHash: c.ContentHash,
			Outcome:     OutcomeSkipped,
			Error:       reason,
		}
		logCaseResult(opts, index+1, total, c, target, cr)
		return cr, false
	}
	if gt, ok := target.(*GoccyTarget); ok {
		if err := gt.EnsureReady(ctx); err != nil {
			cr := CaseResult{
				CaseName:    c.Name,
				Target:      target.Name(),
				ContentHash: c.ContentHash,
				Outcome:     OutcomeError,
				Error:       fmt.Sprintf("goccy not ready: %v", err),
			}
			logCaseResult(opts, index+1, total, c, target, cr)
			return cr, false
		}
	}
	return CaseResult{}, true
}

func filterCases(cases []Case, name string) ([]Case, error) {
	if name == "" {
		return cases, nil
	}
	filtered := cases[:0]
	for _, c := range cases {
		if c.Name == name {
			filtered = append(filtered, c)
		}
	}
	if len(filtered) == 0 {
		return nil, fmt.Errorf("case %q not found", name)
	}
	return filtered, nil
}

func logCaseResult(opts RunOptions, index, total int, c Case, target Target, cr CaseResult) {
	switch cr.Outcome {
	case OutcomeOK:
		opts.logf("[%d/%d] %s on %s: done (p50 %s, %d rows)",
			index, total, c.Name, target.Name(),
			cr.Latency.P50.Round(time.Millisecond), cr.RowCount)
	case OutcomeSkipped:
		opts.logf("[%d/%d] %s on %s: skipped (%s)",
			index, total, c.Name, target.Name(), cr.Error)
	default:
		opts.logf("[%d/%d] %s on %s: %s (%s)",
			index, total, c.Name, target.Name(), cr.Outcome, cr.Error)
	}
}

func enrichWithBaseline(opts RunOptions, target Target, c Case, cr CaseResult) CaseResult {
	if opts.Compare && opts.Baseline != nil && target.Name() == TargetEmulator {
		if base, ok := opts.Baseline.Cases[c.Name]; ok {
			pass, reason := CompareToBaseline(c, base, cr)
			cr.Pass = &pass
			cr.CompareReason = reason
			cr.BQExecutionP50MS = base.LatencyP50MS()
			bqDenom := base.LatencyP50ForRatio()
			emuNum := cr.CompareLatencyMSForRatio()
			if bqDenom > 0 && emuNum > 0 {
				cr.Ratio = float64(emuNum) / float64(bqDenom)
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
	return cr
}

func runCase(
	ctx context.Context,
	opts RunOptions,
	target Target,
	c Case,
	dataset string,
	timeout time.Duration,
) CaseResult {
	project := c.ProjectID
	switch tt := target.(type) {
	case *BigQueryTarget:
		project = tt.ProjectID()
	case *GoccyTarget:
		project = goccyProject
	}
	dsRef := datasetRef(target.Name(), project, dataset)
	setupBegan := time.Now()
	if setupErr := target.SetupCase(ctx, c, dsRef); setupErr != nil {
		return CaseResult{
			CaseName:    c.Name,
			Target:      target.Name(),
			ContentHash: c.ContentHash,
			Outcome:     OutcomeError,
			Error:       setupErr.Error(),
		}
	}
	opts.logf("    %s on %s: setup done in %s, running %d iterations...",
		c.Name, target.Name(), time.Since(setupBegan).Round(time.Millisecond), c.Iterations)
	_, query := c.Substitute(dsRef, project)
	caseTimeout := c.QueryTimeout(timeout)

	samples, execSamples, queueSamples, slotSamples, phaseIters, last, outcome, lastErr := runQueryIterations(
		ctx, opts, target, c, query, caseTimeout)

	phases := ComputePhaseStats(phaseIters, c.Warmup)
	cr := CaseResult{
		CaseName:       c.Name,
		Target:         target.Name(),
		ContentHash:    c.ContentHash,
		Outcome:        outcome,
		Error:          lastErr,
		Latency:        ComputeLatencyStats(samples, c.Warmup),
		Phases:         phases,
		EngineP50:      EngineP50FromPhases(phases),
		Route:          last.Route,
		ResultHash:     last.ResultHash,
		RowCount:       last.RowCount,
		BytesProcessed: last.BytesProcessed,
	}
	if len(execSamples) > 0 {
		cr.ExecutionP50 = ComputeLatencyStats(execSamples, c.Warmup).P50
	}
	if len(queueSamples) > 0 {
		cr.QueueP50 = ComputeLatencyStats(queueSamples, c.Warmup).P50
	}
	if len(slotSamples) > 0 {
		cr.TotalSlotMsP50 = ComputeInt64P50(slotSamples, c.Warmup)
	}
	return cr
}

func runQueryIterations(
	ctx context.Context,
	opts RunOptions,
	target Target,
	c Case,
	query string,
	timeout time.Duration,
) (
	samples, execSamples, queueSamples []time.Duration,
	slotSamples []int64,
	phaseIters []map[string]int64,
	last QueryResult,
	outcome Outcome,
	lastErr string,
) {
	outcome = OutcomeOK
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
		logQueryIteration(opts, c, target, i, res)
		samples = append(samples, res.Elapsed)
		if res.ExecutionValid {
			execSamples = append(execSamples, res.ExecutionOnly)
		}
		if res.QueueOnly > 0 {
			queueSamples = append(queueSamples, res.QueueOnly)
		}
		if res.ExecutionValid {
			slotSamples = append(slotSamples, res.SlotMs)
		}
		if len(res.Phases) > 0 {
			phaseIters = append(phaseIters, res.Phases)
		}
	}
	return samples, execSamples, queueSamples, slotSamples, phaseIters, last, outcome, lastErr
}

func logQueryIteration(opts RunOptions, c Case, target Target, i int, res QueryResult) {
	label := ""
	if i < c.Warmup {
		label = " (warmup)"
	}
	opts.logf("    %s on %s: iteration %d/%d%s took %s",
		c.Name, target.Name(), i+1, c.Iterations, label,
		res.Elapsed.Round(time.Millisecond))
	if target.Name() == TargetBigQuery && res.ExecutionValid {
		clientOverhead := res.Elapsed - res.ExecutionOnly
		opts.logf("      bq stats: execution=%s queue=%s slot_ms=%d client_overhead=%s",
			res.ExecutionOnly.Round(time.Millisecond),
			res.QueueOnly.Round(time.Millisecond),
			res.SlotMs,
			clientOverhead.Round(time.Millisecond))
	}
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
