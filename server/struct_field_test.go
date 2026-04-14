package server_test

import (
	"context"
	"strings"
	"testing"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
)

// TestStructFieldNamesWithBackticks tests that STRUCT field names are properly
// quoted with backticks, which is essential for:
// 1. Field names that are SQL reserved keywords (e.g., "select", "from", "where", "current")
// 2. Field names with special characters (e.g., "field-name", "field.name")
// 3. Field names that would otherwise cause SQL parsing errors
//
// This test validates the fix in PR Recidiviz/bigquery-emulator#40.
func TestStructFieldNamesWithBackticks(t *testing.T) {
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

	// Create a table with STRUCT fields that have problematic names
	project := types.NewProject(
		projectID,
		types.NewDataset(
			datasetID,
			types.NewTable(
				tableID,
				[]*types.Column{
					types.NewColumn("id", types.INT64),
					// STRUCT with reserved keyword field names
					types.NewColumn(
						"metadata",
						types.STRUCT,
						types.ColumnFields(
							types.NewColumn("select", types.STRING),
							types.NewColumn("from", types.STRING),
							types.NewColumn("where", types.STRING),
							types.NewColumn("current", types.STRING),
						),
					),
					// STRUCT with special character field names
					types.NewColumn(
						"attributes",
						types.STRUCT,
						types.ColumnFields(
							types.NewColumn("field-with-dash", types.STRING),
							types.NewColumn("field.with.dots", types.INT64),
						),
					),
					// Nested STRUCT
					types.NewColumn(
						"nested_data",
						types.STRUCT,
						types.ColumnFields(
							types.NewColumn("outer", types.STRING),
							types.NewColumn(
								"inner",
								types.STRUCT,
								types.ColumnFields(
									types.NewColumn("inner_field", types.INT64),
								),
							),
						),
					),
					// Array of STRUCT
					types.NewColumn(
						"items",
						types.STRUCT,
						types.ColumnFields(
							types.NewColumn("item-id", types.INT64),
							types.NewColumn("item.name", types.STRING),
						),
						types.ColumnMode(types.RepeatedMode),
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

	// Test 0: Verify table schema was created correctly
	t.Run("verify table schema", func(t *testing.T) {
		table := client.Dataset(datasetID).Table(tableID)
		metadata, err := table.Metadata(ctx)
		if err != nil {
			t.Fatalf("failed to get table metadata: %v", err)
		}

		// Find the metadata column (STRUCT with reserved keyword field names)
		var metadataField *bigquery.FieldSchema
		for _, field := range metadata.Schema {
			if field.Name == "metadata" {
				metadataField = field
				break
			}
		}

		if metadataField == nil {
			t.Fatal("metadata field not found in schema")
		}

		if metadataField.Type != bigquery.RecordFieldType {
			t.Errorf("expected metadata to be RECORD type, got %v", metadataField.Type)
		}

		// Verify the nested field names exist
		expectedFields := []string{"select", "from", "where", "current"}
		foundFields := make(map[string]bool)
		for _, field := range metadataField.Schema {
			foundFields[field.Name] = true
		}

		for _, expected := range expectedFields {
			if !foundFields[expected] {
				t.Errorf("expected field %q in metadata STRUCT, but it was not found", expected)
			}
		}

		t.Logf("Table schema created successfully with %d nested fields in metadata STRUCT", len(metadataField.Schema))
	})

	// Test 1: Insert data using SQL to verify table was created with proper field names
	t.Run("insert data using SQL", func(t *testing.T) {
		insertSQL := `
			INSERT INTO ` + "`" + datasetID + "." + tableID + "`" + `
			(id, metadata, attributes, nested_data, items)
			VALUES (
				1,
				STRUCT("value1" AS ` + "`select`" + `, "value2" AS ` + "`from`" + `, "value3" AS ` + "`where`" + `, "value4" AS ` + "`current`" + `),
				STRUCT("dash-value" AS ` + "`field-with-dash`" + `, 42 AS ` + "`field.with.dots`" + `),
				STRUCT("outer-value" AS ` + "`outer`" + `, STRUCT(100 AS inner_field) AS ` + "`inner`" + `),
				[
					STRUCT(1 AS ` + "`item-id`" + `, "item1" AS ` + "`item.name`" + `),
					STRUCT(2 AS ` + "`item-id`" + `, "item2" AS ` + "`item.name`" + `)
				]
			)`

		job, err := client.Query(insertSQL).Run(ctx)
		if err != nil {
			t.Fatalf("failed to run insert query: %v", err)
		}

		status, err := job.Wait(ctx)
		if err != nil {
			t.Fatalf("failed to wait for insert job: %v", err)
		}

		if err := status.Err(); err != nil {
			t.Fatalf("insert job failed: %v", err)
		}
	})

	// Test 2: Query STRUCT fields with reserved keyword names
	t.Run("query struct with reserved keyword field names", func(t *testing.T) {
		// First, let's check if the data was actually inserted
		countQuery := client.Query(`SELECT COUNT(*) as count FROM ` + "`" + datasetID + "." + tableID + "`")
		countIt, err := countQuery.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute count query: %v", err)
		}
		var countRow struct{ Count int64 }
		if err := countIt.Next(&countRow); err != nil {
			t.Fatalf("failed to read count: %v", err)
		}
		t.Logf("Table has %d rows", countRow.Count)

		query := client.Query(`
				SELECT
					metadata.` + "`select`" + ` as select_field,
					metadata.` + "`from`" + ` as from_field,
					metadata.` + "`where`" + ` as where_field,
					metadata.` + "`current`" + ` as current_field
				FROM ` + "`" + datasetID + "." + tableID + "`")

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute query: %v", err)
		}

		var row struct {
			SelectField  string `bigquery:"select_field"`
			FromField    string `bigquery:"from_field"`
			WhereField   string `bigquery:"where_field"`
			CurrentField string `bigquery:"current_field"`
		}

		if err := it.Next(&row); err != nil {
			if err == iterator.Done {
				t.Fatal("expected at least one row")
			}
			t.Fatalf("failed to read row: %v", err)
		}

		if row.SelectField != "value1" || row.FromField != "value2" || row.WhereField != "value3" || row.CurrentField != "value4" {
			t.Errorf("unexpected values: got (%s, %s, %s, %s), want (value1, value2, value3, value4)",
				row.SelectField, row.FromField, row.WhereField, row.CurrentField)
		}
	})

	// Test 3: Query STRUCT fields with special characters
	t.Run("query struct with special character field names", func(t *testing.T) {
		query := client.Query(`
			SELECT
				attributes.` + "`field-with-dash`" + ` as dash_field,
				attributes.` + "`field.with.dots`" + ` as dots_field
			FROM ` + "`" + datasetID + "." + tableID + "`")

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute query: %v", err)
		}

		var row struct {
			DashField string `bigquery:"dash_field"`
			DotsField int64  `bigquery:"dots_field"`
		}

		if err := it.Next(&row); err != nil {
			if err == iterator.Done {
				t.Fatal("expected at least one row")
			}
			t.Fatalf("failed to read row: %v", err)
		}

		if row.DashField != "dash-value" || row.DotsField != 42 {
			t.Errorf("unexpected values: got (%s, %d), want (dash-value, 42)",
				row.DashField, row.DotsField)
		}
	})

	// Test 4: Query nested STRUCT
	t.Run("query nested struct", func(t *testing.T) {
		query := client.Query(`
			SELECT
				nested_data.outer,
				nested_data.inner.inner_field AS inner_field
			FROM ` + "`" + datasetID + "." + tableID + "`")

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute query: %v", err)
		}

		var row struct {
			Outer      string `bigquery:"outer"`
			InnerField int64  `bigquery:"inner_field"`
		}

		if err := it.Next(&row); err != nil {
			if err == iterator.Done {
				t.Fatal("expected at least one row")
			}
			t.Fatalf("failed to read row: %v", err)
		}

		if row.Outer != "outer-value" || row.InnerField != 100 {
			t.Errorf("unexpected values: got (%s, %d), want (outer-value, 100)",
				row.Outer, row.InnerField)
		}
	})

	// Test 5: Query array of STRUCT with special character field names
	t.Run("query array of struct with special character field names", func(t *testing.T) {
		// For arrays of structs, we need to transform the field names in SQL to valid Go identifiers
		query := client.Query(`
			SELECT ARRAY(
				SELECT AS STRUCT
					item.` + "`item-id`" + ` AS item_id,
					item.` + "`item.name`" + ` AS item_name
				FROM UNNEST(items) AS item
			) AS items
			FROM ` + "`" + datasetID + "." + tableID + "`")

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute query: %v", err)
		}

		// Define a struct type for each item in the array
		type Item struct {
			ItemID   int64  `bigquery:"item_id"`
			ItemName string `bigquery:"item_name"`
		}

		var row struct {
			Items []Item `bigquery:"items"`
		}

		if err := it.Next(&row); err != nil {
			if err == iterator.Done {
				t.Fatal("expected at least one row")
			}
			t.Fatalf("failed to read row: %v", err)
		}

		if len(row.Items) != 2 {
			t.Fatalf("expected 2 items, got %d", len(row.Items))
		}

		// Check first item
		if row.Items[0].ItemID != 1 || row.Items[0].ItemName != "item1" {
			t.Errorf("unexpected first item: got (id=%d, name=%s), want (id=1, name=item1)",
				row.Items[0].ItemID, row.Items[0].ItemName)
		}

		// Check second item
		if row.Items[1].ItemID != 2 || row.Items[1].ItemName != "item2" {
			t.Errorf("unexpected second item: got (id=%d, name=%s), want (id=2, name=item2)",
				row.Items[1].ItemID, row.Items[1].ItemName)
		}
	})

	// Test 6: Create a STRUCT literal in a query with backtick-requiring field names
	t.Run("create struct literal with reserved keyword fields", func(t *testing.T) {
		query := client.Query(`
			SELECT STRUCT(
				"test1" AS ` + "`select`" + `,
				"test2" AS ` + "`from`" + `,
				"test3" AS ` + "`where`" + `,
				"test4" AS ` + "`current`" + `
			) as test_struct`)

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute query: %v", err)
		}

		type TestStruct struct {
			Select  string `bigquery:"select"`
			From    string `bigquery:"from"`
			Where   string `bigquery:"where"`
			Current string `bigquery:"current"`
		}

		var row struct {
			TestStruct TestStruct `bigquery:"test_struct"`
		}

		if err := it.Next(&row); err != nil {
			if err == iterator.Done {
				t.Fatal("expected at least one row")
			}
			t.Fatalf("failed to read row: %v", err)
		}

		if row.TestStruct.Select != "test1" ||
			row.TestStruct.From != "test2" ||
			row.TestStruct.Where != "test3" ||
			row.TestStruct.Current != "test4" {
			t.Errorf("unexpected struct values: got (select=%s, from=%s, where=%s, current=%s), want (test1, test2, test3, test4)",
				row.TestStruct.Select, row.TestStruct.From, row.TestStruct.Where, row.TestStruct.Current)
		}
	})
}

