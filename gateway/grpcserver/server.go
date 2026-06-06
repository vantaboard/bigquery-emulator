// Package grpcserver hosts the public BigQuery Storage gRPC surface on the
// gateway process. Official client libraries dial
// google.cloud.bigquery.storage.v1.BigQueryRead / BigQueryWrite; the shim
// in gateway/handlers/bqstorage adapts those RPCs to the engine's internal
// bigquery_emulator.v1.StorageRead / StorageWrite contracts.
package grpcserver

import (
	"errors"
	"fmt"
	"net"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

// Server wraps the public Storage gRPC listener the gateway owns.
type Server struct {
	srv *grpc.Server
	lis net.Listener
}

// Start binds address and registers every public gRPC surface the gateway
// exposes (Storage, Connection, Reservation, Analytics Hub, BigQuery v2).
// eng may be nil in gateway-only mode; storage RPCs then return UNAVAILABLE.
func Start(address string, eng *engine.Client, deps handlers.Dependencies) (*Server, error) {
	if address == "" {
		return nil, errors.New("grpcserver: empty address")
	}
	lis, err := net.Listen("tcp", address)
	if err != nil {
		return nil, fmt.Errorf("grpcserver: listen %s: %w", address, err)
	}
	srv := grpc.NewServer(grpc.Creds(insecure.NewCredentials()))
	RegisterAll(srv, eng, deps)
	return &Server{srv: srv, lis: lis}, nil
}

// Serve blocks until the server stops or the listener fails.
func (s *Server) Serve() error {
	if s == nil || s.srv == nil || s.lis == nil {
		return errors.New("grpcserver: server not initialized")
	}
	return s.srv.Serve(s.lis)
}

// Stop gracefully shuts down the gRPC server.
func (s *Server) Stop() {
	if s == nil || s.srv == nil {
		return
	}
	s.srv.GracefulStop()
}

// Close stops the server and closes the listener.
func (s *Server) Close() error {
	if s == nil {
		return nil
	}
	s.Stop()
	if s.lis != nil {
		return s.lis.Close()
	}
	return nil
}
