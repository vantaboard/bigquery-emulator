package googlesqlcorpus

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const (
	BucketEngineBug         = "engine-bug"
	BucketNotYetLanded      = "not-yet-landed-route"
	BucketFeatureOutOfScope = "corpus-feature-out-of-scope"
	BucketPinnedPass        = "pinned-pass"
)

// Options configures a corpus run.
type Options struct {
	CorpusDir  string
	Manifest   *Manifest
	GatePinned bool
	TriageMode bool
	Harness    runner.HarnessOptions
	Profile    string
	ProjectID  string
	DatasetID  string
	Out        io.Writer
	Err        io.Writer
}

// Result is one case outcome.
type Result struct {
	ID         string `json:"id"`
	File       string `json:"file"`
	Name       string `json:"name"`
	Status     string `json:"status"`
	Bucket     string `json:"bucket,omitempty"`
	Message    string `json:"message,omitempty"`
	Diff       string `json:"diff,omitempty"`
	DurationMs int64  `json:"duration_ms"`
}

// Report aggregates a corpus invocation.
type Report struct {
	Summary struct {
		Total   int `json:"total"`
		Passed  int `json:"passed"`
		Failed  int `json:"failed"`
		Skipped int `json:"skipped"`
	} `json:"summary"`
	Results []Result `json:"results"`
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

// LoadCorpusDir parses every .test file under dir.
func LoadCorpusDir(dir string) ([]TestCase, error) {
	var cases []TestCase
	err := filepath.WalkDir(dir, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if d.IsDir() || !strings.HasSuffix(path, ".test") {
			return nil
		}
		b, err := os.ReadFile(path) //nolint:gosec // corpus dir is CLI-controlled
		if err != nil {
			return err
		}
		tf, err := ParseFile(path, string(b))
		if err != nil {
			return err
		}
		cases = append(cases, tf.Cases...)
		return nil
	})
	return cases, err
}

func normalizeOptions(opts *Options) {
	if opts.CorpusDir == "" {
		opts.CorpusDir = "conformance/googlesql-corpus/corpus"
	}
	if opts.Manifest == nil {
		opts.Manifest = &Manifest{}
	}
	if opts.TriageMode && opts.Manifest.Triage == nil {
		opts.Manifest.Triage = make(map[string]TriageEntry)
	}
	if opts.Profile == "" {
		opts.Profile = runner.ProfileDuckDB
	}
	if opts.ProjectID == "" {
		opts.ProjectID = "googlesql-corpus"
	}
	if opts.DatasetID == "" {
		opts.DatasetID = "ds1"
	}
	if opts.Out == nil {
		opts.Out = os.Stdout
	}
	if opts.Err == nil {
		opts.Err = os.Stderr
	}
}

// Run executes the corpus against the emulator.
func Run(ctx context.Context, opts Options) (*Report, error) {
	normalizeOptions(&opts)

	cases, err := LoadCorpusDir(opts.CorpusDir)
	if err != nil {
		return nil, err
	}

	profile, ok := runner.LookupProfile(opts.Profile)
	if !ok {
		return nil, fmt.Errorf("unknown profile %q", opts.Profile)
	}

	env, err := runner.StartEmulator(ctx, opts.Harness, profile)
	if err != nil {
		return nil, fmt.Errorf("start emulator: %w", err)
	}
	defer func() { _ = env.Close() }()

	base := env.BaseURL + "/bigquery/v2/projects/" + opts.ProjectID
	if err := seedDataset(ctx, base, opts.DatasetID); err != nil {
		return nil, fmt.Errorf("seed dataset: %w", err)
	}

	report := &Report{}
	for _, tc := range cases {
		res := runCase(ctx, base, opts, tc)
		report.Results = append(report.Results, res)
		report.Summary.Total++
		switch res.Status {
		case string(runner.StatusPass):
			report.Summary.Passed++
		case string(runner.StatusFail):
			report.Summary.Failed++
		case string(runner.StatusSkip):
			report.Summary.Skipped++
		}
		_, _ = fmt.Fprintf(opts.Out, "%s %s %s\n", res.Status, res.ID, res.Message)
		if res.Diff != "" {
			_, _ = fmt.Fprintf(opts.Out, "%s\n", res.Diff)
		}
	}
	return report, nil
}