// TestEmptyStructInsertion tests that loading JSON data with empty STRUCT values
// works correctly when IgnoreUnknownValues is set to true. This corresponds to
// the Python BigQuery client test that loads {"conductor": {}} into a table
// with a STRUCT field containing nested fields.
func TestEmptyStructInsertion(t *testing.T) {
	ctx := context.Background()

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}

	const (
		projectID = "test"
		datasetID = "ds1"
		tableID   = "t1"
	)

	// Create a table with a STRUCT field "conductor" containing a FLOAT64 field "length"
	project := types.NewProject(
		projectID,
		types.NewDataset(
			datasetID,
			types.NewTable(
				tableID,
				[]*types.Column{
					types.NewColumn(
						"conductor",
						types.STRUCT,
						types.ColumnFields(
							types.NewColumn("length", types.FLOAT64),
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

	// Test loading JSON with an empty struct {"conductor": {}}
	t.Run("load empty struct", func(t *testing.T) {
		jsonData := `{"conductor": {}}`

		source := bigquery.NewReaderSource(strings.NewReader(jsonData))
		source.SourceFormat = bigquery.JSON

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

		t.Log("Successfully loaded empty struct")
	})

	// Test querying the loaded data
	t.Run("query conductor.length from loaded data", func(t *testing.T) {
		query := client.Query("SELECT conductor.length FROM `" + datasetID + "." + tableID + "`")

		it, err := query.Read(ctx)
		if err != nil {
			t.Fatalf("failed to execute query: %v", err)
		}

		var row struct {
			Length bigquery.NullFloat64 `bigquery:"length"`
		}

		if err := it.Next(&row); err != nil {
			if err == iterator.Done {
				t.Fatal("expected at least one row")
			}
			t.Fatalf("failed to read row: %v", err)
		}

		// The empty struct should result in NULL for the length field
		if row.Length.Valid {
			t.Errorf("expected NULL for conductor.length, got %v", row.Length.Float64)
		}

		t.Logf("Query succeeded: conductor.length is NULL as expected for empty struct")
	})
}
