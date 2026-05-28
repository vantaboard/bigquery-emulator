package handlers

import (
	"net/http"
)

// modelListKind is the `kind` field for a models.list response. See
// docs/bigquery/docs/reference/rest/v2/models/list.md.
const modelListKind = "bigquery#listModelsResponse"

// ModelList implements `bigquery.models.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models
//
// The internal Catalog gRPC service has no model registry (BQML models
// require a trained-model store that the emulator does not implement),
// so the handler returns the BigQuery-shaped empty page. Client
// libraries that probe this endpoint at startup (e.g. nodejs-bigquery's
// `Models` test scaffold) get a structurally-valid response instead of
// a 404 from the catch-all `NotFound` handler. When BQML lands the
// stub can be promoted in place without changing the wire shape.
func ModelList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind: modelListKind,
			"models":        []any{},
		})
	}
}

// ModelGet implements `bigquery.models.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}
//
// Returns 404 with the BigQuery-shaped error envelope: the emulator
// has no BQML store so by definition any specific modelId is unknown.
// Distinct from `NotImplemented` because a 501 here would confuse
// client libraries that treat 404 as "absent" and 501 as "fatal".
func ModelGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		modelID := r.PathValue("modelId")
		writeError(w, http.StatusNotFound, reasonNotFound,
			"Not found: Model "+projectID+":"+datasetID+"."+modelID)
	}
}

// ModelPatch implements `bigquery.models.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}
//
// The emulator has no BQML store; patch is wired so clients see a
// structured 501 instead of a 404 catch-all when they try to update
// a model that was supposed to exist.
func ModelPatch(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// ModelDelete implements `bigquery.models.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}
//
// Returns 404 (no BQML store), matching ModelGet so a
// list-get-delete sample loop behaves predictably.
func ModelDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		modelID := r.PathValue("modelId")
		writeError(w, http.StatusNotFound, reasonNotFound,
			"Not found: Model "+projectID+":"+datasetID+"."+modelID)
	}
}
