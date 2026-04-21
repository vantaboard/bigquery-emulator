//go:build !linux

package connection

// tryCgroupMemoryLimitBytes returns cgroup memory limit on non-Linux builds.
func tryCgroupMemoryLimitBytes() (uint64, bool) {
	return 0, false
}
