package differential

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const JSONSchemaVersion = 1

// Options configures a differential replay invocation.
type Options struct {
	CorpusDir       string
	OracleDir       string
	IncludeSelfTest bool
	Harness         runner.HarnessOptions
	Profile         string
	Output          string
	Out             io.Writer
	Err             io.Writer
}

// Result is one corpus case outcome.
type Result struct {
	Case         string         `json:"case"`
	Path         string         `json:"path"`
	Profile      string         `json:"profile"`
	Status       runner.Status  `json:"status"`
	Divergence   DivergenceKind `json:"divergence,omitempty"`
	KnownFailing bool           `json:"known_failing,omitempty"`
	OracleSource string         `json:"oracle_source,omitempty"`
	DurationMs   int64          `json:"duration_ms"`
	Message      string         `json:"message,omitempty"`
	Diff         string         `json:"diff,omitempty"`
}

// Report aggregates a differential lane run.
type Report struct {
	SchemaVersion int            `json:"schema_version"`
	Summary       runner.Summary `json:"summary"`
	Results       []Result       `json:"results"`
}

// ExitCode mirrors the fixture runner semantics.
func (r *Report) ExitCode() int {
	if r == nil {
		return 2
	}
	if r.Summary.Failed > 0 {
		return 1
	}
	return 0
}

// Run replays every corpus case against the emulator and diffs vs committed oracles.
func Run(ctx context.Context, opts Options) (*Report, error) {
	normalizeOptions(&opts)

	cases, err := LoadCorpusDir(opts.CorpusDir, opts.IncludeSelfTest)
	if err != nil {
		return nil, err
	}
	profile, ok := runner.LookupProfile(opts.Profile)
	if !ok {
		return nil, fmt.Errorf("unknown profile %q", opts.Profile)
	}

	report := &Report{SchemaVersion: JSONSchemaVersion}
	for _, c := range cases {
		res := runCase(ctx, opts, profile, c)
		report.Results = append(report.Results, res)
		report.Summary.Total++
		switch res.Status {
		case runner.StatusPass:
			report.Summary.Passed++
		case runner.StatusFail:
			report.Summary.Failed++
		case runner.StatusSkip:
			report.Summary.Skipped++
		}
	}
	if err := renderReport(opts, report); err != nil {
		return report, err
	}
	return report, nil
}

func normalizeOptions(opts *Options) {
	if opts.CorpusDir == "" {
		opts.CorpusDir = DefaultCorpusDir
	}
	if opts.OracleDir == "" {
		opts.OracleDir = DefaultOracleDir
	}
	if opts.Profile == "" {
		opts.Profile = runner.ProfileDuckDB
	}
	if opts.Output == "" {
		opts.Output = "text"
	}
	if opts.Out == nil {
		opts.Out = os.Stdout
	}
	if opts.Err == nil {
		opts.Err = os.Stderr
	}
}

func runCase(ctx context.Context, opts Options, profile runner.Profile, c *CorpusCase) Result {
	started := time.Now()
	res := Result{
		Case:         c.Name,
		Path:         c.Path,
		Profile:      profile.Name,
		Status:       runner.StatusFail,
		KnownFailing: c.KnownFailing,
		OracleSource: c.OracleSource,
	}

	oracle, err := LoadOracle(opts.OracleDir, c.OracleRef)
	if err != nil {
		res.Message = "load oracle: " + err.Error()
		res.Divergence = KindCrash
		return finish(res, started)
	}
	if res.OracleSource == "" {
		res.OracleSource = oracle.OracleSource
	}

	env, startErr := runner.StartEmulator(ctx, opts.Harness, profile)
	if startErr != nil {
		res.Message = "start emulator: " + startErr.Error()
		res.Divergence = KindCrash
		return finish(res, started)
	}
	defer func() { _ = env.Close() }()

	base := env.BaseURL + "/bigquery/v2/projects/" + c.ProjectID
	if setupErr := runner.RunSetupSteps(ctx, base, c.Setup, c.DefaultDataset); setupErr != nil {
		res.Message = setupErr.Error()
		res.Divergence = ClassifyDivergence(ClassifyInput{RunnerMessage: res.Message})
		return finishMaybeKnown(res, started, c.KnownFailing)
	}

	params := toWireParams(c.QueryParameters)
	queryBody, err := runner.MarshalJobsQueryBody(c.Query, c.DefaultDataset, params)
	if err != nil {
		res.Message = err.Error()
		res.Divergence = KindCrash
		return finish(res, started)
	}
	status, body, queryErr := runner.DoRequest(ctx, base+"/queries", queryBody)
	if queryErr != nil {
		res.Message = "query rpc: " + queryErr.Error()
		res.Divergence = ClassifyDivergence(ClassifyInput{RunnerMessage: res.Message})
		return finishMaybeKnown(res, started, c.KnownFailing)
	}
	return compareAgainstOracle(res, oracle, c, status, body, started)
}

