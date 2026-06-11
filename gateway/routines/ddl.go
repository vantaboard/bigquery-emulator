package routines

import (
	"strings"
	"unicode"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const (
	routineTypeScalarFunction = "SCALAR_FUNCTION"
	routineTypeTableFunction  = "TABLE_VALUED_FUNCTION"
	routineTypeProcedure      = "PROCEDURE"
	routineLanguageSQL        = "SQL"
	sqlTypeArray              = "ARRAY"
	sqlTypeStruct             = "STRUCT"
	sqlTypeAnyType            = "ANY TYPE"
)

// RegisterFromDDL parses CREATE FUNCTION / CREATE PROCEDURE DDL and
// upserts the routine into store. Returns the target reference when
// registration succeeds.
func RegisterFromDDL(store *Store, projectID, defaultDatasetID, sql string) *bqtypes.RoutineReference {
	rt, ok := parseCreateRoutineDDL(projectID, defaultDatasetID, sql)
	if !ok {
		return nil
	}
	store.Upsert(rt)
	ref := rt.RoutineReference
	return &ref
}

// ParseCreateRoutineDDL parses CREATE FUNCTION / PROCEDURE DDL into a
// Routine snapshot (used by REST/catalog round-trip helpers).
func ParseCreateRoutineDDL(projectID, defaultDatasetID, sql string) (bqtypes.Routine, bool) {
	return parseCreateRoutineDDL(projectID, defaultDatasetID, sql)
}

func parseCreateRoutineDDL(projectID, defaultDatasetID, sql string) (bqtypes.Routine, bool) {
	rest, routineType, ok := stripCreateRoutineHeader(sql)
	if !ok {
		return bqtypes.Routine{}, false
	}
	name, rest, ok := parseQuotedName(rest)
	if !ok {
		return bqtypes.Routine{}, false
	}
	pID, dID, rID := splitRoutineName(projectID, defaultDatasetID, name)
	args, returnType, body, ok := parseRoutineSignature(rest)
	if !ok {
		return bqtypes.Routine{}, false
	}
	now := nowMillis()
	return bqtypes.Routine{
		Etag: MintEtag(),
		RoutineReference: bqtypes.RoutineReference{
			ProjectID: pID,
			DatasetID: dID,
			RoutineID: rID,
		},
		RoutineType:      bqtypes.RoutineType(routineType),
		Language:         routineLanguageSQL,
		Arguments:        args,
		ReturnType:       returnType,
		DefinitionBody:   body,
		CreationTime:     now,
		LastModifiedTime: now,
	}, true
}

func stripCreateRoutineHeader(sql string) (rest, routineType string, ok bool) {
	trimmed := strings.TrimSpace(sql)
	upper := strings.ToUpper(trimmed)
	switch {
	case strings.HasPrefix(upper, "CREATE OR REPLACE FUNCTION"),
		strings.HasPrefix(upper, "CREATE FUNCTION"):
		routineType = routineTypeScalarFunction
	case strings.HasPrefix(upper, "CREATE OR REPLACE TABLE FUNCTION"),
		strings.HasPrefix(upper, "CREATE TABLE FUNCTION"):
		routineType = routineTypeTableFunction
	case strings.HasPrefix(upper, "CREATE OR REPLACE PROCEDURE"),
		strings.HasPrefix(upper, "CREATE PROCEDURE"):
		routineType = routineTypeProcedure
	default:
		return "", "", false
	}
	rest = trimmed
	for _, prefix := range []string{
		"CREATE OR REPLACE TABLE FUNCTION",
		"CREATE TABLE FUNCTION",
		"CREATE OR REPLACE FUNCTION",
		"CREATE FUNCTION",
		"CREATE OR REPLACE PROCEDURE",
		"CREATE PROCEDURE",
	} {
		if len(rest) >= len(prefix) && strings.EqualFold(rest[:len(prefix)], prefix) {
			return strings.TrimSpace(rest[len(prefix):]), routineType, true
		}
	}
	return "", "", false
}

func parseRoutineSignature(rest string) (args []bqtypes.RoutineArgument,
	returnType *bqtypes.StandardSqlDataType, body string, ok bool,
) {
	if !strings.HasPrefix(rest, "(") {
		return nil, nil, "", false
	}
	argsRaw, rest, ok := scanBalanced(rest, '(', ')')
	if !ok {
		return nil, nil, "", false
	}
	args, _ = parseArgumentList(strings.TrimSpace(argsRaw))
	rest = strings.TrimSpace(rest)
	if strings.HasPrefix(strings.ToUpper(rest), "RETURNS") {
		rest = strings.TrimSpace(rest[len("RETURNS"):])
		typeRaw, consumed, typed := scanSQLType(rest)
		if !typed {
			return nil, nil, "", false
		}
		returnType = typeRaw
		rest = strings.TrimSpace(rest[consumed:])
	}
	rest = strings.TrimSpace(rest)
	if !strings.HasPrefix(strings.ToUpper(rest), "AS") {
		return nil, nil, "", false
	}
	rest = strings.TrimSpace(rest[len("AS"):])
	body, ok = parseDefinitionBody(rest)
	if !ok || body == "" {
		return nil, nil, "", false
	}
	return args, returnType, body, true
}

func parseQuotedName(s string) (name, rest string, ok bool) {
	s = strings.TrimSpace(s)
	if len(s) == 0 {
		return "", "", false
	}
	if s[0] == '`' {
		end := strings.Index(s[1:], "`")
		if end < 0 {
			return "", "", false
		}
		return s[1 : end+1], strings.TrimSpace(s[end+2:]), true
	}
	// Unquoted identifier: read until '(' or whitespace boundary.
	i := 0
	for i < len(s) && !unicode.IsSpace(rune(s[i])) && s[i] != '(' {
		i++
	}
	if i == 0 {
		return "", "", false
	}
	return s[:i], strings.TrimSpace(s[i:]), true
}

func splitRoutineName(projectID, defaultDatasetID, name string) (project, dataset, routine string) {
	parts := strings.Split(name, ".")
	switch len(parts) {
	case 1:
		return projectID, defaultDatasetID, parts[0]
	case 2:
		return projectID, parts[0], parts[1]
	default:
		return parts[0], parts[1], parts[len(parts)-1]
	}
}

func scanBalanced(s string, open, close byte) (inner, rest string, ok bool) {
	if len(s) == 0 || s[0] != open {
		return "", "", false
	}
	depth := 0
	angle := 0
	for i := 0; i < len(s); i++ {
		switch s[i] {
		case '<':
			angle++
		case '>':
			if angle > 0 {
				angle--
			}
		case open:
			if angle == 0 {
				depth++
			}
		case close:
			if angle == 0 {
				depth--
				if depth == 0 {
					return s[1:i], strings.TrimSpace(s[i+1:]), true
				}
			}
		}
	}
	return "", "", false
}

func parseArgumentList(raw string) ([]bqtypes.RoutineArgument, bool) {
	if raw == "" {
		return nil, true
	}
	var out []bqtypes.RoutineArgument
	for len(raw) > 0 {
		raw = strings.TrimSpace(raw)
		if raw == "" {
			break
		}
		nameEnd := 0
		for nameEnd < len(raw) && (unicode.IsLetter(rune(raw[nameEnd])) ||
			unicode.IsDigit(rune(raw[nameEnd])) || raw[nameEnd] == '_') {
			nameEnd++
		}
		if nameEnd == 0 {
			return nil, false
		}
		name := raw[:nameEnd]
		raw = strings.TrimSpace(raw[nameEnd:])
		typ, consumed, ok := scanSQLType(raw)
		if !ok {
			return nil, false
		}
		out = append(out, bqtypes.RoutineArgument{
			Name:     name,
			DataType: typ,
		})
		raw = strings.TrimSpace(raw[consumed:])
		if raw == "" {
			break
		}
		if raw[0] != ',' {
			return nil, false
		}
		raw = strings.TrimSpace(raw[1:])
	}
	return out, true
}

func scanSQLType(s string) (*bqtypes.StandardSqlDataType, int, bool) {
	s = strings.TrimSpace(s)
	if s == "" {
		return nil, 0, false
	}
	upper := strings.ToUpper(s)
	switch {
	case strings.HasPrefix(upper, "ANY TYPE"):
		return &bqtypes.StandardSqlDataType{
			TypeKind: bqtypes.SQLTypeKind("ANY TYPE"),
		}, len("ANY TYPE"), true
	case strings.HasPrefix(upper, "ARRAY<"):
		inner, consumed, ok := scanAngleInner(s[len("ARRAY<"):])
		if !ok {
			return nil, 0, false
		}
		elem, _, ok := scanSQLType(inner)
		if !ok {
			return nil, 0, false
		}
		total := len("ARRAY<") + consumed
		return &bqtypes.StandardSqlDataType{
			TypeKind:         bqtypes.SQLTypeKind(sqlTypeArray),
			ArrayElementType: elem,
		}, total, true
	case strings.HasPrefix(upper, "STRUCT<"):
		inner, consumed, ok := scanAngleInner(s[len("STRUCT<"):])
		if !ok {
			return nil, 0, false
		}
		fields, ok := parseStructFields(inner)
		if !ok {
			return nil, 0, false
		}
		total := len("STRUCT<") + consumed
		return &bqtypes.StandardSqlDataType{
			TypeKind: bqtypes.SQLTypeKind(sqlTypeStruct),
			StructType: &bqtypes.StandardSqlStructType{
				Fields: fields,
			},
		}, total, true
	default:
		end := 0
		for end < len(s) && (unicode.IsLetter(rune(s[end])) ||
			unicode.IsDigit(rune(s[end])) || s[end] == '_') {
			end++
		}
		if end == 0 {
			return nil, 0, false
		}
		return &bqtypes.StandardSqlDataType{
			TypeKind: bqtypes.SQLTypeKind(strings.ToUpper(s[:end])),
		}, end, true
	}
}

func scanAngleInner(s string) (inner string, consumed int, ok bool) {
	depth := 1
	for i := 0; i < len(s); i++ {
		switch s[i] {
		case '<':
			depth++
		case '>':
			depth--
			if depth == 0 {
				return s[:i], i + 1, true
			}
		}
	}
	return "", 0, false
}

func parseStructFields(raw string) ([]bqtypes.StandardSqlField, bool) {
	raw = strings.TrimSpace(raw)
	if raw == "" {
		return nil, true
	}
	var out []bqtypes.StandardSqlField
	for len(raw) > 0 {
		raw = strings.TrimSpace(raw)
		nameEnd := 0
		for nameEnd < len(raw) && (unicode.IsLetter(rune(raw[nameEnd])) ||
			unicode.IsDigit(rune(raw[nameEnd])) || raw[nameEnd] == '_') {
			nameEnd++
		}
		if nameEnd == 0 {
			return nil, false
		}
		name := raw[:nameEnd]
		raw = strings.TrimSpace(raw[nameEnd:])
		typ, consumed, ok := scanSQLType(raw)
		if !ok {
			return nil, false
		}
		out = append(out, bqtypes.StandardSqlField{
			Name: name,
			Type: *typ,
		})
		raw = strings.TrimSpace(raw[consumed:])
		if raw == "" {
			break
		}
		if raw[0] != ',' {
			return nil, false
		}
		raw = strings.TrimSpace(raw[1:])
	}
	return out, true
}

func parseDefinitionBody(s string) (string, bool) {
	s = strings.TrimSpace(s)
	if len(s) == 0 {
		return "", false
	}
	if s[0] == '(' {
		inner, _, ok := scanBalanced(s, '(', ')')
		return strings.TrimSpace(inner), ok
	}
	// Language-specific quoted bodies (JavaScript UDFs) — take the
	// first quoted span verbatim.
	if s[0] == '\'' || s[0] == '"' {
		quote := s[0]
		var b strings.Builder
		escaped := false
		for i := 1; i < len(s); i++ {
			c := s[i]
			if escaped {
				b.WriteByte(c)
				escaped = false
				continue
			}
			if c == '\\' {
				escaped = true
				continue
			}
			if c == quote {
				return b.String(), true
			}
			b.WriteByte(c)
		}
		return "", false
	}
	return s, true
}
