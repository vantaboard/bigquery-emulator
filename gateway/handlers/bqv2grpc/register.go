package bqv2grpc

import (
	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
)

// RegisterGRPC wires all BigQuery v2 gRPC services onto srv. Unimplemented
// methods on each embedded server return UNIMPLEMENTED automatically.
func RegisterGRPC(srv grpc.ServiceRegistrar, deps handlers.Dependencies) {
	if srv == nil {
		return
	}
	bigquerypb.RegisterDatasetServiceServer(srv, newDatasetServer(deps))
	bigquerypb.RegisterTableServiceServer(srv, newTableServer(deps))
	bigquerypb.RegisterJobServiceServer(srv, newJobServer(deps))
	bigquerypb.RegisterProjectServiceServer(srv, newProjectServer(deps))
	bigquerypb.RegisterRoutineServiceServer(srv, newRoutineServer(deps))
	bigquerypb.RegisterModelServiceServer(srv, &ModelServer{})
	bigquerypb.RegisterRowAccessPolicyServiceServer(srv, &RowAccessPolicyServer{})
}

// ModelServer stubs google.cloud.bigquery.v2.ModelService.
type ModelServer struct {
	bigquerypb.UnimplementedModelServiceServer
}

// RowAccessPolicyServer stubs google.cloud.bigquery.v2.RowAccessPolicyService.
type RowAccessPolicyServer struct {
	bigquerypb.UnimplementedRowAccessPolicyServiceServer
}
