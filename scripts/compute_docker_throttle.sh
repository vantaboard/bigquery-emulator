#!/usr/bin/env bash
# Compute docker build + runtime throttle env vars for the bigquery-emulator
# stack, written to stdout as `export KEY=value` lines for `eval`-style
# consumption from a Taskfile / shell:
#
#   eval "$(bash scripts/compute_docker_throttle.sh)"
#
# Picks are dynamic against /proc/meminfo's *MemAvailable* (live RAM minus
# what is already in use by the IDE / browser / kernel page cache), not
# MemTotal, because the in-container Bazel build (engine-builder-bazel
# stage) auto-detects the host's full MemTotal from the container's
# perspective and routinely OOM-kills the host when 10+ concurrent
# clang++ processes each spike to 2-3 GiB on GoogleSQL's heavy template
# files (e.g. resolved_ast_rewrite_visitor_*.cc, the analyzer rewriters).
# See `.cursor/rules/process-hygiene.mdc` for the failure-mode background.
#
# Bazel's `--local_resources=memory=N` is a *scheduling hint*, not an OS-
# enforced cap, so the script bounds *job count* against
# `MemAvailable / per-job-budget` instead of relying on a single memory
# number to throttle the build. Picked values:
#
#   BAZEL_JOBS            min(cpu_jobs, mem_jobs), floor BAZEL_JOB_FLOOR (=2)
#   BAZEL_MEM_MB          jobs * BAZEL_PER_JOB_MEM_MB (matches the picked
#                         scheduling shape so Bazel's scheduler stays honest)
#   BIGQUERY_EMULATOR_MEM_LIMIT   `<run_mem>m` cgroup cap for the running
#                                 container; defaults to BAZEL_AVAIL_PCT %
#                                 of MemAvailable, floored at 2 GiB
#   BIGQUERY_EMULATOR_CPUS        nproc-2 (floor 2.0) — leaves two cores
#                                 for the host scheduler / IDE
#
# Tuning knobs (all optional, read from the calling environment):
#
#   BAZEL_PER_JOB_MEM_MB  RAM budget per concurrent clang job (default 3072
#                         = 3 GiB; leaves headroom above the observed
#                         ~2.5 GiB peak for heavy googlesql templates).
#   BAZEL_AVAIL_PCT       Percent of MemAvailable the build may consume
#                         (default 80; honors the user's stated preference).
#   BAZEL_CPU_RESERVE     Cores to leave free for the host (default 4).
#   BAZEL_JOB_FLOOR       Minimum job count even on a tiny host (default 2).
#   BAZEL_JOBS            Hard override; printed verbatim if set.
#   BAZEL_MEM_MB          Hard override; printed verbatim if set.
#   BIGQUERY_EMULATOR_MEM_LIMIT, BIGQUERY_EMULATOR_CPUS
#                         Hard overrides; printed verbatim if set.
#
# A short summary of the picks (and the host inputs that drove them) is
# emitted on stderr so an interactive `task ...` run shows the operator
# why the build was throttled this way.
set -eo pipefail

per_job_mb="${BAZEL_PER_JOB_MEM_MB:-3072}"
avail_pct="${BAZEL_AVAIL_PCT:-80}"
cpu_reserve="${BAZEL_CPU_RESERVE:-4}"
job_floor="${BAZEL_JOB_FLOOR:-2}"

cores=$(nproc 2>/dev/null || echo 4)
if [ -r /proc/meminfo ]; then
	avail_mb=$(awk '/^MemAvailable:/ { printf "%d", $2 / 1024; exit }' /proc/meminfo)
	total_mb=$(awk '/^MemTotal:/ { printf "%d", $2 / 1024; exit }' /proc/meminfo)
else
	# Non-Linux fallback (macOS Docker Desktop runs the build in a Linux VM
	# anyway, so the in-container /proc/meminfo always exists at build time;
	# this branch only matters when the script is invoked on the host shell
	# of a non-Linux machine that lacks /proc).
	avail_mb=4096
	total_mb=8192
fi

# Build-time RAM budget = avail_pct % of MemAvailable. Floor at per_job_mb
# so a heavily-loaded host still gets at least one Bazel worker.
budget_mb=$((avail_mb * avail_pct / 100))
if [ "$budget_mb" -lt "$per_job_mb" ]; then
	budget_mb=$per_job_mb
fi

# Jobs bounded by RAM (each concurrent clang gets per_job_mb headroom).
mem_jobs=$((budget_mb / per_job_mb))
if [ "$mem_jobs" -lt 1 ]; then mem_jobs=1; fi

# Jobs bounded by CPU (leave cpu_reserve cores for the host scheduler).
cpu_jobs=$((cores - cpu_reserve))
if [ "$cpu_jobs" -lt "$job_floor" ]; then cpu_jobs=$job_floor; fi

# Take the lower of the two bounds so neither RAM nor CPU is oversubscribed.
jobs=$mem_jobs
if [ "$cpu_jobs" -lt "$jobs" ]; then jobs=$cpu_jobs; fi
if [ "$jobs" -lt "$job_floor" ]; then jobs=$job_floor; fi

# Match the scheduling hint to the picked job count (jobs * per_job_mb) so
# Bazel's own scheduler does not pack more actions onto the workers than
# the underlying RAM budget actually allows.
mem_mb=$((jobs * per_job_mb))

# Runtime cgroup caps. Independent of build-time picks because the running
# container only hosts the engine + gateway (typically << 4 GiB resident);
# the cap exists as a safety net against a pathological query trip in the
# GoogleSQL analyzer, not as a primary OOM guard.
run_mem_mb=$((avail_mb * avail_pct / 100))
if [ "$run_mem_mb" -lt 2048 ]; then run_mem_mb=2048; fi

run_cpus=$((cores - 2))
if [ "$run_cpus" -lt 2 ]; then run_cpus=2; fi

# Honor caller overrides if set (so `BAZEL_JOBS=8 task ...` still wins).
final_bazel_jobs="${BAZEL_JOBS:-$jobs}"
final_bazel_mem_mb="${BAZEL_MEM_MB:-$mem_mb}"
final_run_mem="${BIGQUERY_EMULATOR_MEM_LIMIT:-${run_mem_mb}m}"
final_run_cpus="${BIGQUERY_EMULATOR_CPUS:-${run_cpus}.0}"

# Human summary on stderr so the picks are visible in an interactive run
# without polluting the stdout that `eval` consumes.
{
	printf 'compute_docker_throttle: host: cores=%s MemAvailable=%s MiB MemTotal=%s MiB\n' \
		"$cores" "$avail_mb" "$total_mb"
	printf 'compute_docker_throttle: build: BAZEL_JOBS=%s BAZEL_MEM_MB=%s (per_job=%s MiB, avail_pct=%s%%)\n' \
		"$final_bazel_jobs" "$final_bazel_mem_mb" "$per_job_mb" "$avail_pct"
	printf 'compute_docker_throttle: runtime: mem_limit=%s cpus=%s\n' \
		"$final_run_mem" "$final_run_cpus"
} >&2

# Stdout: `export KEY=value` lines for `eval`.
printf 'export BAZEL_JOBS=%s\n' "$final_bazel_jobs"
printf 'export BAZEL_MEM_MB=%s\n' "$final_bazel_mem_mb"
printf 'export BIGQUERY_EMULATOR_MEM_LIMIT=%s\n' "$final_run_mem"
printf 'export BIGQUERY_EMULATOR_CPUS=%s\n' "$final_run_cpus"
