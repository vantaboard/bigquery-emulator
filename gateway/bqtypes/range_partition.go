package bqtypes

import (
	"bytes"
	"encoding/json"
	"fmt"
	"strconv"
)

// RangePartitioning describes BigQuery integer-range partitioning. The
// only currently-supported `Range.Interval` granularity is integer
// buckets (`start`, `end`, `interval`); the field is just round-tripped
// for now.
type RangePartitioning struct {
	Field string         `json:"field,omitempty"`
	Range *RangePartSpec `json:"range,omitempty"`
}

// RangePartSpec is the `range` sub-object of RangePartitioning. All
// three integer fields are wire-serialized as decimal strings to mirror
// BigQuery REST. See docs/bigquery/docs/reference/rest/v2/tables/get.md.
type RangePartSpec struct {
	Start    string `json:"start,omitempty"`
	End      string `json:"end,omitempty"`
	Interval string `json:"interval,omitempty"`
}

// UnmarshalJSON accepts JSON numbers because the Node client posts
// range.start as a number on tables.insert.
func (r *RangePartSpec) UnmarshalJSON(data []byte) error {
	var raw struct {
		Start    json.RawMessage `json:"start,omitempty"`
		End      json.RawMessage `json:"end,omitempty"`
		Interval json.RawMessage `json:"interval,omitempty"`
	}
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	var err error
	if r.Start, err = unmarshalRangeIntString(raw.Start); err != nil {
		return fmt.Errorf("start: %w", err)
	}
	if r.End, err = unmarshalRangeIntString(raw.End); err != nil {
		return fmt.Errorf("end: %w", err)
	}
	if r.Interval, err = unmarshalRangeIntString(raw.Interval); err != nil {
		return fmt.Errorf("interval: %w", err)
	}
	return nil
}

func unmarshalRangeIntString(raw json.RawMessage) (string, error) {
	if raw == nil {
		return "", nil
	}
	trim := bytes.TrimSpace(raw)
	if len(trim) == 0 || bytes.Equal(trim, []byte("null")) {
		return "", nil
	}
	if trim[0] == '"' {
		var s string
		if err := json.Unmarshal(trim, &s); err != nil {
			return "", err
		}
		return s, nil
	}
	var n json.Number
	if err := json.Unmarshal(trim, &n); err != nil {
		return "", err
	}
	i, err := n.Int64()
	if err != nil {
		return "", err
	}
	return strconv.FormatInt(i, 10), nil
}
