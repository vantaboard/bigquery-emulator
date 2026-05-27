// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package migration holds emulator integration tests for the BigQuery Migration API.
package migration

import (
	"context"
	"errors"
	"os"
	"testing"

	apimigration "cloud.google.com/go/bigquery/migration/apiv2alpha"
	migrationpb "cloud.google.com/go/bigquery/migration/apiv2alpha/migrationpb"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/bqopts"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/internal/testutil"
	"google.golang.org/api/iterator"
)

func TestEmulatorMigrationWorkflowREST(t *testing.T) {
	if os.Getenv("BIGQUERY_MIGRATION_EMULATOR_HOST") == "" && os.Getenv("BIGQUERY_EMULATOR_HOST") == "" {
		t.Skip("set BIGQUERY_EMULATOR_HOST or BIGQUERY_MIGRATION_EMULATOR_HOST for migration emulator tests")
	}
	tc := testutil.SystemTest(t)
	ctx := context.Background()

	client, err := apimigration.NewRESTClient(ctx, bqopts.MigrationRESTClientOptions()...)
	if err != nil {
		t.Fatalf("NewRESTClient: %v", err)
	}
	defer func() { _ = client.Close() }()

	parent := "projects/" + tc.ProjectID + "/locations/us"
	created, err := client.CreateMigrationWorkflow(ctx, &migrationpb.CreateMigrationWorkflowRequest{
		Parent: parent,
		MigrationWorkflow: &migrationpb.MigrationWorkflow{
			DisplayName: "golang-samples-migration-emulator",
		},
	})
	if err != nil {
		t.Fatalf("CreateMigrationWorkflow: %v", err)
	}
	if created.GetName() == "" {
		t.Fatal("expected non-empty workflow name")
	}
	if got := created.GetState(); got != migrationpb.MigrationWorkflow_DRAFT {
		t.Fatalf("state: got %v want DRAFT", got)
	}

	if err := client.StartMigrationWorkflow(ctx, &migrationpb.StartMigrationWorkflowRequest{Name: created.GetName()}); err != nil {
		t.Fatalf("StartMigrationWorkflow: %v", err)
	}
	got, err := client.GetMigrationWorkflow(ctx, &migrationpb.GetMigrationWorkflowRequest{Name: created.GetName()})
	if err != nil {
		t.Fatalf("GetMigrationWorkflow: %v", err)
	}
	if got.GetState() != migrationpb.MigrationWorkflow_RUNNING {
		t.Fatalf("after start: state %v, want RUNNING", got.GetState())
	}

	it := client.ListMigrationWorkflows(ctx, &migrationpb.ListMigrationWorkflowsRequest{Parent: parent, PageSize: 50})
	found := false
	for {
		wf, err := it.Next()
		if errors.Is(err, iterator.Done) {
			break
		}
		if err != nil {
			t.Fatalf("ListMigrationWorkflows: %v", err)
		}
		if wf.GetName() == created.GetName() {
			found = true
		}
	}
	if !found {
		t.Fatal("list workflows: created workflow not in page")
	}

	if err := client.DeleteMigrationWorkflow(ctx, &migrationpb.DeleteMigrationWorkflowRequest{Name: created.GetName()}); err != nil {
		t.Fatalf("DeleteMigrationWorkflow: %v", err)
	}
	if _, err := client.GetMigrationWorkflow(ctx, &migrationpb.GetMigrationWorkflowRequest{Name: created.GetName()}); err == nil {
		t.Fatal("expected error after delete")
	}
}
