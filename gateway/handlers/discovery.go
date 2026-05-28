package handlers

import (
	"net/http"
	"sync"
)

// Discovery implements the BigQuery v2 discovery endpoint:
//
//	GET /discovery/v1/apis/bigquery/v2/rest
//
// Google API client libraries fetch a discovery document at startup to
// learn the method surface of the service they are talking to. The
// emulator serves a hand-written, minimal subset of the upstream
// discovery JSON that lists exactly the methods routed in
// [gateway.NewServer] (see docs/REST_API.md). It is deliberately small:
// just enough that a client library can find a `kind`, enumerate the
// method ids, and locate their paths/HTTP verbs.
//
// The shape follows Google's documented `discovery#restDescription`
// format. We do not claim parity with the upstream document for fields
// like `schemas`, `auth`, or `revision`; clients that depend on those
// should hit the real BigQuery discovery URL.
func Discovery(_ Dependencies) http.HandlerFunc {
	doc := buildDiscoveryDocument()
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, doc)
	}
}

// buildDiscoveryDocument constructs the minimal restDescription served
// by the emulator. It is built once and reused for every request.
//
// The catalog of methods here is the authoritative list mirrored in
// docs/REST_API.md and gateway/server.go. Keep all three in sync when
// adding a new endpoint: add the mux entry, add the table row, and add
// a method entry here.
var buildDiscoveryDocument = sync.OnceValue(func() discoveryDocument {
	return discoveryDocument{
		Kind:             discoveryKind,
		Etag:             "",
		DiscoveryVersion: "v1",
		ID:               "bigquery:v2",
		Name:             "bigquery",
		Version:          "v2",
		Title:            "BigQuery API (emulator)",
		Description: "Local BigQuery emulator REST surface. " +
			"This discovery document lists only the methods the emulator " +
			"actually routes; see docs/REST_API.md for the canonical mapping.",
		Protocol:    "rest",
		RootURL:     "",
		ServicePath: "bigquery/v2/",
		BasePath:    "/bigquery/v2/",
		BaseURL:     "/bigquery/v2/",
		BatchPath:   "batch/bigquery/v2",
		Parameters:  commonParameters(),
		Resources: map[string]discoveryResource{
			discoveryResourceProjects: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList: {
						ID:         "bigquery.projects.list",
						Path:       "projects",
						HTTPMethod: http.MethodGet,
					},
					"getServiceAccount": {
						ID:             "bigquery.projects.getServiceAccount",
						Path:           "projects/{projectId}/serviceAccount",
						HTTPMethod:     http.MethodGet,
						ParameterOrder: []string{paramProjectID},
						Parameters: map[string]discoveryParameter{
							paramProjectID: pathString(paramProjectID),
						},
					},
				},
			},
			discoveryResourceDatasets: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList:   datasetsListMethod(),
					discoveryMethodInsert: datasetsInsertMethod(),
					discoveryMethodGet:    datasetsGetMethod(),
					discoveryMethodUpdate: datasetsUpdateMethod(),
					discoveryMethodPatch:  datasetsPatchMethod(),
					discoveryMethodDelete: datasetsDeleteMethod(),
					"undelete":            datasetsUndeleteMethod(),
				},
			},
			discoveryResourceTables: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList:   tablesListMethod(),
					discoveryMethodInsert: tablesInsertMethod(),
					discoveryMethodGet:    tablesGetMethod(),
					discoveryMethodUpdate: tablesUpdateMethod(),
					discoveryMethodPatch:  tablesPatchMethod(),
					discoveryMethodDelete: tablesDeleteMethod(),
					"getIamPolicy":        tablesIamMethod("getIamPolicy"),
					"setIamPolicy":        tablesIamMethod("setIamPolicy"),
					"testIamPermissions":  tablesIamMethod("testIamPermissions"),
				},
			},
			discoveryResourceTabledata: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList: tabledataListMethod(),
					"insertAll":         tabledataInsertAllMethod(),
				},
			},
			discoveryResourceJobs: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList:   jobsListMethod(),
					discoveryMethodInsert: jobsInsertMethod(),
					discoveryMethodGet:    jobsGetMethod(),
					"cancel":              jobsCancelMethod(),
					discoveryMethodDelete: jobsDeleteMethod(),
					discoveryMethodQuery:  jobsQueryMethod(),
					"getQueryResults":     jobsGetQueryResultsMethod(),
				},
			},
			discoveryResourceModels: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList:   modelsListMethod(),
					discoveryMethodGet:    modelsGetMethod(),
					discoveryMethodPatch:  modelsPatchMethod(),
					discoveryMethodDelete: modelsDeleteMethod(),
				},
			},
			discoveryResourceRoutines: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList:   routinesListMethod(),
					discoveryMethodInsert: routinesInsertMethod(),
					discoveryMethodGet:    routinesGetMethod(),
					discoveryMethodUpdate: routinesUpdateMethod(),
					discoveryMethodDelete: routinesDeleteMethod(),
				},
			},
			discoveryResourceRowPolicy: {
				Methods: map[string]discoveryMethod{
					discoveryMethodList: rowAccessPoliciesListMethod(),
				},
			},
		},
	}
})

