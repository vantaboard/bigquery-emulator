package bqconnection

import (
	"context"
	"net"
	"testing"

	"cloud.google.com/go/bigquery/connection/apiv1/connectionpb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/test/bufconn"
)

const bufSize = 1024 * 1024

func startTestServer(t *testing.T) (connectionpb.ConnectionServiceClient, func()) {
	t.Helper()
	lis := bufconn.Listen(bufSize)
	srv := grpc.NewServer()
	RegisterGRPC(srv, handlers.Dependencies{})
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
	return connectionpb.NewConnectionServiceClient(conn), cleanup
}

func TestListConnectionsEmpty(t *testing.T) {
	client, cleanup := startTestServer(t)
	defer cleanup()

	resp, err := client.ListConnections(context.Background(), &connectionpb.ListConnectionsRequest{
		Parent: "projects/p/locations/US",
	})
	if err != nil {
		t.Fatalf("ListConnections: %v", err)
	}
	if len(resp.GetConnections()) != 0 {
		t.Fatalf("connections = %d, want 0", len(resp.GetConnections()))
	}
}

func TestCreateAndGetConnection(t *testing.T) {
	client, cleanup := startTestServer(t)
	defer cleanup()
	ctx := context.Background()
	parent := "projects/p/locations/US"

	created, err := client.CreateConnection(ctx, &connectionpb.CreateConnectionRequest{
		Parent:       parent,
		ConnectionId: "conn1",
		Connection: &connectionpb.Connection{
			FriendlyName: "test",
		},
	})
	if err != nil {
		t.Fatalf("CreateConnection: %v", err)
	}
	if created.GetName() != parent+"/connections/conn1" {
		t.Fatalf("name = %q", created.GetName())
	}

	got, err := client.GetConnection(ctx, &connectionpb.GetConnectionRequest{
		Name: parent + "/connections/conn1",
	})
	if err != nil {
		t.Fatalf("GetConnection: %v", err)
	}
	if got.GetFriendlyName() != "test" {
		t.Fatalf("friendlyName = %q", got.GetFriendlyName())
	}
}
