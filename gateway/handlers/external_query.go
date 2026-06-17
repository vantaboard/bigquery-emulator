package handlers

import (
	"context"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/external"
)

// queryDefaultDatasetForExecute resolves defaultDataset and materializes
// tableDefinitions before ExecuteQuery. Returns ok=false when an HTTP error
// was written.
func queryDefaultDatasetForExecute(
	deps Dependencies,
	w http.ResponseWriter,
	r *http.Request,
	projectID string,
	req *bqtypes.QueryRequest,
) (string, bool) {
	defaultDataset := resolveDefaultDataset(deps, req.DefaultDataset)
	defaultDataset, extErr := prepareQueryExternalTables(
		r.Context(), deps, projectID, req.TableDefinitions, defaultDataset)
	if writeExternalTableError(w, extErr) {
		return "", false
	}
	return defaultDataset, true
}

// prepareQueryExternalTables materializes ephemeral tableDefinitions and
// returns the default dataset id to forward to the engine. When err is
// non-nil the caller should emit an HTTP error (jobs.query) or record a
// failed job (jobs.insert).
func prepareQueryExternalTables(
	ctx context.Context,
	deps Dependencies,
	projectID string,
	tableDefs map[string]bqtypes.ExternalDataConfiguration,
	defaultDataset string,
) (string, error) {
	if len(tableDefs) == 0 || deps.Catalog == nil {
		return defaultDataset, nil
	}
	return external.PrepareTableDefinitionsWith(ctx, deps.Catalog, projectID, tableDefs, defaultDataset,
		externalResolver(deps))
}

// writeExternalTableError maps gateway-side external table failures to
// BigQuery-shaped HTTP responses for the synchronous query API.
func writeExternalTableError(w http.ResponseWriter, err error) bool {
	if err == nil {
		return false
	}
	writeError(w, http.StatusBadRequest, reasonInvalidQuery,
		"Could not prepare external table: "+err.Error())
	return true
}

// insertExternalTable materializes a GCS-backed external table on insert.
// Returns false when an error response was written.
func insertExternalTable(
	w http.ResponseWriter,
	r *http.Request,
	deps Dependencies,
	projectID, datasetID, tableID string,
	t *bqtypes.Table,
) bool {
	if t.Type == "" {
		t.Type = externalTableType
	}
	err := external.MaterializeWith(r.Context(), deps.Catalog, external.Target{
		ProjectID: projectID,
		DatasetID: datasetID,
		TableID:   tableID,
		Schema:    t.Schema,
	}, t.ExternalDataConfiguration, externalResolver(deps))
	return !writeExternalTableInsertError(w, err)
}

// writeExternalTableInsertError maps external table failures on tables.insert.
func writeExternalTableInsertError(w http.ResponseWriter, err error) bool {
	if err == nil {
		return false
	}
	writeError(w, http.StatusBadRequest, reasonInvalid,
		"Could not create external table: "+err.Error())
	return true
}
