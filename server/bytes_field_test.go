package server_test

import (
	"context"
	"encoding/base64"
	"strings"
	"testing"

	"cloud.google.com/go/bigquery"
	"github.com/google/go-cmp/cmp"
	"github.com/vantaboard/bigquery-emulatorlator/server"
	"github.com/vantaboard/bigquery-emulatorlator/types"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
)

// TestBytesFieldBase64Encoding tests that BYTES fields are correctly handled with base64 encoding.
//
// This test verifies the fix for https://github.com/Recidiviz/bigquery-emulator/pull/55
//
// According to BigQuery documentation:
// - BYTES fields must be base64-encoded when sent via JSON API (tabledata.insertAll)
// - BYTES fields are returned as base64-encoded strings when queried
// - The emulator should NOT double-encode values that are already base64-encoded
//
// Reference: https://cloud.google.com/bigquery/docs/reference/rest/v2/tabledata/insertAll
// The BigQuery client library automatically base64-encodes byte strings before sending.
//
// Example from the bug report:
// - Original bytes: []byte{72, 101, 108, 108, 111} (Hello)
// - Expected base64: 'SGVsbG8='
// - Bug behavior (double-encoded): 'U0dWc2JHOD0=' (base64 of 'SGVsbG8=')
func TestBytesFieldBase64Encoding(t *testing.T) {
	ctx := context.Background()

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}

	const (
		projectID = "test"
		datasetID = "test_dataset"
		tableID   = "test_table"
	)

	// Create a table with id (INT64) and binary_data (BYTES) fields
	project := types.NewProject(
		projectID,
		types.NewDataset(
			datasetID,
			types.NewTable(
				tableID,
				[]*types.Column{
					types.NewColumn("id", types.INT64),
					types.NewColumn("binary_data", types.BYTES),
				},
				nil,
			),
		),
	)

	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}

	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()

	client, err := bigquery.NewClient(
		ctx,
		projectID,
		option.WithEndpoint(testServer.URL),
		option.WithoutAuthentication(),
	)
	if err != nil {
		t.Fatal(err)
	}
	defer client.Close()

	// Test case 1: Simple ASCII string "Hello"
	// Original bytes: []byte{72, 101, 108, 108, 111}
	// Expected base64: 'SGVsbG8='
	helloBytes := []byte{72, 101, 108, 108, 111} // "Hello"
	helloBase64 := base64.StdEncoding.EncodeToString(helloBytes)
	if helloBase64 != "SGVsbG8=" {
		t.Fatalf("expected hello_base64 to be 'SGVsbG8=', got %s", helloBase64)
	}

	// Test case 2: Binary data with various byte values
	binaryBytes := []byte{0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x00, 0xFF, 0xAB}
	binaryBase64 := base64.StdEncoding.EncodeToString(binaryBytes)

	// Test case 3: Empty bytes
	emptyBytes := []byte{}
	emptyBase64 := base64.StdEncoding.EncodeToString(emptyBytes)

	// Load data into table using JSON format
	// The BigQuery client expects base64-encoded strings for BYTES fields
	jsonData := strings.Join([]string{
		`{"id": 1, "binary_data": "` + helloBase64 + `"}`,
		`{"id": 2, "binary_data": "` + binaryBase64 + `"}`,
		`{"id": 3, "binary_data": "` + emptyBase64 + `"}`,
		`{"id": 4, "binary_data": null}`,
	}, "\n")

	schema := bigquery.Schema{
		{Name: "id", Type: bigquery.IntegerFieldType, Required: true},
		{Name: "binary_data", Type: bigquery.BytesFieldType},
	}

	source := bigquery.NewReaderSource(strings.NewReader(jsonData))
	source.SourceFormat = bigquery.JSON
	source.Schema = schema

	loader := client.Dataset(datasetID).Table(tableID).LoaderFrom(source)

	job, err := loader.Run(ctx)
	if err != nil {
		t.Fatalf("failed to run load job: %v", err)
	}

	status, err := job.Wait(ctx)
	if err != nil {
		t.Fatalf("failed to wait for load job: %v", err)
	}

	if err := status.Err(); err != nil {
		t.Fatalf("load job failed: %v", err)
	}

	// Query the data back and verify it's not double-encoded
	t.Run("query returns correctly encoded bytes", func(t *testing.T) {
		query := client.Query("SELECT id, binary_data FROM `" + projectID + "." + datasetID + "." + tableID + "` ORDER BY id")
		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("query failed: %v", err)
		}

		type row struct {
			ID         int64  `bigquery:"id"`
			BinaryData []byte `bigquery:"binary_data"`
		}

		var rows []*row
		for {
			var r row
			if err := it.Next(&r); err != nil {
				if err == iterator.Done {
					break
				}
				t.Fatalf("iterator failed: %v", err)
			}
			rows = append(rows, &r)
		}

		expectedRows := []*row{
			{ID: 1, BinaryData: helloBytes},
			{ID: 2, BinaryData: binaryBytes},
			{ID: 3, BinaryData: emptyBytes},
			{ID: 4, BinaryData: nil},
		}

		if diff := cmp.Diff(expectedRows, rows); diff != "" {
			t.Errorf("(-want +got):\n%s", diff)
		}

		// Additional verification: ensure the returned bytes match the original
		if !cmp.Equal(rows[0].BinaryData, helloBytes) {
			t.Errorf("row 1: expected binary_data to be %v, got %v", helloBytes, rows[0].BinaryData)
			// Check if it was double-encoded
			decodedOnce := rows[0].BinaryData
			if string(decodedOnce) == helloBase64 {
				t.Errorf("row 1: binary_data appears to be still base64-encoded (not decoded)")
			}
		}

		if !cmp.Equal(rows[1].BinaryData, binaryBytes) {
			t.Errorf("row 2: expected binary_data to be %v, got %v", binaryBytes, rows[1].BinaryData)
		}

		if !cmp.Equal(rows[2].BinaryData, emptyBytes) {
			t.Errorf("row 3: expected binary_data to be %v, got %v", emptyBytes, rows[2].BinaryData)
		}

		if rows[3].BinaryData != nil {
			t.Errorf("row 4: expected binary_data to be nil, got %v", rows[3].BinaryData)
		}
	})

	// Test TO_BASE64 function to verify encoding is correct
	// When we call TO_BASE64 on the stored bytes, it should return the same base64 string
	// we originally provided (proving the data was stored as raw bytes, not as base64 string)
	t.Run("TO_BASE64 function produces expected results", func(t *testing.T) {
		query := client.Query(`
			SELECT
				id,
				binary_data,
				TO_BASE64(binary_data) as explicit_base64
			FROM ` + "`" + projectID + "." + datasetID + "." + tableID + "`" + `
			WHERE binary_data IS NOT NULL
			ORDER BY id
		`)
		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("query with TO_BASE64 failed: %v", err)
		}

		type row struct {
			ID             int64  `bigquery:"id"`
			BinaryData     []byte `bigquery:"binary_data"`
			ExplicitBase64 string `bigquery:"explicit_base64"`
		}

		var rows []*row
		for {
			var r row
			if err := it.Next(&r); err != nil {
				if err == iterator.Done {
					break
				}
				t.Fatalf("iterator failed: %v", err)
			}
			rows = append(rows, &r)
		}

		// Verify that TO_BASE64(binary_data) produces the original base64 string
		// This proves the data was stored as raw bytes, not as a base64-encoded string
		expectedRows := []*row{
			{
				ID:             1,
				BinaryData:     helloBytes,
				ExplicitBase64: helloBase64,
			},
			{
				ID:             2,
				BinaryData:     binaryBytes,
				ExplicitBase64: binaryBase64,
			},
			{
				ID:             3,
				BinaryData:     emptyBytes,
				ExplicitBase64: emptyBase64,
			},
		}

		if diff := cmp.Diff(expectedRows, rows); diff != "" {
			t.Errorf("TO_BASE64 results (-want +got):\n%s", diff)
		}

		// Verify that the explicit_base64 matches what we expect
		if string(rows[0].ExplicitBase64) != helloBase64 {
			t.Errorf("row 1: TO_BASE64(binary_data) expected %s, got %s", helloBase64, string(rows[0].ExplicitBase64))
			// If this fails, it might indicate double-encoding
			doubleEncoded := base64.StdEncoding.EncodeToString([]byte(helloBase64))
			if string(rows[0].ExplicitBase64) == doubleEncoded {
				t.Errorf("row 1: TO_BASE64(binary_data) appears to be double-encoded! Got %s (double-encoded value)", doubleEncoded)
			}
		}
	})

	// Test using the Inserter API (alternative to JSON loading)
	t.Run("inserter API handles bytes correctly", func(t *testing.T) {
		type TestRow struct {
			ID         int64  `bigquery:"id"`
			BinaryData []byte `bigquery:"binary_data"`
		}

		table := client.Dataset(datasetID).Table(tableID)
		inserter := table.Inserter()

		// Insert additional test data using Inserter API
		testBytes := []byte{0xDE, 0xAD, 0xBE, 0xEF}
		row := TestRow{
			ID:         5,
			BinaryData: testBytes,
		}

		if err := inserter.Put(ctx, []*TestRow{&row}); err != nil {
			t.Fatalf("inserter.Put failed: %v", err)
		}

		// Query back the inserted row
		query := client.Query("SELECT id, binary_data FROM `" + projectID + "." + datasetID + "." + tableID + "` WHERE id = 5")
		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("query failed: %v", err)
		}

		var r TestRow
		if err := it.Next(&r); err != nil {
			t.Fatalf("iterator failed: %v", err)
		}

		if !cmp.Equal(r.BinaryData, testBytes) {
			t.Errorf("expected binary_data to be %v, got %v", testBytes, r.BinaryData)
		}

		// Verify no more rows
		if err := it.Next(&r); err != iterator.Done {
			t.Errorf("expected only one row, but got more")
		}
	})
}
