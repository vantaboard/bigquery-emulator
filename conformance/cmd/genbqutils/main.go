// genbqutils converts a bigquery-utils extractor JSON manifest into native
// conformance YAML fixtures under conformance/thirdparty-fixtures/bigquery_utils/.
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
	"gopkg.in/yaml.v3"
)

type manifestCase struct {
	Inputs         []string `json:"inputs,omitempty"`
	ExpectedOutput string   `json:"expected_output"`
	InputColumns   []string `json:"input_columns,omitempty"`
	InputRows      string   `json:"input_rows,omitempty"`
}

type manifestUDF struct {
	Family            string         `json:"family"`
	Name              string         `json:"name"`
	Kind              string         `json:"kind,omitempty"`
	UpstreamSQLX      string         `json:"upstream_sqlx"`
	UpstreamTestCases string         `json:"upstream_test_cases"`
	CreateSQL         string         `json:"create_sql"`
	Cases             []manifestCase `json:"cases"`
}

type manifest struct {
	SourceSHA string        `json:"source_sha"`
	Emitted   []manifestUDF `json:"emitted"`
	Skipped   []struct {
		Family string `json:"family"`
		Name   string `json:"name"`
		Reason string `json:"reason"`
	} `json:"skipped"`
}

var nonAlnum = regexp.MustCompile(`[^a-z0-9]+`)

func main() {
	outDir := flag.String(
		"out-dir",
		"conformance/thirdparty-fixtures/bigquery_utils/known_failing",
		"output root (wiped each run)",
	)
	flag.Parse()

	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		fatal("read stdin: %v", err)
	}
	var m manifest
	if unmarshalErr := json.Unmarshal(data, &m); unmarshalErr != nil {
		fatal("parse manifest: %v", unmarshalErr)
	}

	root, err := repoRoot()
	if err != nil {
		fatal("%v", err)
	}
	absOut := *outDir
	if !filepath.IsAbs(absOut) {
		absOut = filepath.Join(root, absOut)
	}

	if err := wipeDir(absOut); err != nil {
		fatal("wipe %s: %v", absOut, err)
	}

	for _, udf := range m.Emitted {
		if err := writeFixture(root, absOut, m.SourceSHA, udf); err != nil {
			fatal("write %s/%s: %v", udf.Family, udf.Name, err)
		}
	}

	fmt.Fprintf(os.Stderr, "genbqutils: wrote %d fixtures to %s\n", len(m.Emitted), absOut)
}

func repoRoot() (string, error) {
	wd, err := os.Getwd()
	if err != nil {
		return "", err
	}
	dir := wd
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", fmt.Errorf("could not find repo root from %s", wd)
		}
		dir = parent
	}
}

func wipeDir(dir string) error {
	if err := os.RemoveAll(dir); err != nil {
		return err
	}
	return os.MkdirAll(dir, 0o750)
}

func fixtureName(family, name string) string {
	parts := []string{"bqutils"}
	for seg := range strings.SplitSeq(family, "/") {
		if seg != "" {
			parts = append(parts, seg)
		}
	}
	parts = append(parts, name)
	raw := strings.Join(parts, "_")
	return nonAlnum.ReplaceAllString(strings.ToLower(raw), "_")
}

func projectID(name string) string {
	slug := nonAlnum.ReplaceAllString(strings.ToLower(name), "-")
	slug = strings.Trim(slug, "-")
	if slug == "" {
		slug = "udf"
	}
	return "proj-bqutils-" + slug
}

func buildQuery(udf manifestUDF) string {
	if udf.Kind == "udaf" {
		return buildUdafQuery(udf)
	}
	var b strings.Builder
	b.WriteString("WITH cases AS (\n")
	for i, tc := range udf.Cases {
		if i > 0 {
			b.WriteString("  UNION ALL\n")
		}
		args := strings.Join(tc.Inputs, ", ")
		fmt.Fprintf(&b, "  SELECT %d AS case_id, TO_JSON_STRING(%s(%s)) AS actual, TO_JSON_STRING(%s) AS expected\n",
			i, udf.Name, args, tc.ExpectedOutput)
	}
	b.WriteString(")\n")
	// NULL = NULL is UNKNOWN in SQL; treat two NULL JSON strings as equal.
	b.WriteString(
		"SELECT case_id, IFNULL(actual = expected, actual IS NULL AND expected IS NULL) AS matches FROM cases ORDER BY case_id\n",
	)
	return b.String()
}

