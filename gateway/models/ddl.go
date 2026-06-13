package models

import (
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

var modelTypeRE = regexp.MustCompile(`(?i)model_type\s*=\s*'([^']+)'`)

// RegisterFromDDL parses CREATE MODEL DDL and upserts metadata into store.
func RegisterFromDDL(store *Store, projectID, defaultDatasetID, sql string) *bqtypes.ModelReference {
	m, ok := parseCreateModelDDL(projectID, defaultDatasetID, sql)
	if !ok {
		return nil
	}
	store.Upsert(m)
	ref := m.ModelReference
	return &ref
}

func parseCreateModelDDL(projectID, defaultDatasetID, sql string) (bqtypes.Model, bool) {
	trimmed := strings.TrimSpace(sql)
	upper := strings.ToUpper(trimmed)
	switch {
	case strings.HasPrefix(upper, "CREATE OR REPLACE MODEL"):
		trimmed = strings.TrimSpace(trimmed[len("CREATE OR REPLACE MODEL"):])
	case strings.HasPrefix(upper, "CREATE MODEL IF NOT EXISTS"):
		trimmed = strings.TrimSpace(trimmed[len("CREATE MODEL IF NOT EXISTS"):])
	case strings.HasPrefix(upper, "CREATE MODEL"):
		trimmed = strings.TrimSpace(trimmed[len("CREATE MODEL"):])
	default:
		return bqtypes.Model{}, false
	}
	name, _, ok := parseQuotedOrBareName(trimmed)
	if !ok {
		return bqtypes.Model{}, false
	}
	pID, dID, mID := splitModelName(projectID, defaultDatasetID, name)
	modelType := "LINEAR_REG"
	if m := modelTypeRE.FindStringSubmatch(sql); len(m) == 2 {
		modelType = strings.ToUpper(strings.TrimSpace(m[1]))
	}
	now := nowMillis()
	return bqtypes.Model{
		ModelReference: bqtypes.ModelReference{
			ProjectID: pID,
			DatasetID: dID,
			ModelID:   mID,
		},
		ModelType:        modelType,
		CreationTime:     now,
		LastModifiedTime: now,
		Etag:             MintEtag(),
	}, true
}

func splitModelName(projectID, defaultDatasetID, name string) (pID, dID, mID string) {
	parts := strings.Split(name, ".")
	switch len(parts) {
	case 3:
		return parts[0], parts[1], parts[2]
	case 2:
		return projectID, parts[0], parts[1]
	default:
		return projectID, defaultDatasetID, strings.Trim(parts[0], "`")
	}
}

func parseQuotedOrBareName(rest string) (name, tail string, ok bool) {
	rest = strings.TrimSpace(rest)
	if rest == "" {
		return "", "", false
	}
	if rest[0] == '`' {
		end := strings.Index(rest[1:], "`")
		if end < 0 {
			return "", "", false
		}
		name = rest[1 : end+1]
		return name, strings.TrimSpace(rest[end+2:]), true
	}
	idx := strings.IndexAny(rest, " \t\n\r(")
	if idx < 0 {
		return rest, "", true
	}
	return rest[:idx], strings.TrimSpace(rest[idx:]), true
}

func nowMillis() string {
	return strconv.FormatInt(time.Now().UTC().UnixMilli(), 10)
}
