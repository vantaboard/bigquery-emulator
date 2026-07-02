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
		if serveErr := srv.Serve(lis); serveErr != nil {
			t.Logf("grpc serve: %v", serveErr)
		}
	}()
	dialer := func(context.Context, string) (net.Conn, error) {
		return lis.Dial()
	}
	conn, dialErr := grpc.NewClient("passthrough:///bufnet",
		grpc.WithContextDialer(dialer),
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if dialErr != nil {
		t.Fatalf("grpc.NewClient: %v", dialErr)
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
	connName := parent + "/connections/conn1"

	created := createTestConnection(t, client, ctx, parent, connName)
	assertGetFriendlyName(t, client, ctx, connName, "test")
	assertUpdateFriendlyName(t, client, ctx, connName, "renamed")
	deleteAndAssertNotFound(t, client, ctx, connName)
	assertRegistryFileExists(t, dataDir)
	assertDeletedAfterRestart(t, dataDir, connName)
	assertPersistedAfterRestart(t, dataDir, parent, ctx)
	_ = created
}

func createTestConnection(
	t *testing.T,
	client connectionpb.ConnectionServiceClient,
	ctx context.Context,
	parent, connName string,
) *connectionpb.Connection {
	t.Helper()
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
	if created.GetName() != connName {
		t.Fatalf("name = %q", created.GetName())
	}
	return created
}

func assertGetFriendlyName(
	t *testing.T,
	client connectionpb.ConnectionServiceClient,
	ctx context.Context,
	connName, want string,
) {
	t.Helper()
	got, err := client.GetConnection(ctx, &connectionpb.GetConnectionRequest{Name: connName})
	if err != nil {
		t.Fatalf("GetConnection: %v", err)
	}
	if got.GetFriendlyName() != want {
		t.Fatalf("friendlyName = %q", got.GetFriendlyName())
	}
}

func assertUpdateFriendlyName(
	t *testing.T,
	client connectionpb.ConnectionServiceClient,
	ctx context.Context,
	connName, want string,
) {
	t.Helper()
	updated, err := client.UpdateConnection(ctx, &connectionpb.UpdateConnectionRequest{
		Name:       connName,
		Connection: &connectionpb.Connection{FriendlyName: want},
	})
	if err != nil {
		t.Fatalf("UpdateConnection: %v", err)
	}
	if updated.GetFriendlyName() != want {
		t.Fatalf("friendlyName = %q", updated.GetFriendlyName())
	}
}

func deleteAndAssertNotFound(
	t *testing.T,
	client connectionpb.ConnectionServiceClient,
	ctx context.Context,
	connName string,
) {
	t.Helper()
	if _, err := client.DeleteConnection(ctx, &connectionpb.DeleteConnectionRequest{Name: connName}); err != nil {
		t.Fatalf("DeleteConnection: %v", err)
	}
	_, getErr := client.GetConnection(ctx, &connectionpb.GetConnectionRequest{Name: connName})
	if getErr == nil {
		t.Fatal("expected NotFound after delete")
	} else if status.Code(getErr) != codes.NotFound {
		t.Fatalf("GetConnection after delete: %v", getErr)
	}
}

func assertRegistryFileExists(t *testing.T, dataDir string) {
	t.Helper()
	registry := filepath.Join(dataDir, "external", "connections", "_registry", "connections.json")
	if _, readErr := os.ReadFile(registry); readErr != nil {
		t.Fatalf("registry file: %v", readErr)
	}
}

func assertDeletedAfterRestart(t *testing.T, dataDir, connName string) {
	t.Helper()
	client2, cleanup2 := startTestServer(t, dataDir)
	defer cleanup2()
	ctx := context.Background()
	_, getErr := client2.GetConnection(ctx, &connectionpb.GetConnectionRequest{Name: connName})
	if getErr == nil {
		t.Fatal("expected NotFound after restart (deleted)")
	}
}

func assertPersistedAfterRestart(t *testing.T, dataDir, parent string, ctx context.Context) {
	t.Helper()
	client2, cleanup2 := startTestServer(t, dataDir)
	defer cleanup2()
	created2, err := client2.CreateConnection(ctx, &connectionpb.CreateConnectionRequest{
		Parent:       parent,
		ConnectionId: "conn2",
		Connection:   &connectionpb.Connection{FriendlyName: "persist"},
	})
	if err != nil {
		t.Fatalf("CreateConnection restart: %v", err)
	}
	cleanup2()
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
