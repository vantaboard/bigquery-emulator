package handlers

import "net/http"

// TableList implements `bigquery.tables.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables
func TableList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableInsert implements `bigquery.tables.insert`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables
func TableInsert(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableGet implements `bigquery.tables.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
func TableGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableUpdate implements `bigquery.tables.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
func TableUpdate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TablePatch implements `bigquery.tables.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
func TablePatch(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableDelete implements `bigquery.tables.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
func TableDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableGetIamPolicy implements `bigquery.tables.getIamPolicy`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:getIamPolicy
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableGetIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableSetIamPolicy implements `bigquery.tables.setIamPolicy`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:setIamPolicy
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableSetIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableTestIamPermissions implements `bigquery.tables.testIamPermissions`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:testIamPermissions
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableTestIamPermissions(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableDataList implements `bigquery.tabledata.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data
func TableDataList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableDataInsertAll implements `bigquery.tabledata.insertAll`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll
func TableDataInsertAll(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableCustomMethodPOST dispatches the AIP-136 custom-method POST
// endpoints registered against `/tables/{tableId}` -- the three IAM
// helpers BigQuery exposes for table resources.
func TableCustomMethodPOST(deps Dependencies) http.HandlerFunc {
	getPolicy := TableGetIamPolicy(deps)
	setPolicy := TableSetIamPolicy(deps)
	testPerms := TableTestIamPermissions(deps)
	return func(w http.ResponseWriter, r *http.Request) {
		_, op := splitColonOp(r.PathValue("tableId"))
		switch op {
		case "getIamPolicy":
			getPolicy(w, r)
		case "setIamPolicy":
			setPolicy(w, r)
		case "testIamPermissions":
			testPerms(w, r)
		case "":
			writeError(w, http.StatusMethodNotAllowed, "invalid",
				"POST is not allowed on a table resource. "+
					"Use POST /tables to create, /insertAll to stream rows, "+
					"or a documented :op IAM custom method.")
		default:
			writeError(w, http.StatusNotFound, "notFound",
				"Unknown table custom method ':"+op+"'.")
		}
	}
}
