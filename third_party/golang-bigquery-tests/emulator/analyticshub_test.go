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

	"cloud.google.com/go/bigquery"
	analyticshub "cloud.google.com/go/bigquery/analyticshub/apiv1"
	"cloud.google.com/go/bigquery/analyticshub/apiv1/analyticshubpb"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/bqopts"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/internal/testutil"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/snippets/bqtestutil"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

func TestAnalyticsHubExchange(t *testing.T) {
	if os.Getenv("BIGQUERY_EMULATOR_HOST") != "" && os.Getenv("BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT") == "" {
		t.Skip("Analytics Hub uses gRPC; set BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT to the emulator gRPC host:port or unset BIGQUERY_EMULATOR_HOST")
	}
	tc := testutil.SystemTest(t)
	ctx := context.Background()

	bqClient, err := bigquery.NewClient(ctx, tc.ProjectID, bqopts.ClientOptions()...)
	if err != nil {
		t.Fatalf("bigquery.NewClient: %v", err)
	}
	dsName, err := bqtestutil.UniqueBQName("analyticshub_emulator_dataset")
	if err != nil {
		t.Fatalf("UniqueBQName: %v", err)
	}
	dataset := bqClient.Dataset(dsName)
	if err := dataset.Create(ctx, nil); err != nil {
		t.Fatalf("dataset.Create: %v", err)
	}
	defer func() {
		_ = dataset.DeleteWithContents(ctx)
		_ = bqClient.Close()
	}()
	datasetSource := fmt.Sprintf("projects/%s/datasets/%s", tc.ProjectID, dsName)

	ahubClient, err := analyticshub.NewClient(ctx, bqopts.AnalyticsHubGRPCClientOptions()...)
	if err != nil {
		t.Fatalf("analyticshub.NewClient: %v", err)
	}
	defer ahubClient.Close()

	const location = "US"
	exchangeID := "EmulatorDataExchange"
	parent := fmt.Sprintf("projects/%s/locations/%s", tc.ProjectID, location)

	exchange, err := ahubClient.CreateDataExchange(ctx, &analyticshubpb.CreateDataExchangeRequest{
		Parent:         parent,
		DataExchangeId: exchangeID,
		DataExchange: &analyticshubpb.DataExchange{
			DisplayName: "Emulator Data Exchange",
		},
	})
	if err != nil {
		if status.Code(err) != codes.AlreadyExists {
			t.Fatalf("CreateDataExchange: %v", err)
		}
		exchange, err = ahubClient.GetDataExchange(ctx, &analyticshubpb.GetDataExchangeRequest{
			Name: fmt.Sprintf("%s/dataExchanges/%s", parent, exchangeID),
		})
		if err != nil {
			t.Fatalf("GetDataExchange: %v", err)
		}
	}
	if exchange.GetName() == "" {
		t.Fatal("expected non-empty exchange name")
	}

	listingID := "EmulatorListing"
	listing, err := ahubClient.CreateListing(ctx, &analyticshubpb.CreateListingRequest{
		Parent:    fmt.Sprintf("%s/dataExchanges/%s", parent, exchangeID),
		ListingId: listingID,
		Listing: &analyticshubpb.Listing{
			DisplayName: "Emulator Listing",
			Source: &analyticshubpb.Listing_BigqueryDataset{
				BigqueryDataset: &analyticshubpb.Listing_BigQueryDatasetSource{
					Dataset: datasetSource,
				},
			},
		},
	})
	if err != nil {
		t.Fatalf("CreateListing: %v", err)
	}
	if listing.GetName() == "" {
		t.Fatal("expected non-empty listing name")
	}

	if err := ahubClient.DeleteDataExchange(ctx, &analyticshubpb.DeleteDataExchangeRequest{
		Name: fmt.Sprintf("%s/dataExchanges/%s", parent, exchangeID),
	}); err != nil {
		t.Fatalf("DeleteDataExchange: %v", err)
	}
}
