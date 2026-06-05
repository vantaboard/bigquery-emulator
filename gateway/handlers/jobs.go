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
	"github.com/vantaboard/bigquery-emulator/gateway/load"
	"github.com/vantaboard/bigquery-emulator/gateway/middleware"
)

// jobListKind is the value the BigQuery REST API returns for the
// `kind` field of a JobList response. See
// docs/bigquery/docs/reference/rest/v2/jobs/list.md.
const jobListKind = "bigquery#jobList"

// jobCancelKind is the value of `kind` on a JobCancelResponse, the
// envelope `jobs.cancel` returns. The body wraps the updated Job.
// See docs/bigquery/docs/reference/rest/v2/jobs/cancel.md.
const jobCancelKind = "bigquery#jobCancelResponse"

// jobConfigurationKindQuery is the value of `configuration.jobType`
// for a query job. The wire schema spells the type discriminator in
// upper-case (QUERY / LOAD / COPY / EXTRACT); we round-trip it as the
// caller posts it but stamp it explicitly when the caller leaves it
// empty so a subsequent `jobs.get` doesn't lose the discriminator.
const (
	jobConfigurationKindQuery   = "QUERY"
	jobConfigurationKindLoad    = "LOAD"
	jobConfigurationKindCopy    = "COPY"
	jobConfigurationKindExtract = "EXTRACT"
)

// queryParamTrue is the wire literal BigQuery's REST surface uses for
// boolean query parameters (e.g. `allUsers=true`, `deleteContents=true`).
// Promoted to a constant so the goconst lint counter does not flag
// the repeated literal across handlers.
const queryParamTrue = "true"

// JobList implements `bigquery.jobs.list`:
//
//	GET /bigquery/v2/projects/{projectId}/jobs
//
// Supports the documented query parameters `allUsers`, `maxResults`,
// `minCreationTime`, `maxCreationTime`, `pageToken`, `projection`,
// `stateFilter`, and `parentJobId`. `allUsers=true` is rejected with
// a documented 501 because the emulator does not have an auth
// context to scope cross-user listings to; every other documented
// parameter is honored by `Registry.ListByProject`.
//
// The per-entry shape mirrors upstream's "minimal" projection
// (`kind`, `id`, `jobReference`, `state`, `status`, `statistics`,
// `configuration`, `user_email`); we surface the full registry Job
// today because the emulator's per-job payload is already small and
// projection-trimming has no behavioral upside before plan tp08
// inflates the schema.
func JobList(deps Dependencies) http.HandlerFunc {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		q := r.URL.Query()
		if q.Get("allUsers") == queryParamTrue {
			writeError(w, http.StatusNotImplemented, reasonNotImplemented,
				"jobs.list with allUsers=true is not supported; "+
					"the emulator has no auth context to scope cross-user "+
					"listings.")
			return
		}
		opts := jobs.ListOptions{
			MaxResults:      clampToInt(parseUintQuery(q, "maxResults", 0)),
			PageToken:       q.Get("pageToken"),
			ParentJobID:     q.Get("parentJobId"),
			MinCreationTime: clampToInt64(parseUintQuery(q, "minCreationTime", 0)),
			MaxCreationTime: clampToInt64(parseUintQuery(q, "maxCreationTime", 0)),
			StateFilter:     q["stateFilter"],
		}
		items, nextPageToken := deps.Jobs.ListByProject(projectID, opts)
		resp := map[string]any{
			resourceKeyKind: jobListKind,
			"jobs":          items,
		}
		if nextPageToken != "" {
			resp["nextPageToken"] = nextPageToken
		}
		writeJSON(w, http.StatusOK, resp)
	}
}

