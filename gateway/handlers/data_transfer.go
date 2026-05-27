package handlers

import (
	"net/http"
)

// BigQuery Data Transfer Service v1 REST shell.
//
// Upstream lives at `https://bigquerydatatransfer.googleapis.com/v1/...`.
// Client libraries (cloud.google.com/go/bigquery/datatransfer,
// google-cloud-bigquery-datatransfer for Python/Node/Java) honor
// `BIGQUERY_EMULATOR_HOST` for the local emulator path. The transfer
// service has its own resource model (dataSources catalog, transfer
// configs, transfer runs / messages) which the engine does not
// implement.
//
// This shell mirrors the BigQuery v2 `models` / `routines` posture:
// list endpoints return the documented empty page so client probes
// succeed, get returns 404, create/update return 501. Both the
// project-scoped and the location-scoped path families are wired
// (the client libraries pick whichever the user's API region demands).
// Routes registered (the Data Transfer API has both project-scoped
// and location-scoped variants — clients pick based on whether they
// want regional placement):
//
//   GET    /v1/projects/{projectId}/dataSources
//   GET    /v1/projects/{projectId}/dataSources/{dataSourceId}
//   GET    /v1/projects/{projectId}/locations/{location}/dataSources
//   GET    /v1/projects/{projectId}/locations/{location}/dataSources/{dataSourceId}
//   GET    /v1/projects/{projectId}/transferConfigs
//   GET    /v1/projects/{projectId}/transferConfigs/{configId}
//   POST   /v1/projects/{projectId}/transferConfigs
//   GET    /v1/projects/{projectId}/locations/{location}/transferConfigs
//   GET    /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}
//   POST   /v1/projects/{projectId}/locations/{location}/transferConfigs

// DataTransferDataSourceList implements `dataSources.list`. Returns the
// documented empty page so client libraries that probe the catalog at
// startup get a structurally-valid response.
func DataTransferDataSourceList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"dataSources": []any{},
		})
	}
}

// DataTransferDataSourceGet implements `dataSources.get`. Returns 404
// for every dataSourceId because the catalog is empty.
func DataTransferDataSourceGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		writeError(w, http.StatusNotFound, "notFound",
			"Not found: DataSource "+r.PathValue("dataSourceId"))
	}
}

// DataTransferConfigList implements `transferConfigs.list`. Returns
// the documented empty page.
func DataTransferConfigList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"transferConfigs": []any{},
		})
	}
}

// DataTransferConfigGet implements `transferConfigs.get`. Returns 404
// (no config store yet).
func DataTransferConfigGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		writeError(w, http.StatusNotFound, "notFound",
			"Not found: TransferConfig "+r.PathValue("configId"))
	}
}

// DataTransferConfigCreate implements `transferConfigs.create`. Returns
// 501 (mutating without a backing store would leak fake state).
func DataTransferConfigCreate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
