package seed

import (
	"errors"
	"strings"
	"testing"
)

// gwDefaultProject is the canonical "gateway-supplied default
// project id" the resolver tests use. Pulled into a constant so a
// test reader doesn't have to grep for the same literal across
// three subtests. The shared "frequently-repeated" literals
// (project/dataset triples, env values) live in
// testconsts_test.go; this one stays local because it's only
// touched here.
const gwDefaultProject = "gw-default"

// TestSeedRequest_ValidateAccepts pins the canonical valid request
// shapes the orchestrator dispatches on (project scope, dataset
// scope, single-table scope).
func TestSeedRequest_ValidateAccepts(t *testing.T) {
	cases := []struct {
		name string
		req  SeedRequest
	}{
		{
			name: "project-scope",
			req:  SeedRequest{Source: SeedEndpointRef{Project: "p"}},
		},
		{
			name: "dataset-scope",
			req:  SeedRequest{Source: SeedEndpointRef{Project: "p", Dataset: "d"}},
		},
		{
			name: "table-scope",
			req: SeedRequest{
				Source: SeedEndpointRef{Project: "p", Dataset: "d", Table: "t"},
			},
		},
		{
			name: "dataset-remap",
			req: SeedRequest{
				Source:      SeedEndpointRef{Project: "p", Dataset: "d"},
				Destination: &SeedDestinationRef{Dataset: "d2"},
			},
		},
		{
			name: "with-max-rows",
			req: SeedRequest{
				Source:          SeedEndpointRef{Project: "p", Dataset: "d", Table: "t"},
				MaxRowsPerTable: 100,
			},
		},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			if err := c.req.Validate(); err != nil {
				t.Fatalf("Validate: %v", err)
			}
		})
	}
}

// TestSeedRequest_ValidateRejects locks the invariants the
// orchestrator depends on. The wording isn't asserted (would be
// brittle) but the sentinel error class is.
func TestSeedRequest_ValidateRejects(t *testing.T) {
	cases := []struct {
		name string
		req  *SeedRequest
		want string
	}{
		{literalNil, nil, literalNil},
		{"no-project", &SeedRequest{}, "project"},
		{"table-without-dataset", &SeedRequest{
			Source: SeedEndpointRef{Project: "p", Table: "t"},
		}, "dataset"},
		{"dest-table-without-source-table", &SeedRequest{
			Source:      SeedEndpointRef{Project: "p", Dataset: "d"},
			Destination: &SeedDestinationRef{Table: "x"},
		}, "destination.table"},
		{"dest-dataset-without-source-dataset", &SeedRequest{
			Source:      SeedEndpointRef{Project: "p"},
			Destination: &SeedDestinationRef{Dataset: "d"},
		}, "destination.dataset"},
		{"negative-max", &SeedRequest{
			Source:          SeedEndpointRef{Project: "p"},
			MaxRowsPerTable: -1,
		}, "maxRowsPerTable"},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			err := c.req.Validate()
			if err == nil {
				t.Fatal("Validate returned nil; want error")
			}
			if !errors.Is(err, ErrInvalidRequest) {
				t.Errorf("error not wrapping ErrInvalidRequest: %v", err)
			}
			if !strings.Contains(err.Error(), c.want) {
				t.Errorf("err=%q does not mention %q", err, c.want)
			}
		})
	}
}

