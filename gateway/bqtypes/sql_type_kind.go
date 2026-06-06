package bqtypes

import (
	"encoding/json"
	"fmt"
)

// SqlTypeKind is StandardSqlDataType.typeKind on the wire. Gapic v2 REST
// may send the enum as a string ("INT64") or as a numeric proto enum (2).
type SqlTypeKind string

func sqlTypeKindFromNumeric(n int) (SqlTypeKind, bool) {
	switch n {
	case 2:
		return "INT64", true
	case 5:
		return "BOOL", true
	case 7:
		return "FLOAT64", true
	case 8:
		return "STRING", true
	case 9:
		return "BYTES", true
	case 10:
		return "DATE", true
	case 16:
		return "ARRAY", true
	case 17:
		return "STRUCT", true
	case 19:
		return "TIMESTAMP", true
	case 20:
		return "TIME", true
	case 21:
		return "DATETIME", true
	case 22:
		return "GEOGRAPHY", true
	case 23:
		return "NUMERIC", true
	case 24:
		return "BIGNUMERIC", true
	case 25:
		return "JSON", true
	case 26:
		return "INTERVAL", true
	default:
		return "", false
	}
}

// UnmarshalJSON accepts string enum names or numeric gapic v2 values.
func (t *SqlTypeKind) UnmarshalJSON(data []byte) error {
	if string(data) == "null" {
		*t = ""
		return nil
	}
	var raw any
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	switch v := raw.(type) {
	case string:
		*t = SqlTypeKind(v)
		return nil
	case float64:
		if tk, ok := sqlTypeKindFromNumeric(int(v)); ok {
			*t = tk
			return nil
		}
		return fmt.Errorf("bqtypes: unknown typeKind enum value %d", int(v))
	default:
		return fmt.Errorf("bqtypes: typeKind must be string or number, got %T", raw)
	}
}
