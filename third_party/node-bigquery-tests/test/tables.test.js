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
const {describe, it, before, after, beforeEach} = require('mocha');
const path = require('path');
const {randomUUID} = require('crypto');
const {Storage} = require('@google-cloud/storage');
const {BigQuery} = require('@google-cloud/bigquery');
const {getSampleProjectId} = require('../lib/sampleProjectEnv');
const {DataCatalogClient, PolicyTagManagerClient} =
  require('@google-cloud/datacatalog').v1;
const {warnIgnoringNotFound} = require('../lib/warnIgnoringNotFound');
const {execSample} = require('../lib/execSample');

const createTable = require('../createTable.js').main;
const createTablePartitioned = require('../createTablePartitioned.js').main;
const createTableRangePartitioned =
  require('../createTableRangePartitioned.js').main;
const createTableClustered = require('../createTableClustered.js').main;
const removeTableClustering = require('../removeTableClustering.js').main;
const nestedRepeatedSchema = require('../nestedRepeatedSchema.js').main;
const createTableColumnACL = require('../createTableColumnACL.js').main;
const updateTableColumnACL = require('../updateTableColumnACL.js').main;
const getTable = require('../getTable.js').main;
const tableExists = require('../tableExists.js').main;
const listTables = require('../listTables.js').main;
const updateTableDescription = require('../updateTableDescription.js').main;
const updateTableExpiration = require('../updateTableExpiration.js').main;
const labelTable = require('../labelTable.js').main;
const deleteLabelTable = require('../deleteLabelTable.js').main;
const loadLocalFile = require('../loadLocalFile.js').main;
const browseTable = require('../browseTable.js').main;
const extractTableToGCS = require('../extractTableToGCS.js').main;
const extractTableJSON = require('../extractTableJSON.js').main;
const extractTableCompressed = require('../extractTableCompressed.js').main;
const loadTableGCSORC = require('../loadTableGCSORC.js').main;
const loadTableGCSParquet = require('../loadTableGCSParquet.js').main;
const loadTableGCSAvro = require('../loadTableGCSAvro.js').main;
const loadTableURIFirestore = require('../loadTableURIFirestore.js').main;
const loadCSVFromGCS = require('../loadCSVFromGCS.js').main;
const loadJSONFromGCS = require('../loadJSONFromGCS.js').main;
const loadTablePartitioned = require('../loadTablePartitioned.js').main;
const loadTableClustered = require('../loadTableClustered.js').main;
const addColumnLoadAppend = require('../addColumnLoadAppend.js').main;
const relaxColumnLoadAppend = require('../relaxColumnLoadAppend.js').main;
const loadCSVFromGCSAutodetect = require('../loadCSVFromGCSAutodetect.js').main;
const loadJSONFromGCSAutodetect =
  require('../loadJSONFromGCSAutodetect.js').main;
const loadCSVFromGCSTruncate = require('../loadCSVFromGCSTruncate.js').main;
const loadJSONFromGCSTruncate = require('../loadJSONFromGCSTruncate.js').main;
const loadParquetFromGCSTruncate =
  require('../loadParquetFromGCSTruncate.js').main;
const loadOrcFromGCSTruncate = require('../loadOrcFromGCSTruncate.js').main;
const loadTableGCSAvroTruncate = require('../loadTableGCSAvroTruncate.js').main;
const copyTable = require('../copyTable.js').main;
const insertRowsAsStream = require('../insertRowsAsStream.js').main;
const insertingDataTypes = require('../insertingDataTypes.js').main;
const copyTableMultipleSource = require('../copyTableMultipleSource.js').main;
const addEmptyColumn = require('../addEmptyColumn.js').main;
const relaxColumn = require('../relaxColumn.js').main;
const getTableLabels = require('../getTableLabels.js').main;
const createView = require('../createView.js').main;
const getView = require('../getView.js').main;
const updateViewQuery = require('../updateViewQuery.js').main;
const deleteTable = require('../deleteTable.js').main;
const undeleteTable = require('../undeleteTable.js').main;

const dataCatalog = new DataCatalogClient();
const policyTagManager = new PolicyTagManagerClient();

