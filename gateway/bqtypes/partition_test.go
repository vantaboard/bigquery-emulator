package bqtypes

import (
	"testing"
	"time"
)

const (
	eventsTable   = "events"
	dayPartition  = "DAY"
	hourPartition = "HOUR"
)

func TestSplitPartitionDecorator(t *testing.T) {
	cases := []struct {
		id, base, typ, start string
	}{
		{eventsTable, eventsTable, "", ""},
		{"events$2024", eventsTable, "YEAR", "2024-01-01 00:00:00+00"},
		{"events$202403", eventsTable, "MONTH", "2024-03-01 00:00:00+00"},
		{"events$20240304", eventsTable, dayPartition, "2024-03-04 00:00:00+00"},
		{"events$2024030410", eventsTable, hourPartition, "2024-03-04 10:00:00+00"},
	}
	for _, tc := range cases {
		base, partition, err := SplitPartitionDecorator(tc.id)
		if err != nil {
			t.Fatalf("%s: %v", tc.id, err)
		}
		if base != tc.base || (partition == nil) != (tc.typ == "") {
			t.Fatalf("%s: got %q, %+v", tc.id, base, partition)
		}
		if partition != nil && (partition.Type != tc.typ || partition.Time != tc.start) {
			t.Errorf("%s: got %+v, want %s %s", tc.id, partition, tc.typ, tc.start)
		}
	}
	for _, id := range []string{"events$2024030", "events$20241304", "events$__UNPARTITIONED__"} {
		if _, _, err := SplitPartitionDecorator(id); err == nil {
			t.Errorf("%s: want an error", id)
		}
	}
}

func TestPartitionDecoratorCheckTable(t *testing.T) {
	day := &PartitionDecorator{Type: dayPartition}
	if err := day.CheckTable(eventsTable, dayPartition); err != nil {
		t.Errorf("DAY table: %v", err)
	}
	for _, typ := range []string{"", hourPartition} {
		if err := day.CheckTable(eventsTable, typ); err == nil {
			t.Errorf("%q table: want an error", typ)
		}
	}
}

func TestPartitionStart(t *testing.T) {
	cases := []struct {
		value any
		typ   string
		want  string
	}{
		{"2024-03-04T10:30:00Z", dayPartition, "2024-03-04 00:00:00+00"},
		{"2024-03-04 10:30:00+00", hourPartition, "2024-03-04 10:00:00+00"},
		{"2024-03-04 10:30:00", "MONTH", "2024-03-01 00:00:00+00"},
		{time.Date(2024, 3, 4, 10, 30, 0, 0, time.UTC), "YEAR", "2024-01-01 00:00:00+00"},
	}
	for _, tc := range cases {
		got, err := PartitionStart(tc.value, tc.typ)
		if err != nil || got != tc.want {
			t.Errorf("PartitionStart(%v, %s) = %q, %v; want %q", tc.value, tc.typ, got, err, tc.want)
		}
	}
	for _, value := range []any{"yesterday", 42} {
		if _, err := PartitionStart(value, dayPartition); err == nil {
			t.Errorf("PartitionStart(%v): want an error", value)
		}
	}
}
