package handlers

import (
	"context"

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
	if desc.GetDdlSql() != "" {
		if parsed, ok := routines.ParseCreateRoutineDDL(
			ref.GetProjectId(), ref.GetDatasetId(), desc.GetDdlSql()); ok {
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
		}
	}
	return rt
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
