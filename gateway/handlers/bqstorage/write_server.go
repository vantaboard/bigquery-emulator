// Package bqstorage is the public BigQuery Storage gRPC shim. It registers
// google.cloud.bigquery.storage.v1.BigQueryRead / BigQueryWrite on the
// gateway listener and adapts RPCs to the engine's internal
// bigquery_emulator.v1.StorageRead / StorageWrite contracts.
package bqstorage

import (
	"context"
	"errors"
	"io"
	"strings"
	"time"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/types/descriptorpb"
	"google.golang.org/protobuf/types/known/timestamppb"
	"google.golang.org/protobuf/types/known/wrapperspb"
)

// WriteServer implements the public BigQueryWrite gRPC service.
type WriteServer struct {
	storagepb.UnimplementedBigQueryWriteServer
	engine *engine.Client
}

func (s *WriteServer) requireEngine() error {
	if s == nil || s.engine == nil || s.engine.StorageWrite == nil {
		return status.Error(codes.Unavailable, "BigQuery Storage Write API requires a running engine subprocess")
	}
	return nil
}

func (s *WriteServer) CreateWriteStream(
	ctx context.Context,
	req *storagepb.CreateWriteStreamRequest,
) (*storagepb.WriteStream, error) {
	if err := s.requireEngine(); err != nil {
		return nil, err
	}
	streamType := storagepb.WriteStream_COMMITTED
	if ws := req.GetWriteStream(); ws != nil &&
		ws.GetType() != storagepb.WriteStream_TYPE_UNSPECIFIED {
		streamType = ws.GetType()
	}
	if streamType == storagepb.WriteStream_COMMITTED ||
		streamType == storagepb.WriteStream_TYPE_UNSPECIFIED {
		return s.defaultWriteStream(ctx, req.GetParent())
	}
	stream, err := s.engine.StorageWrite.CreateWriteStream(ctx, &enginepb.CreateWriteStreamRequest{
		Parent:      req.GetParent(),
		WriteStream: engineWriteStreamFromPublic(req.GetWriteStream()),
	})
	if err != nil {
		return nil, err
	}
	return publicWriteStreamFromEngine(stream), nil
}

func defaultWriteStreamName(parent string) string {
	return strings.TrimRight(parent, "/") + "/streams/_default"
}

func (s *WriteServer) defaultWriteStream(
	ctx context.Context,
	parent string,
) (*storagepb.WriteStream, error) {
	name := defaultWriteStreamName(parent)
	existing, err := s.engine.StorageWrite.GetWriteStream(ctx, &enginepb.GetWriteStreamRequest{
		Name: name,
	})
	if err == nil {
		out := publicWriteStreamFromEngine(existing)
		out.Name = name
		out.Type = storagepb.WriteStream_COMMITTED
		return out, nil
	}
	// Mint schema metadata via CreateWriteStream; the engine registers the
	// reserved _default stream lazily on the first AppendRows.
	probe, err := s.engine.StorageWrite.CreateWriteStream(ctx, &enginepb.CreateWriteStreamRequest{
		Parent: parent,
		WriteStream: &enginepb.WriteStream{
			Type: enginepb.WriteStream_COMMITTED,
		},
	})
	if err != nil {
		return nil, err
	}
	out := publicWriteStreamFromEngine(probe)
	out.Name = name
	out.Type = storagepb.WriteStream_COMMITTED
	return out, nil
}

func (s *WriteServer) AppendRows(stream storagepb.BigQueryWrite_AppendRowsServer) error {
	if err := s.requireEngine(); err != nil {
		return err
	}
	ctx := stream.Context()
	engineStream, err := s.engine.StorageWrite.AppendRows(ctx)
	if err != nil {
		return err
	}

	var cachedProtoDesc *descriptorpb.DescriptorProto
	for {
		req, recvErr := stream.Recv()
		if errors.Is(recvErr, io.EOF) {
			return nil
		}
		if recvErr != nil {
			return recvErr
		}
		engineReq, convErr := s.publicAppendRequestToEngine(ctx, req, &cachedProtoDesc)
		if convErr != nil {
			return status.Errorf(codes.InvalidArgument, "decode AppendRows: %v", convErr)
		}
		if err := engineStream.Send(engineReq); err != nil {
			return err
		}
		engineResp, err := engineStream.Recv()
		if err != nil {
			return err
		}
		if err := stream.Send(publicAppendResponseFromEngine(req.GetWriteStream(), engineResp)); err != nil {
			return err
		}
	}
}

