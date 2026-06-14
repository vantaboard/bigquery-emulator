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

// SkipEmulatorBQML skips tests that require real BigQuery ML semantics
// (model metadata APIs, non-null prediction/metric values) when
// BIGQUERY_EMULATOR_HOST targets the local emulator. CREATE MODEL and
// ML.PREDICT / ML.EVALUATE / ML.FORECAST return schema-correct
// placeholders locally, but these samples still need catalog model
// CRUD or real inference results.
func SkipEmulatorBQML(t *testing.T) {
	t.Helper()
	if strings.TrimSpace(os.Getenv("BIGQUERY_EMULATOR_HOST")) != "" {
		t.Skip("BigQuery ML samples need model metadata or real predictions; emulator returns placeholders only")
	}
}
