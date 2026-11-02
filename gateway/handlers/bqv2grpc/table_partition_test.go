package bqv2grpc

import (
	"context"
	"testing"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/protobuf/types/known/wrapperspb"
)

type registeringCatalog struct {
	enginepb.CatalogClient
	registered *enginepb.RegisterTableRequest
}

func (c *registeringCatalog) RegisterTable(
	_ context.Context,
	in *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	c.registered = in
	return &enginepb.RegisterTableResponse{}, nil
}

func TestInsertTableForwardsIngestionPartitionType(t *testing.T) {
	cases := []struct {
		name             string
		timePartitioning *bigquerypb.TimePartitioning
		want             string
	}{
		{"not partitioned", nil, ""},
		{"ingestion time", &bigquerypb.TimePartitioning{Type: "HOUR"}, "HOUR"},
		{"column", &bigquerypb.TimePartitioning{Type: "DAY", Field: wrapperspb.String("ts")}, ""},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			cat := &registeringCatalog{}
			server := newTableServer(handlers.Dependencies{Catalog: cat, Metadata: handlers.NewMetadataStore()})
			_, err := server.InsertTable(context.Background(), &bigquerypb.InsertTableRequest{
				ProjectId: "proj",
				DatasetId: "ds",
				Table: &bigquerypb.Table{
					TableReference:   &bigquerypb.TableReference{TableId: "t"},
					TimePartitioning: tc.timePartitioning,
				},
			})
			if err != nil {
				t.Fatalf("InsertTable: %v", err)
			}
			if got := cat.registered.GetSchema().GetIngestionPartitionType(); got != tc.want {
				t.Errorf("ingestion_partition_type = %q, want %q", got, tc.want)
			}
		})
	}
}
