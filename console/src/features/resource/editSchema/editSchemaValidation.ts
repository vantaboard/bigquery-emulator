import type { SchemaFieldDraft, SchemaValidationError } from '../schema/types';

export interface EditSchemaFieldDraft extends SchemaFieldDraft {
    isExisting: boolean;
    originalMode: string;
    defaultValue?: string;
}

const ALLOWED_MODE_TRANSITIONS: Record<string, string[]> = {
    REQUIRED: ['NULLABLE'],
    NULLABLE: ['NULLABLE'],
    REPEATED: ['REPEATED'],
};

export function validateEditSchemaFields(fields: EditSchemaFieldDraft[]): SchemaValidationError[] {
    const errors: SchemaValidationError[] = [];
    const names = new Set<string>();

    fields.forEach((field, index) => {
        const path = String(index);
        const name = field.name.trim();
        if (!name) {
            errors.push({ path, message: 'Field name is required' });
            return;
        }
        if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(name)) {
            errors.push({ path, message: 'Field name must start with a letter or underscore' });
        }
        if (names.has(name)) {
            errors.push({ path, message: `Duplicate field name "${name}"` });
        }
        names.add(name);

        if (field.isExisting) {
            const allowed = ALLOWED_MODE_TRANSITIONS[field.originalMode] ?? [field.originalMode];
            if (!allowed.includes(field.mode)) {
                errors.push({
                    path,
                    message: `Mode cannot change from ${field.originalMode} to ${field.mode}`,
                });
            }
        }
    });

    return errors;
}

export function editSchemaFieldsToTableFields(fields: EditSchemaFieldDraft[]) {
    return fields
        .filter((f) => f.name.trim())
        .map((field) => ({
            name: field.name.trim(),
            type: field.type,
            mode: field.mode,
            description: field.description?.trim() || null,
        }));
}

export function editSchemaDefaultValues(fields: EditSchemaFieldDraft[]): Record<string, string> {
    const out: Record<string, string> = {};
    for (const field of fields) {
        const value = field.defaultValue?.trim();
        if (value) out[field.name.trim()] = value;
    }
    return out;
}
