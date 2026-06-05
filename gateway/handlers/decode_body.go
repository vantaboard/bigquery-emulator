package handlers

import (
	"encoding/json"
	"io"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func decodeDatasetBody(w http.ResponseWriter, r *http.Request) (bqtypes.Dataset, bool) {
	var ds bqtypes.Dataset
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not read dataset request body: "+err.Error())
		return ds, false
	}
	if len(body) == 0 {
		return ds, true
	}
	if err := json.Unmarshal(body, &ds); err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not parse dataset request body as JSON: "+err.Error())
		return ds, false
	}
	return ds, true
}

func decodeTableBody(w http.ResponseWriter, r *http.Request) (bqtypes.Table, bool) {
	var t bqtypes.Table
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not read table request body: "+err.Error())
		return t, false
	}
	if len(body) == 0 {
		return t, true
	}
	if err := json.Unmarshal(body, &t); err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not parse table request body as JSON: "+err.Error())
		return t, false
	}
	return t, true
}
