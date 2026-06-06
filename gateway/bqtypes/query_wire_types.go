package bqtypes

// QueryRequest is the body of POST /bigquery/v2/projects/{projectId}/queries.
//
// Mirrors the QueryRequest definition under
// docs/bigquery/docs/reference/rest/v2/jobs/query.md. Fields the
// emulator does not honor today are still parsed so client libraries
// don't get unmarshal errors.
type QueryRequest struct {
	Kind                               string            `json:"kind,omitempty"` // bigquery#queryRequest
	Query                              string            `json:"query"`
	MaxResults                         uint32            `json:"maxResults,omitempty"`
	DefaultDataset                     *DatasetReference `json:"defaultDataset,omitempty"`
	TimeoutMs                          uint32            `json:"timeoutMs,omitempty"`
	DestinationEncryptionConfiguration map[string]any    `json:"destinationEncryptionConfiguration,omitempty"`
	DryRun                             bool              `json:"dryRun,omitempty"`
	// PreserveNulls is deprecated upstream but still parsed.
	PreserveNulls      *bool                `json:"preserveNulls,omitempty"`
	UseQueryCache      *bool                `json:"useQueryCache,omitempty"`
	UseLegacySQL       *bool                `json:"useLegacySql,omitempty"`
	ParameterMode      string               `json:"parameterMode,omitempty"`
	Parameters         []QueryParameter     `json:"queryParameters,omitempty"`
	Location           string               `json:"location,omitempty"`
	FormatOptions      map[string]any       `json:"formatOptions,omitempty"`
	ConnProperties     []ConnectionProperty `json:"connectionProperties,omitempty"`
	Labels             map[string]string    `json:"labels,omitempty"`
	MaximumBytesBilled string               `json:"maximumBytesBilled,omitempty"`
	RequestID          string               `json:"requestId,omitempty"`
	CreateSession      bool                 `json:"createSession,omitempty"`
	JobCreationMode    string               `json:"jobCreationMode,omitempty"`
	JobTimeoutMs       string               `json:"jobTimeoutMs,omitempty"`
	Reservation        string               `json:"reservation,omitempty"`
	// TableDefinitions maps ephemeral external table ids to their
	// ExternalDataConfiguration for this query (temporary external
	// tables). Mirrors JobConfigurationQuery.tableDefinitions.
	TableDefinitions map[string]ExternalDataConfiguration `json:"tableDefinitions,omitempty"`
	// DestinationTable mirrors JobConfigurationQuery.destinationTable for
	// synchronous jobs.query calls that materialize results to a table.
	DestinationTable    *TableReference `json:"destinationTable,omitempty"`
	WriteDisposition    string          `json:"writeDisposition,omitempty"`
	SchemaUpdateOptions []string        `json:"schemaUpdateOptions,omitempty"`
}

// QueryResponse is the body of POST /bigquery/v2/projects/{projectId}/queries.
//
// Mirrors the QueryResponse definition under
// docs/bigquery/docs/reference/rest/v2/jobs/query.md.
type QueryResponse struct {
	Kind                string         `json:"kind,omitempty"` // bigquery#queryResponse
	Schema              *TableSchema   `json:"schema,omitempty"`
	JobReference        *JobReference  `json:"jobReference,omitempty"`
	JobCreationReason   map[string]any `json:"jobCreationReason,omitempty"`
	QueryID             string         `json:"queryId,omitempty"`
	Location            string         `json:"location,omitempty"`
	TotalRows           string         `json:"totalRows,omitempty"`
	PageToken           string         `json:"pageToken,omitempty"`
	Rows                []Row          `json:"rows,omitempty"`
	TotalBytesProcessed string         `json:"totalBytesProcessed,omitempty"`
	JobComplete         bool           `json:"jobComplete"`
	Errors              []ErrorProto   `json:"errors,omitempty"`
	CacheHit            bool           `json:"cacheHit,omitempty"`
	NumDmlAffectedRows  string         `json:"numDmlAffectedRows,omitempty"`
	SessionInfo         *SessionInfo   `json:"sessionInfo,omitempty"`
	DmlStats            *DmlStats      `json:"dmlStats,omitempty"`
	TotalBytesBilled    string         `json:"totalBytesBilled,omitempty"`
	TotalSlotMs         string         `json:"totalSlotMs,omitempty"`
	CreationTime        string         `json:"creationTime,omitempty"`
	StartTime           string         `json:"startTime,omitempty"`
	EndTime             string         `json:"endTime,omitempty"`
	Statistics          *JobStatistics `json:"statistics,omitempty"`
}

