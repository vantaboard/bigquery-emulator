package googlesqlcorpus

import (
	"fmt"
	"strings"
)

// TestFile is one vendored GoogleSQL compliance .test file.
type TestFile struct {
	Path     string
	Defaults FileDefaults
	Cases    []TestCase
}

// FileDefaults captures file-level directives such as
// [default required_features=...].
type FileDefaults struct {
	RequiredFeatures []string
}

// TestCase is one statement/expected-result pair from a .test file.
type TestCase struct {
	File             string
	Name             string
	RequiredFeatures []string
	PrepareDatabase  bool
	SQL              string
	Expected         ExpectedResult
	ExpectError      string
	Line             int // 1-based line of the case's first directive
}

// ParseFile splits a byte-identical upstream .test file into cases.
func ParseFile(path string, content string) (*TestFile, error) {
	blocks := splitTestBlocks(content)
	out := &TestFile{Path: path}
	fileDefaults := FileDefaults{}

	for _, block := range blocks {
		block = strings.TrimSpace(block)
		if block == "" {
			continue
		}
		meta, body, err := splitMetaAndBody(block)
		if err != nil {
			return nil, fmt.Errorf("%s: %w", path, err)
		}
		if len(meta.Defaults.RequiredFeatures) > 0 {
			fileDefaults = mergeDefaults(fileDefaults, meta.Defaults)
			out.Defaults = fileDefaults
		}
		if strings.TrimSpace(body) == "" {
			continue
		}
		tc, err := parseCase(path, meta, body, fileDefaults)
		if err != nil {
			return nil, err
		}
		out.Cases = append(out.Cases, tc)
	}
	return out, nil
}

func splitTestBlocks(content string) []string {
	lines := strings.Split(content, "\n")
	var blocks []string
	var cur strings.Builder
	for _, line := range lines {
		if strings.TrimSpace(line) == "==" {
			blocks = append(blocks, cur.String())
			cur.Reset()
			continue
		}
		if cur.Len() > 0 {
			cur.WriteByte('\n')
		}
		cur.WriteString(line)
	}
	if tail := strings.TrimSpace(cur.String()); tail != "" {
		blocks = append(blocks, tail)
	}
	return blocks
}

type blockMeta struct {
	Name             string
	RequiredFeatures []string
	PrepareDatabase  bool
	Defaults         FileDefaults
	Line             int
}

func splitMetaAndBody(block string) (blockMeta, string, error) {
	lines := strings.Split(block, "\n")
	meta := blockMeta{Line: 1}
	var bodyLines []string
	inBody := false
	for i, line := range lines {
		trim := strings.TrimSpace(line)
		if !inBody && strings.HasPrefix(trim, "[") && strings.HasSuffix(trim, "]") {
			if err := applyDirective(trim, &meta); err != nil {
				return blockMeta{}, "", fmt.Errorf("line %d: %w", i+1, err)
			}
			if meta.Line == 1 {
				meta.Line = i + 1
			}
			continue
		}
		if strings.TrimSpace(line) == "" && !inBody && len(bodyLines) == 0 {
			continue
		}
		inBody = true
		bodyLines = append(bodyLines, line)
	}
	return meta, strings.Join(bodyLines, "\n"), nil
}

func applyDirective(directive string, meta *blockMeta) error {
	inner := strings.TrimSuffix(strings.TrimPrefix(directive, "["), "]")
	if after, ok := strings.CutPrefix(inner, "default "); ok {
		key, val, ok := strings.Cut(after, "=")
		if !ok {
			return fmt.Errorf("invalid default directive %q", directive)
		}
		switch strings.TrimSpace(key) {
		case "required_features":
			meta.Defaults.RequiredFeatures = splitCSV(val)
		default:
			return fmt.Errorf("unsupported default directive %q", key)
		}
		return nil
	}
	key, val, ok := strings.Cut(inner, "=")
	if !ok {
		key = inner
		val = ""
	}
	switch strings.TrimSpace(key) {
	case "name":
		meta.Name = strings.TrimSpace(val)
	case "required_features":
		meta.RequiredFeatures = splitCSV(val)
	case "prepare_database":
		meta.PrepareDatabase = true
	case "load_proto_files", "load_proto_names", "load_enum_names",
		"parameters", "labels", "forbidden_features":
		// Parsed for triage; runner skips cases that need these today.
		return nil
	default:
		// Unknown directives are ignored so upstream additions do not
		// break the parser; triage buckets them later if needed.
		return nil
	}
	return nil
}

func mergeDefaults(cur, add FileDefaults) FileDefaults {
	if len(add.RequiredFeatures) > 0 {
		cur.RequiredFeatures = add.RequiredFeatures
	}
	return cur
}

func parseCase(path string, meta blockMeta, body string, defaults FileDefaults) (TestCase, error) {
	sep := "\n--\n"
	idx := strings.Index(body, sep)
	if idx < 0 {
		sep = "--"
		idx = strings.Index(body, sep)
	}
	if idx < 0 {
		return TestCase{}, fmt.Errorf("%s case %q: missing -- separator", path, meta.Name)
	}
	sql := strings.TrimSpace(body[:idx])
	expectedRaw := strings.TrimSpace(body[idx+len(sep):])
	if sql == "" {
		return TestCase{}, fmt.Errorf("%s case %q: empty SQL", path, meta.Name)
	}
	var exp ExpectedResult
	var expectErr string
	if after, ok := strings.CutPrefix(expectedRaw, "ERROR:"); ok {
		expectErr = strings.TrimSpace(after)
	} else {
		var err error
		exp, err = ParseExpected(expectedRaw)
		if err != nil {
			return TestCase{}, fmt.Errorf("%s case %q: parse expected: %w", path, meta.Name, err)
		}
	}
	features := append([]string{}, defaults.RequiredFeatures...)
	features = append(features, meta.RequiredFeatures...)
	return TestCase{
		File:             path,
		Name:             meta.Name,
		RequiredFeatures: dedupe(features),
		PrepareDatabase:  meta.PrepareDatabase,
		SQL:              sql,
		Expected:         exp,
		ExpectError:      expectErr,
		Line:             meta.Line,
	}, nil
}

func splitCSV(s string) []string {
	var out []string
	for part := range strings.SplitSeq(s, ",") {
		part = strings.TrimSpace(part)
		if part != "" {
			out = append(out, part)
		}
	}
	return out
}

func dedupe(in []string) []string {
	seen := make(map[string]bool, len(in))
	var out []string
	for _, v := range in {
		if seen[v] {
			continue
		}
		seen[v] = true
		out = append(out, v)
	}
	return out
}
