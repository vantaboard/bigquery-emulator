package server

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/internal/execution"
)

func TestNew_DuckDBWithoutRegisteredDriver(t *testing.T) {
	if duckDriverRegistered() {
		t.Skip("googlesqlengineduck is registered (build with duckdb tags); skipping missing-driver check")
	}
	_, err := New(TempStorage, WithExecutionBackend(execution.BackendDuckDB))
	if err == nil {
		t.Fatal("expected error when duckdb backend requested but driver not registered")
	}
}
