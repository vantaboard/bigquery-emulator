import { TYPES_WITH_MAX_LENGTH } from './constants';
import type { SchemaFieldDraft, SchemaValidationError } from './types';

function validateField(field: SchemaFieldDraft, path: string, errors: SchemaValidationError[]): void {
    const name = field.name.trim();
    if (!name) {
        errors.push({ path, message: 'Field name is required' });
    } else if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(name)) {
        errors.push({ path, message: 'Field name must start with a letter or underscore' });
    }

    if (field.type === 'RECORD') {
        const nested = field.fields ?? [];
        if (nested.length === 0) {
            errors.push({ path, message: 'RECORD fields must have at least one nested field' });
        }
        nested.forEach((child, index) => validateField(child, `${path}.${index}`, errors));
    }

    if (field.type === 'RANGE' && !field.rangeElementType) {
        errors.push({ path, message: 'RANGE fields require an element type' });
    }

    if (TYPES_WITH_MAX_LENGTH.has(field.type) && field.maxLength !== undefined && field.maxLength <= 0) {
        errors.push({ path, message: 'Max length must be a positive number' });
    }
}

export function validateSchemaFields(fields: SchemaFieldDraft[]): SchemaValidationError[] {
    const errors: SchemaValidationError[] = [];
    if (fields.length === 0) {
        errors.push({ path: 'schema', message: 'At least one schema field is required' });
        return errors;
    }
    fields.forEach((field, index) => validateField(field, String(index), errors));
    return errors;
}

export function isValidTableId(name: string): boolean {
    const trimmed = name.trim();
    if (!trimmed || trimmed.length > 1024) return false;
    return /^[A-Za-z_][A-Za-z0-9_]*$/.test(trimmed);
}
