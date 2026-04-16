// FYI: https://cloud.google.com/bigquery/docs/reference/storage/libraries?hl=ja#client-libraries-usage-go
package server_test

import (
	"bytes"
	"cloud.google.com/go/civil"
	"context"
	"fmt"
	"io"
	"math/rand"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
	bqStorage "cloud.google.com/go/bigquery/storage/apiv1"
	storagepb "cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"cloud.google.com/go/bigquery/storage/managedwriter"
	"cloud.google.com/go/bigquery/storage/managedwriter/adapt"
	"github.com/GoogleCloudPlatform/golang-samples/bigquery/snippets/managedwriter/exampleproto"
	"github.com/apache/arrow-go/v18/arrow"
	"github.com/apache/arrow-go/v18/arrow/ipc"
	"github.com/apache/arrow-go/v18/arrow/memory"
	"github.com/goccy/go-json"
	gax "github.com/googleapis/gax-go/v2"
	goavro "github.com/linkedin/goavro/v2"
	"github.com/vantaboard/bigquery-emulator/server"
	bigqueryv2 "google.golang.org/api/bigquery/v2"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
	"google.golang.org/genproto/googleapis/rpc/errdetails"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"google.golang.org/protobuf/proto"

	"github.com/vantaboard/bigquery-emulator/types"
)

var (
	rpcOpts = gax.WithGRPCOptions(
		grpc.MaxCallRecvMsgSize(1024 * 1024 * 129),
	)
	outputColumns = []string{"id", "name", "structarr", "birthday", "skillNum", "created_at", "age", "birth_date", "wake_time", "score", "active", "metadata"}
)

func TestStorageReadAVRO(t *testing.T) {
	const (
		project = "test"
		dataset = "dataset1"
		table   = "table_a"
	)
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	if err := bqServer.Load(server.YAMLSource(filepath.Join("testdata", "data.yaml"))); err != nil {
		t.Fatal(err)
	}
	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()
	opts, err := testServer.GRPCClientOptions(ctx)
	if err != nil {
		t.Fatal(err)
	}
	bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
	if err != nil {
		t.Fatal(err)
	}
	defer bqReadClient.Close()

	readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", project, dataset, table)

	tableReadOptions := &storagepb.ReadSession_TableReadOptions{
		RowRestriction: `id = 1`,
	}

	createReadSessionRequest := &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", project),
		ReadSession: &storagepb.ReadSession{
			Table:       readTable,
			DataFormat:  storagepb.DataFormat_AVRO,
			ReadOptions: tableReadOptions,
		},
		MaxStreamCount: 1,
	}

	// Create the session from the request.
	session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
	if err != nil {
		t.Fatalf("CreateReadSession: %v", err)
	}
	if len(session.GetStreams()) == 0 {
		t.Fatalf("no streams in session.  if this was a small query result, consider writing to output to a named table.")
	}

	// We'll use only a single stream for reading data from the table.  Because
	// of dynamic sharding, this will yield all the rows in the table. However,
	// if you wanted to fan out multiple readers you could do so by having a
	// increasing the MaxStreamCount.
	readStream := session.GetStreams()[0].Name

	ch := make(chan *storagepb.ReadRowsResponse)

	// Use a waitgroup to coordinate the reading and decoding goroutines.
	var wg sync.WaitGroup

	// Start the reading in one goroutine.
	wg.Add(1)
	go func() {
		defer wg.Done()
		defer close(ch)
		if err := processStream(t, ctx, bqReadClient, readStream, ch); err != nil {
			t.Errorf("processStream failure: %v", err)
		}
	}()

	// Start Avro processing and decoding in another goroutine.
	wg.Add(1)
	go func() {
		defer wg.Done()
		if err := processAvro(t, ctx, session.GetAvroSchema().GetSchema(), ch); err != nil {
			t.Errorf("error processing %s: %v", storagepb.DataFormat_AVRO, err)
		}
	}()

	// Wait until both the reading and decoding goroutines complete.
	wg.Wait()
}

func TestStorageReadARROW(t *testing.T) {
	const (
		projectName = "test"
		dataset     = "dataset1"
		table       = "table_a"
	)
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	if err := bqServer.Load(server.YAMLSource(filepath.Join("testdata", "data.yaml"))); err != nil {
		t.Fatal(err)
	}
	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()
	opts, err := testServer.GRPCClientOptions(ctx)
	if err != nil {
		t.Fatal(err)
	}
	bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
	if err != nil {
		t.Fatal(err)
	}
	defer bqReadClient.Close()

	project := types.NewProject(projectName, types.NewDataset(dataset,
		types.NewTable("table1", []*types.Column{
			types.NewColumn("id", types.STRING),
		}, nil),
	))

	bqClient, err := buildClient(ctx, project, testServer)
	if err != nil {
		t.Fatal(err)
	}
	defer bqClient.Close()

	q := bqClient.Query(fmt.Sprintf("SELECT * FROM `%s`.`%s`", dataset, table))
	q.QueryConfig.Dst = &bigquery.Table{ProjectID: projectName, DatasetID: dataset, TableID: "table_a_materialized"}
	job, err := q.Run(ctx)
	if err != nil {
		t.Fatal(err)
	}
	_, err = job.Wait(ctx)
	if err != nil {
		t.Fatal(err)
	}

	readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectName, q.QueryConfig.Dst.DatasetID, q.QueryConfig.Dst.TableID)

	tableReadOptions := &storagepb.ReadSession_TableReadOptions{
		SelectedFields: outputColumns,
		RowRestriction: `id = 1`,
	}

	createReadSessionRequest := &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", projectName),
		ReadSession: &storagepb.ReadSession{
			Table:       readTable,
			DataFormat:  storagepb.DataFormat_ARROW,
			ReadOptions: tableReadOptions,
		},
		MaxStreamCount: 1,
	}

	// Create the session from the request.
	session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
	if err != nil {
		t.Fatalf("CreateReadSession: %v", err)
	}
	if len(session.GetStreams()) == 0 {
		t.Fatalf("no streams in session.  if this was a small query result, consider writing to output to a named table.")
	}

	// We'll use only a single stream for reading data from the table.  Because
	// of dynamic sharding, this will yield all the rows in the table. However,
	// if you wanted to fan out multiple readers you could do so by having a
	// increasing the MaxStreamCount.
	readStream := session.GetStreams()[0].Name

	ch := make(chan *storagepb.ReadRowsResponse)

	// Use a waitgroup to coordinate the reading and decoding goroutines.
	var wg sync.WaitGroup

	// Start the reading in one goroutine.
	wg.Add(1)
	go func() {
		defer wg.Done()
		defer close(ch)
		if err := processStream(t, ctx, bqReadClient, readStream, ch); err != nil {
			t.Errorf("processStream failure: %v", err)
		}
	}()

	// Start Arrow processing and decoding in another goroutine.
	wg.Add(1)
	go func() {
		defer wg.Done()
		if err := processArrow(t, ctx, session.GetArrowSchema().GetSerializedSchema(), ch); err != nil {
			t.Errorf("error processing %s: %v", storagepb.DataFormat_ARROW, err)
		}
	}()

	// Wait until both the reading and decoding goroutines complete.
	wg.Wait()
}

