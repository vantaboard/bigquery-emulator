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

package emulator

import (
	"context"
	"fmt"
	"os"
	"testing"

	connection "cloud.google.com/go/bigquery/connection/apiv1"
	"cloud.google.com/go/bigquery/connection/apiv1/connectionpb"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/bqopts"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/internal/testutil"
	"google.golang.org/api/iterator"
)

func TestConnectionList(t *testing.T) {
	if os.Getenv("BIGQUERY_EMULATOR_HOST") != "" && os.Getenv("BIGQUERY_STORAGE_GRPC_ENDPOINT") == "" {
		t.Skip("BigQuery Connection API uses gRPC; set BIGQUERY_STORAGE_GRPC_ENDPOINT to the emulator gRPC host:port or unset BIGQUERY_EMULATOR_HOST")
	}
	tc := testutil.SystemTest(t)
	ctx := context.Background()

	client, err := connection.NewClient(ctx, bqopts.ConnectionGRPCClientOptions()...)
	if err != nil {
		t.Fatalf("NewClient: %v", err)
	}
	defer client.Close()

	parent := fmt.Sprintf("projects/%s/locations/US", tc.ProjectID)
	it := client.ListConnections(ctx, &connectionpb.ListConnectionsRequest{Parent: parent})
	for {
		_, err := it.Next()
		if err == iterator.Done {
			break
		}
		if err != nil {
			t.Fatalf("ListConnections: %v", err)
		}
	}
}
