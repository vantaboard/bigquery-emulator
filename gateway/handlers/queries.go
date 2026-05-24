package handlers

import (
	"encoding/json"
	"io"
	"net/http"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// queryResponseKind is the value the BigQuery REST API returns for the
// `kind` field of a QueryResponse resource. See
// docs/bigquery/docs/reference/rest/v2/jobs/query.md.
const queryResponseKind = "bigquery#queryResponse"

// QueryRun implements `bigquery.jobs.query`:
//
//	POST /bigquery/v2/projects/{projectId}/queries
//
// The synchronous query API. The request body is a QueryRequest (see
// gateway/bqtypes); the response is a QueryResponse with a partial result
// page, or an empty result set + non-empty `jobReference` if the query
// is still running and the client should poll `jobs.getQueryResults`.
//
// Today this handler implements only the dry-run branch (Phase 4c):
// when `dryRun=true`, it forwards the SQL to `enginepb.Query.DryRun`
// (which calls `googlesql::Analyzer` on the C++ side) and turns the
// resulting analyzed schema + estimated bytes into a QueryResponse
// with `jobComplete=true` and an empty rows page. The non-dry-run
// branch lands in Phase 5.
//
// SQL dialect: BigQuery's `useLegacySql` field defaults to true on the
// wire. The emulator only supports GoogleSQL because the engine is
// GoogleSQL's analyzer + reference impl. Queries that explicitly set
// `useLegacySql=true` are rejected with HTTP 400 + `reason: invalidQuery`;
// unset and `useLegacySql=false` are both treated as GoogleSQL.
//
// Idempotency: `requestId` provides 15-minute idempotency for matching
// requests, per the upstream docs.
func QueryRun(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		body, err := io.ReadAll(r.Body)
		if err != nil {
			writeError(w, http.StatusBadRequest, "invalid",
				"Could not read query request body: "+err.Error())
			return
		}
		var req bqtypes.QueryRequest
		if len(body) > 0 {
			if err := json.Unmarshal(body, &req); err != nil {
				writeError(w, http.StatusBadRequest, "invalid",
					"Could not parse query request body as JSON: "+err.Error())
				return
			}
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

		if req.DryRun {
			runQueryDryRun(deps, w, r, &req)
			return
		}

		// Non-dry-run query execution lands in Phase 5; see ROADMAP.md.
		NotImplemented(w, r)
	}
}

// runQueryDryRun handles the dryRun=true branch of QueryRun. It
// forwards the request to `enginepb.Query.DryRun`, which on the C++
// side runs the SQL through `googlesql::Analyzer` and returns the
// resolved output schema + an estimated bytes-processed value. The
// gateway folds those into a `QueryResponse` with `jobComplete=true`
// and no rows -- the BigQuery REST contract for a successful dry run.
//
// When `deps.Query` is nil (the gateway was started without an engine
// subprocess), the handler degrades to the 501 stub the rest of the
// route table uses, so unit-mode runs (`task emulator:run
// --engine_binary=""`) keep returning a structured error envelope.
func runQueryDryRun(deps Dependencies, w http.ResponseWriter, r *http.Request,
	req *bqtypes.QueryRequest) {
	if deps.Query == nil {
		NotImplemented(w, r)
		return
	}
	projectID := r.PathValue("projectId")

	// Pass a defaultDataset hint to the engine when the client set
	// `defaultDataset` in the QueryRequest. The wire field on the
	// engine side carries the dataset id only -- the project comes
	// from `project_id`, which is always taken from the URL.
	defaultDataset := ""
	if req.DefaultDataset != nil {
		defaultDataset = req.DefaultDataset.DatasetID
	}

	engineReq := &enginepb.QueryRequest{
		ProjectId:        projectID,
		DefaultDatasetId: defaultDataset,
		Sql:              req.Query,
		UseLegacySql:     req.UseLegacySQL != nil && *req.UseLegacySQL,
	}

	resp, err := deps.Query.DryRun(r.Context(), engineReq)
	if queryGRPCToHTTPError(w, err) {
		return
	}

	out := bqtypes.QueryResponse{
		Kind:                queryResponseKind,
		Schema:              schemaFromProto(resp.GetSchema()),
		TotalBytesProcessed: strconv.FormatInt(resp.GetEstimatedBytesProcessed(), 10),
		JobComplete:         true,
	}
	writeJSON(w, http.StatusOK, out)
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