func processStream(t *testing.T, ctx context.Context, client *bqStorage.BigQueryReadClient, st string, ch chan<- *storagepb.ReadRowsResponse) error {
	var offset int64

	// Streams may be long-running.  Rather than using a global retry for the
	// stream, implement a retry that resets once progress is made.
	retryLimit := 3
	retries := 0
	for {
		// Send the initiating request to start streaming row blocks.
		rowStream, err := client.ReadRows(ctx, &storagepb.ReadRowsRequest{
			ReadStream: st,
			Offset:     offset,
		}, rpcOpts)
		if err != nil {
			return fmt.Errorf("couldn't invoke ReadRows: %v", err)
		}

		// Process the streamed responses.
		for {
			r, err := rowStream.Recv()
			if err == io.EOF {
				return nil
			}
			if err != nil {
				// If there is an error, check whether it is a retryable
				// error with a retry delay and sleep instead of increasing
				// retries count.
				var retryDelayDuration time.Duration
				if errorStatus, ok := status.FromError(err); ok && errorStatus.Code() == codes.ResourceExhausted {
					for _, detail := range errorStatus.Details() {
						retryInfo, ok := detail.(*errdetails.RetryInfo)
						if !ok {
							continue
						}
						retryDelay := retryInfo.GetRetryDelay()
						retryDelayDuration = time.Duration(retryDelay.Seconds)*time.Second + time.Duration(retryDelay.Nanos)*time.Nanosecond
						break
					}
				}
				if retryDelayDuration != 0 {
					t.Fatalf("processStream failed with a retryable error, retrying in %v", retryDelayDuration)
					time.Sleep(retryDelayDuration)
				} else {
					retries++
					if retries >= retryLimit {
						return fmt.Errorf("processStream retries exhausted: %v", err)
					}
				}
				// break the inner loop, and try to recover by starting a new streaming
				// ReadRows call at the last known good offset.
				break
			} else {
				// Reset retries after a successful response.
				retries = 0
			}

			rc := r.GetRowCount()
			if rc > 0 {
				// Bookmark our progress in case of retries and send the rowblock on the channel.
				offset = offset + rc
				// We're making progress, reset retries.
				retries = 0
				ch <- r
			}
		}
	}
}

// processAvro receives row blocks from a channel, and uses the provided Avro
// schema to decode the blocks into individual row messages for printing.  Will
// continue to run until the channel is closed or the provided context is
// cancelled.
func processAvro(t *testing.T, ctx context.Context, schema string, ch <-chan *storagepb.ReadRowsResponse) error {
	// Establish a decoder that can process blocks of messages using the
	// reference schema. All blocks share the same schema, so the decoder
	// can be long-lived.
	codec, err := goavro.NewCodec(schema)
	if err != nil {
		return fmt.Errorf("couldn't create codec: %v", err)
	}

	for {
		select {
		case <-ctx.Done():
			// Context was cancelled.  Stop.
			return ctx.Err()
		case rows, ok := <-ch:
			if !ok {
				// Channel closed, no further avro messages.  Stop.
				return nil
			}
			undecoded := rows.GetAvroRows().GetSerializedBinaryRows()
			for len(undecoded) > 0 {
				datum, remainingBytes, err := codec.NativeFromBinary(undecoded)

				if err != nil {
					if err == io.EOF {
						break
					}
					return fmt.Errorf("decoding error with %d bytes remaining: %v", len(undecoded), err)
				}
				validateDatum(t, datum)
				undecoded = remainingBytes
			}
		}
	}
}

func processArrow(t *testing.T, ctx context.Context, schema []byte, ch <-chan *storagepb.ReadRowsResponse) error {
	mem := memory.NewGoAllocator()
	buf := bytes.NewBuffer(schema)
	r, err := ipc.NewReader(buf, ipc.WithAllocator(mem))
	if err != nil {
		return err
	}
	aschema := r.Schema()
	for {
		select {
		case <-ctx.Done():
			// Context was cancelled.  Stop.
			return ctx.Err()
		case rows, ok := <-ch:
			if !ok {
				// Channel closed, no further arrow messages.  Stop.
				return nil
			}
			undecoded := rows.GetArrowRecordBatch().GetSerializedRecordBatch()
			if len(undecoded) > 0 {
				// Prepend the schema to the record batch data
				// This is the expected format for BigQuery Storage API
				buf = bytes.NewBuffer(schema)
				buf.Write(undecoded)
				r, err = ipc.NewReader(buf, ipc.WithAllocator(mem), ipc.WithSchema(aschema))
				if err != nil {
					return err
				}
				for r.Next() {
					rec := r.RecordBatch()
					validateArrowRecord(t, rec)
				}
			}
		}
	}
}

func validateArrowRecord(t *testing.T, record arrow.Record) {
	out, err := record.MarshalJSON()
	if err != nil {
		t.Fatal(err)
	}
	list := []map[string]interface{}{}
	if err := json.Unmarshal(out, &list); err != nil {
		t.Fatal(err)
	}
	if len(list) == 0 {
		t.Fatal("failed to get arrow record")
	}
	first := list[0]
	if len(first) != len(outputColumns) {
		t.Fatalf("failed to get arrow record %+v", first)
	}
}

func validateDatum(t *testing.T, d interface{}) {
	m, ok := d.(map[string]interface{})
	if !ok {
		t.Logf("failed type assertion: %v", d)
	}
	if len(m) != len(outputColumns) {
		t.Fatalf("failed to receive table data. expected columns %v but got %v", outputColumns, m)
	}
}

