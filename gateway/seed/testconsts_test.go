package seed

// Shared test-fixture literals used across the seed package's
// *_test.go files. Lives in its own file so the per-test bodies
// stay focused on intent and so goconst's "string X has N
// occurrences" warnings stay at zero without each individual test
// having to invent its own constant.
//
// Add a constant here when a literal is referenced from three or
// more test files (or three or more places inside one test file);
// otherwise keep it inline.
const (
	bqTypeInt64    = "INT64"
	bqTypeString   = "STRING"
	colName        = "name"
	rowValueAda    = "ada"
	tableKindTable = "TABLE"
	literalNil     = "nil"

	testSeedToken   = "s3cret"
	testProjectProd = "prod"
	testDatasetDS   = "ds"
	tableRefProdDSA = "prod.ds.a"
	tableRefPeople  = "prod.ds.people"

	envQuotaProject  = "quota"
	envGCloudProject = "gcp"
	envGCloudLegacy  = "gcloud"
)
