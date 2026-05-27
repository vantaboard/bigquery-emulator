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

const {assert} = require('chai');
const {describe, it, before, beforeEach, after} = require('mocha');
const {randomUUID} = require('crypto');

const {BigQuery} = require('@google-cloud/bigquery');
const {warnIgnoringNotFound} = require('../lib/warnIgnoringNotFound');
const {execSample} = require('../lib/execSample');

const queryStackOverflow = require('../queryStackOverflow.js').main;
const query = require('../query.js').main;
const queryJobOptional = require('../queryJobOptional.js').main;
const queryDryRun = require('../queryDryRun.js').main;
const queryDisableCache = require('../queryDisableCache.js').main;
const queryParamsNamed = require('../queryParamsNamed.js').main;
const queryParamsNamedTypes = require('../queryParamsNamedTypes.js').main;
const queryParamsPositional = require('../queryParamsPositional.js').main;
const queryParamsPositionalTypes =
  require('../queryParamsPositionalTypes.js').main;
const queryParamsStructs = require('../queryParamsStructs.js').main;
const queryParamsArrays = require('../queryParamsArrays.js').main;
const queryParamsTimestamps = require('../queryParamsTimestamps.js').main;
const queryDestinationTable = require('../queryDestinationTable.js').main;
const queryLegacy = require('../queryLegacy.js').main;
const queryClusteredTable = require('../queryClusteredTable.js').main;
const queryLegacyLargeResults = require('../queryLegacyLargeResults.js').main;
const createTable = require('../createTable.js').main;
const addColumnQueryAppend = require('../addColumnQueryAppend.js').main;
const relaxColumnQueryAppend = require('../relaxColumnQueryAppend.js').main;
const queryBatch = require('../queryBatch.js').main;
const ddlCreateView = require('../ddlCreateView.js').main;
const queryExternalGCSPerm = require('../queryExternalGCSPerm.js').main;
const queryExternalGCSTemp = require('../queryExternalGCSTemp.js').main;
const createRoutineDDL = require('../createRoutineDDL.js').main;

const GCLOUD_TESTS_PREFIX = 'nodejs_samples_tests_queries';

const generateUuid = () =>
  `${GCLOUD_TESTS_PREFIX}_${randomUUID()}`.replace(/-/gi, '_');

const datasetId = generateUuid();
const tableId = generateUuid();
const destTableId = generateUuid();
const routineId = generateUuid();
let projectId;

const bigquery = new BigQuery();

