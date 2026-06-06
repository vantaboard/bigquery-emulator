package bqtypes

import (
	"encoding/json"
	"testing"
)

func TestRangePartSpecUnmarshalNumeric(t *testing.T) {
	t.Parallel()
	var spec RangePartSpec
	if err := json.Unmarshal([]byte(`{"start":0,"end":100,"interval":10}`), &spec); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}
	if spec.Start != "0" || spec.End != "100" || spec.Interval != "10" {
		t.Fatalf("spec = %+v", spec)
	}
}
