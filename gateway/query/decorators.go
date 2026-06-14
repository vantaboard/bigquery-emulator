package query

import (
	"errors"
	"fmt"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// backtickDecoratedRE matches `project.dataset.table@123` or `dataset.table@-3600000`.
var backtickDecoratedRE = regexp.MustCompile("`([^`]+)@(-?[0-9]+)`")

// LowerTableDecorators rewrites BigQuery table time decorators embedded in
// backtick table paths to FOR SYSTEM_TIME AS OF, matching the engine's
// historical read path. Relative offsets (@-3600000) are resolved against
// the current UTC clock at rewrite time.
func LowerTableDecorators(sql string) (string, error) {
	trim := strings.TrimSpace(sql)
	if trim == "" {
		return sql, nil
	}
	if hasDecoratorConflict(trim) {
		return "", errors.New(
			"cannot use table decorator with FOR SYSTEM_TIME AS OF")
	}
	return backtickDecoratedRE.ReplaceAllStringFunc(sql, func(match string) string {
		parts := backtickDecoratedRE.FindStringSubmatch(match)
		if len(parts) != 3 {
			return match
		}
		base := parts[1]
		raw := parts[2]
		epoch, err := resolveDecoratorEpoch(raw)
		if err != nil {
			return match
		}
		return fmt.Sprintf("`%s` FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(%d)",
			base, epoch)
	}), nil
}

func hasDecoratorConflict(sql string) bool {
	upper := strings.ToUpper(sql)
	if !strings.Contains(upper, "FOR SYSTEM_TIME AS OF") {
		return false
	}
	return backtickDecoratedRE.MatchString(sql) ||
		legacyBracketDecoratorRE.MatchString(sql)
}

func resolveDecoratorEpoch(raw string) (int64, error) {
	if strings.HasPrefix(raw, "-") {
		offset, err := strconv.ParseInt(raw, 10, 64)
		if err != nil {
			return 0, err
		}
		return time.Now().UTC().UnixMilli() + offset, nil
	}
	return strconv.ParseInt(raw, 10, 64)
}