// JobStatistics is the outer BigQuery REST `Job.statistics`
// envelope. The emulator only populates the per-query subset today;
// the load / extract / copy variants exist on the wire but the
// emulator surfaces them empty until the matching handlers ship.
// Mirrors docs/bigquery/docs/reference/rest/v2/JobStatistics.md.
type JobStatistics struct {
	SessionInfo *SessionInfo    `json:"sessionInfo,omitempty"`
	Query       *JobStatistics2 `json:"query,omitempty"`
}

// JobStatistics2 is the per-query statistics block exposed under
// `Job.statistics.query`. Today the emulator surfaces only
// `statementType` (see `docs/ENGINE_POLICY.md`
// item 5) and the loopback-only `emulatorRoute` debug field (see
// `docs/ENGINE_POLICY.md`); the other
// fields land alongside the long-running-jobs follow-up. Mirrors
// docs/bigquery/docs/reference/rest/v2/JobStatistics2.md.
type JobStatistics2 struct {
	// StatementType is the BigQuery REST canonical statement-type
	// string (`SELECT` / `INSERT` / `CREATE_TABLE` / `DROP_TABLE` /
	// ...). The frontend's `StatementTypeFor` C++ helper is the
	// source of truth for which `Resolved*Stmt` maps to which
	// string; statements with no canonical value (e.g. shapes the
	// REST surface does not enumerate) leave the field empty so
	// the encoder omits the JSON property entirely.
	StatementType string `json:"statementType,omitempty"`

	// EmulatorRoute is the canonical lowercase-snake spelling of
	// the `Disposition` the C++ coordinator's `RouteClassifier`
	// chose for the query (`duckdb_native`, `duckdb_rewrite`,
	// `duckdb_udf`, `semantic_executor`, `control_op`,
	// `local_stub`, `unsupported`). It is an emulator-internal
	// debug field NOT present on the public BigQuery REST surface;
	// `gateway/middleware/loopback.go`'s `WithLoopbackTag`
	// middleware tags loopback callers, and only the handlers
	// running for those callers surface the field. The
	// conformance harness in
	// `conformance/cmd/runner` reads it back to assert per-query
	// routing decisions (`expected.route`); BigQuery client
	// libraries running against a non-loopback emulator see the
	// field omitted entirely.
	EmulatorRoute string `json:"emulatorRoute,omitempty"`

	// DdlTargetRoutine is populated on CREATE_FUNCTION /
	// CREATE_PROCEDURE DDL statements. Mirrors upstream
	// JobStatistics2.ddlTargetRoutine.
	DdlTargetRoutine *RoutineReference `json:"ddlTargetRoutine,omitempty"`

	// TotalBytesProcessed is the per-query bytes estimate surfaced
	// under statistics.query (QueryJob reads this sub-object).
	TotalBytesProcessed string `json:"totalBytesProcessed,omitempty"`
}

// SessionInfo tracks the session a query is running under, when sessions
// are in use. Mirrors docs/bigquery/docs/reference/rest/v2/SessionInfo.md.
type SessionInfo struct {
	SessionID string `json:"sessionId,omitempty"`
}

// DmlStats is the per-DML-statement statistics envelope. Mirrors
// docs/bigquery/docs/reference/rest/v2/DmlStats.md.
type DmlStats struct {
	InsertedRowCount string `json:"insertedRowCount,omitempty"`
	UpdatedRowCount  string `json:"updatedRowCount,omitempty"`
	DeletedRowCount  string `json:"deletedRowCount,omitempty"`
}

// QueryParameter is a positional or named query parameter.
type QueryParameter struct {
	Name           string               `json:"name,omitempty"`
	ParameterType  *QueryParameterType  `json:"parameterType,omitempty"`
	ParameterValue *QueryParameterValue `json:"parameterValue,omitempty"`
}

// QueryParameterType describes the BigQuery type of a query parameter.
type QueryParameterType struct {
	Type        string                     `json:"type"`
	ArrayType   *QueryParameterType        `json:"arrayType,omitempty"`
	StructTypes []QueryParameterStructType `json:"structTypes,omitempty"`
}

// QueryParameterStructType is one field of a STRUCT parameter type.
type QueryParameterStructType struct {
	Name        string             `json:"name,omitempty"`
	Type        QueryParameterType `json:"type"`
	Description string             `json:"description,omitempty"`
}

// QueryParameterValue is the value of a query parameter.
type QueryParameterValue struct {
	Value        string                         `json:"value,omitempty"`
	ArrayValues  []QueryParameterValue          `json:"arrayValues,omitempty"`
	StructValues map[string]QueryParameterValue `json:"structValues,omitempty"`
}

// ConnectionProperty is a session/connection-level setting.
type ConnectionProperty struct {
	Key   string `json:"key"`
	Value string `json:"value"`
}
