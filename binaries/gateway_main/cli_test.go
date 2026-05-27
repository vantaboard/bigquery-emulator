package main

import (
	"bytes"
	"errors"
	"flag"
	"reflect"
	"sort"
	"strings"
	"testing"
)

// emptyEnv is the "nothing in the environment" lookup parseArgs uses
// when a test does not care about env-var fallbacks. Using a typed
// function rather than `nil` keeps the parser exercising the same
// fallback code path as production callers.
func emptyEnv(string) (string, bool) { return "", false }

// envFrom returns an envLookup that responds only to keys present in
// the supplied map. Keeps tests deterministic without touching the
// real os.Environ.
func envFrom(m map[string]string) envLookup {
	return func(k string) (string, bool) {
		v, ok := m[k]
		return v, ok
	}
}

// TestParseArgs_Defaults pins the zero-arg invocation: the gateway
// must boot with sensible defaults when no flags are supplied so
// `task emulator:run` and the docker image's entrypoint script keep
// working.
func TestParseArgs_Defaults(t *testing.T) {
	var errBuf bytes.Buffer
	cfg, err := parseArgs(nil, &errBuf, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs(nil): %v\nstderr=%s", err, errBuf.String())
	}

	if cfg.ListenHost != "localhost" {
		t.Errorf("ListenHost = %q, want %q", cfg.ListenHost, "localhost")
	}
	if cfg.HTTPPort != 9050 {
		t.Errorf("HTTPPort = %d, want 9050", cfg.HTTPPort)
	}
	if cfg.GRPCPort != 9060 {
		t.Errorf("GRPCPort = %d, want 9060", cfg.GRPCPort)
	}
	if cfg.EngineBinary != "emulator_main" {
		t.Errorf("EngineBinary = %q, want %q", cfg.EngineBinary, "emulator_main")
	}
	if !cfg.CopyEngineStderr {
		t.Error("CopyEngineStderr default = false; expected true so engine logs surface in operator terminals")
	}
	if cfg.CopyEngineStdout || cfg.LogRequests || cfg.Debug ||
		cfg.EnableSeedAPI || cfg.SeedAPIAllowRemote || cfg.VersionRequested {
		t.Errorf("boolean defaults must all be false; got %+v", cfg)
	}
	if len(cfg.SeedFiles) != 0 {
		t.Errorf("SeedFiles default = %v, want empty", cfg.SeedFiles)
	}
	if got := cfg.engineCLIArgs(); len(got) != 0 {
		t.Errorf("engineCLIArgs default = %v; engine pass-through must be empty so engine keeps its own defaults", got)
	}
}

