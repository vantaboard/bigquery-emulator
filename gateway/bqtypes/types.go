// Package bqtypes contains wire-compatible Go structs for the small slice
// of the BigQuery v2 REST API the emulator currently understands.
//
// We do not re-generate these from the official Discovery doc yet; this
// hand-written subset is enough to compile and exercise the route table.
// As we flesh out handlers, types here can be replaced by generated code
// (e.g. via `google.golang.org/api/bigquery/v2`'s generated structs) or
// expanded inline.
package bqtypes

// DatasetReference is a stable handle to a dataset.
type DatasetReference struct {
	ProjectID string `json:"projectId"`
	DatasetID string `json:"datasetId"`
}

// TableReference is a stable handle to a table.
type TableReference struct {
	ProjectID string `json:"projectId"`
	DatasetID string `json:"datasetId"`
	TableID   string `json:"tableId"`
}

// JobReference is a stable handle to a job.
type JobReference struct {
	ProjectID string `json:"projectId"`
	JobID     string `json:"jobId"`
	Location  string `json:"location,omitempty"`
}

// Dataset is the BigQuery Dataset resource (subset).
//
// Access is the dataset ACL — a list of role bindings. The field is
// always serialized (no `omitempty`) because the Java BigQuery client
// calls `new ArrayList<>(dataset.getAcl())` on the deserialized
// response, which NPEs when the field is null. Live BigQuery returns
// an empty array for newly-created datasets; the emulator must
// preserve that shape so AuthorizeDatasetIT-style ACL-mutation flows
// work end-to-end. See the failing-IT inventory in
// `docs/ENGINE_POLICY.md`.
//
// Labels is always serialized (no `omitempty`) for the same reason:
// the Node `getDatasetLabels` sample (and several upstream Python
// snippets) call `Object.entries(dataset.metadata.labels)` /
// `dict(dataset.labels)` on the deserialized response, which raises
// `TypeError: Cannot convert undefined or null to object` /
// `TypeError: argument of type 'NoneType' is not iterable` when the
// field is missing. Live BigQuery returns `labels: {}` for a newly
// created dataset; the resource builder defaults a nil map to `{}` to
// match. Same for Table.Labels below.
type Dataset struct {
	Kind                     string            `json:"kind,omitempty"` // bigquery#dataset
	ID                       string            `json:"id,omitempty"`
	DatasetReference         DatasetReference  `json:"datasetReference"`
	FriendlyName             string            `json:"friendlyName,omitempty"`
	Description              string            `json:"description,omitempty"`
	Location                 string            `json:"location,omitempty"`
	Etag                     string            `json:"etag,omitempty"`
	CreationTime             string            `json:"creationTime,omitempty"`
	LastModifiedTime         string            `json:"lastModifiedTime,omitempty"`
	Access                   []map[string]any  `json:"access"`
	Labels                   map[string]string `json:"labels"`
	DefaultTableExpirationMs string            `json:"defaultTableExpirationMs,omitempty"`
	// DefaultPartitionExpirationMs is inherited by new time-partitioned
	// tables in the dataset. See
	// docs/bigquery/docs/reference/rest/v2/datasets/get.md.
	DefaultPartitionExpirationMs string `json:"defaultPartitionExpirationMs,omitempty"`
	// DefaultCollation is BigQuery's per-dataset default text
	// collation (typically `und:ci` for the case-insensitive lane
	// the upstream node sample exercises). The emulator does not
	// honor it at query time today, but the value still has to
	// round-trip through GET/PATCH so client libraries observe the
	// shape they expect. See
	// docs/bigquery/docs/reference/rest/v2/datasets/get.md.
	DefaultCollation string `json:"defaultCollation,omitempty"`
}

