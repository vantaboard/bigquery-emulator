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

// Loaded first by Mocha (--require) so every test file sees a project id when
// GOLANG_SAMPLES_PROJECT_ID / NODE_SAMPLES_PROJECT_ID / emulator defaults apply.
require('../lib/suppressMetadataLookupWarnings');
const {ensureGoogleAuthProjectEnv} = require('../lib/sampleProjectEnv');
ensureGoogleAuthProjectEnv();

// When talking to the local BigQuery emulator, TCP Host is loopback but tests may set
// `apiEndpoint` to a regional Google hostname (e.g. us-east4-bigquery.googleapis.com).
// Mirror that into X-BigQuery-Emulator-Api-Region so api/apiregion/policy.go can enforce
// dataset location parity without editing each test file.
const headerBigQueryEmulatorAPIRegion = 'X-BigQuery-Emulator-Api-Region';

function patchBigQueryConstructorForEmulatorRegion() {
  if (!process.env.BIGQUERY_EMULATOR_HOST) {
    return;
  }
  const {endpointRegionFromApiEndpoint} = require('../lib/endpointRegionFromApiEndpoint');
  const corePath = require.resolve('@google-cloud/bigquery/build/src/bigquery.js');
  const core = require(corePath);
  const Original = core.BigQuery;
  if (Original.__goGooglesqlEmulatorRegionPatched) {
    return;
  }
  class BigQueryWithEmulatorRegion extends Original {
    constructor(opts = {}) {
      const region = endpointRegionFromApiEndpoint(opts.apiEndpoint);
      const interceptors_ = [...(opts.interceptors_ || [])];
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
      super(Object.assign({}, opts, {interceptors_}));
    }
  }
  BigQueryWithEmulatorRegion.__goGooglesqlEmulatorRegionPatched = true;
  core.BigQuery = BigQueryWithEmulatorRegion;
}

patchBigQueryConstructorForEmulatorRegion();
