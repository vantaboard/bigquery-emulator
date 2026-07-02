package bqconnection

import (
	"context"
	"strings"

	"cloud.google.com/go/bigquery/connection/apiv1/connectionpb"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/types/known/emptypb"
	"google.golang.org/protobuf/types/known/fieldmaskpb"
)

// Server implements the BigQuery Connection API gRPC surface.
type Server struct {
	connectionpb.UnimplementedConnectionServiceServer
	store *Store
	cfg   *sourceconfig.Config
}

// RegisterGRPC wires ConnectionService onto srv.
func RegisterGRPC(srv grpc.ServiceRegistrar, deps handlers.Dependencies) {
	if srv == nil {
		return
	}
	st, err := OpenStore(deps.ExternalSources)
	if err != nil {
		return
	}
	connectionpb.RegisterConnectionServiceServer(srv, &Server{store: st, cfg: deps.ExternalSources})
}

// ListConnections returns connections under parent.
func (s *Server) ListConnections(
	_ context.Context,
	req *connectionpb.ListConnectionsRequest,
) (*connectionpb.ListConnectionsResponse, error) {
	if req == nil || req.GetParent() == "" {
		return nil, status.Error(codes.InvalidArgument, "parent is required")
	}
	return &connectionpb.ListConnectionsResponse{
		Connections: s.store.List(req.GetParent()),
	}, nil
}

// CreateConnection stores a connection record on disk.
func (s *Server) CreateConnection(
	_ context.Context,
	req *connectionpb.CreateConnectionRequest,
) (*connectionpb.Connection, error) {
	if req == nil || req.GetParent() == "" || req.GetConnectionId() == "" {
		return nil, status.Error(codes.InvalidArgument, "parent and connection_id are required")
	}
	name := req.GetParent() + "/connections/" + req.GetConnectionId()
	if _, ok := s.store.Get(name); ok {
		return nil, status.Errorf(codes.AlreadyExists, "Connection %s already exists", name)
	}
	conn := req.GetConnection()
	if conn == nil {
		conn = &connectionpb.Connection{}
	}
	out, err := CloneConnection(conn)
	if err != nil {
		return nil, status.Errorf(codes.Internal, "clone connection: %v", err)
	}
	out.Name = name
	out.FriendlyName = conn.GetFriendlyName()
	out.Description = AnnotateFixtureDescription(s.cfg, name, conn.GetDescription())
	copyConnectionProperties(out, conn)
	if err := s.store.Put(out); err != nil {
		return nil, status.Errorf(codes.Internal, "persist connection: %v", err)
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
	conn, ok := s.store.Get(req.GetName())
	if !ok {
		return nil, status.Errorf(codes.NotFound, "Connection %s not found", req.GetName())
	}
	return conn, nil
}

// UpdateConnection mutates an existing connection and persists it.
func (s *Server) UpdateConnection(
	_ context.Context,
	req *connectionpb.UpdateConnectionRequest,
) (*connectionpb.Connection, error) {
	if req == nil || req.GetName() == "" {
		return nil, status.Error(codes.InvalidArgument, "name is required")
	}
	existing, ok := s.store.Get(req.GetName())
	if !ok {
		return nil, status.Errorf(codes.NotFound, "Connection %s not found", req.GetName())
	}
	patch := req.GetConnection()
	if patch == nil {
		return nil, status.Error(codes.InvalidArgument, "connection is required")
	}
	out, err := CloneConnection(existing)
	if err != nil {
		return nil, status.Errorf(codes.Internal, "clone connection: %v", err)
	}
	applyConnectionUpdateMask(out, patch, req.GetUpdateMask())
	if err := s.store.Put(out); err != nil {
		return nil, status.Errorf(codes.Internal, "persist connection: %v", err)
	}
	return out, nil
}

// DeleteConnection removes a connection record.
func (s *Server) DeleteConnection(
	_ context.Context,
	req *connectionpb.DeleteConnectionRequest,
) (*emptypb.Empty, error) {
	if req == nil || req.GetName() == "" {
		return nil, status.Error(codes.InvalidArgument, "name is required")
	}
	if _, ok := s.store.Get(req.GetName()); !ok {
		return nil, status.Errorf(codes.NotFound, "Connection %s not found", req.GetName())
	}
	if err := s.store.Delete(req.GetName()); err != nil {
		return nil, status.Errorf(codes.Internal, "delete connection: %v", err)
	}
	return &emptypb.Empty{}, nil
}

func applyConnectionUpdateMask(dst, patch *connectionpb.Connection, mask *fieldmaskpb.FieldMask) {
	if dst == nil || patch == nil {
		return
	}
	paths := mask.GetPaths()
	if len(paths) == 0 {
		if patch.FriendlyName != "" {
			dst.FriendlyName = patch.FriendlyName
		}
		if patch.Description != "" {
			dst.Description = patch.Description
		}
		copyConnectionProperties(dst, patch)
		return
	}
	for _, p := range paths {
		switch strings.TrimSpace(strings.ToLower(p)) {
		case "friendly_name", "friendlyname":
			dst.FriendlyName = patch.FriendlyName
		case "description":
			dst.Description = patch.Description
		case "cloud_sql", "cloudsql":
			if patch.GetCloudSql() != nil {
				dst.Properties = &connectionpb.Connection_CloudSql{CloudSql: patch.GetCloudSql()}
			}
		case "cloud_spanner", "cloudspanner":
			if patch.GetCloudSpanner() != nil {
				dst.Properties = &connectionpb.Connection_CloudSpanner{CloudSpanner: patch.GetCloudSpanner()}
			}
		case "aws":
			if patch.GetAws() != nil {
				dst.Properties = &connectionpb.Connection_Aws{Aws: patch.GetAws()}
			}
		case "azure":
			if patch.GetAzure() != nil {
				dst.Properties = &connectionpb.Connection_Azure{Azure: patch.GetAzure()}
			}
		case "cloud_resource", "cloudresource":
			if patch.GetCloudResource() != nil {
				dst.Properties = &connectionpb.Connection_CloudResource{CloudResource: patch.GetCloudResource()}
			}
		case "spark":
			if patch.GetSpark() != nil {
				dst.Properties = &connectionpb.Connection_Spark{Spark: patch.GetSpark()}
			}
		}
	}
}

func copyConnectionProperties(dst, src *connectionpb.Connection) {
	if dst == nil || src == nil {
		return
	}
	switch p := src.Properties.(type) {
	case *connectionpb.Connection_CloudSql:
		dst.Properties = &connectionpb.Connection_CloudSql{CloudSql: p.CloudSql}
	case *connectionpb.Connection_Aws:
		dst.Properties = &connectionpb.Connection_Aws{Aws: p.Aws}
	case *connectionpb.Connection_Azure:
		dst.Properties = &connectionpb.Connection_Azure{Azure: p.Azure}
	case *connectionpb.Connection_CloudSpanner:
		dst.Properties = &connectionpb.Connection_CloudSpanner{CloudSpanner: p.CloudSpanner}
	case *connectionpb.Connection_CloudResource:
		dst.Properties = &connectionpb.Connection_CloudResource{CloudResource: p.CloudResource}
	case *connectionpb.Connection_Spark:
		dst.Properties = &connectionpb.Connection_Spark{Spark: p.Spark}
	case *connectionpb.Connection_SalesforceDataCloud:
		dst.Properties = &connectionpb.Connection_SalesforceDataCloud{SalesforceDataCloud: p.SalesforceDataCloud}
	}
}
