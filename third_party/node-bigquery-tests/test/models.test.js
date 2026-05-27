// Copyright 2019 Google LLC
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

const {BigQuery} = require('@google-cloud/bigquery');
const {assert} = require('chai');
const {describe, it, before, beforeEach, after} = require('mocha');
const {randomUUID} = require('crypto');
const {warnIgnoringNotFound} = require('../lib/warnIgnoringNotFound');
const {execSample} = require('../lib/execSample');

const getModel = require('../getModel.js').main;
const listModels = require('../listModels.js').main;
const updateModel = require('../updateModel.js').main;
const createModel = require('../createModel.js').main;
const deleteModel = require('../deleteModel.js').main;

const GCLOUD_TESTS_PREFIX = 'nodejs_samples_tests_models';

const bigquery = new BigQuery();

describe('Models', function () {
  // Increase timeout to accommodate model creation.
  this.timeout(300000);
  const datasetId = `${GCLOUD_TESTS_PREFIX}_${randomUUID()}`.replace(
    /-/gi,
    '_',
  );
  const modelId = `${GCLOUD_TESTS_PREFIX}_${randomUUID()}`.replace(/-/gi, '_');

  before(async () => {
    const query = `CREATE OR REPLACE MODEL \`${datasetId}.${modelId}\`
        OPTIONS(model_type='logistic_reg') AS
        SELECT
          IF(totals.transactions IS NULL, 0, 1) AS label,
          IFNULL(device.operatingSystem, "") AS os,
          device.isMobile AS is_mobile,
          IFNULL(geoNetwork.country, "") AS country,
          IFNULL(totals.pageviews, 0) AS pageviews
        FROM
          \`bigquery-public-data.google_analytics_sample.ga_sessions_*\`
        WHERE
          _TABLE_SUFFIX BETWEEN '20160801' AND '20170631'
        LIMIT  100000;`;

    const datasetOptions = {
      location: 'US',
    };

    const queryOptions = {
      query: query,
    };

    await bigquery.createDataset(datasetId, datasetOptions);

    // Run query to create a model
    const [job] = await bigquery.createQueryJob(queryOptions);

    // Wait for the query to finish
    await job.getQueryResults();
  });

  beforeEach(async function () {
    this.currentTest.retries(2);
  });

  after(async () => {
    await bigquery
      .dataset(datasetId)
      .delete({force: true})
      .catch(warnIgnoringNotFound);
  });

  it('should retrieve a model if it exists', async () => {
    const output = await execSample(getModel, datasetId, modelId);
    assert.include(output, 'Model:');
    assert.include(output, datasetId && modelId);
  });

  it('should list models', async () => {
    const output = await execSample(listModels, datasetId);
    assert.include(output, 'Models:');
    assert.include(output, datasetId);
  });

  it('should list models streaming', async () => {
    const output = await execSample(getModel, datasetId, modelId);
    assert.include(output, modelId);
  });

  it("should update model's metadata", async () => {
    const output = await execSample(updateModel, datasetId, modelId);
    assert.include(output, `${modelId} description: A really great model.`);
  });
});

describe('Create/Delete Model', () => {
  const datasetId = `${GCLOUD_TESTS_PREFIX}_delete_${randomUUID()}`.replace(
    /-/gi,
    '_',
  );
  const modelId = `${GCLOUD_TESTS_PREFIX}_delete_${randomUUID()}`.replace(
    /-/gi,
    '_',
  );

  before(async () => {
    const datasetOptions = {
      location: 'US',
    };
    await bigquery.createDataset(datasetId, datasetOptions);
  });

  beforeEach(async function () {
    this.currentTest.retries(2);
  });

  after(async () => {
    await bigquery
      .dataset(datasetId)
      .delete({force: true})
      .catch(warnIgnoringNotFound);
  });

  it('should create a model', async () => {
    const output = await execSample(createModel, datasetId, modelId);
    assert.include(output, `Model ${modelId} created.`);
    const [exists] = await bigquery.dataset(datasetId).model(modelId).exists();
    assert.strictEqual(exists, true);
  });

  it('should delete a model', async () => {
    const output = await execSample(deleteModel, datasetId, modelId);
    assert.include(output, `Model ${modelId} deleted.`);
    const [exists] = await bigquery.dataset(datasetId).model(modelId).exists();
    assert.strictEqual(exists, false);
  });
});
