package runner

import (
	"context"
	"errors"
	"testing"
	"time"

	"cloud.google.com/go/bigquery"
	"google.golang.org/api/googleapi"
)

func TestIsRateLimitErr(t *testing.T) {
	cases := []struct {
		name string
		err  error
		want bool
	}{
		{"nil", nil, false},
		{"plain", errors.New("boom"), false},
		{"not found", &googleapi.Error{Code: 404}, false},
		{"too many requests", &googleapi.Error{Code: 429}, true},
		{"service unavailable", &googleapi.Error{Code: 503}, true},
		{"internal", &googleapi.Error{Code: 500}, true},
		{
			"reason rate limit",
			&googleapi.Error{Code: 403, Errors: []googleapi.ErrorItem{{Reason: "rateLimitExceeded"}}},
			true,
		},
		{
			"400 job rate limit reason",
			&googleapi.Error{Code: 400, Errors: []googleapi.ErrorItem{{Reason: "jobRateLimitExceeded"}}},
			true,
		},
		{
			"400 message only",
			&googleapi.Error{
				Code:    400,
				Message: "Job exceeded rate limits: Your table exceeded quota for table update operations, jobRateLimitExceeded",
			},
			true,
		},
		{"wrapped", &googleapi.Error{Code: 429}, true},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if got := isRateLimitErr(tc.err); got != tc.want {
				t.Fatalf("isRateLimitErr(%v) = %v, want %v", tc.err, got, tc.want)
			}
		})
	}
}

// withFastBackoff shrinks the retry waits so tests do not sleep for seconds.
func withFastBackoff(t *testing.T) {
	t.Helper()
	base, maxB, maxR := bqBaseBackoff, bqMaxBackoff, bqMaxRetries
	bqBaseBackoff = time.Millisecond
	bqMaxBackoff = 5 * time.Millisecond
	bqMaxRetries = 4
	t.Cleanup(func() {
		bqBaseBackoff, bqMaxBackoff, bqMaxRetries = base, maxB, maxR
	})
}

func TestRetryOnRateLimitSucceedsAfterThrottle(t *testing.T) {
	withFastBackoff(t)
	rateErr := &googleapi.Error{Code: 429}
	attempts := 0
	job, err := retryOnRateLimit(context.Background(), func(context.Context) (*bigquery.Job, error) {
		attempts++
		if attempts < 3 {
			return nil, rateErr
		}
		return nil, nil
	})
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if job != nil {
		t.Fatalf("expected nil job sentinel, got %v", job)
	}
	if attempts != 3 {
		t.Fatalf("attempts = %d, want 3", attempts)
	}
}

func TestRetryOnRateLimitNonRetryable(t *testing.T) {
	withFastBackoff(t)
	other := errors.New("syntax error")
	attempts := 0
	_, err := retryOnRateLimit(context.Background(), func(context.Context) (*bigquery.Job, error) {
		attempts++
		return nil, other
	})
	if !errors.Is(err, other) {
		t.Fatalf("err = %v, want %v", err, other)
	}
	if attempts != 1 {
		t.Fatalf("attempts = %d, want 1 (no retry on non-rate-limit)", attempts)
	}
}

func TestRetryOnRateLimitExhausted(t *testing.T) {
	withFastBackoff(t)
	attempts := 0
	_, err := retryOnRateLimit(context.Background(), func(context.Context) (*bigquery.Job, error) {
		attempts++
		return nil, &googleapi.Error{Code: 429}
	})
	if err == nil {
		t.Fatal("expected exhaustion error")
	}
	if attempts != bqMaxRetries+1 {
		t.Fatalf("attempts = %d, want %d", attempts, bqMaxRetries+1)
	}
}

func TestRetryOnRateLimitContextCancel(t *testing.T) {
	withFastBackoff(t)
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	_, err := retryOnRateLimit(ctx, func(context.Context) (*bigquery.Job, error) {
		return nil, &googleapi.Error{Code: 429}
	})
	if !errors.Is(err, context.Canceled) {
		t.Fatalf("err = %v, want context.Canceled", err)
	}
}
