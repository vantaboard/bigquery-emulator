//go:build linux

package connection

import (
	"os"
	"strconv"
	"strings"
)

// tryCgroupMemoryLimitBytes returns cgroup v2 memory.max if set to a finite limit.
func tryCgroupMemoryLimitBytes() (uint64, bool) {
	b, err := os.ReadFile("/sys/fs/cgroup/memory.max")
	if err != nil {
		return 0, false
	}
	s := strings.TrimSpace(string(b))
	if s == "" || s == "max" {
		return 0, false
	}
	n, err := strconv.ParseUint(s, 10, 64)
	if err != nil || n == 0 {
		return 0, false
	}
	return n, true
}
