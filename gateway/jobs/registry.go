// Package jobs is the gateway-side, in-memory record of every
// BigQuery job the emulator has accepted in this process. It feeds
// the synchronous `jobs.query` response (the `jobReference` and
// timing statistics it must emit alongside rows) and is the source
// of truth `jobs.get` / `jobs.list` will read from once those land.
//
// Scope today:
//
//   - One process-local Registry per gateway. State is volatile;
//     restarts wipe the table. Spanner-emulator does the same with
//     its in-memory metadata catalog.
//   - Jobs are minted by `jobs.query` (the sync query API) only.
//     `jobs.insert` -- the metadata-only POST that creates query /
//     load / copy / extract jobs asynchronously -- still returns the
//     gateway-only NotImplemented stub.
//   - Jobs are recorded as `DONE` straight away. The emulator runs
//     each query synchronously, so a pending/running window never
//     exists on the wire from the caller's perspective. Async
//     execution lands later when DML / long-running jobs need real
//     lifecycle transitions.
//
// The shape of `Job`, `Status`, and `Statistics` mirrors the subset of
// `https://docs.cloud.google.com/bigquery/docs/reference/rest/v2/jobs#Job`
// the emulator emits today. JSON tags match the upstream wire field
// names so a stored `*Job` round-trips through `jobs.get` without an
// extra translation layer.
package jobs

