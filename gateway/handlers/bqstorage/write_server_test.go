package bqstorage

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
)

func TestWriteServerLocation(t *testing.T) {
	t.Parallel()
	metadata := handlers.NewMetadataStore()
	metadata.PutDataset("p", "eu", bqtypes.Dataset{Location: "EU"})
	metadata.PutDataset("p", "unset", bqtypes.Dataset{})
	server := &WriteServer{metadata: metadata}

	cases := []struct {
		name string
		want string
	}{
		{name: "projects/p/datasets/eu/tables/t/streams/_default", want: "EU"},
		{name: "projects/p/datasets/eu/tables/t", want: "EU"},
		{name: "projects/p/datasets/unset/tables/t/streams/_default", want: "US"},
		{name: "projects/p/datasets/missing/tables/t/streams/_default", want: "US"},
		{name: "not-a-stream-name", want: "US"},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			if got := server.location(tc.name); got != tc.want {
				t.Fatalf("location(%q) = %q, want %q", tc.name, got, tc.want)
			}
		})
	}

	if got := (&WriteServer{}).location("projects/p/datasets/eu/tables/t"); got != "US" {
		t.Fatalf("location without a metadata store = %q, want %q", got, "US")
	}
}