func TestStorageWrite(t *testing.T) {
	for _, test := range []struct {
		name                            string
		streamType                      storagepb.WriteStream_Type
		isDefaultStream                 bool
		expectedRowsAfterFirstWrite     int
		expectedRowsAfterSecondWrite    int
		expectedRowsAfterThirdWrite     int
		expectedRowsAfterExplicitCommit int
	}{
		{
			name:                            "pending",
			streamType:                      storagepb.WriteStream_PENDING,
			expectedRowsAfterFirstWrite:     0,
			expectedRowsAfterSecondWrite:    0,
			expectedRowsAfterThirdWrite:     0,
			expectedRowsAfterExplicitCommit: 6,
		},
		{
			name:                            "committed",
			streamType:                      storagepb.WriteStream_COMMITTED,
			expectedRowsAfterFirstWrite:     1,
			expectedRowsAfterSecondWrite:    4,
			expectedRowsAfterThirdWrite:     6,
			expectedRowsAfterExplicitCommit: 6,
		},
		{
			name:                            "default",
			streamType:                      storagepb.WriteStream_COMMITTED,
			isDefaultStream:                 true,
			expectedRowsAfterFirstWrite:     1,
			expectedRowsAfterSecondWrite:    4,
			expectedRowsAfterThirdWrite:     6,
			expectedRowsAfterExplicitCommit: 6,
		},
	} {
		const (
			projectID = "test"
			datasetID = "test"
			tableID   = "sample"
		)

		ctx := context.Background()
		bqServer, err := server.New(server.TempStorage)
		if err != nil {
			t.Fatal(err)
		}
		if err := bqServer.Load(
			server.StructSource(
				types.NewProject(
					projectID,
					types.NewDataset(
						datasetID,
						types.NewTable(
							tableID,
							[]*types.Column{
								types.NewColumn("bool_col", types.BOOL),
								types.NewColumn("bytes_col", types.BYTES),
								types.NewColumn("float64_col", types.FLOAT64),
								types.NewColumn("int64_col", types.INT64),
								types.NewColumn("string_col", types.STRING),
								types.NewColumn("date_col", types.DATE),
								types.NewColumn("datetime_col", types.DATETIME),
								types.NewColumn("geography_col", types.GEOGRAPHY),
								types.NewColumn("numeric_col", types.NUMERIC),
								types.NewColumn("bignumeric_col", types.BIGNUMERIC),
								types.NewColumn("time_col", types.TIME),
								types.NewColumn("timestamp_col", types.TIMESTAMP),
								types.NewColumn("int64_list", types.INT64, types.ColumnMode(types.RepeatedMode)),
								types.NewColumn(
									"struct_col",
									types.STRUCT,
									types.ColumnFields(
										types.NewColumn("sub_int_col", types.INT64),
									),
								),
								types.NewColumn(
									"struct_list",
									types.STRUCT,
									types.ColumnFields(
										types.NewColumn("sub_int_col", types.INT64),
									),
									types.ColumnMode(types.RepeatedMode),
								),
							},
							nil,
						),
					),
				),
			),
		); err != nil {
			t.Fatal(err)
		}
		testServer := bqServer.TestServer()
		defer func() {
			testServer.Close()
			bqServer.Close()
		}()
		opts, err := testServer.GRPCClientOptions(ctx)
		if err != nil {
			t.Fatal(err)
		}

		client, err := managedwriter.NewClient(ctx, projectID, opts...)
		if err != nil {
			t.Fatal(err)
		}
		defer client.Close()
		t.Run(test.name, func(t *testing.T) {
			var writeStreamName string
			fullTableName := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectID, datasetID, tableID)
			if !test.isDefaultStream {
				writeStream, err := client.CreateWriteStream(ctx, &storagepb.CreateWriteStreamRequest{
					Parent: fullTableName,
					WriteStream: &storagepb.WriteStream{
						Type: test.streamType,
					},
				})
				if err != nil {
					t.Fatalf("CreateWriteStream: %v", err)
				}
				writeStreamName = writeStream.GetName()
			}
			m := &exampleproto.SampleData{}
			descriptorProto, err := adapt.NormalizeDescriptor(m.ProtoReflect().Descriptor())
			if err != nil {
				t.Fatalf("NormalizeDescriptor: %v", err)
			}
			var writerOptions []managedwriter.WriterOption
			if test.isDefaultStream {
				writerOptions = append(writerOptions, managedwriter.WithType(managedwriter.DefaultStream))
				writerOptions = append(writerOptions, managedwriter.WithDestinationTable(fullTableName))
			} else {
				writerOptions = append(writerOptions, managedwriter.WithStreamName(writeStreamName))
			}
			writerOptions = append(writerOptions, managedwriter.WithSchemaDescriptor(descriptorProto))
			managedStream, err := client.NewManagedStream(
				ctx,
				writerOptions...,
			)
			if err != nil {
				t.Fatalf("NewManagedStream: %v", err)
			}

			bqClient, err := bigquery.NewClient(
				ctx,
				projectID,
				option.WithEndpoint(testServer.URL),
				option.WithoutAuthentication(),
			)
			if err != nil {
				t.Fatal(err)
			}
			defer bqClient.Close()

			rows, err := generateExampleMessages(1)
			if err != nil {
				t.Fatalf("generateExampleMessages: %v", err)
			}

			var (
				curOffset int64
				results   []*managedwriter.AppendResult
			)
			result, err := managedStream.AppendRows(ctx, rows, managedwriter.WithOffset(0))
			if err != nil {
				t.Fatalf("AppendRows first call error: %v", err)
			}
			// Wait for result to ensure data is committed before checking row count
			if _, err := result.GetResult(ctx); err != nil {
				t.Fatalf("AppendRows first call result error: %v", err)
			}

			iter := bqClient.Dataset(datasetID).Table(tableID).Read(ctx)
			resultRowCount := countRows(t, iter)
			if resultRowCount != test.expectedRowsAfterFirstWrite {
				t.Fatalf("expected the number of rows after first AppendRows %d but got %d", test.expectedRowsAfterFirstWrite, resultRowCount)
			}

			results = append(results, result)
			curOffset = curOffset + 1
			rows, err = generateExampleMessages(3)
			if err != nil {
				t.Fatalf("generateExampleMessages: %v", err)
			}
			result, err = managedStream.AppendRows(ctx, rows, managedwriter.WithOffset(curOffset))
			if err != nil {
				t.Fatalf("AppendRows second call error: %v", err)
			}
			// Wait for result to ensure data is committed before checking row count
			if _, err := result.GetResult(ctx); err != nil {
				t.Fatalf("AppendRows second call result error: %v", err)
			}

			iter = bqClient.Dataset(datasetID).Table(tableID).Read(ctx)
			resultRowCount = countRows(t, iter)
			if resultRowCount != test.expectedRowsAfterSecondWrite {
				t.Fatalf("expected the number of rows after second AppendRows %d but got %d", test.expectedRowsAfterSecondWrite, resultRowCount)
			}

			results = append(results, result)
			curOffset = curOffset + 3
			rows, err = generateExampleMessages(2)
			if err != nil {
				t.Fatalf("generateExampleMessages: %v", err)
			}
			result, err = managedStream.AppendRows(ctx, rows, managedwriter.WithOffset(curOffset))
			if err != nil {
				t.Fatalf("AppendRows third call error: %v", err)
			}
			results = append(results, result)

			for k, v := range results {
				recvOffset, err := v.GetResult(ctx)
				if err != nil {
					t.Fatalf("append %d returned error: %v", k, err)
				}
				t.Logf("Successfully appended data at offset %d", recvOffset)
			}

			iter = bqClient.Dataset(datasetID).Table(tableID).Read(ctx)
			resultRowCount = countRows(t, iter)
			if resultRowCount != test.expectedRowsAfterThirdWrite {
				t.Fatalf("expected the number of rows after third AppendRows %d but got %d", test.expectedRowsAfterThirdWrite, resultRowCount)
			}

			rowCount, err := managedStream.Finalize(ctx)
			if err != nil {
				t.Fatalf("error during Finalize: %v", err)
			}

			t.Logf("Stream %s finalized with %d rows", managedStream.StreamName(), rowCount)

			req := &storagepb.BatchCommitWriteStreamsRequest{
				Parent:       managedwriter.TableParentFromStreamName(managedStream.StreamName()),
				WriteStreams: []string{managedStream.StreamName()},
			}

			resp, err := client.BatchCommitWriteStreams(ctx, req)
			if err != nil {
				t.Fatalf("client.BatchCommit: %v", err)
			}
			if len(resp.GetStreamErrors()) > 0 {
				t.Fatalf("stream errors present: %v", resp.GetStreamErrors())
			}

			iter = bqClient.Dataset(datasetID).Table(tableID).Read(ctx)
			resultRowCount = countRows(t, iter)
			if resultRowCount != test.expectedRowsAfterExplicitCommit {
				t.Fatalf("expected the number of rows after Finalize %d but got %d", test.expectedRowsAfterExplicitCommit, resultRowCount)
			}

			t.Logf("Table data committed at %s", resp.GetCommitTime().AsTime().Format(time.RFC3339Nano))
		})
	}
}

