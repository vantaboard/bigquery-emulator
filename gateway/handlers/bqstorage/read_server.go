package bqstorage

import (
	"context"
	"errors"
	"io"
	"strings"
	"sync"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// ReadServer implements the public BigQueryRead gRPC service by adapting
// requests to the engine's internal StorageRead contract and encoding row
// pages as Arrow IPC record batches.
type ReadServer struct {
	storagepb.UnimplementedBigQueryReadServer
	engine *engine.Client

	mu       sync.RWMutex
	sessions map[string]*enginepb.TableSchema
}

func (s *ReadServer) requireEngine() error {
	if s == nil || s.engine == nil || s.engine.StorageRead == nil {
		return status.Error(codes.Unavailable, "BigQuery Storage Read API requires a running engine subprocess")
	}
	return nil
}

func (s *ReadServer) rememberSession(name string, schema *enginepb.TableSchema) {
	if name == "" || schema == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.sessions == nil {
		s.sessions = make(map[string]*enginepb.TableSchema)
	}
	s.sessions[name] = schema
}

func (s *ReadServer) sessionSchema(streamName string) *enginepb.TableSchema {
	sessionName := streamName
	if i := strings.LastIndex(streamName, "/streams/"); i >= 0 {
		sessionName = streamName[:i]
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.sessions[sessionName]
}

func (s *ReadServer) CreateReadSession(
	ctx context.Context,
	req *storagepb.CreateReadSessionRequest,
) (*storagepb.ReadSession, error) {
	if err := s.requireEngine(); err != nil {
		return nil, err
	}
	dataFormat := storagepb.DataFormat_ARROW
	if rs := req.GetReadSession(); rs != nil && rs.GetDataFormat() != storagepb.DataFormat_DATA_FORMAT_UNSPECIFIED {
		dataFormat = rs.GetDataFormat()
	}
	if dataFormat == storagepb.DataFormat_AVRO {
		return nil, status.Error(
			codes.Unimplemented,
			"Avro ReadRows encoding is not implemented by the emulator storage shim",
		)
	}
	session, err := s.engine.StorageRead.CreateReadSession(ctx, engineCreateReadSessionRequest(req))
	if err != nil {
		return nil, err
	}
	s.rememberSession(session.GetName(), session.GetSchema())
	return publicReadSessionFromEngine(session, dataFormat)
}

func (s *ReadServer) ReadRows(
	req *storagepb.ReadRowsRequest,
	stream storagepb.BigQueryRead_ReadRowsServer,
) error {
	if err := s.requireEngine(); err != nil {
		return err
	}
	ctx := stream.Context()
	engineStream, err := s.engine.StorageRead.ReadRows(ctx, &enginepb.ReadRowsRequest{
		ReadStream: req.GetReadStream(),
		Offset:     req.GetOffset(),
	})
	if err != nil {
		return err
	}

	batchSchema := s.sessionSchema(req.GetReadStream())
	sentSchema := false
	for {
		page, recvErr := engineStream.Recv()
		if errors.Is(recvErr, io.EOF) {
			return nil
		}
		if recvErr != nil {
			return recvErr
		}
		if len(page.GetRows()) == 0 {
			continue
		}
		schema := batchSchema
		if schema == nil {
			schema = inferSchemaFromRow(page.GetRows()[0])
		}
		batch, err := rowsToArrowBatch(schema, page.GetRows())
		if err != nil {
			return status.Errorf(codes.Internal, "encode Arrow batch: %v", err)
		}
		resp := &storagepb.ReadRowsResponse{
			Rows: &storagepb.ReadRowsResponse_ArrowRecordBatch{
				ArrowRecordBatch: batch,
			},
			RowCount: batch.GetRowCount(),
		}
		if !sentSchema {
			arrowSchema, schemaErr := serializeArrowSchema(schema)
			if schemaErr != nil {
				return status.Errorf(codes.Internal, "encode Arrow schema: %v", schemaErr)
			}
			resp.Schema = &storagepb.ReadRowsResponse_ArrowSchema{ArrowSchema: arrowSchema}
			sentSchema = true
		}
		if err := stream.Send(resp); err != nil {
			return err
		}
	}
}

func (s *ReadServer) SplitReadStream(
	context.Context,
	*storagepb.SplitReadStreamRequest,
) (*storagepb.SplitReadStreamResponse, error) {
	return nil, status.Error(codes.Unimplemented, "SplitReadStream is not implemented by the emulator storage shim")
}

func inferSchemaFromRow(row *enginepb.DataRow) *enginepb.TableSchema {
	if row == nil {
		return &enginepb.TableSchema{}
	}
	schema := &enginepb.TableSchema{}
	for i := range row.GetCells() {
		schema.Fields = append(schema.Fields, &enginepb.FieldSchema{
			Name: columnName(i),
			Type: "STRING",
			Mode: "NULLABLE",
		})
	}
	return schema
}

func columnName(i int) string {
	const prefix = "col_"
	if i < 10 {
		return prefix + string(rune('0'+i))
	}
	return prefix + "x"
}
