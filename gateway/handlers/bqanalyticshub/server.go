package bqanalyticshub

import (
	"context"
	"sync"

	"cloud.google.com/go/bigquery/analyticshub/apiv1/analyticshubpb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/types/known/emptypb"
)

// Server implements the Analytics Hub gRPC surface with in-memory storage.
type Server struct {
	analyticshubpb.UnimplementedAnalyticsHubServiceServer
	exchanges sync.Map // name string -> *analyticshubpb.DataExchange
	listings  sync.Map // name string -> *analyticshubpb.Listing
}

// RegisterGRPC wires AnalyticsHubService onto srv.
func RegisterGRPC(srv grpc.ServiceRegistrar) {
	if srv == nil {
		return
	}
	analyticshubpb.RegisterAnalyticsHubServiceServer(srv, &Server{})
}

// CreateDataExchange registers a data exchange. Returns AlreadyExists when
// the name is taken.
func (s *Server) CreateDataExchange(
	_ context.Context,
	req *analyticshubpb.CreateDataExchangeRequest,
) (*analyticshubpb.DataExchange, error) {
	if req == nil || req.GetParent() == "" || req.GetDataExchangeId() == "" {
		return nil, status.Error(codes.InvalidArgument, "parent and data_exchange_id are required")
	}
	name := req.GetParent() + "/dataExchanges/" + req.GetDataExchangeId()
	in := req.GetDataExchange()
	if in == nil {
		in = &analyticshubpb.DataExchange{}
	}
	out := &analyticshubpb.DataExchange{
		Name:        name,
		DisplayName: in.GetDisplayName(),
		Description: in.GetDescription(),
	}
	if _, loaded := s.exchanges.LoadOrStore(name, out); loaded {
		return nil, status.Errorf(codes.AlreadyExists, "DataExchange %s already exists", name)
	}
	return out, nil
}

// GetDataExchange returns a stored data exchange.
func (s *Server) GetDataExchange(
	_ context.Context,
	req *analyticshubpb.GetDataExchangeRequest,
) (*analyticshubpb.DataExchange, error) {
	if req == nil || req.GetName() == "" {
		return nil, status.Error(codes.InvalidArgument, "name is required")
	}
	v, ok := s.exchanges.Load(req.GetName())
	if !ok {
		return nil, status.Errorf(codes.NotFound, "DataExchange %s not found", req.GetName())
	}
	ex, _ := v.(*analyticshubpb.DataExchange)
	return ex, nil
}

// DeleteDataExchange removes a data exchange and its listings.
func (s *Server) DeleteDataExchange(
	_ context.Context,
	req *analyticshubpb.DeleteDataExchangeRequest,
) (*emptypb.Empty, error) {
	if req == nil || req.GetName() == "" {
		return nil, status.Error(codes.InvalidArgument, "name is required")
	}
	if _, ok := s.exchanges.LoadAndDelete(req.GetName()); !ok {
		return nil, status.Errorf(codes.NotFound, "DataExchange %s not found", req.GetName())
	}
	prefix := req.GetName() + "/listings/"
	s.listings.Range(func(key, _ any) bool {
		if name, ok := key.(string); ok && len(name) > len(prefix) && name[:len(prefix)] == prefix {
			s.listings.Delete(name)
		}
		return true
	})
	return &emptypb.Empty{}, nil
}

// CreateListing registers a listing under a data exchange.
func (s *Server) CreateListing(
	_ context.Context,
	req *analyticshubpb.CreateListingRequest,
) (*analyticshubpb.Listing, error) {
	if req == nil || req.GetParent() == "" || req.GetListingId() == "" {
		return nil, status.Error(codes.InvalidArgument, "parent and listing_id are required")
	}
	if _, ok := s.exchanges.Load(req.GetParent()); !ok {
		return nil, status.Errorf(codes.NotFound, "DataExchange %s not found", req.GetParent())
	}
	name := req.GetParent() + "/listings/" + req.GetListingId()
	in := req.GetListing()
	if in == nil {
		in = &analyticshubpb.Listing{}
	}
	out := &analyticshubpb.Listing{
		Name:        name,
		DisplayName: in.GetDisplayName(),
		Description: in.GetDescription(),
		Source:      in.GetSource(),
	}
	if _, loaded := s.listings.LoadOrStore(name, out); loaded {
		return nil, status.Errorf(codes.AlreadyExists, "Listing %s already exists", name)
	}
	return out, nil
}
