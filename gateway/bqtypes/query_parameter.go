package bqtypes

import (
	"bytes"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
)

const parameterTypeStruct = "STRUCT"

// UnmarshalJSON accepts BigQuery REST parameter values where `value`
// may be encoded as a JSON string, number, or bool. The engine expects
// a decimal string in `value_json`; scalars are normalized here.
func (v *QueryParameterValue) UnmarshalJSON(data []byte) error {
	type wireQueryParameterValue struct {
		Value        json.RawMessage            `json:"value"`
		ArrayValues  []json.RawMessage          `json:"arrayValues"`
		StructValues map[string]json.RawMessage `json:"structValues"`
	}
	var raw wireQueryParameterValue
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	if raw.Value != nil {
		v.Value = normalizeParameterScalarJSON(raw.Value)
	}
	if len(raw.ArrayValues) > 0 {
		v.ArrayValues = make([]QueryParameterValue, 0, len(raw.ArrayValues))
		for _, elem := range raw.ArrayValues {
			var nested QueryParameterValue
			if err := json.Unmarshal(elem, &nested); err != nil {
				// Element may be a bare scalar (number/bool/string).
				nested.Value = normalizeParameterScalarJSON(elem)
			}
			v.ArrayValues = append(v.ArrayValues, nested)
		}
	}
	if len(raw.StructValues) > 0 {
		v.StructValues = make(map[string]QueryParameterValue, len(raw.StructValues))
		for name, field := range raw.StructValues {
			var nested QueryParameterValue
			if err := json.Unmarshal(field, &nested); err != nil {
				nested.Value = normalizeParameterScalarJSON(field)
			}
			v.StructValues[name] = nested
		}
	}
	return nil
}

// ValueJSON returns the JSON literal string forwarded to the engine
// as `value_json`. Scalars use the normalized `value` field; ARRAY
// and STRUCT parameters marshal the nested shape.
func (v *QueryParameterValue) ValueJSON() string {
	if v == nil {
		return ""
	}
	if len(v.ArrayValues) > 0 {
		elems := make([]json.RawMessage, 0, len(v.ArrayValues))
		for i := range v.ArrayValues {
			elems = append(elems, json.RawMessage(v.ArrayValues[i].marshalParameterJSON()))
		}
		raw, err := json.Marshal(elems)
		if err != nil {
			return "[]"
		}
		return string(raw)
	}
	if len(v.StructValues) > 0 {
		obj := make(map[string]json.RawMessage, len(v.StructValues))
		for name, field := range v.StructValues {
			obj[name] = json.RawMessage(field.marshalParameterJSON())
		}
		raw, err := json.Marshal(obj)
		if err != nil {
			return "{}"
		}
		return string(raw)
	}
	return v.Value
}

// ParameterTypeWire returns the engine `type_kind` and optional
// `type_json` descriptor for a REST query parameter type.
func ParameterTypeWire(t *QueryParameterType) (typeKind, typeJSON string) {
	if t == nil {
		return "", ""
	}
	switch t.Type {
	case parameterTypeStruct:
		if len(t.StructTypes) == 0 {
			return parameterTypeStruct, ""
		}
		parts := make([]string, 0, len(t.StructTypes))
		for _, st := range t.StructTypes {
			fk, _ := ParameterTypeWire(&st.Type)
			parts = append(parts, st.Name+":"+fk)
		}
		return parameterTypeStruct, strings.Join(parts, ",")
	default:
		return t.Type, ""
	}
}

// ParameterValueWire returns the JSON literal forwarded as engine
// `value_json`. STRUCT parameters use a positional JSON array aligned
// with `parameterType.structTypes`.
func ParameterValueWire(pt *QueryParameterType, v *QueryParameterValue) string {
	if v == nil {
		return ""
	}
	if pt != nil && pt.Type == parameterTypeStruct && len(pt.StructTypes) > 0 &&
		len(v.StructValues) > 0 {
		elems := make([]json.RawMessage, 0, len(pt.StructTypes))
		for _, st := range pt.StructTypes {
			fv := v.StructValues[st.Name]
			elems = append(elems, json.RawMessage(fv.marshalParameterJSON()))
		}
		raw, err := json.Marshal(elems)
		if err != nil {
			return "[]"
		}
		return string(raw)
	}
	return v.ValueJSON()
}

func (v QueryParameterValue) marshalParameterJSON() []byte {
	if len(v.ArrayValues) > 0 || len(v.StructValues) > 0 {
		raw, err := json.Marshal(v)
		if err != nil {
			return []byte("null")
		}
		return raw
	}
	if v.Value == "" {
		return []byte("null")
	}
	// Re-marshal through json so numeric/bool strings become proper JSON.
	var decoded any
	if err := json.Unmarshal([]byte(v.Value), &decoded); err == nil {
		raw, err := json.Marshal(decoded)
		if err == nil {
			return raw
		}
	}
	return []byte(strconv.Quote(v.Value))
}

// normalizeParameterScalarJSON converts a JSON scalar token into the
// decimal-string form the C++ parameter parser expects.
func normalizeParameterScalarJSON(raw json.RawMessage) string {
	trimmed := bytes.TrimSpace(raw)
	if len(trimmed) == 0 || bytes.Equal(trimmed, []byte("null")) {
		return ""
	}
	var asString string
	if err := json.Unmarshal(trimmed, &asString); err == nil {
		return asString
	}
	var asBool bool
	if err := json.Unmarshal(trimmed, &asBool); err == nil {
		return strconv.FormatBool(asBool)
	}
	var asInt int64
	if err := json.Unmarshal(trimmed, &asInt); err == nil {
		return strconv.FormatInt(asInt, 10)
	}
	var asFloat float64
	if err := json.Unmarshal(trimmed, &asFloat); err == nil {
		return strconv.FormatFloat(asFloat, 'f', -1, 64)
	}
	return string(trimmed)
}

// ParseQueryParameters unmarshals a queryParameters JSON array, used
// by unit tests and any handler that decodes parameters outside the
// main QueryRequest body.
func ParseQueryParameters(data []byte) ([]QueryParameter, error) {
	var params []QueryParameter
	if len(data) == 0 {
		return nil, nil
	}
	if err := json.Unmarshal(data, &params); err != nil {
		return nil, fmt.Errorf("parse queryParameters: %w", err)
	}
	return params, nil
}
