package server_test

import (
	"context"
	"testing"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulatorlator/server"
	"github.com/vantaboard/bigquery-emulatorlator/types"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
)

// TestCopyJob tests the table copy functionality added in PR #41.
// It verifies that data can be copied from one table to another using BigQuery copy jobs.
func TestCopyJob(t *testing.T) {
	ctx := context.Background()

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}

	const (
		projectID     = "test"
		datasetID     = "test_dataset"
		sourceTableID = "source_table"
		destTableID   = "dest_table"
		dest2TableID  = "dest2_table"
	)

	// Create a source table with some data
	project := types.NewProject(
		projectID,
		types.NewDataset(
			datasetID,
			types.NewTable(
				sourceTableID,
				[]*types.Column{
					types.NewColumn("id", types.INT64),
					types.NewColumn("name", types.STRING),
					types.NewColumn("value", types.FLOAT64),
					types.NewColumn("active", types.BOOL),
				},
				types.Data{
					map[string]interface{}{
						"id":     1,
						"name":   "Alice",
						"value":  100.5,
						"active": true,
					},
					map[string]interface{}{
						"id":     2,
						"name":   "Bob",
						"value":  200.75,
						"active": false,
					},
					map[string]interface{}{
						"id":     3,
						"name":   "Charlie",
						"value":  300.25,
						"active": true,
					},
				},
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

	// Test 1: Basic copy job
	t.Run("basic copy", func(t *testing.T) {
		srcTable := client.Dataset(datasetID).Table(sourceTableID)
		dstTable := client.Dataset(datasetID).Table(destTableID)

		copier := dstTable.CopierFrom(srcTable)
		job, err := copier.Run(ctx)
		if err != nil {
			t.Fatalf("failed to start copy job: %v", err)
		}

		status, err := job.Wait(ctx)
		if err != nil {
			t.Fatalf("copy job failed: %v", err)
		}

		if err := status.Err(); err != nil {
			t.Fatalf("copy job returned error: %v", err)
		}

		// Verify the destination table has the same data
		iter := dstTable.Read(ctx)
		var rowCount int
		for {
			var row map[string]bigquery.Value
			if err := iter.Next(&row); err != nil {
				if err == iterator.Done {
					break
				}
				t.Fatalf("failed to read row: %v", err)
			}
			rowCount++
		}

		if rowCount != 3 {
			t.Errorf("expected 3 rows in destination table, got %d", rowCount)
		}
	})

	// Test 2: Verify data integrity after copy
	t.Run("data integrity", func(t *testing.T) {
		query := client.Query(`
			SELECT id, name, value, active
			FROM ` + "`" + datasetID + "." + destTableID + "`" + `
			ORDER BY id`)

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to query destination table: %v", err)
		}

		expected := []struct {
			ID     int64
			Name   string
			Value  float64
			Active bool
		}{
			{1, "Alice", 100.5, true},
			{2, "Bob", 200.75, false},
			{3, "Charlie", 300.25, true},
		}

		for i, exp := range expected {
			var row struct {
				ID     int64
				Name   string
				Value  float64
				Active bool
			}

			if err := it.Next(&row); err != nil {
				if err == iterator.Done {
					t.Fatalf("expected more rows, got only %d", i)
				}
				t.Fatalf("failed to read row: %v", err)
			}

			if row.ID != exp.ID || row.Name != exp.Name || row.Value != exp.Value || row.Active != exp.Active {
				t.Errorf("row %d: got (%d, %s, %f, %t), want (%d, %s, %f, %t)",
					i, row.ID, row.Name, row.Value, row.Active,
					exp.ID, exp.Name, exp.Value, exp.Active)
			}
		}
	})

	// Test 3: Copy with WriteDisposition=WriteTruncate
	t.Run("copy with write truncate", func(t *testing.T) {
		// First, insert some data into dest2_table
		insertQuery := client.Query(`
			CREATE TABLE ` + "`" + datasetID + "." + dest2TableID + "`" + ` AS
			SELECT 999 AS id, "Existing" AS name, 999.99 AS value, false AS active`)

		job, err := insertQuery.Run(ctx)
		if err != nil {
			t.Fatalf("failed to create initial table: %v", err)
		}

		status, err := job.Wait(ctx)
		if err != nil {
			t.Fatalf("failed to wait for initial table creation: %v", err)
		}

		if err := status.Err(); err != nil {
			t.Fatalf("initial table creation failed: %v", err)
		}

		// Verify initial data exists
		dstTable := client.Dataset(datasetID).Table(dest2TableID)
		iter := dstTable.Read(ctx)
		var rowCount int
		for {
			var row map[string]bigquery.Value
			if err := iter.Next(&row); err != nil {
				if err == iterator.Done {
					break
				}
				t.Fatalf("failed to read initial row: %v", err)
			}
			rowCount++
		}

		if rowCount != 1 {
			t.Errorf("expected 1 initial row, got %d", rowCount)
		}

		// Now copy with WriteTruncate
		srcTable := client.Dataset(datasetID).Table(sourceTableID)
		copier := dstTable.CopierFrom(srcTable)
		copier.WriteDisposition = bigquery.WriteTruncate

		job, err = copier.Run(ctx)
		if err != nil {
			t.Fatalf("failed to start copy job with truncate: %v", err)
		}

		status, err = job.Wait(ctx)
		if err != nil {
			t.Fatalf("copy job with truncate failed: %v", err)
		}

		if err := status.Err(); err != nil {
			t.Fatalf("copy job with truncate returned error: %v", err)
		}

		// Verify the destination table has only the copied data (not the initial data)
		iter = dstTable.Read(ctx)
		rowCount = 0
		for {
			var row map[string]bigquery.Value
			if err := iter.Next(&row); err != nil {
				if err == iterator.Done {
					break
				}
				t.Fatalf("failed to read row after truncate: %v", err)
			}

			// Verify it's not the initial data
			if id, ok := row["id"].(int64); ok && id == 999 {
				t.Error("found initial data after WriteTruncate, expected only copied data")
			}

			rowCount++
		}

		if rowCount != 3 {
			t.Errorf("expected 3 rows after WriteTruncate, got %d", rowCount)
		}
	})

	// Test 4: Copy non-existent source table (should fail)
	t.Run("copy from non-existent table", func(t *testing.T) {
		srcTable := client.Dataset(datasetID).Table("nonexistent_table")
		dstTable := client.Dataset(datasetID).Table("dest_fail_table")

		copier := dstTable.CopierFrom(srcTable)
		job, err := copier.Run(ctx)
		if err != nil {
			// Expected to fail
			return
		}

		status, err := job.Wait(ctx)
		if err == nil && status.Err() == nil {
			t.Error("expected copy from non-existent table to fail, but it succeeded")
		}
	})

	// Test 5: Copy to existing table with WriteDisposition=WriteAppend
	t.Run("copy with write append", func(t *testing.T) {
		appendTableID := "append_table"

		// Create initial table with one row
		createQuery := client.Query(`
			CREATE TABLE ` + "`" + datasetID + "." + appendTableID + "`" + ` AS
			SELECT 100 AS id, "Initial" AS name, 50.0 AS value, true AS active`)

		job, err := createQuery.Run(ctx)
		if err != nil {
			t.Fatalf("failed to create initial table: %v", err)
		}

		status, err := job.Wait(ctx)
		if err != nil {
			t.Fatalf("failed to wait for initial table creation: %v", err)
		}

		if err := status.Err(); err != nil {
			t.Fatalf("initial table creation failed: %v", err)
		}

		// Copy with WriteAppend
		srcTable := client.Dataset(datasetID).Table(sourceTableID)
		dstTable := client.Dataset(datasetID).Table(appendTableID)

		copier := dstTable.CopierFrom(srcTable)
		copier.WriteDisposition = bigquery.WriteAppend

		job, err = copier.Run(ctx)
		if err != nil {
			t.Fatalf("failed to start copy job with append: %v", err)
		}

		status, err = job.Wait(ctx)
		if err != nil {
			t.Fatalf("copy job with append failed: %v", err)
		}

		if err := status.Err(); err != nil {
			t.Fatalf("copy job with append returned error: %v", err)
		}

		// Verify the destination table has both initial and copied data
		iter := dstTable.Read(ctx)
		rowCount := 0
		foundInitial := false
		for {
			var row map[string]bigquery.Value
			if err := iter.Next(&row); err != nil {
				if err == iterator.Done {
					break
				}
				t.Fatalf("failed to read row after append: %v", err)
			}

			if id, ok := row["id"].(int64); ok && id == 100 {
				foundInitial = true
			}

			rowCount++
		}

		if !foundInitial {
			t.Error("initial data not found after WriteAppend")
		}

		// Should have 1 initial row + 3 copied rows = 4 total
		if rowCount != 4 {
			t.Errorf("expected 4 rows after WriteAppend, got %d", rowCount)
		}
	})
}

// TestCopyJobWithComplexTypes tests copying tables with complex data types
func TestCopyJobWithComplexTypes(t *testing.T) {
	ctx := context.Background()

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}

	const (
		projectID     = "test"
		datasetID     = "complex_dataset"
		sourceTableID = "complex_source"
		destTableID   = "complex_dest"
	)

	// Create a source table with complex types
	project := types.NewProject(
		projectID,
		types.NewDataset(
			datasetID,
			types.NewTable(
				sourceTableID,
				[]*types.Column{
					types.NewColumn("id", types.INT64),
					types.NewColumn("tags", types.STRING, types.ColumnMode(types.RepeatedMode)),
					types.NewColumn(
						"metadata",
						types.STRUCT,
						types.ColumnFields(
							types.NewColumn("key", types.STRING),
							types.NewColumn("value", types.STRING),
						),
					),
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

	// Insert data with complex types
	insertQuery := client.Query(`
		INSERT INTO ` + "`" + datasetID + "." + sourceTableID + "`" + `
		(id, tags, metadata)
		VALUES (
			1,
			["tag1", "tag2", "tag3"],
			STRUCT("key1" AS key, "value1" AS value)
		)`)

	job, err := insertQuery.Run(ctx)
	if err != nil {
		t.Fatalf("failed to insert data: %v", err)
	}

	status, err := job.Wait(ctx)
	if err != nil {
		t.Fatalf("insert job failed: %v", err)
	}

	if err := status.Err(); err != nil {
		t.Fatalf("insert job returned error: %v", err)
	}

	// Copy the table
	srcTable := client.Dataset(datasetID).Table(sourceTableID)
	dstTable := client.Dataset(datasetID).Table(destTableID)

	copier := dstTable.CopierFrom(srcTable)
	job, err = copier.Run(ctx)
	if err != nil {
		t.Fatalf("failed to start copy job: %v", err)
	}

	status, err = job.Wait(ctx)
	if err != nil {
		t.Fatalf("copy job failed: %v", err)
	}

	if err := status.Err(); err != nil {
		t.Fatalf("copy job returned error: %v", err)
	}

	// Verify the copied data
	query := client.Query(`
		SELECT id, tags, metadata.key, metadata.value
		FROM ` + "`" + datasetID + "." + destTableID + "`")

	it, err := query.Read(ctx)
	if err != nil {
		t.Fatalf("failed to query destination table: %v", err)
	}

	var row struct {
		ID    int64
		Tags  []string
		Key   string
		Value string
	}

	if err := it.Next(&row); err != nil {
		if err == iterator.Done {
			t.Fatal("expected at least one row")
		}
		t.Fatalf("failed to read row: %v", err)
	}

	if row.ID != 1 {
		t.Errorf("expected ID=1, got %d", row.ID)
	}

	if len(row.Tags) != 3 || row.Tags[0] != "tag1" || row.Tags[1] != "tag2" || row.Tags[2] != "tag3" {
		t.Errorf("expected tags=[tag1, tag2, tag3], got %v", row.Tags)
	}

	if row.Key != "key1" || row.Value != "value1" {
		t.Errorf("expected metadata=(key1, value1), got (%s, %s)", row.Key, row.Value)
	}
}
