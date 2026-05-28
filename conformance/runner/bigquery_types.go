package runner

// BigQuery TypeKind spellings the row-diff path uses to dispatch to
// the right scalar comparator. Promoted from inline switch literals so
// goconst stops flagging the repeats and the wire spelling stays a
// single source of truth.
const (
	bqTypeINT64   = "INT64"
	bqTypeFLOAT64 = "FLOAT64"
	bqTypeSTRING  = "STRING"
)

// Canonical bool literal spellings emitted by the engine on the JSON
// wire. The diff path canonicalizes input to one of these before
// comparing.
const (
	boolLiteralTrue  = "true"
	boolLiteralFalse = "false"
)
