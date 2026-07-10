import { describe, expect, it } from 'vitest';

import {
    datasetMetadataFromBq,
    jobRefFromBq,
    resourceTypeFromBq,
    routineFromBq,
    routineIdsFromList,
    tableDataFromBq,
    tableEntriesFromList,
    tableIdsFromList,
    tableMetadataFromBq,
    type BqDataset,
    type BqJob,
    type BqRoutine,
    type BqRoutineList,
    type BqTable,
    type BqTableData,
    type BqTableList,
} from './bqRest';

describe('resourceTypeFromBq', () => {
    it('maps BQ table types to ResourceType', () => {
        expect(resourceTypeFromBq({ type: 'VIEW' })).toBe('VIEW');
        expect(resourceTypeFromBq({ type: 'MATERIALIZED_VIEW' })).toBe('MATERIALIZED_VIEW');
        expect(resourceTypeFromBq({ type: 'SNAPSHOT' })).toBe('SNAPSHOT');
        expect(resourceTypeFromBq({ type: 'EXTERNAL' })).toBe('EXTERNAL');
    });

    it('defaults unknown types to TABLE', () => {
        expect(resourceTypeFromBq({ type: 'UNKNOWN' })).toBe('TABLE');
        expect(resourceTypeFromBq({})).toBe('TABLE');
    });
});

describe('tableEntriesFromList', () => {
    it('parses tableId and type from tables.list entries', () => {
        const data: BqTableList = {
            tables: [
                { tableReference: { tableId: 't1' }, type: 'TABLE' },
                { tableReference: { tableId: 'v1' }, type: 'VIEW' },
                { tableReference: { tableId: 'mv1' }, type: 'MATERIALIZED_VIEW' },
                { tableReference: { tableId: 's1' }, type: 'SNAPSHOT' },
                { tableReference: { tableId: 'e1' }, type: 'EXTERNAL' },
            ],
        };
        expect(tableEntriesFromList(data)).toEqual([
            { tableId: 't1', resourceType: 'TABLE' },
            { tableId: 'v1', resourceType: 'VIEW' },
            { tableId: 'mv1', resourceType: 'MATERIALIZED_VIEW' },
            { tableId: 's1', resourceType: 'SNAPSHOT' },
            { tableId: 'e1', resourceType: 'EXTERNAL' },
        ]);
    });

    it('defaults missing type to TABLE and skips entries without tableId', () => {
        const data: BqTableList = {
            tables: [{ tableReference: {} }, { tableReference: { tableId: 'plain' } }],
        };
        expect(tableEntriesFromList(data)).toEqual([{ tableId: 'plain', resourceType: 'TABLE' }]);
        expect(tableIdsFromList(data)).toEqual(['plain']);
    });
});

describe('datasetMetadataFromBq', () => {
    it('maps dataset fields from BQ JSON', () => {
        const raw: BqDataset = {
            datasetReference: { datasetId: 'my_dataset', projectId: 'p' },
            friendlyName: 'My Dataset',
            description: 'desc',
            location: 'US',
            creationTime: '1000',
            lastModifiedTime: '2000',
            defaultTableExpirationMs: '3600000',
            defaultCollation: 'und:ci',
            defaultRoundingMode: 'ROUND_HALF_AWAY_FROM_ZERO',
            maxTimeTravelHours: '168',
            isCaseInsensitive: true,
            labels: { env: 'test' },
            replicas: [{ location: 'EU' }],
        };
        const meta = datasetMetadataFromBq('p', 'my_dataset', raw);
        expect(meta).toEqual({
            id: 'my_dataset',
            friendlyName: 'My Dataset',
            description: 'desc',
            location: 'US',
            creationTime: new Date(1000).toISOString(),
            lastModifiedTime: new Date(2000).toISOString(),
            defaultTableExpirationMs: 3600000,
            defaultCollation: 'und:ci',
            defaultRoundingMode: 'ROUND_HALF_AWAY_FROM_ZERO',
            maxTimeTravelHours: 168,
            isCaseInsensitive: true,
            labels: { env: 'test' },
            tags: {},
            replicas: [{ location: 'EU' }],
        });
    });
});

