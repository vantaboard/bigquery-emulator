package runner

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// RESTClient issues jobs.query calls against a BigQuery REST emulator.
type RESTClient struct {
	BaseURL   string
	ProjectID string
	HTTP      *http.Client
}

func NewRESTClient(baseURL, projectID string) *RESTClient {
	return &RESTClient{
		BaseURL:   baseURL,
		ProjectID: projectID,
		HTTP:      &http.Client{Timeout: 0},
	}
}

func (c *RESTClient) PostQuery(ctx context.Context, sql string) (int, []byte, error) {
	body, err := json.Marshal(map[string]any{
		"query":        sql,
		"useLegacySql": false,
	})
	if err != nil {
		return 0, nil, err
	}
	url := fmt.Sprintf("%s/bigquery/v2/projects/%s/queries", c.BaseURL, c.ProjectID)
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, url, bytes.NewReader(body))
	if err != nil {
		return 0, nil, err
	}
	req.Header.Set("Content-Type", "application/json")
	resp, err := c.HTTP.Do(req)
	if err != nil {
		return 0, nil, err
	}
	defer func() { _ = resp.Body.Close() }()
	data, err := io.ReadAll(resp.Body)
	return resp.StatusCode, data, err
}

// ParseQueryResponse decodes a successful jobs.query body.
func ParseQueryResponse(body []byte) (bqtypes.QueryResponse, error) {
	var out bqtypes.QueryResponse
	if err := json.Unmarshal(body, &out); err != nil {
		return out, err
	}
	return out, nil
}

// RESTRowsToMaps converts REST f/v rows to string maps.
func RESTRowsToMaps(schema *bqtypes.TableSchema, rows []bqtypes.Row) []map[string]string {
	if schema == nil {
		return nil
	}
	names := make([]string, len(schema.Fields))
	for i, f := range schema.Fields {
		names[i] = f.Name
	}
	out := make([]map[string]string, 0, len(rows))
	for _, row := range rows {
		m := make(map[string]string, len(names))
		for i, name := range names {
			if i < len(row.F) {
				m[name] = cellToString(row.F[i].V)
			}
		}
		out = append(out, m)
	}
	return out
}

func timedQuery(
	ctx context.Context,
	fn func(context.Context) (QueryResult, error),
	timeout time.Duration,
) (QueryResult, error) {
	ctx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()
	start := time.Now()
	res, err := fn(ctx)
	res.Elapsed = time.Since(start)
	return res, err
}
