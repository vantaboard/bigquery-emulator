//go:build integration

package e2e

import (
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// TestQueryParameterWireMatrix exercises scalar query-parameter wire forms
// end-to-end through the REST jobs.query path (plan 04).
func TestQueryParameterWireMatrix(t *testing.T) {
	env := startEmulator(t)
	base := env.URL() + "/bigquery/v2/projects/proj-params-matrix"

	cases := []struct {
		name       string
		query      string
		paramType  string
		paramValue string
		wantCell   string
	}{
		{
			name:       "timestamp_naive_iso",
			query:      "SELECT FORMAT_TIMESTAMP('%Y-%m-%d %H:%M:%E*S', @p) AS v",
			paramType:  "TIMESTAMP",
			paramValue: "2026-06-22T10:00:00",
			wantCell:   "2026-06-22 10:00:00",
		},
		{
			name:       "timestamp_space",
			query:      "SELECT FORMAT_TIMESTAMP('%Y-%m-%d %H:%M:%E*S', @p) AS v",
			paramType:  "TIMESTAMP",
			paramValue: "2026-06-22 10:00:00",
			wantCell:   "2026-06-22 10:00:00",
		},
		{
			name:       "date_iso",
			query:      "SELECT FORMAT_DATE('%Y-%m-%d', @p) AS v",
			paramType:  "DATE",
			paramValue: "2020-06-15",
			wantCell:   "2020-06-15",
		},
		{
			name:       "datetime_space",
			query:      "SELECT FORMAT_DATETIME('%Y-%m-%d %H:%M:%E*S', @p) AS v",
			paramType:  "DATETIME",
			paramValue: "2020-06-15 12:30:45",
			wantCell:   "2020-06-15 12:30:45",
		},
		{
			name:       "time_fractional",
			query:      "SELECT FORMAT_TIME('%H:%M:%E*S', @p) AS v",
			paramType:  "TIME",
			paramValue: "12:30:45.123456",
			wantCell:   "12:30:45.123456",
		},
		{
			name:       "numeric_string",
			query:      "SELECT CAST(@p AS STRING) AS v",
			paramType:  "NUMERIC",
			paramValue: "3.14159",
			wantCell:   "3.14159",
		},
		{
			name:       "bytes_base64",
			query:      "SELECT TO_BASE64(@p) AS v",
			paramType:  "BYTES",
			paramValue: "SGVsbG8=",
			wantCell:   "SGVsbG8=",
		},
	}

	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			body := fmt.Sprintf(`{
			  "query": %q,
			  "useLegacySql": false,
			  "queryParameters": [{
			    "name": "p",
			    "parameterType": {"type": %q},
			    "parameterValue": {"value": %q}
			  }]
			}`, tc.query, tc.paramType, tc.paramValue)
			status, respBody := doJSON(t, http.MethodPost, base+"/queries", []byte(body))
			if status != http.StatusOK {
				t.Fatalf("jobs.query -> %d; body=%s", status, string(respBody))
			}
			var resp bqtypes.QueryResponse
			if err := json.Unmarshal(respBody, &resp); err != nil {
				t.Fatalf("decode QueryResponse: %v", err)
			}
			if len(resp.Rows) != 1 || len(resp.Rows[0].F) != 1 {
				t.Fatalf("rows = %+v, want one cell", resp.Rows)
			}
			if got := fmt.Sprint(resp.Rows[0].F[0].V); got != tc.wantCell {
				t.Errorf("cell = %q, want %q", got, tc.wantCell)
			}
		})
	}
}