func seedDataset(ctx context.Context, base, dataset string) error {
	body := fmt.Sprintf(
		`{"datasetReference":{"projectId":"%s","datasetId":"%s"},"location":"US"}`,
		projectIDFromBase(base), dataset)
	status, respBody, err := runner.DoRequest(ctx, base+"/datasets", []byte(body))
	if err != nil {
		return err
	}
	if status == 409 {
		return nil
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("datasets.insert -> %d: %s", status, string(respBody))
	}
	return nil
}

func runCase(ctx context.Context, base string, opts Options, tc TestCase) Result {
	started := time.Now()
	res := baseResult(tc)
	if skip, ok := skipCase(tc, opts); ok {
		return finish(skip, started)
	}

	status, body, err := runner.QueryViaGateway(ctx, base, tc.SQL)
	if err != nil {
		res.Message = "query rpc: " + err.Error()
		res.Bucket = BucketEngineBug
		return finish(res, started)
	}
	if status < 200 || status >= 300 {
		res.Message = fmt.Sprintf("query failed HTTP %d", status)
		res.Diff = string(body)
		res.Bucket = classifyFailure(tc, res.Message)
		return finish(res, started)
	}

	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		res.Message = "decode response: " + err.Error()
		res.Bucket = BucketEngineBug
		return finish(res, started)
	}

	cols := schemaColumns(run.Schema)
	exp := runner.Expectation{
		Match: chooseMatch(tc.Expected.Ordered),
		Rows:  ToRunnerRows(tc.Expected.Rows, cols),
	}
	if diff := runner.CompareRows(exp, run.Schema, run.Rows); diff != "" {
		res.Message = "row mismatch"
		res.Diff = diff
		res.Bucket = BucketEngineBug
		return finish(res, started)
	}

	res.Status = string(runner.StatusPass)
	res.Bucket = BucketPinnedPass
	if opts.TriageMode {
		opts.Manifest.Triage[res.ID] = TriageEntry{Bucket: BucketPinnedPass}
	}
	return finish(res, started)
}

func baseResult(tc TestCase) Result {
	return Result{
		ID:     CaseID(tc),
		File:   tc.File,
		Name:   tc.Name,
		Status: string(runner.StatusFail),
	}
}

func skipCase(tc TestCase, opts Options) (Result, bool) {
	res := baseResult(tc)
	switch {
	case tc.PrepareDatabase:
		res.Status = string(runner.StatusSkip)
		res.Bucket = BucketNotYetLanded
		res.Message = "prepare_database seeding not yet implemented"
		return res, true
	case tc.ExpectError != "":
		res.Status = string(runner.StatusSkip)
		res.Bucket = BucketFeatureOutOfScope
		res.Message = "error-expectation cases deferred in starter lane"
		return res, true
	default:
		if ok, why := opts.Manifest.ShouldRun(tc, opts.GatePinned); !ok {
			res.Status = string(runner.StatusSkip)
			res.Bucket = BucketFeatureOutOfScope
			res.Message = why
			return res, true
		}
	}
	return Result{}, false
}

func chooseMatch(ordered bool) runner.MatchMode {
	if ordered {
		return runner.MatchOrdered
	}
	return runner.MatchUnordered
}

func classifyFailure(tc TestCase, msg string) string {
	lower := strings.ToLower(msg)
	if strings.Contains(lower, "unimplemented") || strings.Contains(lower, "not implemented") {
		return BucketNotYetLanded
	}
	for _, f := range tc.RequiredFeatures {
		for _, skip := range []string{"PROTO", "JSON", "GRAPH", "PIPE", "MATCH_RECOGNIZE"} {
			if strings.Contains(f, skip) {
				return BucketFeatureOutOfScope
			}
		}
	}
	return BucketEngineBug
}

func finish(r Result, started time.Time) Result {
	r.DurationMs = time.Since(started).Milliseconds()
	return r
}

func schemaColumns(schema *bqtypes.TableSchema) []string {
	if schema == nil {
		return nil
	}
	out := make([]string, len(schema.Fields))
	for i, f := range schema.Fields {
		out[i] = f.Name
	}
	return out
}

func projectIDFromBase(base string) string {
	const marker = "/projects/"
	i := strings.LastIndex(base, marker)
	if i < 0 {
		return ""
	}
	rest := base[i+len(marker):]
	if before, _, ok := strings.Cut(rest, "/"); ok {
		return before
	}
	return rest
}