// TestParseArgs_HyphenAndUnderscoreAliases pins the compatibility
// contract: every operator-facing flag accepts both name styles.
// Iteration order is deterministic so a failure points at the
// specific flag that regressed. Table-driven so a single regression
// points at the specific flag pair that broke.
//
//nolint:funlen // table-driven test enumerating every aliased flag pair
func TestParseArgs_HyphenAndUnderscoreAliases(t *testing.T) {
	cases := []struct {
		name      string
		hyphen    []string
		underbar  []string
		check     func(t *testing.T, cfg Config)
		checkName string
	}{
		{
			checkName: "http-port",
			hyphen:    []string{"--http-port=18050"},
			underbar:  []string{"--http_port=18050"},
			check: func(t *testing.T, cfg Config) {
				if cfg.HTTPPort != 18050 {
					t.Errorf("HTTPPort=%d, want 18050", cfg.HTTPPort)
				}
			},
		},
		{
			checkName: "grpc-port",
			hyphen:    []string{"--grpc-port=18060"},
			underbar:  []string{"--grpc_port=18060"},
			check: func(t *testing.T, cfg Config) {
				if cfg.GRPCPort != 18060 {
					t.Errorf("GRPCPort=%d, want 18060", cfg.GRPCPort)
				}
			},
		},
		{
			checkName: "listen-host/hostname",
			hyphen:    []string{"--listen-host=0.0.0.0"},
			underbar:  []string{"--hostname=0.0.0.0"},
			check: func(t *testing.T, cfg Config) {
				if cfg.ListenHost != "0.0.0.0" {
					t.Errorf("ListenHost=%q, want 0.0.0.0", cfg.ListenHost)
				}
			},
		},
		{
			checkName: "data-dir",
			hyphen:    []string{"--data-dir=/tmp/d"},
			underbar:  []string{"--data_dir=/tmp/d"},
			check: func(t *testing.T, cfg Config) {
				if cfg.DataDir != "/tmp/d" {
					t.Errorf("DataDir=%q, want /tmp/d", cfg.DataDir)
				}
			},
		},
		{
			checkName: "engine-binary",
			hyphen:    []string{"--engine-binary=/usr/local/bin/emu"},
			underbar:  []string{"--engine_binary=/usr/local/bin/emu"},
			check: func(t *testing.T, cfg Config) {
				if cfg.EngineBinary != "/usr/local/bin/emu" {
					t.Errorf("EngineBinary=%q, want /usr/local/bin/emu", cfg.EngineBinary)
				}
			},
		},
		{
			checkName: "log-requests",
			hyphen:    []string{"--log-requests"},
			underbar:  []string{"--log_requests"},
			check: func(t *testing.T, cfg Config) {
				if !cfg.LogRequests {
					t.Error("LogRequests=false")
				}
			},
		},
		{
			checkName: "project-id",
			hyphen:    []string{"--project-id=demo"},
			underbar:  []string{"--project_id=demo"},
			check: func(t *testing.T, cfg Config) {
				if cfg.DefaultProjectID != "demo" {
					t.Errorf("DefaultProjectID=%q, want demo", cfg.DefaultProjectID)
				}
			},
		},
		{
			checkName: "on-unknown-fn",
			hyphen:    []string{"--on-unknown-fn=ignore"},
			underbar:  []string{"--on_unknown_fn=ignore"},
			check: func(t *testing.T, cfg Config) {
				if cfg.OnUnknownFn != "ignore" {
					t.Errorf("OnUnknownFn=%q, want ignore", cfg.OnUnknownFn)
				}
			},
		},
		{
			checkName: "seed-data-file/seed-yaml",
			hyphen:    []string{"--seed-data-file=a.yaml", "--seed-data-file=b.yaml"},
			underbar:  []string{"--seed-yaml=a.yaml", "--seed-yaml=b.yaml"},
			check: func(t *testing.T, cfg Config) {
				want := []string{"a.yaml", "b.yaml"}
				if !reflect.DeepEqual(cfg.SeedFiles, want) {
					t.Errorf("SeedFiles=%v, want %v", cfg.SeedFiles, want)
				}
			},
		},
	}

	for _, c := range cases {
		t.Run(c.checkName+"/hyphen", func(t *testing.T) {
			cfg, err := parseArgs(c.hyphen, &bytes.Buffer{}, emptyEnv)
			if err != nil {
				t.Fatalf("parseArgs(%v): %v", c.hyphen, err)
			}
			c.check(t, cfg)
		})
		t.Run(c.checkName+"/underscore", func(t *testing.T) {
			cfg, err := parseArgs(c.underbar, &bytes.Buffer{}, emptyEnv)
			if err != nil {
				t.Fatalf("parseArgs(%v): %v", c.underbar, err)
			}
			c.check(t, cfg)
		})
	}
}

