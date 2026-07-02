// Package bqtypes contains wire-compatible Go structs for the small slice
// of the BigQuery v2 REST API the emulator currently understands.
//
// We do not re-generate these from the official Discovery doc yet; this
// hand-written subset is enough to compile and exercise the route table.
// As we flesh out handlers, types here can be replaced by generated code
// (e.g. via `google.golang.org/api/bigquery/v2`'s generated structs) or
// expanded inline.
package bqtypes

import (
	"encoding/json"
	"fmt"
	"strconv"
)

// labelsWireState is populated by Dataset/Table UnmarshalJSON when the body
// carries an explicit labels field (including label-delete null values).
type labelsWireState struct {
	present bool
	delete  []string
}

type collationWireState struct {
	present bool
}

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
	Kind                     string           `json:"kind,omitempty"` // bigquery#dataset
	ID                       string           `json:"id,omitempty"`
	DatasetReference         DatasetReference `json:"datasetReference"`
	FriendlyName             string           `json:"friendlyName,omitempty"`
	Description              string           `json:"description,omitempty"`
	Location                 string           `json:"location,omitempty"`
	Etag                     string           `json:"etag,omitempty"`
	CreationTime             string           `json:"creationTime,omitempty"`
	LastModifiedTime         string           `json:"lastModifiedTime,omitempty"`
	Access                   []map[string]any `json:"access"`
	Labels                   ResourceLabels   `json:"labels"`
	DefaultTableExpirationMs string           `json:"defaultTableExpirationMs,omitempty"`
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
	// DefaultRoundingMode is inherited by new NUMERIC/BIGNUMERIC columns in
	// tables created in this dataset. Round-trips via the gateway overlay.
	DefaultRoundingMode string `json:"defaultRoundingMode,omitempty"`
	// MaxTimeTravelHours is the dataset time-travel window (48–168 hours).
	MaxTimeTravelHours string `json:"maxTimeTravelHours,omitempty"`
	// IsCaseInsensitive marks dataset/table name lookups as case-insensitive.
	IsCaseInsensitive *bool `json:"isCaseInsensitive,omitempty"`
	// ResourceTags are GCP resource manager tags attached to the dataset.
	ResourceTags map[string]string `json:"resourceTags,omitempty"`
	// Replicas echoes cross-region replica references supplied on write;
	// the emulator does not model active replication.
	Replicas []TableReference `json:"replicas,omitempty"`
	// ExternalDatasetReference marks a Spanner / Cloud SQL external dataset.
	ExternalDatasetReference *ExternalDatasetReference `json:"externalDatasetReference,omitempty"`

	labelsWire            labelsWireState    `json:"-"`
	defaultCollationWire  collationWireState `json:"-"`
	DefaultCollationSet   bool               `json:"-"`
	omitEmptyLabelsOnWire bool               `json:"-"`
}

// LabelsPatchPresent reports whether a decoded request body explicitly set labels.
func (d Dataset) LabelsPatchPresent() bool {
	return d.labelsWire.present
}

// LabelsToDelete returns label keys cleared via JSON null in the request body.
func (d Dataset) LabelsToDelete() []string {
	return d.labelsWire.delete
}

// DefaultCollationPresent reports whether the request body explicitly set
// defaultCollation (including empty string to clear).
func (d Dataset) DefaultCollationPresent() bool {
	return d.defaultCollationWire.present
}

// SetOmitEmptyLabelsOnWire omits the labels JSON field on PATCH responses
// when empty so Node deleteLabel* samples log `undefined` for apiResponse.labels.
func (d *Dataset) SetOmitEmptyLabelsOnWire(v bool) {
	d.omitEmptyLabelsOnWire = v
}

