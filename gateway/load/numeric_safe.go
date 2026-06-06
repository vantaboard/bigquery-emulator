package load

import "math"

func uint64ToSignedInt64(v uint64) int64 {
	if v > uint64(math.MaxInt64) {
		return math.MaxInt64
	}
	return int64(v)
}

func datastorePropMarker(prop string) []byte {
	n := len(prop)
	if n == 0 || n > 255 {
		return nil
	}
	buf := make([]byte, 2+n)
	buf[0] = 0x1a
	buf[1] = byte(n) //nolint:gosec // n is range-checked to [1,255] above
	copy(buf[2:], prop)
	return buf
}