// Table is the BigQuery Table resource (subset).
//
// Labels is always serialized (no `omitempty`); see the matching note
// on Dataset.Labels. tableResource defaults a nil map to `{}` so the
// upstream `getTableLabels` sample's `Object.entries(table.metadata.labels)`
// returns an empty iterator instead of erroring.
type Table struct {
	Kind             string            `json:"kind,omitempty"` // bigquery#table
	ID               string            `json:"id,omitempty"`
	TableReference   TableReference    `json:"tableReference"`
	FriendlyName     string            `json:"friendlyName,omitempty"`
	Description      string            `json:"description,omitempty"`
	Schema           *TableSchema      `json:"schema,omitempty"`
	Type             string            `json:"type,omitempty"` // TABLE | VIEW | EXTERNAL
	NumRows          string            `json:"numRows,omitempty"`
	NumBytes         string            `json:"numBytes,omitempty"`
	CreationTime     string            `json:"creationTime,omitempty"`
	LastModifiedTime string            `json:"lastModifiedTime,omitempty"`
	Etag             string            `json:"etag,omitempty"`
	Labels           map[string]string `json:"labels"`
	// ExpirationTime is the wall-clock time at which the table
	// expires, encoded as a decimal string of milliseconds since
	// epoch -- BigQuery REST always serializes int64 timestamps
	// as strings to dodge JavaScript's 53-bit integer ceiling.
	// `omitempty` is intentional: live BigQuery omits the field
	// when the table has no expiration.
	ExpirationTime string `json:"expirationTime,omitempty"`
	// RangePartitioning is the integer-range partitioning spec
	// (`{field, range:{start,end,interval}}`) the upstream node
	// `createTableRangePartitioned` sample sets and the matching
	// test asserts on the GET response.
	RangePartitioning *RangePartitioning `json:"rangePartitioning,omitempty"`
	// TimePartitioning is the (TIME / DAY / HOUR / MONTH / YEAR)
	// time-based partitioning spec. Not exercised by every test
	// but parallel to RangePartitioning so the roundtrip helper
	// can carry it without dropping the field on the floor.
	TimePartitioning *TimePartitioning `json:"timePartitioning,omitempty"`
	// Clustering is the per-table clustering spec the upstream
	// node `createTableClustered` sample sets via
	// `{ fields: ['city', 'zipcode'] }`.
	Clustering *Clustering `json:"clustering,omitempty"`
	// DefaultCollation is the table-level default text collation
	// (typically `und:ci`). Mirrors Dataset.DefaultCollation;
	// see that field's comment for the round-trip rationale.
	DefaultCollation string `json:"defaultCollation,omitempty"`
	// RequirePartitionFilter mirrors the table-level partition-filter
	// requirement BigQuery REST exposes. Pointer semantics let PATCH
	// bodies set `false` explicitly without conflating unset and false.
	RequirePartitionFilter *bool `json:"requirePartitionFilter,omitempty"`
	// View holds the view definition when Type is VIEW.
	View *ViewDefinition `json:"view,omitempty"`
	// MaterializedView holds the MV definition when Type is
	// MATERIALIZED_VIEW. The query is analyzed at insert time to
	// infer the catalog schema when the client omits an explicit
	// TableSchema (see QueryMaterializedViewIT).
	MaterializedView *MaterializedViewDefinition `json:"materializedView,omitempty"`
}

// ViewDefinition is the BigQuery REST view sub-object. See
// docs/bigquery/docs/reference/rest/v2/tables#ViewDefinition.
type ViewDefinition struct {
	Query string `json:"query,omitempty"`
}

// MaterializedViewDefinition is the BigQuery REST materializedView
// sub-object. See docs/bigquery/docs/reference/rest/v2/tables#MaterializedViewDefinition.
type MaterializedViewDefinition struct {
	Query string `json:"query,omitempty"`
}

// RangePartitioning describes BigQuery integer-range partitioning. The
// only currently-supported `Range.Interval` granularity is integer
// buckets (`start`, `end`, `interval`); the field is just round-tripped
// for now.
type RangePartitioning struct {
	Field string         `json:"field,omitempty"`
	Range *RangePartSpec `json:"range,omitempty"`
}

// RangePartSpec is the `range` sub-object of RangePartitioning. All
// three integer fields are wire-serialized as decimal strings to mirror
// BigQuery REST. See docs/bigquery/docs/reference/rest/v2/tables/get.md.
type RangePartSpec struct {
	Start    string `json:"start,omitempty"`
	End      string `json:"end,omitempty"`
	Interval string `json:"interval,omitempty"`
}

// TimePartitioning describes time-based partitioning. Carried for
// roundtrip only; the emulator does not enforce partition expiration.
type TimePartitioning struct {
	Type         string `json:"type,omitempty"`
	Field        string `json:"field,omitempty"`
	ExpirationMs string `json:"expirationMs,omitempty"`
}

// Clustering is the per-table clustering spec.
type Clustering struct {
	Fields []string `json:"fields,omitempty"`
}

// TableSchema is the BigQuery TableSchema resource.
type TableSchema struct {
	Fields []TableFieldSchema `json:"fields,omitempty"`
}

// TableFieldSchema is one column in a TableSchema.
type TableFieldSchema struct {
	Name        string             `json:"name"`
	Type        string             `json:"type"`           // STRING, INT64, FLOAT64, BOOL, TIMESTAMP, ...
	Mode        string             `json:"mode,omitempty"` // NULLABLE, REQUIRED, REPEATED
	Description string             `json:"description,omitempty"`
	Fields      []TableFieldSchema `json:"fields,omitempty"` // for STRUCT/RECORD
}

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
	Query *JobStatistics2 `json:"query,omitempty"`
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

// Row is one row of query results, in BigQuery's `f`/`v` shape.
type Row struct {
	F []Cell `json:"f"`
}

// Cell is one column value within a Row. Value is either a string, a
// nested Row, or a list (for REPEATED) -- BigQuery encodes everything as
// strings/objects/arrays at the wire level.
type Cell struct {
	V any `json:"v"`
}

