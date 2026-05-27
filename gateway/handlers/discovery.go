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
			"projects": {
				Methods: map[string]discoveryMethod{
					"list": {
						ID:         "bigquery.projects.list",
						Path:       "projects",
						HTTPMethod: http.MethodGet,
					},
					"getServiceAccount": {
						ID:             "bigquery.projects.getServiceAccount",
						Path:           "projects/{projectId}/serviceAccount",
						HTTPMethod:     http.MethodGet,
						ParameterOrder: []string{"projectId"},
						Parameters: map[string]discoveryParameter{
							"projectId": pathString("projectId"),
						},
					},
				},
			},
			"datasets": {
				Methods: map[string]discoveryMethod{
					"list":     datasetsListMethod(),
					"insert":   datasetsInsertMethod(),
					"get":      datasetsGetMethod(),
					"update":   datasetsUpdateMethod(),
					"patch":    datasetsPatchMethod(),
					"delete":   datasetsDeleteMethod(),
					"undelete": datasetsUndeleteMethod(),
				},
			},
			"tables": {
				Methods: map[string]discoveryMethod{
					"list":               tablesListMethod(),
					"insert":             tablesInsertMethod(),
					"get":                tablesGetMethod(),
					"update":             tablesUpdateMethod(),
					"patch":              tablesPatchMethod(),
					"delete":             tablesDeleteMethod(),
					"getIamPolicy":       tablesIamMethod("getIamPolicy"),
					"setIamPolicy":       tablesIamMethod("setIamPolicy"),
					"testIamPermissions": tablesIamMethod("testIamPermissions"),
				},
			},
			"tabledata": {
				Methods: map[string]discoveryMethod{
					"list":      tabledataListMethod(),
					"insertAll": tabledataInsertAllMethod(),
				},
			},
			"jobs": {
				Methods: map[string]discoveryMethod{
					"list":            jobsListMethod(),
					"insert":          jobsInsertMethod(),
					"get":             jobsGetMethod(),
					"cancel":          jobsCancelMethod(),
					"delete":          jobsDeleteMethod(),
					"query":           jobsQueryMethod(),
					"getQueryResults": jobsGetQueryResultsMethod(),
				},
			},
			"models": {
				Methods: map[string]discoveryMethod{
					"list":   modelsListMethod(),
					"get":    modelsGetMethod(),
					"patch":  modelsPatchMethod(),
					"delete": modelsDeleteMethod(),
				},
			},
			"routines": {
				Methods: map[string]discoveryMethod{
					"list":   routinesListMethod(),
					"insert": routinesInsertMethod(),
					"get":    routinesGetMethod(),
					"update": routinesUpdateMethod(),
					"delete": routinesDeleteMethod(),
				},
			},
			"rowAccessPolicies": {
				Methods: map[string]discoveryMethod{
					"list": rowAccessPoliciesListMethod(),
				},
			},
		},
	}
})

// discoveryKind is the kind value Google's discovery service stamps on
// every restDescription. The verification command (`jq .kind`) asserts
// this exact string, so it must not drift.
const discoveryKind = "discovery#restDescription"

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
		Type:        "string",
		Location:    "path",
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
			Type:        "string",
			Location:    "query",
			Description: "Data format for the response.",
		},
		"prettyPrint": {
			Type:        "boolean",
			Location:    "query",
			Description: "Returns response with indentations and line breaks.",
		},
		"key": {
			Type:        "string",
			Location:    "query",
			Description: "API key. Ignored by the emulator.",
		},
		"access_token": {
			Type:        "string",
			Location:    "query",
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
		ParameterOrder: []string{"projectId"},
		Parameters:     map[string]discoveryParameter{"projectId": pathString("projectId")},
	}
}

func datasetsInsertMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.datasets.insert",
		Path:           "projects/{projectId}/datasets",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{"projectId"},
		Parameters:     map[string]discoveryParameter{"projectId": pathString("projectId")},
	}
}

func datasetsGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.datasets.get",
		Path:           "projects/{projectId}/datasets/{datasetId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "datasetId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"datasetId": pathString("datasetId"),
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
		ParameterOrder: []string{"projectId", "datasetId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"datasetId": pathString("datasetId"),
		},
	}
}

func tableScopedParams() map[string]discoveryParameter {
	return map[string]discoveryParameter{
		"projectId": pathString("projectId"),
		"datasetId": pathString("datasetId"),
		"tableId":   pathString("tableId"),
	}
}

func tablesListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tables.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "datasetId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"datasetId": pathString("datasetId"),
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
		ParameterOrder: []string{"projectId", "datasetId", "tableId"},
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
		ParameterOrder: []string{"projectId", "datasetId", "tableId"},
		Parameters:     tableScopedParams(),
	}
}

func tabledataListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tabledata.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "datasetId", "tableId"},
		Parameters:     tableScopedParams(),
	}
}

func tabledataInsertAllMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.tabledata.insertAll",
		Path:           "projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{"projectId", "datasetId", "tableId"},
		Parameters:     tableScopedParams(),
	}
}

func jobsListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.list",
		Path:           "projects/{projectId}/jobs",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId"},
		Parameters:     map[string]discoveryParameter{"projectId": pathString("projectId")},
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
		ParameterOrder: []string{"projectId", "jobId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"jobId":     pathString("jobId"),
		},
	}
}

func jobsCancelMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.cancel",
		Path:           "projects/{projectId}/jobs/{jobId}/cancel",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{"projectId", "jobId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"jobId":     pathString("jobId"),
		},
	}
}

func jobsDeleteMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.delete",
		Path:           "projects/{projectId}/jobs/{jobId}/delete",
		HTTPMethod:     http.MethodDelete,
		ParameterOrder: []string{"projectId", "jobId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"jobId":     pathString("jobId"),
		},
	}
}

func jobsQueryMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.query",
		Path:           "projects/{projectId}/queries",
		HTTPMethod:     http.MethodPost,
		ParameterOrder: []string{"projectId"},
		Parameters:     map[string]discoveryParameter{"projectId": pathString("projectId")},
	}
}

func jobsGetQueryResultsMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.jobs.getQueryResults",
		Path:           "projects/{projectId}/queries/{jobId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "jobId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"jobId":     pathString("jobId"),
		},
	}
}

// modelScopedParams covers the path captures shared by every
// bigquery.models.* method that targets a specific model.
func modelScopedParams() map[string]discoveryParameter {
	return map[string]discoveryParameter{
		"projectId": pathString("projectId"),
		"datasetId": pathString("datasetId"),
		"modelId":   pathString("modelId"),
	}
}

func modelsListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.models.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/models",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "datasetId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"datasetId": pathString("datasetId"),
		},
	}
}

func modelsGetMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.models.get",
		Path:           "projects/{projectId}/datasets/{datasetId}/models/{modelId}",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "datasetId", "modelId"},
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
		"projectId": pathString("projectId"),
		"datasetId": pathString("datasetId"),
		"routineId": pathString("routineId"),
	}
}

func routinesListMethod() discoveryMethod {
	return discoveryMethod{
		ID:             "bigquery.routines.list",
		Path:           "projects/{projectId}/datasets/{datasetId}/routines",
		HTTPMethod:     http.MethodGet,
		ParameterOrder: []string{"projectId", "datasetId"},
		Parameters: map[string]discoveryParameter{
			"projectId": pathString("projectId"),
			"datasetId": pathString("datasetId"),
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
		ParameterOrder: []string{"projectId", "datasetId", "routineId"},
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
		ParameterOrder: []string{"projectId", "datasetId", "tableId"},
		Parameters:     tableScopedParams(),
	}
}
