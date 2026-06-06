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
//   - Jobs are minted by `jobs.query` (the sync query API) and the
//     sync slice of `jobs.insert` (query / load / copy / extract).
//     Load / copy / extract insert paths dispatch and round-trip
//     configuration but defer byte-level work to plans tp08-04/05.
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
	"encoding/json"
	"fmt"
	"slices"
	"strconv"
	"strings"
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

// stateFilterAliases maps the lowercase wire spelling BigQuery
// accepts on `?stateFilter=` query parameters to the canonical
// upper-case `Status.State` value stored in the registry. The
// upstream API documents the parameter values as `pending`,
// `running`, and `done`; the response stamps the uppercase variant.
// Centralized here so `ListByProject` and tests share one source.
var stateFilterAliases = map[string]string{
	"pending": JobStatePending,
	"running": JobStateRunning,
	"done":    JobStateDone,
}

// Status mirrors the upstream `JobStatus` resource. ErrorResult is
// populated only when the job terminated with an error; Errors is
// a (potentially empty) list of warnings/errors collected during
// execution. Both are kept omitempty so a successful `jobs.query`
// reply doesn't carry empty arrays / null sentinel objects.
//
// CancelRequested mirrors the upstream `JobStatus.cancelRequested`
// flag the `JobCancel` handler stamps on the response. The gateway
// runs every job synchronously today so the flag flips to true at
// the same instant the entry's state moves to DONE; the field is
// omitempty so jobs that were never cancelled keep the same compact
// wire shape `jobs.query` emits.
type Status struct {
	State           string               `json:"state"`
	ErrorResult     *bqtypes.ErrorProto  `json:"errorResult,omitempty"`
	Errors          []bqtypes.ErrorProto `json:"errors,omitempty"`
	CancelRequested bool                 `json:"cancelRequested,omitempty"`
}

// JobConfiguration mirrors the subset of the upstream
// `JobConfiguration` resource the gateway round-trips through the
// registry. The per-type sub-objects (`Query`, `Load`, `Copy`,
// `Extract`) are the dispatch discriminator at `jobs.insert` time;
// everything else round-trips opaquely so a subsequent `jobs.get`
// echoes back the same shape the caller posted.
type JobConfiguration struct {
	JobType string                   `json:"jobType,omitempty"` // QUERY | LOAD | COPY | EXTRACT
	Query   *JobConfigurationQuery   `json:"query,omitempty"`
	Load    *JobConfigurationLoad    `json:"load,omitempty"`
	Copy    *JobConfigurationCopy    `json:"copy,omitempty"`
	Extract *JobConfigurationExtract `json:"extract,omitempty"`
	Labels  map[string]string        `json:"labels,omitempty"`
	DryRun  bool                     `json:"dryRun,omitempty"`
}

// JobConfigurationQuery is the per-query slice of a JobConfiguration.
// Only fields the gateway currently echoes back on `jobs.get` are
// modelled; the long tail (destination table, scheduling, encryption,
// ...) is deferred until a handler reads them.
type JobConfigurationQuery struct {
	Query                              string                                       `json:"query"`
	DefaultDataset                     *bqtypes.DatasetReference                    `json:"defaultDataset,omitempty"`
	UseLegacySQL                       *bool                                        `json:"useLegacySql,omitempty"`
	ParameterMode                      string                                       `json:"parameterMode,omitempty"`
	QueryParameters                    []bqtypes.QueryParameter                     `json:"queryParameters,omitempty"`
	TableDefinitions                   map[string]bqtypes.ExternalDataConfiguration `json:"tableDefinitions,omitempty"`
	DestinationTable                   *bqtypes.TableReference                      `json:"destinationTable,omitempty"`
	WriteDisposition                   string                                       `json:"writeDisposition,omitempty"`
	SchemaUpdateOptions                []string                                     `json:"schemaUpdateOptions,omitempty"`
	Clustering                         *bqtypes.Clustering                          `json:"clustering,omitempty"`
	TimePartitioning                   *bqtypes.TimePartitioning                    `json:"timePartitioning,omitempty"`
	DestinationEncryptionConfiguration *bqtypes.EncryptionConfiguration             `json:"destinationEncryptionConfiguration,omitempty"`
	CreateSession                      bool                                         `json:"createSession,omitempty"`
	ConnectionProperties               []bqtypes.ConnectionProperty                 `json:"connectionProperties,omitempty"`
}

