package runner

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"strings"
	"syscall"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// SessionOptions configures a session-lane run.
type SessionOptions struct {
	SessionsPath    string
	IncludeSelfTest bool
	Harness         HarnessOptions
	Profiles        []string
	Output          string
	Out             io.Writer
	Err             io.Writer
}

// RunSessions executes every session fixture against one long-lived engine per
// session x profile cell.
func RunSessions(ctx context.Context, opts SessionOptions) (*Report, error) {
	opts, err := prepareSessionOptions(opts)
	if err != nil {
		return nil, err
	}
	sessions, err := LoadSessionDir(opts.SessionsPath, opts.IncludeSelfTest)
	if err != nil {
		return nil, err
	}
	if len(sessions) == 0 {
		return nil, fmt.Errorf("no sessions found under %s", opts.SessionsPath)
	}
	enabled, err := resolveProfiles(opts.Profiles)
	if err != nil {
		return nil, err
	}
	report := iterateSessionMatrix(ctx, sessions, enabled, opts)
	if opts.Output == outputFormatJSON {
		if err := writeJSONReport(opts.Out, report); err != nil {
			return report, fmt.Errorf("write json report: %w", err)
		}
	} else {
		writeTextSummary(opts.Out, report)
	}
	return report, nil
}

func prepareSessionOptions(opts SessionOptions) (SessionOptions, error) {
	if opts.Out == nil {
		opts.Out = os.Stdout
	}
	if opts.Err == nil {
		opts.Err = os.Stderr
	}
	if opts.Output == "" {
		opts.Output = outputFormatText
	}
	if opts.Output != outputFormatText && opts.Output != outputFormatJSON {
		return opts, fmt.Errorf("unknown --output %q (want text or json)", opts.Output)
	}
	if opts.SessionsPath == "" {
		opts.SessionsPath = DefaultSessionsDir
	}
	return opts, nil
}

