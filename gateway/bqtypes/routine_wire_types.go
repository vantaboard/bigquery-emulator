package bqtypes

import (
	"encoding/json"
	"fmt"
)

// RoutineReference is a stable handle to a routine (UDF / TVF / procedure).
type RoutineReference struct {
	ProjectID string `json:"projectId"`
	DatasetID string `json:"datasetId"`
	RoutineID string `json:"routineId"`
}

// StandardSqlDataType mirrors the BigQuery REST StandardSqlDataType
// resource. See docs/bigquery/docs/reference/rest/v2/StandardSqlDataType.md.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlDataType struct {
	TypeKind         SQLTypeKind            `json:"typeKind"`
	ArrayElementType *StandardSqlDataType   `json:"arrayElementType,omitempty"`
	StructType       *StandardSqlStructType `json:"structType,omitempty"`
	RangeElementType *StandardSqlDataType   `json:"rangeElementType,omitempty"`
}

// StandardSqlStructType is the struct sub-object of StandardSqlDataType.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlStructType struct {
	Fields []StandardSqlField `json:"fields,omitempty"`
}

// StandardSqlField is one field of a STRUCT type.
//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlField struct {
	Name string              `json:"name"`
	Type StandardSqlDataType `json:"type"`
}

//
//nolint:revive // wire name uses Sql, not SQL
type StandardSqlTableType struct {
	Columns []StandardSqlField `json:"columns,omitempty"`
}

// RoutineArgument is an input/output argument of a routine.
type RoutineArgument struct {
	Name         string               `json:"name,omitempty"`
	ArgumentKind string               `json:"argumentKind,omitempty"`
	Mode         string               `json:"mode,omitempty"`
	DataType     *StandardSqlDataType `json:"dataType,omitempty"`
}

// RoutineType is the fine-grained routine kind on the wire. Gapic v2
// REST may send the enum as a string ("SCALAR_FUNCTION") or as a
// numeric proto enum (1 = SCALAR_FUNCTION, 2 = PROCEDURE, …).
type RoutineType string

func routineTypeFromNumeric(n int) (RoutineType, bool) {
	switch n {
	case 1:
		return "SCALAR_FUNCTION", true
	case 2:
		return "PROCEDURE", true
	case 3:
		return "TABLE_VALUED_FUNCTION", true
	default:
		return "", false
	}
}

// UnmarshalJSON accepts string enum names or numeric gapic v2 values.
func (t *RoutineType) UnmarshalJSON(data []byte) error {
	if string(data) == jsonNullLiteral {
		*t = ""
		return nil
	}
	var raw any
	if err := json.Unmarshal(data, &raw); err != nil {
		return err
	}
	switch v := raw.(type) {
	case string:
		*t = RoutineType(v)
		return nil
	case float64:
		if rt, ok := routineTypeFromNumeric(int(v)); ok {
			*t = rt
			return nil
		}
		return fmt.Errorf("bqtypes: unknown routineType enum value %d", int(v))
	default:
		return fmt.Errorf("bqtypes: routineType must be string or number, got %T", raw)
	}
}

// Routine is the BigQuery Routine resource (subset).
// See docs/bigquery/docs/reference/rest/v2/routines.md.
type Routine struct {
	Etag             string                `json:"etag,omitempty"`
	RoutineReference RoutineReference      `json:"routineReference"`
	RoutineType      RoutineType           `json:"routineType,omitempty"`
	CreationTime     string                `json:"creationTime,omitempty"`
	LastModifiedTime string                `json:"lastModifiedTime,omitempty"`
	Language         RoutineLanguage       `json:"language,omitempty"`
	Arguments        []RoutineArgument     `json:"arguments,omitempty"`
	ReturnType       *StandardSqlDataType  `json:"returnType,omitempty"`
	ReturnTableType  *StandardSqlTableType `json:"returnTableType,omitempty"`
	DefinitionBody   string                `json:"definitionBody,omitempty"`
	Description      string                `json:"description,omitempty"`
	StrictMode       *bool                 `json:"strictMode,omitempty"`
	PythonOptions    *PythonOptions        `json:"pythonOptions,omitempty"`
}

// PythonOptions mirrors the BigQuery REST PythonOptions resource for
// LANGUAGE python routines.
type PythonOptions struct {
	EntryPoint string   `json:"entryPoint,omitempty"`
	Packages   []string `json:"packages,omitempty"`
}
