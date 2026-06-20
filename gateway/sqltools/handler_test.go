package sqltools

import (
	"bytes"
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
)

type stubSQLToolsClient struct {
	parseCalled bool
}

func (s *stubSQLToolsClient) Format(
	context.Context, *enginepb.FormatSqlRequest, ...grpc.CallOption,
) (*enginepb.FormatSqlResponse, error) {
	return &enginepb.FormatSqlResponse{}, nil
}

func (s *stubSQLToolsClient) Parse(
	_ context.Context, _ *enginepb.ParseSqlRequest, _ ...grpc.CallOption,
) (*enginepb.ParseSqlResponse, error) {
	s.parseCalled = true
	return &enginepb.ParseSqlResponse{}, nil
}

func (s *stubSQLToolsClient) Tokenize(
	context.Context, *enginepb.TokenizeSqlRequest, ...grpc.CallOption,
) (*enginepb.TokenizeSqlResponse, error) {
	return &enginepb.TokenizeSqlResponse{}, nil
}

func (s *stubSQLToolsClient) Complete(
	context.Context, *enginepb.CompleteSqlRequest, ...grpc.CallOption,
) (*enginepb.CompleteSqlResponse, error) {
	return &enginepb.CompleteSqlResponse{}, nil
}

func (s *stubSQLToolsClient) Analyze(
	context.Context, *enginepb.AnalyzeSqlRequest, ...grpc.CallOption,
) (*enginepb.AnalyzeSqlResponse, error) {
	return &enginepb.AnalyzeSqlResponse{}, nil
}

func TestDiagnosticStartByteZeroSurvivesJSON(t *testing.T) {
	diag := diagnosticFromProto("SELECT 1", offsetUnitUTF8, &enginepb.SqlDiagnostic{
		Line: 1, Column: 1, Message: "err", Severity: "error",
		StartByte: 0, EndByte: 6,
	})
	b, err := json.Marshal(diag)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}
	body := string(b)
	if !strings.Contains(body, `"startByte":0`) {
		t.Fatalf("body=%q; expected startByte:0", body)
	}
	if !strings.Contains(body, `"endByte":6`) {
		t.Fatalf("body=%q; expected endByte:6", body)
	}
}

func TestDiagnosticStartUtf16ZeroWhenRequested(t *testing.T) {
	diag := diagnosticFromProto("SELECT 1", offsetUnitUTF16, &enginepb.SqlDiagnostic{
		Line: 1, Column: 1, Message: "err", Severity: "error",
		StartByte: 0, EndByte: 6,
	})
	b, err := json.Marshal(diag)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}
	body := string(b)
	if !strings.Contains(body, `"startUtf16":0`) {
		t.Fatalf("body=%q; expected startUtf16:0", body)
	}
	if !strings.Contains(body, `"endUtf16":6`) {
		t.Fatalf("body=%q; expected endUtf16:6", body)
	}
}

func TestTokenStartUtf16ZeroWhenRequested(t *testing.T) {
	tok := tokenFromProto("SELECT 1", offsetUnitUTF16, &enginepb.SqlToken{
		Kind: "keyword", Image: "SELECT", StartByte: 0, EndByte: 6,
	})
	b, err := json.Marshal(tok)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}
	body := string(b)
	if !strings.Contains(body, `"startUtf16":0`) {
		t.Fatalf("body=%q; expected startUtf16:0", body)
	}
}

func TestParseEmptySQLReturns400(t *testing.T) {
	stub := &stubSQLToolsClient{}
	deps := HandlerDeps{
		Access: AccessConfig{AllowRemote: true},
		Client: &engine.Client{SQLTools: stub},
	}
	body := bytes.NewBufferString(`{"sql":""}`)
	req := httptest.NewRequest(http.MethodPost, "/api/emulator/sql/parse", body)
	req.RemoteAddr = "127.0.0.1:1234"
	rec := httptest.NewRecorder()
	deps.handleParse(rec, req)
	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status=%d, want 400; body=%s", rec.Code, rec.Body.String())
	}
	if stub.parseCalled {
		t.Fatal("Parse RPC should not run for empty sql")
	}
	if !strings.Contains(rec.Body.String(), "sql is required") {
		t.Fatalf("body=%q; expected sql is required", rec.Body.String())
	}
}