// JobInsert implements `bigquery.jobs.insert` (metadata-only variant):
//
//	POST /bigquery/v2/projects/{projectId}/jobs
//
// The body is a Job resource with `configuration.{query|load|copy|
// extract}`. Query jobs execute synchronously through the engine;
// load / copy / extract dispatch and round-trip configuration with
// per-type statistics but defer byte-level work to plans tp08-04/05.
//
// For the query branch the handler:
//
//  1. Mints (or honors a caller-supplied) jobId on the inbound
//     JobReference.
//  2. Forwards the SQL to `enginepb.Query.ExecuteQuery` -- the same
//     RPC `QueryRun` (jobs.query) uses -- so the engine path is
//     shared. The streamed schema / rows / dml stats are captured on
//     the registry's `QueryResult` so a follow-up
//     `jobs.getQueryResults` replays them.
//  3. Records the resulting Job in `deps.Jobs` so a subsequent
//     `jobs.list` / `jobs.get` / `jobs.cancel` / `jobs.delete` can
//     find it by id, then returns the Job verbatim with HTTP 200.
//
// Engine-side analysis errors (table not found, syntax error, ...)
// are captured into `Status.ErrorResult` instead of being surfaced
// as a 4xx — that mirrors BigQuery's `jobs.insert` contract, which
// always succeeds at the API level and reflects per-query failures
// through the Job's status. Transport-level failures (the engine
// process unreachable, `deps.Query` nil) still return 501 so unit-
// mode runs (`task emulator:run --engine_binary=""`) keep producing
// a structured error envelope.
func JobInsert(deps Dependencies) http.HandlerFunc {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return func(w http.ResponseWriter, r *http.Request) {
		body, err := io.ReadAll(r.Body)
		if err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid,
				"Could not read job request body: "+err.Error())
			return
		}
		var posted jobs.Job
		if len(body) > 0 {
			if err := json.Unmarshal(body, &posted); err != nil {
				writeError(w, http.StatusBadRequest, reasonInvalid,
					"Could not parse job request body as JSON: "+err.Error())
				return
			}
		}
		cfg := posted.Configuration
		if cfg == nil {
			writeError(w, http.StatusBadRequest, reasonInvalid,
				"Job configuration is required.")
			return
		}
		switch {
		case cfg.Query != nil:
			if deps.Query == nil {
				NotImplemented(w, r)
				return
			}
			runSyncQueryInsert(deps, w, r, &posted, cfg)
		case cfg.Load != nil:
			runSyncLoadInsert(deps, w, r, &posted, cfg)
		case cfg.Copy != nil:
			runSyncCopyInsert(deps, w, r, &posted, cfg)
		case cfg.Extract != nil:
			runSyncExtractInsert(deps, w, r, &posted, cfg)
		default:
			writeError(w, http.StatusNotImplemented, reasonNotImplemented,
				"jobs.insert: configuration must include query, load, copy, or extract.")
		}
	}
}

// JobInsertUpload implements `bigquery.jobs.insert` (media-upload variant):
//
//	POST /upload/bigquery/v2/projects/{projectId}/jobs
//
// Selected via `?uploadType=multipart` or `?uploadType=resumable`. The
// emulator must accept both because the official client libraries pick
// one based on payload size. Plan tp04 leaves this surface untouched —
// load uploads land in plan tp08.
func JobInsertUpload(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobGet implements `bigquery.jobs.get`:
//
//	GET /bigquery/v2/projects/{projectId}/jobs/{jobId}
//
// Looks up the job in `deps.Jobs` by jobId, returning the stored Job
// verbatim. Mismatched projectIds (URL path vs. stored reference) and
// missing entries both map to a BigQuery-shaped 404 so the upstream
// `not found` contract holds; the `location` query parameter, when
// set, is matched against the stored jobReference and a wrong
// location also returns 404 (mirroring the upstream behavior of
// hiding cross-region jobs behind the same envelope).
func JobGet(deps Dependencies) http.HandlerFunc {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		jobID := r.PathValue("jobId")
		job, ok := deps.Jobs.Get(jobID)
		if !ok || job.JobReference.ProjectID != projectID {
			writeJobNotFound(w, projectID, jobID, "")
			return
		}
		if loc := r.URL.Query().Get("location"); loc != "" &&
			job.JobReference.Location != "" &&
			loc != job.JobReference.Location {
			writeJobNotFound(w, projectID, jobID, loc)
			return
		}
		writeJSON(w, http.StatusOK, job)
	}
}

