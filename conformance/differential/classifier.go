package differential

import (
	"strings"
)

// DivergenceKind classifies emulator vs oracle outcomes for the differential lane.
type DivergenceKind string

const (
	KindMatch              DivergenceKind = "match"
	KindFeatureGap         DivergenceKind = "feature_gap"
	KindSemanticDivergence DivergenceKind = "semantic_divergence"
	KindErrorDivergence    DivergenceKind = "error_divergence"
	KindCrash              DivergenceKind = "crash"
)

// ClassifyInput carries the signals ClassifyDivergence needs.
type ClassifyInput struct {
	OracleSuccess   bool
	EmulatorSuccess bool
	EmulatorStatus  int
	EmulatorBody    []byte
	Diff            string
	RunnerMessage   string
}

// ClassifyDivergence maps a replay outcome to a divergence bucket.
func ClassifyDivergence(in ClassifyInput) DivergenceKind {
	msg := strings.ToLower(in.RunnerMessage + " " + in.Diff + " " + string(in.EmulatorBody))
	if isCrashSignal(msg) {
		return KindCrash
	}
	if isFeatureGap(msg) {
		return KindFeatureGap
	}
	if in.OracleSuccess != in.EmulatorSuccess {
		return KindErrorDivergence
	}
	if in.Diff != "" {
		return KindSemanticDivergence
	}
	return KindMatch
}

func isFeatureGap(msg string) bool {
	needles := []string{
		"unimplemented",
		"not implemented",
		"not yet implemented",
		"setoperationscan op is not union all",
		"withrefscan without active withscan bindings",
	}
	for _, n := range needles {
		if strings.Contains(msg, n) {
			return true
		}
	}
	return false
}

func isCrashSignal(msg string) bool {
	needles := []string{
		"signal: killed",
		"signal: aborted",
		"engine process exited",
		"lost connection",
		"connection refused",
		"broken pipe",
		"segfault",
		"core dumped",
	}
	for _, n := range needles {
		if strings.Contains(msg, n) {
			return true
		}
	}
	return false
}