func (s *WriteServer) GetWriteStream(
	ctx context.Context,
	req *storagepb.GetWriteStreamRequest,
) (*storagepb.WriteStream, error) {
	if err := s.requireEngine(); err != nil {
		return nil, err
	}
	stream, err := s.engine.StorageWrite.GetWriteStream(ctx, &enginepb.GetWriteStreamRequest{
		Name: req.GetName(),
	})
	if err == nil {
		return publicWriteStreamFromEngine(stream), nil
	}
	if before, ok := strings.CutSuffix(req.GetName(), "/streams/_default"); ok {
		parent := before
		return s.defaultWriteStream(ctx, parent)
	}
	return nil, err
}

func (s *WriteServer) FinalizeWriteStream(
	ctx context.Context,
	req *storagepb.FinalizeWriteStreamRequest,
) (*storagepb.FinalizeWriteStreamResponse, error) {
	if err := s.requireEngine(); err != nil {
		return nil, err
	}
	resp, err := s.engine.StorageWrite.FinalizeWriteStream(ctx, &enginepb.FinalizeWriteStreamRequest{
		Name: req.GetName(),
	})
	if err != nil {
		return nil, err
	}
	return &storagepb.FinalizeWriteStreamResponse{
		RowCount: resp.GetRowCount(),
	}, nil
}

func (s *WriteServer) BatchCommitWriteStreams(
	ctx context.Context,
	req *storagepb.BatchCommitWriteStreamsRequest,
) (*storagepb.BatchCommitWriteStreamsResponse, error) {
	if err := s.requireEngine(); err != nil {
		return nil, err
	}
	resp, err := s.engine.StorageWrite.BatchCommitWriteStreams(
		ctx,
		&enginepb.BatchCommitWriteStreamsRequest{
			Parent:       req.GetParent(),
			WriteStreams: append([]string(nil), req.GetWriteStreams()...),
		},
	)
	if err != nil {
		return nil, err
	}
	out := &storagepb.BatchCommitWriteStreamsResponse{}
	if ts := resp.GetCommitTime(); ts != "" {
		if t, parseErr := time.Parse(time.RFC3339, ts); parseErr == nil {
			out.CommitTime = timestamppb.New(t)
		}
	}
	return out, nil
}

func (s *WriteServer) FlushRows(
	ctx context.Context,
	req *storagepb.FlushRowsRequest,
) (*storagepb.FlushRowsResponse, error) {
	if err := s.requireEngine(); err != nil {
		return nil, err
	}
	offset := int64(0)
	if req.GetOffset() != nil {
		offset = req.GetOffset().GetValue()
	}
	resp, err := s.engine.StorageWrite.FlushRows(ctx, &enginepb.FlushRowsRequest{
		WriteStream: req.GetWriteStream(),
		Offset:      offset,
	})
	if err != nil {
		return nil, err
	}
	return &storagepb.FlushRowsResponse{Offset: resp.GetOffset()}, nil
}

func (s *WriteServer) publicAppendRequestToEngine(
	ctx context.Context,
	req *storagepb.AppendRowsRequest,
	cachedProtoDesc **descriptorpb.DescriptorProto,
) (*enginepb.AppendRowsRequest, error) {
	if req == nil {
		return nil, status.Error(codes.InvalidArgument, "nil AppendRowsRequest")
	}
	out := &enginepb.AppendRowsRequest{
		WriteStream: req.GetWriteStream(),
		TraceId:     req.GetTraceId(),
	}
	if req.GetOffset() != nil {
		out.Offset = req.GetOffset().GetValue()
	}
	switch rows := req.GetRows().(type) {
	case *storagepb.AppendRowsRequest_ProtoRows:
		engineRows, err := protoDataToEngineRows(
			ctx,
			s.engine,
			req.GetWriteStream(),
			rows.ProtoRows,
			cachedProtoDesc,
		)
		if err != nil {
			return nil, err
		}
		out.ProtoRows = &enginepb.AppendRowsRequest_ProtoData{Rows: engineRows}
	case *storagepb.AppendRowsRequest_ArrowRows:
		return nil, status.Error(
			codes.Unimplemented,
			"Arrow AppendRows is not implemented by the emulator storage shim",
		)
	default:
		return out, nil
	}
	return out, nil
}

func publicAppendResponseFromEngine(
	writeStream string,
	in *enginepb.AppendRowsResponse,
) *storagepb.AppendRowsResponse {
	if in == nil {
		return &storagepb.AppendRowsResponse{WriteStream: writeStream}
	}
	out := &storagepb.AppendRowsResponse{WriteStream: writeStream}
	if msg := in.GetErrorMessage(); msg != "" {
		out.Response = &storagepb.AppendRowsResponse_Error{
			Error: status.New(codes.InvalidArgument, msg).Proto(),
		}
		return out
	}
	result := &storagepb.AppendRowsResponse_AppendResult{}
	if ar := in.GetAppendResult(); ar != nil {
		result.Offset = wrapperspb.Int64(ar.GetOffset())
	}
	out.Response = &storagepb.AppendRowsResponse_AppendResult_{
		AppendResult: result,
	}
	return out
}