// UnmarshalJSON accepts labels values of JSON null (label delete) and the
// usual string map entries client libraries send on datasets.patch.
func (d *Dataset) UnmarshalJSON(data []byte) error {
	type alias Dataset
	var raw struct {
		alias
		Labels           json.RawMessage `json:"labels,omitempty"`
		DefaultCollation json.RawMessage `json:"defaultCollation,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*d = Dataset(raw.alias)
	patch, err := parseLabelsJSON(raw.Labels)
	if err != nil {
		return err
	}
	if patch.present {
		d.Labels = ResourceLabels(patch.values)
		d.labelsWire = labelsWireState{present: true, delete: patch.delete}
	}
	if raw.DefaultCollation != nil {
		d.defaultCollationWire.present = true
		d.DefaultCollationSet = true
		if err := json.Unmarshal(raw.DefaultCollation, &d.DefaultCollation); err != nil {
			return fmt.Errorf("defaultCollation: %w", err)
		}
	}
	return nil
}

// MarshalJSON emits labels:{} by default; omits labels when empty after a
// label-delete PATCH so Node clients surface apiResponse.labels as undefined.
func (d Dataset) MarshalJSON() ([]byte, error) {
	type alias Dataset
	var raw []byte
	var err error
	if d.omitEmptyLabelsOnWire && len(d.Labels) == 0 {
		raw, err = marshalWithoutJSONField(alias(d), "labels")
	} else {
		raw, err = json.Marshal(alias(d))
	}
	if err != nil || !d.DefaultCollationSet {
		return raw, err
	}
	return injectJSONStringField(raw, "defaultCollation", d.DefaultCollation)
}

// Table is the BigQuery Table resource (subset).
//
// Labels is always serialized (no `omitempty`); see the matching note
// on Dataset.Labels. tableResource defaults a nil map to `{}` so the
// upstream `getTableLabels` sample's `Object.entries(table.metadata.labels)`
// returns an empty iterator instead of erroring.
type Table struct {
	Kind           string         `json:"kind,omitempty"` // bigquery#table
	ID             string         `json:"id,omitempty"`
	TableReference TableReference `json:"tableReference"`
	FriendlyName   string         `json:"friendlyName,omitempty"`
	Description    string         `json:"description,omitempty"`
	Schema         *TableSchema   `json:"schema,omitempty"`
	Type           string         `json:"type,omitempty"` // TABLE | VIEW | EXTERNAL
	NumRows        string         `json:"numRows,omitempty"`
	NumBytes       string         `json:"numBytes,omitempty"`
	// Output-only storage breakdown fields. The gateway stubs these to "0"
	// until the engine exposes byte accounting RPCs.
	NumLongTermBytes           string         `json:"numLongTermBytes,omitempty"`
	NumActiveLogicalBytes      string         `json:"numActiveLogicalBytes,omitempty"`
	NumTotalLogicalBytes       string         `json:"numTotalLogicalBytes,omitempty"`
	NumCurrentPhysicalBytes    string         `json:"numCurrentPhysicalBytes,omitempty"`
	NumPhysicalBytes           string         `json:"numPhysicalBytes,omitempty"`
	NumActivePhysicalBytes     string         `json:"numActivePhysicalBytes,omitempty"`
	NumLongTermPhysicalBytes   string         `json:"numLongTermPhysicalBytes,omitempty"`
	NumTimeTravelPhysicalBytes string         `json:"numTimeTravelPhysicalBytes,omitempty"`
	CreationTime               string         `json:"creationTime,omitempty"`
	LastModifiedTime           string         `json:"lastModifiedTime,omitempty"`
	Etag                       string         `json:"etag,omitempty"`
	Labels                     ResourceLabels `json:"labels"`
	// ExpirationTime is the wall-clock time at which the table
	// expires, encoded as a decimal string of milliseconds since
	// epoch -- BigQuery REST always serializes int64 timestamps
	// as strings to dodge JavaScript's 53-bit integer ceiling.
	// `omitempty` is intentional: live BigQuery omits the field
	// when the table has no expiration.
	ExpirationTime MillisTimestamp `json:"expirationTime,omitempty"`
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
	// Location is the BigQuery region for the table (inherited from
	// its dataset on live BigQuery). Client libraries such as
	// google-cloud-bigquery and BigFrames read this on tables.get.
	Location string `json:"location,omitempty"`
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
	// ExternalDataConfiguration describes a table backed by data
	// outside the emulator catalog (GCS CSV/JSON/Parquet, ...).
	// Persisted in the gateway MetadataStore and materialized into
	// the engine catalog at insert/query time for supported formats.
	ExternalDataConfiguration *ExternalDataConfiguration `json:"externalDataConfiguration,omitempty"`
	// EncryptionConfiguration stores the opaque CMEK kmsKeyName the
	// client supplied on create/load/update. Not enforced by the emulator.
	EncryptionConfiguration *EncryptionConfiguration `json:"encryptionConfiguration,omitempty"`
	// DefaultRoundingMode is inherited by new NUMERIC/BIGNUMERIC columns.
	DefaultRoundingMode string `json:"defaultRoundingMode,omitempty"`
	// CaseInsensitive marks table name lookups as case-insensitive within
	// a case-insensitive dataset.
	CaseInsensitive *bool `json:"caseInsensitive,omitempty"`
	// ResourceTags are GCP resource manager tags attached to the table.
	ResourceTags map[string]string `json:"resourceTags,omitempty"`
	// TableConstraints carries primary/foreign key metadata (not enforced).
	TableConstraints *TableConstraints `json:"tableConstraints,omitempty"`
	// Replicas echoes cross-region replica references on write.
	Replicas []TableReference `json:"replicas,omitempty"`
	// BiglakeConfiguration marks a BigLake-managed table (unsupported).
	BiglakeConfiguration *BiglakeConfiguration `json:"biglakeConfiguration,omitempty"`
	// ObjectTableOptions marks an object table (unsupported).
	ObjectTableOptions *ObjectTableOptions `json:"objectTableOptions,omitempty"`

	labelsWire            labelsWireState    `json:"-"`
	defaultCollationWire  collationWireState `json:"-"`
	DefaultCollationSet   bool               `json:"-"`
	omitEmptyLabelsOnWire bool               `json:"-"`
}

// LabelsPatchPresent reports whether a decoded request body explicitly set labels.
func (t Table) LabelsPatchPresent() bool {
	return t.labelsWire.present
}

// LabelsToDelete returns label keys cleared via JSON null in the request body.
func (t Table) LabelsToDelete() []string {
	return t.labelsWire.delete
}

// DefaultCollationPresent reports whether the request body explicitly set
// defaultCollation (including empty string to clear).
func (t Table) DefaultCollationPresent() bool {
	return t.defaultCollationWire.present
}

// SetOmitEmptyLabelsOnWire omits the labels JSON field on PATCH responses
// when empty so Node deleteLabel* samples log `undefined` for apiResponse.labels.
func (t *Table) SetOmitEmptyLabelsOnWire(v bool) {
	t.omitEmptyLabelsOnWire = v
}

// UnmarshalJSON accepts expirationTime as a decimal string or JSON number and
// labels values of JSON null (label delete).
func (t *Table) UnmarshalJSON(data []byte) error {
	type alias Table
	var raw struct {
		alias
		ExpirationTime   json.RawMessage `json:"expirationTime,omitempty"`
		Labels           json.RawMessage `json:"labels,omitempty"`
		DefaultCollation json.RawMessage `json:"defaultCollation,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*t = Table(raw.alias)
	if raw.ExpirationTime != nil {
		var ts MillisTimestamp
		if err := json.Unmarshal(raw.ExpirationTime, &ts); err != nil {
			return fmt.Errorf("expirationTime: %w", err)
		}
		t.ExpirationTime = ts
	}
	patch, err := parseLabelsJSON(raw.Labels)
	if err != nil {
		return err
	}
	if patch.present {
		t.Labels = ResourceLabels(patch.values)
		t.labelsWire = labelsWireState{present: true, delete: patch.delete}
	}
	if raw.DefaultCollation != nil {
		t.defaultCollationWire.present = true
		t.DefaultCollationSet = true
		if err := json.Unmarshal(raw.DefaultCollation, &t.DefaultCollation); err != nil {
			return fmt.Errorf("defaultCollation: %w", err)
		}
	}
	return nil
}

// MarshalJSON emits labels:{} by default; omits labels when empty after a
// label-delete PATCH so Node clients surface apiResponse.labels as undefined.
func (t Table) MarshalJSON() ([]byte, error) {
	type alias Table
	var raw []byte
	var err error
	if t.omitEmptyLabelsOnWire && len(t.Labels) == 0 {
		raw, err = marshalWithoutJSONField(alias(t), "labels")
	} else {
		raw, err = json.Marshal(alias(t))
	}
	if err != nil || !t.DefaultCollationSet {
		return raw, err
	}
	return injectJSONStringField(raw, "defaultCollation", t.DefaultCollation)
}

// ExternalDataConfiguration mirrors the BigQuery REST external data
// source object. See docs/bigquery/docs/reference/rest/v2/tables.md.
type ExternalDataConfiguration struct {
	SourceURIs              []string                 `json:"sourceUris,omitempty"`
	SourceFormat            string                   `json:"sourceFormat,omitempty"`
	Autodetect              bool                     `json:"autodetect,omitempty"`
	Schema                  *TableSchema             `json:"schema,omitempty"`
	CsvOptions              *CsvOptions              `json:"csvOptions,omitempty"`
	GoogleSheetsOptions     *GoogleSheetsOptions     `json:"googleSheetsOptions,omitempty"`
	HivePartitioningOptions *HivePartitioningOptions `json:"hivePartitioningOptions,omitempty"`
	IgnoreUnknownValues     bool                     `json:"ignoreUnknownValues,omitempty"`
	MaxBadRecords           int                      `json:"maxBadRecords,omitempty"`
	Compression             string                   `json:"compression,omitempty"`
}

// HivePartitioningOptions mirrors the BigQuery REST hivePartitioningOptions
// object. See docs/bigquery/docs/reference/rest/v2/tables.md.
type HivePartitioningOptions struct {
	Mode                   string   `json:"mode,omitempty"`
	SourceURIPrefix        string   `json:"sourceUriPrefix,omitempty"`
	RequirePartitionFilter bool     `json:"requirePartitionFilter,omitempty"`
	Fields                 []string `json:"fields,omitempty"`
}

// CsvOptions is the csvOptions sub-object of ExternalDataConfiguration.
type CsvOptions struct {
	FieldDelimiter      string `json:"fieldDelimiter,omitempty"`
	Quote               string `json:"quote,omitempty"`
	Encoding            string `json:"encoding,omitempty"`
	AllowJaggedRows     bool   `json:"allowJaggedRows,omitempty"`
	AllowQuotedNewlines bool   `json:"allowQuotedNewlines,omitempty"`
	skipLeadingRows     int
}

// SkipLeadingRows returns the number of leading CSV rows to skip.
func (o *CsvOptions) SkipLeadingRows() int {
	if o == nil {
		return 0
	}
	return o.skipLeadingRows
}

// UnmarshalJSON accepts skipLeadingRows as JSON number or decimal string.
func (o *CsvOptions) UnmarshalJSON(data []byte) error {
	type alias CsvOptions
	var raw struct {
		alias
		SkipLeadingRows any `json:"skipLeadingRows,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*o = CsvOptions(raw.alias)
	if raw.SkipLeadingRows == nil {
		return nil
	}
	switch v := raw.SkipLeadingRows.(type) {
	case float64:
		o.skipLeadingRows = int(v)
	case string:
		n, err := strconv.Atoi(v)
		if err != nil {
			return fmt.Errorf("csvOptions.skipLeadingRows: %w", err)
		}
		o.skipLeadingRows = n
	default:
		return fmt.Errorf("csvOptions.skipLeadingRows: unsupported type %T", v)
	}
	return nil
}

// GoogleSheetsOptions is the googleSheetsOptions sub-object.
type GoogleSheetsOptions struct {
	Range           string `json:"range,omitempty"`
	skipLeadingRows int
}

// SkipLeadingRows returns the number of leading sheet rows to skip.
func (o *GoogleSheetsOptions) SkipLeadingRows() int {
	if o == nil {
		return 0
	}
	return o.skipLeadingRows
}

// UnmarshalJSON accepts skipLeadingRows as JSON number or decimal string.
func (o *GoogleSheetsOptions) UnmarshalJSON(data []byte) error {
	type alias GoogleSheetsOptions
	var raw struct {
		alias
		SkipLeadingRows any `json:"skipLeadingRows,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	*o = GoogleSheetsOptions(raw.alias)
	if raw.SkipLeadingRows == nil {
		return nil
	}
	switch v := raw.SkipLeadingRows.(type) {
	case float64:
		o.skipLeadingRows = int(v)
	case string:
		n, err := strconv.Atoi(v)
		if err != nil {
			return fmt.Errorf("googleSheetsOptions.skipLeadingRows: %w", err)
		}
		o.skipLeadingRows = n
	default:
		return fmt.Errorf("googleSheetsOptions.skipLeadingRows: unsupported type %T", v)
	}
	return nil
}

// TableConstraints mirrors the BigQuery REST tableConstraints object.
type TableConstraints struct {
	PrimaryKey *PrimaryKey `json:"primaryKey,omitempty"`
}

// PrimaryKey is the primaryKey sub-object of TableConstraints.
type PrimaryKey struct {
	Columns []string `json:"columns,omitempty"`
}

// ViewDefinition is the BigQuery REST view sub-object. See
// docs/bigquery/docs/reference/rest/v2/tables#ViewDefinition.
type ViewDefinition struct {
	Query        string `json:"query,omitempty"`
	UseLegacySQL bool   `json:"useLegacySql,omitempty"`
}

// MaterializedViewDefinition is the BigQuery REST materializedView
// sub-object. See docs/bigquery/docs/reference/rest/v2/tables#MaterializedViewDefinition.
type MaterializedViewDefinition struct {
	Query string `json:"query,omitempty"`
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
	Name                   string         `json:"name"`
	Type                   string         `json:"type"`           // STRING, INT64, FLOAT64, BOOL, TIMESTAMP, ...
	Mode                   string         `json:"mode,omitempty"` // NULLABLE, REQUIRED, REPEATED
	Description            string         `json:"description,omitempty"`
	DefaultValueExpression string         `json:"defaultValueExpression,omitempty"`
	Collation              string         `json:"collation,omitempty"`
	PolicyTags             *PolicyTagList `json:"policyTags,omitempty"`
	// MaskKind is an emulator extension for column-level data masking
	// (NULLIFY | SHA256 | DEFAULT_VALUE | DENIED). BigQuery clients
	// ignore unknown JSON fields; the gateway persists this via
	// SetColumnGovernance on tables.insert/patch/update.
	MaskKind string             `json:"maskKind,omitempty"`
	Fields   []TableFieldSchema `json:"fields,omitempty"` // for STRUCT/RECORD
}

// ExternalDatasetReference links a dataset to an external Spanner / Cloud SQL source.
type ExternalDatasetReference struct {
	Connection string `json:"connection,omitempty"`
	Source     string `json:"source,omitempty"`
}

// BiglakeConfiguration marks a BigLake-managed table.
type BiglakeConfiguration struct {
	ConnectionID string `json:"connectionId,omitempty"`
	StorageURI   string `json:"storageUri,omitempty"`
	FileFormat   string `json:"fileFormat,omitempty"`
	TableFormat  string `json:"tableFormat,omitempty"`
}

// ObjectTableOptions marks an object table over GCS object metadata.
type ObjectTableOptions struct {
	SourceURIs []string `json:"sourceUris,omitempty"`
}
