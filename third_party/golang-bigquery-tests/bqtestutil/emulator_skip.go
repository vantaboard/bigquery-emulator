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

package bqtestutil

import (
	"os"
	"strings"
	"testing"
)

// SkipEmulatorBQML skips tests that require BigQuery ML (CREATE MODEL, model
// export, model metadata) when BIGQUERY_EMULATOR_HOST targets the local emulator.
func SkipEmulatorBQML(t *testing.T) {
	t.Helper()
	if strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST")) != "" {
		t.Skip("BigQuery ML is not implemented by the bigquery-emulator")
	}
}

// SkipEmulatorManagedWriterDefaultStream skips DefaultStream append tests when
// the emulator cannot yet decode all proto row shapes (e.g. extreme BIGNUMERIC).
func SkipEmulatorManagedWriterDefaultStream(t *testing.T) {
	t.Helper()
	if strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST")) != "" {
		t.Skip("ManagedWriter DefaultStream proto append with full type matrix is not yet supported by the emulator")
	}
}

// SkipEmulatorNumericAggregateQuery skips queries whose result includes
// SUM/aggregates over NUMERIC columns when the emulator host is set.
func SkipEmulatorNumericAggregateQuery(t *testing.T) {
	t.Helper()
	if strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST")) != "" {
		t.Skip("NUMERIC aggregate query results are not yet fully supported by the emulator")
	}
}
