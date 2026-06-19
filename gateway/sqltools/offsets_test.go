package sqltools

import (
	"testing"
)

func TestUTF16OffsetRoundTrip(t *testing.T) {
	sql := "SELECT 'é'"
	byteOffset := len("SELECT '")
	got := utf8ByteOffsetToCodeUnit(sql, byteOffset)
	if got != 8 {
		t.Fatalf("utf8ByteOffsetToCodeUnit(%q, %d) = %d, want 8", sql, byteOffset, got)
	}
	back := codeUnitToUtf8ByteOffset(sql, got)
	if back != byteOffset {
		t.Fatalf("round trip = %d, want %d", back, byteOffset)
	}
}

func TestConvertCursorToUTF8(t *testing.T) {
	sql := "SELECT 'é'"
	cursor := int32(8)
	got := convertCursorToUTF8(sql, offsetUnitUTF16, cursor)
	if got != int32(len("SELECT '")) {
		t.Fatalf("convertCursorToUTF8 = %d, want %d", got, len("SELECT '"))
	}
}
