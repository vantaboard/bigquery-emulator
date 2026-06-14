package bqstorage

// BigQuery schema type and mode spellings used by the storage read/write
// shim when mapping engine cells to Arrow, Avro, and proto rows.
const (
	bqModeRequired = "REQUIRED"
	bqModeNullable = "NULLABLE"
	bqModeRepeated = "REPEATED"

	bqTypeINT64      = "INT64"
	bqTypeINTEGER    = "INTEGER"
	bqTypeFLOAT64    = "FLOAT64"
	bqTypeFLOAT      = "FLOAT"
	bqTypeBOOL       = "BOOL"
	bqTypeTIMESTAMP  = "TIMESTAMP"
	bqTypeDATETIME   = "DATETIME"
	bqTypeSTRING     = "STRING"
	bqTypeBYTES      = "BYTES"
	bqTypeDATE       = "DATE"
	bqTypeTIME       = "TIME"
	bqTypeNUMERIC    = "NUMERIC"
	bqTypeBIGNUMERIC = "BIGNUMERIC"
	bqTypeJSON       = "JSON"
	bqTypeGEOGRAPHY  = "GEOGRAPHY"
	bqTypeSTRUCT     = "STRUCT"
	bqTypeRECORD     = "RECORD"
)

// Avro schema map keys and primitive type names.
const (
	avroKeyType        = "type"
	avroKeyName        = "name"
	avroKeyLogicalType = "logicalType"
	avroTypeLong       = "long"
	avroTypeBytes      = "bytes"
	avroTypeString     = "string"
	avroTypeRecord     = "record"
)
