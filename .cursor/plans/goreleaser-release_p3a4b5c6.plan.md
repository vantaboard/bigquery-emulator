---
name: goreleaser-release
overview: "Phase 9b: GoReleaser binaries and semantic-release workflow."
todos:
  - id: goreleaser
    content: "Add .goreleaser.yml: gateway for 4 platforms; bundle emulator binary from CI artifact"
    status: completed
  - id: semantic-release
    content: "Wire .releaserc.yml with GitHub release workflow; publish Docker on tag"
    status: completed
isProject: false
---

# Phase 9b: Goreleaser Release

## Prerequisites

- [docker-compose-smoke_o2f3a4b5.plan.md](docker-compose-smoke_o2f3a4b5.plan.md)

## Verification

```bash
goreleaser release --snapshot
```

## Done criteria

- Snapshot release produces archives

## Next plan(s)

- [profile-docs-version_q4b5c6d7.plan.md](profile-docs-version_q4b5c6d7.plan.md)
