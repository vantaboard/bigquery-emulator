package handlers

import (
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// expandQueryParamsInSQL applies gateway-side SQL rewrites for query parameters
// the DuckDB transpiler cannot lower yet (ARRAY IN UNNEST).
func expandQueryParamsInSQL(sql string, params []bqtypes.QueryParameter) string {
	sql = expandArrayParamsInSQL(sql, params)
	return expandPositionalArrayParamsInSQL(sql, params)
}

// expandArrayParamsInSQL rewrites `IN UNNEST(@name)` filters into `IN (...)`
// literal lists when the caller supplied a named ARRAY query parameter.
// The DuckDB transpiler does not yet lower IN UNNEST(array_param) shapes;
// expanding at the gateway preserves analyzer binding for scalar params while
// unblocking thirdparty array-parameter samples.
func expandArrayParamsInSQL(sql string, params []bqtypes.QueryParameter) string {
	out := sql
	for _, p := range params {
		if p.Name == "" || p.ParameterType == nil ||
			strings.ToUpper(p.ParameterType.Type) != sqlTypeARRAY {
			continue
		}
		if p.ParameterValue == nil || len(p.ParameterValue.ArrayValues) == 0 {
			continue
		}
		quoted := make([]string, 0, len(p.ParameterValue.ArrayValues))
		for _, av := range p.ParameterValue.ArrayValues {
			if av.Value == "" {
				continue
			}
			quoted = append(quoted, fmt.Sprintf("'%s'",
				strings.ReplaceAll(av.Value, "'", "''")))
		}
		if len(quoted) == 0 {
			continue
		}
		list := strings.Join(quoted, ", ")
		name := p.Name
		out = strings.ReplaceAll(out, "NOT IN UNNEST(@"+name+")", "NOT IN ("+list+")")
		out = strings.ReplaceAll(out, "NOT IN UNNEST(`"+name+"`)", "NOT IN ("+list+")")
		out = strings.ReplaceAll(out, "IN UNNEST(@"+name+")", "IN ("+list+")")
		out = strings.ReplaceAll(out, "IN UNNEST(`"+name+"`)", "IN ("+list+")")
	}
	return out
}

// stripExpandedArrayParams removes ARRAY parameters that expandQueryParamsInSQL
// inlined via IN/NOT IN UNNEST so the engine is not asked to bind them.
func stripExpandedArrayParams(
	originalSQL, expandedSQL string,
	params []bqtypes.QueryParameter,
) []bqtypes.QueryParameter {
	if len(params) == 0 {
		return params
	}
	out := make([]bqtypes.QueryParameter, 0, len(params))
	remaining := originalSQL
	for _, p := range params {
		if p.ParameterType == nil ||
			strings.ToUpper(strings.TrimSpace(p.ParameterType.Type)) != sqlTypeARRAY {
			out = append(out, p)
			continue
		}
		if p.Name != "" {
			if namedArrayParamWasExpanded(originalSQL, expandedSQL, p.Name) {
				continue
			}
			out = append(out, p)
			continue
		}
		if !strings.Contains(remaining, "IN UNNEST(?)") {
			out = append(out, p)
			continue
		}
		if p.ParameterValue == nil || len(p.ParameterValue.ArrayValues) == 0 {
			out = append(out, p)
			continue
		}
		remaining = strings.Replace(remaining, "IN UNNEST(?)", "IN (__expanded__)", 1)
	}
	return out
}

func namedArrayParamWasExpanded(originalSQL, expandedSQL, name string) bool {
	for _, pattern := range []string{
		"IN UNNEST(@" + name + ")",
		"NOT IN UNNEST(@" + name + ")",
		"IN UNNEST(`" + name + "`)",
		"NOT IN UNNEST(`" + name + "`)",
	} {
		if strings.Contains(originalSQL, pattern) && !strings.Contains(expandedSQL, pattern) {
			return true
		}
	}
	return false
}

// stripExpandedPositionalArrayParams removes positional ARRAY parameters
// that expandQueryParamsInSQL inlined via IN UNNEST(?) so engine binding
// indices stay aligned with the remaining ? placeholders.
func stripExpandedPositionalArrayParams(sql string, params []bqtypes.QueryParameter) []bqtypes.QueryParameter {
	return stripExpandedArrayParams(sql, expandQueryParamsInSQL(sql, params), params)
}

func expandPositionalArrayParamsInSQL(sql string, params []bqtypes.QueryParameter) string {
	out := sql
	for _, p := range params {
		if p.Name != "" || p.ParameterType == nil {
			continue
		}
		if strings.ToUpper(strings.TrimSpace(p.ParameterType.Type)) != "ARRAY" {
			continue
		}
		if !strings.Contains(out, "IN UNNEST(?)") {
			continue
		}
		if p.ParameterValue == nil || len(p.ParameterValue.ArrayValues) == 0 {
			continue
		}
		quoted := make([]string, 0, len(p.ParameterValue.ArrayValues))
		for _, av := range p.ParameterValue.ArrayValues {
			if av.Value == "" {
				continue
			}
			quoted = append(quoted, fmt.Sprintf("'%s'",
				strings.ReplaceAll(av.Value, "'", "''")))
		}
		if len(quoted) == 0 {
			continue
		}
		out = strings.Replace(out, "IN UNNEST(?)",
			"IN ("+strings.Join(quoted, ", ")+")", 1)
	}
	return out
}
