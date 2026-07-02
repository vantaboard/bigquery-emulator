package handlers

import (
	"context"
	"net/http"
	"slices"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/routines"
)

func routineCatalogEnabled(deps *Dependencies) bool {
	return deps.Catalog != nil
}

func routineRefProto(projectID, datasetID, routineID string) *enginepb.RoutineRef {
	return &enginepb.RoutineRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		RoutineId: routineID,
	}
}

func routineListKey(ref bqtypes.RoutineReference) string {
	return ref.ProjectID + ":" + ref.DatasetID + "." + ref.RoutineID
}

func routineTypeFromFilter(filter string) string {
	const prefix = "routineType:"
	if filter == "" || !strings.HasPrefix(filter, prefix) {
		return ""
	}
	return strings.TrimSpace(filter[len(prefix):])
}

// overlayRoutineFromStore merges gateway-store timestamps and etag onto a
// catalog-backed routine. The engine catalog does not persist those fields.
func overlayRoutineFromStore(catalog bqtypes.Routine, stored bqtypes.Routine, ok bool) bqtypes.Routine {
	if !ok {
		return catalog
	}
	if stored.CreationTime != "" {
		catalog.CreationTime = stored.CreationTime
	}
	if stored.LastModifiedTime != "" {
		catalog.LastModifiedTime = stored.LastModifiedTime
	}
	if stored.Etag != "" {
		catalog.Etag = stored.Etag
	}
	return catalog
}

func ensureRoutineTimestamps(rt *bqtypes.Routine) {
	if rt.CreationTime == "" {
		rt.CreationTime = nowMillis()
	}
	if rt.LastModifiedTime == "" {
		rt.LastModifiedTime = rt.CreationTime
	}
}

func routineFromDescriptor(desc *enginepb.RoutineDescriptor) bqtypes.Routine {
	if desc == nil {
		return bqtypes.Routine{}
	}
	ref := desc.GetRoutine()
	rt := bqtypes.Routine{
		RoutineReference: bqtypes.RoutineReference{
			ProjectID: ref.GetProjectId(),
			DatasetID: ref.GetDatasetId(),
			RoutineID: ref.GetRoutineId(),
		},
		RoutineType:    bqtypes.RoutineType(desc.GetRoutineType()),
		Language:       bqtypes.RoutineLanguage(desc.GetLanguage()),
		DefinitionBody: desc.GetDefinitionBody(),
	}
	ddl := desc.GetDdlSql()
	if ddl == "" {
		return rt
	}
	parsed, ok := routines.ParseCreateRoutineDDL(ref.GetProjectId(), ref.GetDatasetId(), ddl)
	if !ok {
		return rt
	}
	applyRoutineFromDDL(&rt, parsed)
	return rt
}

func applyRoutineFromDDL(rt *bqtypes.Routine, parsed bqtypes.Routine) {
	if parsed.DefinitionBody != "" {
		rt.DefinitionBody = parsed.DefinitionBody
	}
	if len(parsed.Arguments) > 0 {
		rt.Arguments = parsed.Arguments
	}
	if parsed.ReturnType != nil {
		rt.ReturnType = parsed.ReturnType
	}
	if parsed.RoutineType != "" {
		rt.RoutineType = parsed.RoutineType
	}
	if parsed.Language != "" {
		rt.Language = parsed.Language
	}
	if parsed.PythonOptions != nil {
		rt.PythonOptions = parsed.PythonOptions
	}
}

func catalogGetRoutine(
	ctx context.Context,
	deps *Dependencies,
	projectID, datasetID, routineID string,
) (bqtypes.Routine, bool) {
	resp, err := deps.Catalog.GetRoutine(ctx, &enginepb.GetRoutineRequest{
		Routine: routineRefProto(projectID, datasetID, routineID),
	})
	if err != nil || resp == nil || resp.GetRoutine() == nil {
		return bqtypes.Routine{}, false
	}
	return routineFromDescriptor(resp.GetRoutine()), true
}

func catalogListRoutines(ctx context.Context, deps *Dependencies, projectID, datasetID string) []bqtypes.Routine {
	resp, err := deps.Catalog.ListRoutines(ctx, &enginepb.ListRoutinesRequest{
		Dataset: &enginepb.DatasetRef{
			ProjectId: projectID,
			DatasetId: datasetID,
		},
	})
	if err != nil || resp == nil {
		return nil
	}
	out := make([]bqtypes.Routine, 0, len(resp.GetRoutines()))
	for _, desc := range resp.GetRoutines() {
		out = append(out, routineFromDescriptor(desc))
	}
	return out
}

