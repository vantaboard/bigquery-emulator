#!/bin/sh
# Container shim for the BigQuery emulator's Go REST gateway.
#
# The Go binary at /opt/bigquery-emulator/gateway_main keeps its
# upstream `--hostname=localhost` default so local-developer behaviour
# stays loopback-only. Inside this image we want the public REST port
# to be reachable through `docker run -p 9050:9050`, which requires
# binding to all interfaces, so this shim injects `--hostname=0.0.0.0`
# whenever the caller did not pass an explicit `--hostname`.
#
# Pass `--hostname=...` (or `-hostname=...`) to override.

set -eu

has_hostname=0
for arg in "$@"; do
    case "${arg}" in
        --hostname|--hostname=*|-hostname|-hostname=*)
            has_hostname=1
            break
            ;;
    esac
done

if [ "${has_hostname}" -eq 0 ]; then
    set -- --hostname=0.0.0.0 "$@"
fi

exec /opt/bigquery-emulator/gateway_main "$@"
