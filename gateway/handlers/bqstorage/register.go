package bqstorage

import (
	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
)

// RegisterGRPC wires the public BigQuery Storage Read/Write services onto
// srv. The gateway calls this during startup so client libraries dialing
// BIGQUERY_STORAGE_GRPC_ENDPOINT reach google.cloud.bigquery.storage.v1
// rather than the engine-internal bigquery_emulator.v1.* service names.
// metadata supplies the dataset locations that write streams report.
func RegisterGRPC(srv grpc.ServiceRegistrar, eng *engine.Client, metadata *handlers.MetadataStore) {
	if srv == nil {
		return
	}
	read := &ReadServer{engine: eng}
	write := &WriteServer{engine: eng, metadata: metadata}
	storagepb.RegisterBigQueryReadServer(srv, read)
	storagepb.RegisterBigQueryWriteServer(srv, write)
}
