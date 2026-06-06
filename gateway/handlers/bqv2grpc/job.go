package bqv2grpc

import (
	"context"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// JobServer implements google.cloud.bigquery.v2.JobService (ListJobs only).
type JobServer struct {
	bigquerypb.UnimplementedJobServiceServer
	deps handlers.Dependencies
}

func newJobServer(deps handlers.Dependencies) *JobServer {
	if deps.Jobs == nil {
		deps.Jobs = jobs.NewRegistry()
	}
	return &JobServer{deps: deps}
}

// ListJobs returns jobs from the in-memory registry.
func (s *JobServer) ListJobs(
	_ context.Context,
	req *bigquerypb.ListJobsRequest,
) (*bigquerypb.JobList, error) {
	if req.GetAllUsers() {
		return nil, unimplemented(
			"jobs.list with allUsers=true is not supported; " +
				"the emulator has no auth context to scope cross-user listings.")
	}
	opts := jobs.ListOptions{
		MaxResults:      int(req.GetMaxResults().GetValue()),
		PageToken:       req.GetPageToken(),
		ParentJobID:     req.GetParentJobId(),
		MinCreationTime: int64(req.GetMinCreationTime()),
		MaxCreationTime: int64(req.GetMaxCreationTime().GetValue()),
		StateFilter:     stateFiltersFromProto(req.GetStateFilter()),
	}
	items, nextPageToken := s.deps.Jobs.ListByProject(req.GetProjectId(), opts)
	out := make([]*bigquerypb.ListFormatJob, 0, len(items))
	for _, j := range items {
		out = append(out, jobListEntryToProto(j))
	}
	resp := &bigquerypb.JobList{
		Kind: "bigquery#jobList",
		Jobs: out,
	}
	if nextPageToken != "" {
		resp.NextPageToken = nextPageToken
	}
	return resp, nil
}

func stateFiltersFromProto(filters []bigquerypb.ListJobsRequest_StateFilter) []string {
	if len(filters) == 0 {
		return nil
	}
	out := make([]string, 0, len(filters))
	for _, f := range filters {
		switch f {
		case bigquerypb.ListJobsRequest_PENDING:
			out = append(out, "pending")
		case bigquerypb.ListJobsRequest_RUNNING:
			out = append(out, "running")
		case bigquerypb.ListJobsRequest_DONE:
			out = append(out, "done")
		}
	}
	return out
}
