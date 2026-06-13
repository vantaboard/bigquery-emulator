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

// legacyBracketDecoratorRE matches legacy snapshot decorators
// [project:dataset.table@epoch] or [project:dataset.table@-offset].
var legacyBracketDecoratorRE = regexp.MustCompile(
	`\[([a-zA-Z0-9_-]+):([a-zA-Z0-9_]+)\.([a-zA-Z0-9_]+)@(-?[0-9]+)\]`)

// legacyBareTableRE matches [dataset.table] when no project is given.
var legacyBareTableRE = regexp.MustCompile(`\[([a-zA-Z0-9_]+)\.([a-zA-Z0-9_]+)\]`)

// legacyBareDecoratorRE matches [dataset.table@epoch] without a project.
var legacyBareDecoratorRE = regexp.MustCompile(
	`\[([a-zA-Z0-9_]+)\.([a-zA-Z0-9_]+)@(-?[0-9]+)\]`)

// PrepareEngineSQL translates limited legacy SQL to GoogleSQL when
// useLegacy is true. The engine only accepts GoogleSQL; callers must
// clear UseLegacySql on the forwarded enginepb.QueryRequest.
func PrepareEngineSQL(useLegacy bool, sql, projectID, defaultDataset string) (string, error) {
	if useLegacy {
		normalized, err := NormalizeLegacySQL(sql, projectID, defaultDataset)
		if err != nil {
			return "", err
		}
		return LowerTableDecorators(normalized)
	}
	return LowerTableDecorators(sql)
}

// NormalizeLegacySQL rewrites bracket-style legacy table references to
// GoogleSQL backtick form. Full legacy SQL dialect is not supported.
func NormalizeLegacySQL(sql, projectID, defaultDataset string) (string, error) {
	if strings.TrimSpace(sql) == "" {
		return "", errors.New("legacy SQL query is empty")
	}
	if hasDecoratorConflict(sql) {
		return "", errors.New(
			"Cannot use table decorator with FOR SYSTEM_TIME AS OF")
	}
	out := legacyBracketDecoratorRE.ReplaceAllStringFunc(sql, func(match string) string {
		parts := legacyBracketDecoratorRE.FindStringSubmatch(match)
		if len(parts) != 5 {
			return match
		}
		epoch, err := resolveDecoratorEpoch(parts[4])
		if err != nil {
			return match
		}
		return fmt.Sprintf("`%s.%s.%s` FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(%d)",
			parts[1], parts[2], parts[3], epoch)
	})
	out = legacyBracketTableRE.ReplaceAllStringFunc(out, func(match string) string {
		parts := legacyBracketTableRE.FindStringSubmatch(match)
		if len(parts) != 4 {
			return match
		}
		return fmt.Sprintf("`%s.%s.%s`", parts[1], parts[2], parts[3])
	})
	if legacyBracketTableRE.MatchString(out) {
		return "", errors.New("legacy SQL contains unsupported table reference syntax")
	}
	if legacyBareDecoratorRE.MatchString(out) {
		project := strings.TrimSpace(projectID)
		if project == "" {
			return "", errors.New("legacy SQL [dataset.table@epoch] requires a project context")
		}
		out = legacyBareDecoratorRE.ReplaceAllStringFunc(out, func(match string) string {
			parts := legacyBareDecoratorRE.FindStringSubmatch(match)
			if len(parts) != 4 {
				return match
			}
			epoch, err := resolveDecoratorEpoch(parts[3])
			if err != nil {
				return match
			}
			return fmt.Sprintf("`%s.%s.%s` FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(%d)",
				project, parts[1], parts[2], epoch)
		})
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
