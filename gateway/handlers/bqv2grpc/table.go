package bqv2grpc

import (
	"context"
	"strconv"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/protobuf/types/known/emptypb"
	"google.golang.org/protobuf/types/known/wrapperspb"
)

// TableServer implements google.cloud.bigquery.v2.TableService.
type TableServer struct {
	bigquerypb.UnimplementedTableServiceServer
	deps handlers.Dependencies
}

func newTableServer(deps handlers.Dependencies) *TableServer {
	return &TableServer{deps: deps}
}

// ListTables lists tables from the engine catalog.
func (s *TableServer) ListTables(
	ctx context.Context,
	req *bigquerypb.ListTablesRequest,
) (*bigquerypb.TableList, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	if s.deps.Catalog == nil {
		return &bigquerypb.TableList{
			Kind:       "bigquery#tableList",
			Tables:     []*bigquerypb.ListFormatTable{},
			TotalItems: wrapperspb.Int32(0),
		}, nil
	}
	resp, err := s.deps.Catalog.ListTables(ctx, &enginepb.ListTablesRequest{
		Dataset: &enginepb.DatasetRef{
			ProjectId: projectID,
			DatasetId: datasetID,
		},
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	items := make([]*bigquerypb.ListFormatTable, 0, len(resp.GetTables()))
	for _, ref := range resp.GetTables() {
		labels := map[string]string{}
		tableType := tableTypeTable
		if overlay, ok := s.deps.Metadata.GetTable(
			ref.GetProjectId(), ref.GetDatasetId(), ref.GetTableId(),
		); ok {
			if overlay.Labels != nil {
				labels = map[string]string(overlay.Labels)
			}
			if overlay.Type != "" {
				tableType = overlay.Type
			}
		}
		items = append(items, listTableFromRef(
			ref.GetProjectId(), ref.GetDatasetId(), ref.GetTableId(), tableType, labels))
	}
	return &bigquerypb.TableList{
		Kind:       "bigquery#tableList",
		Tables:     items,
		TotalItems: wrapperspb.Int32(int32FromInt(len(items))),
	}, nil
}

// InsertTable registers a table in the engine catalog.
func (s *TableServer) InsertTable(
	ctx context.Context,
	req *bigquerypb.InsertTableRequest,
) (*bigquerypb.Table, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	t := tableToREST(req.GetTable())
	tableID := t.TableReference.TableID
	if tableID == "" {
		return nil, invalidArg("tableReference.tableId is required")
	}
	if s.deps.Catalog == nil {
		return nil, unimplemented("table insert requires an engine")
	}
	_, err := s.deps.Catalog.RegisterTable(ctx, &enginepb.RegisterTableRequest{
		Table: &enginepb.TableRef{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
		Schema: schemaToEngine(req.GetTable().GetSchema()),
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	if t.DefaultCollation != "" {
		t.Schema = bqtypes.ApplyDefaultCollationToStringFields(t.Schema, t.DefaultCollation)
	}
	s.deps.Metadata.PutTable(projectID, datasetID, tableID, t)
	if s.deps.Snapshots != nil {
		created := strconv.FormatInt(nowMillis(), 10)
		if ms, parseErr := strconv.ParseInt(created, 10, 64); parseErr == nil {
			s.deps.Snapshots.RecordCreation(projectID, datasetID, tableID, ms)
		}
	}
	return tableFromREST(projectID, datasetID, tableID, t), nil
}

// GetTable returns table metadata from the engine.
func (s *TableServer) GetTable(
	ctx context.Context,
	req *bigquerypb.GetTableRequest,
) (*bigquerypb.Table, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	tableID := req.GetTableId()
	if s.deps.Catalog == nil {
		return tableFromREST(projectID, datasetID, tableID, bqtypes.Table{}), nil
	}
	resp, err := s.deps.Catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{
		Table: &enginepb.TableRef{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	t := bqtypes.Table{Schema: schemaFromProto(schemaToEngineProto(resp.GetSchema()))}
	if overlay, ok := s.deps.Metadata.GetTable(projectID, datasetID, tableID); ok {
		t = applyTableOverlay(t, overlay)
	}
	if s.deps.Snapshots != nil {
		if ct, ok := s.deps.Snapshots.CreationTimeMs(projectID, datasetID, tableID); ok {
			t.CreationTime = strconv.FormatInt(ct, 10)
		}
	}
	if rowsResp, listErr := s.deps.Catalog.ListRows(ctx, &enginepb.ListRowsRequest{
		Table: &enginepb.TableRef{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
		StartIndex: 0,
		MaxResults: 0,
	}); listErr == nil {
		t.NumRows = strconv.FormatInt(rowsResp.GetTotalRows(), 10)
	} else if t.NumRows == "" {
		t.NumRows = "0"
	}
	return tableFromREST(projectID, datasetID, tableID, t), nil
}

// UpdateTable replaces table metadata in the store.
func (s *TableServer) UpdateTable(
	ctx context.Context,
	req *bigquerypb.UpdateOrPatchTableRequest,
) (*bigquerypb.Table, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	tableID := req.GetTableId()
	t := tableToREST(req.GetTable())
	s.deps.Metadata.PutTable(projectID, datasetID, tableID, t)
	return tableFromREST(projectID, datasetID, tableID, t), nil
}

// PatchTable merges table metadata in the store.
func (s *TableServer) PatchTable(
	ctx context.Context,
	req *bigquerypb.UpdateOrPatchTableRequest,
) (*bigquerypb.Table, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	tableID := req.GetTableId()
	t := tableToREST(req.GetTable())
	s.deps.Metadata.MergeTable(projectID, datasetID, tableID, t)
	if overlay, ok := s.deps.Metadata.GetTable(projectID, datasetID, tableID); ok {
		t = applyTableOverlay(t, overlay)
	}
	return tableFromREST(projectID, datasetID, tableID, t), nil
}

// DeleteTable drops a table from the engine catalog.
func (s *TableServer) DeleteTable(
	ctx context.Context,
	req *bigquerypb.DeleteTableRequest,
) (*emptypb.Empty, error) {
	projectID := req.GetProjectId()
	datasetID := req.GetDatasetId()
	tableID := req.GetTableId()
	if s.deps.Catalog == nil {
		return nil, unimplemented("table delete requires an engine")
	}
	if s.deps.Snapshots != nil {
		_ = s.deps.Snapshots.CaptureBeforeDelete(ctx, s.deps.Catalog,
			projectID, datasetID, tableID)
	}
	_, err := s.deps.Catalog.DropTable(ctx, &enginepb.DropTableRequest{
		Table: &enginepb.TableRef{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
	})
	if err != nil {
		return nil, grpcStatusFromEngine(err)
	}
	s.deps.Metadata.DeleteTable(projectID, datasetID, tableID)
	return &emptypb.Empty{}, nil
}

func schemaToEngineProto(s *enginepb.TableSchema) *bigquerypb.TableSchema {
	if s == nil {
		return nil
	}
	out := &bigquerypb.TableSchema{Fields: make([]*bigquerypb.TableFieldSchema, 0, len(s.GetFields()))}
	for _, f := range s.GetFields() {
		field := &bigquerypb.TableFieldSchema{
			Name: f.GetName(),
			Type: f.GetType(),
			Mode: f.GetMode(),
		}
		if f.GetDescription() != "" {
			field.Description = wrapperspb.String(f.GetDescription())
		}
		out.Fields = append(out.Fields, field)
	}
	return out
}

func applyTableOverlay(base, overlay bqtypes.Table) bqtypes.Table {
	if overlay.FriendlyName != "" {
		base.FriendlyName = overlay.FriendlyName
	}
	if overlay.Description != "" {
		base.Description = overlay.Description
	}
	if overlay.Type != "" {
		base.Type = overlay.Type
	}
	if overlay.Labels != nil {
		base.Labels = overlay.Labels
	}
	if overlay.Schema != nil {
		base.Schema = overlay.Schema
	}
	if overlay.DefaultCollation != "" {
		base.DefaultCollation = overlay.DefaultCollation
	}
	return base
}
