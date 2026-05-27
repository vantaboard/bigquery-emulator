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
	"fmt"
	"net"
	"net/http"
	"os"
	"strings"
	"time"
)

const (
	defaultEmulatorHTTPDialTimeout = 15 * time.Second
	defaultEmulatorGRPCDialTimeout = 15 * time.Second
)

func dialTimeoutFromEnv(name string, def time.Duration) time.Duration {
	s := strings.TrimSpace(os.Getenv(name))
	if s == "" {
		return def
	}
	d, err := time.ParseDuration(s)
	if err != nil || d <= 0 {
		return def
	}
	return d
}

func emulatorHTTPDialTimeout() time.Duration {
	return dialTimeoutFromEnv("BIGQUERY_EMULATOR_HTTP_DIAL_TIMEOUT", defaultEmulatorHTTPDialTimeout)
}

func emulatorGRPCDialTimeout() time.Duration {
	return dialTimeoutFromEnv("BIGQUERY_EMULATOR_GRPC_DIAL_TIMEOUT", defaultEmulatorGRPCDialTimeout)
}

// emulatorHintRoundTripper wraps connection errors with a hint when talking to
// BIGQUERY_EMULATOR_HOST so tests fail fast instead of hanging on the default transport.
type emulatorHintRoundTripper struct {
	Base   http.RoundTripper
	EnvVar string
	Host   string
}

func (t *emulatorHintRoundTripper) RoundTrip(req *http.Request) (*http.Response, error) {
	base := t.Base
	if base == nil {
		base = http.DefaultTransport
	}
	resp, err := base.RoundTrip(req)
	if err != nil {
		return nil, wrapEmulatorReachabilityErr(err, t.EnvVar, t.Host)
	}
	return resp, nil
}

func wrapEmulatorReachabilityErr(err error, envVar, host string) error {
	if err == nil {
		return nil
	}
	hint := fmt.Sprintf("%s=%q appears unreachable — is the BigQuery emulator running? (e.g. task emulator:start from go-googlesql)", envVar, host)
	// google.golang.org/api (and net/http) may wrap RoundTrip errors in *url.Error.
	// cloud.google.com/go/bigquery v1.74+ treats *url.Error strings containing
	// "connection refused" / "connection reset" as retryable, so scrub those tokens
	// from the detail we embed (see bigquery.retryableError).
	detail := redactForBigQueryRetryPredicate(fmt.Sprintf("%v", err))
	return &emulatorUnreachableError{msg: fmt.Sprintf("%s: %s", detail, hint)}
}

// redactForBigQueryRetryPredicate removes substrings matched by bigquery.retryableError on
// *url.Error (connection refused / reset) so wrapped errors are not retried forever.
func redactForBigQueryRetryPredicate(s string) string {
	// Order matters for multi-word phrases before single-token edits.
	replacements := []struct{ from, to string }{
		{"connection refused", "tcp not accepted"},
		{"Connection refused", "tcp not accepted"},
		{"connection reset by peer", "tcp reset by peer"},
		{"connection reset", "tcp reset"},
		{"Connection reset", "tcp reset"},
		{"i/o timeout", "I/O timed out"},
		{"I/O timeout", "I/O timed out"},
		{"context deadline exceeded", "deadline exceeded"},
	}
	out := s
	for _, r := range replacements {
		out = strings.ReplaceAll(out, r.from, r.to)
	}
	return out
}

// emulatorUnreachableError is an opaque wrapper so cloud.google.com/go/bigquery does not
// treat dial failures to the emulator as infinitely retryable transport blips.
type emulatorUnreachableError struct {
	msg string
}

func (e *emulatorUnreachableError) Error() string { return e.msg }

func grpcEmulatorDialer(timeout time.Duration, envVar, host string) func(context.Context, string) (net.Conn, error) {
	return func(ctx context.Context, addr string) (net.Conn, error) {
		d := net.Dialer{KeepAlive: 30 * time.Second}
		dialTimeout := timeout
		if deadline, ok := ctx.Deadline(); ok {
			if rem := time.Until(deadline); rem > 0 && rem < dialTimeout {
				dialTimeout = rem
			}
		}
		d.Timeout = dialTimeout
		conn, err := d.DialContext(ctx, "tcp", addr)
		if err != nil {
			return nil, wrapEmulatorReachabilityErr(err, envVar, host)
		}
		return conn, nil
	}
}

// newEmulatorHTTPClient returns an HTTP client with a bounded dial timeout and
// optional X-BigQuery-Emulator-Api-Region injection (when region is non-empty).
func newEmulatorHTTPClient(envVar, hostForMessage, region string, dialTimeout time.Duration) *http.Client {
	transport := http.DefaultTransport.(*http.Transport).Clone()
	transport.DialContext = (&net.Dialer{
		Timeout:   dialTimeout,
		KeepAlive: 30 * time.Second,
	}).DialContext
	hinted := &emulatorHintRoundTripper{Base: transport, EnvVar: envVar, Host: hostForMessage}
	var rt http.RoundTripper = hinted
	if strings.TrimSpace(region) != "" {
		rt = &emulatorAPIRegionRoundTripper{
			Base:   hinted,
			Region: strings.TrimSpace(region),
		}
	}
	return &http.Client{Transport: rt}
}
