// Copyright 2020 Google LLC
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

const {assert} = require('chai');
const {describe, it} = require('mocha');

const {BigQuery} = require('@google-cloud/bigquery');
const {getBigQueryClientOptions} = require('../lib/bigqueryEmulatorClientOptions');
const {execSample} = require('../lib/execSample');

const setUserAgent = require('../setUserAgent.js').main;
const setClientEndpoint = require('../setClientEndpoint.js').main;

const emulatorHost = process.env.BIGQUERY_EMULATOR_HOST;

describe('Client', () => {
  it('should should set providedUserAgent', async () => {
    const output = await execSample(setUserAgent);
    assert.match(output, /User agent:/);
    assert.match(output, /my-user-agent/);
  });
  it('should should set client endpoint', async () => {
    let output = await execSample(setClientEndpoint, 'us-east4');
    assert.match(output, /API Endpoint:/);
    if (emulatorHost) {
      assert.match(output, /http:\/\//);
    } else {
      assert.match(output, /https:\/\/us-east4-bigquery.googleapis.com/);
    }

    output = await execSample(setClientEndpoint, 'eu');
    assert.match(output, /API Endpoint:/);
    if (emulatorHost) {
      assert.match(output, /http:\/\//);
    } else {
      assert.match(output, /https:\/\/eu-bigquery.googleapis.com/);
    }
  });

  if (emulatorHost) {
    it('should run SELECT 1 against the emulator (HTTP smoke)', async function () {
      this.timeout(20000);
      const bigquery = new BigQuery({...getBigQueryClientOptions()});
      const [rows] = await bigquery.query({query: 'SELECT 1 AS n', location: 'US'});
      assert.strictEqual(rows[0].n, 1);
    });
  }
});
