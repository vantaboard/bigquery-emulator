package bqtypes_test

// Shared schema and StandardSqlDataType spellings for bqtypes regression
// tests. Centralized so goconst stops flagging repeats across files.
const (
	typeKindINT64     = "INT64"
	typeKindFLOAT64   = "FLOAT64"
	typeKindSTRING    = "STRING"
	typeKindTIMESTAMP = "TIMESTAMP"

	schemaTypeSTRING  = "STRING"
	schemaTypeINTEGER = "INTEGER"
	schemaTypeSTRUCT  = "STRUCT"
	typeKindARRAY     = "ARRAY"
	collationUndCI    = "und:ci"
	fieldNameAge      = "Age"
)