// JobCancel implements `bigquery.jobs.cancel`:
//
//	POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel
//
// Returns a `JobCancelResponse` (kind + job) per the upstream wire
// shape. The registry flips the job to DONE with CancelRequested=true
// for non-terminal entries; terminal jobs (DONE / cancelled) get the
// cancel-requested flag stamped but their state stays put — the
// upstream API is documented as idempotent.
func JobCancel(deps Dependencies) http.HandlerFunc {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		jobID := r.PathValue("jobId")
		job, ok := deps.Jobs.Get(jobID)
		if !ok || job.JobReference.ProjectID != projectID {
			writeJobNotFound(w, projectID, jobID, "")
			return
		}
		updated, ok := deps.Jobs.Cancel(jobID)
		if !ok {
			writeJobNotFound(w, projectID, jobID, "")
			return
		}
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind: jobCancelKind,
			"job":           updated,
		})
	}
}

// JobDelete implements `bigquery.jobs.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete
//
// The literal `/delete` suffix is the upstream URL template, not a
// typo (see docs/bigquery/docs/reference/rest/v2/jobs/delete.md).
// Removes job metadata; if {jobId} is a script parent, child job
// metadata is also dropped in the same call. Returns HTTP 204 on
// success; 404 with the BigQuery error envelope when the jobId is
// unknown.
func JobDelete(deps Dependencies) http.HandlerFunc {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		jobID := r.PathValue("jobId")
		job, ok := deps.Jobs.Get(jobID)
		if !ok || job.JobReference.ProjectID != projectID {
			writeJobNotFound(w, projectID, jobID, "")
			return
		}
		if !deps.Jobs.Delete(jobID) {
			writeJobNotFound(w, projectID, jobID, "")
			return
		}
		w.WriteHeader(http.StatusNoContent)
	}
}

// clampToInt safely narrows a uint64 wire value (BigQuery REST
// transmits maxResults / page size as decimal strings parsed as
// uint64 here) into Go's platform-native int. Values above `math.
// MaxInt` saturate at the platform max so the gosec G115 narrowing
// guard does not need a per-call branch in every handler.
func clampToInt(v uint64) int {
	if v > uint64(maxInt) {
		return maxInt
	}
	return int(v)
}

// clampToInt64 saturates a uint64 at `math.MaxInt64` before narrowing
// to int64. The BigQuery REST surface documents creation timestamps
// as ms-since-epoch so the practical range stays well below 2^63, but
// the explicit guard keeps the gosec G115 lint clean.
func clampToInt64(v uint64) int64 {
	if v > uint64(int64Max) {
		return int64Max
	}
	return int64(v)
}

// maxInt and int64Max are platform constants used by the clamp
// helpers above. Spelled out here (instead of importing `math`) to
// keep the import surface minimal for the few callers that need
// them.
const (
	maxInt   = int(^uint(0) >> 1)
	int64Max = int64(^uint64(0) >> 1)
)

// millisString converts t to BigQuery's wire timestamp format:
// decimal milliseconds since the Unix epoch. The handlers reach for
// this on per-call timestamps (`finalizeDoneJob`, `finalizeFailedJob`)
// the way the jobs package's `Statistics` block already serializes
// `creationTime` / `startTime` / `endTime`.
func millisString(t time.Time) string {
	return strconv.FormatInt(t.UnixMilli(), 10)
}

// writeJobNotFound emits the BigQuery-shaped 404 envelope `jobs.get`,
// `jobs.cancel`, `jobs.delete`, and `jobs.getQueryResults` all return
// for an unknown job. When `location` is non-empty the message
// appends "in location <loc>" so the caller can tell a wrong-region
// lookup apart from a truly missing entry.
func writeJobNotFound(w http.ResponseWriter, projectID, jobID, location string) {
	msg := "Not found: Job " + projectID + ":" + jobID
	if location != "" {
		msg += " in location " + location
	}
	writeError(w, http.StatusNotFound, reasonNotFound, msg)
}

