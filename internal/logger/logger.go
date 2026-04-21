package logger

import (
	"context"
	"log/slog"
	"net/http"
)

type loggerKey struct{}
type httpRequestKey struct{}
type logFieldsKey struct{}

func WithLogger(ctx context.Context, l *slog.Logger) context.Context {
	return context.WithValue(ctx, loggerKey{}, l)
}

func Logger(ctx context.Context) *slog.Logger {
	l, _ := ctx.Value(loggerKey{}).(*slog.Logger)
	if l == nil {
		return slog.Default()
	}
	return l
}

// WithHTTPRequest attaches stable HTTP metadata for error and access logs.
func WithHTTPRequest(ctx context.Context, r *http.Request) context.Context {
	return context.WithValue(ctx, httpRequestKey{}, httpRequestAttrs(r))
}

func httpRequestAttrs(r *http.Request) []slog.Attr {
	if r == nil {
		return nil
	}
	return []slog.Attr{
		slog.String("http.method", r.Method),
		slog.String("http.path", r.URL.Path),
		slog.String("http.query", truncateRunes(r.URL.RawQuery, 512)),
		slog.String("remote_addr", r.RemoteAddr),
		slog.String("http.host", r.Host),
	}
}

// HTTPRequestAttrs returns attrs previously stored with WithHTTPRequest.
func HTTPRequestAttrs(ctx context.Context) []slog.Attr {
	v := ctx.Value(httpRequestKey{})
	if v == nil {
		return nil
	}
	return v.([]slog.Attr)
}

// WithLogFields merges additional structured attrs into the request context for error logging.
func WithLogFields(ctx context.Context, attrs ...slog.Attr) context.Context {
	var prev []slog.Attr
	if v := ctx.Value(logFieldsKey{}); v != nil {
		prev = v.([]slog.Attr)
	}
	merged := make([]slog.Attr, 0, len(prev)+len(attrs))
	merged = append(merged, prev...)
	merged = append(merged, attrs...)
	return context.WithValue(ctx, logFieldsKey{}, merged)
}

// LogFields returns extra attrs from WithLogFields.
func LogFields(ctx context.Context) []slog.Attr {
	if v := ctx.Value(logFieldsKey{}); v != nil {
		return v.([]slog.Attr)
	}
	return nil
}

func truncateRunes(s string, max int) string {
	if max <= 0 || len(s) <= max {
		return s
	}
	return s[:max] + "…"
}
