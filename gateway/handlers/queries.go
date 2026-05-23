package handlers

import (
	"encoding/json"
	"io"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// QueryRun implements `bigquery.jobs.query`:
//
//	POST /bigquery/v2/projects/{projectId}/queries
//
// The synchronous query API. The request body is a QueryRequest (see
// gateway/bqtypes); the response is a QueryResponse with a partial result
// page, or an empty result set + non-empty `jobReference` if the query
// is still running and the client should poll `jobs.getQueryResults`.
//
// SQL dialect: BigQuery's `useLegacySql` field defaults to true on the
// wire. The emulator only supports GoogleSQL because the engine is
// GoogleSQL's analyzer + reference impl. Queries that explicitly set
// `useLegacySql=true` are rejected with HTTP 400 + `reason: invalidQuery`;
// unset and `useLegacySql=false` are both treated as GoogleSQL.
//
// Idempotency: `requestId` provides 15-minute idempotency for matching
// requests, per the upstream docs.
func QueryRun(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		// Read the body once so we can both pre-flight the dialect check
		// and pass the parsed request along to the engine when it lands.
		// A malformed body is reported up front so clients see a
		// structured error rather than the generic 501 stub.
		body, err := io.ReadAll(r.Body)
		if err != nil {
			writeError(w, http.StatusBadRequest, "invalid",
				"Could not read query request body: "+err.Error())
			return
		}
		if len(body) > 0 {
			var req bqtypes.QueryRequest
			if err := json.Unmarshal(body, &req); err != nil {
				writeError(w, http.StatusBadRequest, "invalid",
					"Could not parse query request body as JSON: "+err.Error())
				return
			}
			// Reject legacy SQL up front. The emulator only supports
			// GoogleSQL because the engine is GoogleSQL's analyzer +
			// reference impl; see docs/REST_API.md "SQL dialect".
			if req.UseLegacySQL != nil && *req.UseLegacySQL {
				writeError(w, http.StatusBadRequest, "invalidQuery",
					"useLegacySql=true is not supported by the BigQuery emulator. "+
						"Set useLegacySql=false (GoogleSQL); see docs/REST_API.md.")
				return
			}
		}
		NotImplemented(w, r)
	}
}

// QueryGetResults implements `bigquery.jobs.getQueryResults`:
//
//	GET /bigquery/v2/projects/{projectId}/queries/{jobId}
//
// Pages through results of a running or completed query job. Supports
// `startIndex`, `pageToken`, `maxResults`, `timeoutMs`, `location`, and
// `formatOptions` per the upstream docs.
func QueryGetResults(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
