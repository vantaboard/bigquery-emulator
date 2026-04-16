/**
 * Test for empty array handling in REPEATED fields.
 * 
 * When inserting an empty array into a REPEATED field and then querying it back,
 * the Node.js BigQuery client crashes because the emulator returns null instead
 * of an empty array structure.
 */

import { describe, it, expect, beforeAll, afterAll, afterEach } from 'vitest';
import {
  BigQueryEmulatorContainer,
  BQ_EMULATOR_PROJECT_ID,
} from './utils/BigQueryEmulatorContainer.js';
import {
  createBigQueryClient,
  createTestHelper,
  BigQueryTestHelper,
} from './utils/testSetup.js';
import { BigQuery } from '@google-cloud/bigquery';

describe('Empty Array in REPEATED Fields', () => {
  let emulator: BigQueryEmulatorContainer;
  let client: BigQuery;
  let helper: BigQueryTestHelper;

  beforeAll(async () => {
    emulator = await BigQueryEmulatorContainer.start();
    client = createBigQueryClient(emulator);
    helper = createTestHelper(client);
  });

  afterAll(async () => {
    await emulator.stop();
  });

  afterEach(async () => {
    await helper.deleteAllDatasets();
  });

  it('should handle empty array in REPEATED STRING field', async () => {
    // Create table with REPEATED STRING field
    await helper.createTable(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      {
        fields: [
          { name: 'id', type: 'INTEGER', mode: 'REQUIRED' },
          { name: 'tags', type: 'STRING', mode: 'REPEATED' },
        ],
      }
    );

    // Insert row with empty array
    await helper.insertRows(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      [{ id: 1, tags: [] }]
    );

    // Query the data back - this should not crash
    const query = `SELECT id, tags FROM \`${BQ_EMULATOR_PROJECT_ID}.test_dataset.test_table\``;
    const rows = await helper.query(query);

    expect(rows).toHaveLength(1);
    expect(rows[0].id).toBe(1);
    expect(rows[0].tags).toEqual([]);
  });

  it('should handle empty array in REPEATED INTEGER field', async () => {
    await helper.createTable(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      {
        fields: [
          { name: 'id', type: 'STRING', mode: 'REQUIRED' },
          { name: 'values', type: 'INTEGER', mode: 'REPEATED' },
        ],
      }
    );

    await helper.insertRows(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      [{ id: 'test1', values: [] }]
    );

    const query = `SELECT id, values FROM \`${BQ_EMULATOR_PROJECT_ID}.test_dataset.test_table\``;
    const rows = await helper.query(query);

    expect(rows).toHaveLength(1);
    expect(rows[0].id).toBe('test1');
    expect(rows[0].values).toEqual([]);
  });

  it('should handle mix of empty and non-empty arrays', async () => {
    await helper.createTable(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      {
        fields: [
          { name: 'id', type: 'INTEGER', mode: 'REQUIRED' },
          { name: 'items', type: 'STRING', mode: 'REPEATED' },
        ],
      }
    );

    await helper.insertRows(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      [
        { id: 1, items: [] },
        { id: 2, items: ['apple', 'banana'] },
        { id: 3, items: [] },
        { id: 4, items: ['orange'] },
      ]
    );

    const query = `SELECT id, items FROM \`${BQ_EMULATOR_PROJECT_ID}.test_dataset.test_table\` ORDER BY id`;
    const rows = await helper.query(query);

    expect(rows).toHaveLength(4);
    expect(rows[0]).toEqual({ id: 1, items: [] });
    expect(rows[1]).toEqual({ id: 2, items: ['apple', 'banana'] });
    expect(rows[2]).toEqual({ id: 3, items: [] });
    expect(rows[3]).toEqual({ id: 4, items: ['orange'] });
  });

  it('should handle empty arrays in multiple REPEATED fields', async () => {
    await helper.createTable(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      {
        fields: [
          { name: 'id', type: 'INTEGER', mode: 'REQUIRED' },
          { name: 'tags', type: 'STRING', mode: 'REPEATED' },
          { name: 'scores', type: 'FLOAT', mode: 'REPEATED' },
        ],
      }
    );

    await helper.insertRows(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      [{ id: 1, tags: [], scores: [] }]
    );

    const query = `SELECT id, tags, scores FROM \`${BQ_EMULATOR_PROJECT_ID}.test_dataset.test_table\``;
    const rows = await helper.query(query);

    expect(rows).toHaveLength(1);
    expect(rows[0]).toEqual({ id: 1, tags: [], scores: [] });
  });

  it('should handle REPEATED STRUCT with empty array', async () => {
    await helper.createTable(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      {
        fields: [
          { name: 'id', type: 'INTEGER', mode: 'REQUIRED' },
          {
            name: 'records',
            type: 'RECORD',
            mode: 'REPEATED',
            fields: [
              { name: 'name', type: 'STRING', mode: 'NULLABLE' },
              { name: 'value', type: 'INTEGER', mode: 'NULLABLE' },
            ],
          },
        ],
      }
    );

    await helper.insertRows(
      { datasetId: 'test_dataset', tableId: 'test_table' },
      [{ id: 1, records: [] }]
    );

    const query = `SELECT id, records FROM \`${BQ_EMULATOR_PROJECT_ID}.test_dataset.test_table\``;
    const rows = await helper.query(query);

    expect(rows).toHaveLength(1);
    expect(rows[0].id).toBe(1);
    expect(rows[0].records).toEqual([]);
  });
});