// @google-cloud/storage v7 reads STORAGE_EMULATOR_HOST as the JSON baseUrl. If it
// lacks /storage/v1, insertBucket 404s; if it includes /storage/v1, uploads use
// apiEndpoint + "/upload/storage/v1/..." and double the path. Clear the env for
// ctor and pass apiEndpoint as host root only (see storage.js baseUrl fallback).
const savedStorageEmulatorHost = process.env.STORAGE_EMULATOR_HOST;
if (savedStorageEmulatorHost) {
  delete process.env.STORAGE_EMULATOR_HOST;
}

function storageClientOptions() {
  const opts = {};
  const pid = getSampleProjectId();
  if (pid) {
    opts.projectId = pid;
  }
  const raw = savedStorageEmulatorHost || process.env.STORAGE_EMULATOR_HOST;
  if (raw) {
    const trimmed = String(raw).trim().replace(/\/$/, '');
    let base = /^https?:\/\//i.test(trimmed)
      ? trimmed
      : `http://${trimmed.replace(/^\/\//, '')}`;
    base = base.replace(/\/storage\/v1\/?$/i, '');
    opts.apiEndpoint = base;
  }
  return opts;
}

const storage = new Storage(storageClientOptions());
if (savedStorageEmulatorHost !== undefined) {
  process.env.STORAGE_EMULATOR_HOST = savedStorageEmulatorHost;
}

const GCLOUD_TESTS_PREFIX = 'nodejs_samples_tests';

const generateUuid = () =>
  `${GCLOUD_TESTS_PREFIX}_${randomUUID()}`.replace(/-/gi, '_');

const datasetId = generateUuid();
const srcDatasetId = datasetId;
const destDatasetId = generateUuid();
const tableId = generateUuid();
const nestedTableId = generateUuid();
const partitionedTableId = generateUuid();
const srcTableId = tableId;
const aclTableId = generateUuid();
const destTableId = generateUuid();
const viewId = generateUuid();
const bucketName = generateUuid();
const exportCSVFileName = 'data.json';
const exportJSONFileName = 'data.json';
const importFileName = 'data.avro';
const partialDataFileName = 'partialdata.csv';
const localFilePath = path.join(__dirname, `../resources/${importFileName}`);
const testExpirationTime = Date.now() + 2 * 60 * 60 * 1000; // Add two hours
let projectId;
let policyTag0;
let policyTag1;
const partialDataFilePath = path.join(
  __dirname,
  `../resources/${partialDataFileName}`,
);
const bigquery = new BigQuery();

