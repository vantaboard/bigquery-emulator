package runner

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"slices"
	"sort"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// Status is the per-fixture verdict the runner emits.
type Status string

const (
	StatusPass Status = "PASS"
	StatusFail Status = "FAIL"
	StatusSkip Status = "SKIP"
)

// JSONSchemaVersion is the on-the-wire `schema_version` plan-41 CI
// pivots on. Bumped only on a breaking output-shape change.
const JSONSchemaVersion = 1

// outputFormatText is the runner's default --output format: a
// human-readable text renderer. Hoisted to a const so the default,
// the validator, and the dispatcher all reference one source of truth.
const outputFormatText = "text"

// Result is one fixture x profile outcome. The JSON tags mirror what
// plan-41's diff CI consumes; keep them stable.
type Result struct {
	Fixture    string `json:"fixture"`
	Path       string `json:"path"`
	Profile    string `json:"profile"`
	Status     Status `json:"status"`
	DurationMs int64  `json:"duration_ms"`
	Message    string `json:"message,omitempty"`
	Diff       string `json:"diff,omitempty"`
}

// Summary is the matrix-level aggregate the JSON output starts with.
type Summary struct {
	Total   int `json:"total"`
	Passed  int `json:"passed"`
	Failed  int `json:"failed"`
	Skipped int `json:"skipped"`
}

// Report is the top-level JSON payload. `schema_version` is the field
// plan 41's CI key off, so a downstream consumer can refuse a report
// it does not understand without parsing the rest.
type Report struct {
	SchemaVersion int      `json:"schema_version"`
	Summary       Summary  `json:"summary"`
	Results       []Result `json:"results"`
}

// Options bundles the CLI flags the runner needs to do its job. The
// CLI in `conformance/cmd/runner` parses these and hands the Options
// over without further interpretation.
type Options struct {
	// FixturesPath points at the directory (or single file) the
	// runner loads.
	FixturesPath string

	// Harness carries the engine-binary / connect / stdio settings.
	Harness HarnessOptions

	// Profiles restricts which profiles the matrix iterates over.
	// Empty means "all known profiles".
	Profiles []string

	// UpdateBaselines overwrites the `expected:` block of every
	// fixture with the actual response. Used to bootstrap new
	// fixtures (plan 42 leans on this). When true, every fixture
	// is reported as PASS regardless of the original expected
	// block.
	UpdateBaselines bool

	// Output controls the renderer: "text" (default) or "json".
	Output string

	// Out / Err are the writers the renderer dispatches to.
	// Default: os.Stdout / os.Stderr.
	Out io.Writer
	Err io.Writer
}

// Run executes the conformance matrix once and returns the resulting
// Report plus a non-nil error if a runner-internal failure occurred
// (bad YAML, can't start engine, output renderer crashed, etc).
//
// A fixture FAILing returns a non-nil Report with Summary.Failed > 0
// but a nil error. The CLI maps these to the documented exit codes
// (1 vs 2). Callers that want the exit-code semantics call ExitCode
// on the returned Report.
func Run(ctx context.Context, opts Options) (*Report, error) {
	opts, err := prepareOptions(opts)
	if err != nil {
		return nil, err
	}
	fixtures, err := LoadDir(opts.FixturesPath)
	if err != nil {
		return nil, err
	}
	if len(fixtures) == 0 {
		return nil, fmt.Errorf("no fixtures found under %s", opts.FixturesPath)
	}
	enabled, err := resolveProfiles(opts.Profiles)
	if err != nil {
		return nil, err
	}
	report := iterateMatrix(ctx, fixtures, enabled, opts)
	if opts.Output == "json" {
		if err := writeJSONReport(opts.Out, report); err != nil {
			return report, fmt.Errorf("write json report: %w", err)
		}
	} else {
		writeTextSummary(opts.Out, report)
	}
	if opts.UpdateBaselines {
		// `--update-baselines` rewrites fixtures in-place; the
		// rewrite is wired into runOne (one rewrite per fixture x
		// profile is harmless because subsequent rewrites land on
		// the same canonical form).
		_, _ = io.WriteString(opts.Err,
			"runner: --update-baselines overwrote `expected:` blocks; review the diff before committing\n")
	}
	return report, nil
}

