package handlers

import (
	"net/http"
	"strconv"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// datasetKind is the value the BigQuery REST API returns for the
// `kind` field of a Dataset resource. See
// docs/bigquery/docs/reference/rest/v2/datasets/get.md.
const datasetKind = "bigquery#dataset"

// datasetListKind is the `kind` field for a DatasetList response. See
// docs/bigquery/docs/reference/rest/v2/datasets/list.md.
const datasetListKind = "bigquery#datasetList"

// datasetIDFromPath returns the {projectId}/{datasetId} pair captured
// by the route pattern. It strips any trailing AIP-136 custom-method
// suffix (e.g. ":undelete") from the datasetId so the same helper can
// be reused by DatasetCustomMethodPOST.
func datasetIDFromPath(r *http.Request) (projectID, datasetID string) {
	projectID = r.PathValue("projectId")
	datasetID, _ = splitColonOp(r.PathValue("datasetId"))
	return projectID, datasetID
}

// nowMillis is the BigQuery REST representation of a timestamp: a
// decimal string of milliseconds since epoch.
func nowMillis() string {
	return strconv.FormatInt(time.Now().UnixMilli(), 10)
}

// datasetResource builds a Dataset resource for a successful response.
// Stamps Kind, ID, and timestamps; preserves any caller-provided
// metadata (FriendlyName, Description, Location) that the engine does
// not need to know about.
//
// Access is materialized to an empty slice when the caller did not
// provide one. The Java BigQuery client wraps `dataset.getAcl()` in
// `new ArrayList<>(...)`, which NPEs on a null value; live BigQuery
// returns `access: []` for newly-created datasets and ACL-mutation
// flows like AuthorizeDatasetIT depend on that shape.
//
// Labels is materialized to an empty map for the same reason: upstream
// samples call `Object.entries(dataset.metadata.labels)` /
// `dict(dataset.labels)` on the deserialized response, which raises
// `TypeError: Cannot convert undefined or null to object` /
// `TypeError: argument of type 'NoneType' is not iterable` on a nil
// value. The bqtypes.Dataset.Labels tag omits `omitempty` so the empty
// map round-trips as `"labels":{}` on the wire.
func datasetResource(projectID, datasetID string, ds bqtypes.Dataset) bqtypes.Dataset {
	ds.Kind = datasetKind
	ds.ID = projectID + ":" + datasetID
	ds.DatasetReference = bqtypes.DatasetReference{
		ProjectID: projectID,
		DatasetID: datasetID,
	}
	if ds.CreationTime == "" {
		ds.CreationTime = nowMillis()
	}
	if ds.LastModifiedTime == "" {
		ds.LastModifiedTime = ds.CreationTime
	}
	if ds.Access == nil {
		ds.Access = []map[string]any{}
	}
	if ds.Labels == nil {
		ds.Labels = bqtypes.ResourceLabels{}
	}
	if ds.Location == "" {
		ds.Location = "US"
	}
	return ds
}

// DatasetList implements `bigquery.datasets.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets
//
// Calls the Catalog.ListDatasets RPC and folds the (deterministically
// ordered, ascending dataset_id) result into a BigQuery datasetList
// envelope. The shape matches
// docs/bigquery/docs/reference/rest/v2/datasets/list.md.
//
// Each returned entry is the minimal dataset-list shape upstream
// emits: kind, id (projectId:datasetId), datasetReference, and an
// empty labels object so client samples that call
// `Object.entries(item.metadata.labels)` on each iteration item do
// not raise (mirrors TestDatasetGetLabelsIsEmptyObjectNotNull).
//
// Pagination: no `nextPageToken` today. The emulator is single-host
// and the catalog never exceeds a handful of datasets in practice;
// the engine helper returns every entry in one shot. When that
// changes (large-catalog stress lane / horizontal sharding) the
// gateway can grow a token by re-keying on dataset_id and slicing
// the response here.
func DatasetList(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		if deps.Catalog == nil {
			writeJSON(w, http.StatusOK, map[string]any{
				resourceKeyKind:     datasetListKind,
				resourceKeyDatasets: []bqtypes.Dataset{},
			})
			return
		}
		resp, err := deps.Catalog.ListDatasets(r.Context(), &enginepb.ListDatasetsRequest{
			ProjectId: projectID,
		})
		if grpcToHTTPError(w, err) {
			return
		}
		items := make([]map[string]any, 0, len(resp.GetDatasets()))
		for _, ref := range resp.GetDatasets() {
			labels := bqtypes.ResourceLabels{}
			if overlay, ok := deps.Metadata.GetDataset(
				ref.GetProjectId(), ref.GetDatasetId(),
			); ok && overlay.Labels != nil {
				labels = overlay.Labels
			}
			items = append(items, map[string]any{
				"kind": datasetKind,
				"id":   ref.GetProjectId() + ":" + ref.GetDatasetId(),
				"datasetReference": bqtypes.DatasetReference{
					ProjectID: ref.GetProjectId(),
					DatasetID: ref.GetDatasetId(),
				},
				"labels": labels,
			})
		}
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind:     datasetListKind,
			resourceKeyDatasets: items,
		})
	}
}