// runSyncQueryInsert is the sync slice of `JobInsert`'s implementation.
// Pulled out of the handler closure so the inbound-body validation +
// auth gating stays a thin top-level switch (cyclop / funlen caps).
//
// The flow mirrors `runQueryExecute` (the `jobs.query` handler's
// engine call) so analysis / streaming errors funnel through the
// same gRPC-to-HTTP mapping. The single difference is that
// `JobInsert` always returns a Job on success, never the bare
// `QueryResponse` payload `jobs.query` emits — the upstream API
// surfaces row data only on the sync `jobs.query` and follow-up
// `jobs.getQueryResults` calls.
func runSyncQueryInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	if cfg.DryRun {
		runSyncQueryDryRunInsert(deps, w, r, posted, cfg)
		return
	}
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)

	useLegacy := false
	if cfg.Query.UseLegacySQL != nil {
		useLegacy = *cfg.Query.UseLegacySQL
	}
	engineReq := &enginepb.QueryRequest{
		ProjectId:        projectID,
		DefaultDatasetId: defaultDatasetID(cfg.Query.DefaultDataset),
		Sql:              cfg.Query.Query,
		UseLegacySql:     useLegacy,
		Parameters:       parametersToEngineMap(cfg.Query.QueryParameters),
	}

	start := time.Now().UTC()
	stream, err := deps.Query.ExecuteQuery(r.Context(), engineReq)
	if err != nil {
		finalizeFailedJob(deps, job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	schema, dmlStats, rows, statementType, emulatorRoute, streamErr := drainSyncStream(stream)
	end := time.Now().UTC()
	if streamErr != nil {
		finalizeFailedJob(deps, job, start, streamErr)
		writeJSON(w, http.StatusOK, job)
		return
	}
	finalizeDoneJob(deps, job, start, end, schema, dmlStats, rows, statementType, emulatorRoute, r)
	writeJSON(w, http.StatusOK, job)
}

// runSyncLoadInsert accepts a load-job body, fetches source bytes, parses
// CSV/JSON, and bulk-inserts into the destination table.
func runSyncLoadInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)
	start := time.Now().UTC()
	if deps.Catalog == nil {
		finalizeDeferredDataPlaneJob(job, cfg, start, "load")
		writeJSON(w, http.StatusOK, job)
		return
	}
	result, err := load.Execute(r.Context(), deps.Catalog, cfg.Load, projectID)
	if err != nil {
		finalizeFailedJob(deps, job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	finalizeSuccessfulLoadJob(job, start, result)
	writeJSON(w, http.StatusOK, job)
}

// runSyncCopyInsert accepts a copy-job body. Row copy lands in thirdparty-05.
func runSyncCopyInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)
	start := time.Now().UTC()
	finalizeDeferredDataPlaneJob(job, cfg, start, "copy")
	writeJSON(w, http.StatusOK, job)
}

// runSyncExtractInsert accepts an extract-job body. GCS export lands in thirdparty-05.
func runSyncExtractInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)
	start := time.Now().UTC()
	finalizeDeferredDataPlaneJob(job, cfg, start, "extract")
	writeJSON(w, http.StatusOK, job)
}

// finalizeSuccessfulLoadJob marks a LOAD job DONE with statistics.load and
// no errorResult.
func finalizeSuccessfulLoadJob(job *jobs.Job, start time.Time, result load.Result) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = nil
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.Load = load.FormatStatistics(result)
}

