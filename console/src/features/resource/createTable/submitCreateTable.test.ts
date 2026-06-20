import { describe, expect, it } from 'vitest';

import { createEmptySchemaField } from '@/features/resource/schema/schemaJson';

import { defaultCreateTableForm } from './defaults';
import { buildInsertTableBody } from './submitCreateTable';

describe('buildInsertTableBody', () => {
    it('builds table insert payload with schema, partitioning, and clustering', () => {
        const form = defaultCreateTableForm('proj', 'ds');
        form.tableName = 'events';
        form.schemaFields = [
            { ...createEmptySchemaField(), name: 'id', type: 'INT64', mode: 'REQUIRED' },
            { ...createEmptySchemaField(), name: 'created_at', type: 'TIMESTAMP' },
        ];
        form.partitionType = 'field';
        form.partitionField = 'created_at';
        form.partitionExpirationDays = '30';
        form.clusteringFields = 'id, created_at';
        form.labels = [{ id: 'l1', key: 'env', value: 'test' }];

        const body = buildInsertTableBody(form);
        expect(body.tableReference).toEqual({ projectId: 'proj', datasetId: 'ds', tableId: 'events' });
        expect(body.schema?.fields).toHaveLength(2);
        expect(body.timePartitioning).toMatchObject({ type: 'DAY', field: 'created_at' });
        expect(body.clustering).toEqual({ fields: ['id', 'created_at'] });
        expect(body.labels).toEqual({ env: 'test' });
    });
});