// DatasetInsert implements `bigquery.datasets.insert`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets
//
// Decodes the Dataset body, calls Catalog.RegisterDataset on the
// engine, and returns the newly-created Dataset resource on success.
// The dataset's `datasetReference.datasetId` is required; the projectId
// is taken from the URL because the upstream API treats the path's
// projectId as authoritative when both are set.
func DatasetInsert(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		ds, ok := decodeDatasetBody(w, r)
		if !ok {
			return
		}
		datasetID := ds.DatasetReference.DatasetID
		if datasetID == "" {
			writeError(w, http.StatusBadRequest, "invalid",
				"datasetReference.datasetId is required")
			return
		}
		if err := validateDatasetLocation(r, ds.Location); err != nil {
			writeError(w, http.StatusBadRequest, "invalid", err.Error())
			return
		}
		if rejectUnsupportedDatasetPosture(w, &ds) {
			return
		}
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		_, err := deps.Catalog.RegisterDataset(r.Context(), &enginepb.RegisterDatasetRequest{
			Dataset: &enginepb.DatasetRef{
				ProjectId: projectID,
				DatasetId: datasetID,
			},
			Location: ds.Location,
		})
		if grpcToHTTPError(w, err) {
			return
		}
		deps.Metadata.PutDataset(projectID, datasetID, ds)
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
}

// DatasetGet implements `bigquery.datasets.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// The Catalog gRPC service does not yet expose a Get RPC (only
// Register/Drop/List), so existence is checked via ListDatasets before
// synthesizing the Dataset resource from path parameters plus any
// MetadataStore overlay.
//
// REST-only metadata (labels, defaultCollation, friendlyName, ...) is
// surfaced from the in-memory MetadataStore so a prior
// Insert/Patch/Update round-trips through GET — required by the node
// `getDatasetLabels` sample's `Object.entries(dataset.metadata.labels)`
// loop.
func DatasetGet(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		exists, err := catalogDatasetExists(r.Context(), deps, projectID, datasetID)
		if err != nil {
			if grpcToHTTPError(w, err) {
				return
			}
		}
		if !exists {
			writeDatasetNotFound(w, projectID, datasetID)
			return
		}
		ds := bqtypes.Dataset{}
		if overlay, ok := deps.Metadata.GetDataset(projectID, datasetID); ok {
			ds = applyDatasetMetadataOverlay(ds, overlay)
		}
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
}

// DatasetUpdate implements `bigquery.datasets.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// Full replacement of the Dataset metadata. The engine catalog does
// not yet have an update RPC, so the handler echoes the request body
// back as the canonical resource (stamping kind/id/timestamps) and
// records the REST-only metadata fields in the in-memory store so a
// subsequent GET returns the updated values.
func DatasetUpdate(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		exists, err := catalogDatasetExists(r.Context(), deps, projectID, datasetID)
		if err != nil {
			if grpcToHTTPError(w, err) {
				return
			}
		}
		if !exists {
			writeDatasetNotFound(w, projectID, datasetID)
			return
		}
		ds, ok := decodeDatasetBody(w, r)
		if !ok {
			return
		}
		if rejectUnsupportedDatasetPosture(w, &ds) {
			return
		}
		deps.Metadata.PutDataset(projectID, datasetID, ds)
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
}

// DatasetPatch implements `bigquery.datasets.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// Sparse update; mirrors DatasetUpdate's metadata-stash posture so
// upstream `setMetadata` + `getMetadata` sequences roundtrip the
// REST-only fields.
func DatasetPatch(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		exists, err := catalogDatasetExists(r.Context(), deps, projectID, datasetID)
		if err != nil {
			if grpcToHTTPError(w, err) {
				return
			}
		}
		if !exists {
			writeDatasetNotFound(w, projectID, datasetID)
			return
		}
		ds, ok := decodeDatasetBody(w, r)
		if !ok {
			return
		}
		if rejectUnsupportedDatasetPosture(w, &ds) {
			return
		}
		deps.Metadata.MergeDataset(projectID, datasetID, ds)
		if overlay, ok := deps.Metadata.GetDataset(projectID, datasetID); ok {
			ds = applyDatasetMetadataOverlay(ds, overlay)
		}
		if ds.LabelsPatchPresent() && len(ds.Labels) == 0 {
			ds.SetOmitEmptyLabelsOnWire(true)
		}
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
}

// DatasetDelete implements `bigquery.datasets.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// Honors the documented `deleteContents` query parameter by forwarding
// it as DropDatasetRequest.delete_contents; without it the engine
// refuses to drop a non-empty dataset (FailedPrecondition → 400).
func DatasetDelete(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		deleteContents := r.URL.Query().Get("deleteContents") == queryParamTrue
		_, err := deps.Catalog.DropDataset(r.Context(), &enginepb.DropDatasetRequest{
			Dataset: &enginepb.DatasetRef{
				ProjectId: projectID,
				DatasetId: datasetID,
			},
			DeleteContents:   deleteContents,
			RestMetadataJson: deps.Metadata.RestMetadataJSON(projectID, datasetID),
		})
		if grpcToHTTPError(w, err) {
			return
		}
		deps.Metadata.DeleteDataset(projectID, datasetID)
		if deleteContents {
			deps.Metadata.DeleteTablesInDataset(projectID, datasetID)
		}
		writeJSON(w, http.StatusOK, struct{}{})
	}
}

// DatasetUndelete implements `bigquery.datasets.undelete`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}:undelete
//
// Reached via DatasetCustomMethodPOST after parsing the trailing :op.
func DatasetUndelete(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		resp, err := deps.Catalog.UndeleteDataset(r.Context(), &enginepb.UndeleteDatasetRequest{
			Dataset: &enginepb.DatasetRef{
				ProjectId: projectID,
				DatasetId: datasetID,
			},
		})
		if grpcToHTTPError(w, err) {
			return
		}
		if resp != nil && resp.GetRestMetadataJson() != "" {
			deps.Metadata.RestoreDatasetRestMetadataJSON(
				projectID, datasetID, resp.GetRestMetadataJson())
		}
		ds, ok := deps.Metadata.GetDataset(projectID, datasetID)
		if !ok {
			ds = bqtypes.Dataset{Location: "US"}
		}
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
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
