package bqtypes_test

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestEmulatorCMEKKeyUSCentral(t *testing.T) {
	t.Parallel()
	got := bqtypes.EmulatorCMEKKeyUSCentral("my-project", "test")
	want := "projects/my-project/locations/us-central1/keyRings/emulator/cryptoKeys/test"
	if got != want {
		t.Fatalf("got %q, want %q", got, want)
	}
}
