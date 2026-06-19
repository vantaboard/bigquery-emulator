package sqltools

import (
	"math"
	"strings"
	"unicode/utf16"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

const (
	offsetUnitUTF8  = "utf8"
	offsetUnitUTF16 = "utf16"
	sqlToolsVersion = "1.0"
)

func normalizeOffsetUnit(unit string) string {
	switch strings.ToLower(strings.TrimSpace(unit)) {
	case offsetUnitUTF16:
		return offsetUnitUTF16
	default:
		return offsetUnitUTF8
	}
}

func utf8ByteOffsetToCodeUnit(sql string, byteOffset int) int {
	if byteOffset <= 0 {
		return 0
	}
	if byteOffset > len(sql) {
		byteOffset = len(sql)
	}
	return len(utf16.Encode([]rune(sql[:byteOffset])))
}

func codeUnitToUtf8ByteOffset(sql string, codeUnit int) int {
	if codeUnit <= 0 {
		return 0
	}
	units := utf16.Encode([]rune(sql))
	if codeUnit > len(units) {
		codeUnit = len(units)
	}
	if codeUnit == 0 {
		return 0
	}
	return len(string(utf16.Decode(units[:codeUnit])))
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

func convertCursorToUTF8(sql string, unit string, cursor int32) int32 {
	if normalizeOffsetUnit(unit) != offsetUnitUTF16 {
		return cursor
	}
	return int32FromInt(codeUnitToUtf8ByteOffset(sql, int(cursor)))
}

func convertReplacementFromUTF8(sql string, unit string, start, end int32) (int32, int32) {
	if normalizeOffsetUnit(unit) != offsetUnitUTF16 {
		return start, end
	}
	return int32FromInt(utf8ByteOffsetToCodeUnit(sql, int(start))),
		int32FromInt(utf8ByteOffsetToCodeUnit(sql, int(end)))
}

type diagnosticWire struct {
	Line       int32  `json:"line"`
	Column     int32  `json:"column"`
	Message    string `json:"message"`
	Severity   string `json:"severity"`
	EndLine    int32  `json:"endLine,omitempty"`
	EndColumn  int32  `json:"endColumn,omitempty"`
	StartByte  int32  `json:"startByte,omitempty"`
	EndByte    int32  `json:"endByte,omitempty"`
	StartUTF16 int32  `json:"startUtf16,omitempty"`
	EndUTF16   int32  `json:"endUtf16,omitempty"`
}

func diagnosticFromProto(sql string, unit string, diag *enginepb.SqlDiagnostic) diagnosticWire {
	out := diagnosticWire{
		Line:      diag.GetLine(),
		Column:    diag.GetColumn(),
		Message:   diag.GetMessage(),
		Severity:  diag.GetSeverity(),
		EndLine:   diag.GetEndLine(),
		EndColumn: diag.GetEndColumn(),
		StartByte: diag.GetStartByte(),
		EndByte:   diag.GetEndByte(),
	}
	if normalizeOffsetUnit(unit) == offsetUnitUTF16 && sql != "" {
		if out.StartByte >= 0 {
			out.StartUTF16 = int32FromInt(utf8ByteOffsetToCodeUnit(sql, int(out.StartByte)))
		}
		if out.EndByte >= 0 {
			out.EndUTF16 = int32FromInt(utf8ByteOffsetToCodeUnit(sql, int(out.EndByte)))
		}
	}
	return out
}

type tokenWire struct {
	Kind       string `json:"kind"`
	Image      string `json:"image"`
	StartByte  int32  `json:"startByte"`
	EndByte    int32  `json:"endByte"`
	StartUTF16 int32  `json:"startUtf16,omitempty"`
	EndUTF16   int32  `json:"endUtf16,omitempty"`
}

func tokenFromProto(sql string, unit string, tok *enginepb.SqlToken) tokenWire {
	out := tokenWire{
		Kind:      tok.GetKind(),
		Image:     tok.GetImage(),
		StartByte: tok.GetStartByte(),
		EndByte:   tok.GetEndByte(),
	}
	if normalizeOffsetUnit(unit) == offsetUnitUTF16 && sql != "" {
		out.StartUTF16 = int32FromInt(utf8ByteOffsetToCodeUnit(sql, int(out.StartByte)))
		out.EndUTF16 = int32FromInt(utf8ByteOffsetToCodeUnit(sql, int(out.EndByte)))
	}
	return out
}

type offsetRequest struct {
	OffsetUnit string `json:"offsetUnit"`
}
