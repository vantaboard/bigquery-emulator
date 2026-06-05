package bqtypes_test

import (
	"encoding/json"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestResourceLabelsUnmarshalNullDeletesKey(t *testing.T) {
	t.Parallel()
	var labels bqtypes.ResourceLabels
	if err := json.Unmarshal([]byte(`{"color":null,"env":"prod"}`), &labels); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if _, ok := labels["color"]; ok {
		t.Fatalf("labels = %v, want color removed", labels)
	}
	if labels["env"] != "prod" {
		t.Fatalf("labels = %v, want env=prod", labels)
	}
}

func TestResourceLabelsMarshalNilIsEmptyObject(t *testing.T) {
	t.Parallel()
	var labels bqtypes.ResourceLabels
	raw, err := json.Marshal(labels)
	if err != nil {
		t.Fatalf("Marshal: %v", err)
	}
	if string(raw) != `{}` {
		t.Fatalf("got %s, want {}", raw)
	}
}

func TestTableExpirationTimeUnmarshalNumber(t *testing.T) {
	t.Parallel()
	var table bqtypes.Table
	if err := json.Unmarshal([]byte(`{"expirationTime":1234567890}`), &table); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if table.ExpirationTime.String() != "1234567890" {
		t.Fatalf("expirationTime = %q, want %q", table.ExpirationTime, "1234567890")
	}
}

func TestUnmarshalWriteDispositionArray(t *testing.T) {
	t.Parallel()
	wd, err := bqtypes.UnmarshalWriteDisposition([]byte(`["WRITE_APPEND"]`))
	if err != nil {
		t.Fatalf("UnmarshalWriteDisposition: %v", err)
	}
	if wd != "WRITE_APPEND" {
		t.Fatalf("writeDisposition = %q, want WRITE_APPEND", wd)
	}
}
