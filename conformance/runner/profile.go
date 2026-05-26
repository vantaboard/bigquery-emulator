package runner

import "sort"

// Profile is one named engine + storage combination the runner can
// drive. Profiles are static today (`memory` and `duckdb`); a future
// plan may make these configurable via a top-level config file.
//
// `EmulatorMainFlags` is the flag list the harness passes when
// spawning `emulator_main`; the runner does not include `--host_port`
// here because the harness picks a free port per spawn.
type Profile struct {
	// Name is the user-facing profile identifier (`memory`, `duckdb`).
	// Fixtures reference this in their `profiles:` field, and the
	// runner echoes it in every result line.
	Name string

	// Engine is the `--engine` value (`reference_impl` or `duckdb`).
	Engine string

	// Storage is the `--storage` value (`memory` or `duckdb`).
	Storage string

	// OnUnknownFn is the `--on_unknown_fn` policy. The DuckDB
	// profile uses `fallback` so the DuckDB transpiler's known
	// gaps (DDL and most DML lower through the reference-impl
	// engine via the FallbackEngine wrapper) do not surface as
	// UNIMPLEMENTED. The memory profile leaves it default
	// (`unimplemented`) so any divergence stays loud.
	OnUnknownFn string
}

// EmulatorMainArgs returns the `emulator_main` flag list for this
// profile, ready to splice into `exec.Command` args (the harness
// prepends `--host_port` and may append `--data_dir`).
func (p Profile) EmulatorMainArgs() []string {
	args := []string{"--engine", p.Engine, "--storage", p.Storage}
	if p.OnUnknownFn != "" {
		args = append(args, "--on_unknown_fn", p.OnUnknownFn)
	}
	return args
}

var profiles = []Profile{
	{
		Name:    "memory",
		Engine:  "reference_impl",
		Storage: "memory",
	},
	{
		Name:        "duckdb",
		Engine:      "duckdb",
		Storage:     "duckdb",
		OnUnknownFn: "fallback",
	},
}

// KnownProfiles returns a defensive copy of the known profile table.
// The slice is alphabetized by name so the matrix iteration order is
// stable.
func KnownProfiles() []Profile {
	out := make([]Profile, len(profiles))
	copy(out, profiles)
	sort.Slice(out, func(i, j int) bool { return out[i].Name < out[j].Name })
	return out
}

// LookupProfile resolves a profile name to its Profile entry. Returns
// (zero, false) for unknown names.
func LookupProfile(name string) (Profile, bool) {
	for _, p := range profiles {
		if p.Name == name {
			return p, true
		}
	}
	return Profile{}, false
}

func profileNames() []string {
	out := make([]string, 0, len(profiles))
	for _, p := range profiles {
		out = append(out, p.Name)
	}
	sort.Strings(out)
	return out
}
