package seedfile

import (
	"path/filepath"
	"regexp"
	"slices"
	"strings"
)

// PublicDataProject is the BigQuery project id thirdparty samples use for
// public dataset queries.
const PublicDataProject = "bigquery-public-data"

// PublicDataSeedRelPath is the repo-relative path to the bundled YAML
// fixture. Docker copies it under /opt/bigquery-emulator/.
const PublicDataSeedRelPath = "testdata/public-data/bigquery-public-data.yaml"

// PublicDataSeedContainerPath is where the runtime image installs the
// fixture so gateway_main can pass --seed-data-file without host mounts.
const PublicDataSeedContainerPath = "/opt/bigquery-emulator/testdata/public-data/bigquery-public-data.yaml"

// SeededPublicTables lists project.dataset.table resources the bundled
// fixture materializes. Skip matrices (python emulator_pytest_skip,
// third_party/README.md) treat only these refs as emulator-backed.
var SeededPublicTables = []string{
	PublicDataProject + ".usa_names.usa_1910_2013",
	PublicDataProject + ".samples.shakespeare",
	PublicDataProject + ".stackoverflow.posts_questions",
	PublicDataProject + ".ml_datasets.penguins",
}

var publicTableRefRE = regexp.MustCompile(
	`bigquery-public-data[.:]([a-zA-Z0-9_]+)[.:]([a-zA-Z0-9_]+)`,
)

// PublicDataSeedPathFromRoot returns the absolute path to the bundled
// fixture given a repository root directory.
func PublicDataSeedPathFromRoot(repoRoot string) string {
	return filepath.Join(repoRoot, PublicDataSeedRelPath)
}

// PublicDataRefsInText returns normalized project.dataset.table refs
// found in SQL or sample source text.
func PublicDataRefsInText(text string) map[string]struct{} {
	out := make(map[string]struct{})
	for _, m := range publicTableRefRE.FindAllStringSubmatch(text, -1) {
		if len(m) < 3 {
			continue
		}
		ref := PublicDataProject + "." + m[1] + "." + m[2]
		out[ref] = struct{}{}
	}
	return out
}

// PublicDataRefsFullySeeded reports whether every bigquery-public-data
// table reference in text is covered by SeededPublicTables.
func PublicDataRefsFullySeeded(text string) bool {
	refs := PublicDataRefsInText(text)
	if len(refs) == 0 {
		return false
	}
	seeded := make(map[string]struct{}, len(SeededPublicTables))
	for _, t := range SeededPublicTables {
		seeded[t] = struct{}{}
	}
	for ref := range refs {
		if _, ok := seeded[ref]; !ok {
			return false
		}
	}
	return true
}

// IsSeededPublicTable returns true when ref is one of the bundled tables.
// ref may be project.dataset.table or dataset.table (project assumed).
func IsSeededPublicTable(ref string) bool {
	ref = strings.TrimSpace(ref)
	if !strings.HasPrefix(ref, PublicDataProject+".") {
		ref = PublicDataProject + "." + ref
	}
	return slices.Contains(SeededPublicTables, ref)
}
