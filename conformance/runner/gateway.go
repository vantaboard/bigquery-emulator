package runner

import (
	"context"
	"encoding/json"
	"fmt"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// CompareRows diffs actual query rows against an Expectation using the
// same typed-cell engine as the YAML fixture lane. Returns an empty
// string on match.
func CompareRows(exp Expectation, schema *bqtypes.TableSchema, actualRows []bqtypes.Row) string {
	return rowDiff(exp, schema, actualRows)
}

// QueryViaGateway posts a GoogleSQL statement to the gateway's
// jobs.query endpoint and returns the HTTP status plus raw body.
func QueryViaGateway(ctx context.Context, baseURL, sql string) (int, []byte, error) {
	return postQuery(ctx, baseURL, sql)
}

// SetupSQLViaGateway runs an arbitrary statement through jobs.query for
// catalog seeding (CREATE TABLE, etc.).
func SetupSQLViaGateway(ctx context.Context, baseURL, sql string) error {
	status, body, err := postQuery(ctx, baseURL, sql)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("setup sql -> %d: %s", status, snippet(body))
	}
	return nil
}

// DoRequest posts JSON to a gateway URL. Exported for sub-lanes.
func DoRequest(ctx context.Context, url string, body []byte) (int, []byte, error) {
	return doRequest(ctx, url, body)
}

func postQuery(ctx context.Context, baseURL, sql string) (int, []byte, error) {
	return postQueryWithDefaultDataset(ctx, baseURL, sql, "")
}

func postQueryWithDefaultDataset(ctx context.Context, baseURL, sql, defaultDataset string) (int, []byte, error) {
	queryBody, err := marshalJobsQueryBody(sql, defaultDataset)
	if err != nil {
		return 0, nil, err
	}
	return doRequest(ctx, baseURL+"/queries", queryBody)
}

func marshalJobsQueryBody(sql, defaultDataset string) ([]byte, error) {
	body := map[string]any{
		"query":        sql,
		"useLegacySql": false,
	}
	if defaultDataset != "" {
		body["defaultDataset"] = map[string]string{"datasetId": defaultDataset}
	}
	queryBody, err := json.Marshal(body)
	if err != nil {
		return nil, fmt.Errorf("marshal query: %w", err)
	}
	return queryBody, nil
}
