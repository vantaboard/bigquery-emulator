package handlers

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

var parseCreateViewDDLCases = []struct {
	name    string
	sql     string
	project string
	dataset string
	table   string
	query   string
	typ     string
}{
	{
		name:    "three_part_quoted",
		sql:     "CREATE OR REPLACE VIEW `p.d.v` AS " + testViewSQL,
		project: "p",
		dataset: "d",
		table:   "v",
		query:   testViewSQL,
		typ:     viewTableType,
	},
	{
		name:    "two_part_default_project",
		sql:     "CREATE VIEW ds.my_view AS " + testViewSQL,
		project: testProjectID,
		dataset: "ds",
		table:   "my_view",
		query:   testViewSQL,
		typ:     viewTableType,
	},
	{
		name:    "one_part_default_dataset",
		sql:     "CREATE VIEW my_view AS " + testViewSQL,
		project: testProjectID,
		dataset: testDefaultDatasetIDMain,
		table:   "my_view",
		query:   testViewSQL,
		typ:     viewTableType,
	},
	{
		name:    "materialized_view",
		sql:     "CREATE MATERIALIZED VIEW `p.d.mv` AS " + testViewSQL,
		project: "p",
		dataset: "d",
		table:   "mv",
		query:   testViewSQL,
		typ:     materializedViewTableType,
	},
	{
		name:    "query_with_nested_as",
		sql:     "CREATE VIEW v AS SELECT 1 AS x, 2 AS y",
		project: testProjectID,
		dataset: testDefaultDatasetIDMain,
		table:   "v",
		query:   "SELECT 1 AS x, 2 AS y",
		typ:     viewTableType,
	},
}

func TestParseCreateViewDDL(t *testing.T) {
	t.Parallel()
	for _, tc := range parseCreateViewDDLCases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			assertParseCreateViewDDL(t, tc)
		})
	}
}

func assertParseCreateViewDDL(t *testing.T, tc struct {
	name, sql, project, dataset, table, query, typ string
},
) {
	t.Helper()
	got, ok := parseCreateViewDDL(tc.project, testDefaultDatasetIDMain, tc.sql)
	if !ok {
		t.Fatal("parseCreateViewDDL returned false")
	}
	ref := got.TableReference
	if ref.ProjectID != tc.project || ref.DatasetID != tc.dataset || ref.TableID != tc.table {
		t.Errorf("ref = %+v, want %s.%s.%s", ref, tc.project, tc.dataset, tc.table)
	}
	if got.Type != tc.typ {
		t.Errorf("type = %q, want %q", got.Type, tc.typ)
	}
	if query := viewDDLQueryText(got); query != tc.query {
		t.Errorf("query = %q, want %q", query, tc.query)
	}
}

func viewDDLQueryText(t bqtypes.Table) string {
	if t.View != nil {
		return t.View.Query
	}
	if t.MaterializedView != nil {
		return t.MaterializedView.Query
	}
	return ""
}

func TestParseDropViewDDL(t *testing.T) {
	t.Parallel()
	p, d, id, ok := parseDropViewDDL(testProjectID, testDatasetID, "DROP VIEW `p.d.v`", false)
	if !ok {
		t.Fatal("expected ok")
	}
	if p != "p" || d != "d" || id != "v" {
		t.Errorf("got %s.%s.%s", p, d, id)
	}
	_, _, _, ok = parseDropViewDDL(testProjectID, testDatasetID, "DROP TABLE t", false)
	if ok {
		t.Error("DROP TABLE should not parse as DROP VIEW")
	}
	_, _, _, ok = parseDropViewDDL(testProjectID, testDatasetID, "DROP VIEW IF EXISTS ds.v", false)
	if !ok {
		t.Fatal("DROP VIEW IF EXISTS expected ok")
	}
	p, d, id, ok = parseDropViewDDL(testProjectID, testDatasetID, "DROP MATERIALIZED VIEW ds.mv", true)
	if !ok || p != testProjectID || d != "ds" || id != "mv" {
		t.Errorf("materialized drop = %s.%s.%s ok=%v", p, d, id, ok)
	}
}
