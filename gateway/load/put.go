package load

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"
)

// PutGCS uploads object bytes to fake-gcs or the JSON API media endpoint.
func PutGCS(ctx context.Context, gsURI string, contentType string, data []byte) error {
	bucket, object, err := parseGSURI(gsURI)
	if err != nil {
		return err
	}
	base := storageEmulatorBase()
	uploadURL := fmt.Sprintf("%s/upload/storage/v1/b/%s/o?uploadType=media&name=%s",
		base, url.PathEscape(bucket), url.QueryEscape(object))

	req, err := http.NewRequestWithContext(ctx, http.MethodPost, uploadURL, bytes.NewReader(data))
	if err != nil {
		return err
	}
	if contentType == "" {
		contentType = "application/octet-stream"
	}
	req.Header.Set("Content-Type", contentType)
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("put %s: %w", gsURI, err)
	}
	defer func() { _ = resp.Body.Close() }()
	if resp.StatusCode != http.StatusOK && resp.StatusCode != http.StatusCreated {
		body, _ := io.ReadAll(io.LimitReader(resp.Body, 512))
		return fmt.Errorf("put %s: HTTP %d: %s", gsURI, resp.StatusCode, strings.TrimSpace(string(body)))
	}
	return nil
}

func parseGSURI(gsURI string) (bucket, object string, err error) {
	rest := strings.TrimPrefix(gsURI, "gs://")
	slash := strings.Index(rest, "/")
	if slash <= 0 || slash == len(rest)-1 {
		return "", "", fmt.Errorf("invalid gs:// uri: %q", gsURI)
	}
	return rest[:slash], rest[slash+1:], nil
}