// ErrorProto is BigQuery's per-error detail object.
type ErrorProto struct {
	Reason    string `json:"reason,omitempty"`
	Location  string `json:"location,omitempty"`
	DebugInfo string `json:"debugInfo,omitempty"`
	Message   string `json:"message,omitempty"`
}

// TableDataInsertAllRequest is the body of
// POST .../tables/{tableId}/insertAll. See
// docs/bigquery/docs/reference/rest/v2/tabledata/insertAll.md.
type TableDataInsertAllRequest struct {
	Kind                string                         `json:"kind,omitempty"`
	SkipInvalidRows     bool                           `json:"skipInvalidRows,omitempty"`
	IgnoreUnknownValues bool                           `json:"ignoreUnknownValues,omitempty"`
	TemplateSuffix      string                         `json:"templateSuffix,omitempty"`
	Rows                []TableDataInsertAllRequestRow `json:"rows,omitempty"`
	TraceID             string                         `json:"traceId,omitempty"`
}

// TableDataInsertAllRequestRow is one entry in
// `TableDataInsertAllRequest.rows`. `Json` is BigQuery's name for the
// per-row payload; we keep the JSON tag as-is so unmarshaling works
// against the official client libraries.
type TableDataInsertAllRequestRow struct {
	InsertID string         `json:"insertId,omitempty"`
	JSON     map[string]any `json:"json"`
}

// TableDataInsertAllResponse is the response of insertAll. See
// docs/bigquery/docs/reference/rest/v2/tabledata/insertAll.md.
type TableDataInsertAllResponse struct {
	Kind         string                         `json:"kind,omitempty"`
	InsertErrors []TableDataInsertAllErrorEntry `json:"insertErrors,omitempty"`
}

// TableDataInsertAllErrorEntry mirrors insertErrors[*] in the
// insertAll response: per-row error attribution.
type TableDataInsertAllErrorEntry struct {
	Index  uint64       `json:"index"`
	Errors []ErrorProto `json:"errors,omitempty"`
}

// RoutineReference is a stable handle to a routine (UDF / TVF / procedure).
type RoutineReference struct {
	ProjectID string `json:"projectId"`
	DatasetID string `json:"datasetId"`
	RoutineID string `json:"routineId"`
}

// StandardSqlDataType mirrors the BigQuery REST StandardSqlDataType
// resource. See docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlDataType struct {
	TypeKind         string                 `json:"typeKind"`
	ArrayElementType *StandardSqlDataType   `json:"arrayElementType,omitempty"`
	StructType       *StandardSqlStructType `json:"structType,omitempty"`
	RangeElementType *StandardSqlDataType   `json:"rangeElementType,omitempty"`
}

// StandardSqlStructType is the struct sub-object of StandardSqlDataType.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlStructType struct {
	Fields []StandardSqlField `json:"fields,omitempty"`
}

// StandardSqlField is one field of a STRUCT type.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlField struct {
	Name string              `json:"name"`
	Type StandardSqlDataType `json:"type"`
}

// StandardSqlTableType is the return-table type for table-valued functions.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlTableType struct {
	Columns []StandardSqlField `json:"columns,omitempty"`
}

// RoutineArgument is an input/output argument of a routine.
type RoutineArgument struct {
	Name         string               `json:"name,omitempty"`
	ArgumentKind string               `json:"argumentKind,omitempty"`
	Mode         string               `json:"mode,omitempty"`
	DataType     *StandardSqlDataType `json:"dataType,omitempty"`
}

// Routine is the BigQuery Routine resource (subset).
// See docs/bigquery/docs/reference/rest/v2/routines.md.
type Routine struct {
	Etag             string                `json:"etag,omitempty"`
	RoutineReference RoutineReference      `json:"routineReference"`
	RoutineType      string                `json:"routineType,omitempty"`
	CreationTime     string                `json:"creationTime,omitempty"`
	LastModifiedTime string                `json:"lastModifiedTime,omitempty"`
	Language         string                `json:"language,omitempty"`
	Arguments        []RoutineArgument     `json:"arguments,omitempty"`
	ReturnType       *StandardSqlDataType  `json:"returnType,omitempty"`
	ReturnTableType  *StandardSqlTableType `json:"returnTableType,omitempty"`
	DefinitionBody   string                `json:"definitionBody,omitempty"`
	Description      string                `json:"description,omitempty"`
	StrictMode       *bool                 `json:"strictMode,omitempty"`
}

// TableDataList is the response of GET .../tables/{tableId}/data.
// See docs/bigquery/docs/reference/rest/v2/tabledata/list.md.
type TableDataList struct {
	Kind      string `json:"kind,omitempty"` // bigquery#tableDataList
	Etag      string `json:"etag,omitempty"`
	TotalRows string `json:"totalRows,omitempty"`
	PageToken string `json:"pageToken,omitempty"`
	Rows      []Row  `json:"rows,omitempty"`
}
