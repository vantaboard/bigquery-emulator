// CLI parsing for the gateway_main binary.
//
// The parser lives in its own file so it can be exercised by unit
// tests (cli_test.go) without forking a process. The output is a
// normalized Config struct that main() turns into a gateway.Options.
//
// # Flag aliasing
//
// Every operator-facing flag accepts both the legacy
// underscore-separated name this repository started with
// (`--http_port`) and the hyphen-separated equivalent used by
// `go-googlesql`'s `bq-emulator` (`--http-port`). Both names target
// the same parsed value; whichever appears last on the command line
// wins, the same way Go's `flag` package handles late overrides for
// any single flag. This keeps existing scripts/Taskfiles working
// while letting operators copy invocation snippets straight from the
// upstream documentation.
//
// # Environment-variable fallbacks
//
// Three settings honor environment variables when the CLI flag is
// not supplied, matching go-googlesql's documented contract:
//
//   - BIGQUERY_EMULATOR_INITIAL_DATA_DIR (also EMULATOR_INITIAL_DATA_DIR)
//     populates --initial-data-dir.
//   - BIGQUERY_EMULATOR_SEED_TOKEN populates --seed-api-seed-token.
//   - BIGQUERY_EMULATOR_DATA_DIR populates --data-dir.
//
// All three are still overridable from the command line; the env
// vars are only consulted when the operator did not say anything.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io"
	"strconv"
	"strings"
)

// Config is the normalized result of parsing argv. All operator-facing
// CLI flags collapse to one field here so the rest of the program can
// read settings without juggling pointer indirection or alias bookkeeping.
type Config struct {
	// VersionRequested is true when `--version` was passed; main()
	// short-circuits before any side effect.
	VersionRequested bool

	// ListenHost is the host the HTTP gateway binds to. Maps to
	// `--listen-host` / `--hostname`.
	ListenHost string

	// HTTPPort is the BigQuery REST listener port. Maps to
	// `--http-port` / `--http_port`.
	HTTPPort int

	// GRPCPort is the engine gRPC port. The gateway also dials this
	// port via the loopback interface. Maps to `--grpc-port` /
	// `--grpc_port`.
	GRPCPort int

	// EngineBinary is the absolute or basename path to the C++ engine
	// subprocess. Empty disables the subprocess (gateway-only /
	// unit-test mode). Maps to `--engine-binary` / `--engine_binary`.
	EngineBinary string

	// DataDir is the persistent storage root. Forwarded to the engine
	// as `--data_dir`. Maps to `--data-dir` / `--data_dir` /
	// `BIGQUERY_EMULATOR_DATA_DIR`.
	DataDir string

	// InitialDataDir is a template directory the gateway copies into
	// DataDir on first start (when DataDir does not yet contain an
	// initialized catalog). Maps to `--initial-data-dir` /
	// `BIGQUERY_EMULATOR_INITIAL_DATA_DIR` / `EMULATOR_INITIAL_DATA_DIR`.
	InitialDataDir string

	// CopyEngineStdout / CopyEngineStderr forward the engine
	// subprocess's stdio to the gateway's. Maps to
	// `--copy-engine-stdout` / `--copy_engine_stdout` /
	// `--copy-engine-stderr` / `--copy_engine_stderr`.
	CopyEngineStdout bool
	CopyEngineStderr bool

	// LogRequests prints each REST request and response. Maps to
	// `--log-requests` / `--log_requests`.
	LogRequests bool

	// Debug enables verbose lifecycle logging. Maps to `--debug`.
	Debug bool

	// DefaultProjectID is the default project clients act against
	// when seeding or applying YAML data without an explicit project.
	// Maps to `--project-id` / `--project_id`.
	DefaultProjectID string

	// DefaultDatasetLocation is the BigQuery location stamped on
	// datasets created without an explicit location. Maps to
	// `--default-dataset-location`.
	DefaultDatasetLocation string

	// EnableSeedAPI registers the `POST /api/emulator/seed` route
	// and its operation polling endpoint. Off by default for local
	// safety.
	EnableSeedAPI bool

	// SeedAPIAllowRemote allows non-loopback callers to invoke the
	// seed API. Off by default.
	SeedAPIAllowRemote bool

	// SeedAPISeedToken, when non-empty, requires matching header
	// `X-BigQuery-Emulator-Seed-Token` on every seed API request.
	// Falls back to `BIGQUERY_EMULATOR_SEED_TOKEN` when the flag is
	// empty.
	SeedAPISeedToken string

	// SeedFiles is the repeatable list of YAML seed-data files to
	// apply once the engine is SERVING. Maps to `--seed-data-file`
	// / `--seed-yaml`.
	SeedFiles []string
}

