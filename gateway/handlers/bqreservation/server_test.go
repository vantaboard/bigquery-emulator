package bqreservation

import (
	"context"
	"net"
	"testing"

	"cloud.google.com/go/bigquery/reservation/apiv1/reservationpb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/test/bufconn"
)

const bufSize = 1024 * 1024

func startTestServer(t *testing.T) (reservationpb.ReservationServiceClient, func()) {
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
	return reservationpb.NewReservationServiceClient(conn), cleanup
}

func TestListCapacityCommitmentsEmpty(t *testing.T) {
	client, cleanup := startTestServer(t)
	defer cleanup()

	resp, err := client.ListCapacityCommitments(context.Background(),
		&reservationpb.ListCapacityCommitmentsRequest{
			Parent: "projects/p/locations/US",
		})
	if err != nil {
		t.Fatalf("ListCapacityCommitments: %v", err)
	}
	if len(resp.GetCapacityCommitments()) != 0 {
		t.Fatalf("commitments = %d, want 0", len(resp.GetCapacityCommitments()))
	}
}

func TestListReservationsEmpty(t *testing.T) {
	client, cleanup := startTestServer(t)
	defer cleanup()

	resp, err := client.ListReservations(context.Background(),
		&reservationpb.ListReservationsRequest{
			Parent: "projects/p/locations/US",
		})
	if err != nil {
		t.Fatalf("ListReservations: %v", err)
	}
	if len(resp.GetReservations()) != 0 {
		t.Fatalf("reservations = %d, want 0", len(resp.GetReservations()))
	}
}
