package bqanalyticshub

import (
	"context"
	"net"
	"testing"

	"cloud.google.com/go/bigquery/analyticshub/apiv1/analyticshubpb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/status"
	"google.golang.org/grpc/test/bufconn"
)

const bufSize = 1024 * 1024

func startTestServer(t *testing.T) (analyticshubpb.AnalyticsHubServiceClient, func()) {
	t.Helper()
	lis := bufconn.Listen(bufSize)
	srv := grpc.NewServer()
	RegisterGRPC(srv)
	go func() {
		if err := srv.Serve(lis); err != nil {
			t.Logf("grpc serve: %v", err)
		}
	}()
	dialer := func(context.Context, string) (net.Conn, error) {
		return lis.Dial()
	}
	conn, err := grpc.NewClient("passthrough:///bufnet",
		grpc.WithContextDialer(dialer),
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		t.Fatalf("grpc.NewClient: %v", err)
	}
	cleanup := func() {
		conn.Close()
		srv.Stop()
	}
	return analyticshubpb.NewAnalyticsHubServiceClient(conn), cleanup
}

func TestAnalyticsHubExchangeLifecycle(t *testing.T) {
	client, cleanup := startTestServer(t)
	defer cleanup()
	exerciseAnalyticsHubExchange(t, client, context.Background(), "projects/p/locations/US", "EmulatorDataExchange")
}

func exerciseAnalyticsHubExchange(
	t *testing.T,
	client analyticshubpb.AnalyticsHubServiceClient,
	ctx context.Context,
	parent, exchangeID string,
) {
	t.Helper()
	exchange, err := client.CreateDataExchange(ctx, &analyticshubpb.CreateDataExchangeRequest{
		Parent:         parent,
		DataExchangeId: exchangeID,
		DataExchange: &analyticshubpb.DataExchange{
			DisplayName: "Emulator Data Exchange",
		},
	})
	if err != nil {
		t.Fatalf("CreateDataExchange: %v", err)
	}
	if exchange.GetName() == "" {
		t.Fatal("expected non-empty exchange name")
	}

	_, err = client.CreateDataExchange(ctx, &analyticshubpb.CreateDataExchangeRequest{
		Parent:         parent,
		DataExchangeId: exchangeID,
		DataExchange:   &analyticshubpb.DataExchange{},
	})
	if status.Code(err) != codes.AlreadyExists {
		t.Fatalf("duplicate CreateDataExchange err = %v, want AlreadyExists", err)
	}

	got, err := client.GetDataExchange(ctx, &analyticshubpb.GetDataExchangeRequest{
		Name: parent + "/dataExchanges/" + exchangeID,
	})
	if err != nil {
		t.Fatalf("GetDataExchange: %v", err)
	}
	if got.GetDisplayName() != "Emulator Data Exchange" {
		t.Fatalf("displayName = %q", got.GetDisplayName())
	}

	createAnalyticsHubListing(t, client, ctx, parent, exchangeID)

	if _, err := client.DeleteDataExchange(ctx, &analyticshubpb.DeleteDataExchangeRequest{
		Name: parent + "/dataExchanges/" + exchangeID,
	}); err != nil {
		t.Fatalf("DeleteDataExchange: %v", err)
	}
}

func createAnalyticsHubListing(
	t *testing.T,
	client analyticshubpb.AnalyticsHubServiceClient,
	ctx context.Context,
	parent, exchangeID string,
) {
	t.Helper()
	listing, err := client.CreateListing(ctx, &analyticshubpb.CreateListingRequest{
		Parent:    parent + "/dataExchanges/" + exchangeID,
		ListingId: "EmulatorListing",
		Listing: &analyticshubpb.Listing{
			DisplayName: "Emulator Listing",
			Source: &analyticshubpb.Listing_BigqueryDataset{
				BigqueryDataset: &analyticshubpb.Listing_BigQueryDatasetSource{
					Dataset: "projects/p/datasets/ds",
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
}
