package query

import "testing"

func TestRewriteInfoSchemaJobsSQLRegionDataset(t *testing.T) {
	t.Parallel()
	in := "SELECT job_id FROM `region-us`.`INFORMATION_SCHEMA.JOBS_BY_PROJECT` WHERE state = 'DONE'"
	got := RewriteInfoSchemaJobsSQL(in, "dev")
	want := "SELECT job_id FROM `dev`.`_bqemu_jobs`.`JOBS` WHERE state = 'DONE'"
	if got != want {
		t.Fatalf("got %q, want %q", got, want)
	}
}

func TestReferencesInfoSchemaJobs(t *testing.T) {
	t.Parallel()
	if !ReferencesInfoSchemaJobs("FROM `region-eu`.`INFORMATION_SCHEMA.JOBS`") {
		t.Fatal("expected match")
	}
	if ReferencesInfoSchemaJobs("FROM `ds`.INFORMATION_SCHEMA.TABLES") {
		t.Fatal("unexpected match")
	}
}