func compareAgainstOracle(
	res Result,
	oracle *Oracle,
	c *CorpusCase,
	status int,
	body []byte,
	started time.Time,
) Result {
	exp := ExpectationFromOracle(oracle, c.Match)
	emulatorSuccess := status >= 200 && status < 300

	if exp.Error != nil {
		return compareErrorOracle(res, exp, c, status, body, emulatorSuccess, started)
	}
	if !emulatorSuccess {
		res.Message = fmt.Sprintf("query failed with HTTP %d", status)
		res.Diff = "body: " + snippet(body)
		res.Divergence = ClassifyDivergence(ClassifyInput{
			OracleSuccess: true, EmulatorSuccess: false,
			EmulatorStatus: status, EmulatorBody: body,
			Diff: res.Diff, RunnerMessage: res.Message,
		})
		return finishMaybeKnown(res, started, c.KnownFailing)
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		res.Message = "decode QueryResponse: " + err.Error()
		res.Divergence = KindCrash
		return finish(res, started)
	}
	if diff := runner.CompareRows(exp, run.Schema, run.Rows); diff != "" {
		res.Message = "row mismatch"
		res.Diff = diff
		res.Divergence = ClassifyDivergence(ClassifyInput{
			OracleSuccess: true, EmulatorSuccess: true,
			EmulatorStatus: status, EmulatorBody: body,
			Diff: diff, RunnerMessage: res.Message,
		})
		return finishMaybeKnown(res, started, c.KnownFailing)
	}
	res.Status = runner.StatusPass
	res.Divergence = KindMatch
	return finish(res, started)
}

func compareErrorOracle(
	res Result,
	exp runner.Expectation,
	c *CorpusCase,
	status int,
	body []byte,
	emulatorSuccess bool,
	started time.Time,
) Result {
	if emulatorSuccess {
		res.Message = "expected error, got success"
		res.Diff = fmt.Sprintf("status: %d\nbody: %s", status, snippet(body))
	} else if diff := runner.CompareError(*exp.Error, status, body); diff != "" {
		res.Message = "error mismatch"
		res.Diff = diff
	} else {
		res.Status = runner.StatusPass
		res.Divergence = KindMatch
		return finish(res, started)
	}
	res.Divergence = ClassifyDivergence(ClassifyInput{
		OracleSuccess: false, EmulatorSuccess: emulatorSuccess,
		EmulatorStatus: status, EmulatorBody: body,
		Diff: res.Diff, RunnerMessage: res.Message,
	})
	return finishMaybeKnown(res, started, c.KnownFailing)
}

func finishMaybeKnown(res Result, started time.Time, knownFailing bool) Result {
	res = finish(res, started)
	if knownFailing && res.Status == runner.StatusFail {
		res.Status = runner.StatusSkip
		res.Message = "known_failing (expected divergence): " + res.Message
	}
	return res
}

func finish(res Result, started time.Time) Result {
	res.DurationMs = time.Since(started).Milliseconds()
	return res
}

func toWireParams(params []QueryParameterYAML) []bqtypes.QueryParameter {
	if len(params) == 0 {
		return nil
	}
	out := make([]bqtypes.QueryParameter, 0, len(params))
	for _, p := range params {
		paramType := &bqtypes.QueryParameterType{Type: strings.ToUpper(p.Type)}
		paramValue := &bqtypes.QueryParameterValue{Value: p.Value}

		if elem := strings.TrimSpace(p.ArrayElementType); elem != "" {
			paramType.ArrayType = &bqtypes.QueryParameterType{
				Type: strings.ToUpper(elem),
			}
		}
		if len(p.StructFields) > 0 {
			for _, f := range p.StructFields {
				paramType.StructTypes = append(paramType.StructTypes, bqtypes.QueryParameterStructType{
					Name: f.Name,
					Type: bqtypes.QueryParameterType{Type: strings.ToUpper(f.Type)},
				})
			}
		}
		if len(p.ArrayValues) > 0 {
			paramValue.ArrayValues = make([]bqtypes.QueryParameterValue, 0, len(p.ArrayValues))
			for _, v := range p.ArrayValues {
				paramValue.ArrayValues = append(paramValue.ArrayValues, bqtypes.QueryParameterValue{
					Value: v,
				})
			}
			paramValue.Value = ""
		}
		if len(p.StructValues) > 0 {
			paramValue.StructValues = make(map[string]bqtypes.QueryParameterValue, len(p.StructValues))
			for name, v := range p.StructValues {
				paramValue.StructValues[name] = bqtypes.QueryParameterValue{Value: v}
			}
			paramValue.Value = ""
		}

		out = append(out, bqtypes.QueryParameter{
			Name:           p.Name,
			ParameterType:  paramType,
			ParameterValue: paramValue,
		})
	}
	return out
}

func snippet(b []byte) string {
	const limit = 240
	s := strings.TrimSpace(string(b))
	if len(s) > limit {
		s = s[:limit] + "..."
	}
	return s
}

func renderReport(opts Options, report *Report) error {
	switch opts.Output {
	case "json":
		enc := json.NewEncoder(opts.Out)
		enc.SetIndent("", "  ")
		return enc.Encode(report)
	default:
		return renderText(opts.Out, report)
	}
}

func renderText(w io.Writer, report *Report) error {
	_, _ = fmt.Fprintf(w, "differential summary: %d total, %d passed, %d failed, %d skipped\n",
		report.Summary.Total, report.Summary.Passed, report.Summary.Failed, report.Summary.Skipped)
	for _, r := range report.Results {
		line := fmt.Sprintf("%s %s %s", r.Status, r.Case, r.Divergence)
		if r.Message != "" {
			line += " — " + r.Message
		}
		_, _ = fmt.Fprintln(w, line)
		if r.Diff != "" {
			_, _ = fmt.Fprintln(w, r.Diff)
		}
	}
	return nil
}
