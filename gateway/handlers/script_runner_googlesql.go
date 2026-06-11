package handlers

import "strings"

// scriptNeedsGoogleSQLExecutor mirrors
// backend/engine/coordinator/script_executor_internal.cc so the gateway
// preserves DECLARE syntax for scripts routed through
// googlesql::ScriptExecutor (CREATE CONSTANT lowering breaks IF/WHILE scope).
func scriptNeedsGoogleSQLExecutor(sql string) bool {
	trimmed := strings.TrimSpace(sqlForScriptDetection(sql))
	upper := strings.ToUpper(trimmed)
	if strings.HasPrefix(upper, "IF ") ||
		strings.HasPrefix(upper, "WHILE ") ||
		strings.HasPrefix(upper, "LOOP ") ||
		strings.HasPrefix(upper, "REPEAT") ||
		strings.HasPrefix(upper, "FOR ") ||
		strings.HasPrefix(upper, "RAISE ") ||
		strings.HasPrefix(upper, "EXECUTE IMMEDIATE") ||
		strings.HasPrefix(upper, "EXCEPTION") {
		return true
	}
	upper = strings.ToUpper(sqlForScriptDetection(sql))
	return strings.Contains(upper, " IF ") ||
		strings.Contains(upper, "\nIF ") ||
		strings.Contains(upper, " WHILE ") ||
		strings.Contains(upper, "\nWHILE ") ||
		strings.Contains(upper, " LOOP ") ||
		strings.Contains(upper, "\nLOOP ") ||
		strings.Contains(upper, " REPEAT") ||
		strings.Contains(upper, "\nREPEAT") ||
		(strings.Contains(upper, " FOR ") && strings.Contains(upper, " IN ")) ||
		(strings.Contains(upper, "\nFOR ") && strings.Contains(upper, " IN ")) ||
		strings.Contains(upper, "EXCEPTION") ||
		strings.Contains(upper, " RAISE ") ||
		strings.Contains(upper, "\nRAISE ") ||
		strings.Contains(upper, "EXECUTE IMMEDIATE")
}