func countRows(t *testing.T, iter *bigquery.RowIterator) int {
	var resultRowCount int
	for {
		v := map[string]bigquery.Value{}
		if err := iter.Next(&v); err != nil {
			if err == iterator.Done {
				break
			}
			t.Fatal(err)
		}
		resultRowCount++
	}
	return resultRowCount
}

// TestArrayFieldSerialization verifies that array fields are correctly serialized to Arrow format.
// This is a regression test for GitHub issue #399 where array fields caused a panic:
// "panic: arrow/array: field 2 has 6 rows. want=3"
// The bug was caused by calling listBuilder.Append() once per array element instead of once per row.
func TestArrayFieldSerialization(t *testing.T) {
	const (
		projectID = "test"
		datasetID = "testdataset"
		tableID   = "testtable"
	)

	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()

	// Load test data with array fields
	if err := bqServer.Load(
		server.StructSource(
			types.NewProject(
				projectID,
				types.NewDataset(
					datasetID,
					types.NewTable(
						tableID,
						[]*types.Column{
							types.NewColumn("x", types.INT64),
							types.NewColumn("y", types.STRING),
							types.NewColumn("a", types.INT64, types.ColumnMode(types.RepeatedMode)),
						},
						types.Data{
							{
								"x": 1,
								"y": "1 str",
								"a": []interface{}{1, 2, 3},
							},
							{
								"x": 2,
								"y": "2nd str",
								"a": []interface{}{}, // Empty array
							},
							{
								"x": 33,
								"y": "3rd string",
								"a": []interface{}{0, 0, 0},
							},
						},
					),
				),
			),
		),
	); err != nil {
		t.Fatal(err)
	}

	opts, err := testServer.GRPCClientOptions(ctx)
	if err != nil {
		t.Fatal(err)
	}
	bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
	if err != nil {
		t.Fatal(err)
	}
	defer bqReadClient.Close()

	readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectID, datasetID, tableID)

	// Select all columns including the array field
	tableReadOptions := &storagepb.ReadSession_TableReadOptions{
		SelectedFields: []string{"x", "y", "a"},
	}

	createReadSessionRequest := &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", projectID),
		ReadSession: &storagepb.ReadSession{
			Table:       readTable,
			DataFormat:  storagepb.DataFormat_ARROW,
			ReadOptions: tableReadOptions,
		},
		MaxStreamCount: 1,
	}

	session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
	if err != nil {
		t.Fatalf("CreateReadSession failed: %v", err)
	}

	if len(session.GetStreams()) != 1 {
		t.Fatalf("expected 1 stream but got %d", len(session.GetStreams()))
	}

	readStream := session.GetStreams()[0].Name

	// This is where the bug would occur - reading rows with array fields
	rowStream, err := bqReadClient.ReadRows(ctx, &storagepb.ReadRowsRequest{
		ReadStream: readStream,
	}, rpcOpts)
	if err != nil {
		t.Fatalf("failed to create ReadRows stream: %v", err)
	}

	// Get the schema for decoding
	serializedSchema := session.GetArrowSchema().GetSerializedSchema()
	mem := memory.NewGoAllocator()
	buf := bytes.NewBuffer(serializedSchema)
	schemaReader, err := ipc.NewReader(buf, ipc.WithAllocator(mem))
	if err != nil {
		t.Fatalf("Failed to read schema: %v", err)
	}
	aschema := schemaReader.Schema()

	totalRows := 0
	for {
		resp, err := rowStream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			t.Fatalf("rowStream.Recv error: %v", err)
		}

		arrowRecordBatch := resp.GetArrowRecordBatch()
		if arrowRecordBatch != nil {
			serializedBatch := arrowRecordBatch.GetSerializedRecordBatch()
			if len(serializedBatch) > 0 {
				buf = bytes.NewBuffer(serializedSchema)
				buf.Write(serializedBatch)

				reader, err := ipc.NewReader(buf, ipc.WithAllocator(mem), ipc.WithSchema(aschema))
				if err != nil {
					t.Fatalf("Failed to create Arrow reader: %v", err)
				}

				for reader.Next() {
					rec := reader.RecordBatch()
					totalRows += int(rec.NumRows())

					// Verify we have the expected 3 columns
					if rec.NumCols() != 3 {
						t.Fatalf("Expected 3 columns, got %d", rec.NumCols())
					}

					// Verify the array column exists and can be accessed
					arrayCol := rec.Column(2) // The 'a' column
					if arrayCol == nil {
						t.Fatal("array column is nil")
					}

					t.Logf("Successfully read %d rows with array field", rec.NumRows())
				}

				if err := reader.Err(); err != nil {
					t.Fatalf("Arrow reader error: %v", err)
				}
			}
		}
	}

	// Verify we read all 3 rows
	if totalRows != 3 {
		t.Fatalf("Expected to read 3 rows, got %d", totalRows)
	}

	t.Log("Successfully validated array field serialization in Arrow format")
}

// TestNilReadOptions verifies that the emulator handles nil ReadOptions without panicking.
// This is a regression test for a bug where requests without ReadOptions would cause
// a nil pointer dereference.
func TestNilReadOptions(t *testing.T) {
	const (
		projectName = "test"
		dataset     = "dataset1"
		table       = "table_a"
	)

	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	if err := bqServer.Load(server.YAMLSource(filepath.Join("testdata", "data.yaml"))); err != nil {
		t.Fatal(err)
	}
	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()

	opts, err := testServer.GRPCClientOptions(ctx)
	if err != nil {
		t.Fatal(err)
	}
	bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
	if err != nil {
		t.Fatal(err)
	}
	defer bqReadClient.Close()

	readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectName, dataset, table)

	// Create a request with nil ReadOptions - this should not panic
	createReadSessionRequest := &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", projectName),
		ReadSession: &storagepb.ReadSession{
			Table:       readTable,
			DataFormat:  storagepb.DataFormat_ARROW,
			ReadOptions: nil, // Explicitly nil
		},
		MaxStreamCount: 1,
	}

	session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
	if err != nil {
		t.Fatalf("CreateReadSession with nil ReadOptions failed: %v", err)
	}

	// Verify the session was created successfully
	if len(session.GetStreams()) != 1 {
		t.Fatalf("expected 1 stream but got %d", len(session.GetStreams()))
	}

	// Verify we can read from the stream (should return all columns)
	readStream := session.GetStreams()[0].Name
	rowStream, err := bqReadClient.ReadRows(ctx, &storagepb.ReadRowsRequest{
		ReadStream: readStream,
	}, rpcOpts)
	if err != nil {
		t.Fatalf("failed to create ReadRows stream: %v", err)
	}

	// Try to read at least one response to verify the stream works
	resp, err := rowStream.Recv()
	if err != nil && err != io.EOF {
		t.Fatalf("failed to read from stream: %v", err)
	}
	if resp != nil {
		t.Logf("Successfully read from stream with nil ReadOptions, got %d row(s)", resp.RowCount)
	}
}

