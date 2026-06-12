package runner

import (
	"context"
	"fmt"
	"net/http"
	"os/exec"
	"strings"
	"time"
)

const defaultGoccyImage = "ghcr.io/goccy/bigquery-emulator:0.7.2"

// goccyProject is the single project the goccy container is started
// with. goccy/bigquery-emulator 404s on any other project id, so every
// case runs under this one (dataset names are unique per case).
const goccyProject = "bench"

// DefaultGoccyImage returns the pinned goccy container reference.
func DefaultGoccyImage() string { return defaultGoccyImage }

// GoccyTarget drives the goccy/bigquery-emulator Docker image.
type GoccyTarget struct {
	opts       TargetOptions
	container  string
	hostPort   int
	client     *RESTClient
	httpClient *http.Client
}

func NewGoccyTarget(opts TargetOptions) *GoccyTarget {
	return &GoccyTarget{opts: opts}
}

func (t *GoccyTarget) Name() TargetName { return TargetGoccy }

func (t *GoccyTarget) Start(ctx context.Context) error {
	image := t.opts.GoccyImage
	if image == "" {
		image = defaultGoccyImage
	}
	port, err := freePort()
	if err != nil {
		return err
	}
	t.hostPort = port
	name := fmt.Sprintf("bq-bench-goccy-%d", time.Now().UnixNano())
	args := []string{
		"run", "--rm", "-d",
		"--name", name,
		"-p", fmt.Sprintf("127.0.0.1:%d:9050", port),
		image,
		"--project=" + goccyProject,
	}
	cmd := exec.CommandContext(ctx, "docker", args...)
	if out, err := cmd.CombinedOutput(); err != nil {
		return fmt.Errorf("docker run %s: %w: %s", image, err, strings.TrimSpace(string(out)))
	}
	t.container = name
	t.httpClient = &http.Client{Timeout: 0}
	if err := t.waitReady(ctx); err != nil {
		_ = t.Cleanup(ctx)
		return err
	}
	t.client = &RESTClient{
		BaseURL:   fmt.Sprintf("http://127.0.0.1:%d", port),
		ProjectID: "bench",
		HTTP:      t.httpClient,
	}
	return nil
}

func (t *GoccyTarget) waitReady(ctx context.Context) error {
	deadline, ok := ctx.Deadline()
	if !ok {
		deadline = time.Now().Add(60 * time.Second)
	}
	url := fmt.Sprintf("http://127.0.0.1:%d/bigquery/v2/projects/bench/queries", t.hostPort)
	body := []byte(`{"query":"SELECT 1","useLegacySql":false}`)
	for time.Now().Before(deadline) {
		req, err := http.NewRequestWithContext(ctx, http.MethodPost, url, strings.NewReader(string(body)))
		if err != nil {
			return err
		}
		req.Header.Set("Content-Type", "application/json")
		resp, err := t.httpClient.Do(req)
		if err == nil {
			resp.Body.Close()
			if resp.StatusCode >= 200 && resp.StatusCode < 500 {
				return nil
			}
		}
		time.Sleep(500 * time.Millisecond)
	}
	return fmt.Errorf("goccy emulator on port %d not ready", t.hostPort)
}

func (t *GoccyTarget) SetupCase(ctx context.Context, c Case, dataset string) error {
	setup, _ := c.Substitute(dataset, goccyProject)
	t.client.ProjectID = goccyProject
	for _, sql := range setup {
		status, body, err := t.client.PostQuery(ctx, sql)
		if err != nil {
			return err
		}
		if status < 200 || status >= 300 {
			return fmt.Errorf("setup sql -> HTTP %d: %s", status, snippet(body))
		}
	}
	return nil
}

func (t *GoccyTarget) RunQuery(ctx context.Context, c Case, sql string, timeout time.Duration) (QueryResult, error) {
	t.client.ProjectID = goccyProject
	if timeout <= 0 {
		timeout = time.Duration(defaultTimeoutMS) * time.Millisecond
	}
	return timedQuery(ctx, func(ctx context.Context) (QueryResult, error) {
		status, body, err := t.client.PostQuery(ctx, sql)
		if err != nil {
			return QueryResult{Error: err.Error()}, err
		}
		if status < 200 || status >= 300 {
			return QueryResult{Error: fmt.Sprintf("HTTP %d: %s", status, snippet(body))},
				fmt.Errorf("query failed: HTTP %d", status)
		}
		resp, err := ParseQueryResponse(body)
		if err != nil {
			return QueryResult{Error: err.Error()}, err
		}
		rows := RESTRowsToMaps(resp.Schema, resp.Rows)
		hash, _ := HashRows(rows)
		return QueryResult{
			Rows:       rows,
			RowCount:   len(rows),
			ResultHash: hash,
		}, nil
	}, timeout)
}

func (t *GoccyTarget) Cleanup(ctx context.Context) error {
	if t.container == "" {
		return nil
	}
	cmd := exec.CommandContext(ctx, "docker", "rm", "-f", t.container)
	_ = cmd.Run()
	t.container = ""
	return nil
}

// ImageTag extracts the tag from a full docker image reference.
func ImageTag(image string) string {
	if i := strings.LastIndex(image, ":"); i >= 0 {
		return image[i+1:]
	}
	return image
}

var _ Target = (*GoccyTarget)(nil)
