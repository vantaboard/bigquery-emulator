package handlers

import "net/http"

// ProjectList implements `bigquery.projects.list`.
func ProjectList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		NotImplemented(w, r)
	}
}

// ProjectGet implements `bigquery.projects.getServiceAccount`-style probes
// against /bigquery/v2/projects/{projectId}.
func ProjectGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		NotImplemented(w, r)
	}
}
