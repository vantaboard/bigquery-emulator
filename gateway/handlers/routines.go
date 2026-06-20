package handlers

import (
	"encoding/json"
	"io"
	"net/http"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/routines"
)

// routineListKind is the `kind` field for a routines.list response. See
// docs/bigquery/docs/reference/rest/v2/routines/list.md.
const routineListKind = "bigquery#listRoutinesResponse"

const (
	defaultRoutineType     = "SCALAR_FUNCTION"
	defaultRoutineLanguage = "SQL"
)

func routineStore(deps *Dependencies) *routines.Store {
	if deps.Routines == nil {
		deps.Routines = routines.NewStore()
	}
	return deps.Routines
}

func routineIDFromPath(r *http.Request) (projectID, datasetID, routineID string) {
	return r.PathValue("projectId"), r.PathValue("datasetId"), r.PathValue("routineId")
}

func decodeRoutineBody(w http.ResponseWriter, r *http.Request) (bqtypes.Routine, bool) {
	var rt bqtypes.Routine
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"Could not read routine request body: "+err.Error())
		return rt, false
	}
	if len(body) == 0 {
		return rt, true
	}
	if err := json.Unmarshal(body, &rt); err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"Could not parse routine request body as JSON: "+err.Error())
		return rt, false
	}
	return rt, true
}

func routineResource(projectID, datasetID, routineID string, rt bqtypes.Routine) bqtypes.Routine {
	rt.RoutineReference = bqtypes.RoutineReference{
		ProjectID: projectID,
		DatasetID: datasetID,
		RoutineID: routineID,
	}
	if rt.RoutineType == "" {
		rt.RoutineType = defaultRoutineType
	}
	if rt.Language == "" {
		rt.Language = defaultRoutineLanguage
	}
	if rt.CreationTime == "" {
		rt.CreationTime = nowMillis()
	}
	rt.LastModifiedTime = nowMillis()
	if rt.Etag == "" {
		rt.Etag = routines.MintEtag()
	}
	return rt
}

// routineListEntry trims a routine to the fields upstream list returns
// when readMask is unset.
func routineListEntry(rt bqtypes.Routine) bqtypes.Routine {
	return bqtypes.Routine{
		Etag:             rt.Etag,
		RoutineReference: rt.RoutineReference,
		RoutineType:      rt.RoutineType,
		CreationTime:     rt.CreationTime,
		LastModifiedTime: rt.LastModifiedTime,
		Language:         rt.Language,
	}
}

// RoutineList implements `bigquery.routines.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines
func RoutineList(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		var all []bqtypes.Routine
		if routineCatalogEnabled(&deps) {
			all = mergeRoutineSources(r.Context(), &deps, projectID, datasetID, r.URL.Query().Get("filter"))
		} else {
			all = routineStore(&deps).List(projectID, datasetID, r.URL.Query().Get("filter"))
		}
		items := make([]bqtypes.Routine, 0, len(all))
		for _, rt := range all {
			items = append(items, routineListEntry(rt))
		}
		resp := map[string]any{
			resourceKeyKind: routineListKind,
			"routines":      items,
		}
		if maxResults := r.URL.Query().Get("maxResults"); maxResults != "" {
			if n, err := strconv.Atoi(maxResults); err == nil && n >= 0 && n < len(items) {
				resp["routines"] = items[:n]
				resp["nextPageToken"] = strconv.Itoa(n)
			}
		}
		writeJSON(w, http.StatusOK, resp)
	}
}

// RoutineGet implements `bigquery.routines.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}
func RoutineGet(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, routineID := routineIDFromPath(r)
		rt, ok := routineLookupExisting(r.Context(), &deps, projectID, datasetID, routineID)
		if !ok {
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Not found: Routine "+projectID+":"+datasetID+"."+routineID)
			return
		}
		writeJSON(w, http.StatusOK, rt)
	}
}

// RoutineInsert implements `bigquery.routines.insert`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines
func RoutineInsert(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		rt, ok := decodeRoutineBody(w, r)
		if !ok {
			return
		}
		routineID := rt.RoutineReference.RoutineID
		if routineID == "" {
			writeError(w, http.StatusBadRequest, reasonInvalid,
				"Required routineReference.routineId is missing.")
			return
		}
		if rt.DefinitionBody == "" {
			writeError(w, http.StatusBadRequest, reasonInvalid,
				"Required definitionBody is missing.")
			return
		}
		if rt.RoutineType == "" {
			rt.RoutineType = defaultRoutineType
		}
		if rt.Language == "" {
			rt.Language = defaultRoutineLanguage
		}
		out := routineResource(projectID, datasetID, routineID, rt)
		if routineCatalogEnabled(&deps) {
			if catalogInsertRoutine(r.Context(), w, &deps, projectID, datasetID, routineID, out) {
				return
			}
		} else if !routineStore(&deps).Insert(out) {
			writeError(w, http.StatusConflict, reasonDuplicate,
				"Already Exists: Routine "+projectID+":"+datasetID+"."+routineID)
			return
		}
		routineStore(&deps).Upsert(out)
		writeJSON(w, http.StatusOK, out)
	}
}

// RoutineUpdate implements `bigquery.routines.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}
func RoutineUpdate(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, routineID := routineIDFromPath(r)
		existing, ok := routineLookupExisting(r.Context(), &deps, projectID, datasetID, routineID)
		if !ok {
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Not found: Routine "+projectID+":"+datasetID+"."+routineID)
			return
		}
		rt, ok := decodeRoutineBody(w, r)
		if !ok {
			return
		}
		out := routineResource(projectID, datasetID, routineID, rt)
		out.CreationTime = existing.CreationTime
		out.Etag = routines.MintEtag()
		if routineCatalogEnabled(&deps) {
			if err := catalogUpsertRoutine(r.Context(), &deps, out); err != nil {
				if grpcToHTTPError(w, err) {
					return
				}
				return
			}
		}
		routineStore(&deps).Upsert(out)
		writeJSON(w, http.StatusOK, out)
	}
}

// RoutineDelete implements `bigquery.routines.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}
func RoutineDelete(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, routineID := routineIDFromPath(r)
		if routineCatalogEnabled(&deps) {
			if err := catalogDeleteRoutine(r.Context(), &deps, projectID, datasetID, routineID); err != nil {
				if grpcToHTTPError(w, err) {
					return
				}
				return
			}
		}
		if !routineStore(&deps).Delete(projectID, datasetID, routineID) {
			if !routineCatalogEnabled(&deps) {
				writeError(w, http.StatusNotFound, reasonNotFound,
					"Not found: Routine "+projectID+":"+datasetID+"."+routineID)
				return
			}
		}
		writeJSON(w, http.StatusOK, struct{}{})
	}
}
