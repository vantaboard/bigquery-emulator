package bqv2grpc

import "math"

func uint64FromNonNegativeInt64(v int64) uint64 {
	if v < 0 {
		return 0
	}
	return uint64(v)
}

func int64FromUint64(v uint64) int64 {
	if v > uint64(math.MaxInt64) {
		return math.MaxInt64
	}
	return int64(v)
}

func int32FromInt(v int) int32 {
	if v > int(math.MaxInt32) {
		return math.MaxInt32
	}
	if v < int(math.MinInt32) {
		return math.MinInt32
	}
	return int32(v)
}
