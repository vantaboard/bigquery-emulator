package bqtypes

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

// TableDataList is the response of GET .../tables/{tableId}/data.
// See docs/bigquery/docs/reference/rest/v2/tabledata/list.md.
type TableDataList struct {
	Kind      string `json:"kind,omitempty"` // bigquery#tableDataList
	Etag      string `json:"etag,omitempty"`
	TotalRows string `json:"totalRows,omitempty"`
	PageToken string `json:"pageToken,omitempty"`
	Rows      []Row  `json:"rows,omitempty"`
}