describe('Queries', () => {
  before(async () => {
    const schema = [{name: 'age', type: 'STRING', mode: 'REQUIRED'}];
    const options = {
      schema: schema,
    };
    await bigquery.createDataset(datasetId);
    await bigquery.dataset(datasetId).createTable(destTableId);
    const [tableData] = await bigquery
      .dataset(datasetId)
      .createTable(tableId, options);
    projectId = tableData.metadata.tableReference.projectId;
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

  it('should query stackoverflow', async () => {
    const output = await execSample(queryStackOverflow);
    assert.match(output, /Query Results:/);
    assert.match(output, /views/);
  });

  it('should run a query', async () => {
    const output = await execSample(query);
    assert.match(output, /Rows:/);
    assert.match(output, /name/);
  });

  it('should run a query stateless mode', async () => {
    const output = await execSample(queryJobOptional);
    assert.match(output, /Rows:/);
    assert.match(output, /name/);
  });

  it('should run a query as a dry run', async () => {
    const output = await execSample(queryDryRun);
    assert.match(output, /Status:/);
    assert.include(output, '\nJob Statistics:');
    assert.include(output, 'DONE');
    assert.include(output, 'totalBytesProcessed:');
  });

  it('should run a query with the cache disabled', async () => {
    const output = await execSample(queryDisableCache);
    assert.match(output, /Rows:/);
    assert.match(output, /corpus/);
  });

  it('should run a query with named params', async () => {
    const output = await execSample(queryParamsNamed);
    assert.match(output, /Rows:/);
    assert.match(output, /word_count/);
  });

  it('should run a query with named params and provided types', async () => {
    const output = await execSample(queryParamsNamedTypes);
    assert.match(output, /Rows:/);
    assert.match(output, /word/);
  });

  it('should run a query with positional params', async () => {
    const output = await execSample(queryParamsPositional);
    assert.match(output, /Rows:/);
    assert.match(output, /word_count/);
  });

  it('should run a query with positional params and provided types', async () => {
    const output = await execSample(queryParamsPositionalTypes);
    assert.match(output, /Rows:/);
    assert.match(output, /word/);
  });

  it('should run a query with struct params', async () => {
    const output = await execSample(queryParamsStructs);
    assert.match(output, /Rows:/);
    assert.match(output, /foo/);
  });

  it('should run a query with array params', async () => {
    const output = await execSample(queryParamsArrays);
    assert.match(output, /Rows:/);
    assert.match(output, /count/);
  });

  it('should run a query with timestamp params', async () => {
    const output = await execSample(queryParamsTimestamps);
    assert.match(output, /Rows:/);
    assert.match(output, /BigQueryTimestamp/);
  });

  it('should run a query with a destination table', async () => {
    // Do not reuse `tableId`: this job replaces the destination schema with the
    // query output (name only), which would break later tests that still expect `age`.
    const queryDestTableId = generateUuid();
    const output = await execSample(
      queryDestinationTable,
      datasetId,
      queryDestTableId,
    );
    assert.include(output, `Query results loaded to table ${queryDestTableId}`);
  });

  it('should run a query with legacy SQL', async () => {
    const output = await execSample(queryLegacy);
    assert.match(output, /Rows:/);
    assert.match(output, /word/);
  });

  it('should run a query with a clustered destination table', async () => {
    const clusteredTableId = generateUuid();
    const output = await execSample(
      queryClusteredTable,
      datasetId,
      clusteredTableId,
    );
    assert.match(output, /started/);
    assert.match(output, /Status/);
  });

  it('should run a query with legacy SQL and large results', async () => {
    const destTableId = generateUuid();
    const output = await execSample(
      queryLegacyLargeResults,
      datasetId,
      destTableId,
      projectId,
    );
    assert.match(output, /Rows:/);
    assert.match(output, /word/);
  });

  it('should add a new column via a query job', async () => {
    const destTableId = generateUuid();
    await execSample(createTable, datasetId, destTableId, 'name:STRING');
    const output = await execSample(
      addColumnQueryAppend,
      datasetId,
      destTableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery
      .dataset(datasetId)
      .table(destTableId)
      .getRows();
    assert.ok(rows.length > 0);
  });

  it('should relax columns via a query job', async () => {
    const output = await execSample(
      relaxColumnQueryAppend,
      projectId,
      datasetId,
      tableId,
    );

    assert.match(output, /1 fields in the schema are required\./);
    assert.match(output, /0 fields in the schema are now required\./);
  });

  it('should run a query at batch priority', async () => {
    const output = await execSample(queryBatch);
    assert.match(output, /Job/);
    assert.match(output, /is currently in state/);
  });

  it('should create a view via DDL query', async () => {
    const output = await execSample(ddlCreateView, projectId, datasetId);
    assert.match(output, /Created new view/);
  });

  it('should query an external data source with permanent table', async () => {
    const permTableId = generateUuid();
    const output = await execSample(
      queryExternalGCSPerm,
      datasetId,
      permTableId,
    );
    assert.match(output, /Rows:/);
    assert.match(output, /post_abbr/);
  });

  it('should query an external data source with temporary table', async () => {
    const output = await execSample(queryExternalGCSTemp);
    assert.match(output, /Rows:/);
    assert.match(output, /post_abbr/);
  });

  it('should create a routine using DDL', async () => {
    const output = await execSample(
      createRoutineDDL,
      projectId,
      datasetId,
      routineId,
    );
    assert.include(output, `Routine ${routineId} created.`);
  });
});
