#!/usr/bin/env bash
# Retry docker pull for flaky registry/network on GitHub-hosted runners.
#
# Prefer mirror.gcr.io/… mirrors over docker.io when possible.
#
# Usage:
#   bash scripts/docker_pull_retry.sh IMAGE [IMAGE…]
#
# Tunables:
#   DOCKER_PULL_MAX_ATTEMPTS      retries per image (default 6)
#   DOCKER_PULL_RETRY_DELAY_SEC   initial backoff seconds (default 10)
set -euo pipefail

if [ "$#" -lt 1 ]; then
	echo "usage: $0 IMAGE [IMAGE…]" >&2
	exit 2
fi

pull_max_attempts="${DOCKER_PULL_MAX_ATTEMPTS:-6}"
pull_initial_delay="${DOCKER_PULL_RETRY_DELAY_SEC:-10}"

pull_one() {
	local image="$1"
	local attempt=1 delay="$pull_initial_delay"
	echo "Pulling ${image}" >&2
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

for image in "$@"; do
	pull_one "$image"
done
