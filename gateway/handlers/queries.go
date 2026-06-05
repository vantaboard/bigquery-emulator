package handlers

import (
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"net/url"
	"strconv"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/middleware"
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
// wire. The emulator only supports GoogleSQL because the engine pairs
// GoogleSQL's analyzer with the DuckDB transpiler. Queries that
// explicitly set `useLegacySql=true` are rejected with HTTP 400 +
// `reason: invalidQuery`; unset and `useLegacySql=false` are both
// treated as GoogleSQL.
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
		// GoogleSQL because the engine pairs GoogleSQL's analyzer
		// with the DuckDB transpiler; see docs/REST_API.md "SQL
		// dialect".
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
	req *bqtypes.QueryRequest,
) {
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
		Parameters:       parametersToEngineMap(req.Parameters),
	}

	resp, err := deps.Query.DryRun(r.Context(), engineReq)
	if queryGRPCToHTTPError(w, err) {
		return
	}

	out := bqtypes.QueryResponse{
		Kind:                queryResponseKind,
		Schema:              schemaFromProto(resp.GetSchema()),
		TotalBytesProcessed: formatDryRunBytes(resp.GetEstimatedBytesProcessed()),
		JobComplete:         true,
	}
	writeJSON(w, http.StatusOK, out)
}

// formatDryRunBytes renders estimated bytes as the decimal string
// BigQuery REST always emits, defaulting to "0" when the engine omits
// a value.
func formatDryRunBytes(estimated int64) string {
	if estimated < 0 {
		return "0"
	}
	return strconv.FormatInt(estimated, 10)
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
	req *bqtypes.QueryRequest,
) {
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
		Parameters:       parametersToEngineMap(req.Parameters),
	}

	start := time.Now().UTC()
	stream, err := deps.Query.ExecuteQuery(r.Context(), engineReq)
	if queryGRPCToHTTPError(w, err) {
		return
	}
	schema, dmlStats, rows, statementType, emulatorRoute, ok := streamQueryResults(w, stream)
	if !ok {
		return
	}
	end := time.Now().UTC()

	// Record the completed job (with its rows + schema cached)
	// before assembling the response so the jobReference we emit
	// is the same one a later jobs.get / jobs.getQueryResults will
	// find. The current registry does not track engine-side
	// bytes-processed yet, so we stamp 0; the long-running-jobs
	// follow-up wires the real metric.
	restSchema := schemaFromProto(schema)
	restDmlStats := dmlStatsFromProto(dmlStats)
	result := &jobs.QueryResult{
		Schema:        restSchema,
		Rows:          rows,
		DmlStats:      restDmlStats,
		StatementType: statementType,
		EmulatorRoute: emulatorRoute,
	}
	job := deps.Jobs.CompleteQueryWithResult(
		projectID, req.Location, 0, start, end, result)
	// Surface the `emulatorRoute` debug field only to loopback
	// callers so external BigQuery client libraries pointed at the
	// emulator see the same JSON shape they would against the
	// public REST surface. Non-loopback callers get an empty
	// string, which `assembleQueryResponse` translates into "no
	// emulatorRoute property" because the JSON struct tag is
	// `omitempty`. See
	// `docs/ENGINE_POLICY.md`.
	visibleRoute := ""
	if middleware.IsLoopback(r.Context()) {
		visibleRoute = emulatorRoute
	}
	out := assembleQueryResponse(
		job, restSchema, rows, dmlStats, restDmlStats, statementType,
		visibleRoute)
	writeJSON(w, http.StatusOK, out)
}

