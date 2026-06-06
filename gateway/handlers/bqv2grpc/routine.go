package bqv2grpc

import (
	"context"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"github.com/vantaboard/bigquery-emulator/gateway/routines"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/types/known/emptypb"
)

// RoutineServer implements google.cloud.bigquery.v2.RoutineService.
type RoutineServer struct {
	bigquerypb.UnimplementedRoutineServiceServer
	deps handlers.Dependencies
}

func newRoutineServer(deps handlers.Dependencies) *RoutineServer {
	return &RoutineServer{deps: deps}
}

func (s *RoutineServer) routineStore() *routines.Store {
	if s.deps.Routines == nil {
		s.deps.Routines = routines.NewStore()
	}
	return s.deps.Routines
}

// ListRoutines returns routines from the in-memory store.
func (s *RoutineServer) ListRoutines(
	_ context.Context,
	req *bigquerypb.ListRoutinesRequest,
) (*bigquerypb.ListRoutinesResponse, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	all := s.routineStore().List(projectID, datasetID, req.GetFilter())
	items := make([]*bigquerypb.Routine, 0, len(all))
	for _, rt := range all {
		items = append(items, routineFromREST(
			rt.RoutineReference.ProjectID,
			rt.RoutineReference.DatasetID,
			rt.RoutineReference.RoutineID,
			rt,
		))
	}
	return &bigquerypb.ListRoutinesResponse{Routines: items}, nil
}

// GetRoutine returns a routine from the in-memory store.
func (s *RoutineServer) GetRoutine(
	_ context.Context,
	req *bigquerypb.GetRoutineRequest,
) (*bigquerypb.Routine, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	routineID := req.GetRoutineId()
	rt, ok := s.routineStore().Get(projectID, datasetID, routineID)
	if !ok {
		return nil, routineNotFound(projectID, datasetID, routineID)
	}
	return routineFromREST(projectID, datasetID, routineID, rt), nil
}

// InsertRoutine registers a new routine.
func (s *RoutineServer) InsertRoutine(
	_ context.Context,
	req *bigquerypb.InsertRoutineRequest,
) (*bigquerypb.Routine, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	rt := routineToREST(req.GetRoutine())
	routineID := rt.RoutineReference.RoutineID
	if routineID == "" {
		return nil, invalidArg("Required routineReference.routineId is missing.")
	}
	if rt.DefinitionBody == "" {
		return nil, invalidArg("Required definitionBody is missing.")
	}
	if rt.RoutineType == "" {
		rt.RoutineType = "SCALAR_FUNCTION"
	}
	if rt.Language == "" {
		rt.Language = "SQL"
	}
	out := routineFromREST(projectID, datasetID, routineID, rt)
	rest := routineToREST(out)
	if !s.routineStore().Insert(rest) {
		return nil, status.Errorf(codes.AlreadyExists,
			"Already Exists: Routine %s:%s.%s", projectID, datasetID, routineID)
	}
	return out, nil
}

// UpdateRoutine replaces an existing routine.
func (s *RoutineServer) UpdateRoutine(
	_ context.Context,
	req *bigquerypb.UpdateRoutineRequest,
) (*bigquerypb.Routine, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	routineID := req.GetRoutineId()
	existing, ok := s.routineStore().Get(projectID, datasetID, routineID)
	if !ok {
		return nil, routineNotFound(projectID, datasetID, routineID)
	}
	rt := routineToREST(req.GetRoutine())
	out := routineFromREST(projectID, datasetID, routineID, rt)
	out.CreationTime = parseMillis(existing.CreationTime)
	out.Etag = routines.MintEtag()
	s.routineStore().Upsert(routineToREST(out))
	return out, nil
}

// DeleteRoutine removes a routine from the store.
func (s *RoutineServer) DeleteRoutine(
	_ context.Context,
	req *bigquerypb.DeleteRoutineRequest,
) (*emptypb.Empty, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	routineID := req.GetRoutineId()
	if !s.routineStore().Delete(projectID, datasetID, routineID) {
		return nil, routineNotFound(projectID, datasetID, routineID)
	}
	return &emptypb.Empty{}, nil
}
