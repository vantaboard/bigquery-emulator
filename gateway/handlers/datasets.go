package handlers

import (
	"encoding/json"
	"io"
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

// decodeDatasetBody reads and unmarshals a Dataset JSON body. An empty
// body is treated as an empty Dataset so handlers can choose whether
// to require fields explicitly. Any decode failure is reported with a
// BigQuery-shaped 400 envelope and the returned bool is false.
func decodeDatasetBody(w http.ResponseWriter, r *http.Request) (bqtypes.Dataset, bool) {
	var ds bqtypes.Dataset
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not read dataset request body: "+err.Error())
		return ds, false
	}
	if len(body) == 0 {
		return ds, true
	}
	if err := json.Unmarshal(body, &ds); err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not parse dataset request body as JSON: "+err.Error())
		return ds, false
	}
	return ds, true
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
	ds.LastModifiedTime = nowMillis()
	return ds
}

// DatasetList implements `bigquery.datasets.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets
//
// The internal Catalog gRPC service has no list method (catalog state
// is owned by Storage; cf. proto/emulator.proto), so the handler
// returns the BigQuery-shaped empty page until Storage gains a list
// API. The shape matches docs/bigquery/docs/reference/rest/v2/datasets/list.md
// so client libraries can iterate without special-casing the emulator.
func DatasetList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"kind":     datasetListKind,
			"datasets": []bqtypes.Dataset{},
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
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
}

// DatasetGet implements `bigquery.datasets.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// The Catalog gRPC service does not yet expose a Get RPC (only
// Register/Drop), so this handler returns a synthesized Dataset
// resource derived from the path parameters. It is a Phase-3 stub
// that satisfies clients which only need the reference + kind to
// proceed; a true existence check lands when Storage grows a
// DescribeDataset method.
func DatasetGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, bqtypes.Dataset{}))
	}
}

// DatasetUpdate implements `bigquery.datasets.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// Update is a full replacement of the Dataset metadata. The engine
// catalog does not yet have an update RPC, so this handler echoes the
// caller-supplied body back as the canonical resource, stamping the
// kind/id/timestamps that the client expects in the response.
func DatasetUpdate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		ds, ok := decodeDatasetBody(w, r)
		if !ok {
			return
		}
		writeJSON(w, http.StatusOK, datasetResource(projectID, datasetID, ds))
	}
}

// DatasetPatch implements `bigquery.datasets.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}
//
// Patch is a sparse update; the body contains only the fields to
// change. Until the engine grows a true patch RPC this handler simply
// echoes the request back as the resource, which is enough for client
// libraries that immediately read the response value.
func DatasetPatch(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID := datasetIDFromPath(r)
		ds, ok := decodeDatasetBody(w, r)
		if !ok {
			return
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
		_, err := deps.Catalog.DropDataset(r.Context(), &enginepb.DropDatasetRequest{
			Dataset: &enginepb.DatasetRef{
				ProjectId: projectID,
				DatasetId: datasetID,
			},
			DeleteContents: r.URL.Query().Get("deleteContents") == "true",
		})
		if grpcToHTTPError(w, err) {
			return
		}
		writeJSON(w, http.StatusOK, struct{}{})
	}
}

// DatasetUndelete implements `bigquery.datasets.undelete`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}:undelete
//
// Reached via DatasetCustomMethodPOST after parsing the trailing :op.
// The engine has no undelete RPC yet; remain a 501 stub.
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
