package grpcserver

import (
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers/bqanalyticshub"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers/bqconnection"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers/bqreservation"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers/bqstorage"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers/bqv2grpc"
	"google.golang.org/grpc"
)

// RegisterAll wires every public gRPC surface the gateway exposes on the
// storage listener: BigQuery Storage Read/Write, Connection, Reservation,
// Analytics Hub, and BigQuery v2 resource services.
func RegisterAll(srv grpc.ServiceRegistrar, eng *engine.Client, deps handlers.Dependencies) {
	if srv == nil {
		return
	}
	bqstorage.RegisterGRPC(srv, eng)
	bqconnection.RegisterGRPC(srv, deps)
	bqreservation.RegisterGRPC(srv)
	bqanalyticshub.RegisterGRPC(srv)
	bqv2grpc.RegisterGRPC(srv, deps)
}
