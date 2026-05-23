# Batch load data using the Storage Write API

This document describes how to use the
[BigQuery Storage Write API](https://docs.cloud.google.com/bigquery/docs/write-api) to batch load data
into BigQuery.

In batch-load scenarios, an application writes data and commits it as a single
atomic transaction. When using the Storage Write API to batch load
data, create one or more streams in *pending type*. Pending type supports
stream-level transactions. Records are buffered in a pending state until you
commit the stream.

For batch workloads, also consider using the Storage Write API
through the [Apache Spark SQL connector for BigQuery](https://github.com/GoogleCloudDataproc/spark-bigquery-connector#writing-data-to-bigquery)
using Managed Service for Apache Spark, rather than writing custom
Storage Write API code.

The Storage Write API is well-suited to a *data pipeline*
architecture. A main process creates a number of streams. For each stream, it
assigns a worker thread or a separate process to write a portion of the batch
data. Each worker creates a connection to its stream, writes data, and finalizes
its stream when it's done. After all of the workers signal successful completion
to the main process, the main process commits the data. If a worker fails, its
assigned portion of the data will not show up in the final results, and the
whole worker can be safely retried. In a more sophisticated pipeline, workers
checkpoint their progress by reporting the last offset written to the main
process. This approach can result in a robust pipeline that is resilient to
failures.

## Batch load data using pending type

To use pending type, the application does the following:

1. Call `CreateWriteStream` to create one or more streams in pending type.
2. For each stream, call `AppendRows` in a loop to write batches of records.
3. For each stream, call `FinalizeWriteStream`. After you call this method, you cannot write any more rows to the stream. If you call `AppendRows` after calling `FinalizeWriteStream`, it returns a [`StorageError`](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.StorageError) with `StorageErrorCode.STREAM_FINALIZED` in the `google.rpc.Status` error. For more information the `google.rpc.Status` error model, see [Errors](https://docs.cloud.google.com/apis/design/errors).
4. Call `BatchCommitWriteStreams` to commit the streams. After you call this method, the data becomes available for reading. If there is an error committing any of the streams, the error is returned in the `stream_errors` field of the [`BatchCommitWriteStreamsResponse`](https://docs.cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#batchcommitwritestreamsresponse).

Committing is an atomic operation, and you can commit multiple streams at once.
A stream can only be committed once, so if the commit operation fails, it is
safe to retry it. Until you commit a stream, the data is pending and not visible
to reads.

After the stream is finalized and before it is committed, the data can remain in
the buffer for up to 4 hours. Pending streams must be committed within 24 hours.
There is a quota limit on the
[total size of the pending stream buffer](https://docs.cloud.google.com/bigquery/quotas#writeapi_pending_stream).

The following code shows how to write data in pending type:

### C#


To learn how to install and use the client library for BigQuery, see
[BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries).


For more information, see the
[BigQuery C# API
reference documentation](https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Api.Gax/latest/Google.Api.Gax.Grpc.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.html;
    using https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.html;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Threading.Tasks;
    using static Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.Types;

    public class AppendRowsPendingSample
    {
        /// <summary>
        /// This code sample demonstrates how to write records in pending mode.
        /// Create a write stream, write some sample data, and commit the stream to append the rows.
        /// The CustomerRecord proto used in the sample can be seen in Resources folder and generated C# is placed in Data folder in
        /// https://github.com/GoogleCloudPlatform/dotnet-docs-samples/tree/main/bigquery-storage/api/BigQueryStorage.Samples
        /// </summary>
        public async Task AppendRowsPendingAsync(string projectId, string datasetId, string tableId)
        {
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html bigQueryWriteClient = await https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html#Google_Cloud_BigQuery_Storage_V1_BigQueryWriteClient_CreateAsync_System_Threading_CancellationToken_();
            // Initialize a write stream for the specified table.
            // When creating the stream, choose the type. Use the Pending type to wait
            // until the stream is committed before it is visible. See:
            // https://cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.WriteStream.Type
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html stream = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html { Type = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.Types.Type.htmls.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.Types.Type.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.Types.Type.html#Google_Cloud_BigQuery_Storage_V1_WriteStream_Types_Type_Pending };
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.TableName.html tableName = https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.TableName.html.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.TableName.html#Google_Cloud_BigQuery_Storage_V1_TableName_FromProjectDatasetTable_System_String_System_String_System_String_(projectId, datasetId, tableId);

            stream = await bigQueryWriteClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html#Google_Cloud_BigQuery_Storage_V1_BigQueryWriteClient_CreateWriteStreamAsync_Google_Cloud_BigQuery_Storage_V1_CreateWriteStreamRequest_Google_Api_Gax_Grpc_CallSettings_(tableName, stream);

            // Initialize streaming call, retrieving the stream object
            BigQueryWriteClient.AppendRowsStream rowAppender = bigQueryWriteClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html#Google_Cloud_BigQuery_Storage_V1_BigQueryWriteClient_AppendRows_Google_Api_Gax_Grpc_CallSettings_Google_Api_Gax_Grpc_BidirectionalStreamingSettings_();

            // Sending requests and retrieving responses can be arbitrarily interleaved.
            // Exact sequence will depend on client/server behavior.
            // Create task to do something with responses from server.
            Task appendResultsHandlerTask = Task.Run(async () =>
            {
                AsyncResponseStream<AppendRowsResponse> appendRowResults = rowAppender.GetResponseStream();
                while (await appendRowResults.MoveNextAsync())
                {
                    AppendRowsResponse responseItem = appendRowResults.Current;
                    // Do something with responses.
                    if (responseItem.AppendResult != null)
                    {
                        Console.WriteLine($"Appending rows resulted in: {responseItem.AppendResult}");
                    }
                    if (responseItem.Error != null)
                    {
                        Console.Error.WriteLine($"Appending rows resulted in an error: {responseItem.Error.Message}");
                        foreach (RowError rowError in responseItem.RowErrors)
                        {
                            Console.Error.WriteLine($"Row Error: {rowError}");
                        }
                    }
                }
                // The response stream has completed.
            });

            // List of records to be appended in the table.
            List<CustomerRecord> records = new List<CustomerRecord>
            {
                new CustomerRecord { CustomerNumber = 1, CustomerName = "Alice" },
                new CustomerRecord { CustomerNumber = 2, CustomerName = "Bob" }
            };

            // Create a batch of row data by appending serialized bytes to the
            // SerializedRows repeated field.
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.Types.ProtoData.html protoData = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.Types.ProtoData.html
            {
                WriterSchema = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.ProtoSchema.html { ProtoDescriptor = CustomerRecord.Descriptor.ToProto() },
                Rows = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.ProtoRows.html { SerializedRows = { records.Select(r => r.https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.MessageExtensions.html#Google_Protobuf_MessageExtensions_ToByteString_Google_Protobuf_IMessage_()) } }
            };

            // Initialize the append row request.
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.html appendRowRequest = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.html
            {
                WriteStreamAsWriteStreamName = stream.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html#Google_Cloud_BigQuery_Storage_V1_WriteStream_WriteStreamName,
                ProtoRows = protoData
            };

            // Stream a request to the server.
            await rowAppender.WriteAsync(appendRowRequest);

            // Append a second batch of data.
            protoData = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.Types.ProtoData.html
            {
                Rows = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.ProtoRows.html { SerializedRows = { new CustomerRecord { CustomerNumber = 3, CustomerName = "Charles" }.https://docs.cloud.google.com/dotnet/docs/reference/Google.Protobuf/latest/Google.Protobuf.MessageExtensions.html#Google_Protobuf_MessageExtensions_ToByteString_Google_Protobuf_IMessage_() } }
            };

            // Since this is the second request, you only need to include the row data.
            // The name of the stream and protocol buffers descriptor is only needed in
            // the first request.
            appendRowRequest = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.AppendRowsRequest.html
            {
                // If Offset is not present, the write is performed at the current end of stream.
                ProtoRows = protoData
            };

            await rowAppender.WriteAsync(appendRowRequest);

            // Complete writing requests to the stream.
            await rowAppender.WriteCompleteAsync();

            // Await the handler. This will complete once all server responses have been processed.
            await appendResultsHandlerTask;

            // A Pending type stream must be "finalized" before being committed. No new
            // records can be written to the stream after this method has been called.
            await bigQueryWriteClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html#Google_Cloud_BigQuery_Storage_V1_BigQueryWriteClient_FinalizeWriteStreamAsync_Google_Cloud_BigQuery_Storage_V1_FinalizeWriteStreamRequest_Google_Api_Gax_Grpc_CallSettings_(stream.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html#Google_Cloud_BigQuery_Storage_V1_WriteStream_Name);
            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BatchCommitWriteStreamsRequest.html batchCommitWriteStreamsRequest = new https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BatchCommitWriteStreamsRequest.html
            {
                Parent = tableName.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.TableName.html#Google_Cloud_BigQuery_Storage_V1_TableName_ToString(),
                WriteStreams = { stream.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html#Google_Cloud_BigQuery_Storage_V1_WriteStream_Name }
            };

            https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BatchCommitWriteStreamsResponse.html batchCommitWriteStreamsResponse =
                await bigQueryWriteClient.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BigQueryWriteClient.html#Google_Cloud_BigQuery_Storage_V1_BigQueryWriteClient_BatchCommitWriteStreamsAsync_Google_Cloud_BigQuery_Storage_V1_BatchCommitWriteStreamsRequest_Google_Api_Gax_Grpc_CallSettings_(batchCommitWriteStreamsRequest);
            if (batchCommitWriteStreamsResponse.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BatchCommitWriteStreamsResponse.html#Google_Cloud_BigQuery_Storage_V1_BatchCommitWriteStreamsResponse_StreamErrors?.Count > 0)
            {
                // Handle errors here.
                Console.WriteLine("Error committing write streams. Individual errors:");
                foreach (https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.StorageError.html error in batchCommitWriteStreamsResponse.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.BatchCommitWriteStreamsResponse.html#Google_Cloud_BigQuery_Storage_V1_BatchCommitWriteStreamsResponse_StreamErrors)
                {
                    Console.WriteLine(error.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.StorageError.html#Google_Cloud_BigQuery_Storage_V1_StorageError_ErrorMessage);
                }            
            }
            else
            {
                Console.WriteLine($"Writes to stream {stream.https://docs.cloud.google.com/dotnet/docs/reference/Google.Cloud.BigQuery.Storage.V1/latest/Google.Cloud.BigQuery.Storage.V1.WriteStream.html#Google_Cloud_BigQuery_Storage_V1_WriteStream_Name} have been committed.");
            }
        }
    }

<br />

### Go


To learn how to install and use the client library for BigQuery, see
[BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries).


For more information, see the
[BigQuery Go API
reference documentation](https://pkg.go.dev/cloud.google.com/go/bigquery/storage?tab=doc).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).


    import (
    	"context"
    	"fmt"
    	"io"
    	"math/rand"
    	"time"

    	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
    	"cloud.google.com/go/bigquery/storage/managedwriter"
    	"cloud.google.com/go/bigquery/storage/managedwriter/adapt"
    	"github.com/GoogleCloudPlatform/golang-samples/bigquery/snippets/managedwriter/exampleproto"
    	"google.golang.org/protobuf/proto"
    )

    // generateExampleMessages generates a slice of serialized protobuf messages using a statically defined
    // and compiled protocol buffer file, and returns the binary serialized representation.
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
    			BignumericCol: proto.String("578960446186580977117854925043439539266.34992332820282019728792003956564819967"),

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

    // appendToPendingStream demonstrates using the managedwriter package to write some example data
    // to a pending stream, and then committing it to a table.
    func appendToPendingStream(w io.Writer, projectID, datasetID, tableID string) error {
    	// projectID := "myproject"
    	// datasetID := "mydataset"
    	// tableID := "mytable"

    	ctx := context.Background()
    	// Instantiate a managedwriter client to handle interactions with the service.
    	client, err := managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_Client_NewClient(ctx, projectID)
    	if err != nil {
    		return fmt.Errorf("managedwriter.NewClient: %w", err)
    	}
    	// Close the client when we exit the function.
    	defer client.Close()

    	// Create a new pending stream.  We'll use the stream name to construct a writer.
    	pendingStream, err := client.CreateWriteStream(ctx, &storagepb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1/storagepb.html#cloud_google_com_go_bigquery_storage_apiv1_storagepb_CreateWriteStreamRequest{
    		Parent: fmt.Sprintf("projects/%s/datasets/%s/tables/%s", projectID, datasetID, tableID),
    		WriteStream: &storagepb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1/storagepb.html#cloud_google_com_go_bigquery_storage_apiv1_storagepb_WriteStream{
    			Type: storagepb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1/storagepb.html#cloud_google_com_go_bigquery_storage_apiv1_storagepb_WriteStream_TYPE_UNSPECIFIED_WriteStream_COMMITTED_WriteStream_PENDING_WriteStream_BUFFERED,
    		},
    	})
    	if err != nil {
    		return fmt.Errorf("CreateWriteStream: %w", err)
    	}

    	// We need to communicate the descriptor of the protocol buffer message we're using, which
    	// is analagous to the "schema" for the message.  Both SampleData and SampleStruct are
    	// two distinct messages in the compiled proto file, so we'll use adapt.NormalizeDescriptor
    	// to unify them into a single self-contained descriptor representation.
    	m := &exampleproto.SampleData{}
    	descriptorProto, err := adapt.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter/adapt.html#cloud_google_com_go_bigquery_storage_managedwriter_adapt_NormalizeDescriptor(m.ProtoReflect().Descriptor())
    	if err != nil {
    		return fmt.Errorf("NormalizeDescriptor: %w", err)
    	}

    	// Instantiate a ManagedStream, which manages low level details like connection state and provides
    	// additional features like a future-like callback for appends, etc.  NewManagedStream can also create
    	// the stream on your behalf, but in this example we're being explicit about stream creation.
    	managedStream, err := client.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_Client_NewManagedStream(ctx, managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_WriterOption_WithStreamName(pendingStream.GetName()),
    		managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_WriterOption_WithSchemaDescriptor(descriptorProto))
    	if err != nil {
    		return fmt.Errorf("NewManagedStream: %w", err)
    	}
    	defer managedStream.Close()

    	// First, we'll append a single row.
    	rows, err := generateExampleMessages(1)
    	if err != nil {
    		return fmt.Errorf("generateExampleMessages: %w", err)
    	}

    	// We'll keep track of the current offset in the stream with curOffset.
    	var curOffset int64
    	// We can append data asyncronously, so we'll check our appends at the end.
    	var results []*managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_AppendResult

    	result, err := managedStream.AppendRows(ctx, rows, managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_AppendOption_WithOffset(0))
    	if err != nil {
    		return fmt.Errorf("AppendRows first call error: %w", err)
    	}
    	results = append(results, result)

    	// Advance our current offset.
    	curOffset = curOffset + 1

    	// This time, we'll append three more rows in a single request.
    	rows, err = generateExampleMessages(3)
    	if err != nil {
    		return fmt.Errorf("generateExampleMessages: %w", err)
    	}
    	result, err = managedStream.AppendRows(ctx, rows, managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_AppendOption_WithOffset(curOffset))
    	if err != nil {
    		return fmt.Errorf("AppendRows second call error: %w", err)
    	}
    	results = append(results, result)

    	// Advance our offset again.
    	curOffset = curOffset + 3

    	// Finally, we'll append two more rows.
    	rows, err = generateExampleMessages(2)
    	if err != nil {
    		return fmt.Errorf("generateExampleMessages: %w", err)
    	}
    	result, err = managedStream.AppendRows(ctx, rows, managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_AppendOption_WithOffset(curOffset))
    	if err != nil {
    		return fmt.Errorf("AppendRows third call error: %w", err)
    	}
    	results = append(results, result)

    	// Now, we'll check that our batch of three appends all completed successfully.
    	// Monitoring the results could also be done out of band via a goroutine.
    	for k, v := range results {
    		// GetResult blocks until we receive a response from the API.
    		recvOffset, err := v.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_AppendResult_GetResult(ctx)
    		if err != nil {
    			return fmt.Errorf("append %d returned error: %w", k, err)
    		}
    		fmt.Fprintf(w, "Successfully appended data at offset %d.\n", recvOffset)
    	}

    	// We're now done appending to this stream.  We now mark pending stream finalized, which blocks
    	// further appends.
    	rowCount, err := managedStream.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_ManagedStream_Finalize(ctx)
    	if err != nil {
    		return fmt.Errorf("error during Finalize: %w", err)
    	}

    	fmt.Fprintf(w, "Stream %s finalized with %d rows.\n", managedStream.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_ManagedStream_StreamName(), rowCount)

    	// To commit the data to the table, we need to run a batch commit.  You can commit several streams
    	// atomically as a group, but in this instance we'll only commit the single stream.
    	req := &storagepb.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1/storagepb.html#cloud_google_com_go_bigquery_storage_apiv1_storagepb_BatchCommitWriteStreamsRequest{
    		Parent:       managedwriter.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_TableParentFromStreamName(managedStream.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_ManagedStream_StreamName()),
    		WriteStreams: []string{managedStream.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/managedwriter.html#cloud_google_com_go_bigquery_storage_managedwriter_ManagedStream_StreamName()},
    	}

    	resp, err := client.BatchCommitWriteStreams(ctx, req)
    	if err != nil {
    		return fmt.Errorf("client.BatchCommit: %w", err)
    	}
    	if len(resp.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1/storagepb.html#cloud_google_com_go_bigquery_storage_apiv1_storagepb_BatchCommitWriteStreamsResponse_GetStreamErrors()) > 0 {
    		return fmt.Errorf("stream errors present: %v", resp.https://docs.cloud.google.com/go/docs/reference/cloud.google.com/go/bigquery/latest/storage/apiv1/storagepb.html#cloud_google_com_go_bigquery_storage_apiv1_storagepb_BatchCommitWriteStreamsResponse_GetStreamErrors())
    	}

    	fmt.Fprintf(w, "Table data committed at %s\n", resp.GetCommitTime().AsTime().Format(time.RFC3339Nano))

    	return nil
    }

<br />

### Java


To learn how to install and use the client library for BigQuery, see
[BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries).


For more information, see the
[BigQuery Java API
reference documentation](https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    import com.google.api.core.https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.core.ApiFuture.html;
    import com.google.api.core.https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.core.ApiFutureCallback.html;
    import com.google.api.core.https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.core.ApiFutures.html;
    import com.google.api.gax.retrying.https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.AppendRowsResponse.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsRequest.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsResponse.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.CreateWriteStreamRequest.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.Exceptions.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.Exceptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.Exceptions.StorageException.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.FinalizeWriteStreamResponse.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.StorageError.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.TableName.html;
    import com.google.cloud.bigquery.storage.v1.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html;
    import com.google.common.util.concurrent.MoreExecutors;
    import com.google.protobuf.Descriptors.DescriptorValidationException;
    import java.io.IOException;
    import java.util.concurrent.ExecutionException;
    import java.util.concurrent.Phaser;
    import javax.annotation.concurrent.GuardedBy;
    import org.json.JSONArray;
    import org.json.JSONObject;
    import org.threeten.bp.Duration;

    public class WritePendingStream {

      public static void runWritePendingStream()
          throws DescriptorValidationException, InterruptedException, IOException {
        // TODO(developer): Replace these variables before running the sample.
        String projectId = "MY_PROJECT_ID";
        String datasetName = "MY_DATASET_NAME";
        String tableName = "MY_TABLE_NAME";

        writePendingStream(projectId, datasetName, tableName);
      }

      public static void writePendingStream(String projectId, String datasetName, String tableName)
          throws DescriptorValidationException, InterruptedException, IOException {
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html client = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html.create();
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.TableName.html parentTable = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.TableName.html.of(projectId, datasetName, tableName);

        DataWriter writer = new DataWriter();
        // One time initialization.
        writer.initialize(parentTable, client);

        try {
          // Write two batches of fake data to the stream, each with 10 JSON records.  Data may be
          // batched up to the maximum request size:
          // https://cloud.google.com/bigquery/quotas#write-api-limits
          long offset = 0;
          for (int i = 0; i < 2; i++) {
            // Create a JSON object that is compatible with the table schema.
            JSONArray jsonArr = new JSONArray();
            for (int j = 0; j < 10; j++) {
              JSONObject record = new JSONObject();
              record.put("col1", String.format("batch-record %03d-%03d", i, j));
              jsonArr.put(record);
            }
            writer.append(jsonArr, offset);
            offset += jsonArr.length();
          }
        } catch (ExecutionException e) {
          // If the wrapped exception is a StatusRuntimeException, check the state of the operation.
          // If the state is INTERNAL, CANCELLED, or ABORTED, you can retry. For more information, see:
          // https://grpc.github.io/grpc-java/javadoc/io/grpc/StatusRuntimeException.html
          System.out.println("Failed to append records. \n" + e);
        }

        // Final cleanup for the stream.
        writer.cleanup(client);
        System.out.println("Appended records successfully.");

        // Once all streams are done, if all writes were successful, commit all of them in one request.
        // This example only has the one stream. If any streams failed, their workload may be
        // retried on a new stream, and then only the successful stream should be included in the
        // commit.
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsRequest.html commitRequest =
            https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsRequest.html.newBuilder()
                .setParent(parentTable.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.TableName.html#com_google_cloud_bigquery_storage_v1_TableName_toString__())
                .https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsRequest.Builder.html#com_google_cloud_bigquery_storage_v1_BatchCommitWriteStreamsRequest_Builder_addWriteStreams_java_lang_String_(writer.getStreamName())
                .build();
        https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsResponse.html commitResponse = client.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html#com_google_cloud_bigquery_storage_v1_BigQueryWriteClient_batchCommitWriteStreams_com_google_cloud_bigquery_storage_v1_BatchCommitWriteStreamsRequest_(commitRequest);
        // If the response does not have a commit time, it means the commit operation failed.
        if (commitResponse.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsResponse.html#com_google_cloud_bigquery_storage_v1_BatchCommitWriteStreamsResponse_hasCommitTime__() == false) {
          for (https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.StorageError.html err : commitResponse.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BatchCommitWriteStreamsResponse.html#com_google_cloud_bigquery_storage_v1_BatchCommitWriteStreamsResponse_getStreamErrorsList__()) {
            System.out.println(err.getErrorMessage());
          }
          throw new RuntimeException("Error committing the streams");
        }
        System.out.println("Appended and committed records successfully.");
      }

      // A simple wrapper object showing how the stateful stream writer should be used.
      private static class DataWriter {

        private https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.html streamWriter;
        // Track the number of in-flight requests to wait for all responses before shutting down.
        private final Phaser inflightRequestCount = new Phaser(1);

        private final Object lock = new Object();

        @GuardedBy("lock")
        private RuntimeException error = null;

        void initialize(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.TableName.html parentTable, https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html client)
            throws IOException, DescriptorValidationException, InterruptedException {
          // Initialize a write stream for the specified table.
          // For more information on WriteStream.Type, see:
          // https://googleapis.dev/java/google-cloud-bigquerystorage/latest/com/google/cloud/bigquery/storage/v1/WriteStream.Type.html
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html stream = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html.newBuilder().setType(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html.Type.PENDING).build();

          // Configure in-stream automatic retry settings.
          // Error codes that are immediately retried:
          // * ABORTED, UNAVAILABLE, CANCELLED, INTERNAL, DEADLINE_EXCEEDED
          // Error codes that are retried with exponential backoff:
          // * RESOURCE_EXHAUSTED
          https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.html retrySettings =
              https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.html.newBuilder()
                  .https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.Builder.html#com_google_api_gax_retrying_RetrySettings_Builder_setInitialRetryDelay_org_threeten_bp_Duration_(Duration.ofMillis(500))
                  .https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.Builder.html#com_google_api_gax_retrying_RetrySettings_Builder_setRetryDelayMultiplier_double_(1.1)
                  .https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.Builder.html#com_google_api_gax_retrying_RetrySettings_Builder_setMaxAttempts_int_(5)
                  .https://docs.cloud.google.com/java/docs/reference/gax/latest/com.google.api.gax.retrying.RetrySettings.Builder.html#com_google_api_gax_retrying_RetrySettings_Builder_setMaxRetryDelay_org_threeten_bp_Duration_(Duration.ofMinutes(1))
                  .build();

          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.CreateWriteStreamRequest.html createWriteStreamRequest =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.CreateWriteStreamRequest.html.newBuilder()
                  .setParent(parentTable.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.TableName.html#com_google_cloud_bigquery_storage_v1_TableName_toString__())
                  .setWriteStream(stream)
                  .build();
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html writeStream = client.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html#com_google_cloud_bigquery_storage_v1_BigQueryWriteClient_createWriteStream_com_google_cloud_bigquery_storage_v1_CreateWriteStreamRequest_(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html#com_google_cloud_bigquery_storage_v1_BigQueryWriteClient_createWriteStream_com_google_cloud_bigquery_storage_v1_CreateWriteStreamRequest_Request);

          // Use the JSON stream writer to send records in JSON format.
          // For more information about JsonStreamWriter, see:
          // https://cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter
          streamWriter =
              https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.html.newBuilder(writeStream.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html#com_google_cloud_bigquery_storage_v1_WriteStream_getName__(), writeStream.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.WriteStream.html#com_google_cloud_bigquery_storage_v1_WriteStream_getTableSchema__())
                  .setRetrySettings(retrySettings)
                  .build();
        }

        public void append(JSONArray data, long offset)
            throws DescriptorValidationException, IOException, ExecutionException {
          synchronized (this.lock) {
            // If earlier appends have failed, we need to reset before continuing.
            if (this.error != null) {
              throw this.error;
            }
          }
          // Append asynchronously for increased throughput.
          ApiFuture<AppendRowsResponse> future = streamWriter.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.html#com_google_cloud_bigquery_storage_v1_JsonStreamWriter_append_com_google_gson_JsonArray_(data, offset);
          https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.core.ApiFutures.html.https://docs.cloud.google.com/java/docs/reference/api-common/latest/com.google.api.core.ApiFutures.html#com_google_api_core_ApiFutures__V_addCallback_com_google_api_core_ApiFuture_V__com_google_api_core_ApiFutureCallback___super_V__(
              future, new AppendCompleteCallback(this), MoreExecutors.directExecutor());
          // Increase the count of in-flight requests.
          inflightRequestCount.register();
        }

        public void cleanup(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html client) {
          // Wait for all in-flight requests to complete.
          inflightRequestCount.arriveAndAwaitAdvance();

          // Close the connection to the server.
          streamWriter.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.html#com_google_cloud_bigquery_storage_v1_JsonStreamWriter_close__();

          // Verify that no error occurred in the stream.
          synchronized (this.lock) {
            if (this.error != null) {
              throw this.error;
            }
          }

          // Finalize the stream.
          https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.FinalizeWriteStreamResponse.html finalizeResponse =
              client.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.BigQueryWriteClient.html#com_google_cloud_bigquery_storage_v1_BigQueryWriteClient_finalizeWriteStream_com_google_cloud_bigquery_storage_v1_FinalizeWriteStreamRequest_(streamWriter.getStreamName());
          System.out.println("Rows written: " + finalizeResponse.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.FinalizeWriteStreamResponse.html#com_google_cloud_bigquery_storage_v1_FinalizeWriteStreamResponse_getRowCount__());
        }

        public String getStreamName() {
          return streamWriter.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.JsonStreamWriter.html#com_google_cloud_bigquery_storage_v1_JsonStreamWriter_getStreamName__();
        }

        static class AppendCompleteCallback implements ApiFutureCallback<AppendRowsResponse> {

          private final DataWriter parent;

          public AppendCompleteCallback(DataWriter parent) {
            this.parent = parent;
          }

          public void onSuccess(https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.AppendRowsResponse.html response) {
            System.out.format("Append %d success\n", response.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.AppendRowsResponse.html#com_google_cloud_bigquery_storage_v1_AppendRowsResponse_getAppendResult__().getOffset().getValue());
            done();
          }

          public void onFailure(Throwable throwable) {
            synchronized (this.parent.lock) {
              if (this.parent.error == null) {
                https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.Exceptions.StorageException.html storageException = https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.Exceptions.html.https://docs.cloud.google.com/java/docs/reference/google-cloud-bigquerystorage/latest/com.google.cloud.bigquery.storage.v1.Exceptions.html#com_google_cloud_bigquery_storage_v1_Exceptions_toStorageException_com_google_rpc_Status_java_lang_Throwable_(throwable);
                this.parent.error =
                    (storageException != null) ? storageException : new RuntimeException(throwable);
              }
            }
            System.out.format("Error: %s\n", throwable.toString());
            done();
          }

          private void done() {
            // Reduce the count of in-flight requests.
            this.parent.inflightRequestCount.arriveAndDeregister();
          }
        }
      }
    }

<br />

### Node.js


To learn how to install and use the client library for BigQuery, see
[BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries).


For more information, see the
[BigQuery Node.js API
reference documentation](https://googleapis.dev/nodejs/bigquery/latest/index.html).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    const {adapt, managedwriter} = require('https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/overview.html');
    const {WriterClient, Writer} = managedwriter;

    const customer_record_pb = require('./customer_record_pb.js');
    const {CustomerRecord} = customer_record_pb;

    const protobufjs = require('protobufjs');
    require('protobufjs/ext/descriptor');

    async function appendRowsPending() {
      /**
       * If you make updates to the customer_record.proto protocol buffers definition,
       * run:
       *   pbjs customer_record.proto -t static-module -w commonjs -o customer_record.js
       *   pbjs customer_record.proto -t json --keep-case -o customer_record.json
       * from the /samples directory to generate the customer_record module.
       */

      // So that BigQuery knows how to parse the serialized_rows, create a
      // protocol buffer representation of your message descriptor.
      const root = protobufjs.loadSync('./customer_record.json');
      const descriptor = root.lookupType('CustomerRecord').toDescriptor('proto2');
      const protoDescriptor = adapt.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/overview.html(descriptor).toJSON();

      /**
       * TODO(developer): Uncomment the following lines before running the sample.
       */
      // projectId = 'my_project';
      // datasetId = 'my_dataset';
      // tableId = 'my_table';

      const destinationTable = `projects/${projectId}/datasets/${datasetId}/tables/${tableId}`;
      const streamType = managedwriter.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/overview.html;
      const writeClient = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/overview.html({projectId});
      try {
        const writeStream = await writeClient.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/bigquery-storage/managedwriter.writerclient.html({
          streamType,
          destinationTable,
        });
        const streamId = writeStream.name;
        console.log(`Stream created: ${streamId}`);

        const connection = await writeClient.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/bigquery-storage/managedwriter.writerclient.html({
          streamId,
        });
        const writer = new https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/bigquery-storage/managedwriter.writer.html({
          connection,
          protoDescriptor,
        });

        let serializedRows = [];
        const pendingWrites = [];

        // Row 1
        let row = {
          rowNum: 1,
          customerName: 'Octavia',
        };
        serializedRows.push(CustomerRecord.encode(row).finish());

        // Row 2
        row = {
          rowNum: 2,
          customerName: 'Turing',
        };
        serializedRows.push(CustomerRecord.encode(row).finish());

        // Set an offset to allow resuming this stream if the connection breaks.
        // Keep track of which requests the server has acknowledged and resume the
        // stream at the first non-acknowledged message. If the server has already
        // processed a message with that offset, it will return an ALREADY_EXISTS
        // error, which can be safely ignored.

        // The first request must always have an offset of 0.
        let offsetValue = 0;

        // Send batch.
        let pw = writer.appendRows({serializedRows}, offsetValue);
        pendingWrites.push(pw);

        serializedRows = [];

        // Row 3
        row = {
          rowNum: 3,
          customerName: 'Bell',
        };
        serializedRows.push(CustomerRecord.encode(row).finish());

        // Offset must equal the number of rows that were previously sent.
        offsetValue = 2;

        // Send batch.
        pw = writer.appendRows({serializedRows}, offsetValue);
        pendingWrites.push(pw);

        const results = await Promise.all(
          pendingWrites.map(pw => pw.getResult()),
        );
        console.log('Write results:', results);

        const {rowCount} = await connection.finalize();
        console.log(`Row count: ${rowCount}`);

        const response = await writeClient.https://docs.cloud.google.com/nodejs/docs/reference/bigquery-storage/latest/bigquery-storage/managedwriter.writerclient.html({
          parent: destinationTable,
          writeStreams: [streamId],
        });

        console.log(response);
      } catch (err) {
        console.log(err);
      } finally {
        writeClient.close();
      }
    }

<br />

### Python

This example shows a simple record with two fields. For a longer example that
shows how to send different data types, including `STRUCT` types, see the
[append_rows_proto2 sample](https://github.com/googleapis/google-cloud-python/blob/main/packages/google-cloud-bigquery-storage/samples/snippets/append_rows_proto2.py) on GitHub.

<br />


To learn how to install and use the client library for BigQuery, see
[BigQuery client libraries](https://docs.cloud.google.com/bigquery/docs/reference/storage/libraries).


For more information, see the
[BigQuery Python API
reference documentation](https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest).


To authenticate to BigQuery, set up Application Default Credentials.
For more information, see

[Set up authentication for client libraries](https://docs.cloud.google.com/bigquery/docs/authentication#client-libs).

    """
    This code sample demonstrates how to write records in pending mode
    using the low-level generated client for Python.
    """

    from google.protobuf import descriptor_pb2

    from google.cloud import https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest
    from google.cloud.bigquery_storage_v1 import https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html, https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.html

    # If you update the customer_record.proto protocol buffer definition, run:
    #
    #   protoc --python_out=. customer_record.proto
    #
    # from the samples/snippets directory to generate the customer_record_pb2.py module.
    from . import customer_record_pb2


    def create_row_data(row_num: int, name: str):
        row = customer_record_pb2.CustomerRecord()
        row.row_num = row_num
        row.customer_name = name
        return row.SerializeToString()


    def append_rows_pending(project_id: str, dataset_id: str, table_id: str):
        """Create a write stream, write some sample data, and commit the stream."""
        write_client = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest.BigQueryWriteClient()
        parent = write_client.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.services.big_query_write.BigQueryWriteClient.html#google_cloud_bigquery_storage_v1_services_big_query_write_BigQueryWriteClient_table_path(project_id, dataset_id, table_id)
        write_stream = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.WriteStream.html()

        # When creating the stream, choose the type. Use the PENDING type to wait
        # until the stream is committed before it is visible. See:
        # https://cloud.google.com/bigquery/docs/reference/storage/rpc/google.cloud.bigquery.storage.v1#google.cloud.bigquery.storage.v1.WriteStream.Type
        write_stream.type_ = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.WriteStream.html.Type.PENDING
        write_stream = write_client.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.services.big_query_write.BigQueryWriteClient.html#google_cloud_bigquery_storage_v1_services_big_query_write_BigQueryWriteClient_create_write_stream(
            parent=parent, write_stream=write_stream
        )
        stream_name = write_stream.name

        # Create a template with fields needed for the first request.
        request_template = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.html()

        # The initial request must contain the stream name.
        request_template.write_stream = stream_name

        # So that BigQuery knows how to parse the serialized_rows, generate a
        # protocol buffer representation of your message descriptor.
        proto_schema = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.ProtoSchema.html()
        proto_descriptor = descriptor_pb2.DescriptorProto()
        customer_record_pb2.CustomerRecord.DESCRIPTOR.CopyToProto(proto_descriptor)
        proto_schema.proto_descriptor = proto_descriptor
        proto_data = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.ProtoData.html()
        proto_data.writer_schema = proto_schema
        request_template.proto_rows = proto_data

        # Some stream types support an unbounded number of requests. Construct an
        # AppendRowsStream to send an arbitrary number of requests to a stream.
        append_rows_stream = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.AppendRowsStream.html(write_client, request_template)

        # Create a batch of row data by appending proto2 serialized bytes to the
        # serialized_rows repeated field.
        proto_rows = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.ProtoRows.html()
        proto_rows.serialized_rows.append(create_row_data(1, "Alice"))
        proto_rows.serialized_rows.append(create_row_data(2, "Bob"))

        # Set an offset to allow resuming this stream if the connection breaks.
        # Keep track of which requests the server has acknowledged and resume the
        # stream at the first non-acknowledged message. If the server has already
        # processed a message with that offset, it will return an ALREADY_EXISTS
        # error, which can be safely ignored.
        #
        # The first request must always have an offset of 0.
        request = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.html()
        request.offset = 0
        proto_data = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.ProtoData.html()
        proto_data.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.reader.ReadRowsStream.html#google_cloud_bigquery_storage_v1_reader_ReadRowsStream_rows = proto_rows
        request.proto_rows = proto_data

        response_future_1 = append_rows_stream.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.AppendRowsStream.html#google_cloud_bigquery_storage_v1beta2_writer_AppendRowsStream_send(request)

        # Send another batch.
        proto_rows = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.ProtoRows.html()
        proto_rows.serialized_rows.append(create_row_data(3, "Charles"))

        # Since this is the second request, you only need to include the row data.
        # The name of the stream and protocol buffers DESCRIPTOR is only needed in
        # the first request.
        request = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.html()
        proto_data = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.AppendRowsRequest.ProtoData.html()
        proto_data.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.reader.ReadRowsStream.html#google_cloud_bigquery_storage_v1_reader_ReadRowsStream_rows = proto_rows
        request.proto_rows = proto_data

        # Offset must equal the number of rows that were previously sent.
        request.offset = 2

        response_future_2 = append_rows_stream.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.AppendRowsStream.html#google_cloud_bigquery_storage_v1beta2_writer_AppendRowsStream_send(request)

        print(response_future_1.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.AppendRowsFuture.html#google_cloud_bigquery_storage_v1beta2_writer_AppendRowsFuture_result())
        print(response_future_2.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.AppendRowsFuture.html#google_cloud_bigquery_storage_v1beta2_writer_AppendRowsFuture_result())

        # Shutdown background threads and close the streaming connection.
        append_rows_stream.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1beta2.writer.AppendRowsStream.html#google_cloud_bigquery_storage_v1beta2_writer_AppendRowsStream_close()

        # A PENDING type stream must be "finalized" before being committed. No new
        # records can be written to the stream after this method has been called.
        write_client.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.services.big_query_write.BigQueryWriteClient.html#google_cloud_bigquery_storage_v1_services_big_query_write_BigQueryWriteClient_finalize_write_stream(name=write_stream.name)

        # Commit the stream you created earlier.
        batch_commit_write_streams_request = https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.html.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.types.BatchCommitWriteStreamsRequest.html()
        batch_commit_write_streams_request.parent = parent
        batch_commit_write_streams_request.write_streams = [write_stream.name]
        write_client.https://docs.cloud.google.com/python/docs/reference/bigquerystorage/latest/google.cloud.bigquery_storage_v1.services.big_query_write.BigQueryWriteClient.html#google_cloud_bigquery_storage_v1_services_big_query_write_BigQueryWriteClient_batch_commit_write_streams(batch_commit_write_streams_request)

        print(f"Writes to stream: '{write_stream.name}' have been committed.")

This code example depends on a compiled protocol module,
`customer_record_pb2.py`. To create the compiled module, execute
`protoc --python_out=. customer_record.proto`, where `protoc` is the
protocol buffer compiler. The `customer_record.proto` file defines the format
of the messages used in the Python example.


    // The BigQuery Storage API expects protocol buffer data to be encoded in the
    // proto2 wire format. This allows it to disambiguate missing optional fields
    // from default values without the need for wrapper types.
    syntax = "proto2";

    // Define a message type representing the rows in your table. The message
    // cannot contain fields which are not present in the table.
    message CustomerRecord {

      optional string customer_name = 1;

      // Use the required keyword for client-side validation of required fields.
      required int64 row_num = 2;
    }

<br />