// TestResolveBillingProject_FallbackOrder pins the documented
// project resolution chain. Each case turns on one fallback at a
// time so a regression points at the specific level that broke.
// Walks every documented fallback level: request, gateway default,
// three env vars, source project.
//
//nolint:funlen // table-driven test that walks every documented fallback level
func TestResolveBillingProject_FallbackOrder(t *testing.T) {
	srcProject := "src-proj"
	base := SeedRequest{Source: SeedEndpointRef{Project: srcProject}}

	emptyEnv := func(string) (string, bool) { return "", false }
	env := func(m map[string]string) func(string) (string, bool) {
		return func(k string) (string, bool) {
			v, ok := m[k]
			return v, ok
		}
	}

	cases := []struct {
		name           string
		mutateReq      func(*SeedRequest)
		gatewayDefault string
		getenv         func(string) (string, bool)
		want           string
	}{
		{
			name: "request-billingProject-wins",
			mutateReq: func(r *SeedRequest) {
				r.BillingProject = "req-bill"
			},
			gatewayDefault: gwDefaultProject,
			getenv: env(map[string]string{
				EnvGoogleCloudQuotaProject: envQuotaProject,
			}),
			want: "req-bill",
		},
		{
			name:           "gateway-default-beats-env",
			gatewayDefault: gwDefaultProject,
			getenv: env(map[string]string{
				EnvGoogleCloudQuotaProject: envQuotaProject,
				EnvGoogleCloudProject:      envGCloudProject,
			}),
			want: gwDefaultProject,
		},
		{
			name: "quota-env-used-when-flag-empty",
			getenv: env(map[string]string{
				EnvGoogleCloudQuotaProject: envQuotaProject,
				EnvGoogleCloudProject:      envGCloudProject,
			}),
			want: envQuotaProject,
		},
		{
			name: "gcp-env-when-quota-empty",
			getenv: env(map[string]string{
				EnvGoogleCloudProject: envGCloudProject,
				EnvGcloudProject:      envGCloudLegacy,
			}),
			want: envGCloudProject,
		},
		{
			name: "gcloud-env-fallback",
			getenv: env(map[string]string{
				EnvGcloudProject: envGCloudLegacy,
			}),
			want: envGCloudLegacy,
		},
		{
			name:   "source-project-final-fallback",
			getenv: emptyEnv,
			want:   srcProject,
		},
		{
			name:   "nil-env-still-falls-back-to-source",
			getenv: nil,
			want:   srcProject,
		},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			req := base
			if c.mutateReq != nil {
				c.mutateReq(&req)
			}
			got := ResolveBillingProject(req, c.gatewayDefault, c.getenv)
			if got != c.want {
				t.Errorf("ResolveBillingProject=%q, want %q", got, c.want)
			}
		})
	}
}

// TestDecodeRequest_RejectsUnknownFields pins the strict-decode
// posture: a typo like "billing_project" (snake_case) is not
// silently dropped because the orchestrator would otherwise charge
// the seed against the wrong project.
func TestDecodeRequest_RejectsUnknownFields(t *testing.T) {
	_, err := DecodeRequest([]byte(`{"source":{"project":"p"},"billing_project":"x"}`))
	if err == nil {
		t.Fatal("DecodeRequest accepted unknown field; should reject typos")
	}
	if !errors.Is(err, ErrInvalidRequest) {
		t.Errorf("error not wrapping ErrInvalidRequest: %v", err)
	}
}

// TestDecodeRequest_HappyPath confirms a well-formed body decodes
// into the expected SeedRequest. Pin only the fields the rest of
// the package consumes; the rest are wire-tested by the handler
// tests.
func TestDecodeRequest_HappyPath(t *testing.T) {
	req, err := DecodeRequest([]byte(`{
		"source":{"project":"p","dataset":"d","table":"t"},
		"destination":{"dataset":"d2","table":"t2"},
		"maxRowsPerTable":42,
		"billingProject":"bill"
	}`))
	if err != nil {
		t.Fatalf("DecodeRequest: %v", err)
	}
	if req.Source.Project != "p" || req.Source.Dataset != "d" || req.Source.Table != "t" {
		t.Errorf("source = %+v", req.Source)
	}
	if req.Destination == nil || req.Destination.Dataset != "d2" || req.Destination.Table != "t2" {
		t.Errorf("destination = %+v", req.Destination)
	}
	if req.MaxRowsPerTable != 42 {
		t.Errorf("maxRowsPerTable = %d", req.MaxRowsPerTable)
	}
	if req.BillingProject != "bill" {
		t.Errorf("billingProject = %q", req.BillingProject)
	}
}
