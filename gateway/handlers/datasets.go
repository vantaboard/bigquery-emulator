package handlers

import "net/http"

// DatasetList implements `bigquery.datasets.list`.
func DatasetList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetInsert implements `bigquery.datasets.insert`.
func DatasetInsert(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetGet implements `bigquery.datasets.get`.
func DatasetGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetUpdate implements `bigquery.datasets.update`.
func DatasetUpdate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetPatch implements `bigquery.datasets.patch`.
func DatasetPatch(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// DatasetDelete implements `bigquery.datasets.delete`.
func DatasetDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
