package runner

import "sort"

// Profile is one named runtime configuration the runner can drive.
// Today there is only one (`duckdb`) since the emulator collapsed
// onto DuckDB-only engine + storage; the type is kept around so the
// fixture / CLI surface stays generic if a second profile lands
// later.
//
// `EmulatorMainArgs` is the flag list the harness passes when
// spawning `emulator_main`; the runner does not include
// `--host_port` here because the harness picks a free port per
// spawn.
type Profile struct {
	// Name is the user-facing profile identifier. Fixtures reference
	// this in their `profiles:` field, and the runner echoes it in
	// every result line.
	Name string
}

// EmulatorMainArgs returns the `emulator_main` flag list for this
// profile. The DuckDB profile has no engine / storage selector
// flags (those were removed when the reference-impl + in-memory
// storage backends were deleted), so this is an empty slice today.
// Keeping the helper means fixtures and the runner do not have to
// branch on profile name.
func (p Profile) EmulatorMainArgs() []string {
	return nil
}

var profiles = []Profile{
	{
		Name: "duckdb",
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
