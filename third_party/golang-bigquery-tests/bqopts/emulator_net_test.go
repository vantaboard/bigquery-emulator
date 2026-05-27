// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package bqopts

import (
	"context"
	"net/http"
	"strings"
	"syscall"
	"testing"
	"time"
)

func TestWrapEmulatorReachabilityErr_connectionRefused(t *testing.T) {
	t.Parallel()
	wrapped := wrapEmulatorReachabilityErr(syscall.ECONNREFUSED, "BIGQUERY_EMULATOR_HOST", "127.0.0.1:9050")
	if wrapped == nil || !strings.Contains(wrapped.Error(), "emulator") {
		t.Fatalf("expected hint in error, got %v", wrapped)
	}
}

func TestNewEmulatorHTTPClient_unreachableFailsFast(t *testing.T) {
	cli := newEmulatorHTTPClient("BIGQUERY_EMULATOR_HOST", "127.0.0.1:1", "", 2*time.Second)
	ctx := context.Background()
	req, err := http.NewRequestWithContext(ctx, http.MethodGet, "http://127.0.0.1:1/", nil)
	if err != nil {
		t.Fatal(err)
	}
	start := time.Now()
	_, err = cli.Do(req)
	if err == nil {
		t.Fatal("expected error dialing closed port")
	}
	if time.Since(start) > 10*time.Second {
		t.Fatalf("dial took too long (%v); expected bounded dial timeout", time.Since(start))
	}
	if !strings.Contains(err.Error(), "emulator") {
		t.Fatalf("expected emulator hint in error, got: %v", err)
	}
}

func TestDialTimeoutFromEnv_invalidFallsBack(t *testing.T) {
	t.Setenv("BIGQUERY_EMULATOR_HTTP_DIAL_TIMEOUT", "not-a-duration")
	if got := dialTimeoutFromEnv("BIGQUERY_EMULATOR_HTTP_DIAL_TIMEOUT", time.Second); got != time.Second {
		t.Fatalf("got %v", got)
	}
}