// envLookup mirrors `os.LookupEnv` for tests. The default impl simply
// calls into the real env; tests inject a deterministic map so they
// don't depend on the running process's environment.
type envLookup func(key string) (string, bool)

// parseArgs builds a one-shot flag.FlagSet, registers every supported
// flag (with hyphen and underscore aliases), parses argv, and returns
// the resulting Config. errOut is where the FlagSet writes usage and
// error messages on parse failure -- pass os.Stderr in production,
// or a *bytes.Buffer in tests.
//
// The argv parameter does NOT include the program name; pass
// os.Args[1:] to mirror Go's `flag.Parse()` contract.
//
// Internally this composes three steps so the function stays
// readable: applyDefaults seeds the zero-value Config, registerFlags
// wires every supported flag (including hyphen/underscore aliases)
// onto a one-shot FlagSet, and applyEnvFallbacks fills in missing
// values from the operator's environment after the FlagSet wins.
func parseArgs(argv []string, errOut io.Writer, getenv envLookup) (Config, error) {
	if getenv == nil {
		// Test-friendly default that never reads the real env, so
		// parseArgs is deterministic when callers don't pass a
		// fixture explicitly.
		getenv = noEnv
	}

	cfg := defaultConfig()
	fs := flag.NewFlagSet("gateway_main", flag.ContinueOnError)
	fs.SetOutput(errOut)

	versionFlag := false
	registerFlags(fs, &cfg, &versionFlag)

	if err := fs.Parse(argv); err != nil {
		return Config{}, fmt.Errorf("parse flags: %w", err)
	}
	cfg.VersionRequested = versionFlag

	if err := validatePorts(cfg); err != nil {
		return Config{}, err
	}
	applyEnvFallbacks(&cfg, getenv)
	return cfg, nil
}

// noEnv is the stand-in environment lookup parseArgs uses when the
// caller passes nil; it always returns "no such variable" so
// parseArgs is deterministic during tests that don't care about env
// fallbacks.
func noEnv(string) (string, bool) { return "", false }

// defaultConfig returns the seed Config parseArgs starts from. Pulled
// out so the defaults are visible in one place (and so tests that
// drive parseArgs directly can assert against the same baseline).
func defaultConfig() Config {
	return Config{
		ListenHost:       "localhost",
		HTTPPort:         9050,
		GRPCPort:         9060,
		EngineBinary:     "emulator_main",
		CopyEngineStderr: true,
	}
}

