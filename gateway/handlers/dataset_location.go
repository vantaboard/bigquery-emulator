package handlers

import (
	"fmt"
	"net/http"
	"strings"
)

// headerEmulatorAPIRegion is sent by thirdparty harnesses when the
// Node/Go client uses a regional Google API hostname while the TCP
// connection targets loopback. See third_party/node-bigquery-tests/
// test/setup.js and third_party/golang-bigquery-tests/bqopts.
const headerEmulatorAPIRegion = "X-BigQuery-Emulator-Api-Region"

// datasetMultiRegions is the small multi-region set upstream samples
// exercise. See docs/bigquery/docs/locations.md.
var datasetMultiRegions = map[string]struct{}{
	"US": {},
	"EU": {},
}

// datasetRegions is a subset of supported single regions wide enough
// for thirdparty samples (us-east4/us-central1/eu, ...).
var datasetRegions = map[string]struct{}{
	"africa-south1":           {},
	"asia-east1":              {},
	"asia-east2":              {},
	"asia-northeast1":         {},
	"asia-northeast2":         {},
	"asia-northeast3":         {},
	"asia-south1":             {},
	"asia-south2":             {},
	"asia-southeast1":         {},
	"asia-southeast2":         {},
	"australia-southeast1":    {},
	"australia-southeast2":    {},
	"europe-central2":         {},
	"europe-north1":           {},
	"europe-southwest1":       {},
	"europe-west1":            {},
	"europe-west10":           {},
	"europe-west12":           {},
	"europe-west2":            {},
	"europe-west3":            {},
	"europe-west4":            {},
	"europe-west6":            {},
	"europe-west8":            {},
	"europe-west9":            {},
	"me-central1":             {},
	"me-central2":             {},
	"me-west1":                {},
	"northamerica-northeast1": {},
	"northamerica-northeast2": {},
	"southamerica-east1":      {},
	"southamerica-west1":      {},
	"us-central1":             {},
	"us-east1":                {},
	"us-east4":                {},
	"us-east5":                {},
	"us-south1":               {},
	"us-west1":                {},
	"us-west2":                {},
	"us-west3":                {},
	"us-west4":                {},
}

func emulatorAPIRegion(r *http.Request) string {
	return strings.ToLower(strings.TrimSpace(r.Header.Get(headerEmulatorAPIRegion)))
}

// normalizeDatasetLocation canonicalizes a BigQuery dataset location
// string. Returns empty when the value is not recognized.
func normalizeDatasetLocation(location string) string {
	loc := strings.TrimSpace(location)
	if loc == "" {
		return "US"
	}
	if upper := strings.ToUpper(loc); len(upper) <= 3 {
		if _, ok := datasetMultiRegions[upper]; ok {
			return upper
		}
	}
	lower := strings.ToLower(loc)
	if _, ok := datasetRegions[lower]; ok {
		return lower
	}
	return ""
}

// locationMatchesAPIRegion enforces regional-endpoint parity exercised
// by node-bigquery-tests `should fail to create a dataset using a
// different region from the client endpoint`.
func locationMatchesAPIRegion(normalizedLocation, apiRegion string) bool {
	if apiRegion == "" {
		return true
	}
	if strings.EqualFold(normalizedLocation, apiRegion) {
		return true
	}
	// eu-bigquery.googleapis.com + location "eu" (normalized to EU).
	if apiRegion == "eu" && normalizedLocation == "EU" {
		return true
	}
	return false
}

// validateDatasetLocation checks the dataset location before any
// engine RPC so invalid regions surface as "Invalid storage region"
// ahead of duplicate-id errors from RegisterDataset.
func validateDatasetLocation(r *http.Request, location string) error {
	normalized := normalizeDatasetLocation(location)
	if normalized == "" {
		raw := strings.TrimSpace(location)
		if raw == "" {
			raw = "US"
		}
		return fmt.Errorf("Invalid storage region: %s", raw) //nolint:staticcheck // BigQuery client error text
	}
	apiRegion := emulatorAPIRegion(r)
	if !locationMatchesAPIRegion(normalized, apiRegion) {
		display := strings.TrimSpace(location)
		if display == "" {
			display = normalized
		}
		return fmt.Errorf("Invalid storage region: %s", display) //nolint:staticcheck // BigQuery client error text
	}
	return nil
}