// finalizeDeferredDataPlaneJob marks a tp08 foundation job DONE with an
// errorResult naming the deferred data-plane plan and zeroed per-type
// statistics counters. HTTP stays 200 per the jobs.insert contract;
// clients inspect job.status.errorResult.
func finalizeDeferredDataPlaneJob(job *jobs.Job, cfg *jobs.JobConfiguration, start time.Time, kind string) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = &bqtypes.ErrorProto{
		Reason: reasonNotImplemented,
		Message: "jobs.insert: " + kind + " job data plane not yet implemented; " +
			"load / copy / extract execution lands in thirdparty-04/05.",
	}
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	switch kind {
	case "load":
		inputFiles := "0"
		if cfg.Load != nil {
			inputFiles = strconv.Itoa(len(cfg.Load.SourceURIs))
		}
		job.Statistics.Load = &jobs.LoadStatistics{
			InputFiles:     inputFiles,
			InputFileBytes: "0",
			OutputRows:     "0",
			OutputBytes:    "0",
			BadRecords:     "0",
		}
	case "copy":
		job.Statistics.Copy = &jobs.CopyStatistics{
			CopiedRows:         "0",
			CopiedLogicalBytes: "0",
		}
	case "extract":
		var counts []string
		if cfg.Extract != nil && len(cfg.Extract.DestinationURIs) > 0 {
			counts = make([]string, len(cfg.Extract.DestinationURIs))
			for i := range counts {
				counts[i] = "0"
			}
		}
		job.Statistics.Extract = &jobs.ExtractStatistics{
			DestinationURIFileCounts: counts,
			InputBytes:               "0",
		}
	}
}

// runSyncQueryDryRunInsert handles jobs.insert with configuration.dryRun
// set. It forwards the SQL to enginepb.Query.DryRun and returns a DONE
// job whose statistics.totalBytesProcessed mirrors jobs.query dry-run.
func runSyncQueryDryRunInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)

	useLegacy := false
	if cfg.Query.UseLegacySQL != nil {
		useLegacy = *cfg.Query.UseLegacySQL
	}
	engineReq := &enginepb.QueryRequest{
		ProjectId:        projectID,
		DefaultDatasetId: defaultDatasetID(cfg.Query.DefaultDataset),
		Sql:              cfg.Query.Query,
		UseLegacySql:     useLegacy,
		Parameters:       parametersToEngineMap(cfg.Query.QueryParameters),
	}

	start := time.Now().UTC()
	resp, err := deps.Query.DryRun(r.Context(), engineReq)
	end := time.Now().UTC()
	if err != nil {
		finalizeFailedJob(deps, job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	job.Status.State = jobs.JobStateDone
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.TotalBytesProcessed = formatDryRunBytes(resp.GetEstimatedBytesProcessed())
	writeJSON(w, http.StatusOK, job)
}

// newPendingJob seeds the registry with a PENDING entry derived from
// the inbound `jobs.insert` body and returns the writable handle the
// rest of the flow stamps results onto. ProjectID always wins over
// the body's `jobReference.projectId` (URL path is authoritative);
// the caller-provided jobId, if any, is preserved verbatim.
func newPendingJob(deps Dependencies, projectID string, posted *jobs.Job, cfg *jobs.JobConfiguration) *jobs.Job {
	jobID := posted.JobReference.JobID
	if jobID == "" {
		jobID = deps.Jobs.NewJobID()
	}
	if cfg.JobType == "" {
		switch {
		case cfg.Load != nil:
			cfg.JobType = jobConfigurationKindLoad
		case cfg.Copy != nil:
			cfg.JobType = jobConfigurationKindCopy
		case cfg.Extract != nil:
			cfg.JobType = jobConfigurationKindExtract
		default:
			cfg.JobType = jobConfigurationKindQuery
		}
	}
	job := &jobs.Job{
		Kind: jobs.JobKind,
		ID:   projectID + ":" + jobID,
		JobReference: bqtypes.JobReference{
			ProjectID: projectID,
			JobID:     jobID,
			Location:  posted.JobReference.Location,
		},
		Status:        jobs.Status{State: jobs.JobStatePending},
		Statistics:    jobs.Statistics{CreationTime: nowMillis()},
		Configuration: cfg,
	}
	deps.Jobs.Register(job)
	return job
}

// finalizeFailedJob flips a PENDING job to DONE + errorResult derived
// from the engine error and records the failure timestamps. The
// gateway leaves the message verbatim because BigQuery's REST surface
// surfaces analyzer errors with their raw position-tagged shape
// (e.g. "Unrecognized name: x [at 1:8]"); rewriting them would lose
// the column / row markers the upstream samples assert on.
//
// We deliberately leave `Status.Errors` nil: the upstream `jobs.insert`
// contract returns the job synchronously with a status envelope the
// caller polls later, and the official BigQuery Node client wraps any
// non-nil `status.errors` array into an `ApiError` immediately (see
// `@google-cloud/bigquery/src/bigquery.ts` -> createJob), which would
// turn an "engine reports analysis failure" into a thrown exception
// instead of a Job-with-error caller can inspect. `errorResult` is
// the right field for that single terminal error; clients that want
// the full list compose it from `errorResult` + any execution-time
// warnings (none today; the emulator runs jobs to completion).
func finalizeFailedJob(_ Dependencies, job *jobs.Job, start time.Time, err error) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = &bqtypes.ErrorProto{
		Reason:  reasonInvalidQuery,
		Message: bqStyleMessage(err.Error()),
	}
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
}

