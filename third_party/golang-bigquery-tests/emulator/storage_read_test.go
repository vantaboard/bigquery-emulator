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
	"io"
	"os"
	"testing"

	bqStorage "cloud.google.com/go/bigquery/storage/apiv1"
	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/bqopts"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/internal/testutil"
	gax "github.com/googleapis/gax-go/v2"
	"google.golang.org/grpc"
)

var storageReadRPCOpts = gax.WithGRPCOptions(
	grpc.MaxCallRecvMsgSize(1024 * 1024 * 129),
)

func TestStorageReadSession(t *testing.T) {
	if os.Getenv("BIGQUERY_EMULATOR_HOST") != "" && os.Getenv("BIGQUERY_STORAGE_GRPC_ENDPOINT") == "" {
		t.Skip("BigQuery Storage Read API uses gRPC; set BIGQUERY_STORAGE_GRPC_ENDPOINT to the emulator gRPC host:port or unset BIGQUERY_EMULATOR_HOST")
	}
	tc := testutil.SystemTest(t)
	ctx := context.Background()

	readOpts := bqopts.StorageGRPCClientOptions()
	var client *bqStorage.BigQueryReadClient
	var err error
	if len(readOpts) > 0 {
		client, err = bqStorage.NewBigQueryReadClient(ctx, readOpts...)
	} else {
		client, err = bqStorage.NewBigQueryReadClient(ctx)
	}
	if err != nil {
		t.Fatalf("NewBigQueryReadClient: %v", err)
	}
	defer client.Close()

	readTable := "projects/bigquery-public-data/datasets/usa_names/tables/usa_1910_current"
	session, err := client.CreateReadSession(ctx, &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", tc.ProjectID),
		ReadSession: &storagepb.ReadSession{
			Table:      readTable,
			DataFormat: storagepb.DataFormat_AVRO,
			ReadOptions: &storagepb.ReadSession_TableReadOptions{
				SelectedFields: []string{"name", "number", "state"},
				RowRestriction: `state = "WA"`,
			},
		},
		MaxStreamCount: 1,
	}, storageReadRPCOpts)
	if err != nil {
		t.Fatalf("CreateReadSession: %v", err)
	}
	if session.GetName() == "" {
		t.Fatal("expected non-empty session name")
	}
	if len(session.GetStreams()) == 0 {
		t.Fatal("expected at least one stream")
	}

	stream, err := client.ReadRows(ctx, &storagepb.ReadRowsRequest{
		ReadStream: session.GetStreams()[0].Name,
	}, storageReadRPCOpts)
	if err != nil {
		t.Fatalf("ReadRows: %v", err)
	}

	var rows int64
	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			t.Fatalf("stream.Recv: %v", err)
		}
		rows += resp.GetRowCount()
	}
	if rows == 0 {
		t.Fatal("expected at least one row from storage read")
	}
}