// TestStreamCountNormalization verifies that the emulator correctly handles
// MaxStreamCount requests and normalizes them to the supported count.
// This is a regression test for a bug where Python clients requesting multiple
// streams would encounter issues.
func TestStreamCountNormalization(t *testing.T) {
	const (
		projectName = "test"
		dataset     = "dataset1"
		table       = "table_a"
	)

	tests := []struct {
		name                string
		maxStreamCount      int32
		expectError         bool
		expectedStreamCount int
		errorContains       string
	}{
		{
			name:                "request with MaxStreamCount = 0 should normalize to 1",
			maxStreamCount:      0,
			expectError:         false,
			expectedStreamCount: 1,
		},
		{
			name:                "request with MaxStreamCount = 1 should work",
			maxStreamCount:      1,
			expectError:         false,
			expectedStreamCount: 1,
		},
		{
			name:           "request with MaxStreamCount = 2 should error",
			maxStreamCount: 2,
			expectError:    true,
			errorContains:  "currently supports only 1 stream(s)",
		},
		{
			name:           "request with MaxStreamCount = 10 should error (Python client scenario)",
			maxStreamCount: 10,
			expectError:    true,
			errorContains:  "currently supports only 1 stream(s)",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			ctx := context.Background()
			bqServer, err := server.New(server.TempStorage)
			if err != nil {
				t.Fatal(err)
			}
			if err := bqServer.Load(server.YAMLSource(filepath.Join("testdata", "data.yaml"))); err != nil {
				t.Fatal(err)
			}
			testServer := bqServer.TestServer()
			defer func() {
				testServer.Close()
				bqServer.Close()
			}()

			opts, err := testServer.GRPCClientOptions(ctx)
			if err != nil {
				t.Fatal(err)
			}
			bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
			if err != nil {
				t.Fatal(err)
			}
			defer bqReadClient.Close()

			readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectName, dataset, table)

			createReadSessionRequest := &storagepb.CreateReadSessionRequest{
				Parent: fmt.Sprintf("projects/%s", projectName),
				ReadSession: &storagepb.ReadSession{
					Table:      readTable,
					DataFormat: storagepb.DataFormat_ARROW,
					ReadOptions: &storagepb.ReadSession_TableReadOptions{
						SelectedFields: []string{"id", "name"},
					},
				},
				MaxStreamCount: tt.maxStreamCount,
			}

			session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)

			if tt.expectError {
				if err == nil {
					t.Fatalf("expected error containing %q but got none", tt.errorContains)
				}
				if !strings.Contains(err.Error(), tt.errorContains) {
					t.Fatalf("expected error containing %q but got: %v", tt.errorContains, err)
				}
				t.Logf("Got expected error: %v", err)
				return
			}

			// For successful cases
			if err != nil {
				t.Fatalf("unexpected error: %v", err)
			}

			// Verify the stream count
			actualStreamCount := len(session.GetStreams())
			if actualStreamCount != tt.expectedStreamCount {
				t.Fatalf("expected %d stream(s) but got %d", tt.expectedStreamCount, actualStreamCount)
			}

			// Verify we can actually read from the stream
			if actualStreamCount > 0 {
				readStream := session.GetStreams()[0].Name
				rowStream, err := bqReadClient.ReadRows(ctx, &storagepb.ReadRowsRequest{
					ReadStream: readStream,
				}, rpcOpts)
				if err != nil {
					t.Fatalf("failed to create ReadRows stream: %v", err)
				}

				// Try to read at least one response to verify the stream works
				resp, err := rowStream.Recv()
				if err != nil && err != io.EOF {
					t.Fatalf("failed to read from stream: %v", err)
				}
				if resp != nil {
					t.Logf("Successfully read from normalized stream with %d row(s)", resp.RowCount)
				}
			}
		})
	}
}

func generateExampleMessages(numMessages int) ([][]byte, error) {
	msgs := make([][]byte, numMessages)
	for i := 0; i < numMessages; i++ {

		random := rand.New(rand.NewSource(time.Now().UnixNano()))

		// Our example data embeds an array of structs, so we'll construct that first.
		sList := make([]*exampleproto.SampleStruct, 5)
		for i := 0; i < int(random.Int63n(5)+1); i++ {
			sList[i] = &exampleproto.SampleStruct{
				SubIntCol: proto.Int64(random.Int63()),
			}
		}

		m := &exampleproto.SampleData{
			BoolCol:    proto.Bool(true),
			BytesCol:   []byte("some bytes"),
			Float64Col: proto.Float64(3.14),
			Int64Col:   proto.Int64(123),
			StringCol:  proto.String("example string value"),

			// These types require special encoding/formatting to transmit.

			// DATE values are number of days since the Unix epoch.

			DateCol: proto.Int32(int32(time.Now().UnixNano() / 86400000000000)),

			// DATETIME uses the literal format.
			DatetimeCol: proto.String("2022-01-01 12:13:14.000000"),

			// GEOGRAPHY uses Well-Known-Text (WKT) format.
			GeographyCol: proto.String("POINT(-122.350220 47.649154)"),

			// NUMERIC and BIGNUMERIC can be passed as string, or more efficiently
			// using a packed byte representation.
			NumericCol:    proto.String("99999999999999999999999999999.999999999"),
			BignumericCol: proto.String("-578960446186580977117854925043439539266.34992332820282019728792003956564819968"),

			// TIME also uses literal format.
			TimeCol: proto.String("12:13:14.000000"),

			// TIMESTAMP uses microseconds since Unix epoch.
			TimestampCol: proto.Int64(time.Now().UnixNano() / 1000),

			// Int64List is an array of INT64 types.
			Int64List: []int64{2, 4, 6, 8},

			// This is a required field, and thus must be present.
			RowNum: proto.Int64(23),

			// StructCol is a single nested message.
			StructCol: &exampleproto.SampleStruct{
				SubIntCol: proto.Int64(random.Int63()),
			},

			// StructList is a repeated array of a nested message.
			StructList: sList,
		}
		b, err := proto.Marshal(m)
		if err != nil {
			return nil, fmt.Errorf("error generating message %d: %w", i, err)
		}
		msgs[i] = b
	}
	return msgs, nil
}

