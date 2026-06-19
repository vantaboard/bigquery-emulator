package handlers

import (
	"net/http"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// registerInsertedTable forwards the decoded Table to the engine catalog
// (view registry, external table path, or generic RegisterTable).
// Returns false when the handler already wrote an error response.
func registerInsertedTable(
	w http.ResponseWriter,
	r *http.Request,
	deps Dependencies,
	projectID, datasetID, tableID string,
	t *bqtypes.Table,
) bool {
	switch {
	case t.View != nil && t.View.Query != "":
		// A logical view must be registered in the engine's view
		// registry (the same path CREATE VIEW DDL takes) so reads
		// inline its stored definition. Registering an empty backing
		// table instead — as the generic branch below does — shadows
		// the view in the engine catalog (FindTable resolves storage
		// before the view registry), so SELECT ... FROM <view>
		// silently returns zero rows. This is the REST-API analogue
		// of the CREATE-VIEW-on-read fix.
		return insertLogicalView(w, r, deps, projectID, datasetID, tableID, t.View.Query)
	case t.ExternalDataConfiguration != nil:
		return insertExternalTable(w, r, deps, projectID, datasetID, tableID, t)
	default:
		_, err := deps.Catalog.RegisterTable(r.Context(), &enginepb.RegisterTableRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
			Schema: schemaToProto(t.Schema),
		})
		return !grpcToHTTPError(w, err)
	}
}

func writeInsertedTableResponse(
	w http.ResponseWriter,
	deps Dependencies,
	r *http.Request,
	projectID, datasetID, tableID string,
	t bqtypes.Table,
) {
	if t.DefaultCollation != "" {
		t.Schema = bqtypes.ApplyDefaultCollationToStringFields(t.Schema, t.DefaultCollation)
	}
	deps.Metadata.PutTable(projectID, datasetID, tableID, t)
	SyncColumnGovernanceFromSchema(r.Context(), deps, projectID, datasetID, tableID, t.Schema)
	created := nowMillis()
	if deps.Snapshots != nil {
		if ms, parseErr := strconv.ParseInt(created, 10, 64); parseErr == nil {
			deps.Snapshots.RecordCreation(projectID, datasetID, tableID, ms)
		}
	}
	out := t
	if out.DefaultCollation != "" {
		out.Schema = bqtypes.ApplyDefaultCollationToStringFields(out.Schema, out.DefaultCollation)
	}
	writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, out))
}
