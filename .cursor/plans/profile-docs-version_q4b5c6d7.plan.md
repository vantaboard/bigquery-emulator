---
name: profile-docs-version
overview: "Phase 9c: profile docs, --version flags, README release section."
todos:
  - id: profile-docs
    content: "Document --profile=ci|duckdb|dev in README and ROADMAP Phase 9"
    status: completed
  - id: version-flag
    content: "Add --version to gateway and emulator_main with git sha + semver ldflags"
    status: completed
isProject: false
---

# Phase 9c: Profile Docs Version

## Prerequisites

- [goreleaser-release_p3a4b5c6.plan.md](goreleaser-release_p3a4b5c6.plan.md)

## Verification

```bash
./bin/gateway_main --version && ./build-out/emulator_main --version
```

## Done criteria

- --version reports consistent semver
- Profile docs complete
