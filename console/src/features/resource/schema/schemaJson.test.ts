import { describe, expect, it } from 'vitest';

import {
    createEmptySchemaField,
    formatSchemaFieldsJson,
    parseSchemaFieldsJson,
    schemaFieldsToBqPayload,
} from './schemaJson';
import { validateSchemaFields } from './validation';

describe('schemaJson', () => {
    it('round-trips nested RECORD fields with max length and RANGE', () => {
        const fields = [
            {
                ...createEmptySchemaField(),
                name: 'user',
                type: 'RECORD',
                fields: [
                    { ...createEmptySchemaField(), name: 'id', type: 'INT64', mode: 'REQUIRED' },
                    {
                        ...createEmptySchemaField(),
                        name: 'email',
                        type: 'STRING',
                        maxLength: 255,
                    },
                ],
            },
            {
                ...createEmptySchemaField(),
                name: 'active_period',
                type: 'RANGE',
                rangeElementType: 'DATE',
            },
        ];

        const json = formatSchemaFieldsJson(fields);
        const parsed = parseSchemaFieldsJson(json);
        expect(parsed).toHaveLength(2);
        expect(parsed[0].fields).toHaveLength(2);
        expect(parsed[0].fields?.[1].maxLength).toBe(255);
        expect(parsed[1].rangeElementType).toBe('DATE');
    });

    it('maps fields to BQ payload omitting default mode', () => {
        const payload = schemaFieldsToBqPayload([
            { ...createEmptySchemaField(), name: 'id', type: 'INT64', mode: 'REQUIRED' },
            { ...createEmptySchemaField(), name: 'note', type: 'STRING', mode: 'NULLABLE' },
        ]);
        expect(payload[0].mode).toBe('REQUIRED');
        expect(payload[1].mode).toBeUndefined();
    });
});

describe('validateSchemaFields', () => {
    it('requires field names and nested RECORD children', () => {
        const errors = validateSchemaFields([
            createEmptySchemaField(),
            { ...createEmptySchemaField(), name: 'nested', type: 'RECORD', fields: [] },
        ]);
        expect(errors.some((e) => e.message.includes('Field name is required'))).toBe(true);
        expect(errors.some((e) => e.message.includes('nested field'))).toBe(true);
    });
});