// TestDatetimeTimezoneNaive verifies that DATETIME values are serialized
// without timezone information (i.e., location-naive).
// This is a regression test for a bug where DATETIME values were incorrectly
// serialized with timezone information in Arrow format.
func TestDatetimeTimezoneNaive(t *testing.T) {
	const (
		projectName = "test"
		dataset     = "dataset1"
		table       = "table_a"
	)
	ctx := context.Background()
	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}
	if err := bqServer.Load(server.YAMLSource(filepath.Join("testdata", "data.yaml"))); err != nil {
		t.Fatal(err)
	}
	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()
	opts, err := testServer.GRPCClientOptions(ctx)
	if err != nil {
		t.Fatal(err)
	}
	bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
	if err != nil {
		t.Fatal(err)
	}
	defer bqReadClient.Close()

	readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectName, dataset, table)

	// Select the birthday column which is DATETIME type
	tableReadOptions := &storagepb.ReadSession_TableReadOptions{
		SelectedFields: []string{"id", "birthday"},
		RowRestriction: `id = 1`,
	}

	createReadSessionRequest := &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", projectName),
		ReadSession: &storagepb.ReadSession{
			Table:       readTable,
			DataFormat:  storagepb.DataFormat_ARROW,
			ReadOptions: tableReadOptions,
		},
		MaxStreamCount: 1,
	}

	// Create the session from the request.
	session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
	if err != nil {
		t.Fatalf("CreateReadSession: %v", err)
	}
	if len(session.GetStreams()) == 0 {
		t.Fatalf("no streams in session")
	}

	readStream := session.GetStreams()[0].Name

	// Use the BigQuery Storage client to read rows
	rowStream, err := bqReadClient.ReadRows(ctx, &storagepb.ReadRowsRequest{
		ReadStream: readStream,
	}, rpcOpts)
	if err != nil {
		t.Fatalf("ReadRows: %v", err)
	}

	// Get the schema for decoding
	serializedSchema := session.GetArrowSchema().GetSerializedSchema()
	mem := memory.NewGoAllocator()
	buf := bytes.NewBuffer(serializedSchema)
	schemaReader, err := ipc.NewReader(buf, ipc.WithAllocator(mem))
	if err != nil {
		t.Fatalf("Failed to read schema: %v", err)
	}
	aschema := schemaReader.Schema()

	// Verify the birthday field is timezone-naive
	birthdayFieldIdx := aschema.FieldIndices("birthday")
	if len(birthdayFieldIdx) == 0 {
		t.Fatal("birthday field not found in schema")
	}
	birthdayField := aschema.Field(birthdayFieldIdx[0])

	// The type should be a TimestampType
	tsType, ok := birthdayField.Type.(*arrow.TimestampType)
	if !ok {
		t.Fatalf("birthday field is not a TimestampType, got %T", birthdayField.Type)
	}

	// Verify it has no timezone (should be empty string for location-naive)
	if tsType.TimeZone != "" {
		t.Fatalf("birthday field should be timezone-naive but has timezone: %s", tsType.TimeZone)
	}

	// Verify the unit is microseconds
	if tsType.Unit != arrow.Microsecond {
		t.Fatalf("birthday field should use microsecond unit, got %v", tsType.Unit)
	}

	// Also read and verify the actual data can be decoded properly
	foundData := false
	for {
		resp, err := rowStream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			t.Fatalf("rowStream.Recv: %v", err)
		}

		arrowRecordBatch := resp.GetArrowRecordBatch()
		if arrowRecordBatch != nil {
			serializedBatch := arrowRecordBatch.GetSerializedRecordBatch()
			if len(serializedBatch) > 0 {
				buf = bytes.NewBuffer(serializedSchema)
				buf.Write(serializedBatch)

				reader, err := ipc.NewReader(buf, ipc.WithAllocator(mem), ipc.WithSchema(aschema))
				if err != nil {
					t.Fatalf("Failed to create Arrow reader: %v", err)
				}

				for reader.Next() {
					rec := reader.RecordBatch()
					if rec.NumRows() > 0 {
						foundData = true

						// Verify we can read the birthday column
						birthdayCol := rec.Column(birthdayFieldIdx[0])
						if birthdayCol == nil {
							t.Fatal("birthday column is nil")
						}

						// Verify the column is a valid Arrow array (successfully decoded)
						t.Logf("Successfully decoded birthday column as %T with %d rows", birthdayCol, rec.NumRows())
					}
				}

				if err := reader.Err(); err != nil {
					t.Fatalf("Arrow reader error: %v", err)
				}
			}
		}
	}

	if !foundData {
		t.Fatal("Expected to read at least one row with birthday data")
	}

	t.Log("Successfully validated that DATETIME values are timezone-naive in Arrow format")
}

