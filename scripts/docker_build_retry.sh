#!/usr/bin/env bash
# Retry docker build for flaky registry/network on GitHub-hosted runners.
#
# CI Dockerfiles declare `# syntax=…/docker/dockerfile:…`, which BuildKit
# resolves from a registry before the first build step. Parallel thirdparty
# jobs routinely hit transient Docker Hub timeouts on that metadata fetch;
# prefer `mirror.gcr.io/docker/dockerfile:…` in the Dockerfile syntax line.
#
# Usage (from repo root):
#   bash scripts/docker_build_retry.sh docker build --build-arg … -t tag .
#   bash scripts/docker_build_retry.sh docker buildx build --tag tag --load .
#
# Tunables:
#   DOCKERFILE                    Dockerfile path for syntax pre-pull (default Dockerfile)
#   DOCKER_PULL_MAX_ATTEMPTS      pre-pull retries (default 6)
#   DOCKER_PULL_RETRY_DELAY_SEC   initial pull backoff seconds (default 10)
#   DOCKER_BUILD_MAX_ATTEMPTS     full build retries (default 5)
#   DOCKER_BUILD_RETRY_DELAY_SEC  initial build backoff seconds (default 15)
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

max_attempts="${DOCKER_BUILD_MAX_ATTEMPTS:-5}"
initial_delay="${DOCKER_BUILD_RETRY_DELAY_SEC:-15}"
pull_max_attempts="${DOCKER_PULL_MAX_ATTEMPTS:-6}"
pull_initial_delay="${DOCKER_PULL_RETRY_DELAY_SEC:-10}"
dockerfile="${DOCKERFILE:-Dockerfile}"

pull_dockerfile_frontend() {
	[ -f "$dockerfile" ] || return 0
	local syntax_line image
	syntax_line="$(grep -m1 '^# syntax=' "$dockerfile" 2>/dev/null || true)"
	[ -n "$syntax_line" ] || return 0
	image="${syntax_line#*=}"
	image="${image#"${image%%[![:space:]]*}"}"
	echo "Pre-pulling Dockerfile frontend ${image} (from ${dockerfile})" >&2
	local attempt=1 delay="$pull_initial_delay"
	while [ "$attempt" -le "$pull_max_attempts" ]; do
		if docker pull "$image"; then
			return 0
		fi
		if [ "$attempt" -ge "$pull_max_attempts" ]; then
			echo "docker pull ${image} failed after ${pull_max_attempts} attempts" >&2
			return 1
		fi
		echo "docker pull failed (${attempt}/${pull_max_attempts}); retrying in ${delay}s…" >&2
		sleep "$delay"
		delay=$((delay + delay / 2))
		attempt=$((attempt + 1))
	done
	return 1
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
