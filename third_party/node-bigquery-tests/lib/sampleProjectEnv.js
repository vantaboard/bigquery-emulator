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
 * Resolves a BigQuery project id for samples / tests. Order matches go-googlesql
 * third_party docs: NODE_SAMPLES_PROJECT_ID, GOLANG_SAMPLES_PROJECT_ID, then
 * standard Google env vars. When BIGQUERY_EMULATOR_HOST is set and nothing else
 * provides a project, defaults to EMULATOR_PROJECT_ID or "dev".
 *
 * @returns {string}
 */
function getSampleProjectId() {
  const pick = (...keys) => {
    for (const k of keys) {
      const v = process.env[k];
      if (v !== undefined && String(v).trim() !== '') {
        return String(v).trim();
      }
    }
    return '';
  };
  const id = pick(
    'NODE_SAMPLES_PROJECT_ID',
    'GOLANG_SAMPLES_PROJECT_ID',
    'GOOGLE_CLOUD_PROJECT',
    'GCLOUD_PROJECT',
  );
  if (id) {
    return id;
  }
  if (process.env.BIGQUERY_EMULATOR_HOST) {
    return pick('EMULATOR_PROJECT_ID') || 'dev';
  }
  return '';
}

/**
 * Ensures google-auth-library can find a project id without querying metadata
 * servers: copies the resolved sample project into GOOGLE_CLOUD_PROJECT and
 * GCLOUD_PROJECT when those are unset.
 */
function ensureGoogleAuthProjectEnv() {
  const {normalizeBigQueryEmulatorHost} = require('./normalizeBigQueryEmulatorHost');
  normalizeBigQueryEmulatorHost();
  const pid = getSampleProjectId();
  if (!pid) {
    return;
  }
  if (!process.env.GOOGLE_CLOUD_PROJECT) {
    process.env.GOOGLE_CLOUD_PROJECT = pid;
  }
  if (!process.env.GCLOUD_PROJECT) {
    process.env.GCLOUD_PROJECT = pid;
  }
}

module.exports = {getSampleProjectId, ensureGoogleAuthProjectEnv};
