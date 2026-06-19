#!/bin/sh
# Container shim for the BigQuery emulator's Go REST gateway.
#
# The Go binary at /opt/bigquery-emulator/gateway_main keeps its
# developer-friendly upstream defaults: loopback `--hostname=localhost`,
# no `--data-dir`, and no bundled seed file. Inside this image we want a
# different out-of-the-box posture: the published REST port reachable
# through `docker run -p 9050:9050` (bind 0.0.0.0), a persistent data
# directory on the mounted volume, and the bundled public-dataset seed
# loaded.
#
# These MUST live in the entrypoint shim, not in the image `CMD`.
# Docker REPLACES the entire `CMD` whenever the caller passes any
# argument to `docker run` (the `ENTRYPOINT` stays). So baking these
# operational defaults into `CMD` means a single user flag — e.g.
# `docker run ... :latest --project-id=foo` — silently drops ALL of
# them (no seed data, no persistent data dir), which reads as "my CLI
# arguments aren't being respected".
#
# Instead this shim injects each default ONLY when the caller did not
# already supply that flag, so user-provided flags AUGMENT the defaults
# rather than wiping them. Caller-supplied values always win:
#
#   --hostname / -hostname            overrides the 0.0.0.0 bind
#   --data-dir / --data_dir           overrides the persistent volume path
#   --seed-data-file / --seed-yaml    replaces the bundled seed (repeatable)

set -eu

# Operational defaults baked into the image. Kept here (not in the
# Dockerfile `CMD`) so a caller override of any unrelated flag does not
# drop them — see the header comment.
DEFAULT_DATA_DIR=/var/lib/bigquery-emulator
DEFAULT_SEED_FILE=/opt/bigquery-emulator/testdata/public-data/bigquery-public-data.yaml

has_hostname=0
has_data_dir=0
has_seed_file=0
has_sql_tools=0
for arg in "$@"; do
    case "${arg}" in
        --hostname|--hostname=*|-hostname|-hostname=*)
            has_hostname=1
            ;;
        --data-dir|--data-dir=*|--data_dir|--data_dir=*)
            has_data_dir=1
            ;;
        --seed-data-file|--seed-data-file=*|--seed-yaml|--seed-yaml=*)
            has_seed_file=1
            ;;
        --enable-sql-tools-api)
            has_sql_tools=1
            ;;
    esac
done

# Prepend defaults the caller did not set. Order does not matter to the
# gateway's flag parser; later (caller) flags still win for any single
# flag thanks to Go's `flag` late-override behavior.
if [ "${has_seed_file}" -eq 0 ]; then
    set -- --seed-data-file="${DEFAULT_SEED_FILE}" "$@"
fi
if [ "${has_data_dir}" -eq 0 ]; then
    set -- --data-dir="${DEFAULT_DATA_DIR}" "$@"
fi
if [ "${has_hostname}" -eq 0 ]; then
    set -- --hostname=0.0.0.0 "$@"
fi
if [ "${has_sql_tools}" -eq 0 ]; then
    set -- --enable-sql-tools-api "$@"
fi

exec /opt/bigquery-emulator/gateway_main "$@"
