package handlers

import (
	"strings"
	"unicode"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// parseCreateViewDDL extracts the target table reference and AS-query
// from CREATE [OR REPLACE] [MATERIALIZED] VIEW DDL. defaultDatasetID
// applies when the view name is one- or two-part qualified.
func parseCreateViewDDL(projectID, defaultDatasetID, sql string) (bqtypes.Table, bool) {
	rest, materialized, ok := stripCreateViewHeader(sql)
	if !ok {
		return bqtypes.Table{}, false
	}
	name, rest, ok := parseViewQuotedName(rest)
	if !ok {
		return bqtypes.Table{}, false
	}
	pID, dID, tID := splitViewTableName(projectID, defaultDatasetID, name)
	query, ok := parseViewQueryFromRest(rest)
	if !ok {
		return bqtypes.Table{}, false
	}
	t := bqtypes.Table{
		Type: viewTableType,
		View: &bqtypes.ViewDefinition{Query: query},
	}
	if materialized {
		t.Type = materializedViewTableType
		t.View = nil
		t.MaterializedView = &bqtypes.MaterializedViewDefinition{Query: query}
	}
	_ = pID
	_ = dID
	_ = tID
	t.TableReference = bqtypes.TableReference{
		ProjectID: pID,
		DatasetID: dID,
		TableID:   tID,
	}
	return t, true
}

// parseDropViewDDL extracts the target of DROP [MATERIALIZED] VIEW
// [IF EXISTS] DDL. When materializedOnly is true, only materialized-
// view drop forms match; when false, only logical-view drop forms match.
func parseDropViewDDL(
	projectID, defaultDatasetID, sql string,
	materializedOnly bool,
) (pID, dID, tID string, ok bool) {
	rest, materialized, ok := stripDropViewHeader(sql)
	if !ok {
		return "", "", "", false
	}
	if materializedOnly && !materialized {
		return "", "", "", false
	}
	if !materializedOnly && materialized {
		return "", "", "", false
	}
	name, _, ok := parseViewQuotedName(rest)
	if !ok {
		return "", "", "", false
	}
	pID, dID, tID = splitViewTableName(projectID, defaultDatasetID, name)
	return pID, dID, tID, true
}

func stripCreateViewHeader(sql string) (rest string, materialized bool, ok bool) {
	trimmed := strings.TrimSpace(sql)
	upper := strings.ToUpper(trimmed)
	for _, p := range []struct {
		prefix string
		mat    bool
	}{
		{"CREATE OR REPLACE MATERIALIZED VIEW", true},
		{"CREATE MATERIALIZED VIEW", true},
		{"CREATE OR REPLACE VIEW", false},
		{"CREATE VIEW", false},
	} {
		if strings.HasPrefix(upper, p.prefix) {
			return strings.TrimSpace(trimmed[len(p.prefix):]), p.mat, true
		}
	}
	return "", false, false
}

func stripDropViewHeader(sql string) (rest string, materialized bool, ok bool) {
	trimmed := strings.TrimSpace(sql)
	upper := strings.ToUpper(trimmed)
	for _, p := range []struct {
		prefix string
		mat    bool
	}{
		{"DROP MATERIALIZED VIEW IF EXISTS", true},
		{"DROP MATERIALIZED VIEW", true},
		{"DROP VIEW IF EXISTS", false},
		{"DROP VIEW", false},
	} {
		if strings.HasPrefix(upper, p.prefix) {
			return strings.TrimSpace(trimmed[len(p.prefix):]), p.mat, true
		}
	}
	return "", false, false
}

func parseViewQuotedName(s string) (name, rest string, ok bool) {
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
	i := 0
	for i < len(s) && !unicode.IsSpace(rune(s[i])) {
		i++
	}
	if i == 0 {
		return "", "", false
	}
	return s[:i], strings.TrimSpace(s[i:]), true
}

func splitViewTableName(projectID, defaultDatasetID, name string) (project, dataset, table string) {
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

func parseViewQueryFromRest(rest string) (string, bool) {
	rest = skipViewOptionsClause(strings.TrimSpace(rest))
	rest = strings.TrimSpace(rest)
	idx, ok := findTopLevelAS(rest)
	if !ok {
		return "", false
	}
	query := strings.TrimSpace(rest[idx:])
	if query == "" {
		return "", false
	}
	return query, true
}

func skipViewOptionsClause(rest string) string {
	rest = strings.TrimSpace(rest)
	for strings.HasPrefix(strings.ToUpper(rest), "OPTIONS") {
		if !strings.HasPrefix(rest, "(") && !strings.HasPrefix(strings.ToUpper(rest), "OPTIONS(") {
			break
		}
		open := strings.Index(rest, "(")
		if open < 0 {
			break
		}
		inner, tail, ok := scanViewBalanced(rest[open:], '(', ')')
		if !ok {
			break
		}
		_ = inner
		rest = strings.TrimSpace(tail)
	}
	return rest
}

func findTopLevelAS(s string) (after int, ok bool) {
	depth := 0
	angle := 0
	inQuote := byte(0)
	for i := 0; i < len(s); i++ {
		c := s[i]
		if inQuote != 0 {
			if c == '\\' && i+1 < len(s) {
				i++
				continue
			}
			if c == inQuote {
				inQuote = 0
			}
			continue
		}
		switch c {
		case '\'', '"', '`':
			inQuote = c
		case '<':
			angle++
		case '>':
			if angle > 0 {
				angle--
			}
		case '(', '[':
			depth++
		case ')', ']':
			if depth > 0 {
				depth--
			}
		}
		if depth == 0 && angle == 0 && isViewASKeywordAt(s, i) {
			return i + 2, true
		}
	}
	return 0, false
}

func isViewASKeywordAt(s string, i int) bool {
	if i+2 > len(s) || !strings.EqualFold(s[i:i+2], "AS") {
		return false
	}
	if i > 0 && isViewIdentChar(s[i-1]) {
		return false
	}
	if i+2 < len(s) && isViewIdentChar(s[i+2]) {
		return false
	}
	return true
}

func isViewIdentChar(b byte) bool {
	return unicode.IsLetter(rune(b)) || unicode.IsDigit(rune(b)) || b == '_'
}

func scanViewBalanced(s string, open, close byte) (inner, rest string, ok bool) {
	if len(s) == 0 || s[0] != open {
		return "", "", false
	}
	depth := 0
	inQuote := byte(0)
	for i := 0; i < len(s); i++ {
		c := s[i]
		if inQuote != 0 {
			if c == '\\' && i+1 < len(s) {
				i++
				continue
			}
			if c == inQuote {
				inQuote = 0
			}
			continue
		}
		switch c {
		case '\'', '"', '`':
			inQuote = c
		case open:
			depth++
		case close:
			depth--
			if depth == 0 {
				return s[1:i], strings.TrimSpace(s[i+1:]), true
			}
		}
	}
	return "", "", false
}
