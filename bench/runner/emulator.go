package runner

import (
	"context"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"time"

	conf "github.com/vantaboard/bigquery-emulator/conformance/runner"
)

const defaultEngineBinary = "./bin/emulator_main"

// EmulatorTarget drives the in-repo emulator via an in-process gateway.
type EmulatorTarget struct {
	opts   TargetOptions
	env    *conf.EmulatorEnv
	client *RESTClient
}

func NewEmulatorTarget(opts TargetOptions) *EmulatorTarget {
	return &EmulatorTarget{opts: opts}
}

func (t *EmulatorTarget) Name() TargetName { return TargetEmulator }

func (t *EmulatorTarget) Start(ctx context.Context) error {
	if runtime.GOOS == "windows" {
		return errors.New("emulator benchmarks require POSIX subprocess support")
	}
	bin := t.opts.EngineBinary
	if bin == "" {
		bin = resolveEngineBinary()
	}
	profile, ok := conf.LookupProfile(conf.ProfileDuckDB)
	if !ok {
		return fmt.Errorf("profile %q not found", conf.ProfileDuckDB)
	}
	env, err := conf.StartEmulator(ctx, conf.HarnessOptions{
		EngineBinary: bin,
		EngineStdout: os.Stderr,
		EngineStderr: os.Stderr,
	}, profile)
	if err != nil {
		return err
	}
	t.env = env
	t.client = NewRESTClient(env.BaseURL, "bench")
	return nil
}

func (t *EmulatorTarget) SetupCase(ctx context.Context, c Case, dataset string) error {
	setup, _ := c.Substitute(dataset, c.ProjectID)
	base := fmt.Sprintf("%s/bigquery/v2/projects/%s", t.env.BaseURL, c.ProjectID)
	for _, sql := range setup {
		if err := conf.SetupSQLViaGateway(ctx, base, sql); err != nil {
			return err
		}
	}
	return nil
}

func (t *EmulatorTarget) RunQuery(ctx context.Context, c Case, sql string, timeout time.Duration) (QueryResult, error) {
	t.client.ProjectID = c.ProjectID
	if timeout <= 0 {
		timeout = time.Duration(defaultTimeoutMS) * time.Millisecond
	}
	return timedQuery(ctx, func(ctx context.Context) (QueryResult, error) {
		start := time.Now()
		status, body, err := t.client.PostQuery(ctx, sql)
		if err != nil {
			return QueryResult{Error: err.Error()}, err
		}
		elapsed := time.Since(start)
		if status < 200 || status >= 300 {
			return QueryResult{Elapsed: elapsed, Error: fmt.Sprintf("HTTP %d: %s", status, snippet(body))},
				fmt.Errorf("query failed: HTTP %d", status)
		}
		resp, err := ParseQueryResponse(body)
		if err != nil {
			return QueryResult{Elapsed: elapsed, Error: err.Error()}, err
		}
		rows := RESTRowsToMaps(resp.Schema, resp.Rows)
		hash, _ := HashRows(rows)
		out := QueryResult{
			Elapsed:    elapsed,
			Rows:       rows,
			RowCount:   len(rows),
			ResultHash: hash,
		}
		if resp.Statistics != nil && resp.Statistics.Query != nil {
			out.Route = resp.Statistics.Query.EmulatorRoute
			out.Phases = resp.Statistics.Query.EmulatorPhases
		}
		return out, nil
	}, timeout)
}

func (t *EmulatorTarget) Cleanup(context.Context) error {
	if t.env != nil {
		return t.env.Close()
	}
	return nil
}

func snippet(b []byte) string {
	const max = 200
	if len(b) <= max {
		return string(b)
	}
	return string(b[:max]) + "..."
}

func resolveEngineBinary() string {
	if p := os.Getenv("BIGQUERY_EMULATOR_BIN"); p != "" {
		if _, err := os.Stat(p); err == nil {
			return p
		}
	}
	candidates := []string{defaultEngineBinary, filepath.Join("bin", "emulator_main")}
	for _, c := range candidates {
		if _, err := os.Stat(c); err == nil {
			return c
		}
	}
	return defaultEngineBinary
}

var _ Target = (*EmulatorTarget)(nil)
