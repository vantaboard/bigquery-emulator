package models

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestRegisterFromDDLCreateModel(t *testing.T) {
	t.Parallel()
	store := NewStore()
	sql := "CREATE MODEL `p.ds.my_model` OPTIONS(model_type='linear_reg') AS SELECT 1 AS x"
	ref := RegisterFromDDL(store, "p", "ds", sql)
	if ref == nil {
		t.Fatal("expected model reference")
	}
	if ref.ModelID != "my_model" {
		t.Fatalf("modelID = %q", ref.ModelID)
	}
	m, ok := store.Get("p", "ds", "my_model")
	if !ok {
		t.Fatal("model not found")
	}
	if m.ModelType != "LINEAR_REG" {
		t.Fatalf("modelType = %q", m.ModelType)
	}
}

func TestStoreListDeleteRoundTrip(t *testing.T) {
	t.Parallel()
	store := NewStore()
	store.Upsert(bqtypes.Model{
		ModelReference: bqtypes.ModelReference{ProjectID: "p", DatasetID: "d", ModelID: "m1"},
		ModelType:      "LINEAR_REG",
	})
	if len(store.List("p", "d", "")) != 1 {
		t.Fatal("expected one model")
	}
	if !store.Delete("p", "d", "m1") {
		t.Fatal("delete failed")
	}
	if _, ok := store.Get("p", "d", "m1"); ok {
		t.Fatal("model still present")
	}
}
