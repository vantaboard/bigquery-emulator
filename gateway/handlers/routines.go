package handlers

import (
	"net/http"
)

// routineListKind is the `kind` field for a routines.list response. See
// docs/bigquery/docs/reference/rest/v2/routines/list.md.
const routineListKind = "bigquery#listRoutinesResponse"

// RoutineList implements `bigquery.routines.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines
//
// The engine catalog has no routine (UDF / table-valued function /
// stored procedure) registry yet, so the handler returns the
// BigQuery-shaped empty page. Client libraries that probe this
// endpoint at startup get a structurally-valid response instead of a
// 404 from the catch-all `NotFound` handler. When routines land the
// stub can be promoted in place without changing the wire shape.
func RoutineList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"kind":     routineListKind,
			"routines": []any{},
		})
	}
}

// RoutineGet implements `bigquery.routines.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}
//
// Returns 404 with the BigQuery-shaped error envelope: no routine
// registry exists yet, so by definition any specific routineId is
// unknown. Distinct from `NotImplemented` because a 501 here would
// confuse client libraries that treat 404 as "absent" and 501 as
// "fatal".
func RoutineGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		routineID := r.PathValue("routineId")
		writeError(w, http.StatusNotFound, "notFound",
			"Not found: Routine "+projectID+":"+datasetID+"."+routineID)
	}
}

// RoutineInsert implements `bigquery.routines.insert`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines
//
// Wired as 501 so clients see a structured "not yet implemented"
// rather than a 404. Routines arrive when the engine grows a UDF /
// procedure registry.
func RoutineInsert(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// RoutineUpdate implements `bigquery.routines.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}
//
// Wired as 501 for the same reason as RoutineInsert.
func RoutineUpdate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// RoutineDelete implements `bigquery.routines.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}
//
// Returns 404 (no routine registry), matching RoutineGet so a
// list-get-delete sample loop behaves predictably.
func RoutineDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		routineID := r.PathValue("routineId")
		writeError(w, http.StatusNotFound, "notFound",
			"Not found: Routine "+projectID+":"+datasetID+"."+routineID)
	}
}
