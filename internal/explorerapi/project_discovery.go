package explorerapi

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"sort"
	"strings"
	"time"
)

const (
	emulatorProjectsPath   = "/emulator/v1/projects"
	bigqueryV2ProjectsPath = "/bigquery/v2/projects"
)

// NormalizeEmulatorHost strips accidental schemes and trailing slashes for use in http://%s/...
func NormalizeEmulatorHost(host string) string {
	h := strings.TrimSpace(host)
	h = strings.TrimPrefix(h, "http://")
	h = strings.TrimPrefix(h, "https://")
	return strings.TrimSuffix(h, "/")
}

// FetchProjectIDsFromEmulator queries the Vantaboard fork's /emulator/v1/projects, then falls back to BigQuery REST /bigquery/v2/projects.
func FetchProjectIDsFromEmulator(ctx context.Context, httpClient *http.Client, emulatorHost string) ([]string, error) {
	if emulatorHost == "" {
		return nil, fmt.Errorf("emulator host is empty")
	}
	base := NormalizeEmulatorHost(emulatorHost)

	if httpClient == nil {
		httpClient = &http.Client{Timeout: 15 * time.Second}
	}

	ids, errV1 := getJSONStringSlice(ctx, httpClient, "http://"+base+emulatorProjectsPath)
	if errV1 == nil && len(ids) > 0 {
		return dedupeSorted(ids), nil
	}

	body, errV2 := getHTTPBody(ctx, httpClient, "http://"+base+bigqueryV2ProjectsPath)
	if errV2 != nil {
		if errV1 != nil {
			return nil, fmt.Errorf("emulator v1: %w; bigquery v2: %w", errV1, errV2)
		}
		return nil, fmt.Errorf("bigquery v2: %w", errV2)
	}
	ids, err := parseBigQueryV2ProjectListJSON(body)
	if err != nil {
		return nil, err
	}
	return dedupeSorted(ids), nil
}

func getJSONStringSlice(ctx context.Context, client *http.Client, rawURL string) ([]string, error) {
	body, err := getHTTPBody(ctx, client, rawURL)
	if err != nil {
		return nil, err
	}
	var ids []string
	if err := json.Unmarshal(body, &ids); err != nil {
		return nil, err
	}
	out := make([]string, 0, len(ids))
	for _, id := range ids {
		id = strings.TrimSpace(id)
		if id != "" {
			out = append(out, id)
		}
	}
	if len(out) == 0 {
		return nil, fmt.Errorf("empty project list")
	}
	return out, nil
}

func getHTTPBody(ctx context.Context, client *http.Client, rawURL string) ([]byte, error) {
	req, err := http.NewRequestWithContext(ctx, http.MethodGet, rawURL, nil)
	if err != nil {
		return nil, err
	}
	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	b, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		return nil, fmt.Errorf("GET %s: %s (%s)", rawURL, resp.Status, bytes.TrimSpace(b))
	}
	return b, nil
}

// parseBigQueryV2ProjectListJSON extracts project ids from GET .../bigquery/v2/projects JSON.
func parseBigQueryV2ProjectListJSON(data []byte) ([]string, error) {
	var root map[string]interface{}
	if err := json.Unmarshal(data, &root); err != nil {
		return nil, err
	}
	raw, ok := root["projects"]
	if !ok {
		raw = root["Projects"]
	}
	arr, ok := raw.([]interface{})
	if !ok || len(arr) == 0 {
		return nil, fmt.Errorf("no projects in response")
	}
	var ids []string
	for _, item := range arr {
		m, ok := item.(map[string]interface{})
		if !ok {
			continue
		}
		id := strings.TrimSpace(firstString(m, "id", "Id", "projectId", "ProjectId", "project_id"))
		if id != "" {
			ids = append(ids, id)
			continue
		}
		if ref, ok := m["projectReference"].(map[string]interface{}); ok {
			id = strings.TrimSpace(firstString(ref, "projectId", "ProjectId"))
			if id != "" {
				ids = append(ids, id)
			}
		}
	}
	if len(ids) == 0 {
		return nil, fmt.Errorf("could not parse project ids")
	}
	return ids, nil
}

func firstString(m map[string]interface{}, keys ...string) string {
	for _, k := range keys {
		if v, ok := m[k]; ok {
			if s, ok := v.(string); ok {
				return s
			}
		}
	}
	return ""
}

func dedupeSorted(ids []string) []string {
	seen := make(map[string]struct{}, len(ids))
	var out []string
	for _, id := range ids {
		id = strings.TrimSpace(id)
		if id == "" {
			continue
		}
		if _, ok := seen[id]; ok {
			continue
		}
		seen[id] = struct{}{}
		out = append(out, id)
	}
	sort.Strings(out)
	return out
}

// ApplyProjectIDListEnv merges or overrides discovered projects using BIGQUERY_PROJECT_IDS and BIGQUERY_PROJECT_IDS_MODE.
// MODE=override: only env list (if non-empty); otherwise merge env into discovered (union).
func ApplyProjectIDListEnv(discovered []string, defaultProject string) []string {
	raw := strings.TrimSpace(os.Getenv("BIGQUERY_PROJECT_IDS"))
	mode := strings.ToLower(strings.TrimSpace(os.Getenv("BIGQUERY_PROJECT_IDS_MODE")))

	envIDs := parseCommaSeparatedIDs(raw)
	if mode == "override" {
		if len(envIDs) == 0 {
			if defaultProject != "" {
				return []string{defaultProject}
			}
			return nil
		}
		return dedupeSorted(envIDs)
	}

	merged := append(append([]string{}, discovered...), envIDs...)
	if len(merged) == 0 && defaultProject != "" {
		return []string{defaultProject}
	}
	return dedupeSorted(merged)
}

func parseCommaSeparatedIDs(s string) []string {
	if s == "" {
		return nil
	}
	parts := strings.Split(s, ",")
	var out []string
	for _, p := range parts {
		p = strings.TrimSpace(p)
		if p != "" {
			out = append(out, p)
		}
	}
	return out
}

// PostEmulatorCreateProject POSTs {"id":...} to the emulator admin API.
func PostEmulatorCreateProject(ctx context.Context, httpClient *http.Client, emulatorHost, projectID string) error {
	base := NormalizeEmulatorHost(emulatorHost)
	u := "http://" + base + emulatorProjectsPath
	body := fmt.Sprintf(`{"id":%s}`, jsonString(projectID))
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, u, strings.NewReader(body))
	if err != nil {
		return err
	}
	req.Header.Set("Content-Type", "application/json")
	if httpClient == nil {
		httpClient = &http.Client{Timeout: 15 * time.Second}
	}
	resp, err := httpClient.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	b, _ := io.ReadAll(resp.Body)
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		return fmt.Errorf("POST %s: %s: %s", u, resp.Status, bytes.TrimSpace(b))
	}
	return nil
}

func jsonString(s string) string {
	b, _ := json.Marshal(s)
	return string(b)
}

// ValidateProjectIDForPath ensures :project_id is a single path segment (no slashes / traversal).
func ValidateProjectIDForPath(id string) bool {
	id = strings.TrimSpace(id)
	if id == "" {
		return false
	}
	if strings.Contains(id, "/") || strings.Contains(id, "..") {
		return false
	}
	if strings.Contains(id, "://") {
		return false
	}
	return true
}