// prepareOptions defaults the unset fields of Options and validates
// the values that have a closed enum (currently just --output). Pulled
// out of Run so the orchestrator stays a flat 13-line driver.
func prepareOptions(opts Options) (Options, error) {
	if opts.Out == nil {
		opts.Out = os.Stdout
	}
	if opts.Err == nil {
		opts.Err = os.Stderr
	}
	if opts.Output == "" {
		opts.Output = outputFormatText
	}
	if opts.Output != outputFormatText && opts.Output != "json" {
		return opts, fmt.Errorf("unknown --output %q (want text or json)",
			opts.Output)
	}
	if opts.FixturesPath == "" {
		opts.FixturesPath = "conformance/fixtures"
	}
	return opts, nil
}

// iterateMatrix is the profile x fixture cross product driver. It
// fans each cell out to runOne, accumulates per-status counters, and
// streams text-mode results to opts.Out as they complete.
func iterateMatrix(ctx context.Context, fixtures []*Fixture, enabled []Profile, opts Options) *Report {
	report := &Report{SchemaVersion: JSONSchemaVersion}
	for _, p := range enabled {
		for _, fx := range fixtures {
			if !contains(fx.Profiles, p.Name) {
				continue
			}
			result := runOne(ctx, fx, p, opts)
			report.Results = append(report.Results, result)
			report.Summary.Total++
			switch result.Status {
			case StatusPass:
				report.Summary.Passed++
			case StatusFail:
				report.Summary.Failed++
			case StatusSkip:
				report.Summary.Skipped++
			}
			if opts.Output == outputFormatText {
				writeTextResult(opts.Out, result)
			}
		}
	}
	return report
}

// ExitCode is the recommended process exit code derived from a
// Report. The CLI calls this directly so the runner's exit semantics
// are unit-testable.
func (r *Report) ExitCode() int {
	if r == nil {
		return 2
	}
	if r.Summary.Failed > 0 {
		return 1
	}
	return 0
}

// runOne executes a single fixture x profile cell. The result is
// always non-nil; status is FAIL on any mismatch or runner-internal
// error during the lifecycle. The lifecycle is:
//
//  1. Boot a fresh emulator for the profile (or reuse the connected
//     one).
//  2. Run setup steps in order against the gateway.
//  3. Run the fixture's query against the gateway.
//  4. Diff the response against expected rows or expected error.
//
// `--update-baselines` short-circuits the diff and rewrites the
// fixture in place with the captured rows / error envelope, so the
// fixture writer can bootstrap without authoring the expected block
// by hand.
func runOne(ctx context.Context, fx *Fixture, p Profile, opts Options) Result {
	started := time.Now()
	result := Result{
		Fixture: fx.Name,
		Path:    fx.Path,
		Profile: p.Name,
		Status:  StatusFail,
	}

	env, startErr := StartEmulator(ctx, opts.Harness, p)
	if startErr != nil {
		result.Message = "start emulator: " + startErr.Error()
		return markDuration(result, started)
	}
	defer func() {
		_ = env.Close()
	}()

	base := env.BaseURL + "/bigquery/v2/projects/" + fx.ProjectID
	for i, step := range fx.Setup {
		if stepErr := runSetupStep(ctx, base, step); stepErr != nil {
			result.Message = fmt.Sprintf("setup[%d]: %v", i, stepErr)
			return markDuration(result, started)
		}
	}

	queryBody, marshalErr := json.Marshal(map[string]any{
		"query":        fx.Query,
		"useLegacySql": false,
	})
	if marshalErr != nil {
		result.Message = "marshal query: " + marshalErr.Error()
		return markDuration(result, started)
	}
	status, body, queryErr := doRequest(ctx, http.MethodPost,
		base+"/queries", queryBody)
	if queryErr != nil {
		result.Message = "query rpc: " + queryErr.Error()
		return markDuration(result, started)
	}

	if fx.Expected.Error != nil {
		return markDuration(runErrorPath(fx, opts, result, status, body), started)
	}
	return markDuration(runRowPath(fx, opts, result, status, body), started)
}

// markDuration stamps the elapsed wall time onto a Result. Pulled out
// of runOne so every early return can share the one-liner without
// re-templating the time.Since math.
func markDuration(r Result, started time.Time) Result {
	r.DurationMs = time.Since(started).Milliseconds()
	return r
}

