// Package bqstorage is the shallow-emulator skeleton for the
// BigQuery Storage Write API surface (gRPC, exposed at the storage
// gRPC port per docker-compose.yml). The gRPC layer is intentionally
// NOT registered in this skeleton because doing so would require:
//
//  1. Adding `cloud.google.com/go/bigquery/storage/apiv1/storagepb`
//     (~30 generated proto files for the streaming Write API) and the
//     associated proto-descriptor + arrow-decode helpers go-googlesql
//     keeps in `api/bqstorage/proto_descriptor_normalize.go`,
//     `proto_rows*.go`, etc. (~20 files, ~3 KLOC).
//  2. Hooking AppendRows into this repo's `backend/storage/` for row
//     materialization. The existing tabledata insertAll path is the
//     natural integration point; that surface does not yet expose
//     a streaming-friendly entry from the Go gateway.
//
// Both are well outside the shallow-emulator port budget per
// `docs/ENGINE_POLICY.md`. The
// surface-mapping table below records which failing-IT each
// go-googlesql `api/bqstorage/` symbol satisfies.
//
// Failing-IT → go-googlesql source mapping (shallow-emulator intake table):
//
//	WriteBufferedStreamIT
//	  → BigQueryWrite.CreateWriteStream
//	      ⇒ api/bqstorage/write_streams.go: (s *WriteServer).CreateWriteStream
//	  → BigQueryWrite.AppendRows (bidirectional stream)
//	      ⇒ api/bqstorage/write_append.go: (s *WriteServer).AppendRows
//	      ⇒ api/bqstorage/proto_descriptor_normalize.go (schema reconciliation)
//	      ⇒ api/bqstorage/proto_rows.go (row materialization)
//	      ⇒ api/bqstorage/proto_rows_coercion.go (BQ-type coercion)
//	      ⇒ api/bqstorage/proto_rows_repeated_list.go (REPEATED handling)
//	      ⇒ api/bqstorage/write_quota.go (quota stub: ~quota messaging only)
//	  → BigQueryWrite.FinalizeWriteStream
//	      ⇒ api/bqstorage/write_streams.go: (s *WriteServer).FinalizeWriteStream
//	  → BigQueryWrite.BatchCommitWriteStreams
//	      ⇒ api/bqstorage/write_streams.go: (s *WriteServer).BatchCommitWriteStreams
//
// Out of scope for the shallow-emulator port (deferred to the
// Storage Read follow-up): the entire `api/bqstorage/read*.go`
// family, plus `avro_arrow.go`, `tableschema.go`, `read_partition*.go`,
// `read_projection.go`, `read_rows.go`,
// `read_session_contract_test.go`, `read_source.go`,
// `read_view_parquet.go`. The emulator's existing
// `proto/storage_read.proto` surface
// (`gateway/enginepb/storage_read*.go`) covers the limited Storage
// Read surface today; the gRPC-server follow-up will fold in any
// remaining gaps.
package bqstorage

import (
	"net/http"
)

// Register is the symbolic entry point the gateway will call once
// the BigQueryWrite gRPC surface lands. Until then the gateway has
// no Write API listener; clients dialed at the storage gRPC port
// receive a clean UNIMPLEMENTED from the (existing) Storage Read
// listener for any unmapped method name.
func Register(_ *http.ServeMux) {}
