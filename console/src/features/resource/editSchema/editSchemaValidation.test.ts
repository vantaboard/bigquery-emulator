import { describe, expect, it } from 'vitest';

import { validateEditSchemaFields, type EditSchemaFieldDraft } from './editSchemaValidation';

function field(partial: Partial<EditSchemaFieldDraft> & Pick<EditSchemaFieldDraft, 'name'>): EditSchemaFieldDraft {
    return {
        id: '1',
        type: 'STRING',
        mode: 'NULLABLE',
        originalMode: 'NULLABLE',
        isExisting: false,
        ...partial,
    };
}

describe('validateEditSchemaFields', () => {
    it('allows REQUIRED to NULLABLE for existing fields', () => {
        const errors = validateEditSchemaFields([
            field({ name: 'id', isExisting: true, originalMode: 'REQUIRED', mode: 'NULLABLE', type: 'INT64' }),
        ]);
        expect(errors).toHaveLength(0);
    });

    it('rejects NULLABLE to REQUIRED for existing fields', () => {
        const errors = validateEditSchemaFields([
            field({ name: 'name', isExisting: true, originalMode: 'NULLABLE', mode: 'REQUIRED' }),
        ]);
        expect(errors.some((e) => e.message.includes('Mode cannot change'))).toBe(true);
    });

    it('rejects duplicate field names', () => {
        const errors = validateEditSchemaFields([
            field({ name: 'dup' }),
            field({ name: 'dup', id: '2' }),
        ]);
        expect(errors.some((e) => e.message.includes('Duplicate'))).toBe(true);
    });
});
