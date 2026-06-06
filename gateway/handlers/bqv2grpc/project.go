package bqv2grpc

import (
	"context"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
)

// ProjectServer implements google.cloud.bigquery.v2.ProjectService.
type ProjectServer struct {
	bigquerypb.UnimplementedProjectServiceServer
}

func newProjectServer(_ handlers.Dependencies) *ProjectServer {
	return &ProjectServer{}
}

// GetServiceAccount returns the emulator's synthetic service account email.
func (s *ProjectServer) GetServiceAccount(
	_ context.Context,
	req *bigquerypb.GetServiceAccountRequest,
) (*bigquerypb.GetServiceAccountResponse, error) {
	projectID := req.GetProjectId()
	if projectID == "" {
		projectID = "test-project"
	}
	return &bigquerypb.GetServiceAccountResponse{
		Kind:  "bigquery#getServiceAccountResponse",
		Email: "bigquery-emulator@" + projectID + ".iam.gserviceaccount.com",
	}, nil
}