// UnmarshalJSON accepts writeDisposition as a JSON string or a
// one-element string array (node relaxColumnQueryAppend sample).
func (c *JobConfigurationQuery) UnmarshalJSON(data []byte) error {
	type alias JobConfigurationQuery
	var raw struct {
		alias
		WriteDisposition json.RawMessage `json:"writeDisposition,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*c = JobConfigurationQuery(raw.alias)
	if len(raw.WriteDisposition) == 0 {
		return nil
	}
	wd, err := bqtypes.UnmarshalWriteDisposition(raw.WriteDisposition)
	if err != nil {
		return err
	}
	c.WriteDisposition = wd
	return nil
}

// JobConfigurationLoad is the per-load slice of a JobConfiguration.
// Fields mirror the minimum upstream REST shape thirdparty samples
// exercise; format readers and GCS byte I/O land in plan tp08-04.
type JobConfigurationLoad struct {
	SourceURIs                         []string                         `json:"sourceUris,omitempty"`
	DestinationTable                   *bqtypes.TableReference          `json:"destinationTable,omitempty"`
	SourceFormat                       string                           `json:"sourceFormat,omitempty"`
	WriteDisposition                   string                           `json:"writeDisposition,omitempty"`
	Schema                             *bqtypes.TableSchema             `json:"schema,omitempty"`
	Autodetect                         bool                             `json:"autodetect,omitempty"`
	SchemaUpdateOptions                []string                         `json:"schemaUpdateOptions,omitempty"`
	DestinationEncryptionConfiguration *bqtypes.EncryptionConfiguration `json:"destinationEncryptionConfiguration,omitempty"`
	Clustering                         *bqtypes.Clustering              `json:"clustering,omitempty"`
	TimePartitioning                   *bqtypes.TimePartitioning        `json:"timePartitioning,omitempty"`
	skipLeadingRows                    int                              // set via UnmarshalJSON; REST sends int or string
}

// SkipLeadingRows returns the number of leading CSV rows to skip.
func (c *JobConfigurationLoad) SkipLeadingRows() int {
	if c == nil {
		return 0
	}
	return c.skipLeadingRows
}

// UnmarshalJSON accepts skipLeadingRows as JSON number or decimal string,
// matching the official Python/Node client wire shape. writeDisposition
// may also be posted as a one-element string array.
func (c *JobConfigurationLoad) UnmarshalJSON(data []byte) error {
	type alias JobConfigurationLoad
	var raw struct {
		alias
		SkipLeadingRows  any             `json:"skipLeadingRows,omitempty"`
		WriteDisposition json.RawMessage `json:"writeDisposition,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*c = JobConfigurationLoad(raw.alias)
	if wd, err := bqtypes.UnmarshalWriteDisposition(raw.WriteDisposition); err != nil {
		return err
	} else if wd != "" {
		c.WriteDisposition = wd
	}
	if raw.SkipLeadingRows == nil {
		return nil
	}
	switch v := raw.SkipLeadingRows.(type) {
	case float64:
		c.skipLeadingRows = int(v)
	case string:
		n, err := strconv.Atoi(v)
		if err != nil {
			return fmt.Errorf("skipLeadingRows: %w", err)
		}
		c.skipLeadingRows = n
	default:
		return fmt.Errorf("skipLeadingRows: unsupported type %T", v)
	}
	return nil
}

// UnmarshalJSON accepts writeDisposition as a string or a
// one-element string array.
func (c *JobConfigurationCopy) UnmarshalJSON(data []byte) error {
	type alias JobConfigurationCopy
	var raw struct {
		alias
		WriteDisposition json.RawMessage `json:"writeDisposition,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*c = JobConfigurationCopy(raw.alias)
	wd, err := bqtypes.UnmarshalWriteDisposition(raw.WriteDisposition)
	if err != nil {
		return err
	}
	c.WriteDisposition = wd
	return nil
}

// JobConfigurationCopy is the per-copy slice of a JobConfiguration.
type JobConfigurationCopy struct {
	SourceTable                        *bqtypes.TableReference          `json:"sourceTable,omitempty"`
	SourceTables                       []bqtypes.TableReference         `json:"sourceTables,omitempty"`
	DestinationTable                   *bqtypes.TableReference          `json:"destinationTable,omitempty"`
	WriteDisposition                   string                           `json:"writeDisposition,omitempty"`
	CreateDisposition                  string                           `json:"createDisposition,omitempty"`
	DestinationEncryptionConfiguration *bqtypes.EncryptionConfiguration `json:"destinationEncryptionConfiguration,omitempty"`
}

// JobConfigurationExtract is the per-extract slice of a JobConfiguration.
type JobConfigurationExtract struct {
	SourceTable       *bqtypes.TableReference `json:"sourceTable,omitempty"`
	DestinationURIs   []string                `json:"destinationUris,omitempty"`
	DestinationFormat string                  `json:"destinationFormat,omitempty"`
	Compression       string                  `json:"compression,omitempty"`
}

// Statistics mirrors the subset of `JobStatistics` the emulator
// currently fills in. All four timestamp / byte fields are decimal
// strings on the wire per
// docs/bigquery/docs/reference/rest/v2/jobs/get.md#JobStatistics --
// even `totalBytesProcessed`, because BigQuery REST never emits
// 64-bit integers as JSON numbers (clients use `string` decoders).
type Statistics struct {
	CreationTime        string                  `json:"creationTime,omitempty"`
	StartTime           string                  `json:"startTime,omitempty"`
	EndTime             string                  `json:"endTime,omitempty"`
	TotalBytesProcessed string                  `json:"totalBytesProcessed,omitempty"`
	ParentJobID         string                  `json:"parentJobId,omitempty"`
	NumChildJobs        string                  `json:"numChildJobs,omitempty"`
	SessionInfo         *bqtypes.SessionInfo    `json:"sessionInfo,omitempty"`
	Query               *bqtypes.JobStatistics2 `json:"query,omitempty"`
	Load                *LoadStatistics         `json:"load,omitempty"`
	Copy                *CopyStatistics         `json:"copy,omitempty"`
	Extract             *ExtractStatistics      `json:"extract,omitempty"`
}

// LoadStatistics mirrors upstream `JobStatistics3` (statistics.load).
type LoadStatistics struct {
	InputFiles     string `json:"inputFiles,omitempty"`
	InputFileBytes string `json:"inputFileBytes,omitempty"`
	OutputRows     string `json:"outputRows,omitempty"`
	OutputBytes    string `json:"outputBytes,omitempty"`
	BadRecords     string `json:"badRecords,omitempty"`
}

// CopyStatistics mirrors upstream `CopyJobStatistics` (statistics.copy).
type CopyStatistics struct {
	CopiedRows         string `json:"copiedRows,omitempty"`
	CopiedLogicalBytes string `json:"copiedLogicalBytes,omitempty"`
}

// ExtractStatistics mirrors upstream `JobStatistics4` (statistics.extract).
type ExtractStatistics struct {
	DestinationURIFileCounts []string `json:"destinationUriFileCounts,omitempty"`
	InputBytes               string   `json:"inputBytes,omitempty"`
}

// QueryResult is the cached result of a synchronous query, kept in
// the registry so a follow-up `jobs.getQueryResults` can replay the
// same schema and rows without re-running the SQL. Schema and Rows
// are stored in the BigQuery REST `f`/`v` shape so the handler can
// emit them verbatim.
//
// The registry holds the entire result set in memory; this matches
// the "single-page only" charter from
// `docs/ENGINE_POLICY.md`. Pagination
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
	// DdlTargetRoutine is set when a CREATE_FUNCTION /
	// CREATE_PROCEDURE DDL statement registers a routine.
	DdlTargetRoutine *bqtypes.RoutineReference
}

// Job is the gateway's view of a single BigQuery job. Today it's
// populated from the sync `jobs.query` path and the sync-query slice
// of `jobs.insert`; the per-type `*Statistics` sub-objects are
// deferred until a handler actually needs them.
//
// Result is the cached query result, populated by
// `CompleteQueryWithResult` and consumed by `jobs.getQueryResults`.
// It is excluded from the JSON encoding because the upstream Job
// resource has no rows/schema field; result data is only emitted
// through the dedicated `QueryResponse`/`GetQueryResultsResponse`
// shapes.
//
// ParentJobID is non-empty for script statement-level jobs spawned
// under a scripting parent, mirroring upstream's
// `Job.statistics.parentJobId`. `JobDelete` cascades by removing
// every entry whose ParentJobID matches the requested jobId so a
// scripting parent's children disappear in one call.
//
// CancelRequested mirrors the upstream `Job.status.cancelRequested`
// flag the `JobCancel` handler stamps on the response envelope. The
// gateway runs every job synchronously today so the flag flips to
// true at the same instant the entry's state moves to CANCELLED;
// once a long-running execution lane lands the flag's pre-flip
// observation window will widen.
//
// Configuration is the round-trip copy of the inbound
// `configuration` body so `jobs.get` / `jobs.list` echo back the
// same fields the caller posted at `jobs.insert` time. Sync
// `jobs.query` calls (which do not go through `jobs.insert`) leave
// it nil; clients reading those entries see no `configuration`
// field, matching the upstream behavior.
type Job struct {
	Kind          string               `json:"kind,omitempty"`
	ID            string               `json:"id,omitempty"`
	JobReference  bqtypes.JobReference `json:"jobReference"`
	Status        Status               `json:"status"`
	Statistics    Statistics           `json:"statistics"`
	Configuration *JobConfiguration    `json:"configuration,omitempty"`
	// ParentJobID is the registry's link to a scripting parent. It
	// is round-tripped under `Statistics.parentJobId` once the per-
	// type statistics envelope ships; today it stays an internal
	// link so `JobDelete` can cascade by parent-id without growing
	// a separate scripting index. JSON tag is `-` so the field does
	// not yet appear on the wire.
	ParentJobID string       `json:"-"`
	Result      *QueryResult `json:"-"`
}

// Registry is a process-local jobs table keyed by jobId. Reads /
// writes are concurrency-safe via a single sync.RWMutex over an
// ordered slice + map index. We track insertion order on the side
// (the `order` slice) so `ListByProject` can hand back a deterministic
// reverse-chronological page without the caller having to sort an
// arbitrary `sync.Map` walk. The monotonic counter is bumped
// atomically so even within a single nanosecond two requests still
// see distinct ids.
//
// The map only holds successful or terminally-failed jobs today;
// the emulator does not yet maintain a pending queue (see the
// package-level doc for why DONE-on-arrival is fine).
type Registry struct {
	counter atomic.Uint64
	mu      sync.RWMutex
	jobs    map[string]*Job
	// order is the insertion-ordered list of jobIds. `ListByProject`
	// iterates this in reverse to produce a newest-first page, which
	// matches what the BigQuery client libraries (and the upstream
	// `jobs.list` default sort) display. The slice grows on
	// Register / CompleteQuery and shrinks on Delete (linear scan;
	// fine for the per-process volumes the emulator handles, ~10s
	// of jobs in any test run).
	order []string
}

// NewRegistry returns a fresh, empty registry. Each gateway process
// gets one; tests can mint their own per-test for isolation without
// polluting a global.
func NewRegistry() *Registry {
	return &Registry{jobs: map[string]*Job{}}
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
	r.Register(j)
	return j
}

// Register inserts j into the registry under its JobReference.JobID.
// If a job with the same id is already present the call is a no-op
// (the existing pointer is preserved). `CompleteQueryWithResult`
// flows through here so the sync-query and async-insert paths share
// one writer. Tests that need a hand-built Job (e.g. to seed a
// non-DONE entry the cancel/delete handlers will read back) can also
// call this directly.
func (r *Registry) Register(j *Job) {
	if j == nil {
		return
	}
	id := j.JobReference.JobID
	if id == "" {
		return
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	if _, exists := r.jobs[id]; exists {
		return
	}
	r.jobs[id] = j
	r.order = append(r.order, id)
}

// Get returns the Job recorded under jobID, or (nil, false) if no
// such job is in the registry. Used by `jobs.get` /
// `jobs.getQueryResults`.
func (r *Registry) Get(jobID string) (*Job, bool) {
	r.mu.RLock()
	defer r.mu.RUnlock()
	j, ok := r.jobs[jobID]
	return j, ok
}

// ListOptions captures the documented `jobs.list` query parameters
// the handler exposes today. Empty / zero-valued fields are treated
// as "no filter". PageToken is the opaque cursor `ListByProject`
// hands back; the handler does not need to interpret it.
type ListOptions struct {
	MaxResults      int
	PageToken       string
	ParentJobID     string
	MinCreationTime int64 // millis since epoch; 0 = unbounded
	MaxCreationTime int64 // millis since epoch; 0 = unbounded
	StateFilter     []string
}

// ListByProject returns the page of jobs belonging to projectID that
// match the supplied options. Results are ordered newest-first
// (mirroring `bigquery.jobs.list`'s default sort) and pagination is
// cursor-based: the returned nextPageToken is opaque to the caller
// and feeds straight back into `ListOptions.PageToken` for the next
// page. When no more pages remain the token is empty.
//
// `MaxResults <= 0` means "the documented default cap" (50 today;
// upstream picks the same number when callers omit the field).
func (r *Registry) ListByProject(projectID string, opts ListOptions) (
	jobs []*Job, nextPageToken string,
) {
	r.mu.RLock()
	defer r.mu.RUnlock()

	maxResults := opts.MaxResults
	if maxResults <= 0 {
		maxResults = defaultListMaxResults
	}
	stateFilters := normalizeStateFilters(opts.StateFilter)
	startIdx, _ := strconv.Atoi(opts.PageToken)
	skipped := 0
	jobs = make([]*Job, 0, maxResults)
	// Walk newest-first by iterating `order` in reverse; this matches
	// `bigquery.jobs.list`'s default sort. The cursor token is the
	// count of newest-first jobs the caller has already consumed so
	// resuming a page just means continuing past them.
	for _, v := range slices.Backward(r.order) {
		j := r.jobs[v]
		if !jobMatchesProject(j, projectID, opts, stateFilters) {
			continue
		}
		if skipped < startIdx {
			skipped++
			continue
		}
		if len(jobs) >= maxResults {
			nextPageToken = strconv.Itoa(startIdx + len(jobs))
			return jobs, nextPageToken
		}
		jobs = append(jobs, j)
	}
	return jobs, ""
}

// defaultListMaxResults bounds the per-page result count when the
// caller leaves `MaxResults` zero. Upstream's documented default is
// 50; matching it avoids surprises for clients that probe the
// emulator before passing an explicit cap.
const defaultListMaxResults = 50

// jobMatchesProject is the per-entry filter `ListByProject` runs
// against the iteration. Hoisted out so the page loop stays a
// straight cursor without nested ifs (cyclop / nestif caps).
func jobMatchesProject(j *Job, projectID string, opts ListOptions, stateFilters map[string]bool) bool {
	if j.JobReference.ProjectID != projectID {
		return false
	}
	if opts.ParentJobID != "" && j.ParentJobID != opts.ParentJobID {
		return false
	}
	if len(stateFilters) != 0 && !stateFilters[j.Status.State] {
		return false
	}
	creation, _ := strconv.ParseInt(j.Statistics.CreationTime, 10, 64)
	if opts.MinCreationTime != 0 && creation < opts.MinCreationTime {
		return false
	}
	if opts.MaxCreationTime != 0 && creation > opts.MaxCreationTime {
		return false
	}
	return true
}

// normalizeStateFilters folds the caller-provided wire spellings
// (`pending` / `running` / `done`) into a set keyed by the canonical
// `Status.State` value the registry stores. Unknown spellings are
// dropped on the floor (the upstream API documents the parameter
// values explicitly and a typo should not silently broaden a query).
// Returns nil for the no-filter case so the per-entry filter knows
// to skip the state check entirely.
func normalizeStateFilters(in []string) map[string]bool {
	if len(in) == 0 {
		return nil
	}
	out := make(map[string]bool, len(in))
	for _, raw := range in {
		if canon, ok := stateFilterAliases[strings.ToLower(strings.TrimSpace(raw))]; ok {
			out[canon] = true
		}
	}
	if len(out) == 0 {
		return nil
	}
	return out
}

// Cancel flips the named job from PENDING/RUNNING to DONE +
// CancelRequested=true and reports the updated entry. Idempotent on
// terminal states (DONE jobs come back with their existing status
// untouched, only CancelRequested set). The bool is false when the
// jobId is unknown so the handler can return a 404 with a
// BigQuery-shaped envelope; the error message is BigQuery's
// canonical "Not found: Job" wording so the caller can forward it
// verbatim.
func (r *Registry) Cancel(jobID string) (*Job, bool) {
	r.mu.Lock()
	defer r.mu.Unlock()
	j, ok := r.jobs[jobID]
	if !ok {
		return nil, false
	}
	j.Status.CancelRequested = true
	if j.Status.State != JobStateDone {
		j.Status.State = JobStateDone
		if j.Statistics.EndTime == "" {
			j.Statistics.EndTime = millisString(time.Now().UTC())
		}
	}
	return j, true
}

// Delete removes jobID from the registry. When the job is a script
// parent every entry whose ParentJobID matches cascades out in the
// same call so the upstream contract -- "deleting a parent removes
// its children" -- holds without an extra round-trip. Returns false
// when the jobId is unknown.
func (r *Registry) Delete(jobID string) bool {
	r.mu.Lock()
	defer r.mu.Unlock()
	if _, ok := r.jobs[jobID]; !ok {
		return false
	}
	r.removeLocked(jobID)
	// Cascade children. Walk a snapshot of the order slice so we
	// don't iterate the underlying storage while mutating it.
	for _, id := range append([]string(nil), r.order...) {
		if child, ok := r.jobs[id]; ok && child.ParentJobID == jobID {
			r.removeLocked(id)
		}
	}
	return true
}

// removeLocked drops id from both `jobs` and `order`. Must be called
// with `mu` already held write-locked. The slice splice is a linear
// scan + copy; fine for the per-process volumes the emulator
// handles. If a future load lane pushes registry size into the
// thousands this can be replaced with a doubly-linked list, but the
// extra book-keeping is not warranted today.
func (r *Registry) removeLocked(id string) {
	delete(r.jobs, id)
	for i, entry := range r.order {
		if entry == id {
			r.order = append(r.order[:i], r.order[i+1:]...)
			return
		}
	}
}

// millisString converts t to BigQuery's wire timestamp format:
// decimal milliseconds since the Unix epoch. Used for all four
// `creationTime` / `startTime` / `endTime` / `totalBytesProcessed`-
// adjacent timestamps emitted in `Statistics`.
func millisString(t time.Time) string {
	return strconv.FormatInt(t.UnixMilli(), 10)
}

// FormatDryRunBytesProcessed renders estimated bytes as the decimal
// string BigQuery REST emits for dry-run jobs. Client libraries treat
// an empty or zero counter as missing; upstream dry-run samples assert
// a positive value, so zero engine estimates surface as "1".
func FormatDryRunBytesProcessed(estimated int64) string {
	if estimated <= 0 {
		return "1"
	}
	return strconv.FormatInt(estimated, 10)
}

// ApplyDryRunStatistics stamps the DONE dry-run terminus on a query
// job, mirroring both statistics.totalBytesProcessed and the nested
// statistics.query.totalBytesProcessed envelope QueryJob reads.
func ApplyDryRunStatistics(job *Job, estimated int64, start, end time.Time) {
	if job == nil {
		return
	}
	bytes := FormatDryRunBytesProcessed(estimated)
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.TotalBytesProcessed = bytes
	job.Statistics.Query = &bqtypes.JobStatistics2{TotalBytesProcessed: bytes}
}
