package sqltools

import (
	"encoding/json"
	"io"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const statusInvalid = "invalid"

// HandlerDeps bundles dependencies for SQL tools HTTP handlers.
type HandlerDeps struct {
	Access AccessConfig
	Client *engine.Client
}

// RegisterRoutes installs POST /api/emulator/sql/{format,parse,tokenize,complete}.
func RegisterRoutes(mux *http.ServeMux, deps HandlerDeps) {
	mux.HandleFunc("POST /api/emulator/sql/format", deps.handleFormat)
	mux.HandleFunc("POST /api/emulator/sql/parse", deps.handleParse)
	mux.HandleFunc("POST /api/emulator/sql/tokenize", deps.handleTokenize)
	mux.HandleFunc("POST /api/emulator/sql/complete", deps.handleComplete)
}

type errEnvelope struct {
	Code    int    `json:"code"`
	Status  string `json:"status"`
	Message string `json:"message"`
}

func writeJSON(w http.ResponseWriter, code int, v any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(code)
	_ = json.NewEncoder(w).Encode(v)
}

func writeAccessError(w http.ResponseWriter, err error) {
	if he, ok := err.(interface{ Status() int }); ok {
		writeJSON(w, he.Status(), errEnvelope{
			Code:    he.Status(),
			Status:  statusInvalid,
			Message: err.Error(),
		})
		return
	}
	writeJSON(w, http.StatusForbidden, errEnvelope{
		Code:    http.StatusForbidden,
		Status:  statusInvalid,
		Message: err.Error(),
	})
}

func writeGrpcError(w http.ResponseWriter, err error) {
	st, ok := status.FromError(err)
	if !ok {
		writeJSON(w, http.StatusInternalServerError, errEnvelope{
			Code:    http.StatusInternalServerError,
			Status:  statusInvalid,
			Message: err.Error(),
		})
		return
	}
	httpCode := http.StatusInternalServerError
	switch st.Code() {
	case codes.InvalidArgument:
		httpCode = http.StatusBadRequest
	case codes.NotFound:
		httpCode = http.StatusNotFound
	case codes.FailedPrecondition:
		httpCode = http.StatusPreconditionFailed
	case codes.Unimplemented:
		httpCode = http.StatusNotImplemented
	}
	writeJSON(w, httpCode, errEnvelope{
		Code:    httpCode,
		Status:  statusInvalid,
		Message: st.Message(),
	})
}

func (d HandlerDeps) requireClient(w http.ResponseWriter) bool {
	if d.Client == nil || d.Client.SQLTools == nil {
		writeJSON(w, http.StatusServiceUnavailable, errEnvelope{
			Code:    http.StatusServiceUnavailable,
			Status:  statusInvalid,
			Message: "sql tools engine client is not configured",
		})
		return false
	}
	return true
}

func (d HandlerDeps) readBody(w http.ResponseWriter, r *http.Request) ([]byte, bool) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code:    http.StatusBadRequest,
			Status:  statusInvalid,
			Message: "Could not read request body: " + err.Error(),
		})
		return nil, false
	}
	return body, true
}

type formatRequest struct {
	SQL               string `json:"sql"`
	Strict            bool   `json:"strict"`
	LineLengthLimit   int32  `json:"lineLengthLimit"`
	IndentationSpaces int32  `json:"indentationSpaces"`
}

type diagnosticWire struct {
	Line     int32  `json:"line"`
	Column   int32  `json:"column"`
	Message  string `json:"message"`
	Severity string `json:"severity"`
}

type formatResponse struct {
	FormattedSQL string           `json:"formattedSql"`
	Diagnostics  []diagnosticWire `json:"diagnostics,omitempty"`
}

func (d HandlerDeps) handleFormat(w http.ResponseWriter, r *http.Request) {
	if err := d.Access.CheckAccess(r); err != nil {
		writeAccessError(w, err)
		return
	}
	if !d.requireClient(w) {
		return
	}
	body, ok := d.readBody(w, r)
	if !ok {
		return
	}
	var req formatRequest
	if err := json.Unmarshal(body, &req); err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code: http.StatusBadRequest, Status: statusInvalid,
			Message: "invalid JSON: " + err.Error(),
		})
		return
	}
	if req.SQL == "" {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code: http.StatusBadRequest, Status: statusInvalid,
			Message: "sql is required",
		})
		return
	}
	resp, err := d.Client.SQLTools.Format(r.Context(), &enginepb.FormatSqlRequest{
		Sql:               req.SQL,
		Strict:            req.Strict,
		LineLengthLimit:   req.LineLengthLimit,
		IndentationSpaces: req.IndentationSpaces,
	})
	if err != nil {
		writeGrpcError(w, err)
		return
	}
	out := formatResponse{FormattedSQL: resp.GetFormattedSql()}
	for _, diag := range resp.GetDiagnostics() {
		out.Diagnostics = append(out.Diagnostics, diagnosticWire{
			Line: diag.GetLine(), Column: diag.GetColumn(),
			Message: diag.GetMessage(), Severity: diag.GetSeverity(),
		})
	}
	writeJSON(w, http.StatusOK, out)
}

type parseRequest struct {
	SQL string `json:"sql"`
}

type parseResponse struct {
	StatementKinds []string         `json:"statementKinds"`
	Diagnostics    []diagnosticWire `json:"diagnostics,omitempty"`
}