// mergeRoutineSources unions catalog and in-memory store entries when the
// catalog is enabled so DDL-registered routines appear in list even if the
// engine list lags, and store-only routines remain visible.
func mergeRoutineSources(
	ctx context.Context,
	deps *Dependencies,
	projectID, datasetID, filter string,
) []bqtypes.Routine {
	store := routineStore(deps)
	fromStore := store.List(projectID, datasetID, filter)
	if !routineCatalogEnabled(deps) {
		return fromStore
	}
	wantType := routineTypeFromFilter(filter)
	fromCatalog := catalogListRoutines(ctx, deps, projectID, datasetID)
	byKey := make(map[string]bqtypes.Routine, len(fromCatalog)+len(fromStore))
	order := make([]string, 0, len(fromCatalog)+len(fromStore))
	add := func(rt bqtypes.Routine) {
		if wantType != "" && string(rt.RoutineType) != wantType {
			return
		}
		key := routineListKey(rt.RoutineReference)
		if _, exists := byKey[key]; exists {
			return
		}
		ensureRoutineTimestamps(&rt)
		byKey[key] = rt
		order = append(order, key)
	}
	for _, rt := range fromCatalog {
		ref := rt.RoutineReference
		stored, ok := store.Get(ref.ProjectID, ref.DatasetID, ref.RoutineID)
		add(overlayRoutineFromStore(rt, stored, ok))
	}
	for _, rt := range fromStore {
		key := routineListKey(rt.RoutineReference)
		if _, exists := byKey[key]; exists {
			continue
		}
		add(rt)
	}
	slices.Sort(order)
	out := make([]bqtypes.Routine, 0, len(order))
	for _, key := range order {
		out = append(out, byKey[key])
	}
	return out
}

func routineLookupExisting(
	ctx context.Context,
	deps *Dependencies,
	projectID, datasetID, routineID string,
) (bqtypes.Routine, bool) {
	store := routineStore(deps)
	if routineCatalogEnabled(deps) {
		if rt, ok := catalogGetRoutine(ctx, deps, projectID, datasetID, routineID); ok {
			stored, found := store.Get(projectID, datasetID, routineID)
			rt = overlayRoutineFromStore(rt, stored, found)
			ensureRoutineTimestamps(&rt)
			return rt, true
		}
	}
	rt, ok := store.Get(projectID, datasetID, routineID)
	if ok {
		ensureRoutineTimestamps(&rt)
	}
	return rt, ok
}

// catalogInsertRoutine persists a new routine via the catalog. Returns true when
// the HTTP response has been written (conflict or engine error).
func catalogInsertRoutine(
	ctx context.Context,
	w http.ResponseWriter,
	deps *Dependencies,
	projectID, datasetID, routineID string,
	out bqtypes.Routine,
) bool {
	if _, exists := catalogGetRoutine(ctx, deps, projectID, datasetID, routineID); exists {
		writeError(w, http.StatusConflict, reasonDuplicate,
			"Already Exists: Routine "+projectID+":"+datasetID+"."+routineID)
		return true
	}
	if err := catalogUpsertRoutine(ctx, deps, out); err != nil {
		grpcToHTTPError(w, err)
		return true
	}
	return false
}

func catalogUpsertRoutine(ctx context.Context, deps *Dependencies, rt bqtypes.Routine) error {
	ddl := routines.BuildDDLFromRoutine(rt)
	_, err := deps.Catalog.UpsertRoutine(ctx, &enginepb.UpsertRoutineRequest{
		Routine: &enginepb.RoutineDescriptor{
			Routine: routineRefProto(
				rt.RoutineReference.ProjectID,
				rt.RoutineReference.DatasetID,
				rt.RoutineReference.RoutineID,
			),
			RoutineType:    string(rt.RoutineType),
			Language:       string(rt.Language),
			DefinitionBody: rt.DefinitionBody,
			DdlSql:         ddl,
		},
	})
	return err
}

func catalogDeleteRoutine(ctx context.Context, deps *Dependencies, projectID, datasetID, routineID string) error {
	_, err := deps.Catalog.DeleteRoutine(ctx, &enginepb.DeleteRoutineRequest{
		Routine: routineRefProto(projectID, datasetID, routineID),
	})
	return err
}

// persistRoutineFromDDL registers a routine parsed from CREATE FUNCTION /
// PROCEDURE DDL in the in-memory store and mirrors it to the catalog when
// enabled so RoutineGet sees the same metadata as RoutineInsert.
func persistRoutineFromDDL(
	ctx context.Context,
	deps *Dependencies,
	projectID, defaultDatasetID, sql string,
) *bqtypes.RoutineReference {
	store := routineStore(deps)
	ref := routines.RegisterFromDDL(store, projectID, defaultDatasetID, sql)
	if ref == nil || !routineCatalogEnabled(deps) {
		return ref
	}
	rt, ok := store.Get(ref.ProjectID, ref.DatasetID, ref.RoutineID)
	if !ok {
		return ref
	}
	_ = catalogUpsertRoutine(ctx, deps, rt)
	return ref
}
