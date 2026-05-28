package main

// Sentinel C++ source paths the lint-tool's tests assert against. They
// are exported because both the package's tests and the production
// `isLoggingAllowed` carve-out reference them; promoting the literals
// to a single source of truth keeps the path spelling in one place
// and lets goconst stop flagging the repeats.
const (
	SentinelEmulatorMain     = "binaries/emulator_main/main.cc"
	SentinelEmulatorVersionH = "binaries/emulator_main/version.h"
	SentinelEmulatorVersionT = "binaries/emulator_main/version_test.cc"
	SentinelEngine           = "backend/engine/engine.cc"
	SentinelSmoke            = "tools/googlesql-prebuilt/smoke/smoke.cc"
)