// finalizeDoneJob stamps the success terminus on a PENDING job and
// caches the streamed result on the registry entry so a follow-up
// `jobs.getQueryResults` replays the same schema + rows without a
// re-execute. The loopback gating on `EmulatorRoute` mirrors what
// `QueryRun` does: only loopback callers see the debug field.
func finalizeDoneJob(_ Dependencies, job *jobs.Job, start, end time.Time,
	schema *enginepb.TableSchema, dmlStats *enginepb.DmlStats, rows []bqtypes.Row,
	statementType, emulatorRoute string, r *http.Request,
) {
	job.Status.State = jobs.JobStateDone
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.TotalBytesProcessed = "0"
	restSchema := schemaFromProto(schema)
	restDmlStats := dmlStatsFromProto(dmlStats)
	visibleRoute := ""
	if middleware.IsLoopback(r.Context()) {
		visibleRoute = emulatorRoute
	}
	job.Result = &jobs.QueryResult{
		Schema:        restSchema,
		Rows:          rows,
		DmlStats:      restDmlStats,
		StatementType: statementType,
		EmulatorRoute: visibleRoute,
	}
}

// drainSyncStream is the JobInsert flavor of `streamQueryResults`:
// same proto contract, different error reporting. Whereas
// `streamQueryResults` writes an HTTP envelope and returns an
// `ok=false` short-circuit, this helper returns the raw stream error
// so the caller can fold it into the Job's status.
func drainSyncStream(stream enginepb.Query_ExecuteQueryClient) (
	*enginepb.TableSchema, *enginepb.DmlStats, []bqtypes.Row, string, string, error,
) {
	var schema *enginepb.TableSchema
	var dmlStats *enginepb.DmlStats
	var statementType string
	var emulatorRoute string
	rows := make([]bqtypes.Row, 0)
	for {
		msg, err := stream.Recv()
		if err != nil {
			if errors.Is(err, io.EOF) {
				break
			}
			return nil, nil, nil, "", "", err
		}
		if s := msg.GetSchema(); s != nil {
			if schema == nil {
				schema = s
			}
			continue
		}
		if d := msg.GetDmlStats(); d != nil {
			if dmlStats == nil {
				dmlStats = d
			}
			continue
		}
		if st := msg.GetStatementType(); st != "" {
			if statementType == "" {
				statementType = st
			}
			continue
		}
		if er := msg.GetEmulatorRoute(); er != "" {
			if emulatorRoute == "" {
				emulatorRoute = er
			}
			continue
		}
		rows = append(rows, bqtypes.CellsToRow(msg.GetCells()))
	}
	return schema, dmlStats, rows, statementType, emulatorRoute, nil
}

// defaultDatasetID extracts the dataset ID from an optional
// `defaultDataset` reference, returning empty when the field is
// absent. The wire field on the engine carries the dataset ID only;
// the project comes from `project_id`.
func defaultDatasetID(ref *bqtypes.DatasetReference) string {
	if ref == nil {
		return ""
	}
	return ref.DatasetID
}
