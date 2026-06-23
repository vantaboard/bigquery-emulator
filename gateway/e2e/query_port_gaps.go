//go:build integration

package e2e

import "testing"

// queryPortKnownGaps documents intentional emulator parity gaps for ported
// query port subtests. Prefer fixing engine/gateway when a conformance fixture
// exists; otherwise skip with a link to SHAPE_TRACKER or disposition.
var queryPortKnownGaps = map[string]string{
	"array_concat_agg with format":                "ARRAY_CONCAT_AGG + FORMAT parity not yet implemented (invalid INT64→ARRAY cast)",
	"safe cast for invalid cast":                  "SAFE_CAST invalid cast should yield NULL; engine rejects with semantic error",
	"farm_fingerprint":                            "FARM_FINGERPRINT hash algorithm differs from BigQuery FarmHash64",
	"regexp_contains":                             "UNNEST(array) row order not preserved in DuckDB fast path",
	"regexp_contains2":                            "UNNEST(array) row order not preserved in DuckDB fast path",
	"regexp_extract_all null":                     "REGEXP_EXTRACT_ALL NULL input: emulator returns [] not NULL",
	"rpad string":                                 "RPAD NULL fill: emulator renders literal 'NULL' string",
	"rpad string with pattern":                    "RPAD NULL fill: emulator renders literal 'NULL' string",
	"split":                                       "SPLIT empty trailing field: emulator returns NULL not []",
	"split null delimiter":                        "SPLIT NULL delimiter: emulator returns NULL not []",
	"datetime_trunc isoyear":                      "DATETIME_TRUNC ISOYEAR: DATE literal parse gap in engine",
	"extract date":                                "EXTRACT(ISOWEEK/ISOYEAR) from DATE parity gap",
	"datetime_diff with isoweek":                  "DATETIME_DIFF ISOWEEK: DATE literal parse gap in engine",
	"parse timestamp with %Y-%m-%d %H:%M:%E*S%Ez": "PARSE_TIMESTAMP subsecond wire omits fractional seconds",
	"parse_bignumeric":                            "PARSE_BIGNUMERIC scientific-notation padding differs from BigQuery",
	"create table as select with column list":     "CTAS with explicit column list not wired in DuckDB executor",
}

func queryPortSkipIfKnownGap(t *testing.T, name string) {
	t.Helper()
	if reason, ok := queryPortKnownGaps[name]; ok {
		t.Skip(reason)
	}
}
