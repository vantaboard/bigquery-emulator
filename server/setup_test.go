package server

import (
	"os"
	"testing"
)

func TestMain(m *testing.M) {
	// Explorer API (internal/explorerapi) needs a GCP project id when tests run without
	// BIGQUERY_EMULATOR_HOST (server.New runs before the HTTP listener, so emulator discovery
	// cannot run during init).
	if os.Getenv("BIGQUERY_EMULATOR_HOST") == "" &&
		os.Getenv("GOOGLE_CLOUD_PROJECT") == "" &&
		os.Getenv("GCLOUD_PROJECT") == "" {
		_ = os.Setenv("GOOGLE_CLOUD_PROJECT", "test-project")
	}
	os.Exit(m.Run())
}
