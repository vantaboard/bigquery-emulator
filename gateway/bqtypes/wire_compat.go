package bqtypes

import (
	"bytes"
	"encoding/json"
	"fmt"
	"maps"
	"strconv"
)

// injectJSONStringField forces a top-level string field onto an encoded object.
func injectJSONStringField(raw []byte, key, value string) ([]byte, error) {
	var doc map[string]json.RawMessage
	if err := json.Unmarshal(raw, &doc); err != nil {
		return nil, err
	}
	encoded, err := json.Marshal(value)
	if err != nil {
		return nil, err
	}
	doc[key] = encoded
	return json.Marshal(doc)
}

// marshalWithoutJSONField JSON-encodes v while dropping one top-level field.
func marshalWithoutJSONField(v any, dropField string) ([]byte, error) {
	raw, err := json.Marshal(v)
	if err != nil {
		return nil, err
	}
	var doc map[string]json.RawMessage
	if err := json.Unmarshal(raw, &doc); err != nil {
		return nil, err
	}
	delete(doc, dropField)
	return json.Marshal(doc)
}

// ResourceLabels is a BigQuery labels map on Dataset/Table resources.
// UnmarshalJSON accepts null values as deletion markers (the upstream
// Node `deleteLabelDataset` sample sends `{color: null}` via
// setMetadata). MarshalJSON always emits `{}` for a nil map so client
// libraries that call `Object.entries(resource.labels)` never see a
// missing field.
type ResourceLabels map[string]string

// MarshalJSON implements json.Marshaler.
func (l ResourceLabels) MarshalJSON() ([]byte, error) {
	if l == nil {
		return []byte("{}"), nil
	}
	return json.Marshal(map[string]string(l))
}

// UnmarshalJSON implements json.Unmarshaler.
func (l *ResourceLabels) UnmarshalJSON(data []byte) error {
	var raw map[string]json.RawMessage
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	out := make(ResourceLabels, len(raw))
	for k, v := range raw {
		if bytes.Equal(bytes.TrimSpace(v), []byte("null")) {
			continue
		}
		var s string
		if err := json.Unmarshal(v, &s); err != nil {
			return fmt.Errorf("labels[%q]: %w", k, err)
		}
		out[k] = s
	}
	*l = out
	return nil
}

// MillisTimestamp is a BigQuery REST int64 millis-since-epoch field
// encoded as a decimal string on the wire. UnmarshalJSON also accepts
// JSON numbers because the Node client sometimes posts expirationTime
// as a number on tables.insert.
type MillisTimestamp string

// String returns the canonical decimal string form.
func (t MillisTimestamp) String() string {
	return string(t)
}

// UnmarshalJSON implements json.Unmarshaler.
func (t *MillisTimestamp) UnmarshalJSON(data []byte) error {
	data = bytes.TrimSpace(data)
	if bytes.Equal(data, []byte("null")) {
		*t = ""
		return nil
	}
	if len(data) > 0 && data[0] == '"' {
		var s string
		if err := json.Unmarshal(data, &s); err != nil {
			return err
		}
		*t = MillisTimestamp(s)
		return nil
	}
	var n json.Number
	if err := json.Unmarshal(data, &n); err != nil {
		return fmt.Errorf("millis timestamp: %w", err)
	}
	i, err := n.Int64()
	if err != nil {
		return fmt.Errorf("millis timestamp: %w", err)
	}
	*t = MillisTimestamp(strconv.FormatInt(i, 10))
	return nil
}

type labelsPatch struct {
	values  map[string]string
	delete  []string
	present bool
}

func parseLabelsJSON(data json.RawMessage) (labelsPatch, error) {
	var patch labelsPatch
	if data == nil {
		return patch, nil
	}
	patch.present = true
	var raw map[string]json.RawMessage
	if err := json.Unmarshal(data, &raw); err != nil {
		return patch, err
	}
	patch.values = make(map[string]string, len(raw))
	for key, val := range raw {
		if bytes.Equal(bytes.TrimSpace(val), []byte("null")) {
			patch.delete = append(patch.delete, key)
			continue
		}
		var s string
		if err := json.Unmarshal(val, &s); err != nil {
			return patch, fmt.Errorf("labels[%q]: %w", key, err)
		}
		patch.values[key] = s
	}
	return patch, nil
}

// ApplyLabelsPatch merges explicit labels updates, including JSON-null deletions.
func ApplyLabelsPatch(
	base map[string]string,
	present bool,
	values map[string]string,
	deleteKeys []string,
) map[string]string {
	if !present {
		return base
	}
	out := make(map[string]string, len(base)+len(values))
	maps.Copy(out, base)
	for _, k := range deleteKeys {
		delete(out, k)
	}
	maps.Copy(out, values)
	return out
}

// UnmarshalWriteDisposition accepts a JSON string or a one-element
// string array (the upstream `relaxColumnQueryAppend` sample posts
// writeDisposition as ['WRITE_APPEND']).
func UnmarshalWriteDisposition(raw json.RawMessage) (string, error) {
	if len(raw) == 0 || bytes.Equal(bytes.TrimSpace(raw), []byte("null")) {
		return "", nil
	}
	trim := bytes.TrimSpace(raw)
	if len(trim) > 0 && trim[0] == '[' {
		var arr []string
		if err := json.Unmarshal(trim, &arr); err != nil {
			return "", fmt.Errorf("writeDisposition: %w", err)
		}
		if len(arr) == 1 {
			return arr[0], nil
		}
		return "", fmt.Errorf("writeDisposition: want single-element array, got %d elements", len(arr))
	}
	var s string
	if err := json.Unmarshal(trim, &s); err != nil {
		return "", fmt.Errorf("writeDisposition: %w", err)
	}
	return s, nil
}
