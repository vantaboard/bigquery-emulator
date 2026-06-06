package query

import (
	"errors"
	"fmt"
	"regexp"
	"strings"
)

// legacyBracketTableRE matches legacy SQL table references of the form
// [project:dataset.table] used by thirdparty Node/Python samples.
var legacyBracketTableRE = regexp.MustCompile(
	`\[([a-zA-Z0-9_-]+):([a-zA-Z0-9_]+)\.([a-zA-Z0-9_]+)\]`)

// legacyBareTableRE matches [dataset.table] when no project is given.
var legacyBareTableRE = regexp.MustCompile(`\[([a-zA-Z0-9_]+)\.([a-zA-Z0-9_]+)\]`)

// PrepareEngineSQL translates limited legacy SQL to GoogleSQL when
// useLegacy is true. The engine only accepts GoogleSQL; callers must
// clear UseLegacySql on the forwarded enginepb.QueryRequest.
func PrepareEngineSQL(useLegacy bool, sql, projectID, defaultDataset string) (string, error) {
	if !useLegacy {
		return sql, nil
	}
	return NormalizeLegacySQL(sql, projectID, defaultDataset)
}

// NormalizeLegacySQL rewrites bracket-style legacy table references to
// GoogleSQL backtick form. Full legacy SQL dialect is not supported.
func NormalizeLegacySQL(sql, projectID, defaultDataset string) (string, error) {
	if strings.TrimSpace(sql) == "" {
		return "", errors.New("legacy SQL query is empty")
	}
	out := legacyBracketTableRE.ReplaceAllStringFunc(sql, func(match string) string {
		parts := legacyBracketTableRE.FindStringSubmatch(match)
		if len(parts) != 4 {
			return match
		}
		return fmt.Sprintf("`%s.%s.%s`", parts[1], parts[2], parts[3])
	})
	if legacyBracketTableRE.MatchString(out) {
		return "", errors.New("legacy SQL contains unsupported table reference syntax")
	}
	if legacyBareTableRE.MatchString(out) {
		project := strings.TrimSpace(projectID)
		if project == "" {
			return "", errors.New("legacy SQL [dataset.table] requires a project context")
		}
		out = legacyBareTableRE.ReplaceAllStringFunc(out, func(match string) string {
			parts := legacyBareTableRE.FindStringSubmatch(match)
			if len(parts) != 3 {
				return match
			}
			return fmt.Sprintf("`%s.%s.%s`", project, parts[1], parts[2])
		})
	}
	_ = defaultDataset // reserved for future bare-table defaulting
	return out, nil
}
