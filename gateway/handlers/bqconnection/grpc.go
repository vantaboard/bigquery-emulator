package bqconnection

import (
	"context"
	"sync"

	"cloud.google.com/go/bigquery/connection/apiv1/connectionpb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// Server implements the BigQuery Connection API gRPC surface.
type Server struct {
	connectionpb.UnimplementedConnectionServiceServer
	store sync.Map // name string -> *connectionpb.Connection
}

// RegisterGRPC wires ConnectionService onto srv.
func RegisterGRPC(srv grpc.ServiceRegistrar, _ handlers.Dependencies) {
	if srv == nil {
		return
	}
	connectionpb.RegisterConnectionServiceServer(srv, &Server{})
}

// ListConnections returns an empty page so client startup probes succeed.
func (s *Server) ListConnections(
	_ context.Context,
	_ *connectionpb.ListConnectionsRequest,
) (*connectionpb.ListConnectionsResponse, error) {
	return &connectionpb.ListConnectionsResponse{
		Connections: []*connectionpb.Connection{},
	}, nil
}

// CreateConnection stores a connection record in memory.
func (s *Server) CreateConnection(
	_ context.Context,
	req *connectionpb.CreateConnectionRequest,
) (*connectionpb.Connection, error) {
	if req == nil || req.GetParent() == "" || req.GetConnectionId() == "" {
		return nil, status.Error(codes.InvalidArgument, "parent and connection_id are required")
	}
	name := req.GetParent() + "/connections/" + req.GetConnectionId()
	conn := req.GetConnection()
	if conn == nil {
		conn = &connectionpb.Connection{}
	}
	out := &connectionpb.Connection{
		Name:         name,
		FriendlyName: conn.GetFriendlyName(),
		Description:  conn.GetDescription(),
	}
	if _, loaded := s.store.LoadOrStore(name, out); loaded {
		return nil, status.Errorf(codes.AlreadyExists, "Connection %s already exists", name)
	}
	return out, nil
}

// GetConnection returns a previously created connection.
func (s *Server) GetConnection(
	_ context.Context,
	req *connectionpb.GetConnectionRequest,
) (*connectionpb.Connection, error) {
	if req == nil || req.GetName() == "" {
		return nil, status.Error(codes.InvalidArgument, "name is required")
	}
	v, ok := s.store.Load(req.GetName())
	if !ok {
		return nil, status.Errorf(codes.NotFound, "Connection %s not found", req.GetName())
	}
	conn, _ := v.(*connectionpb.Connection)
	return conn, nil
}
