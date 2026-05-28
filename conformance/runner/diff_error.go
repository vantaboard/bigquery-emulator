package runner

import (
	"encoding/json"
	"fmt"
	"slices"
	"strings"
)

// errorDiff compares the gateway's error envelope against an
// `expected.error` block and returns an empty string on match or a
// human-readable message on mismatch.
func errorDiff(expected ExpectedError, status int, body []byte) string {
	var env struct {
		Error struct {
			Code    int    `json:"code"`
			Message string `json:"message"`
			Status  string `json:"status"`
			Errors  []struct {
				Reason  string `json:"reason"`
				Message string `json:"message"`
			} `json:"errors"`
		} `json:"error"`
	}
	_ = json.Unmarshal(body, &env)

	if expected.Code != 0 && expected.Code != status {
		return fmt.Sprintf("error code: expected %d, got %d (body: %s)",
			expected.Code, status, snippet(body))
	}
	if expected.MessageContains != "" {
		hay := env.Error.Message
		if hay == "" && len(env.Error.Errors) > 0 {
			hay = env.Error.Errors[0].Message
		}
		if !strings.Contains(hay, expected.MessageContains) {
			return fmt.Sprintf(
				"error message: expected to contain %q, got %q (body: %s)",
				expected.MessageContains, hay, snippet(body))
		}
	}
	return ""
}

// snippet truncates a body for inclusion in a diff message; the
// body can be large (the engine emits ZetaSQL parse-error pointers)
// and we want the diff to stay scannable.
func snippet(b []byte) string {
	const limit = 240
	s := strings.TrimSpace(string(b))
	if len(s) > limit {
		s = s[:limit] + "..."
	}
	return s
}

func containsString(haystack []string, needle string) bool {
	return slices.Contains(haystack, needle)
}
