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

const {normalizeBigQueryEmulatorHost} = require('./normalizeBigQueryEmulatorHost');
const {getSampleProjectId} = require('./sampleProjectEnv');

// Matches the gateway regional policy (X-BigQuery-Emulator-Api-Region).
const headerBigQueryEmulatorAPIRegion = 'X-BigQuery-Emulator-Api-Region';

/**
 * Returns constructor options to merge into `new BigQuery({ ... })`.
 * When BIGQUERY_EMULATOR_HOST is set (after normalizing scheme in env), adds
 * optional region interceptor. Includes explicit projectId whenever
 * getSampleProjectId() resolves (same precedence as test/setup.js for ADC).
 */
function getBigQueryClientOptions() {
  normalizeBigQueryEmulatorHost();
  const projectId = getSampleProjectId();
  const base = projectId ? {projectId} : {};
  if (!process.env.BIGQUERY_EMULATOR_HOST) {
    return base;
  }
  const region = process.env.BIGQUERY_EMULATOR_CLIENT_API_REGION;
  const interceptors_ = [];
  if (region) {
    interceptors_.push({
      request(reqOpts) {
        reqOpts.headers = Object.assign({}, reqOpts.headers, {
          [headerBigQueryEmulatorAPIRegion]: region,
        });
        return reqOpts;
      },
    });
  }
  return {
    ...base,
    ...(interceptors_.length ? {interceptors_} : {}),
  };
}

module.exports = {getBigQueryClientOptions};
