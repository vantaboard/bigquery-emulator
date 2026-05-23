package handlers

import "net/http"

// DatasetList implements `bigquery.datasets.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets
func DatasetList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetInsert implements `bigquery.datasets.insert`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets
func DatasetInsert(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetGet implements `bigquery.datasets.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}
func DatasetGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetUpdate implements `bigquery.datasets.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}
func DatasetUpdate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetPatch implements `bigquery.datasets.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}
func DatasetPatch(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetDelete implements `bigquery.datasets.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}
func DatasetDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetUndelete implements `bigquery.datasets.undelete`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}:undelete
//
// Reached via DatasetCustomMethodPOST after parsing the trailing :op.
func DatasetUndelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetCustomMethodPOST dispatches the AIP-136 custom-method POST
// endpoints registered against `/datasets/{datasetId}` (which Go's mux
// can't match as `:op` directly). Today the only such method is
// `datasets.undelete`; future BigQuery additions can be added here.
func DatasetCustomMethodPOST(deps Dependencies) http.HandlerFunc {
	undelete := DatasetUndelete(deps)
	return func(w http.ResponseWriter, r *http.Request) {
		_, op := splitColonOp(r.PathValue("datasetId"))
		switch op {
		case "undelete":
			undelete(w, r)
		case "":
			writeError(w, http.StatusMethodNotAllowed, "invalid",
				"POST is not allowed on a dataset resource. "+
					"Use POST /datasets to create, or a documented :op "+
					"custom method (e.g. :undelete).")
		default:
			writeError(w, http.StatusNotFound, "notFound",
				"Unknown dataset custom method ':"+op+"'.")
		}
	}
}
