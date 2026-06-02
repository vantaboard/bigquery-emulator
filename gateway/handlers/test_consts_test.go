package handlers

// Shared per-package test fixtures. Several handler tests construct the
// same synthetic project / dataset / table identifiers and assert on
// the same SQL type spellings; goconst flagged them because they
// repeated 4+ times across tests. Promoting them to package-level test
// consts keeps the spelling in a single place and lets goconst stop
// counting them.
const (
	testProjectID = "proj"
	testDatasetID = "ds1"
	testTableID   = "t1"
)

// Common BigQuery-wire SQL type spellings asserted by the JSON-shape
// regression tests.
const (
	sqlTypeINT64    = "INT64"
	sqlTypeFLOAT64  = "FLOAT64"
	sqlTypeSTRING   = "STRING"
	sqlModeNullable = "NULLABLE"
	sqlModeRequired = "REQUIRED"
)

// Common synthetic field name used by handler regression tests when
// the column itself has no significance beyond "the second column".
const testColumnName = "name"

// Test-user identities used in jobs.query result-row fixtures.
const (
	testUserAlice = "alice"
)

// Common HTTP wire bits.
const (
	contentTypeJSON = "application/json; charset=utf-8"
)

// testSQLSelectOne is the most common trivial SQL fixture across the
// query / jobs handler regression tests. Promoted to a constant so
// goconst stops counting the repeated literal across files.
const testSQLSelectOne = "SELECT 1"
