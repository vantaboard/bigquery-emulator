//go:build integration

package e2e

import (
	"context"
	"io"
	"net/http"
	"sort"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

// TestStorageReadRoundTrip is the end-to-end story for the
// Storage Read API: stand up the emulator, insertAll rows through the
// REST gateway, then open a dedicated gRPC channel to the engine and
// round-trip those rows through `CreateReadSession` + `ReadRows`.
//
// The Storage Read surface is gRPC-only in production — BigQuery's
// REST gateway does not proxy it, and the
// `google-cloud-bigquery-storage` client libraries open a separate
// gRPC channel against `bigquerystorage.googleapis.com:443`. The
// emulator mirrors that posture: the REST gateway at
// `env.URL()` only speaks the public REST surfaces and the engine's
// gRPC port at `env.EngineAddress()` is where Storage Read lives.
// See `docs/REST_API.md#storage-read-api` for the user-facing version
// of the same explanation.
//
// The scenario runs against the persistent DuckDB store with a
// hermetic data dir under `t.TempDir()`. Each invocation exercises:
//
//  1. `insertAll` to seed the table via the REST path.
//  2. `CreateReadSession` + `ReadRows` over the engine's gRPC surface
//     and assert every inserted row comes back.
//  3. A second `CreateReadSession` + `ReadRows` with a
//     `<column> = <literal>` `row_restriction` and assert the
//     filtered subset.
func TestStorageReadRoundTrip(t *testing.T) {
	env := startEmulatorWithFlags(t, emulatorFlags{
		dataDir: t.TempDir(),
	})
	runStorageReadRoundTrip(t, env, "proj-storage-read-duck",
		"ds_storage_read_duck")
}

// runStorageReadRoundTrip seeds a table via REST `insertAll` and then
// exercises the engine's StorageRead gRPC surface (Catalog + Query
// rides over the same channel through the gateway, but StorageRead
// clients dial the engine directly because the gateway intentionally
// does not proxy it — see docs/REST_API.md).
func runStorageReadRoundTrip(t *testing.T, env *emulatorEnv,
	projectID, datasetID string) {
	t.Helper()
	const tableID = "people"

	// 1. Seed the dataset + table over REST so the engine has a row
	// store to scan. We pick INT64 / STRING / BOOL columns so the
	// row_restriction tests below have something to exercise across
	// all three literal kinds the row_restriction parser accepts.
	base := env.URL() + "/bigquery/v2/projects/" + projectID
	statusCode, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if statusCode != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", statusCode, string(body))
	}

	tableBody := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + tableID + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"name","type":"STRING","mode":"NULLABLE"},
            {"name":"active","type":"BOOL","mode":"NULLABLE"}
        ]}
    }`
	statusCode, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if statusCode != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", statusCode, string(body))
	}

	insertBody := `{
        "rows":[
            {"insertId":"a","json":{"id":1,"name":"ada","active":true}},
            {"insertId":"b","json":{"id":2,"name":"linus","active":false}},
            {"insertId":"c","json":{"id":3,"name":"grace","active":true}},
            {"insertId":"d","json":{"id":4,"name":"rob","active":false}},
            {"insertId":"e","json":{"id":5,"name":"hopper","active":true}}
        ]
    }`
	statusCode, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/insertAll",
		[]byte(insertBody))
	if statusCode != http.StatusOK {
		t.Fatalf("tabledata.insertAll -> %d: %s", statusCode, string(body))
	}

	// 2. Open a dedicated gRPC channel to the engine. The engine
	// surfaces `bigquery_emulator.v1.StorageRead` on the same port as
	// Catalog + Query, but for the StorageRead E2E we deliberately
	// build a fresh client connection (instead of reaching for the
	// gateway's internal channel) so the test mirrors how a real BQ
	// Storage client would discover and talk to the emulator.
	conn, err := grpc.NewClient(env.EngineAddress(),
		grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		t.Fatalf("dial engine at %s: %v", env.EngineAddress(), err)
	}
	t.Cleanup(func() { _ = conn.Close() })
	storage := enginepb.NewStorageReadClient(conn)

	// 3. Unfiltered read: CreateReadSession returns one stream, ReadRows
	// drains it, we collect ids and compare against the seeded set.
	allIDs := readSessionIDs(t, storage, projectID, datasetID, tableID,
		nil /*restriction*/)
	sort.Strings(allIDs)
	wantAll := []string{"1", "2", "3", "4", "5"}
	if !equalStrings(allIDs, wantAll) {
		t.Errorf("unfiltered ReadRows ids = %v, want %v", allIDs, wantAll)
	}

	// 4. Filtered read with `<column> = <literal>` row_restriction.
	// `id = 3` exercises the INT64 literal path; only `grace` should
	// come back.
	filteredIDs := readSessionIDs(t, storage, projectID, datasetID,
		tableID, ptr("id = 3"))
	if !equalStrings(filteredIDs, []string{"3"}) {
		t.Errorf("row_restriction `id = 3` ReadRows ids = %v, want [3]",
			filteredIDs)
	}

	// 5. STRING literal path: `name = 'linus'` -> single row.
	stringFiltered := readSessionIDs(t, storage, projectID, datasetID,
		tableID, ptr("name = 'linus'"))
	if !equalStrings(stringFiltered, []string{"2"}) {
		t.Errorf("row_restriction `name = 'linus'` ids = %v, want [2]",
			stringFiltered)
	}

	// 6. BOOL literal path: `active = true` -> 1, 3, 5.
	boolFiltered := readSessionIDs(t, storage, projectID, datasetID,
		tableID, ptr("active = true"))
	sort.Strings(boolFiltered)
	if !equalStrings(boolFiltered, []string{"1", "3", "5"}) {
		t.Errorf("row_restriction `active = true` ids = %v, want [1 3 5]",
			boolFiltered)
	}

	// 7. Range restrictions are accepted by the engine row_restriction
	// parser; verify the filtered subset matches `id > 0`.
	rangeFiltered := readSessionIDs(t, storage, projectID, datasetID,
		tableID, ptr("id > 0"))
	sort.Strings(rangeFiltered)
	if !equalStrings(rangeFiltered, wantAll) {
		t.Errorf("row_restriction `id > 0` ids = %v, want %v",
			rangeFiltered, wantAll)
	}
}

// readSessionIDs mints a fresh CreateReadSession over the supplied
// table, drains every page of ReadRows, and returns the values of
// the `id` column (the first cell). When `restriction` is non-nil it
// is forwarded as `ReadSession.read_options.row_restriction`. Test
// helpers stay in this file so the per-storage subtests can share the
// same drain logic without re-declaring it.
func readSessionIDs(t *testing.T, storage enginepb.StorageReadClient,
	projectID, datasetID, tableID string,
	restriction *string) []string {
	t.Helper()
	req := &enginepb.CreateReadSessionRequest{
		Parent: "projects/" + projectID,
		ReadSession: &enginepb.ReadSession{
			Table: "projects/" + projectID + "/datasets/" +
				datasetID + "/tables/" + tableID,
		},
	}
	if restriction != nil {
		req.ReadSession.ReadOptions = &enginepb.ReadOptions{
			RowRestriction: *restriction,
		}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	session, err := storage.CreateReadSession(ctx, req)
	if err != nil {
		t.Fatalf("CreateReadSession: %v", err)
	}
	if len(session.GetStreams()) != 1 {
		t.Fatalf("CreateReadSession streams = %d, want 1",
			len(session.GetStreams()))
	}

	readCtx, readCancel := context.WithTimeout(context.Background(),
		30*time.Second)
	defer readCancel()
	stream, err := storage.ReadRows(readCtx, &enginepb.ReadRowsRequest{
		ReadStream: session.GetStreams()[0].GetName(),
	})
	if err != nil {
		t.Fatalf("ReadRows: %v", err)
	}
	var ids []string
	for {
		page, recvErr := stream.Recv()
		if recvErr == io.EOF {
			break
		}
		if recvErr != nil {
			t.Fatalf("ReadRows recv: %v", recvErr)
		}
		for _, row := range page.GetRows() {
			if len(row.GetCells()) == 0 {
				t.Fatalf("ReadRows row has no cells: %+v", row)
			}
			// The engine lowers every primitive onto Cell.string_value
			// regardless of the declared column type; INT64 cells
			// arrive as decimal strings, BOOL cells as "true"/"false".
			ids = append(ids, row.GetCells()[0].GetStringValue())
		}
	}
	return ids
}

func ptr(s string) *string { return &s }

func equalStrings(a, b []string) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}
