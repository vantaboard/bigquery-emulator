package grpcserver

import (
	"context"
	"strings"
	"testing"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/status"
)

func TestStartAcceptsRequestsUpToMaxRequestBytes(t *testing.T) {
	t.Parallel()
	srv, err := Start("127.0.0.1:0", nil, handlers.Dependencies{})
	if err != nil {
		t.Fatalf("Start: %v", err)
	}
	go func() { _ = srv.Serve() }()
	t.Cleanup(func() { _ = srv.Close() })

	conn, err := grpc.NewClient(srv.lis.Addr().String(), grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		t.Fatalf("NewClient: %v", err)
	}
	t.Cleanup(func() { _ = conn.Close() })
	client := storagepb.NewBigQueryWriteClient(conn)

	cases := []struct {
		name  string
		bytes int
		want  codes.Code
	}{
		// Without an engine, a request the server accepts reaches the
		// handler, which answers Unavailable.
		{name: "above gRPC's default 4 MiB", bytes: 5 << 20, want: codes.Unavailable},
		{name: "above MaxRequestBytes", bytes: MaxRequestBytes + 1, want: codes.ResourceExhausted},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			_, err := client.GetWriteStream(context.Background(), &storagepb.GetWriteStreamRequest{
				Name: strings.Repeat("x", tc.bytes),
			})
			if got := status.Code(err); got != tc.want {
				t.Fatalf("GetWriteStream with a %d-byte request: code %v, want %v (%v)", tc.bytes, got, tc.want, err)
			}
		})
	}
}
