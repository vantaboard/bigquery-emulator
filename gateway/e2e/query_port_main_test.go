//go:build integration

package e2e

import (
	"fmt"
	"os"
	"testing"
)

func TestMain(m *testing.M) {
	if os.Getenv("BIGQUERY_EMULATOR_SKIP_QUERY_PORT") != "" {
		os.Exit(m.Run())
	}
	env, err := launchEmulatorForMain()
	if err != nil {
		fmt.Fprintf(os.Stderr, "query port: start emulator: %v\n", err)
		os.Exit(1)
	}
	sharedEmulator = env
	code := m.Run()
	sharedEmulator.tearDown()
	os.Exit(code)
}