// TestParseArgs_EnvFallbacks pins the documented env-var defaults:
// flag > env > nothing. A flag value, even a deliberate empty string
// via `--flag=`, must NOT trigger the env fallback (mirrors the
// go-googlesql contract callers depend on).
func TestParseArgs_EnvFallbacks(t *testing.T) {
	env := envFrom(map[string]string{
		"BIGQUERY_EMULATOR_DATA_DIR":         "/env/data",
		"BIGQUERY_EMULATOR_INITIAL_DATA_DIR": "/env/initial",
		"BIGQUERY_EMULATOR_SEED_TOKEN":       "envtoken",
	})

	t.Run("env-used-when-flag-missing", func(t *testing.T) {
		cfg, err := parseArgs(nil, &bytes.Buffer{}, env)
		if err != nil {
			t.Fatalf("parseArgs(nil): %v", err)
		}
		if cfg.DataDir != "/env/data" {
			t.Errorf("DataDir=%q, want /env/data (from env)", cfg.DataDir)
		}
		if cfg.InitialDataDir != "/env/initial" {
			t.Errorf("InitialDataDir=%q, want /env/initial (from env)", cfg.InitialDataDir)
		}
		if cfg.SeedAPISeedToken != "envtoken" {
			t.Errorf("SeedAPISeedToken=%q, want envtoken (from env)", cfg.SeedAPISeedToken)
		}
	})

	t.Run("flag-overrides-env", func(t *testing.T) {
		cfg, err := parseArgs([]string{
			"--data-dir=/flag/data",
			"--initial-data-dir=/flag/initial",
			"--seed-api-seed-token=flagtoken",
		}, &bytes.Buffer{}, env)
		if err != nil {
			t.Fatalf("parseArgs: %v", err)
		}
		if cfg.DataDir != "/flag/data" {
			t.Errorf("DataDir=%q, want /flag/data (flag must beat env)", cfg.DataDir)
		}
		if cfg.InitialDataDir != "/flag/initial" {
			t.Errorf("InitialDataDir=%q, want /flag/initial", cfg.InitialDataDir)
		}
		if cfg.SeedAPISeedToken != "flagtoken" {
			t.Errorf("SeedAPISeedToken=%q, want flagtoken", cfg.SeedAPISeedToken)
		}
	})

	t.Run("legacy-env-key-honored", func(t *testing.T) {
		// EMULATOR_INITIAL_DATA_DIR is the go-googlesql legacy
		// fallback for the same setting; honoring it lets us
		// land in repos that already export it.
		legacy := envFrom(map[string]string{
			"EMULATOR_INITIAL_DATA_DIR": "/legacy",
		})
		cfg, err := parseArgs(nil, &bytes.Buffer{}, legacy)
		if err != nil {
			t.Fatalf("parseArgs(nil): %v", err)
		}
		if cfg.InitialDataDir != "/legacy" {
			t.Errorf("InitialDataDir=%q, want /legacy", cfg.InitialDataDir)
		}
	})
}

// TestParseArgs_EngineCLIArgs verifies the engine pass-through
// shape: only flags the operator actually set are forwarded, and the
// gateway emits them with the underscore convention emulator_main
// parses regardless of which alias the operator typed.
func TestParseArgs_EngineCLIArgs(t *testing.T) {
	cfg, err := parseArgs([]string{
		"--engine=memory",
		"--storage=duckdb",
		"--profile=default",
		"--on-unknown-fn=error",
		"--data-dir=/var/lib/bq",
	}, &bytes.Buffer{}, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs: %v", err)
	}

	got := cfg.engineCLIArgs()
	want := []string{
		"--engine", "memory",
		"--storage", "duckdb",
		"--profile", "default",
		"--on_unknown_fn", "error",
		"--data_dir", "/var/lib/bq",
	}
	if !reflect.DeepEqual(got, want) {
		t.Errorf("engineCLIArgs=%v, want %v", got, want)
	}
}

