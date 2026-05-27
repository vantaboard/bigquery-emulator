// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

'use strict';

/**
 * Aligns BIGQUERY_EMULATOR_HOST with @google-cloud/bigquery and the go-googlesql
 * emulator: schemeless host:port is rewritten to http://… so the client does not
 * default to https:// against a plain HTTP listener.
 */
function normalizeBigQueryEmulatorHost() {
  const raw = process.env.BIGQUERY_EMULATOR_HOST;
  if (raw === undefined || raw === '') {
    return;
  }
  const trimmed = String(raw).trim();
  if (!trimmed) {
    return;
  }
  if (/^[a-zA-Z][a-zA-Z0-9+.-]*:\/\//.test(trimmed)) {
    return;
  }
  process.env.BIGQUERY_EMULATOR_HOST = `http://${trimmed}`;
}

module.exports = {normalizeBigQueryEmulatorHost};
