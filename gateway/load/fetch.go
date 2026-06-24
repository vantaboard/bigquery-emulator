// Package load implements the data plane for BigQuery LOAD jobs:
// fetch source bytes, parse CSV/JSON, and bulk-insert into the engine catalog.
package load

import (
	"context"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strings"
)

// FetchSource reads all bytes for a load-job source URI. Supports gs://
// (via STORAGE_EMULATOR_HOST or https://storage.googleapis.com) and
// file:// paths for local fixtures.
func FetchSource(ctx context.Context, uri string) ([]byte, error) {
	switch {
	case strings.HasPrefix(uri, "gs://"):
		return fetchGCS(ctx, uri)
	case strings.HasPrefix(uri, "s3://"):
		return fetchS3(ctx, uri)
	case strings.HasPrefix(uri, "file://"):
		path := strings.TrimPrefix(uri, "file://")
		return os.ReadFile(path) //nolint:gosec // LOAD jobs intentionally read caller file:// URIs
	default:
		if filepath.IsAbs(uri) {
			return os.ReadFile(uri) //nolint:gosec // absolute paths for local load samples
		}
		return nil, fmt.Errorf("unsupported sourceUri scheme: %q", uri)
	}
}

func fetchS3(ctx context.Context, s3URI string) ([]byte, error) {
	endpoint := strings.TrimRight(strings.TrimSpace(os.Getenv("S3_ENDPOINT")), "/")
	if endpoint == "" {
		return nil, errors.New("s3:// load sources require S3_ENDPOINT (dev-only); use gs:// or file:// instead")
	}
	rest := strings.TrimPrefix(s3URI, "s3://")
	slash := strings.Index(rest, "/")
	if slash <= 0 || slash == len(rest)-1 {
		return nil, fmt.Errorf("invalid s3:// uri: %q", s3URI)
	}
	bucket := rest[:slash]
	key := rest[slash+1:]
	mediaURL := endpoint + "/" + bucket + "/" + key

	req, err := http.NewRequestWithContext(ctx, http.MethodGet, mediaURL, nil) //nolint:gosec // dev-only S3_ENDPOINT fetch for LOAD jobs
	if err != nil {
		return nil, err
	}
	resp, err := http.DefaultClient.Do(req) //nolint:gosec // dev-only S3_ENDPOINT fetch for LOAD jobs
	if err != nil {
		return nil, fmt.Errorf("fetch %s: %w", s3URI, err)
	}
	defer func() { _ = resp.Body.Close() }()
	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(io.LimitReader(resp.Body, 512))
		return nil, fmt.Errorf("fetch %s: HTTP %d: %s", s3URI, resp.StatusCode, strings.TrimSpace(string(body)))
	}
	data, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("read %s: %w", s3URI, err)
	}
	return data, nil
}

func fetchGCS(ctx context.Context, gsURI string) ([]byte, error) {
	rest := strings.TrimPrefix(gsURI, "gs://")
	slash := strings.Index(rest, "/")
	if slash <= 0 || slash == len(rest)-1 {
		return nil, fmt.Errorf("invalid gs:// uri: %q", gsURI)
	}
	bucket := rest[:slash]
	object := rest[slash+1:]

	base := storageEmulatorBase()
	mediaURL := fmt.Sprintf("%s/storage/v1/b/%s/o/%s?alt=media",
		base, url.PathEscape(bucket), url.PathEscape(object))

	req, err := http.NewRequestWithContext(ctx, http.MethodGet, mediaURL, nil)
	if err != nil {
		return nil, err
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("fetch %s: %w", gsURI, err)
	}
	defer func() { _ = resp.Body.Close() }()
	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(io.LimitReader(resp.Body, 512))
		return nil, fmt.Errorf("fetch %s: HTTP %d: %s", gsURI, resp.StatusCode, strings.TrimSpace(string(body)))
	}
	data, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("read %s: %w", gsURI, err)
	}
	return data, nil
}

// storageEmulatorBase returns the HTTP origin for GCS JSON API media
// downloads. Mirrors scripts/preflight_node_samples_gcs.sh normalization.
func storageEmulatorBase() string {
	host := strings.TrimSpace(os.Getenv("STORAGE_EMULATOR_HOST"))
	if host == "" {
		port := os.Getenv("FAKE_GCS_PORT")
		if port == "" {
			port = "4443"
		}
		return "http://127.0.0.1:" + port
	}
	host = strings.TrimPrefix(host, "http://")
	host = strings.TrimPrefix(host, "https://")
	host = strings.TrimPrefix(host, "//")
	if strings.Contains(host, ":") {
		return "http://" + host
	}
	port := os.Getenv("FAKE_GCS_PORT")
	if port == "" {
		port = "4443"
	}
	return "http://" + host + ":" + port
}
