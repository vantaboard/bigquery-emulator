package seedfile

import (
	"context"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

const partitionedYAML = `
project_id: proj
datasets:
  - id: ds
    tables:
      - id: events
        time_partitioning:
          type: DAY
        schema:
          - name: id
            type: INT64
        rows:
          - id: 1
            _PARTITIONTIME: 2024-03-04T10:30:00Z
          - id: 2
`

func TestApplyPartitionsRowsByTheirPartitionTime(t *testing.T) {
	f, err := Decode([]byte(partitionedYAML), "partitioned.yaml")
	if err != nil {
		t.Fatalf("Decode: %v", err)
	}
	catalog := &insertRowsCaptureClient{}
	if err = Apply(context.Background(), f, seed.NewCatalogApplier(catalog), seed.Defaults{}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
	rows := catalog.lastInsertRows.GetRows()
	if len(rows) != 2 {
		t.Fatalf("len(rows) = %d, want 2", len(rows))
	}
	if got := rows[0].GetPartitionTime(); got != "2024-03-04 00:00:00+00" {
		t.Errorf("rows[0] partition time = %q, want the 2024-03-04 partition", got)
	}
	if got := rows[1].GetPartitionTime(); got != "" {
		t.Errorf("rows[1] partition time = %q, want empty (insert time)", got)
	}
	if got := len(rows[0].GetCells()); got != 1 {
		t.Errorf("rows[0] has %d cells, want 1", got)
	}
}
