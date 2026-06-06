package bqtypes

import (
	"encoding/json"
	"fmt"
)

// RoutineLanguage is the routine language on the wire. Gapic v2 REST may
// send the enum as a string ("SQL") or as a numeric proto enum (1).
type RoutineLanguage string

func routineLanguageFromNumeric(n int) (RoutineLanguage, bool) {
	switch n {
	case 1:
		return "SQL", true
	case 2:
		return "JAVASCRIPT", true
	case 3:
		return "PYTHON", true
	case 4:
		return "JAVA", true
	case 5:
		return "SCALA", true
	default:
		return "", false
	}
}

// UnmarshalJSON accepts string enum names or numeric gapic v2 values.
func (l *RoutineLanguage) UnmarshalJSON(data []byte) error {
	if string(data) == "null" {
		*l = ""
		return nil
	}
	var raw any
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	switch v := raw.(type) {
	case string:
		*l = RoutineLanguage(v)
		return nil
	case float64:
		if lang, ok := routineLanguageFromNumeric(int(v)); ok {
			*l = lang
			return nil
		}
		return fmt.Errorf("bqtypes: unknown language enum value %d", int(v))
	default:
		return fmt.Errorf("bqtypes: language must be string or number, got %T", raw)
	}
}
