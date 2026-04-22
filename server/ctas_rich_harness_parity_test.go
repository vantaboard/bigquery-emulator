package server_test

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/internal/contentdata"
)

func TestHarnessTortoiseRichCTASSQL_isCTASQuery(t *testing.T) {
	sql := harnessTortoiseRichCTASSQL("harnessds", "t", 2, 2)
	if !contentdata.IsCTASQuery(sql) {
		t.Fatalf("harnessTortoiseRichCTASSQL must be classified as CTAS (handler uses QueryCTASInPlace when Dst is set):\n%s", sql)
	}
}
