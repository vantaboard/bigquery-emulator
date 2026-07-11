package bqinternal

// ImplicitQueryResultsDatasetID is the catalog dataset used to materialize
// anonymous SELECT job destinations. It is reachable via the REST API for
// pagination but must not appear in user-facing dataset listings.
const ImplicitQueryResultsDatasetID = "_bqemu_query_results"

// IsUserVisibleDataset reports whether datasetID should appear in datasets.list
// and explorer UI surfaces.
func IsUserVisibleDataset(datasetID string) bool {
	return datasetID != ImplicitQueryResultsDatasetID
}
