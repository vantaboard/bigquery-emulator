package bqconnection

import (
	"context"
	"net"
	"os"
	"path/filepath"
	"testing"

	"cloud.google.com/go/bigquery/connection/apiv1/connectionpb"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/status"
	"google.golang.org/grpc/test/bufconn"
)

const bufSize = 1024 * 1024

func startTestServer(t *testing.T, dataDir string) (connectionpb.ConnectionServiceClient, func()) {
	t.Helper()
	cfg, err := sourceconfig.Load(dataDir)
	if err != nil {
		t.Fatalf("sourceconfig.Load: %v", err)
	}
	lis := bufconn.Listen(bufSize)
	srv := grpc.NewServer()
	RegisterGRPC(srv, handlers.Dependencies{ExternalSources: cfg})
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
	client, cleanup := startTestServer(t, t.TempDir())
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

func TestCreateGetUpdateDeleteRoundTrip(t *testing.T) {
	dataDir := t.TempDir()
	client, cleanup := startTestServer(t, dataDir)
	defer cleanup()
	ctx := context.Background()
	parent := "projects/p/locations/US"

	created, err := client.CreateConnection(ctx, &connectionpb.CreateConnectionRequest{
		Parent:       parent,
		ConnectionId: "conn1",
		Connection: &connectionpb.Connection{
			FriendlyName: "test",
			Properties: &connectionpb.Connection_CloudSql{
				CloudSql: &connectionpb.CloudSqlProperties{
					Database: "db1",
					Type:     connectionpb.CloudSqlProperties_POSTGRES,
				},
			},
		},
	})
	if err != nil {
		t.Fatalf("CreateConnection: %v", err)
	}
	if created.GetName() != parent+"/connections/conn1" {
		t.Fatalf("name = %q", created.GetName())
	}
	if created.GetCloudSql().GetDatabase() != "db1" {
		t.Fatalf("cloudSql = %#v", created.GetCloudSql())
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

	updated, err := client.UpdateConnection(ctx, &connectionpb.UpdateConnectionRequest{
		Name: parent + "/connections/conn1",
		Connection: &connectionpb.Connection{
			FriendlyName: "renamed",
		},
	})
	if err != nil {
		t.Fatalf("UpdateConnection: %v", err)
	}
	if updated.GetFriendlyName() != "renamed" {
		t.Fatalf("friendlyName = %q", updated.GetFriendlyName())
	}

	if _, err := client.DeleteConnection(ctx, &connectionpb.DeleteConnectionRequest{
		Name: parent + "/connections/conn1",
	}); err != nil {
		t.Fatalf("DeleteConnection: %v", err)
	}
	if _, err := client.GetConnection(ctx, &connectionpb.GetConnectionRequest{
		Name: parent + "/connections/conn1",
	}); err == nil {
		t.Fatal("expected NotFound after delete")
	} else if status.Code(err) != codes.NotFound {
		t.Fatalf("GetConnection after delete: %v", err)
	}

	registry := filepath.Join(dataDir, "external", "connections", "_registry", "connections.json")
	if _, err := os.ReadFile(registry); err != nil {
		t.Fatalf("registry file: %v", err)
	}

	// Restart from disk.
	client2, cleanup2 := startTestServer(t, dataDir)
	defer cleanup2()
	if _, err := client2.GetConnection(ctx, &connectionpb.GetConnectionRequest{
		Name: parent + "/connections/conn1",
	}); err == nil {
		t.Fatal("expected NotFound after restart (deleted)")
	}
	created2, err := client2.CreateConnection(ctx, &connectionpb.CreateConnectionRequest{
		Parent:       parent,
		ConnectionId: "conn2",
		Connection:   &connectionpb.Connection{FriendlyName: "persist"},
	})
	if err != nil {
		t.Fatalf("CreateConnection restart: %v", err)
	}
	client2Cleanup := cleanup2
	client2Cleanup()
	client3, cleanup3 := startTestServer(t, dataDir)
	defer cleanup3()
	got2, err := client3.GetConnection(ctx, &connectionpb.GetConnectionRequest{
		Name: created2.GetName(),
	})
	if err != nil {
		t.Fatalf("GetConnection after restart: %v", err)
	}
	if got2.GetFriendlyName() != "persist" {
		t.Fatalf("friendlyName = %q", got2.GetFriendlyName())
	}
}
