#!/usr/bin/env bash
# Retry docker build for flaky registry/network on GitHub-hosted runners.
#
# CI Dockerfiles declare `# syntax=docker/dockerfile:…`, which BuildKit
# resolves from Docker Hub before the first build step. Parallel thirdparty
# jobs routinely hit transient `i/o timeout` / `DeadlineExceeded` errors on
# that metadata fetch; a short pre-pull + bounded retry avoids a full
# workflow failure.
#
# Usage (from repo root):
#   bash scripts/docker_build_retry.sh docker build --build-arg … -t tag .
#   bash scripts/docker_build_retry.sh docker buildx build --tag tag --load .
#
# Tunables:
#   DOCKERFILE                    Dockerfile path for syntax pre-pull (default Dockerfile)
#   DOCKER_BUILD_MAX_ATTEMPTS     default 3
#   DOCKER_BUILD_RETRY_DELAY_SEC  initial backoff seconds (default 15)
set -euo pipefail

if [ "$#" -lt 3 ] || [ "$1" != docker ]; then
	echo "usage: $0 docker build … | docker buildx build …" >&2
	exit 2
fi

case "$2" in
build)
	;;
buildx)
	if [ "${3:-}" != build ]; then
		echo "usage: $0 docker buildx build …" >&2
		exit 2
	fi
	;;
*)
	echo "usage: $0 docker build … | docker buildx build …" >&2
	exit 2
	;;
esac

max_attempts="${DOCKER_BUILD_MAX_ATTEMPTS:-3}"
initial_delay="${DOCKER_BUILD_RETRY_DELAY_SEC:-15}"
dockerfile="${DOCKERFILE:-Dockerfile}"

pull_dockerfile_frontend() {
	[ -f "$dockerfile" ] || return 0
	local syntax_line image
	syntax_line="$(grep -m1 '^# syntax=' "$dockerfile" 2>/dev/null || true)"
	[ -n "$syntax_line" ] || return 0
	image="${syntax_line#*=}"
	image="${image#"${image%%[![:space:]]*}"}"
	echo "Pre-pulling Dockerfile frontend ${image} (from ${dockerfile})" >&2
	docker pull "$image"
}

attempt=1
delay="$initial_delay"
while true; do
	echo "docker build attempt ${attempt}/${max_attempts}" >&2
	if pull_dockerfile_frontend && "$@"; then
		exit 0
	fi
	if [ "$attempt" -ge "$max_attempts" ]; then
		echo "docker build failed after ${max_attempts} attempts" >&2
		exit 1
	fi
	echo "docker build failed; retrying in ${delay}s…" >&2
	sleep "$delay"
	delay=$((delay * 2))
	attempt=$((attempt + 1))
done
