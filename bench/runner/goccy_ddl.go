package runner

import (
	"context"
	"fmt"
	"regexp"
	"strings"
)

// createOrReplaceAs matches CREATE OR REPLACE TABLE/VIEW ... AS (CTAS / view body).
// Bench DDL cases use this shape; goccy 0.8.1 treats a second CREATE as duplicate
// rather than honoring OR REPLACE, so we DROP IF EXISTS then CREATE before timing.
var createOrReplaceAsRE = regexp.MustCompile(
	`(?is)^CREATE\s+OR\s+REPLACE\s+(TABLE|VIEW)\s+(\S+)\s+AS\s+`,
)

// rewriteGoccyCreateOrReplace splits CREATE OR REPLACE TABLE/VIEW ... AS into an
// idempotent DROP + CREATE pair for goccy. ok is false when sql is not that shape.
func rewriteGoccyCreateOrReplace(sql string) (dropSQL, createSQL string, ok bool) {
	trimmed := strings.TrimSpace(sql)
	m := createOrReplaceAsRE.FindStringSubmatch(trimmed)
	if m == nil {
		return "", trimmed, false
	}
	kind := strings.ToUpper(m[1])
	object := m[2]
	dropSQL = fmt.Sprintf("DROP %s IF EXISTS %s", kind, object)
	createSQL = createOrReplaceAsRE.ReplaceAllString(trimmed, "CREATE "+kind+" "+object+" AS ")
	return dropSQL, createSQL, true
}

// prepareGoccyDDLQuery runs an untimed DROP IF EXISTS when sql is CREATE OR REPLACE
// TABLE/VIEW ... AS, then returns equivalent CREATE ... AS for the timed iteration.
func prepareGoccyDDLQuery(ctx context.Context, client *RESTClient, sql string) (string, error) {
	dropSQL, createSQL, rewrite := rewriteGoccyCreateOrReplace(sql)
	if !rewrite {
		return strings.TrimSpace(sql), nil
	}
	status, body, err := client.PostQuery(ctx, dropSQL)
	if err != nil {
		return "", fmt.Errorf("goccy ddl preamble drop: %w", err)
	}
	if status < 200 || status >= 300 {
		return "", fmt.Errorf("goccy ddl preamble drop -> HTTP %d: %s", status, snippet(body))
	}
	return createSQL, nil
}