describe('tableMetadataFromBq', () => {
    it('maps extended table fields including view query and storage stats', () => {
        const raw: BqTable = {
            type: 'VIEW',
            schema: {
                fields: [
                    { name: 'id', type: 'INTEGER', mode: 'REQUIRED' },
                    { name: 'name', type: 'STRING', mode: 'NULLABLE' },
                ],
            },
            numRows: '42',
            numBytes: '1000',
            numLongTermBytes: '200',
            numActiveLogicalBytes: '800',
            numPhysicalBytes: '500',
            numActivePhysicalBytes: '400',
            numLongTermPhysicalBytes: '100',
            numCurrentPhysicalBytes: '450',
            numTimeTravelPhysicalBytes: '50',
            creationTime: '1000',
            lastModifiedTime: '2000',
            expirationTime: '3000',
            description: 'A view',
            location: 'US',
            defaultCollation: 'und:ci',
            defaultRoundingMode: 'ROUND_HALF_AWAY_FROM_ZERO',
            caseInsensitive: true,
            labels: { tier: 'gold' },
            view: { query: 'SELECT * FROM t', useLegacySql: true },
            tableConstraints: {
                primaryKey: { columns: [{ fieldIndex: 0 }] },
            },
        };
        const meta = tableMetadataFromBq('p', 'd', 'v', raw);
        expect(meta.resourceType).toBe('VIEW');
        expect(meta.viewQuery).toBe('SELECT * FROM t');
        expect(meta.useLegacySql).toBe(true);
        expect(meta.primaryKeys).toEqual(['id']);
        expect(meta.labels).toEqual({ tier: 'gold' });
        expect(meta.expirationTime).toBe(new Date(3000).toISOString());
        expect(meta.storage).toEqual({
            numRows: 42,
            totalLogicalBytes: 1000,
            activeLogicalBytes: 800,
            longTermLogicalBytes: 200,
            currentPhysicalBytes: 450,
            totalPhysicalBytes: 500,
            activePhysicalBytes: 400,
            longTermPhysicalBytes: 100,
            timeTravelPhysicalBytes: 50,
        });
        expect(meta.fullyQualifiedName).toBe('p.d.v');
    });

    it('reads materialized view query when view is absent', () => {
        const raw: BqTable = {
            type: 'MATERIALIZED_VIEW',
            materializedView: { query: 'SELECT COUNT(*) FROM t' },
        };
        const meta = tableMetadataFromBq('p', 'd', 'mv', raw);
        expect(meta.resourceType).toBe('MATERIALIZED_VIEW');
        expect(meta.viewQuery).toBe('SELECT COUNT(*) FROM t');
    });
});

describe('routineIdsFromList', () => {
    it('extracts routine ids from list response', () => {
        const data: BqRoutineList = {
            routines: [
                { routineReference: { routineId: 'fn_a' } },
                { routineReference: { routineId: 'fn_b' } },
            ],
        };
        expect(routineIdsFromList(data)).toEqual(['fn_a', 'fn_b']);
    });
});

describe('routineFromBq', () => {
    it('maps routine metadata from BQ JSON', () => {
        const raw: BqRoutine = {
            routineReference: { routineId: 'my_fn' },
            routineType: 'SCALAR_FUNCTION',
            language: 'SQL',
            definitionBody: 'x + 1',
            arguments: [
                {
                    name: 'x',
                    dataType: { typeKind: 'INT64' },
                    argumentKind: 'FIXED_TYPE',
                },
            ],
            returnType: { typeKind: 'INT64' },
            creationTime: '1000',
            lastModifiedTime: '2000',
        };
        const meta = routineFromBq('p', 'd', 'my_fn', raw);
        expect(meta).toEqual({
            id: 'my_fn',
            routineType: 'SCALAR_FUNCTION',
            language: 'SQL',
            definitionBody: 'x + 1',
            arguments: [{ name: 'x', dataType: 'INT64', argumentKind: 'FIXED_TYPE' }],
            returnType: 'INT64',
            creationTime: new Date(1000).toISOString(),
            lastModifiedTime: new Date(2000).toISOString(),
        });
    });
});

describe('tableDataFromBq', () => {
    it('parses table data rows using schema and parseBqValue', () => {
        const raw: BqTableData = {
            rows: [
                { f: [{ v: '1' }, { v: 'hello' }] },
                { f: [{ v: '2' }, { v: 'world' }] },
            ],
            pageToken: 'next',
            totalRows: '99',
        };
        const schema = [
            { name: 'id', type: 'INTEGER', mode: 'REQUIRED' },
            { name: 'name', type: 'STRING', mode: 'NULLABLE' },
        ];
        const page = tableDataFromBq(raw, schema);
        expect(page.rows).toEqual([
            { id: 1, name: 'hello' },
            { id: 2, name: 'world' },
        ]);
        expect(page.pageToken).toBe('next');
        expect(page.totalRows).toBe(99);
    });
});

describe('jobRefFromBq', () => {
    it('maps job reference and status from BQ JSON', () => {
        const raw: BqJob = {
            jobReference: { jobId: 'job123', projectId: 'p' },
            status: {
                state: 'DONE',
                errorResult: { reason: 'invalidQuery', message: 'bad sql' },
            },
        };
        expect(jobRefFromBq(raw)).toEqual({
            jobId: 'job123',
            projectId: 'p',
            state: 'DONE',
            errorResult: { reason: 'invalidQuery', message: 'bad sql' },
        });
    });

    it('defaults missing fields', () => {
        expect(jobRefFromBq({})).toEqual({
            jobId: '',
            projectId: '',
            state: '',
            errorResult: null,
        });
    });
});
