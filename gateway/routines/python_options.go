package routines

import (
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func parsePythonOptionsFromDDL(sql string) *bqtypes.PythonOptions {
	upper := strings.ToUpper(sql)
	pos := strings.Index(upper, "OPTIONS")
	if pos < 0 {
		return nil
	}
	rest := strings.TrimSpace(sql[pos+len("OPTIONS"):])
	if !strings.HasPrefix(rest, "(") {
		return nil
	}
	inner, _, ok := scanBalanced(rest, '(', ')')
	if !ok {
		return nil
	}
	opts := &bqtypes.PythonOptions{}
	for part := range strings.SplitSeq(inner, ",") {
		part = strings.TrimSpace(part)
		if part == "" {
			continue
		}
		key, value, found := strings.Cut(part, "=")
		if !found {
			continue
		}
		key = strings.TrimSpace(strings.Trim(key, `"'`))
		value = strings.TrimSpace(value)
		switch strings.ToUpper(key) {
		case "ENTRY_POINT":
			opts.EntryPoint = parseOptionStringLiteral(value)
		case "PACKAGES":
			opts.Packages = parseOptionStringArray(value)
		}
	}
	if opts.EntryPoint == "" && len(opts.Packages) == 0 {
		return nil
	}
	return opts
}

func parseOptionStringLiteral(value string) string {
	value = strings.TrimSpace(value)
	if len(value) >= 2 {
		quote := value[0]
		if (quote == '\'' || quote == '"') && value[len(value)-1] == quote {
			return value[1 : len(value)-1]
		}
	}
	return strings.Trim(value, `"'`)
}

func parseOptionStringArray(value string) []string {
	value = strings.TrimSpace(value)
	if !strings.HasPrefix(value, "[") {
		return nil
	}
	inner, _, ok := scanBalanced(value, '[', ']')
	if !ok {
		return nil
	}
	var out []string
	for part := range strings.SplitSeq(inner, ",") {
		part = strings.TrimSpace(part)
		if part == "" {
			continue
		}
		out = append(out, parseOptionStringLiteral(part))
	}
	return out
}
