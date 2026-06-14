package external

import (
	"context"
	_ "embed"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

// Public sample sheet (Example Spreadsheet, Class Data tab) used for fixture
// and opt-in live conformance tests.
const (
	ClassDataSheetDocID = "1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"
	classDataSheetGID   = "0"
)

//go:embed fixtures/google_sheets/class_data.csv
var classDataFixtureCSV []byte

// Resolver carries per-source configuration for materialization.
type Resolver struct {
	cfg *sourceconfig.Config
}

// NewResolver returns a resolver; nil cfg uses package defaults.
func NewResolver(cfg *sourceconfig.Config) *Resolver {
	return &Resolver{cfg: cfg}
}

func (r *Resolver) modeSheets(docID string) sourceconfig.Mode {
	if r == nil || r.cfg == nil {
		return sourceconfig.ModeFixture
	}
	return r.cfg.ResolveGoogleSheets(docID)
}

func (r *Resolver) fixturePath(docID, name string) string {
	if r == nil || r.cfg == nil {
		return ""
	}
	return filepath.Join(r.cfg.FixtureRoot(), "google_sheets", docID, name)
}

func fetchGoogleSheetsCSV(
	ctx context.Context,
	r *Resolver,
	cfg *bqtypes.ExternalDataConfiguration,
) ([]byte, error) {
	if len(cfg.SourceURIs) == 0 {
		return nil, errors.New("google sheets external table requires sourceUri")
	}
	docID := sourceconfig.ExtractSheetDocID(cfg.SourceURIs[0])
	if docID == "" {
		return nil, fmt.Errorf("could not parse Google Sheets doc id from %q", cfg.SourceURIs[0])
	}
	mode := r.modeSheets(docID)
	switch mode {
	case sourceconfig.ModeLive:
		return fetchLiveSheetsCSV(ctx, docID, cfg.GoogleSheetsOptions)
	case sourceconfig.ModeLocal:
		return nil, errors.New("google sheets local mode is not supported; use fixture or live")
	default:
		return loadFixtureSheetsCSV(r, docID)
	}
}

func loadFixtureSheetsCSV(r *Resolver, docID string) ([]byte, error) {
	if docID == ClassDataSheetDocID {
		return classDataFixtureCSV, nil
	}
	if r != nil && r.cfg != nil && r.cfg.DataDir != "" {
		for _, name := range []string{"data.csv", "class_data.csv", "sheet.csv"} {
			p := r.fixturePath(docID, name)
			if raw, err := os.ReadFile(p); err == nil { //nolint:gosec // operator data dir
				return raw, nil
			}
		}
	}
	return nil, fmt.Errorf("no fixture snapshot for Google Sheets doc %s", docID)
}

func fetchLiveSheetsCSV(
	ctx context.Context,
	docID string,
	opts *bqtypes.GoogleSheetsOptions,
) ([]byte, error) {
	gid := classDataSheetGID
	if opts != nil && strings.TrimSpace(opts.Range) != "" {
		// Range like "Class Data!A1:F31" — live export uses gid; public sample uses gid 0.
		_ = opts.Range
	}
	exportURL := fmt.Sprintf(
		"https://docs.google.com/spreadsheets/d/%s/export?format=csv&gid=%s",
		url.PathEscape(docID), url.QueryEscape(gid))
	req, err := http.NewRequestWithContext(ctx, http.MethodGet, exportURL, nil)
	if err != nil {
		return nil, err
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("fetch google sheets %s: %w", docID, err)
	}
	defer func() { _ = resp.Body.Close() }()
	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(io.LimitReader(resp.Body, 512))
		return nil, fmt.Errorf("fetch google sheets %s: HTTP %d: %s",
			docID, resp.StatusCode, strings.TrimSpace(string(body)))
	}
	return io.ReadAll(resp.Body)
}

func parseSheetsExternal(
	ctx context.Context,
	r *Resolver,
	cfg *bqtypes.ExternalDataConfiguration,
	schema *bqtypes.TableSchema,
	skipLeading int,
) (load.ParsedRows, error) {
	data, err := fetchGoogleSheetsCSV(ctx, r, cfg)
	if err != nil {
		return load.ParsedRows{}, err
	}
	skip := skipLeading
	if cfg.GoogleSheetsOptions != nil && cfg.GoogleSheetsOptions.SkipLeadingRows > 0 {
		skip = cfg.GoogleSheetsOptions.SkipLeadingRows
	} else if skip == 0 {
		skip = 1
	}
	return load.ParseSource("CSV", data, schema, skip, cfg.Autodetect)
}
