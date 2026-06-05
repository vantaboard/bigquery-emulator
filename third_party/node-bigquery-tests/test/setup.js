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

// When BIGQUERY_EMULATOR_HOST is set, skip entire Mocha files and individual
// tests that require BQML, legacy SQL, or bigquery-public-data catalog
// fixtures (see third_party/README.md and docs/ENGINE_POLICY.md).
const EMULATOR_SKIP_FILES = /(?:^|[\\/])test[\\/](?:models|jobs)\.test\.js$/;

const EMULATOR_SKIP_TITLE_PATTERNS = [
  /legacy SQL/i,
  /different project/i,
];

function patchDescribeForEmulatorSkips() {
  if (!process.env.BIGQUERY_EMULATOR_HOST) {
    return;
  }
  // Ensure Mocha has installed the full describe API (only/skip) before
  // we wrap it — setup.js is --require'd before the runner attaches .only.
  require('mocha');
  const originalDescribe = global.describe;
  if (typeof originalDescribe !== 'function') {
    return;
  }
  function describeWithEmulatorSkips(name, fn) {
    const err = new Error();
    const stack = err.stack || '';
    if (EMULATOR_SKIP_FILES.test(stack)) {
      return originalDescribe.skip(name, fn);
    }
    return originalDescribe(name, fn);
  }
  describeWithEmulatorSkips.only = originalDescribe.only;
  describeWithEmulatorSkips.skip = originalDescribe.skip;
  global.describe = describeWithEmulatorSkips;
}

patchDescribeForEmulatorSkips();

exports.mochaHooks = {
  beforeEach() {
    if (!process.env.BIGQUERY_EMULATOR_HOST) {
      return;
    }
    const title = this.currentTest?.fullTitle?.() || '';
    for (const pattern of EMULATOR_SKIP_TITLE_PATTERNS) {
      if (pattern.test(title)) {
        this.skip();
      }
    }
  },
};