// discoveryKind is the kind value Google's discovery service stamps on
// every restDescription. The verification command (`jq .kind`) asserts
// this exact string, so it must not drift.
const discoveryKind = "discovery#restDescription"

// Discovery-document path-parameter names. The upstream BigQuery REST
// API exposes resources keyed off these {…} segments and the
// discovery JSON has to spell them out verbatim, which is why the same
// string repeats dozens of times across the method tables. Hoisted to
// consts so the JSON wire shape stays a single source of truth.
const (
	paramProjectID = "projectId"
	paramDatasetID = "datasetId"
	paramTableID   = "tableId"
	paramJobID     = "jobId"
	paramModelID   = "modelId"
	paramRoutineID = "routineId"
)

// Discovery-document resource keys. These are the JSON-object keys
// inside the document's top-level `resources` map; client libraries
// dispatch on them to find the method tables for each REST resource.
const (
	discoveryResourceProjects   = "projects"
	discoveryResourceDatasets   = "datasets"
	discoveryResourceTables     = "tables"
	discoveryResourceTabledata  = "tabledata"
	discoveryResourceJobs       = "jobs"
	discoveryResourceModels     = "models"
	discoveryResourceRoutines   = "routines"
	discoveryResourceRowPolicy  = "rowAccessPolicies"
	discoveryMethodList         = "list"
	discoveryMethodGet          = "get"
	discoveryMethodInsert       = "insert"
	discoveryMethodUpdate       = "update"
	discoveryMethodPatch        = "patch"
	discoveryMethodDelete       = "delete"
	discoveryMethodQuery        = "query"
	discoveryParamTypeString    = "string"
	discoveryParamLocationPath  = "path"
	discoveryParamLocationQuery = "query"
)

// discoveryDocument is the trimmed-down restDescription served by the
// emulator. It models only the fields client libraries actually consult
// for routing; the upstream document also contains schemas, scopes,
// auth, and feature flags which the emulator does not need.
type discoveryDocument struct {
	Kind             string                        `json:"kind"`
	Etag             string                        `json:"etag,omitempty"`
	DiscoveryVersion string                        `json:"discoveryVersion"`
	ID               string                        `json:"id"`
	Name             string                        `json:"name"`
	Version          string                        `json:"version"`
	Title            string                        `json:"title"`
	Description      string                        `json:"description,omitempty"`
	Protocol         string                        `json:"protocol"`
	RootURL          string                        `json:"rootUrl"`
	ServicePath      string                        `json:"servicePath"`
	BasePath         string                        `json:"basePath"`
	BaseURL          string                        `json:"baseUrl"`
	BatchPath        string                        `json:"batchPath,omitempty"`
	Parameters       map[string]discoveryParameter `json:"parameters,omitempty"`
	Resources        map[string]discoveryResource  `json:"resources"`
}

type discoveryResource struct {
	Methods map[string]discoveryMethod `json:"methods"`
}

type discoveryMethod struct {
	ID             string                        `json:"id"`
	Path           string                        `json:"path"`
	HTTPMethod     string                        `json:"httpMethod"`
	Description    string                        `json:"description,omitempty"`
	ParameterOrder []string                      `json:"parameterOrder,omitempty"`
	Parameters     map[string]discoveryParameter `json:"parameters,omitempty"`
}

type discoveryParameter struct {
	Type        string `json:"type"`
	Location    string `json:"location"`
	Required    bool   `json:"required,omitempty"`
	Description string `json:"description,omitempty"`
}

// pathString returns a required string path parameter with the given
// name. It is a small ergonomic helper so the method tables below stay
// readable.
func pathString(name string) discoveryParameter {
	return discoveryParameter{
		Type:        discoveryParamTypeString,
		Location:    discoveryParamLocationPath,
		Required:    true,
		Description: name,
	}
}

// commonParameters are the Google-standard query parameters every
// method accepts. We only declare the handful BigQuery clients actually
// pass; the full upstream list is much longer.
func commonParameters() map[string]discoveryParameter {
	return map[string]discoveryParameter{
		"alt": {
			Type:        discoveryParamTypeString,
			Location:    discoveryParamLocationQuery,
			Description: "Data format for the response.",
		},
		"prettyPrint": {
			Type:        "boolean",
			Location:    discoveryParamLocationQuery,
			Description: "Returns response with indentations and line breaks.",
		},
		"key": {
			Type:        discoveryParamTypeString,
			Location:    discoveryParamLocationQuery,
			Description: "API key. Ignored by the emulator.",
		},
		"access_token": {
			Type:        discoveryParamTypeString,
			Location:    discoveryParamLocationQuery,
			Description: "OAuth access token. Ignored by the emulator.",
		},
	}
}

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
