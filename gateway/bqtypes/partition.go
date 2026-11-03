package bqtypes

import (
	"fmt"
	"strings"
	"time"
)

// PartitionDecorator is the partition that a `table$partition` id names.
type PartitionDecorator struct {
	// Type is YEAR, MONTH, DAY or HOUR, from the length of the decorator.
	Type string
	// Time is the start of the partition, in the form the engine takes.
	Time string
}

var partitionDecoratorLayouts = map[int]struct{ typ, layout string }{
	4:  {"YEAR", "2006"},
	6:  {"MONTH", "200601"},
	8:  {"DAY", "20060102"},
	10: {"HOUR", "2006010215"},
}

// SplitPartitionDecorator splits `table$YYYY[MM[DD[HH]]]` into the table id
// and the partition it names. The decorator is nil when the id has no `$`.
func SplitPartitionDecorator(tableID string) (string, *PartitionDecorator, error) {
	base, suffix, found := strings.Cut(tableID, "$")
	if !found {
		return tableID, nil, nil
	}
	l, ok := partitionDecoratorLayouts[len(suffix)]
	t, err := time.Parse(l.layout, suffix)
	if !ok || err != nil {
		return "", nil, fmt.Errorf("invalid partition decorator $%s in table id %s", suffix, tableID)
	}
	return base, &PartitionDecorator{Type: l.typ, Time: formatPartitionTime(t)}, nil
}

// CheckTable returns an error unless the table is partitioned by ingestion
// time with the decorator's partition type.
func (d *PartitionDecorator) CheckTable(tableID, ingestionPartitionType string) error {
	switch ingestionPartitionType {
	case d.Type:
		return nil
	case "":
		return fmt.Errorf("table %s is not partitioned by ingestion time, so it takes no partition decorator", tableID)
	default:
		return fmt.Errorf("table %s is partitioned by %s, but the partition decorator is for %s partitions",
			tableID, ingestionPartitionType, d.Type)
	}
}

// PartitionStart returns the start of the partition of type `typ` (YEAR,
// MONTH, DAY or HOUR) that holds the timestamp `value`, in the form the engine
// takes.
func PartitionStart(value any, typ string) (string, error) {
	var t time.Time
	switch v := value.(type) {
	case time.Time:
		t = v
	case string:
		parsed, err := parseTimestampWireString(v)
		if err != nil {
			return "", fmt.Errorf("partition time %q: %w", v, err)
		}
		t = parsed
	default:
		return "", fmt.Errorf("partition time %v is not a timestamp", value)
	}
	t = t.UTC()
	switch typ {
	case "HOUR":
		t = t.Truncate(time.Hour)
	case "MONTH":
		t = time.Date(t.Year(), t.Month(), 1, 0, 0, 0, 0, time.UTC)
	case "YEAR":
		t = time.Date(t.Year(), time.January, 1, 0, 0, 0, 0, time.UTC)
	default:
		t = time.Date(t.Year(), t.Month(), t.Day(), 0, 0, 0, 0, time.UTC)
	}
	return formatPartitionTime(t), nil
}

func formatPartitionTime(t time.Time) string {
	return t.UTC().Format("2006-01-02 15:04:05+00")
}