func (d HandlerDeps) handleParse(w http.ResponseWriter, r *http.Request) {
	if err := d.Access.CheckAccess(r); err != nil {
		writeAccessError(w, err)
		return
	}
	if !d.requireClient(w) {
		return
	}
	body, ok := d.readBody(w, r)
	if !ok {
		return
	}
	var req parseRequest
	if err := json.Unmarshal(body, &req); err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code: http.StatusBadRequest, Status: statusInvalid,
			Message: "invalid JSON: " + err.Error(),
		})
		return
	}
	resp, err := d.Client.SQLTools.Parse(r.Context(), &enginepb.ParseSqlRequest{Sql: req.SQL})
	if err != nil {
		writeGrpcError(w, err)
		return
	}
	out := parseResponse{StatementKinds: resp.GetStatementKinds()}
	for _, diag := range resp.GetDiagnostics() {
		out.Diagnostics = append(out.Diagnostics, diagnosticWire{
			Line: diag.GetLine(), Column: diag.GetColumn(),
			Message: diag.GetMessage(), Severity: diag.GetSeverity(),
		})
	}
	writeJSON(w, http.StatusOK, out)
}

type tokenizeRequest struct {
	SQL             string `json:"sql"`
	IncludeComments bool   `json:"includeComments"`
}

type tokenWire struct {
	Kind      string `json:"kind"`
	Image     string `json:"image"`
	StartByte int32  `json:"startByte"`
	EndByte   int32  `json:"endByte"`
}

type tokenizeResponse struct {
	Tokens      []tokenWire      `json:"tokens"`
	Diagnostics []diagnosticWire `json:"diagnostics,omitempty"`
}

func (d HandlerDeps) handleTokenize(w http.ResponseWriter, r *http.Request) {
	if err := d.Access.CheckAccess(r); err != nil {
		writeAccessError(w, err)
		return
	}
	if !d.requireClient(w) {
		return
	}
	body, ok := d.readBody(w, r)
	if !ok {
		return
	}
	var req tokenizeRequest
	if err := json.Unmarshal(body, &req); err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code: http.StatusBadRequest, Status: statusInvalid,
			Message: "invalid JSON: " + err.Error(),
		})
		return
	}
	resp, err := d.Client.SQLTools.Tokenize(r.Context(), &enginepb.TokenizeSqlRequest{
		Sql: req.SQL, IncludeComments: req.IncludeComments,
	})
	if err != nil {
		writeGrpcError(w, err)
		return
	}
	out := tokenizeResponse{}
	for _, tok := range resp.GetTokens() {
		out.Tokens = append(out.Tokens, tokenWire{
			Kind: tok.GetKind(), Image: tok.GetImage(),
			StartByte: tok.GetStartByte(), EndByte: tok.GetEndByte(),
		})
	}
	for _, diag := range resp.GetDiagnostics() {
		out.Diagnostics = append(out.Diagnostics, diagnosticWire{
			Line: diag.GetLine(), Column: diag.GetColumn(),
			Message: diag.GetMessage(), Severity: diag.GetSeverity(),
		})
	}
	writeJSON(w, http.StatusOK, out)
}

type completeRequest struct {
	ProjectID        string `json:"projectId"`
	DefaultDatasetID string `json:"defaultDatasetId"`
	SQL              string `json:"sql"`
	CursorByteOffset int32  `json:"cursorByteOffset"`
}

type candidateWire struct {
	Label      string `json:"label"`
	Kind       string `json:"kind"`
	InsertText string `json:"insertText"`
}

type completeResponse struct {
	Candidates       []candidateWire `json:"candidates"`
	ReplacementStart int32           `json:"replacementStart"`
	ReplacementEnd   int32           `json:"replacementEnd"`
}

func (d HandlerDeps) handleComplete(w http.ResponseWriter, r *http.Request) {
	if err := d.Access.CheckAccess(r); err != nil {
		writeAccessError(w, err)
		return
	}
	if !d.requireClient(w) {
		return
	}
	body, ok := d.readBody(w, r)
	if !ok {
		return
	}
	var req completeRequest
	if err := json.Unmarshal(body, &req); err != nil {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code: http.StatusBadRequest, Status: statusInvalid,
			Message: "invalid JSON: " + err.Error(),
		})
		return
	}
	if req.ProjectID == "" {
		writeJSON(w, http.StatusBadRequest, errEnvelope{
			Code: http.StatusBadRequest, Status: statusInvalid,
			Message: "projectId is required",
		})
		return
	}
	resp, err := d.Client.SQLTools.Complete(r.Context(), &enginepb.CompleteSqlRequest{
		ProjectId:        req.ProjectID,
		DefaultDatasetId: req.DefaultDatasetID,
		Sql:              req.SQL,
		CursorByteOffset: req.CursorByteOffset,
	})
	if err != nil {
		writeGrpcError(w, err)
		return
	}
	out := completeResponse{
		ReplacementStart: resp.GetReplacementStart(),
		ReplacementEnd:   resp.GetReplacementEnd(),
	}
	for _, c := range resp.GetCandidates() {
		out.Candidates = append(out.Candidates, candidateWire{
			Label: c.GetLabel(), Kind: c.GetKind(), InsertText: c.GetInsertText(),
		})
	}
	writeJSON(w, http.StatusOK, out)
}
