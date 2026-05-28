package handlers

import (
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"strconv"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
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
// The handler has two branches:
//
//   - dryRun=true forwards the SQL to `enginepb.Query.DryRun` (which
//     calls `googlesql::Analyzer` on the C++ side) and turns the
//     resulting analyzed schema + estimated bytes into a QueryResponse
//     with `jobComplete=true` and an empty rows page.
//   - dryRun=false (or unset) forwards the SQL to
//     `enginepb.Query.ExecuteQuery`, drains the server-streaming
//     response (first message carries the schema, subsequent messages
//     carry one row of cells each), marshals each row through
//     `bqtypes.CellsToRow`, and records a DONE Job in `deps.Jobs` so
//     the returned `jobReference` is discoverable by a later
//     `jobs.get`.
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
	// Default to a per-handler Registry so unit tests that pass a
	// zero-valued Dependencies still get a working job store; the
	// server-mode path passes a process-shared Registry from
	// gateway.NewServer so jobs survive between requests.
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
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

		runQueryExecute(deps, w, r, &req)
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

// runQueryExecute handles the dryRun=false branch of QueryRun. It
// forwards the SQL to the engine's server-streaming
// `enginepb.Query.ExecuteQuery` RPC, drains the schema + row stream,
// marshals every row through `bqtypes.CellsToRow`, and stamps the
// resulting `QueryResponse` with a DONE jobReference recorded in
// `deps.Jobs`.
//
// Stream contract (mirrors the comment on proto QueryResultRow):
// the first message carries the schema; subsequent messages each
// carry one row's cells. The schema reader is defensive -- if a
// later message also sets `schema` it is ignored, and a message
// with neither schema nor cells contributes an empty row.
//
// When `deps.Query` is nil (the gateway was started without an
// engine subprocess), the handler degrades to the structured 501
// stub the rest of the route table uses; unit-mode runs (`task
// emulator:run --engine_binary=""`) keep returning a BigQuery-
// shaped error envelope instead of a panic.
func runQueryExecute(deps Dependencies, w http.ResponseWriter, r *http.Request,
	req *bqtypes.QueryRequest) {
	if deps.Query == nil {
		NotImplemented(w, r)
		return
	}
	projectID := r.PathValue("projectId")

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

	start := time.Now().UTC()
	stream, err := deps.Query.ExecuteQuery(r.Context(), engineReq)
	if queryGRPCToHTTPError(w, err) {
		return
	}

	var schema *enginepb.TableSchema
	var dmlStats *enginepb.DmlStats
	rows := make([]bqtypes.Row, 0)
	for {
		msg, err := stream.Recv()
		if errors.Is(err, io.EOF) {
			break
		}
		if queryGRPCToHTTPError(w, err) {
			return
		}
		if s := msg.GetSchema(); s != nil {
			// Per proto contract the first message carries the
			// schema and subsequent messages carry rows. Keep the
			// first schema we see and ignore any later resends so
			// we don't reset mid-stream.
			if schema == nil {
				schema = s
			}
			continue
		}
		if d := msg.GetDmlStats(); d != nil {
			// Final summary message for an INSERT/UPDATE/DELETE/
			// MERGE statement. The engine emits exactly one of
			// these on the DML path; later messages on the same
			// stream are ignored (the proto contract is "one or
			// the other" per RPC).
			if dmlStats == nil {
				dmlStats = d
			}
			continue
		}
		rows = append(rows, bqtypes.CellsToRow(msg.GetCells()))
	}
	end := time.Now().UTC()

	// Record the completed job (with its rows + schema cached)
	// before assembling the response so the jobReference we emit
	// is the same one a later jobs.get / jobs.getQueryResults will
	// find. The current registry does not track engine-side
	// bytes-processed yet, so we stamp 0; the long-running-jobs
	// follow-up wires the real metric.
	restSchema := schemaFromProto(schema)
	// Build the bqtypes.DmlStats envelope once so the synchronous
	// response and the cached `QueryResult` (replayed by
	// `jobs.getQueryResults`) emit byte-identical row counts.
	var restDmlStats *bqtypes.DmlStats
	if dmlStats != nil {
		inserted := dmlStats.GetInsertedRowCount()
		updated := dmlStats.GetUpdatedRowCount()
		deleted := dmlStats.GetDeletedRowCount()
		restDmlStats = &bqtypes.DmlStats{
			InsertedRowCount: strconv.FormatInt(inserted, 10),
			UpdatedRowCount:  strconv.FormatInt(updated, 10),
			DeletedRowCount:  strconv.FormatInt(deleted, 10),
		}
	}
	result := &jobs.QueryResult{
		Schema:   restSchema,
		Rows:     rows,
		DmlStats: restDmlStats,
	}
	job := deps.Jobs.CompleteQueryWithResult(
		projectID, req.Location, 0, start, end, result)
	jobRef := job.JobReference

	out := bqtypes.QueryResponse{
		Kind:                queryResponseKind,
		Schema:              restSchema,
		JobReference:        &jobRef,
		JobComplete:         true,
		TotalRows:           strconv.FormatUint(uint64(len(rows)), 10),
		Rows:                rows,
		TotalBytesProcessed: job.Statistics.TotalBytesProcessed,
		CreationTime:        job.Statistics.CreationTime,
		StartTime:           job.Statistics.StartTime,
		EndTime:             job.Statistics.EndTime,
		Location:            jobRef.Location,
	}
	if restDmlStats != nil {
		// Surface BigQuery's DML statistics envelope. `dmlStats`
		// carries the per-operation row counts; `numDmlAffectedRows`
		// is the legacy aggregate (sum of inserted + updated +
		// deleted) that older client libraries still read.
		out.DmlStats = restDmlStats
		out.NumDmlAffectedRows = strconv.FormatInt(
			dmlStats.GetInsertedRowCount()+
				dmlStats.GetUpdatedRowCount()+
				dmlStats.GetDeletedRowCount(), 10)
		// DML statements have no result schema or rows; clear the
		// SELECT-shape fields so the response stays consistent with
		// BigQuery's wire encoding (TotalRows = "0", no rows array,
		// no schema).
		out.Schema = nil
		out.Rows = nil
		out.TotalRows = "0"
	}
	writeJSON(w, http.StatusOK, out)
}

// getQueryResultsKind is the value the BigQuery REST API returns for
// the `kind` field of a GetQueryResultsResponse resource. See
// docs/bigquery/docs/reference/rest/v2/jobs/getQueryResults.md.
const getQueryResultsKind = "bigquery#getQueryResultsResponse"

// QueryGetResults implements `bigquery.jobs.getQueryResults`:
//
//	GET /bigquery/v2/projects/{projectId}/queries/{jobId}
//
// Replays the cached rows + schema for a previously-run synchronous
// query. The query-select-e2e charter
// (`.cursor/plans/query-select-e2e_b3e4f5a6.plan.md`) limits this
// handler to single-page reads: the registry holds the entire
// result set in memory at job-completion time and this endpoint
// emits it back in one response. Real cursored pagination (multi-page
// `pageToken` lifecycle, partial reads from a streaming engine) is
// deferred to a later change alongside long-running async jobs.
//
// Documented query parameters and current behavior:
//
//   - `startIndex` (uint): respected; rows < startIndex are skipped.
//   - `maxResults` (uint): respected; rows beyond the slice are
//     truncated. The result is still flagged as complete (no
//     pageToken is emitted) -- the BigQuery contract permits
//     returning fewer rows than requested.
//   - `pageToken` (string): the emulator never mints one, so a
//     non-empty value cannot be honored. We respond with an empty
//     page and `jobComplete=true` to keep client polling loops happy.
//   - `location` (string): when both the stored job's location and
//     the query parameter are non-empty and disagree, returns 404
//     notFound -- the same shape BigQuery uses when callers route a
//     `getQueryResults` to the wrong region.
//   - `timeoutMs`, `formatOptions`: ignored. Queries are synchronous
//     so timeoutMs is moot, and the f/v wire shape is the only
//     output format the emulator emits.
//
// Project mismatches between the URL path and the stored job map to
// 404 notFound rather than 403, matching BigQuery's behavior of
// hiding cross-project jobs behind the same 404 envelope.
func QueryGetResults(deps Dependencies) http.HandlerFunc {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		jobID := r.PathValue("jobId")

		job, ok := deps.Jobs.Get(jobID)
		if !ok || job.JobReference.ProjectID != projectID {
			writeError(w, http.StatusNotFound, "notFound",
				"Not found: Job "+projectID+":"+jobID)
			return
		}
		if loc := r.URL.Query().Get("location"); loc != "" &&
			job.JobReference.Location != "" &&
			loc != job.JobReference.Location {
			writeError(w, http.StatusNotFound, "notFound",
				"Not found: Job "+projectID+":"+jobID+
					" in location "+loc)
			return
		}

		var (
			schema   *bqtypes.TableSchema
			allRows  []bqtypes.Row
			pageRows []bqtypes.Row
			dmlStats *bqtypes.DmlStats
		)
		if result := job.Result; result != nil {
			schema = result.Schema
			allRows = result.Rows
			dmlStats = result.DmlStats
		}

		// pageToken support: the registry never mints one, so any
		// non-empty value is a stale token from a prior emulator run
		// or a client that conflated tabledata.list with
		// getQueryResults. Respond with an empty terminal page so
		// polling loops complete cleanly.
		if r.URL.Query().Get("pageToken") == "" {
			start := uint64(0)
			if s := r.URL.Query().Get("startIndex"); s != "" {
				if v, err := strconv.ParseUint(s, 10, 64); err == nil {
					start = v
				}
			}
			limit := uint64(len(allRows))
			if s := r.URL.Query().Get("maxResults"); s != "" {
				if v, err := strconv.ParseUint(s, 10, 64); err == nil {
					limit = v
				}
			}
			total := uint64(len(allRows))
			if start > total {
				start = total
			}
			end := start + limit
			if end > total {
				end = total
			}
			pageRows = allRows[start:end]
		}

		jobRef := job.JobReference
		out := bqtypes.QueryResponse{
			Kind:                getQueryResultsKind,
			Schema:              schema,
			JobReference:        &jobRef,
			JobComplete:         true,
			TotalRows:           strconv.FormatUint(uint64(len(allRows)), 10),
			Rows:                pageRows,
			TotalBytesProcessed: job.Statistics.TotalBytesProcessed,
			Location:            jobRef.Location,
		}
		if dmlStats != nil {
			// DML replay: re-emit the same `dmlStats` /
			// `numDmlAffectedRows` envelope `jobs.query` sent
			// at submit time, and strip the SELECT-shape fields
			// (schema, rows, totalRows) the same way the
			// synchronous response does.
			out.DmlStats = dmlStats
			inserted, _ := strconv.ParseInt(dmlStats.InsertedRowCount, 10, 64)
			updated, _ := strconv.ParseInt(dmlStats.UpdatedRowCount, 10, 64)
			deleted, _ := strconv.ParseInt(dmlStats.DeletedRowCount, 10, 64)
			out.NumDmlAffectedRows = strconv.FormatInt(
				inserted+updated+deleted, 10)
			out.Schema = nil
			out.Rows = nil
			out.TotalRows = "0"
		}
		writeJSON(w, http.StatusOK, out)
	}
}
