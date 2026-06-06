package bqv2grpc

import (
	"regexp"
	"strconv"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

var (
	notFoundResourceRE = regexp.MustCompile(
		`^(table|dataset) not found: ([^.]+)\.([^.]+)(?:\.([^.]+))?$`)
	alreadyExistsResourceRE = regexp.MustCompile(
		`^(table|dataset) already exists: ([^.]+)\.([^.]+)(?:\.([^.]+))?$`)
)

// grpcStatusFromEngine maps engine gRPC errors to client-facing status codes.
func grpcStatusFromEngine(err error) error {
	if err == nil {
		return nil
	}
	st, ok := status.FromError(err)
	if !ok {
		return status.Errorf(codes.Internal, "Engine RPC failed: %v", err)
	}
	switch st.Code() {
	case codes.OK:
		return nil
	case codes.NotFound, codes.AlreadyExists, codes.InvalidArgument,
		codes.FailedPrecondition, codes.PermissionDenied, codes.Unauthenticated,
		codes.Unimplemented, codes.Unavailable, codes.DeadlineExceeded,
		codes.ResourceExhausted:
		return status.Error(st.Code(), bqStyleMessage(st.Message()))
	default:
		return status.Errorf(codes.Internal, "%s", bqStyleMessage(st.Message()))
	}
}

func bqStyleMessage(msg string) string {
	if m := notFoundResourceRE.FindStringSubmatch(msg); m != nil {
		return bqStyleResourceMessage("Not found", m[1], m[2], m[3], m[4])
	}
	if m := alreadyExistsResourceRE.FindStringSubmatch(msg); m != nil {
		return bqStyleResourceMessage("Already Exists", m[1], m[2], m[3], m[4])
	}
	return msg
}

func bqStyleResourceMessage(verb, noun, project, dataset, table string) string {
	resource := project + ":" + dataset
	if table != "" {
		resource += "." + table
	}
	switch noun {
	case "table":
		return verb + ": Table " + resource
	case "dataset":
		return verb + ": Dataset " + resource
	default:
		return verb + ": " + noun + " " + resource
	}
}

func datasetNotFound(projectID, datasetID string) error {
	return status.Errorf(codes.NotFound, "Not found: Dataset %s:%s", projectID, datasetID)
}

func routineNotFound(projectID, datasetID, routineID string) error {
	return status.Errorf(codes.NotFound, "Not found: Routine %s:%s.%s", projectID, datasetID, routineID)
}

func invalidArg(msg string) error {
	return status.Error(codes.InvalidArgument, msg)
}

func unimplemented(msg string) error {
	return status.Error(codes.Unimplemented, msg)
}

func parseMillis(s string) int64 {
	n, err := strconv.ParseInt(s, 10, 64)
	if err != nil {
		return 0
	}
	return n
}

func formatMillis(n int64) string {
	if n == 0 {
		return ""
	}
	return strconv.FormatInt(n, 10)
}
