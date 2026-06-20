package handlers

// persistViewFromDDL registers a view parsed from CREATE [OR REPLACE]
// VIEW / CREATE [OR REPLACE] MATERIALIZED VIEW DDL in the gateway
// MetadataStore so tables.list / tables.get surface type and
// view.query (or materializedView.query) for query-job-created views.
func persistViewFromDDL(
	deps *Dependencies,
	projectID, defaultDatasetID, sql string,
) {
	t, ok := parseCreateViewDDL(projectID, defaultDatasetID, sql)
	if !ok {
		return
	}
	ref := t.TableReference
	deps.Metadata.PutTable(ref.ProjectID, ref.DatasetID, ref.TableID, t)
}

// evictViewFromDDL removes view metadata stashed by persistViewFromDDL
// after DROP VIEW / DROP MATERIALIZED VIEW DDL. DROP VIEW surfaces as
// statementType DROP_TABLE in the engine envelope; parseDropViewDDL
// distinguishes it from DROP TABLE.
func evictViewFromDDL(
	deps *Dependencies,
	projectID, defaultDatasetID, sql string,
	materializedOnly bool,
) {
	pID, dID, tID, ok := parseDropViewDDL(projectID, defaultDatasetID, sql, materializedOnly)
	if !ok {
		return
	}
	deps.Metadata.DeleteTable(pID, dID, tID)
}

// handleViewDDLAfterQuery mirrors routines/models DDL persistence for
// views created or dropped through jobs.query / jobs.insert query jobs.
func handleViewDDLAfterQuery(
	deps *Dependencies,
	projectID, defaultDatasetID, sql, statementType string,
) {
	switch statementType {
	case "CREATE_VIEW", "CREATE_MATERIALIZED_VIEW":
		persistViewFromDDL(deps, projectID, defaultDatasetID, sql)
	case "DROP_MATERIALIZED_VIEW":
		evictViewFromDDL(deps, projectID, defaultDatasetID, sql, true)
	case "DROP_TABLE":
		evictViewFromDDL(deps, projectID, defaultDatasetID, sql, false)
	}
}
