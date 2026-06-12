package runner

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"maps"
	"sort"
	"strconv"
)

// NormalizeRows returns a deterministic JSON encoding for hashing.
func NormalizeRows(rows []map[string]string) ([]byte, error) {
	cp := make([]map[string]string, len(rows))
	for i, r := range rows {
		cp[i] = make(map[string]string, len(r))
		maps.Copy(cp[i], r)
	}
	sort.Slice(cp, func(i, j int) bool {
		a, _ := json.Marshal(cp[i])
		b, _ := json.Marshal(cp[j])
		return string(a) < string(b)
	})
	return json.Marshal(cp)
}

// HashRows returns SHA-256 hex digest of normalized rows.
func HashRows(rows []map[string]string) (string, error) {
	norm, err := NormalizeRows(rows)
	if err != nil {
		return "", err
	}
	sum := sha256.Sum256(norm)
	return hex.EncodeToString(sum[:]), nil
}

// RowsFromBQ converts REST rows to map form for hashing.
func RowsFromBQ(rows []map[string]any) []map[string]string {
	out := make([]map[string]string, 0, len(rows))
	for _, row := range rows {
		m := make(map[string]string, len(row))
		for k, v := range row {
			m[k] = cellToString(v)
		}
		out = append(out, m)
	}
	return out
}

func cellToString(v any) string {
	switch t := v.(type) {
	case nil:
		return ""
	case string:
		return t
	case float64:
		return jsonNumber(t)
	case bool:
		if t {
			return "true"
		}
		return "false"
	default:
		b, _ := json.Marshal(t)
		return string(b)
	}
}

func jsonNumber(f float64) string {
	if f == float64(int64(f)) {
		return strconv.FormatInt(int64(f), 10)
	}
	return strconv.FormatFloat(f, 'f', -1, 64)
}
