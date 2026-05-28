package handlers

import "net/http"

// The dataset / table / job method definitions are factored into
// individual helpers so each method's parameter set stays grouped with
// its path and id. They are not exported because they are only used to
// populate buildDiscoveryDocument.

func datasetsListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.datasets.list",
		Path:           "projects/{projectId}/datasets",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID},
		Parameters:     map[string]discoveryParameter{paramProjectID: pathString(paramProjectID)},
	}
}

func datasetsInsertMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.datasets.insert",
		Path:           "projects/{projectId}/datasets",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{paramProjectID},
		Parameters:     map[string]discoveryParameter{paramProjectID: pathString(paramProjectID)},
	}
}

func datasetsGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.datasets.get",
		Path:           "projects/{projectId}/datasets/{datasetId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramDatasetID: pathString(paramDatasetID),
		},
	}
}

func datasetsUpdateMethod() discoveryMethod {
	m := datasetsGetMethod()
	m.ID = "bigquery.datasets.update"
	m.HTTPMethod = http.MethodPut
	return m
}

func datasetsPatchMethod() discoveryMethod {
	m := datasetsGetMethod()
	m.ID = "bigquery.datasets.patch"
	m.HTTPMethod = http.MethodPatch
	return m
}

func datasetsDeleteMethod() discoveryMethod {
	m := datasetsGetMethod()
	m.ID = "bigquery.datasets.delete"
	m.HTTPMethod = http.MethodDelete
	return m
}

func datasetsUndeleteMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.datasets.undelete",
		Path:           "projects/{projectId}/datasets/{datasetId}:undelete",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{paramProjectID, paramDatasetID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramDatasetID: pathString(paramDatasetID),
		},
	}
}

func tableScopedParams() map[string]discoveryParameter {
	return map[string]discoveryParameter{
		paramProjectID: pathString(paramProjectID),
		paramDatasetID: pathString(paramDatasetID),
		paramTableID:   pathString(paramTableID),
	}
}

func tablesListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tables.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramDatasetID: pathString(paramDatasetID),
		},
	}
}

func tablesInsertMethod() discoveryMethod {
	m := tablesListMethod()
	m.ID = "bigquery.tables.insert"
	m.HTTPMethod = http.MethodPost
	return m
}

func tablesGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tables.get",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramTableID},
		Parameters:     tableScopedParams(),
	}
}

func tablesUpdateMethod() discoveryMethod {
	m := tablesGetMethod()
	m.ID = "bigquery.tables.update"
	m.HTTPMethod = http.MethodPut
	return m
}

func tablesPatchMethod() discoveryMethod {
	m := tablesGetMethod()
	m.ID = "bigquery.tables.patch"
	m.HTTPMethod = http.MethodPatch
	return m
}

func tablesDeleteMethod() discoveryMethod {
	m := tablesGetMethod()
	m.ID = "bigquery.tables.delete"
	m.HTTPMethod = http.MethodDelete
	return m
}

func tablesIamMethod(op string) discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tables." + op,
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}:" + op,
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramTableID},
		Parameters:     tableScopedParams(),
	}
}

func tabledataListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tabledata.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramTableID},
		Parameters:     tableScopedParams(),
	}
}

func tabledataInsertAllMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tabledata.insertAll",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramTableID},
		Parameters:     tableScopedParams(),
	}
}

func jobsListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.list",
		Path:           "projects/{projectId}/jobs",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID},
		Parameters:     map[string]discoveryParameter{paramProjectID: pathString(paramProjectID)},
	}
}

func jobsInsertMethod() discoveryMethod {
	m := jobsListMethod()
	m.ID = "bigquery.jobs.insert"
	m.HTTPMethod = http.MethodPost
	return m
}

func jobsGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.get",
		Path:           "projects/{projectId}/jobs/{jobId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramJobID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramJobID:     pathString(paramJobID),
		},
	}
}

func jobsCancelMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.cancel",
		Path:           "projects/{projectId}/jobs/{jobId}/cancel",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{paramProjectID, paramJobID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramJobID:     pathString(paramJobID),
		},
	}
}

func jobsDeleteMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.delete",
		Path:           "projects/{projectId}/jobs/{jobId}/delete",
		HTTPMethod:     http.MethodDelete,
		ParameterOrder: []string{paramProjectID, paramJobID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramJobID:     pathString(paramJobID),
		},
	}
}

func jobsQueryMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.query",
		Path:           "projects/{projectId}/queries",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{paramProjectID},
		Parameters:     map[string]discoveryParameter{paramProjectID: pathString(paramProjectID)},
	}
}

func jobsGetQueryResultsMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.getQueryResults",
		Path:           "projects/{projectId}/queries/{jobId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramJobID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramJobID:     pathString(paramJobID),
		},
	}
}

// modelScopedParams covers the path captures shared by every
// bigquery.models.* method that targets a specific model.
func modelScopedParams() map[string]discoveryParameter {
	return map[string]discoveryParameter{
		paramProjectID: pathString(paramProjectID),
		paramDatasetID: pathString(paramDatasetID),
		paramModelID:   pathString(paramModelID),
	}
}

func modelsListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.models.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/models",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramDatasetID: pathString(paramDatasetID),
		},
	}
}

func modelsGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.models.get",
		Path:           "projects/{projectId}/datasets/{datasetId}/models/{modelId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramModelID},
		Parameters:     modelScopedParams(),
	}
}

func modelsPatchMethod() discoveryMethod {
	m := modelsGetMethod()
	m.ID = "bigquery.models.patch"
	m.HTTPMethod = http.MethodPatch
	return m
}

func modelsDeleteMethod() discoveryMethod {
	m := modelsGetMethod()
	m.ID = "bigquery.models.delete"
	m.HTTPMethod = http.MethodDelete
	return m
}

// routineScopedParams covers the path captures shared by every
// bigquery.routines.* method that targets a specific routine.
func routineScopedParams() map[string]discoveryParameter {
	return map[string]discoveryParameter{
		paramProjectID: pathString(paramProjectID),
		paramDatasetID: pathString(paramDatasetID),
		paramRoutineID: pathString(paramRoutineID),
	}
}

func routinesListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.routines.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/routines",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID},
		Parameters: map[string]discoveryParameter{
			paramProjectID: pathString(paramProjectID),
			paramDatasetID: pathString(paramDatasetID),
		},
	}
}

func routinesInsertMethod() discoveryMethod {
	m := routinesListMethod()
	m.ID = "bigquery.routines.insert"
	m.HTTPMethod = http.MethodPost
	return m
}

func routinesGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.routines.get",
		Path:           "projects/{projectId}/datasets/{datasetId}/routines/{routineId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramRoutineID},
		Parameters:     routineScopedParams(),
	}
}

func routinesUpdateMethod() discoveryMethod {
	m := routinesGetMethod()
	m.ID = "bigquery.routines.update"
	m.HTTPMethod = http.MethodPut
	return m
}

func routinesDeleteMethod() discoveryMethod {
	m := routinesGetMethod()
	m.ID = "bigquery.routines.delete"
	m.HTTPMethod = http.MethodDelete
	return m
}

func rowAccessPoliciesListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.rowAccessPolicies.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{paramProjectID, paramDatasetID, paramTableID},
		Parameters:     tableScopedParams(),
	}
}
