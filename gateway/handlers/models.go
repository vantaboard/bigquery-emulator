package handlers

import (
	"context"
	"net/http"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/models"
)

// modelListKind is the `kind` field for a models.list response. See
// docs/bigquery/docs/reference/rest/v2/models/list.md.
const modelListKind = "bigquery#listModelsResponse"

func modelStore(deps *Dependencies) *models.Store {
	if deps.Models == nil {
		deps.Models = models.NewStore()
	}
	return deps.Models
}

func modelIDFromPath(r *http.Request) (projectID, datasetID, modelID string) {
	return r.PathValue("projectId"), r.PathValue("datasetId"), r.PathValue("modelId")
}

func modelListEntry(m bqtypes.Model) bqtypes.Model {
	return bqtypes.Model{
		ModelReference:   m.ModelReference,
		ModelType:        m.ModelType,
		CreationTime:     m.CreationTime,
		LastModifiedTime: m.LastModifiedTime,
		Labels:           m.Labels,
	}
}

// ModelList implements `bigquery.models.list`.
func ModelList(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		all := modelStore(&deps).List(projectID, datasetID, r.URL.Query().Get("filter"))
		items := make([]bqtypes.Model, 0, len(all))
		for _, m := range all {
			items = append(items, modelListEntry(m))
		}
		resp := map[string]any{
			resourceKeyKind: modelListKind,
			"models":        items,
		}
		if maxResults := r.URL.Query().Get("maxResults"); maxResults != "" {
			if n, err := strconv.Atoi(maxResults); err == nil && n >= 0 && n < len(items) {
				resp["models"] = items[:n]
				resp["nextPageToken"] = strconv.Itoa(n)
			}
		}
		writeJSON(w, http.StatusOK, resp)
	}
}

// ModelGet implements `bigquery.models.get`.
func ModelGet(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, modelID := modelIDFromPath(r)
		m, ok := modelStore(&deps).Get(projectID, datasetID, modelID)
		if !ok {
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Not found: Model "+projectID+":"+datasetID+"."+modelID)
			return
		}
		writeJSON(w, http.StatusOK, m)
	}
}

// ModelPatch implements `bigquery.models.patch`.
func ModelPatch(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// ModelDelete implements `bigquery.models.delete`.
func ModelDelete(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, modelID := modelIDFromPath(r)
		if !modelStore(&deps).Delete(projectID, datasetID, modelID) {
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Not found: Model "+projectID+":"+datasetID+"."+modelID)
			return
		}
		w.WriteHeader(http.StatusOK)
	}
}

// persistModelFromDDL registers CREATE MODEL metadata in the in-memory store.
func persistModelFromDDL(
	_ context.Context,
	deps *Dependencies,
	projectID, defaultDatasetID, sql string,
) *bqtypes.ModelReference {
	ref := models.RegisterFromDDL(modelStore(deps), projectID, defaultDatasetID, sql)
	return ref
}

func isCreateModelSQL(sql string) bool {
	trim := strings.ToUpper(strings.TrimSpace(sql))
	return strings.HasPrefix(trim, "CREATE MODEL") ||
		strings.HasPrefix(trim, "CREATE OR REPLACE MODEL") ||
		strings.HasPrefix(trim, "CREATE MODEL IF NOT EXISTS")
}