// runErrorPath drives the error-mode branch of a fixture. It expects
// the engine to have failed (non-2xx) and the error envelope to match
// fx.Expected.Error; the --update-baselines mode rewrites the fixture
// in place using the actual response.
func runErrorPath(fx *Fixture, opts Options, result Result, status int, body []byte) Result {
	if status >= 200 && status < 300 {
		result.Message = "expected error, got success"
		result.Diff = fmt.Sprintf("status: %d\nbody: %s",
			status, snippet(body))
		if opts.UpdateBaselines {
			// Record the actual success result as the new
			// baseline (rows) so the fixture writer can flip
			// the assertion mode.
			_ = rewriteFixtureRows(fx, body)
		}
		return result
	}
	if opts.UpdateBaselines {
		if err := rewriteFixtureError(fx, status, body); err != nil {
			result.Message = "update-baselines: " + err.Error()
			return result
		}
		result.Status = StatusPass
		result.Message = "baseline updated"
		return result
	}
	if diff := errorDiff(*fx.Expected.Error, status, body); diff != "" {
		result.Message = "error mismatch"
		result.Diff = diff
		return result
	}
	result.Status = StatusPass
	return result
}

// runRowPath drives the row-mode branch of a fixture. It expects a
// 2xx response carrying a QueryResponse, then either rewrites the
// fixture (--update-baselines) or diffs the rows against fx.Expected.
func runRowPath(fx *Fixture, opts Options, result Result, status int, body []byte) Result {
	if status < 200 || status >= 300 {
		result.Message = fmt.Sprintf("query failed with HTTP %d", status)
		result.Diff = "body: " + snippet(body)
		return result
	}
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		result.Message = "decode QueryResponse: " + err.Error()
		result.Diff = "body: " + snippet(body)
		return result
	}
	if opts.UpdateBaselines {
		if err := rewriteFixtureRows(fx, body); err != nil {
			result.Message = "update-baselines: " + err.Error()
			return result
		}
		result.Status = StatusPass
		result.Message = "baseline updated"
		return result
	}
	if diff := rowDiff(fx.Expected, run.Schema, run.Rows); diff != "" {
		switch fx.Expected.Match {
		case MatchSchemaOnly:
			result.Message = "schema mismatch"
		case MatchUnordered:
			result.Message = "row multiset mismatch"
		default:
			result.Message = "row mismatch"
		}
		result.Diff = diff
		return result
	}
	result.Status = StatusPass
	return result
}

// runSetupStep dispatches one setup step to the matching helper.
// Errors bubble up unchanged; the caller wraps them with the step
// index for the diff message.
func runSetupStep(ctx context.Context, base string, step SetupStep) error {
	switch {
	case step.Dataset != "":
		return setupDataset(ctx, base, step.Dataset)
	case step.Table != nil:
		return setupTable(ctx, base, step.Table)
	case step.Rows != nil:
		return setupRows(ctx, base, step.Rows)
	case strings.TrimSpace(step.SQL) != "":
		return setupSQL(ctx, base, step.SQL)
	default:
		return errors.New("empty setup step (validated at load time)")
	}
}