// parametersToEngineMap converts the REST `queryParameters` list
// into the engine's `map<string, QueryParameter>` proto field
// (defined in `proto/emulator.proto`). The gateway's wire payload
// is a list of `QueryParameter` objects, each carrying `name`,
// `parameterType`, and `parameterValue`; the engine speaks a
// name-keyed map plus a `type_kind` / `value_json` value pair.
//
// Named parameters flow through unchanged. Positional parameters
// are currently dropped on the floor (the engine proto's map shape
// cannot represent them; the engine binds them by `request.parameters`
// list order via the matching positional-parameter slot the
// analyzer assigned). Wiring positional parameters end-to-end
// requires either a proto change (move from `map` to `repeated`)
// or a synthetic key encoding; both are deferred to the
// gateway-parameters follow-up plan referenced from
// `docs/ENGINE_POLICY.md`'s deliberately-deferred list.
//
// Values with a missing `parameterType` are skipped because the
// engine cannot decode them without a type tag.
func parametersToEngineMap(in []bqtypes.QueryParameter) map[string]*enginepb.QueryParameter {
	if len(in) == 0 {
		return nil
	}
	out := make(map[string]*enginepb.QueryParameter, len(in))
	positionalIdx := 0
	for _, p := range in {
		if p.ParameterType == nil {
			continue
		}
		name := p.Name
		if name == "" {
			name = "p" + strconv.Itoa(positionalIdx)
			positionalIdx++
		}
		var value string
		if p.ParameterValue != nil {
			if len(p.ParameterValue.ArrayValues) > 0 ||
				len(p.ParameterValue.StructValues) > 0 {
				value = p.ParameterValue.ValueJSON()
			} else {
				value = p.ParameterValue.Value
			}
		}
		out[name] = &enginepb.QueryParameter{
			TypeKind:  p.ParameterType.Type,
			ValueJson: value,
		}
	}
	return out
}