describe('Tables', () => {
  before(async () => {
    const [bucket] = await storage.createBucket(bucketName);
    await Promise.all([
      bucket.upload(localFilePath, {resumable: false}),
      bigquery.createDataset(srcDatasetId),
      bigquery.createDataset(destDatasetId),
    ]);

    if (process.env.BIGQUERY_EMULATOR_HOST) {
      projectId = getSampleProjectId() || 'dev';
      policyTag0 = {
        name: 'projects/dev/locations/us/taxonomies/emulator-tax/policyTags/emulator-pt-0',
      };
      policyTag1 = {
        name: 'projects/dev/locations/us/taxonomies/emulator-tax/policyTags/emulator-pt-1',
      };
      return;
    }

    // Delete stale Data Catalog resources
    projectId = await dataCatalog.getProjectId();
    await deleteStaleTaxonomies();

    // Create Data Catalog resources
    const parent = dataCatalog.locationPath(projectId, 'us');
    const taxRequest = {
      parent,
      taxonomy: {
        displayName: generateUuid(),
        activatedPolicyTypes: ['FINE_GRAINED_ACCESS_CONTROL'],
      },
    };
    const [taxonomy] = await policyTagManager.createTaxonomy(taxRequest);
    const tagRequest = {
      parent: taxonomy.name,
      policyTag: {
        displayName: generateUuid(),
      },
    };
    [policyTag0] = await policyTagManager.createPolicyTag(tagRequest);
    tagRequest.policyTag.displayName = generateUuid();
    [policyTag1] = await policyTagManager.createPolicyTag(tagRequest);
  });

  // to avoid getting rate limited
  beforeEach(async function () {
    this.currentTest.retries(2);
  });

  after(async () => {
    await bigquery
      .dataset(srcDatasetId)
      .delete({force: true})
      .catch(warnIgnoringNotFound);
    await bigquery
      .dataset(destDatasetId)
      .delete({force: true})
      .catch(warnIgnoringNotFound);
    await bigquery
      .dataset(datasetId)
      .delete({force: true})
      .catch(warnIgnoringNotFound);
    await storage
      .bucket(bucketName)
      .deleteFiles({force: true})
      .catch(warnIgnoringNotFound);
    await storage
      .bucket(bucketName)
      .deleteFiles({force: true})
      .catch(warnIgnoringNotFound);
    await bigquery
      .dataset(srcDatasetId)
      .delete({force: true})
      .catch(warnIgnoringNotFound);
    await storage.bucket(bucketName).delete().catch(warnIgnoringNotFound);
  });

  it('should create a table', async () => {
    const output = await execSample(createTable, datasetId, tableId);
    assert.include(output, `Table ${tableId} created.`);
    const [exists] = await bigquery.dataset(datasetId).table(tableId).exists();
    assert.ok(exists);
  });

  it('should create a partitioned table', async () => {
    const output = await execSample(
      createTablePartitioned,
      datasetId,
      partitionedTableId,
    );
    assert.include(
      output,
      `Table ${partitionedTableId} created with partitioning:`,
    );
    assert.include(output, "type: 'DAY'");
    assert.include(output, "field: 'date'");
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(partitionedTableId)
      .exists();
    assert.ok(exists);
  });

  it('should create an integer range partitioned table', async () => {
    const rangePartTableId = generateUuid();
    const output = await execSample(
      createTableRangePartitioned,
      datasetId,
      rangePartTableId,
    );
    assert.include(
      output,
      `Table ${rangePartTableId} created with integer range partitioning:`,
    );
    assert.include(
      output,
      "range: { start: '0', end: '100000', interval: '10' }",
    );
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(rangePartTableId)
      .exists();
    assert.ok(exists);
  });

  it('should create a clustered table', async () => {
    const clusteredTableId = generateUuid();
    const output = await execSample(
      createTableClustered,
      datasetId,
      clusteredTableId,
    );
    assert.include(
      output,
      `Table ${clusteredTableId} created with clustering:`,
    );
    assert.include(output, "{ fields: [ 'city', 'zipcode' ] }");
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(clusteredTableId)
      .exists();
    assert.ok(exists);
  });

  it('should update table clustering', async () => {
    const clusteredTableId = generateUuid();
    const output = await execSample(
      removeTableClustering,
      datasetId,
      clusteredTableId,
    );
    assert.include(
      output,
      `Table ${clusteredTableId} created with clustering.`,
    );
    assert.include(output, `Table ${clusteredTableId} updated clustering:`);
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(clusteredTableId)
      .exists();
    assert.ok(exists);
  });

  it('should create a table with nested schema', async () => {
    const output = await execSample(
      nestedRepeatedSchema,
      datasetId,
      nestedTableId,
    );
    assert.include(output, `Table ${nestedTableId} created.`);
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(nestedTableId)
      .exists();
    assert.ok(exists);
  });

  it('should create a table with column-level security', async () => {
    const output = await execSample(
      createTableColumnACL,
      datasetId,
      aclTableId,
      policyTag0.name,
    );
    assert.include(output, `Created table ${aclTableId} with schema:`);
    assert.include(output, policyTag0.name);
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(aclTableId)
      .exists();
    assert.ok(exists);
  });

  it('should update a table with column-level security', async () => {
    const output = await execSample(
      updateTableColumnACL,
      datasetId,
      aclTableId,
      policyTag1.name,
    );
    assert.include(output, `Updated table ${aclTableId} with schema:`);
    assert.include(output, policyTag1.name);
    const [exists] = await bigquery
      .dataset(datasetId)
      .table(aclTableId)
      .exists();
    assert.ok(exists);
  });

  it('should retrieve a table if it exists', async () => {
    const output = await execSample(getTable, datasetId, tableId);
    assert.include(output, 'Table:');
    assert.include(output, datasetId);
    assert.include(output, tableId);
  });

  it('should check whether a table exists', async () => {
    const nonexistentTableId = 'foobar';
    const output = await execSample(
      tableExists,
      datasetId,
      nonexistentTableId,
    );
    assert.include(output, 'Not found');
    assert.include(output, datasetId);
    assert.include(output, nonexistentTableId);
  });

  it('should create/update a table with default collation', async () => {
    const collationTableId = generateUuid();
    const [table] = await bigquery
      .dataset(datasetId)
      .createTable(collationTableId, {
        schema: [
          {name: 'name', type: 'STRING'},
          {name: 'nums', type: 'INTEGER'},
        ],
        defaultCollation: 'und:ci',
        expirationTime: testExpirationTime,
      });
    let [md] = await table.getMetadata();
    assert.equal(md.defaultCollation, 'und:ci');
    for (const field of md.schema.fields) {
      if (field.type === 'STRING') {
        assert.equal(field.collation, 'und:ci');
      }
    }
    // update table collation to case sensitive
    md.defaultCollation = '';
    await table.setMetadata(md);
    [md] = await table.getMetadata();
    assert.equal(md.defaultCollation, '');

    // add field with different collation
    md.schema.fields.push({
      name: 'another_name',
      type: 'STRING',
      collation: 'und:ci',
    });
    await table.setMetadata(md);

    [md] = await table.getMetadata();
    for (const field of md.schema.fields) {
      if (field.type === 'STRING') {
        assert.equal(field.collation, 'und:ci');
      }
    }
  });

  it('should list tables', async () => {
    const output = await execSample(listTables, datasetId);
    assert.match(output, /Tables:/);
    assert.match(output, new RegExp(tableId));
  });

  it("should update table's description", async () => {
    const output = await execSample(
      updateTableDescription,
      datasetId,
      tableId,
    );
    assert.include(output, `${tableId} description: New table description.`);
  });

  it("should update table's expiration", async () => {
    const currentTime = Date.now();
    const expirationTime = currentTime + 1000 * 60 * 60 * 24 * 5;
    const output = await execSample(
      updateTableExpiration,
      datasetId,
      tableId,
      expirationTime,
    );
    assert.include(output, `${tableId}`);
    assert.include(output, `expiration: ${expirationTime}`);
  });

  it('should add label to a table', async () => {
    const output = await execSample(labelTable, datasetId, tableId);
    assert.include(output, `${tableId} labels:`);
    assert.include(output, "{ color: 'green' }");
  });

  it('should delete a label from a table', async () => {
    const output = await execSample(deleteLabelTable, datasetId, tableId);
    assert.include(output, `${tableId} labels:`);
    assert.include(output, 'undefined');
  });

  it('should load a local CSV file', async () => {
    const output = await execSample(
      loadLocalFile,
      datasetId,
      tableId,
      localFilePath,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.strictEqual(rows.length, 1);
  });

  it('should browse table rows', async () => {
    const browseDestTable = generateUuid();
    const output = await execSample(
      browseTable,
      datasetId,
      browseDestTable,
    );
    assert.match(output, /name/);
    assert.match(output, /total people/);
  });

  it('should extract a table to GCS CSV file', async () => {
    const output = await execSample(
      extractTableToGCS,
      datasetId,
      tableId,
      bucketName,
      exportCSVFileName,
    );

    assert.match(output, /created\./);
    const [exists] = await storage
      .bucket(bucketName)
      .file(exportCSVFileName)
      .exists();
    assert.ok(exists);
  });

  it('should extract a table to GCS JSON file', async () => {
    const output = await execSample(
      extractTableJSON,
      datasetId,
      tableId,
      bucketName,
      exportJSONFileName,
    );

    assert.match(output, /created\./);
    const [exists] = await storage
      .bucket(bucketName)
      .file(exportJSONFileName)
      .exists();
    assert.ok(exists);
  });

  it('should extract a table to GCS compressed file', async () => {
    const output = await execSample(
      extractTableCompressed,
      datasetId,
      tableId,
      bucketName,
      exportCSVFileName,
    );

    assert.match(output, /created\./);
    const [exists] = await storage
      .bucket(bucketName)
      .file(exportCSVFileName)
      .exists();
    assert.ok(exists);
  });

  it('should load a GCS ORC file', async () => {
    const tableId = generateUuid();
    const output = await execSample(loadTableGCSORC, datasetId, tableId);
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS Parquet file', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadTableGCSParquet,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS Avro file', async () => {
    const tableId = generateUuid();
    const output = await execSample(loadTableGCSAvro, datasetId, tableId);
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS Firestore backup file', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadTableURIFirestore,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS CSV file with explicit schema', async () => {
    const tableId = generateUuid();
    const output = await execSample(loadCSVFromGCS, datasetId, tableId);
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS JSON file with explicit schema', async () => {
    const tableId = generateUuid();
    const output = await execSample(loadJSONFromGCS, datasetId, tableId);
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS CSV file to partitioned table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadTablePartitioned,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS CSV file to clustered table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadTableClustered,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should add a new column via a GCS file load job', async () => {
    const destTableId = generateUuid();
    await execSample(
      createTable,
      datasetId,
      destTableId,
      'Name:STRING, Age:INTEGER, Weight:FLOAT',
    );
    const output = await execSample(
      addColumnLoadAppend,
      datasetId,
      destTableId,
      localFilePath,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery
      .dataset(datasetId)
      .table(destTableId)
      .getRows();
    assert.ok(rows.length > 0);
  });

  it('should relax a column via a GCS file load job', async () => {
    const destTableId = generateUuid();
    await execSample(createTable, datasetId, destTableId);
    const output = await execSample(
      relaxColumnLoadAppend,
      datasetId,
      destTableId,
      partialDataFilePath,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery
      .dataset(datasetId)
      .table(destTableId)
      .getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS CSV file with autodetected schema', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadCSVFromGCSAutodetect,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS JSON file with autodetected schema', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadJSONFromGCSAutodetect,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS CSV file truncate table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadCSVFromGCSTruncate,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    assert.include(output, 'Write disposition used: WRITE_TRUNCATE.');
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS JSON file truncate table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadJSONFromGCSTruncate,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    assert.include(output, 'Write disposition used: WRITE_TRUNCATE.');
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS parquet file truncate table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadParquetFromGCSTruncate,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    assert.include(output, 'Write disposition used: WRITE_TRUNCATE.');
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS ORC file truncate table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadOrcFromGCSTruncate,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    assert.include(output, 'Write disposition used: WRITE_TRUNCATE.');
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should load a GCS Avro file truncate table', async () => {
    const tableId = generateUuid();
    const output = await execSample(
      loadTableGCSAvroTruncate,
      datasetId,
      tableId,
    );
    assert.match(output, /completed\./);
    assert.include(output, 'Write disposition used: WRITE_TRUNCATE.');
    const [rows] = await bigquery.dataset(datasetId).table(tableId).getRows();
    assert.ok(rows.length > 0);
  });

  it('should copy a table', async () => {
    const output = await execSample(
      copyTable,
      srcDatasetId,
      srcTableId,
      destDatasetId,
      destTableId,
    );
    assert.match(output, /completed\./);
    const [rows] = await bigquery
      .dataset(destDatasetId)
      .table(destTableId)
      .getRows();
    assert.ok(rows.length > 0);
  });

  it('should insert rows', async () => {
    const output = await execSample(
      insertRowsAsStream,
      datasetId,
      tableId,
    );
    assert.match(output, /Inserted 2 rows/);
  });

  it('should insert rows with supported data types', async () => {
    const typesTableId = generateUuid();
    const output = await execSample(
      insertingDataTypes,
      datasetId,
      typesTableId,
    );
    assert.match(output, /Inserted 2 rows/);
  });

  it('copy multiple source tables to a given destination', async () => {
    await execSample(createTable, datasetId, 'destinationTable');
    const output = await execSample(
      copyTableMultipleSource,
      datasetId,
      tableId,
      'destinationTable',
    );
    assert.include(output, 'sourceTable');
    assert.include(output, 'destinationTable');
    assert.include(output, 'createDisposition');
    assert.include(output, 'writeDisposition');
  });

  it('should add a column to the schema', async () => {
    const column = "name: 'size', type: 'STRING'";
    const output = await execSample(addEmptyColumn, datasetId, tableId);
    assert.include(output, column);
  });

  it("should update a column from 'REQUIRED' TO 'NULLABLE'", async () => {
    const column = "name: 'Name', type: 'STRING', mode: 'NULLABLE'";
    await execSample(createTable, datasetId, 'newTable');
    const output = await execSample(relaxColumn, datasetId, 'newTable');
    assert.include(output, column);
  });

  it('should get labels on a table', async () => {
    await execSample(labelTable, datasetId, tableId);
    const output = await execSample(getTableLabels, datasetId, tableId);
    assert.include(output, `${tableId} Labels:`);
    assert.include(output, 'color: green');
  });

  describe('Views', () => {
    beforeEach(async function () {
      this.currentTest.retries(2);
    });

    it('should create a view', async () => {
      const output = await execSample(createView, datasetId, viewId);
      assert.include(output, `View ${viewId} created.`);
      const [exists] = await bigquery.dataset(datasetId).table(viewId).exists();
      assert.ok(exists);
    });

    it('should get a view', async () => {
      const viewId = generateUuid();
      await execSample(createView, datasetId, viewId);
      const output = await execSample(getView, datasetId, viewId);
      assert.match(output, /View at/);
      assert.match(output, /View query:/);
    });

    it('should update a view', async () => {
      const output = await execSample(updateViewQuery, datasetId, viewId);
      assert.include(output, `View ${viewId} updated.`);
    });
  });

  describe('Delete Table', () => {
    const datasetId = `gcloud_tests_${randomUUID()}`.replace(/-/gi, '_');
    const tableId = `gcloud_tests_${randomUUID()}`.replace(/-/gi, '_');

    before(async () => {
      const datasetOptions = {
        location: 'US',
      };
      const tableOptions = {
        location: 'US',
      };

      await bigquery.createDataset(datasetId, datasetOptions);
      // Create a new table in the dataset
      await bigquery.dataset(datasetId).createTable(tableId, tableOptions);
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

    it('should delete a table', async () => {
      const output = await execSample(deleteTable, datasetId, tableId);
      assert.include(output, `Table ${tableId} deleted.`);
      const [exists] = await bigquery
        .dataset(datasetId)
        .table(tableId)
        .exists();
      assert.strictEqual(exists, false);
    });

    it('should undelete a table', async () => {
      const tableId = generateUuid();
      const recoveredTableId = generateUuid();

      await execSample(createTable, datasetId, tableId);
      const output = await execSample(
        undeleteTable,
        datasetId,
        tableId,
        recoveredTableId,
      );

      assert.include(output, `Table ${tableId} deleted.`);
      assert.match(output, /Copied data from deleted table/);
      const [exists] = await bigquery
        .dataset(datasetId)
        .table(recoveredTableId)
        .exists();
      assert.strictEqual(exists, true);
    });
  });

  // Only delete a resource if it is older than 24 hours. That will prevent
  // collisions with parallel CI test runs.
  function isResourceStale(creationTime) {
    const oneDayMs = 86400000;
    const now = new Date();
    const created = new Date(creationTime * 1000);
    return now.getTime() - created.getTime() >= oneDayMs;
  }

  async function deleteStaleTaxonomies() {
    const location = 'us';
    const listTaxonomiesRequest = {
      parent: dataCatalog.locationPath(projectId, location),
    };
    let [taxonomies] = await policyTagManager.listTaxonomies(
      listTaxonomiesRequest,
    );

    taxonomies = taxonomies.filter(taxonomy => {
      return taxonomy.displayName.includes(GCLOUD_TESTS_PREFIX);
    });
    taxonomies.forEach(async taxonomy => {
      if (isResourceStale(taxonomy.taxonomyTimestamps.createTime.seconds)) {
        try {
          await policyTagManager.deleteTaxonomy({name: taxonomy.name});
        } catch (e) {
          console.error(e);
        }
      }
    });
  }
});