// registerFlags wires every supported flag (including hyphen and
// underscore aliases) onto fs. Split out from parseArgs purely so
// the parser body stays under the funlen budget; nothing else
// invokes it.
func registerFlags(fs *flag.FlagSet, cfg *Config, versionFlag *bool) {
	registerString(fs, &cfg.ListenHost, []string{"listen-host", "hostname"},
		"Hostname for the emulator servers.")
	registerInt(fs, &cfg.HTTPPort, []string{"http-port", "http_port"},
		"Port on which to run the BigQuery REST gateway.")
	registerInt(fs, &cfg.GRPCPort, []string{"grpc-port", "grpc_port"},
		"Port on which to run the internal engine gRPC server.")
	registerString(fs, &cfg.EngineBinary, []string{"engine-binary", "engine_binary"},
		"Path to the C++ engine binary. Empty disables the subprocess.")
	registerString(fs, &cfg.DataDir,
		[]string{"data-dir", "data_dir"},
		"Persistent storage root. Passed to the engine as --data_dir. "+
			"Falls back to $BIGQUERY_EMULATOR_DATA_DIR when empty.")
	registerString(fs, &cfg.InitialDataDir,
		[]string{"initial-data-dir"},
		"Template directory copied into --data-dir on first start when "+
			"--data-dir is empty. Falls back to $BIGQUERY_EMULATOR_INITIAL_DATA_DIR "+
			"/ $EMULATOR_INITIAL_DATA_DIR.")
	registerBool(fs, &cfg.CopyEngineStdout, []string{"copy-engine-stdout", "copy_engine_stdout"},
		"Forward the engine subprocess's stdout to the gateway's.")
	registerBool(fs, &cfg.CopyEngineStderr, []string{"copy-engine-stderr", "copy_engine_stderr"},
		"Forward the engine subprocess's stderr to the gateway's.")
	registerBool(fs, &cfg.LogRequests, []string{"log-requests", "log_requests"},
		"Log every REST request and response.")
	registerBool(fs, &cfg.Debug, []string{"debug"},
		"Enable verbose lifecycle logging.")
	registerString(fs, &cfg.DefaultProjectID, []string{"project-id", "project_id"},
		"Default BigQuery project clients are assumed to act against.")
	registerString(fs, &cfg.DefaultDatasetLocation, []string{"default-dataset-location"},
		"Default BigQuery location stamped on datasets created without an "+
			"explicit location (e.g. US, EU).")
	registerBool(fs, &cfg.EnableSeedAPI, []string{"enable-seed-api"},
		"Register POST /api/emulator/seed and the operation polling endpoint.")
	registerBool(fs, &cfg.SeedAPIAllowRemote, []string{"seed-api-allow-remote"},
		"Allow non-loopback callers to invoke the seed API.")
	registerString(fs, &cfg.SeedAPISeedToken,
		[]string{"seed-api-seed-token"},
		"Required value for the X-BigQuery-Emulator-Seed-Token header on every "+
			"seed request. Falls back to $BIGQUERY_EMULATOR_SEED_TOKEN.")
	registerStringSlice(fs, &cfg.SeedFiles, []string{"seed-data-file", "seed-yaml"},
		"YAML seed-data file to apply once the engine reports SERVING (repeatable).")
	registerBool(fs, versionFlag, []string{"version"},
		"Print version information (semver + git commit + build date + Go toolchain) and exit.")
}

// validatePorts rejects out-of-range HTTP/gRPC ports or the case
// where both happen to be the same. Pulled out of parseArgs so the
// branch is testable and parseArgs stays short.
func validatePorts(cfg Config) error {
	if cfg.HTTPPort <= 0 || cfg.HTTPPort > 65535 {
		return fmt.Errorf("invalid --http-port %d: must be in 1..65535", cfg.HTTPPort)
	}
	if cfg.GRPCPort <= 0 || cfg.GRPCPort > 65535 {
		return fmt.Errorf("invalid --grpc-port %d: must be in 1..65535", cfg.GRPCPort)
	}
	if cfg.HTTPPort == cfg.GRPCPort {
		return fmt.Errorf("--http-port and --grpc-port must differ (both %d)", cfg.HTTPPort)
	}
	return nil
}

// applyEnvFallbacks walks the documented flag > env > nothing
// precedence. Mutates cfg in place. Each env-var name is checked in
// the documented order so the precedence stays observable from the
// source.
func applyEnvFallbacks(cfg *Config, getenv envLookup) {
	if cfg.DataDir == "" {
		if v, ok := getenv("BIGQUERY_EMULATOR_DATA_DIR"); ok {
			cfg.DataDir = v
		}
	}
	if cfg.InitialDataDir == "" {
		for _, key := range []string{
			"BIGQUERY_EMULATOR_INITIAL_DATA_DIR",
			"EMULATOR_INITIAL_DATA_DIR",
		} {
			if v, ok := getenv(key); ok && v != "" {
				cfg.InitialDataDir = v
				break
			}
		}
	}
	if cfg.SeedAPISeedToken == "" {
		if v, ok := getenv("BIGQUERY_EMULATOR_SEED_TOKEN"); ok {
			cfg.SeedAPISeedToken = v
		}
	}
}

func (c Config) engineInternalGRPCPort() int {
	// The public BigQuery Storage shim binds --grpc-port; the engine
	// subprocess listens on the next port so both can coexist.
	return c.GRPCPort + 1
}

