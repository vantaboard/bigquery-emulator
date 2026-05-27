package seed

import (
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// isAlreadyExists reports whether err is a gRPC ALREADY_EXISTS
// response from the engine. Extracted into its own helper so every
// "ensure" path in the applier shares one decision point; if the
// engine ever starts using a different code for the duplicate case
// (e.g. FAILED_PRECONDITION with a typed status detail) this is the
// one function to update.
func isAlreadyExists(err error) bool {
	if err == nil {
		return false
	}
	st, ok := status.FromError(err)
	if !ok {
		return false
	}
	return st.Code() == codes.AlreadyExists
}