func buildUdafQuery(udf manifestUDF) string {
	var b strings.Builder
	b.WriteString("WITH cases AS (\n")
	for i, tc := range udf.Cases {
		if i > 0 {
			b.WriteString("  UNION ALL\n")
		}
		var aggCols []string
		var udafArgs []string
		aggIdx := 0
		for _, col := range tc.InputColumns {
			if strings.Contains(col, " NOT AGGREGATE") {
				lit := strings.TrimSpace(strings.Split(col, " NOT AGGREGATE")[0])
				udafArgs = append(udafArgs, lit)
				continue
			}
			alias := fmt.Sprintf("test_input_%d", aggIdx)
			aggCols = append(aggCols, fmt.Sprintf("%s AS %s", col, alias))
			udafArgs = append(udafArgs, alias)
			aggIdx++
		}
		fromClause := tc.InputRows
		if len(aggCols) > 0 {
			fromClause = fmt.Sprintf("SELECT %s FROM (%s)", strings.Join(aggCols, ", "), tc.InputRows)
		}
		fmt.Fprintf(&b,
			"  SELECT %d AS case_id, TO_JSON_STRING(%s(%s)) AS actual, TO_JSON_STRING(%s) AS expected\n  FROM (%s)\n",
			i, udf.Name, strings.Join(udafArgs, ", "), tc.ExpectedOutput, fromClause)
	}
	b.WriteString(")\n")
	b.WriteString(
		"SELECT case_id, IFNULL(actual = expected, actual IS NULL AND expected IS NULL) AS matches FROM cases ORDER BY case_id\n",
	)
	return b.String()
}

func kindLabel(kind string) string {
	if kind == "udaf" {
		return "UDAF"
	}
	return "UDF"
}

func buildFixture(udf manifestUDF) runner.Fixture {
	rows := make([]map[string]any, len(udf.Cases))
	for i := range udf.Cases {
		rows[i] = map[string]any{
			"case_id": strconv.Itoa(i),
			"matches": true,
		}
	}
	return runner.Fixture{
		Name: fixtureName(udf.Family, udf.Name),
		Description: fmt.Sprintf(
			"bigquery-utils %s %s %s (%d cases)",
			udf.Family,
			kindLabel(udf.Kind),
			udf.Name,
			len(udf.Cases),
		),
		Profiles:  []string{runner.ProfileDuckDB},
		ProjectID: projectID(udf.Name),
		Setup: []runner.SetupStep{
			{SQL: strings.TrimSpace(udf.CreateSQL)},
		},
		Query: buildQuery(udf),
		Expected: runner.Expectation{
			Match: runner.MatchOrdered,
			Rows:  rows,
		},
	}
}

func provenanceHeader(sha string, udf manifestUDF) string {
	if sha == "" {
		sha = "unknown"
	}
	return fmt.Sprintf(
		"# Source: GoogleCloudPlatform/bigquery-utils @ %s\n"+
			"#   %s (+ %s)\n"+
			"# License: Apache-2.0. Generated by scripts/sync_bigquery_utils_udfs.sh; do not edit by hand.\n",
		sha, udf.UpstreamSQLX, filepath.Base(udf.UpstreamTestCases),
	)
}

func marshalFixture(f runner.Fixture) ([]byte, error) {
	var body strings.Builder
	enc := yaml.NewEncoder(&body)
	enc.SetIndent(2)
	if err := enc.Encode(&f); err != nil {
		return nil, err
	}
	if err := enc.Close(); err != nil {
		return nil, err
	}
	return []byte(body.String()), nil
}

func writeFixture(repoRoot, outRoot, sha string, udf manifestUDF) error {
	f := buildFixture(udf)
	body, err := marshalFixture(f)
	if err != nil {
		return err
	}

	outPath := filepath.Join(outRoot, udf.Family, udf.Name+".yaml")
	if mkdirErr := os.MkdirAll(filepath.Dir(outPath), 0o750); mkdirErr != nil {
		return mkdirErr
	}

	var out strings.Builder
	out.WriteString(provenanceHeader(sha, udf))
	out.Write(body)
	content := []byte(out.String())

	// Round-trip through the runner loader so schema drift fails fast.
	tmp, err := os.CreateTemp(repoRoot, ".tmp-genbqutils-*.yaml")
	if err != nil {
		return err
	}
	tmpPath := tmp.Name()
	defer func() {
		_ = os.Remove(tmpPath)
	}()
	if _, err := tmp.Write(content); err != nil {
		_ = tmp.Close()
		return err
	}
	if err := tmp.Close(); err != nil {
		return err
	}
	if _, err := runner.Load(tmpPath); err != nil {
		return fmt.Errorf("runner.Load: %w", err)
	}

	return os.WriteFile(outPath, content, 0o600)
}

func fatal(format string, args ...any) {
	fmt.Fprintf(os.Stderr, "genbqutils: "+format+"\n", args...)
	os.Exit(1)
}
