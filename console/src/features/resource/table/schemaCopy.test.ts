import { describe, expect, it } from 'vitest';

import { formatSchemaAsJson, formatSchemaAsTable, schemaGridRows } from './schemaCopy';

const fields = [
    { name: 'id', type: 'INT64', mode: 'REQUIRED', description: 'Primary id' },
    { name: 'name', type: 'STRING', mode: 'NULLABLE', description: null },
];

describe('schemaCopy', () => {
    it('marks primary keys in grid rows', () => {
        const rows = schemaGridRows(fields, ['id']);
        expect(rows[0]?.key).toBe('PK');
        expect(rows[1]?.key).toBe('');
    });

    it('formats schema as BigQuery JSON', () => {
        const json = formatSchemaAsJson(fields);
        const parsed = JSON.parse(json) as { name: string; description?: string }[];
        expect(parsed).toHaveLength(2);
        expect(parsed[0]).toMatchObject({ name: 'id', description: 'Primary id' });
        expect(parsed[1]).not.toHaveProperty('description');
    });

    it('formats schema as TSV table', () => {
        const rows = schemaGridRows(fields, ['id']);
        const tsv = formatSchemaAsTable(rows);
        expect(tsv).toContain('Field name\tType\tMode');
        expect(tsv).toContain('id\tINT64\tREQUIRED\tPrimary id\tPK');
        expect(tsv).toContain('name\tSTRING\tNULLABLE');
    });
});
