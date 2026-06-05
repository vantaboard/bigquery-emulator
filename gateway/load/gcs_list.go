package load

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"
)

// ExpandSourceURIs resolves gs:// wildcards by listing objects from GCS.
// URIs without a '*' pass through unchanged.
func ExpandSourceURIs(ctx context.Context, uris []string) ([]string, error) {
	var out []string
	for _, uri := range uris {
		if !strings.Contains(uri, "*") {
			out = append(out, uri)
			continue
		}
		expanded, err := expandWildcardURI(ctx, uri)
		if err != nil {
			return nil, err
		}
		out = append(out, expanded...)
	}
	return out, nil
}

func expandWildcardURI(ctx context.Context, gsURI string) ([]string, error) {
	bucket, objectPattern, err := splitGSURI(gsURI)
	if err != nil {
		return nil, err
	}
	before, after, ok := strings.Cut(objectPattern, "*")
	if !ok {
		return []string{gsURI}, nil
	}
	prefix := before
	suffix := after

	names, err := ListGCSObjects(ctx, bucket, prefix)
	if err != nil {
		return nil, err
	}
	var matches []string
	for _, name := range names {
		if !matchesGCSWildcard(name, prefix, suffix) {
			continue
		}
		matches = append(matches, fmt.Sprintf("gs://%s/%s", bucket, name))
	}
	if len(matches) == 0 {
		return nil, fmt.Errorf("no objects matched sourceUri %q", gsURI)
	}
	return matches, nil
}

func matchesGCSWildcard(name, prefix, suffix string) bool {
	if !strings.HasPrefix(name, prefix) {
		return false
	}
	if strings.HasSuffix(name, "/") {
		return false
	}
	rest := name[len(prefix):]
	if suffix != "" {
		if !strings.HasSuffix(rest, suffix) {
			return false
		}
		rest = rest[:len(rest)-len(suffix)]
	}
	return rest != ""
}

// ListGCSObjects returns object names under bucket with the given prefix.
func ListGCSObjects(ctx context.Context, bucket, prefix string) ([]string, error) {
	base := storageEmulatorBase()
	var names []string
	pageToken := ""
	for {
		q := url.Values{
			"prefix":     {prefix},
			"maxResults": {"1000"},
		}
		if pageToken != "" {
			q.Set("pageToken", pageToken)
		}
		listURL := fmt.Sprintf("%s/storage/v1/b/%s/o?%s",
			base, url.PathEscape(bucket), q.Encode())

		req, err := http.NewRequestWithContext(ctx, http.MethodGet, listURL, nil)
		if err != nil {
			return nil, err
		}
		resp, err := http.DefaultClient.Do(req)
		if err != nil {
			return nil, fmt.Errorf("list gs://%s/%s: %w", bucket, prefix, err)
		}
		body, err := io.ReadAll(resp.Body)
		_ = resp.Body.Close()
		if err != nil {
			return nil, err
		}
		if resp.StatusCode != http.StatusOK {
			return nil, fmt.Errorf("list gs://%s/%s: HTTP %d: %s",
				bucket, prefix, resp.StatusCode, strings.TrimSpace(string(body)))
		}

		var page struct {
			Items []struct {
				Name string `json:"name"`
			} `json:"items"`
			NextPageToken string `json:"nextPageToken"`
		}
		if err := json.Unmarshal(body, &page); err != nil {
			return nil, fmt.Errorf("decode list response: %w", err)
		}
		for _, item := range page.Items {
			if item.Name != "" {
				names = append(names, item.Name)
			}
		}
		pageToken = page.NextPageToken
		if pageToken == "" {
			break
		}
	}
	return names, nil
}

func splitGSURI(gsURI string) (bucket, object string, err error) {
	if !strings.HasPrefix(gsURI, "gs://") {
		return "", "", fmt.Errorf("invalid gs:// uri: %q", gsURI)
	}
	rest := strings.TrimPrefix(gsURI, "gs://")
	slash := strings.Index(rest, "/")
	if slash <= 0 {
		return "", "", fmt.Errorf("invalid gs:// uri: %q", gsURI)
	}
	return rest[:slash], rest[slash+1:], nil
}

// ObjectPathFromURI returns the object path within its bucket.
func ObjectPathFromURI(gsURI string) (string, error) {
	_, object, err := splitGSURI(gsURI)
	return object, err
}

// BucketFromURI returns the bucket name from a gs:// URI.
func BucketFromURI(gsURI string) (string, error) {
	bucket, _, err := splitGSURI(gsURI)
	return bucket, err
}
