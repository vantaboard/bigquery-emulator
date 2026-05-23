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
type Dataset struct {
	Kind             string           `json:"kind,omitempty"` // bigquery#dataset
	ID               string           `json:"id,omitempty"`
	DatasetReference DatasetReference `json:"datasetReference"`
	FriendlyName     string           `json:"friendlyName,omitempty"`
	Description      string           `json:"description,omitempty"`
	Location         string           `json:"location,omitempty"`
	Etag             string           `json:"etag,omitempty"`
	CreationTime     string           `json:"creationTime,omitempty"`
	LastModifiedTime string           `json:"lastModifiedTime,omitempty"`
}

// Table is the BigQuery Table resource (subset).
type Table struct {
	Kind             string         `json:"kind,omitempty"` // bigquery#table
	ID               string         `json:"id,omitempty"`
	TableReference   TableReference `json:"tableReference"`
	FriendlyName     string         `json:"friendlyName,omitempty"`
	Description      string         `json:"description,omitempty"`
	Schema           *TableSchema   `json:"schema,omitempty"`
	Type             string         `json:"type,omitempty"` // TABLE | VIEW | EXTERNAL
	NumRows          string         `json:"numRows,omitempty"`
	NumBytes         string         `json:"numBytes,omitempty"`
	CreationTime     string         `json:"creationTime,omitempty"`
	LastModifiedTime string         `json:"lastModifiedTime,omitempty"`
	Etag             string         `json:"etag,omitempty"`
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
type QueryRequest struct {
	Kind            string                 `json:"kind,omitempty"` // bigquery#queryRequest
	Query           string                 `json:"query"`
	MaxResults      uint32                 `json:"maxResults,omitempty"`
	DefaultDataset  *DatasetReference      `json:"defaultDataset,omitempty"`
	TimeoutMs       uint32                 `json:"timeoutMs,omitempty"`
	DryRun          bool                   `json:"dryRun,omitempty"`
	UseLegacySQL    *bool                  `json:"useLegacySql,omitempty"`
	Location        string                 `json:"location,omitempty"`
	Parameters      []QueryParameter       `json:"queryParameters,omitempty"`
	Labels          map[string]string      `json:"labels,omitempty"`
	RequestID       string                 `json:"requestId,omitempty"`
	JobCreationMode string                 `json:"jobCreationMode,omitempty"`
	ConnProperties  []ConnectionProperty   `json:"connectionProperties,omitempty"`
	FormatOptions   map[string]interface{} `json:"formatOptions,omitempty"`
}

// QueryResponse is the body of POST /bigquery/v2/projects/{projectId}/queries.
type QueryResponse struct {
	Kind                string        `json:"kind,omitempty"` // bigquery#queryResponse
	Schema              *TableSchema  `json:"schema,omitempty"`
	JobReference        *JobReference `json:"jobReference,omitempty"`
	TotalRows           string        `json:"totalRows,omitempty"`
	PageToken           string        `json:"pageToken,omitempty"`
	Rows                []Row         `json:"rows,omitempty"`
	TotalBytesProcessed string        `json:"totalBytesProcessed,omitempty"`
	JobComplete         bool          `json:"jobComplete"`
	Errors              []ErrorProto  `json:"errors,omitempty"`
	CacheHit            bool          `json:"cacheHit,omitempty"`
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
	V interface{} `json:"v"`
}

// ErrorProto is BigQuery's per-error detail object.
type ErrorProto struct {
	Reason    string `json:"reason,omitempty"`
	Location  string `json:"location,omitempty"`
	DebugInfo string `json:"debugInfo,omitempty"`
	Message   string `json:"message,omitempty"`
}