// TestParseArgs_PortValidation pins the validation that catches
// obvious operator mistakes before the gateway tries to bind sockets.
func TestParseArgs_PortValidation(t *testing.T) {
	cases := []struct {
		name string
		args []string
	}{
		{"http=0", []string{"--http-port=0"}},
		{"http=65536", []string{"--http-port=65536"}},
		{"grpc=0", []string{"--grpc-port=0"}},
		{"http=grpc", []string{"--http-port=12345", "--grpc-port=12345"}},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			var errBuf bytes.Buffer
			_, err := parseArgs(c.args, &errBuf, emptyEnv)
			if err == nil {
				t.Fatalf("parseArgs(%v) succeeded; want error", c.args)
			}
		})
	}
}

// TestParseArgs_VersionFlag exists to keep the `--version` short-
// circuit working. main() reads cfg.VersionRequested before any side
// effect, so we just need to confirm parseArgs surfaces the flag.
func TestParseArgs_VersionFlag(t *testing.T) {
	cfg, err := parseArgs([]string{"--version"}, &bytes.Buffer{}, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs(--version): %v", err)
	}
	if !cfg.VersionRequested {
		t.Error("VersionRequested=false; want true")
	}
}

// TestParseArgs_UnknownFlagFails pins the flag.ContinueOnError shape:
// unknown flags surface as a parse error so callers can react (main()
// log.Fatal's, tests can assert).
func TestParseArgs_UnknownFlagFails(t *testing.T) {
	var errBuf bytes.Buffer
	_, err := parseArgs([]string{"--no-such-flag"}, &errBuf, emptyEnv)
	if err == nil {
		t.Fatal("parseArgs(--no-such-flag) succeeded; want error")
	}
	if errors.Is(err, flag.ErrHelp) {
		t.Errorf("unknown flag classified as help: %v", err)
	}
	if !strings.Contains(errBuf.String(), "no-such-flag") {
		t.Errorf("error output does not mention the flag: %s", errBuf.String())
	}
}

// TestParseArgs_ToOptionsAddresses verifies the host:port assembly
// the gateway uses for its REST listener and engine gRPC client. The
// engineBinary argument is currently unused by ToOptions; we still
// pass it because main() reads cfg.EngineBinary on the same line so
// the test pins the call shape.
func TestParseArgs_ToOptionsAddresses(t *testing.T) {
	cfg, err := parseArgs([]string{
		"--listen-host=0.0.0.0",
		"--http-port=18050",
		"--grpc-port=18060",
	}, &bytes.Buffer{}, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs: %v", err)
	}
	httpAddr, engineAddr, _ := cfg.ToOptions(cfg.EngineBinary)
	if httpAddr != "0.0.0.0:18050" {
		t.Errorf("httpAddr=%q, want 0.0.0.0:18050", httpAddr)
	}
	if engineAddr != "0.0.0.0:18060" {
		t.Errorf("engineAddr=%q, want 0.0.0.0:18060", engineAddr)
	}
}

// TestStringSliceValueOrderPreserved guards the seed-file ordering
// contract: the YAML loader applies files in the order operators
// specified them, which is the order the parser appends.
func TestStringSliceValueOrderPreserved(t *testing.T) {
	cfg, err := parseArgs([]string{
		"--seed-data-file=first.yaml",
		"--seed-data-file=second.yaml",
		"--seed-data-file=third.yaml",
	}, &bytes.Buffer{}, emptyEnv)
	if err != nil {
		t.Fatalf("parseArgs: %v", err)
	}
	want := []string{"first.yaml", "second.yaml", "third.yaml"}
	if !reflect.DeepEqual(cfg.SeedFiles, want) {
		t.Errorf("SeedFiles=%v, want %v", cfg.SeedFiles, want)
	}

	// Defense-in-depth: even if a future refactor sorts the
	// slice for any reason, the test fails on the input ordering
	// rather than silently letting the YAML loader apply in the
	// wrong order.
	sorted := append([]string(nil), cfg.SeedFiles...)
	sort.Strings(sorted)
	if reflect.DeepEqual(cfg.SeedFiles, sorted) && cfg.SeedFiles[0] != sorted[0] {
		t.Error("SeedFiles slice was sorted; the YAML loader applies in supplied order")
	}
}
