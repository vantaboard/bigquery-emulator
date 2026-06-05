package routines

import (
	"testing"
)

func TestRegisterFromDDLCreateFunction(t *testing.T) {
	store := NewStore()
	sql := `CREATE FUNCTION myfunc(x INT64) RETURNS INT64 AS (x * 3)`
	ref := RegisterFromDDL(store, "proj", "ds", sql)
	if ref == nil {
		t.Fatal("RegisterFromDDL returned nil")
	}
	if ref.ProjectID != "proj" || ref.DatasetID != "ds" || ref.RoutineID != "myfunc" {
		t.Fatalf("ref = %+v", ref)
	}
	rt, ok := store.Get("proj", "ds", "myfunc")
	if !ok {
		t.Fatal("routine not stored")
	}
	if rt.DefinitionBody != "x * 3" {
		t.Errorf("body = %q", rt.DefinitionBody)
	}
	if rt.RoutineType != routineTypeScalarFunction {
		t.Errorf("type = %q", rt.RoutineType)
	}
}

func TestRegisterFromDDLComplexArgs(t *testing.T) {
	store := NewStore()
	sql := `CREATE FUNCTION ds.myfunc(
		arr ARRAY<STRUCT<name STRING, val INT64>>
	) AS (
		(SELECT SUM(IF(elem.name = "foo",elem.val,null)) FROM UNNEST(arr) AS elem)
	)`
	ref := RegisterFromDDL(store, "proj", "default_ds", sql)
	if ref == nil {
		t.Fatal("RegisterFromDDL returned nil")
	}
	if ref.DatasetID != "ds" || ref.RoutineID != "myfunc" {
		t.Fatalf("ref = %+v", ref)
	}
	rt, ok := store.Get("proj", "ds", "myfunc")
	if !ok {
		t.Fatal("routine not stored")
	}
	if len(rt.Arguments) != 1 {
		t.Fatalf("arguments = %d", len(rt.Arguments))
	}
	arg := rt.Arguments[0]
	if arg.Name != "arr" {
		t.Errorf("arg name = %q", arg.Name)
	}
	if arg.DataType == nil || arg.DataType.TypeKind != sqlTypeArray {
		t.Fatalf("arg type = %+v", arg.DataType)
	}
	elem := arg.DataType.ArrayElementType
	if elem == nil || elem.TypeKind != sqlTypeStruct {
		t.Fatalf("elem type = %+v", elem)
	}
	if elem.StructType == nil || len(elem.StructType.Fields) != 2 {
		t.Fatalf("struct fields = %+v", elem.StructType)
	}
}

func TestParseSQLType(t *testing.T) {
	typ, n, ok := scanSQLType("INT64")
	if !ok || typ.TypeKind != "INT64" || n != 5 {
		t.Fatalf("INT64 parse failed: %+v %d %v", typ, n, ok)
	}
	typ, _, ok = scanSQLType("ARRAY<STRING>")
	if !ok || typ.TypeKind != "ARRAY" || typ.ArrayElementType.TypeKind != "STRING" {
		t.Fatalf("ARRAY<STRING> = %+v", typ)
	}
}

func TestParseCreateRoutineDDLProcedure(t *testing.T) {
	rt, ok := parseCreateRoutineDDL("p", "d", "CREATE PROCEDURE `proc`(IN x INT64) AS (SELECT 1)")
	if !ok {
		t.Fatal("parse failed")
	}
	if rt.RoutineType != routineTypeProcedure {
		t.Errorf("type = %q", rt.RoutineType)
	}
}
