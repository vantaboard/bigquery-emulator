package server_test

import (
	"context"
	"testing"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"google.golang.org/api/option"
)

// TestJobsGetSetsDefaultStatus verifies get job never omits Status (cloud.google.com/go/bigquery Job.Wait assumes it is set).
func TestJobsGetSetsDefaultStatus(t *testing.T) {
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	project := types.NewProject("test", types.NewDataset("dataset1"))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}
	ts := bqServer.TestServer()
	defer func() {
		ts.Close()
		_ = bqServer.Stop(ctx)
	}()

	client, err := bigquery.NewClient(ctx, "test", option.WithEndpoint(ts.URL), option.WithoutAuthentication())
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()

	job, err := client.Query("CREATE TABLE dataset1.t (x INT64)").Run(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := job.Wait(ctx); err != nil {
		t.Fatal(err)
	}
	j2, err := client.JobFromID(ctx, job.ID())
	if err != nil {
		t.Fatal(err)
	}
	st, err := j2.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if st.State != bigquery.Done {
		t.Fatalf("expected Done state, got %v", st.State)
	}
}
