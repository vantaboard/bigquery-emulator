package bqv2grpc

import (
	"context"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/protobuf/types/known/emptypb"
)

// DatasetServer implements google.cloud.bigquery.v2.DatasetService.
type DatasetServer struct {
	bigquerypb.UnimplementedDatasetServiceServer
	deps handlers.Dependencies
}

func newDatasetServer(deps handlers.Dependencies) *DatasetServer {
	return &DatasetServer{deps: deps}
}

// ListDatasets lists datasets from the engine catalog.
func (s *DatasetServer) ListDatasets(
	ctx context.Context,
	req *bigquerypb.ListDatasetsRequest,
) (*bigquerypb.DatasetList, error) {
	projectID := req.GetProjectId()
	if s.deps.Catalog == nil {
		return &bigquerypb.DatasetList{
			Kind:     "bigquery#datasetList",
			Datasets: []*bigquerypb.ListFormatDataset{},
		}, nil
	}
	resp, err := s.deps.Catalog.ListDatasets(ctx, &enginepb.ListDatasetsRequest{
		ProjectId: projectID,
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	items := make([]*bigquerypb.ListFormatDataset, 0, len(resp.GetDatasets()))
	for _, ref := range resp.GetDatasets() {
		labels := map[string]string{}
		if overlay, ok := s.deps.Metadata.GetDataset(
			ref.GetProjectId(), ref.GetDatasetId(),
		); ok && overlay.Labels != nil {
			labels = map[string]string(overlay.Labels)
		}
		items = append(items, listDatasetFromRef(
			ref.GetProjectId(), ref.GetDatasetId(), labels))
	}
	return &bigquerypb.DatasetList{
		Kind:     "bigquery#datasetList",
		Datasets: items,
	}, nil
}

// InsertDataset registers a dataset in the engine catalog.
func (s *DatasetServer) InsertDataset(
	ctx context.Context,
	req *bigquerypb.InsertDatasetRequest,
) (*bigquerypb.Dataset, error) {
	projectID := req.GetProjectId()
	ds := datasetToREST(req.GetDataset())
	datasetID := ds.DatasetReference.DatasetID
	if datasetID == "" {
		return nil, invalidArg("datasetReference.datasetId is required")
	}
	if s.deps.Catalog == nil {
		return nil, unimplemented("dataset insert requires an engine")
	}
	location := ds.Location
	if location == "" {
		location = "US"
	}
	_, err := s.deps.Catalog.RegisterDataset(ctx, &enginepb.RegisterDatasetRequest{
		Dataset: &enginepb.DatasetRef{
			ProjectId: projectID,
			DatasetId: datasetID,
		},
		Location: location,
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	s.deps.Metadata.PutDataset(projectID, datasetID, ds)
	return datasetFromREST(projectID, datasetID, ds), nil
}

// GetDataset returns dataset metadata.
func (s *DatasetServer) GetDataset(
	ctx context.Context,
	req *bigquerypb.GetDatasetRequest,
) (*bigquerypb.Dataset, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	exists, err := catalogDatasetExists(ctx, s.deps, projectID, datasetID)
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	if !exists {
		return nil, datasetNotFound(projectID, datasetID)
	}
	ds := datasetToREST(&bigquerypb.Dataset{})
	if overlay, ok := s.deps.Metadata.GetDataset(projectID, datasetID); ok {
		ds = overlay
	}
	return datasetFromREST(projectID, datasetID, ds), nil
}

// UpdateDataset replaces dataset metadata in the store.
func (s *DatasetServer) UpdateDataset(
	ctx context.Context,
	req *bigquerypb.UpdateOrPatchDatasetRequest,
) (*bigquerypb.Dataset, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	exists, err := catalogDatasetExists(ctx, s.deps, projectID, datasetID)
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	if !exists {
		return nil, datasetNotFound(projectID, datasetID)
	}
	ds := datasetToREST(req.GetDataset())
	s.deps.Metadata.PutDataset(projectID, datasetID, ds)
	return datasetFromREST(projectID, datasetID, ds), nil
}

// PatchDataset merges dataset metadata in the store.
func (s *DatasetServer) PatchDataset(
	ctx context.Context,
	req *bigquerypb.UpdateOrPatchDatasetRequest,
) (*bigquerypb.Dataset, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	exists, err := catalogDatasetExists(ctx, s.deps, projectID, datasetID)
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	if !exists {
		return nil, datasetNotFound(projectID, datasetID)
	}
	ds := datasetToREST(req.GetDataset())
	s.deps.Metadata.MergeDataset(projectID, datasetID, ds)
	if overlay, ok := s.deps.Metadata.GetDataset(projectID, datasetID); ok {
		ds = overlay
	}
	return datasetFromREST(projectID, datasetID, ds), nil
}

// DeleteDataset drops a dataset from the engine catalog.
func (s *DatasetServer) DeleteDataset(
	ctx context.Context,
	req *bigquerypb.DeleteDatasetRequest,
) (*emptypb.Empty, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	if s.deps.Catalog == nil {
		return nil, unimplemented("dataset delete requires an engine")
	}
	_, err := s.deps.Catalog.DropDataset(ctx, &enginepb.DropDatasetRequest{
		Dataset: &enginepb.DatasetRef{
			ProjectId: projectID,
			DatasetId: datasetID,
		},
		DeleteContents: req.GetDeleteContents(),
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	s.deps.Metadata.DeleteDataset(projectID, datasetID)
	if req.GetDeleteContents() {
		s.deps.Metadata.DeleteTablesInDataset(projectID, datasetID)
	}
	return &emptypb.Empty{}, nil
}

func catalogDatasetExists(
	ctx context.Context,
	deps handlers.Dependencies,
	projectID, datasetID string,
) (bool, error) {
	if deps.Catalog == nil {
		return true, nil
	}
	resp, err := deps.Catalog.ListDatasets(ctx, &enginepb.ListDatasetsRequest{
		ProjectId: projectID,
	})
	if err != nil {
		return false, err
	}
	for _, ref := range resp.GetDatasets() {
		if ref.GetDatasetId() == datasetID {
			return true, nil
		}
	}
	return false, nil
}
