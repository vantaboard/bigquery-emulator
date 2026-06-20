import type { SchemaFieldDraft } from './types';

export interface BqSchemaFieldPayload {
    name: string;
    type: string;
    mode?: string;
    description?: string;
    maxLength?: number;
    rangeElementType?: { type: string };
    fields?: BqSchemaFieldPayload[];
}

let nextFieldId = 0;

export function newSchemaFieldId(): string {
    nextFieldId += 1;
    return `field-${nextFieldId}`;
}

export function createEmptySchemaField(): SchemaFieldDraft {
    return {
        id: newSchemaFieldId(),
        name: '',
        type: 'STRING',
        mode: 'NULLABLE',
    };
}

export function schemaFieldsToBqPayload(fields: SchemaFieldDraft[]): BqSchemaFieldPayload[] {
    return fields
        .filter((f) => f.name.trim())
        .map((field) => {
            const payload: BqSchemaFieldPayload = {
                name: field.name.trim(),
                type: field.type,
            };
            if (field.mode && field.mode !== 'NULLABLE') {
                payload.mode = field.mode;
            }
            if (field.description?.trim()) {
                payload.description = field.description.trim();
            }
            if (field.maxLength !== undefined && field.maxLength > 0) {
                payload.maxLength = field.maxLength;
            }
            if (field.type === 'RANGE' && field.rangeElementType) {
                payload.rangeElementType = { type: field.rangeElementType };
            }
            if (field.type === 'RECORD' && field.fields?.length) {
                payload.fields = schemaFieldsToBqPayload(field.fields);
            }
            return payload;
        });
}

function bqPayloadToSchemaField(raw: BqSchemaFieldPayload, id?: string): SchemaFieldDraft {
    const field: SchemaFieldDraft = {
        id: id ?? newSchemaFieldId(),
        name: raw.name ?? '',
        type: raw.type ?? 'STRING',
        mode: raw.mode ?? 'NULLABLE',
    };
    if (raw.description) field.description = raw.description;
    if (raw.maxLength !== undefined) field.maxLength = raw.maxLength;
    if (raw.rangeElementType?.type) field.rangeElementType = raw.rangeElementType.type;
    if (raw.fields?.length) {
        field.fields = raw.fields.map((nested) => bqPayloadToSchemaField(nested));
    }
    return field;
}

export function schemaFieldsFromBqPayload(raw: BqSchemaFieldPayload[]): SchemaFieldDraft[] {
    return raw.map((field) => bqPayloadToSchemaField(field));
}

export function formatSchemaFieldsJson(fields: SchemaFieldDraft[]): string {
    return JSON.stringify(schemaFieldsToBqPayload(fields), null, 2);
}

export function parseSchemaFieldsJson(text: string): SchemaFieldDraft[] {
    const parsed = JSON.parse(text) as unknown;
    if (!Array.isArray(parsed)) {
        throw new Error('Schema JSON must be an array of field objects');
    }
    return schemaFieldsFromBqPayload(parsed as BqSchemaFieldPayload[]);
}
