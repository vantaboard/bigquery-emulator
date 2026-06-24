package external

import (
	"errors"
	"fmt"
	"slices"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const sourceFormatBigtable = "BIGTABLE"

// IsBigtableSourceFormat reports whether format is BIGTABLE.
func IsBigtableSourceFormat(format string) bool {
	return strings.EqualFold(strings.TrimSpace(format), sourceFormatBigtable)
}

// IsBigtableSourceURI reports whether uri is a Bigtable REST source URI.
func IsBigtableSourceURI(uri string) bool {
	u := strings.TrimSpace(uri)
	return strings.Contains(u, "googleapis.com/bigtable/") ||
		strings.HasPrefix(u, "https://bigtable.googleapis.com/")
}

// ValidateBigtableURI checks the canonical Bigtable external URI shape.
func ValidateBigtableURI(uri string) error {
	if !IsBigtableSourceURI(uri) {
		return fmt.Errorf("invalid Bigtable sourceUri: %q", uri)
	}
	if !strings.Contains(uri, "/projects/") ||
		!strings.Contains(uri, "/instances/") ||
		!strings.Contains(uri, "/tables/") {
		return fmt.Errorf(
			"invalid Bigtable sourceUri (expected .../projects/P/instances/I/tables/T): %q",
			uri,
		)
	}
	return nil
}

// IsAzureBlobURI reports Azure Blob / ADLS URIs the UI may submit.
func IsAzureBlobURI(uri string) bool {
	u := strings.TrimSpace(strings.ToLower(uri))
	return strings.HasPrefix(u, "azure://") ||
		strings.Contains(u, ".blob.core.windows.net/") ||
		strings.Contains(u, ".dfs.core.windows.net/")
}

// IsGoogleDriveURI reports non-Sheets Google Drive URIs.
func IsGoogleDriveURI(uri string) bool {
	u := strings.TrimSpace(uri)
	return strings.Contains(u, "drive.google.com/") &&
		!strings.Contains(u, "spreadsheets")
}

// UnsupportedAzureBlobError is returned for Azure external-table URIs.
func UnsupportedAzureBlobError() error {
	return errors.New("azure blob storage external tables are not supported in the emulator")
}

// UnsupportedDriveError is returned for Google Drive file URIs.
func UnsupportedDriveError() error {
	return errors.New(
		"google drive external tables are not supported in the emulator (use GOOGLE_SHEETS for spreadsheets)",
	)
}

func isBigtable(cfg *bqtypes.ExternalDataConfiguration) bool {
	if IsBigtableSourceFormat(cfg.SourceFormat) {
		return true
	}
	return slices.ContainsFunc(cfg.SourceURIs, IsBigtableSourceURI)
}