func TestStorageReadWithAPICreatedTable(t *testing.T) {
	for _, format := range []struct {
		name       string
		dataFormat storagepb.DataFormat
	}{
		{name: "AVRO", dataFormat: storagepb.DataFormat_AVRO},
		{name: "ARROW", dataFormat: storagepb.DataFormat_ARROW},
	} {
		t.Run(format.name, func(t *testing.T) {
			const (
				projectID = "test"
				datasetID = "test_dataset"
				tableID   = "test_table"
			)

			ctx := context.Background()

			// Create empty server with just project and dataset
			bqServer, err := server.New(server.TempStorage)
			if err != nil {
				t.Fatal(err)
			}
			project := types.NewProject(projectID, types.NewDataset(datasetID))
			if err := bqServer.Load(server.StructSource(project)); err != nil {
				t.Fatal(err)
			}

			testServer := bqServer.TestServer()
			defer func() {
				testServer.Close()
				bqServer.Close()
			}()

			// Create BigQuery client
			bqClient, err := bigquery.NewClient(
				ctx,
				projectID,
				option.WithEndpoint(testServer.URL),
				option.WithoutAuthentication(),
			)
			if err != nil {
				t.Fatal(err)
			}
			defer bqClient.Close()

			// Create table via API with schema that uses INT64 (not INTEGER)
			// This reproduces the issue where BigQuery API returns INT64 type names
			schema := bigquery.Schema{
				{Name: "string_col", Type: bigquery.StringFieldType},
				{Name: "int_col", Type: bigquery.IntegerFieldType},
				{Name: "float_col", Type: bigquery.FloatFieldType},
				{Name: "bool_col", Type: bigquery.BooleanFieldType},
				{Name: "bytes_col", Type: bigquery.BytesFieldType},
				{Name: "date_col", Type: bigquery.DateFieldType},
				{Name: "datetime_col", Type: bigquery.DateTimeFieldType},
				{Name: "timestamp_col", Type: bigquery.TimestampFieldType},
				{Name: "time_col", Type: bigquery.TimeFieldType},
				{Name: "numeric_col", Type: bigquery.NumericFieldType},
				{Name: "bignumeric_col", Type: bigquery.BigNumericFieldType},
				{Name: "array_col", Type: bigquery.StringFieldType, Repeated: true},
				{
					Name: "struct_col",
					Type: bigquery.RecordFieldType,
					Schema: bigquery.Schema{
						{Name: "field1", Type: bigquery.IntegerFieldType},
						{Name: "field2", Type: bigquery.StringFieldType},
					},
				},
			}

			table := bqClient.Dataset(datasetID).Table(tableID)
			if err := table.Create(ctx, &bigquery.TableMetadata{Schema: schema}); err != nil {
				t.Fatalf("failed to create table: %v", err)
			}

			// Check what type names are actually stored in the table metadata
			metadata, err := table.Metadata(ctx)
			if err != nil {
				t.Fatalf("failed to get table metadata: %v", err)
			}
			t.Logf("Table schema after creation:")
			for _, field := range metadata.Schema {
				t.Logf("  Field: %s, Type: %s", field.Name, field.Type)
				if field.Type == bigquery.RecordFieldType && field.Schema != nil {
					for _, subfield := range field.Schema {
						t.Logf("    Subfield: %s, Type: %s", subfield.Name, subfield.Type)
					}
				}
			}

			// Insert data - use string values for date/datetime to avoid import issues
			type TestRow struct {
				StringCol     string                 `bigquery:"string_col"`
				IntCol        int64                  `bigquery:"int_col"`
				FloatCol      float64                `bigquery:"float_col"`
				BoolCol       bool                   `bigquery:"bool_col"`
				BytesCol      []byte                 `bigquery:"bytes_col"`
				DateCol       string                 `bigquery:"date_col"`
				DatetimeCol   string                 `bigquery:"datetime_col"`
				TimestampCol  time.Time              `bigquery:"timestamp_col"`
				TimeCol       civil.Time             `bigquery:"time_col"`
				NumericCol    string                 `bigquery:"numeric_col"`
				BignumericCol string                 `bigquery:"bignumeric_col"`
				ArrayCol      []string               `bigquery:"array_col"`
				StructCol     map[string]interface{} `bigquery:"struct_col"`
			}

			testData := []TestRow{
				{
					StringCol:     "hello",
					IntCol:        42,
					FloatCol:      3.14,
					BoolCol:       true,
					BytesCol:      []byte("abc"),
					DateCol:       "2024-01-01",
					DatetimeCol:   "2024-01-01T12:00:00",
					TimestampCol:  time.Date(2024, 1, 1, 12, 0, 0, 0, time.UTC),
					TimeCol:       civil.Time{Hour: 12},
					NumericCol:    "123.456",
					BignumericCol: "578960446186580977117854925043439539266.34992332820282019728792003956564819967",
					ArrayCol:      []string{"x", "y"},
					StructCol:     map[string]interface{}{"field1": int64(1), "field2": "nested"},
				},
			}

			inserter := table.Inserter()
			if err := inserter.Put(ctx, testData); err != nil {
				t.Fatalf("failed to insert rows: %v", err)
			}

			// Now try to read via Storage API with specified format
			opts, err := testServer.GRPCClientOptions(ctx)
			if err != nil {
				t.Fatal(err)
			}

			bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
			if err != nil {
				t.Fatal(err)
			}
			defer func() { _ = bqReadClient.Close() }()

			readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectID, datasetID, tableID)

			createReadSessionRequest := &storagepb.CreateReadSessionRequest{
				Parent: fmt.Sprintf("projects/%s", projectID),
				ReadSession: &storagepb.ReadSession{
					Table:      readTable,
					DataFormat: format.dataFormat,
				},
				MaxStreamCount: 1,
			}

			session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
			if err != nil {
				t.Fatalf("CreateReadSession: %v", err)
			}

			if len(session.GetStreams()) == 0 {
				t.Fatal("no streams in session")
			}

			// Try to read the data
			stream := session.GetStreams()[0]
			readRowsRequest := &storagepb.ReadRowsRequest{
				ReadStream: stream.Name,
			}

			rowStream, err := bqReadClient.ReadRows(ctx, readRowsRequest, rpcOpts)
			if err != nil {
				t.Fatalf("ReadRows: %v", err)
			}

			// Try to read first response
			resp, err := rowStream.Recv()
			if err != nil && err != io.EOF {
				t.Fatalf("Failed to read data in %s format: %v", format.name, err)
			}

			if resp != nil {
				// Validate we can decode the data properly based on format
				if format.dataFormat == storagepb.DataFormat_AVRO {
					// Validate the AVRO schema matches BigQuery's expected format
					avroSchemaStr := session.GetAvroSchema().GetSchema()
					t.Logf("AVRO Schema:\n%s", avroSchemaStr)

					// Parse the schema as JSON to validate structure
					var avroSchema map[string]interface{}
					if err := json.Unmarshal([]byte(avroSchemaStr), &avroSchema); err != nil {
						t.Fatalf("Failed to parse AVRO schema as JSON: %v", err)
					}

					// Validate root type
					if avroSchema["type"] != "record" {
						t.Fatalf("Expected root type to be 'record', got: %v", avroSchema["type"])
					}

					// Validate fields exist
					fields, ok := avroSchema["fields"].([]interface{})
					if !ok {
						t.Fatalf("Expected 'fields' to be an array")
					}

					// Expected field count (13 fields in our schema)
					expectedFieldCount := 13
					if len(fields) != expectedFieldCount {
						t.Fatalf("Expected %d fields, got %d", expectedFieldCount, len(fields))
					}

					avroRows := resp.GetAvroRows()
					if avroRows != nil {
						t.Logf("Successfully read %d rows in AVRO format from API-created table", resp.RowCount)

						// Decode and verify the AVRO data
						codec, err := goavro.NewCodec(avroSchemaStr)
						if err != nil {
							t.Fatalf("Failed to create AVRO codec: %v", err)
						}

						undecoded := avroRows.GetSerializedBinaryRows()
						if len(undecoded) > 0 {
							datum, _, err := codec.NativeFromBinary(undecoded)
							if err != nil {
								t.Fatalf("Failed to decode AVRO data: %v", err)
							}

							// Verify the decoded data
							datumMap, ok := datum.(map[string]interface{})
							if !ok {
								t.Fatalf("Expected datum to be a map, got %T", datum)
							}

							// Check numeric_col value
							if numericVal, ok := datumMap["numeric_col"]; ok {
								// Extract the actual value from the union type
								if unionMap, ok := numericVal.(map[string]interface{}); ok {
									// Get the value from the union (should be under "bytes.decimal" key)
									for key, val := range unionMap {
										t.Logf("numeric_col union key: %s, value type: %T, value: %v", key, val, val)
									}
								} else {
									t.Logf("numeric_col value type: %T, value: %v", numericVal, numericVal)
								}
							}

							t.Logf("Decoded AVRO datum: %+v", datumMap)
						}
					}
				} else if format.dataFormat == storagepb.DataFormat_ARROW {
					ipcschema := resp.GetArrowSchema().GetSerializedSchema()
					mem := memory.NewGoAllocator()
					buf := bytes.NewBuffer(ipcschema)
					r, err := ipc.NewReader(buf, ipc.WithAllocator(mem))
					if err != nil {
						t.Fatalf("NewReader: %v", err)
					}
					aschema := r.Schema()

					// Verify that NUMERIC and BIGNUMERIC fields use proper decimal types
					for i := 0; i < aschema.NumFields(); i++ {
						field := aschema.Field(i)
						t.Logf("Arrow field %d: name=%s, type=%T (%s)", i, field.Name, field.Type, field.Type)

						switch field.Name {
						case "numeric_col":
							decType, ok := field.Type.(*arrow.Decimal128Type)
							if !ok {
								t.Fatalf("numeric_col should be Decimal128Type, got %T", field.Type)
							}
							if decType.Precision != 38 || decType.Scale != 9 {
								t.Fatalf("numeric_col should have precision=38, scale=9, got precision=%d, scale=%d",
									decType.Precision, decType.Scale)
							}
							t.Logf("✓ numeric_col correctly uses Decimal128 with precision=%d, scale=%d",
								decType.Precision, decType.Scale)
						case "bignumeric_col":
							decType, ok := field.Type.(*arrow.Decimal256Type)
							if !ok {
								t.Fatalf("bignumeric_col should be Decimal256Type, got %T", field.Type)
							}
							if decType.Precision != 76 || decType.Scale != 38 {
								t.Fatalf("bignumeric_col should have precision=76, scale=38, got precision=%d, scale=%d",
									decType.Precision, decType.Scale)
							}
							t.Logf("✓ bignumeric_col correctly uses Decimal256 with precision=%d, scale=%d (BigQuery=76.76 digits)",
								decType.Precision, decType.Scale)
						}
					}

					arrowBatch := resp.GetArrowRecordBatch()
					if arrowBatch != nil {
						undecoded := resp.GetArrowRecordBatch().GetSerializedRecordBatch()

						buf := bytes.NewBuffer(ipcschema)
						buf.Write(undecoded)
						r, err = ipc.NewReader(buf, ipc.WithAllocator(mem), ipc.WithSchema(aschema))
						if err != nil {
							t.Fatalf("NewReader: %v", err)
						}
						for r.Next() {
							rec := r.RecordBatch()

							// Validate that we have rows
							if rec.NumRows() == 0 {
								t.Fatal("Expected at least one row in record batch")
							}

							// Validate and log each column value
							for colIdx := int64(0); colIdx < rec.NumCols(); colIdx++ {
								col := rec.Column(int(colIdx))
								fieldName := aschema.Field(int(colIdx)).Name

								// Ensure column is not nil
								if col == nil {
									t.Fatalf("Column %d (%s) is nil", colIdx, fieldName)
								}

								// Validate column length matches row count
								if col.Len() != int(rec.NumRows()) {
									t.Fatalf("Column %d (%s) length %d does not match row count %d",
										colIdx, fieldName, col.Len(), rec.NumRows())
								}

								// Log the value for the first row
								if rec.NumRows() > 0 {
									valueStr := col.ValueStr(0)
									t.Logf("Column %d (%s): %s", colIdx, fieldName, valueStr)

									// Ensure the value is not just empty or null for most fields
									// (except for nullable fields which we can't easily check here)
									if !col.IsNull(0) && valueStr == "" {
										t.Logf("Warning: Column %d (%s) has empty ValueStr but is not null", colIdx, fieldName)
									}
								}
							}
						}
						t.Logf("Successfully read %d rows in ARROW format from API-created table", resp.RowCount)
					}
				}
			}

			t.Logf("Successfully read data from API-created table in %s format", format.name)
		})
	}
}

