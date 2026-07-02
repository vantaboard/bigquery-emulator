package handlers

import (
	"net/http"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const enginePolicyFederatedURL = "docs/ENGINE_POLICY.md#external-query-and-federated-sources"

func writeUnsupportedFederatedEnvelope(w http.ResponseWriter, feature string) {
	writeError(w, http.StatusNotImplemented, "notImplemented",
		feature+" is not supported by the BigQuery emulator (fixture-backed EXTERNAL_QUERY only; see "+
			enginePolicyFederatedURL+").")
}

// rejectUnsupportedTablePosture returns true when the handler wrote a 501.
func rejectUnsupportedTablePosture(w http.ResponseWriter, t *bqtypes.Table) bool {
	if t == nil {
		return false
	}
	if t.BiglakeConfiguration != nil {
		writeUnsupportedFederatedEnvelope(w,
			"BigLake tables (biglakeConfiguration)")
		return true
	}
	if t.ObjectTableOptions != nil {
		writeUnsupportedFederatedEnvelope(w, "Object tables (objectTableOptions)")
		return true
	}
	if t.ExternalDataConfiguration != nil {
		src := strings.ToUpper(strings.TrimSpace(t.ExternalDataConfiguration.SourceFormat))
		if src == "OBJECT_TABLE" {
			writeUnsupportedFederatedEnvelope(w, "Object tables (OBJECT_TABLE sourceFormat)")
			return true
		}
	}
	return false
}

// rejectUnsupportedDatasetPosture returns true when the handler wrote a 501.
func rejectUnsupportedDatasetPosture(w http.ResponseWriter, ds *bqtypes.Dataset) bool {
	if ds == nil || ds.ExternalDatasetReference == nil {
		return false
	}
	writeUnsupportedFederatedEnvelope(w,
		"External datasets (Spanner / Cloud SQL externalDatasetReference)")
	return true
}