// setupDataset issues a `datasets.insert` for the synthesized
// fixture project / dataset pair. Location is hardcoded to US to
// match the gateway's default; fixtures that want a different
// location have to use a SQL setup step.
func setupDataset(ctx context.Context, base, dataset string) error {
	body := fmt.Sprintf(
		`{"datasetReference":{"projectId":"%s","datasetId":"%s"},"location":"US"}`,
		projectIDFromBase(base), dataset)
	status, respBody, err := doRequest(ctx, http.MethodPost,
		base+"/datasets", []byte(body))
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("datasets.insert -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// setupTable issues a `tables.insert` with the fixture's column
// schema. STRUCT children round-trip through columnToTableField.
func setupTable(ctx context.Context, base string, t *TableSetup) error {
	tableBody := struct {
		TableReference bqtypes.TableReference `json:"tableReference"`
		Schema         struct {
			Fields []bqtypes.TableFieldSchema `json:"fields"`
		} `json:"schema"`
	}{}
	tableBody.TableReference = bqtypes.TableReference{
		ProjectID: projectIDFromBase(base),
		DatasetID: t.Dataset,
		TableID:   t.ID,
	}
	for _, c := range t.Schema {
		tableBody.Schema.Fields = append(tableBody.Schema.Fields,
			columnToTableField(c))
	}
	jsonBody, err := json.Marshal(tableBody)
	if err != nil {
		return fmt.Errorf("marshal table body: %w", err)
	}
	url := fmt.Sprintf("%s/datasets/%s/tables", base, t.Dataset)
	status, respBody, err := doRequest(ctx, http.MethodPost, url, jsonBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("tables.insert -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// setupRows issues a `tabledata.insertAll`. It is the only way to
// seed rows on the DuckDB engine today: INSERT VALUES returns
// UNIMPLEMENTED. The wire shape matches Google's REST API spec
// (each row is wrapped in `{json: {...}}`).
func setupRows(ctx context.Context, base string, rs *RowsSetup) error {
	type insertAllRow struct {
		JSON map[string]any `json:"json"`
	}
	body := struct {
		Kind string         `json:"kind"`
		Rows []insertAllRow `json:"rows"`
	}{
		Kind: "bigquery#tableDataInsertAllRequest",
		Rows: make([]insertAllRow, 0, len(rs.Rows)),
	}
	for _, r := range rs.Rows {
		body.Rows = append(body.Rows, insertAllRow{JSON: r})
	}
	jsonBody, err := json.Marshal(body)
	if err != nil {
		return fmt.Errorf("marshal insertAll body: %w", err)
	}
	url := fmt.Sprintf("%s/datasets/%s/tables/%s/insertAll",
		base, rs.Dataset, rs.Table)
	status, respBody, err := doRequest(ctx, http.MethodPost, url, jsonBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("tabledata.insertAll -> %d: %s",
			status, snippet(respBody))
	}
	return nil
}

// setupSQL runs an arbitrary statement through the gateway's
// `/queries` endpoint. Used for setup phases that do not fit the
// dataset/table/rows shape (e.g. preparing a temp UDF).
func setupSQL(ctx context.Context, base, sql string) error {
	queryBody, err := json.Marshal(map[string]any{
		"query":        sql,
		"useLegacySql": false,
	})
	if err != nil {
		return fmt.Errorf("marshal sql body: %w", err)
	}
	status, respBody, err := doRequest(ctx, http.MethodPost,
		base+"/queries", queryBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("setup sql -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// columnToTableField copies our YAML-decoded SchemaColumn onto the
// `bqtypes.TableFieldSchema` wire shape, recursing for STRUCT
// children so nested fields round-trip cleanly.
func columnToTableField(c SchemaColumn) bqtypes.TableFieldSchema {
	out := bqtypes.TableFieldSchema{
		Name:        c.Name,
		Type:        c.Type,
		Mode:        c.Mode,
		Description: c.Description,
	}
	for _, f := range c.Fields {
		out.Fields = append(out.Fields, columnToTableField(f))
	}
	return out
}

// projectIDFromBase pulls the projectId from a URL of the form
// .../bigquery/v2/projects/<projectId>. Returning the inner segment
// keeps the setup-step builders from having to thread projectId
// through their signatures.
func projectIDFromBase(base string) string {
	const marker = "/projects/"
	i := strings.LastIndex(base, marker)
	if i < 0 {
		return ""
	}
	return base[i+len(marker):]
}

// resolveProfiles maps the CLI's --profile flag values to a stable
// matrix order. Empty input means "all known profiles".
func resolveProfiles(names []string) ([]Profile, error) {
	if len(names) == 0 {
		return KnownProfiles(), nil
	}
	seen := make(map[string]bool, len(names))
	out := make([]Profile, 0, len(names))
	for _, n := range names {
		if seen[n] {
			continue
		}
		seen[n] = true
		p, ok := LookupProfile(n)
		if !ok {
			return nil, fmt.Errorf("unknown --profile %q (known: %s)",
				n, strings.Join(profileNames(), ", "))
		}
		out = append(out, p)
	}
	sort.Slice(out, func(i, j int) bool { return out[i].Name < out[j].Name })
	return out, nil
}

func contains(haystack []string, needle string) bool {
	return slices.Contains(haystack, needle)
}
