package routines

import (
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// BuildDDLFromRoutine renders a CREATE statement suitable for engine
// registration and DuckDB persistence from a REST Routine resource.
func BuildDDLFromRoutine(rt bqtypes.Routine) string {
	ref := rt.RoutineReference
	name := fmt.Sprintf("`%s.%s`", ref.DatasetID, ref.RoutineID)
	switch string(rt.RoutineType) {
	case routineTypeTableFunction:
		return buildTableFunctionDDL(name, rt)
	case routineTypeProcedure:
		return buildProcedureDDL(name, rt)
	default:
		return buildScalarFunctionDDL(name, rt)
	}
}

func buildScalarFunctionDDL(name string, rt bqtypes.Routine) string {
	var b strings.Builder
	b.WriteString("CREATE OR REPLACE FUNCTION ")
	b.WriteString(name)
	b.WriteString("(")
	b.WriteString(formatArgumentList(rt.Arguments))
	b.WriteString(")")
	if rt.ReturnType != nil {
		b.WriteString(" RETURNS ")
		b.WriteString(formatSQLType(rt.ReturnType))
	}
	if rt.Language != "" && !strings.EqualFold(string(rt.Language), routineLanguageSQL) {
		b.WriteString(" LANGUAGE ")
		b.WriteString(string(rt.Language))
	}
	b.WriteString(" AS (")
	b.WriteString(rt.DefinitionBody)
	b.WriteString(")")
	return b.String()
}

func buildTableFunctionDDL(name string, rt bqtypes.Routine) string {
	var b strings.Builder
	b.WriteString("CREATE OR REPLACE TABLE FUNCTION ")
	b.WriteString(name)
	b.WriteString("(")
	b.WriteString(formatArgumentList(rt.Arguments))
	b.WriteString(") AS (")
	b.WriteString(rt.DefinitionBody)
	b.WriteString(")")
	return b.String()
}

func buildProcedureDDL(name string, rt bqtypes.Routine) string {
	var b strings.Builder
	b.WriteString("CREATE OR REPLACE PROCEDURE ")
	b.WriteString(name)
	b.WriteString("(")
	b.WriteString(formatArgumentList(rt.Arguments))
	b.WriteString(") BEGIN ")
	b.WriteString(rt.DefinitionBody)
	b.WriteString(" END")
	return b.String()
}

func formatArgumentList(args []bqtypes.RoutineArgument) string {
	if len(args) == 0 {
		return ""
	}
	parts := make([]string, 0, len(args))
	for _, arg := range args {
		typ := sqlTypeAnyType
		if arg.DataType != nil {
			typ = formatSQLType(arg.DataType)
		}
		parts = append(parts, fmt.Sprintf("%s %s", arg.Name, typ))
	}
	return strings.Join(parts, ", ")
}

func formatSQLType(t *bqtypes.StandardSqlDataType) string {
	if t == nil {
		return sqlTypeAnyType
	}
	kind := string(t.TypeKind)
	if strings.EqualFold(kind, sqlTypeArray) && t.ArrayElementType != nil {
		return fmt.Sprintf("ARRAY<%s>", formatSQLType(t.ArrayElementType))
	}
	if strings.EqualFold(kind, sqlTypeStruct) && t.StructType != nil {
		fields := make([]string, 0, len(t.StructType.Fields))
		for _, f := range t.StructType.Fields {
			fields = append(fields, fmt.Sprintf("%s %s", f.Name, formatSQLType(&f.Type)))
		}
		return fmt.Sprintf("STRUCT<%s>", strings.Join(fields, ", "))
	}
	return kind
}