import (
	"strconv"
	"sync"
	"sync/atomic"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// JobKind is the value of the `kind` field on a BigQuery Job
// resource. Stable across all job types (query / load / copy /
// extract); the per-configuration discriminator lives under
// `configuration.{query,load,...}` on a real Job, which the
// emulator does not populate yet.
const JobKind = "bigquery#job"

// JobState mirrors the upstream `Job.status.state` enum: PENDING
// (admitted but not yet scheduled), RUNNING (work in progress),
// DONE (terminal -- success or failure determined by `errorResult`).
const (
	JobStatePending = "PENDING"
	JobStateRunning = "RUNNING"
	JobStateDone    = "DONE"
)

// Status mirrors the upstream `JobStatus` resource. ErrorResult is
// populated only when the job terminated with an error; Errors is
// a (potentially empty) list of warnings/errors collected during
// execution. Both are kept omitempty so a successful `jobs.query`
// reply doesn't carry empty arrays / null sentinel objects.
type Status struct {
	State       string               `json:"state"`
	ErrorResult *bqtypes.ErrorProto  `json:"errorResult,omitempty"`
	Errors      []bqtypes.ErrorProto `json:"errors,omitempty"`
}

// Statistics mirrors the subset of `JobStatistics` the emulator
// currently fills in. All four timestamp / byte fields are decimal
// strings on the wire per
// docs/bigquery/docs/reference/rest/v2/jobs/get.md#JobStatistics --
// even `totalBytesProcessed`, because BigQuery REST never emits
// 64-bit integers as JSON numbers (clients use `string` decoders).
type Statistics struct {
	CreationTime        string `json:"creationTime,omitempty"`
	StartTime           string `json:"startTime,omitempty"`
	EndTime             string `json:"endTime,omitempty"`
	TotalBytesProcessed string `json:"totalBytesProcessed,omitempty"`
}

// QueryResult is the cached result of a synchronous query, kept in
// the registry so a follow-up `jobs.getQueryResults` can replay the
// same schema and rows without re-running the SQL. Schema and Rows
// are stored in the BigQuery REST `f`/`v` shape so the handler can
// emit them verbatim.
//
// The registry holds the entire result set in memory; this matches
// the "single-page only" charter from
// `.cursor/plans/query-select-e2e_b3e4f5a6.plan.md`. Pagination
// (real `pageToken` lifecycle, cursored reads from a streaming
// engine) is deferred until long-running jobs land.
type QueryResult struct {
	Schema *bqtypes.TableSchema
	Rows   []bqtypes.Row
	// DmlStats is non-nil for an INSERT/UPDATE/DELETE/MERGE job and
	// nil for a SELECT/DDL job. When set, `jobs.getQueryResults`
	// surfaces the same `dmlStats` + `numDmlAffectedRows` envelope
	// the synchronous `jobs.query` response carried, so polling
	// BigQuery clients (e.g. the Go SDK's `JobIterator`) see the
	// row counts on the replay too.
	DmlStats *bqtypes.DmlStats
	// StatementType is the canonical BigQuery REST statement-type
	// string the engine trailed on the `jobs.query` response (e.g.
	// `SELECT`, `INSERT`, `CREATE_TABLE`). Stashed on the cached
	// result so `jobs.getQueryResults` can re-surface the same
	// `Job.statistics.query.statementType` envelope on the replay
	// without re-running the SQL.
	StatementType string
	// EmulatorRoute is the canonical lowercase-snake disposition
	// string the C++ coordinator's `RouteClassifier` chose for the
	// original query (`duckdb_native`, `semantic_executor`,
	// `control_op`, ...). It is an emulator-internal debug field;
	// `jobs.getQueryResults` only surfaces it to loopback callers
	// (the call site enforces the gating via
	// `middleware.IsLoopback`) so the public REST shape stays the
	// same.
	EmulatorRoute string
}

// Job is the gateway's view of a single BigQuery job. Today it's
// populated from the sync `jobs.query` path and is mostly metadata
// -- `configuration`, `selfLink`, `etag`, `user_email`, and the
// per-type `*Statistics` sub-objects are deferred until a handler
// actually needs them.
//
// Result is the cached query result, populated by
// `CompleteQueryWithResult` and consumed by `jobs.getQueryResults`.
// It is excluded from the JSON encoding because the upstream Job
// resource has no rows/schema field; result data is only emitted
// through the dedicated `QueryResponse`/`GetQueryResultsResponse`
// shapes.
type Job struct {
	Kind         string               `json:"kind,omitempty"`
	ID           string               `json:"id,omitempty"`
	JobReference bqtypes.JobReference `json:"jobReference"`
	Status       Status               `json:"status"`
	Statistics   Statistics           `json:"statistics"`
	Result       *QueryResult         `json:"-"`
}

// Registry is a process-local jobs table keyed by jobId. Reads /
// writes are concurrency-safe via sync.Map; the monotonic counter
// is bumped atomically so even within a single nanosecond two
// requests still see distinct ids.
//
// The map only holds successful or terminally-failed jobs because
// the emulator does not (yet) maintain a pending queue; see the
// package-level doc for why DONE-on-arrival is fine today.
type Registry struct {
	counter atomic.Uint64
	jobs    sync.Map
}

// NewRegistry returns a fresh, empty registry. Each gateway process
// gets one; tests can mint their own per-test for isolation without
// polluting a global.
func NewRegistry() *Registry {
	return &Registry{}
}

// NewJobID generates a jobId of the form `job_<unix_nanos>_<seq>`.
// The `job_` prefix matches the convention BigQuery and the official
// client libraries use for auto-generated ids (cf.
// `cloud.google.com/go/bigquery`'s `randomIDFn`). The trailing
// monotonic seq guarantees uniqueness even when two requests collide
// on the same nanosecond, which can happen on coarse-resolution
// clocks (Windows) under heavy concurrency.
func (r *Registry) NewJobID() string {
	seq := r.counter.Add(1)
	return "job_" +
		strconv.FormatInt(time.Now().UnixNano(), 10) + "_" +
		strconv.FormatUint(seq, 10)
}

// CompleteQuery records a query job that already finished -- the
// happy path for sync `jobs.query`. The returned Job carries a
// freshly minted jobReference plus the canonical Status / Statistics
// the caller stamps into the `QueryResponse`. The same `*Job` is
// stored in the registry so a follow-up `jobs.get` can return it
// verbatim.
//
// projectID flows from the URL path. location comes from the
// QueryRequest body (empty when the client did not specify one);
// matching BigQuery, the registry never invents a location.
// totalBytesProcessed reflects how many bytes the engine reported
// scanning -- 0 is acceptable when the engine has not wired the
// metric yet.
func (r *Registry) CompleteQuery(
	projectID, location string,
	totalBytesProcessed int64,
	start, end time.Time,
) *Job {
	return r.CompleteQueryWithResult(
		projectID, location, totalBytesProcessed, start, end, nil)
}

// CompleteQueryWithResult records a finished query job along with the
// schema + rows the engine produced. The result is cached on the Job
// so `jobs.getQueryResults` can replay it without re-running the SQL.
// Pass `result == nil` when no rows are available (the same behavior
// as `CompleteQuery`).
func (r *Registry) CompleteQueryWithResult(
	projectID, location string,
	totalBytesProcessed int64,
	start, end time.Time,
	result *QueryResult,
) *Job {
	jobID := r.NewJobID()
	j := &Job{
		Kind: JobKind,
		ID:   projectID + ":" + jobID,
		JobReference: bqtypes.JobReference{
			ProjectID: projectID,
			JobID:     jobID,
			Location:  location,
		},
		Status: Status{State: JobStateDone},
		Statistics: Statistics{
			CreationTime:        millisString(start),
			StartTime:           millisString(start),
			EndTime:             millisString(end),
			TotalBytesProcessed: strconv.FormatInt(totalBytesProcessed, 10),
		},
		Result: result,
	}
	r.jobs.Store(jobID, j)
	return j
}

// Get returns the Job recorded under jobID, or (nil, false) if no
// such job is in the registry. Used by `jobs.get` /
// `jobs.getQueryResults` once those handlers land.
func (r *Registry) Get(jobID string) (*Job, bool) {
	v, ok := r.jobs.Load(jobID)
	if !ok {
		return nil, false
	}
	return v.(*Job), true
}

// millisString converts t to BigQuery's wire timestamp format:
// decimal milliseconds since the Unix epoch. Used for all four
// `creationTime` / `startTime` / `endTime` / `totalBytesProcessed`-
// adjacent timestamps emitted in `Statistics`.
func millisString(t time.Time) string {
	return strconv.FormatInt(t.UnixMilli(), 10)
}