// streamQueryResults drains the engine's query stream into the
// per-RPC schema, DML stats, row slice, trailing statement type, and
// trailing emulator route. Returns ok=false after emitting an HTTP
// error envelope, in which case the caller must stop processing the
// request.
//
// The proto contract (see `proto/emulator.proto::QueryResultRow`)
// allows up to five message kinds on a single reply: schema, cells,
// dml_stats, statement_type, and emulator_route. The schema and
// dml_stats messages pin themselves to the first arrival (later
// resends are ignored); the two trailers are each emitted at most
// once at end-of-stream.
func streamQueryResults(w http.ResponseWriter, stream enginepb.Query_ExecuteQueryClient) (
	*enginepb.TableSchema, *enginepb.DmlStats, []bqtypes.Row, string, string, bool,
) {
	var schema *enginepb.TableSchema
	var dmlStats *enginepb.DmlStats
	var statementType string
	var emulatorRoute string
	rows := make([]bqtypes.Row, 0)
	for {
		msg, err := stream.Recv()
		if errors.Is(err, io.EOF) {
			break
		}
		if queryGRPCToHTTPError(w, err) {
			return nil, nil, nil, "", "", false
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
		if st := msg.GetStatementType(); st != "" {
			// Trailing per-reply marker the engine emits to tell
			// the gateway which BigQuery REST `statementType`
			// envelope to populate. Keep the first non-empty value
			// and ignore later resends.
			if statementType == "" {
				statementType = st
			}
			continue
		}
		if er := msg.GetEmulatorRoute(); er != "" {
			// Trailing per-reply marker the engine emits with the
			// canonical lowercase-snake disposition string. The
			// gateway forwards it onto
			// `Job.statistics.query.emulatorRoute` for loopback
			// callers only (see
			// `gateway/middleware/loopback.go`); the gating lives
			// at the call site, not here, so the streaming pass
			// stays a straight collector.
			if emulatorRoute == "" {
				emulatorRoute = er
			}
			continue
		}
		rows = append(rows, bqtypes.CellsToRow(msg.GetCells()))
	}
	return schema, dmlStats, rows, statementType, emulatorRoute, true
}

// dmlStatsFromProto converts an engine-side DmlStats message into
// the REST-wire envelope. Returns nil when the engine never emitted
// a DmlStats summary (i.e. the statement was a SELECT, not DML).
func dmlStatsFromProto(d *enginepb.DmlStats) *bqtypes.DmlStats {
	if d == nil {
		return nil
	}
	return &bqtypes.DmlStats{
		InsertedRowCount: strconv.FormatInt(d.GetInsertedRowCount(), 10),
		UpdatedRowCount:  strconv.FormatInt(d.GetUpdatedRowCount(), 10),
		DeletedRowCount:  strconv.FormatInt(d.GetDeletedRowCount(), 10),
	}
}

// assembleQueryResponse builds the synchronous jobs.query response
// envelope: SELECT-shape (schema + rows + totalRows) by default,
// switching to the DML-shape (numDmlAffectedRows + zeroed selects)
// when the stream surfaced a DmlStats message. When the engine
// trailed a non-empty `statement_type` the gateway folds it into
// the BigQuery REST `Job.statistics.query.statementType` envelope;
// when `emulatorRoute` is non-empty (the caller already gated this
// on `middleware.IsLoopback`), it lands on the loopback-only
// `Job.statistics.query.emulatorRoute` debug field.
func assembleQueryResponse(job *jobs.Job, restSchema *bqtypes.TableSchema, rows []bqtypes.Row,
	dmlStats *enginepb.DmlStats, restDmlStats *bqtypes.DmlStats,
	statementType string,
	emulatorRoute string,
) bqtypes.QueryResponse {
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
	if statementType != "" || emulatorRoute != "" {
		out.Statistics = &bqtypes.JobStatistics{
			Query: &bqtypes.JobStatistics2{
				StatementType: statementType,
				EmulatorRoute: emulatorRoute,
			},
		}
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
	return out
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
// (`docs/ENGINE_POLICY.md`) limits this
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

		writeJSON(w, http.StatusOK, assembleGetQueryResultsResponse(r, job))
	}
}

// assembleGetQueryResultsResponse builds the JSON envelope
// `QueryGetResults` returns. Pulled out of the handler to keep its
// cyclomatic budget below the funlen cap once the
// loopback-gated `emulatorRoute` replay landed.
func assembleGetQueryResultsResponse(r *http.Request, job *jobs.Job) bqtypes.QueryResponse {
	var (
		schema        *bqtypes.TableSchema
		allRows       []bqtypes.Row
		dmlStats      *bqtypes.DmlStats
		statementType string
		emulatorRoute string
	)
	if result := job.Result; result != nil {
		schema = result.Schema
		allRows = result.Rows
		dmlStats = result.DmlStats
		statementType = result.StatementType
		emulatorRoute = result.EmulatorRoute
	}

	pageRows, pageToken := paginateResults(allRows, r.URL.Query())
	jobRef := job.JobReference
	out := bqtypes.QueryResponse{
		Kind:                getQueryResultsKind,
		Schema:              schema,
		JobReference:        &jobRef,
		JobComplete:         true,
		TotalRows:           strconv.FormatUint(uint64(len(allRows)), 10),
		Rows:                pageRows,
		PageToken:           pageToken,
		TotalBytesProcessed: job.Statistics.TotalBytesProcessed,
		Location:            jobRef.Location,
	}
	// Replay the loopback-only `emulatorRoute` debug field on
	// `Job.statistics.query` the same way `QueryRun` surfaced it
	// on the original `jobs.query` response: only when the
	// follow-up `getQueryResults` caller is also loopback. The
	// `statementType` field stays unconditional (it's a public
	// BigQuery REST field).
	visibleRoute := ""
	if middleware.IsLoopback(r.Context()) {
		visibleRoute = emulatorRoute
	}
	if statementType != "" || visibleRoute != "" {
		out.Statistics = &bqtypes.JobStatistics{
			Query: &bqtypes.JobStatistics2{
				StatementType: statementType,
				EmulatorRoute: visibleRoute,
			},
		}
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
	return out
}

// defaultQueryResultsPageSize mirrors BigQuery's documented default
// `maxResults` for jobs.getQueryResults when the caller omits it.
const defaultQueryResultsPageSize uint64 = 10000

// paginateResults slices cached query rows using startIndex,
// maxResults, and pageToken. pageToken (when set) is a decimal string
// encoding the next start row index, matching tabledata.list.
func paginateResults(allRows []bqtypes.Row, q url.Values) ([]bqtypes.Row, string) {
	total := uint64(len(allRows))
	start := parseUintQuery(q, "startIndex", 0)
	if tok := q.Get("pageToken"); tok != "" {
		if off, err := strconv.ParseUint(tok, 10, 64); err == nil {
			start = off
		} else {
			return nil, ""
		}
	}
	limit := defaultQueryResultsPageSize
	if q.Get("maxResults") != "" {
		limit = parseUintQuery(q, "maxResults", defaultQueryResultsPageSize)
	}
	if start >= total {
		return nil, ""
	}
	end := min(start+limit, total)
	var nextToken string
	if end < total {
		nextToken = strconv.FormatUint(end, 10)
	}
	return allRows[start:end], nextToken
}

// parseUintQuery returns the named query parameter as a uint64,
// falling back to defaultVal when the value is missing or unparsable.
// Pulled out so the pagination helper stops nesting if-inside-if.
func parseUintQuery(q url.Values, key string, defaultVal uint64) uint64 {
	s := q.Get(key)
	if s == "" {
		return defaultVal
	}
	v, err := strconv.ParseUint(s, 10, 64)
	if err != nil {
		return defaultVal
	}
	return v
}