func TestStorageReadAVROWithINT64Type(t *testing.T) {
	// This test reproduces the case where tables are created using aliased type names
	// and ensures that the corresponding FieldTypes are used in the AVRO serializer
	const (
		projectID = "test"
		datasetID = "test_dataset"
		tableID   = "test_table_int64"
	)

	ctx := context.Background()

	bqServer, err := server.New(server.TempStorage)
	if err != nil {
		t.Fatal(err)
	}

	// Create empty project and dataset first
	project := types.NewProject(projectID, types.NewDataset(datasetID))
	if err := bqServer.Load(server.StructSource(project)); err != nil {
		t.Fatal(err)
	}

	testServer := bqServer.TestServer()
	defer func() {
		testServer.Close()
		bqServer.Close()
	}()

	// Use raw BigQuery v2 API service to insert table with INT64 type names
	// This bypasses ALL normalization and goes directly through tablesInsertHandler
	bqService, err := bigqueryv2.NewService(
		ctx,
		option.WithEndpoint(testServer.URL),
		option.WithoutAuthentication(),
	)
	if err != nil {
		t.Fatal(err)
	}

	// Create table with raw INT64 type names that won't be normalized
	tableToInsert := &bigqueryv2.Table{
		TableReference: &bigqueryv2.TableReference{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
		Schema: &bigqueryv2.TableSchema{
			Fields: []*bigqueryv2.TableFieldSchema{
				{Name: "int_col", Type: "INT64", Mode: "NULLABLE"}, // Raw INT64
				{Name: "string_col", Type: "STRING", Mode: "NULLABLE"},
				{
					Name: "struct_col",
					Type: "RECORD",
					Mode: "NULLABLE",
					Fields: []*bigqueryv2.TableFieldSchema{
						{Name: "field1", Type: "INT64", Mode: "NULLABLE"}, // INT64 in nested field
						{Name: "field2", Type: "STRING", Mode: "NULLABLE"},
					},
				},
			},
		},
	}

	_, err = bqService.Tables.Insert(projectID, datasetID, tableToInsert).Context(ctx).Do()
	if err != nil {
		t.Fatalf("failed to insert table via v2 API: %v", err)
	}

	// Verify the type names are normalized to canonical forms
	retrievedTable, err := bqService.Tables.Get(projectID, datasetID, tableID).Context(ctx).Do()
	if err != nil {
		t.Fatalf("failed to get table: %v", err)
	}
	t.Logf("Stored type for int_col: %s (should be INTEGER, not INT64)", retrievedTable.Schema.Fields[0].Type)
	t.Logf("Stored type for struct_col.field1: %s (should be INTEGER, not INT64)", retrievedTable.Schema.Fields[2].Fields[0].Type)

	// Insert a row using high-level client
	bqClient, err := bigquery.NewClient(ctx, projectID, option.WithEndpoint(testServer.URL), option.WithoutAuthentication())
	if err != nil {
		t.Fatal(err)
	}
	defer bqClient.Close()

	type TestRow struct {
		IntCol    int64                  `bigquery:"int_col"`
		StringCol string                 `bigquery:"string_col"`
		StructCol map[string]interface{} `bigquery:"struct_col"`
	}
	inserter := bqClient.Dataset(datasetID).Table(tableID).Inserter()
	if err := inserter.Put(ctx, []TestRow{{
		IntCol:    42,
		StringCol: "hello",
		StructCol: map[string]interface{}{"field1": int64(1), "field2": "nested"},
	}}); err != nil {
		t.Fatalf("failed to insert: %v", err)
	}

	// Try to read via Storage API with AVRO format
	opts, err := testServer.GRPCClientOptions(ctx)
	if err != nil {
		t.Fatal(err)
	}

	bqReadClient, err := bqStorage.NewBigQueryReadClient(ctx, opts...)
	if err != nil {
		t.Fatal(err)
	}
	defer func() { _ = bqReadClient.Close() }()

	readTable := fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectID, datasetID, tableID)

	createReadSessionRequest := &storagepb.CreateReadSessionRequest{
		Parent: fmt.Sprintf("projects/%s", projectID),
		ReadSession: &storagepb.ReadSession{
			Table:      readTable,
			DataFormat: storagepb.DataFormat_AVRO,
		},
		MaxStreamCount: 1,
	}

	// After normalization, this should succeed (no "unsupported avro type INT64" error)
	session, err := bqReadClient.CreateReadSession(ctx, createReadSessionRequest, rpcOpts)
	if err != nil {
		t.Fatalf("CreateReadSession failed: %v", err)
	}

	if len(session.GetStreams()) == 0 {
		t.Fatal("no streams in session")
	}

	t.Logf("AVRO schema created successfully with normalized types")

	stream := session.GetStreams()[0]
	readRowsRequest := &storagepb.ReadRowsRequest{
		ReadStream: stream.Name,
	}

	rowStream, err := bqReadClient.ReadRows(ctx, readRowsRequest, rpcOpts)
	if err != nil {
		t.Fatalf("ReadRows: %v", err)
	}

	// Should be able to read the data successfully
	resp, err := rowStream.Recv()
	if err != nil {
		t.Fatalf("Failed to receive data: %v", err)
	}

	if resp.RowCount == 0 {
		t.Fatal("Expected to receive at least one row")
	}

	t.Logf("Successfully read %d rows via Storage API with AVRO format after type normalization", resp.RowCount)
}
