//go:build duckdb && duckdb_use_lib

package server_test

import (
	"context"
	"testing"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/internal/execution"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	_ "github.com/vantaboard/go-googlesql-engine"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
)

func TestServer_DuckDBExecutionBackendSmoke(t *testing.T) {
	t.Parallel()
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage, server.WithExecutionBackend(execution.BackendDuckDB))
	if err != nil {
		t.Fatal(err)
	}
	defer func() {
		_ = bqServer.Stop(ctx)
	}()
	project := types.NewProject("duck_smoke_proj")
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	testServer := bqServer.TestServer()
	defer testServer.Close()

	client, err := bigquery.NewClient(
		ctx,
		project.ID,
		option.WithEndpoint(testServer.URL),
		option.WithoutAuthentication(),
	)
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()

	q := client.Query("SELECT 1 AS n")
	it, err := q.Read(ctx)
	if err != nil {
		t.Fatal(err)
	}
	var row []bigquery.Value
	if err := it.Next(&row); err != nil {
		t.Fatal(err)
	}
	if err := it.Next(&row); err != iterator.Done {
		t.Fatalf("expected one row, got extra: %v", err)
	}
	if len(row) != 1 {
		t.Fatalf("row: %v", row)
	}
}