func iterateSessionMatrix(ctx context.Context, sessions []*Session, enabled []Profile, opts SessionOptions) *Report {
	report := &Report{SchemaVersion: JSONSchemaVersion}
	for _, p := range enabled {
		for _, sess := range sessions {
			if !contains(sess.Profiles, p.Name) {
				continue
			}
			result := runSession(ctx, sess, p, opts)
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

func sessionProjectBase(env *EmulatorEnv, projectID string) string {
	return env.BaseURL + "/bigquery/v2/projects/" + projectID
}

func runSession(ctx context.Context, sess *Session, p Profile, opts SessionOptions) Result {
	started := time.Now()
	result := Result{
		Fixture: sess.Name,
		Path:    sess.Path,
		Profile: p.Name,
		Status:  StatusFail,
	}

	env, startErr := StartEmulator(ctx, opts.Harness, p)
	if startErr != nil {
		result.Message = "start emulator: " + startErr.Error()
		return markDuration(result, started)
	}
	defer func() { _ = env.Close() }()

	defaultDataset := sess.DefaultDataset

	for i, step := range sess.Steps {
		// Re-read BaseURL each step: RestartEngine replaces the in-process
		// gateway httptest server and updates env.BaseURL.
		base := sessionProjectBase(env, sess.ProjectID)
		if err := executeSessionStep(ctx, env, base, defaultDataset, step, fmt.Sprintf("[%d]", i)); err != nil {
			result.Message = err.Error()
			return finishSessionMaybeKnown(result, started, sess.KnownFailing)
		}
	}

	result.Status = StatusPass
	return markDuration(result, started)
}

func executeSessionStep(
	ctx context.Context,
	env *EmulatorEnv,
	base, defaultDataset string,
	step SessionStep,
	indexPrefix string,
) error {
	if step.Repeat > 0 {
		for n := 0; n < step.Repeat; n++ {
			for j, nested := range step.Steps {
				prefix := fmt.Sprintf("%s.repeat(%d)[%d]", indexPrefix, n, j)
				if err := executeSessionStep(ctx, env, base, defaultDataset, nested, prefix); err != nil {
					return err
				}
			}
		}
		return nil
	}

	kind, err := step.kind()
	if err != nil {
		return fmt.Errorf("%s: %w", indexPrefix, err)
	}

	switch kind {
	case stepKindSetup:
		if err := RunSetupSteps(ctx, base, []SetupStep{step.asSetupStep()}, defaultDataset); err != nil {
			return fmt.Errorf("%s: %w", indexPrefix, err)
		}
	case stepKindREST:
		if err := runRESTStep(ctx, base, step.REST); err != nil {
			return fmt.Errorf("%s: %w", indexPrefix, err)
		}
	case stepKindRestart:
		if err := env.RestartEngine(ctx); err != nil {
			return fmt.Errorf("%s restart: %w", indexPrefix, err)
		}
	case stepKindQuery:
		if err := runSessionQueryStep(ctx, base, defaultDataset, step, indexPrefix); err != nil {
			return err
		}
	case stepKindAssertionOnly:
	}

	return runSessionAssertions(ctx, env, base, step, indexPrefix)
}

func runSessionQueryStep(
	ctx context.Context,
	base, defaultDataset string,
	step SessionStep,
	indexPrefix string,
) error {
	dd := defaultDataset
	if step.DefaultDataset != "" {
		dd = step.DefaultDataset
	}
	status, body, err := postQueryWithDefaultDataset(ctx, base, step.Query, dd)
	if err != nil {
		return fmt.Errorf("%s query rpc: %w", indexPrefix, err)
	}
	if step.ExpectError != nil {
		if diff := errorDiff(*step.ExpectError, status, body); diff != "" {
			return fmt.Errorf("%s error mismatch: %s", indexPrefix, diff)
		}
		return nil
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("%s query failed with HTTP %d: %s",
			indexPrefix, status, snippet(body))
	}
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		return fmt.Errorf("%s decode QueryResponse: %w", indexPrefix, err)
	}
	exp := Expectation{Match: MatchOrdered, Rows: step.ExpectRows}
	if diff := rowDiff(exp, run.Schema, run.Rows); diff != "" {
		return fmt.Errorf("%s row mismatch: %s", indexPrefix, diff)
	}
	return nil
}

func runSessionAssertions(
	ctx context.Context,
	env *EmulatorEnv,
	base string,
	step SessionStep,
	indexPrefix string,
) error {
	if step.ExpectAlive != nil {
		alive := env.EngineAlive()
		want := *step.ExpectAlive
		if alive != want {
			if !alive {
				return fmt.Errorf(
					"%s expect_alive=true but engine subprocess has exited (signal: aborted or non-zero exit)",
					indexPrefix,
				)
			}
			return fmt.Errorf("%s expect_alive=false but engine subprocess is still running", indexPrefix)
		}
	}
	if step.ExpectTableList != nil {
		if err := assertTableList(ctx, base, step.ExpectTableList); err != nil {
			return fmt.Errorf("%s %w", indexPrefix, err)
		}
	}
	return nil
}

func runRESTStep(ctx context.Context, base string, rest *RESTStep) error {
	url, err := resolveRESTURL(base, rest.Path)
	if err != nil {
		return err
	}
	var body []byte
	if rest.Body != nil {
		body, err = json.Marshal(rest.Body)
		if err != nil {
			return fmt.Errorf("marshal rest body: %w", err)
		}
	}
	status, respBody, err := DoHTTPRequest(ctx, rest.Method, url, body)
	if err != nil {
		return err
	}
	want := rest.ExpectStatus
	if want == 0 {
		if status < 200 || status >= 300 {
			return fmt.Errorf("rest %s %s -> %d: %s",
				rest.Method, rest.Path, status, snippet(respBody))
		}
		return nil
	}
	if status != want {
		return fmt.Errorf("rest %s %s -> %d, want %d: %s",
			rest.Method, rest.Path, status, want, snippet(respBody))
	}
	return nil
}

func resolveRESTURL(base, path string) (string, error) {
	path = strings.TrimSpace(path)
	if path == "" {
		return "", errors.New("rest path is empty")
	}
	if strings.HasPrefix(path, "http://") || strings.HasPrefix(path, "https://") {
		return path, nil
	}
	if strings.HasPrefix(path, "/") {
		// Absolute from gateway host: strip duplicate /bigquery prefix if present.
		if before, _, ok := strings.Cut(base, "/bigquery/"); ok {
			return before + path, nil
		}
		return base + path, nil
	}
	return strings.TrimSuffix(base, "/") + "/" + strings.TrimPrefix(path, "/"), nil
}

func assertTableList(ctx context.Context, base string, exp *TableListExpect) error {
	url := fmt.Sprintf("%s/datasets/%s/tables", base, exp.Dataset)
	status, body, err := DoHTTPRequest(ctx, httpMethodGet, url, nil)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("tables.list -> %d: %s", status, snippet(body))
	}
	ids, err := parseTableListIDs(body)
	if err != nil {
		return err
	}
	if diff := tableListDiff(exp, ids); diff != "" {
		return fmt.Errorf("expect_table_list: %s", diff)
	}
	return nil
}

func parseTableListIDs(body []byte) ([]string, error) {
	var list struct {
		Tables []struct {
			TableReference struct {
				TableID string `json:"tableId"`
			} `json:"tableReference"`
		} `json:"tables"`
	}
	if err := json.Unmarshal(body, &list); err != nil {
		return nil, fmt.Errorf("decode tableList: %w", err)
	}
	out := make([]string, 0, len(list.Tables))
	for _, t := range list.Tables {
		out = append(out, t.TableReference.TableID)
	}
	return out, nil
}

func tableListDiff(exp *TableListExpect, actual []string) string {
	have := make(map[string]bool, len(actual))
	for _, id := range actual {
		have[id] = true
	}
	var missing []string
	for _, want := range exp.Contains {
		if !have[want] {
			missing = append(missing, want)
		}
	}
	var unexpected []string
	for _, forbid := range exp.NotContains {
		if have[forbid] {
			unexpected = append(unexpected, forbid)
		}
	}
	if len(missing) == 0 && len(unexpected) == 0 {
		return ""
	}
	var b strings.Builder
	if len(missing) > 0 {
		fmt.Fprintf(&b, "missing tables: [%s]; ", strings.Join(missing, ", "))
	}
	if len(unexpected) > 0 {
		fmt.Fprintf(&b, "forbidden tables present: [%s]; ", strings.Join(unexpected, ", "))
	}
	fmt.Fprintf(&b, "actual table ids: [%s]", strings.Join(actual, ", "))
	return strings.TrimSpace(b.String())
}

func finishSessionMaybeKnown(r Result, started time.Time, knownFailing bool) Result {
	r = markDuration(r, started)
	if knownFailing && r.Status == StatusFail {
		r.Status = StatusSkip
		r.Message = "known_failing (expected divergence): " + r.Message
	}
	return r
}

// EngineAlive reports whether the spawned engine subprocess is still running.
// Connected-mode envs (no subprocess) always return true.
func (e *EmulatorEnv) EngineAlive() bool {
	if e == nil || e.cmd == nil || e.cmd.Process == nil {
		return true
	}
	if e.cmd.ProcessState != nil && e.cmd.ProcessState.Exited() {
		return false
	}
	if err := e.cmd.Process.Signal(syscall.Signal(0)); err != nil {
		return false
	}
	return true
}
