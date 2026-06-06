package bqstorage

import (
	"context"
	"errors"
	"io"
	"strconv"
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
type readSessionState struct {
	schema     *enginepb.TableSchema
	dataFormat storagepb.DataFormat
}

type ReadServer struct {
	storagepb.UnimplementedBigQueryReadServer
	engine *engine.Client

	mu       sync.RWMutex
	sessions map[string]*readSessionState
}

func (s *ReadServer) requireEngine() error {
	if s == nil || s.engine == nil || s.engine.StorageRead == nil {
		return status.Error(codes.Unavailable, "BigQuery Storage Read API requires a running engine subprocess")
	}
	return nil
}

func (s *ReadServer) rememberSession(
	name string,
	schema *enginepb.TableSchema,
	dataFormat storagepb.DataFormat,
) {
	if name == "" || schema == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.sessions == nil {
		s.sessions = make(map[string]*readSessionState)
	}
	s.sessions[name] = &readSessionState{
		schema:     schema,
		dataFormat: dataFormat,
	}
}

func (s *ReadServer) sessionState(streamName string) *readSessionState {
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
	session, err := s.engine.StorageRead.CreateReadSession(ctx, engineCreateReadSessionRequest(req))
	if err != nil {
		return nil, err
	}
	s.rememberSession(session.GetName(), session.GetSchema(), dataFormat)
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

	state := s.sessionState(req.GetReadStream())
	dataFormat := storagepb.DataFormat_ARROW
	if state != nil && state.dataFormat != storagepb.DataFormat_DATA_FORMAT_UNSPECIFIED {
		dataFormat = state.dataFormat
	}
	return s.pumpEngineReadRows(engineStream, stream, state, dataFormat)
}

func (s *ReadServer) pumpEngineReadRows(
	engineStream enginepb.StorageRead_ReadRowsClient,
	stream storagepb.BigQueryRead_ReadRowsServer,
	state *readSessionState,
	dataFormat storagepb.DataFormat,
) error {
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
		schema := (*enginepb.TableSchema)(nil)
		if state != nil {
			schema = state.schema
		}
		if schema == nil {
			schema = inferSchemaFromRow(page.GetRows()[0])
		}
		resp, err := readRowsResponseForFormat(dataFormat, schema, page.GetRows())
		if err != nil {
			return status.Errorf(codes.Internal, "encode ReadRows batch: %v", err)
		}
		if !sentSchema {
			if err := attachReadRowsSchema(resp, dataFormat, schema); err != nil {
				return err
			}
			sentSchema = true
		}
		if err := stream.Send(resp); err != nil {
			return err
		}
	}
}

func attachReadRowsSchema(
	resp *storagepb.ReadRowsResponse,
	dataFormat storagepb.DataFormat,
	schema *enginepb.TableSchema,
) error {
	switch dataFormat {
	case storagepb.DataFormat_AVRO:
		avroSchema, schemaErr := serializeAvroSchema(schema)
		if schemaErr != nil {
			return status.Errorf(codes.Internal, "encode Avro schema: %v", schemaErr)
		}
		resp.Schema = &storagepb.ReadRowsResponse_AvroSchema{AvroSchema: avroSchema}
	default:
		arrowSchema, schemaErr := serializeArrowSchema(schema)
		if schemaErr != nil {
			return status.Errorf(codes.Internal, "encode Arrow schema: %v", schemaErr)
		}
		resp.Schema = &storagepb.ReadRowsResponse_ArrowSchema{ArrowSchema: arrowSchema}
	}
	return nil
}

func readRowsResponseForFormat(
	dataFormat storagepb.DataFormat,
	schema *enginepb.TableSchema,
	rows []*enginepb.DataRow,
) (*storagepb.ReadRowsResponse, error) {
	switch dataFormat {
	case storagepb.DataFormat_AVRO:
		batch, err := rowsToAvroBatch(schema, rows)
		if err != nil {
			return nil, err
		}
		rowCount := int64(len(rows))
		return &storagepb.ReadRowsResponse{
			Rows:     &storagepb.ReadRowsResponse_AvroRows{AvroRows: batch},
			RowCount: rowCount,
		}, nil
	default:
		batch, err := rowsToArrowBatch(schema, rows)
		if err != nil {
			return nil, err
		}
		rowCount := int64(len(rows))
		return &storagepb.ReadRowsResponse{
			Rows: &storagepb.ReadRowsResponse_ArrowRecordBatch{
				ArrowRecordBatch: batch,
			},
			RowCount: rowCount,
		}, nil
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
			Type: bqTypeSTRING,
			Mode: bqModeNullable,
		})
	}
	return schema
}

func columnName(i int) string {
	return "col_" + strconv.Itoa(i)
}
