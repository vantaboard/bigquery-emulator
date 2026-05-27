// Copyright 2017 Google LLC
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
const {describe, it, after, before, beforeEach} = require('mocha');
const {randomUUID} = require('crypto');
const {warnIgnoringNotFound} = require('../lib/warnIgnoringNotFound');
const {execSample} = require('../lib/execSample');

const createDataset = require('../createDataset.js').main;
const listDatasets = require('../listDatasets.js').main;
const getDataset = require('../getDataset.js').main;
const updateDatasetDescription =
  require('../updateDatasetDescription.js').main;
const updateDatasetExpiration = require('../updateDatasetExpiration.js').main;
const labelDataset = require('../labelDataset.js').main;
const getDatasetLabels = require('../getDatasetLabels.js').main;
const deleteLabelDataset = require('../deleteLabelDataset.js').main;
const updateDatasetAccess = require('../updateDatasetAccess.js').main;
const listDatasetsByLabel = require('../listDatasetsByLabel.js').main;
const deleteDataset = require('../deleteDataset.js').main;
const GCLOUD_TESTS_PREFIX = 'nodejs_samples_tests';
// Only delete datasets owned by this file — avoids wiping parallel suites
// (models, queries, …) that share the nodejs_samples_tests prefix.
const DATASETS_SUITE_ID_PREFIX = `${GCLOUD_TESTS_PREFIX}_datasets`;
const datasetId = `${DATASETS_SUITE_ID_PREFIX}_${randomUUID()}`.replace(
  /-/gi,
  '_',
);

const bigquery = new BigQuery();

describe('Datasets', () => {
  before(async () => {
    // Delete any stale datasets from samples tests
    await deleteDatasets();
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

  it('should create a dataset', async () => {
    const output = await execSample(createDataset, datasetId);
    assert.include(output, `Dataset ${datasetId} created.`);
    const [exists] = await bigquery.dataset(datasetId).exists();
    assert.ok(exists);
  });

  it('should create a dataset using a regional endpoint', async () => {
    const euBigquery = new BigQuery({
      apiEndpoint: 'eu-bigquery.googleapis.com',
    });
    const euDatasetId = datasetId + '_eu';
    await euBigquery.createDataset(euDatasetId, {
      location: 'eu',
    });
    const [exists] = await euBigquery.dataset(euDatasetId).exists();
    assert.ok(exists);
  });

  it('should fail to create a dataset using a different region from the client endpoint', async () => {
    const usEast4Bigquery = new BigQuery({
      apiEndpoint: 'us-east4-bigquery.googleapis.com',
    });
    const usDatasetId = datasetId + '_us';
    let error;
    try {
      await usEast4Bigquery.createDataset(usDatasetId, {
        location: 'us-central1',
      });
    } catch (err) {
      error = err;
    }
    assert.isNotNull(error);
    assert.include(error.message, 'Invalid storage region');
  });

  it('should create/update a dataset with a different default collation', async () => {
    const bigquery = new BigQuery({});
    const collationDatasetId = datasetId + '_collation_test';
    await bigquery.createDataset(collationDatasetId, {
      defaultCollation: 'und:ci',
    });
    const dataset = await bigquery.dataset(collationDatasetId);
    const [exists] = await dataset.exists();
    assert.ok(exists);
    let [md] = await dataset.getMetadata();
    assert.equal(md.defaultCollation, 'und:ci');
    md.defaultCollation = '';
    await dataset.setMetadata(md);
    [md] = await dataset.getMetadata();
    assert.equal(md.defaultCollation, '');
  });

  it('should list datasets', async () => {
    const output = await execSample(listDatasets);
    assert.match(output, /Datasets:/);
    assert.match(output, new RegExp(datasetId));
  });

  it('should list datasets on a different project', async () => {
    const output = await execSample(listDatasets, 'bigquery-public-data');
    assert.match(output, /Datasets:/);
    assert.match(output, new RegExp('usa_names'));
  });

  it('should retrieve a dataset if it exists', async () => {
    const output = await execSample(getDataset, datasetId);
    assert.include(output, 'Dataset:');
    assert.include(output, datasetId);
  });

  it("should update dataset's description", async () => {
    const output = await execSample(updateDatasetDescription, datasetId);
    assert.include(
      output,
      `${datasetId} description: New dataset description.`,
    );
  });

  it("should update dataset's expiration", async () => {
    const output = await execSample(updateDatasetExpiration, datasetId);
    assert.include(output, `${datasetId} expiration: 86400000`);
  });

  it('should add label to a dataset', async () => {
    const output = await execSample(labelDataset, datasetId);
    assert.include(output, `${datasetId} labels:`);
    assert.include(output, "{ color: 'green' }");
  });

  it("should list a dataset's labels", async () => {
    const output = await execSample(getDatasetLabels, datasetId);
    assert.include(output, `${datasetId} Labels:`);
    assert.include(output, 'color: green');
  });

  it('should delete a label from a dataset', async () => {
    const output = await execSample(deleteLabelDataset, datasetId);
    assert.include(output, `${datasetId} labels:`);
    assert.include(output, 'undefined');
  });

  it("should update dataset's access", async () => {
    const output = await execSample(updateDatasetAccess, datasetId);
    assert.include(output, "role: 'READER'");
    assert.include(output, "userByEmail: 'sample.bigquery.dev@gmail.com'");
  });

  it('should filter datasets by label', async () => {
    await execSample(labelDataset, datasetId);
    const output = await execSample(listDatasetsByLabel);
    assert.match(output, /Datasets:/);
    assert.match(output, new RegExp(datasetId));
  });

  it('should delete a dataset', async () => {
    const output = await execSample(deleteDataset, datasetId);
    assert.include(output, `Dataset ${datasetId} deleted.`);
    const [exists] = await bigquery.dataset(datasetId).exists();
    assert.strictEqual(exists, false);
  });

  // Only delete a resource if it is older than 24 hours. That will prevent
  // collisions with parallel CI test runs.
  function isResourceStale(creationTime) {
    const oneDayMs = 86400000;
    const now = new Date();
    const created = new Date(creationTime);
    return now.getTime() - created.getTime() >= oneDayMs;
  }

  async function deleteDatasets() {
    let [datasets] = await bigquery.getDatasets();
    datasets = datasets.filter(dataset =>
      dataset.id.includes(DATASETS_SUITE_ID_PREFIX),
    );

    const emulator = Boolean(process.env.BIGQUERY_EMULATOR_HOST);

    for (const dataset of datasets) {
      let metadata;
      try {
        [metadata] = await dataset.getMetadata();
      } catch (e) {
        console.log(`dataset(${dataset.id}).getMetadata() failed`);
        console.log(e);
        return;
      }
      const creationTime = Number(metadata.creationTime);
      if (emulator || isResourceStale(creationTime)) {
        try {
          await dataset.delete({force: true});
        } catch (e) {
          console.log(`dataset(${dataset.id}).delete() failed`);
          console.log(e);
        }
      }
    }
  }
});