// ToOptions projects the parsed CLI config onto the gateway.Options
// addresses the runtime consumes. storageGRPCAddr is where client
// libraries dial BIGQUERY_STORAGE_GRPC_ENDPOINT; engineAddr is the
// internal bigquery_emulator.v1 listener the gateway dials.
func (c Config) ToOptions(engineBinary string) (httpAddr, storageGRPCAddr, engineAddr string, engineArgs []string) {
	httpAddr = c.ListenHost + ":" + strconv.Itoa(c.HTTPPort)
	storageGRPCAddr = c.ListenHost + ":" + strconv.Itoa(c.GRPCPort)
	if engineBinary != "" {
		engineAddr = c.ListenHost + ":" + strconv.Itoa(c.engineInternalGRPCPort())
	}
	engineArgs = c.engineCLIArgs()
	return httpAddr, storageGRPCAddr, engineAddr, engineArgs
}

// engineCLIArgs renders the engine pass-through flags as a flat
// `--key value` slice. The engine uses double-hyphen underscore-style
// flags (`--data_dir foo`); operators on the gateway side can supply
// the same flag with either dash convention thanks to the aliasing
// above, but here we always emit the form emulator_main parses.
//
// Empty values are dropped; the gateway never forwards a flag the
// operator didn't set. The engine then keeps its own default.
func (c Config) engineCLIArgs() []string {
	type pair struct{ name, value string }
	pairs := []pair{
		{"--data_dir", c.DataDir},
	}
	args := make([]string, 0, len(pairs)*2)
	for _, p := range pairs {
		if p.value == "" {
			continue
		}
		args = append(args, p.name, p.value)
	}
	return args
}

// registerString registers a string flag under every name in
// `names`, all pointing at the same target. The first name in the
// slice owns the description that shows up in `--help`; aliases get
// a "(alias for --<first>)" stub so the help output stays scannable.
func registerString(fs *flag.FlagSet, target *string, names []string, desc string) {
	if len(names) == 0 {
		return
	}
	fs.StringVar(target, names[0], *target, desc)
	for _, alias := range names[1:] {
		fs.StringVar(target, alias, *target, "(alias for --"+names[0]+")")
	}
}

// registerInt is the integer twin of registerString. It uses
// stringTarget under the hood so a malformed value writes an error
// to the FlagSet's output and ParseError surfaces to the caller.
func registerInt(fs *flag.FlagSet, target *int, names []string, desc string) {
	if len(names) == 0 {
		return
	}
	fs.IntVar(target, names[0], *target, desc)
	for _, alias := range names[1:] {
		fs.IntVar(target, alias, *target, "(alias for --"+names[0]+")")
	}
}

// registerBool is the boolean twin of registerString. Bool flags
// uniquely accept the `--name` (no value) and `--name=true` /
// `--name=false` forms; both aliases inherit that behavior.
func registerBool(fs *flag.FlagSet, target *bool, names []string, desc string) {
	if len(names) == 0 {
		return
	}
	fs.BoolVar(target, names[0], *target, desc)
	for _, alias := range names[1:] {
		fs.BoolVar(target, alias, *target, "(alias for --"+names[0]+")")
	}
}

// registerStringSlice registers a repeatable string flag under every
// name in `names`. The Go std flag package has no built-in repeat
// support, so we install a tiny flag.Value that appends to the
// target slice on every Set.
func registerStringSlice(fs *flag.FlagSet, target *[]string, names []string, desc string) {
	if len(names) == 0 {
		return
	}
	value := stringSliceValue{target: target}
	fs.Var(&value, names[0], desc)
	for _, alias := range names[1:] {
		fs.Var(&value, alias, "(alias for --"+names[0]+")")
	}
}

// stringSliceValue is the flag.Value implementation used by
// registerStringSlice. Each `--seed-data-file foo` appends "foo" to
// the target slice in the order the operator supplied them, which is
// the order the seed loader applies them.
type stringSliceValue struct {
	target *[]string
}

func (s *stringSliceValue) String() string {
	if s == nil || s.target == nil {
		return ""
	}
	return strings.Join(*s.target, ",")
}

func (s *stringSliceValue) Set(v string) error {
	if s == nil || s.target == nil {
		return errors.New("stringSliceValue: nil target")
	}
	*s.target = append(*s.target, v)
	return nil
}
