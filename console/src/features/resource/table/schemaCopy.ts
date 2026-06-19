import type { TableSchemaField } from '@/types/api';

const SCHEMA_COLUMNS = [
    'Field name',
    'Type',
    'Mode',
    'Description',
    'Key',
    'Collation',
    'Default Value',
    'Policy Tags',
    'Data Policies',
] as const;

export type SchemaGridRow = TableSchemaField & {
    key: string;
};

export function schemaGridRows(
    fields: TableSchemaField[],
    primaryKeys: string[],
): SchemaGridRow[] {
    const pkSet = new Set(primaryKeys);
    return fields.map((field) => ({
        ...field,
        key: pkSet.has(field.name) ? 'PK' : '',
    }));
}

export function formatSchemaAsJson(fields: TableSchemaField[]): string {
    const payload = fields.map((field) => {
        const entry: Record<string, string> = {
            name: field.name,
            type: field.type,
            mode: field.mode,
        };
        if (field.description?.trim()) {
            entry.description = field.description;
        }
        return entry;
    });
    return JSON.stringify(payload, null, 2);
}

function cellValue(value: string | null | undefined): string {
    return value?.trim() ? value : '';
}

export function formatSchemaAsTable(rows: SchemaGridRow[]): string {
    const header = SCHEMA_COLUMNS.join('\t');
    const body = rows
        .map((row) =>
            [
                row.name,
                row.type,
                row.mode,
                cellValue(row.description),
                row.key,
                '',
                '',
                '',
                '',
            ].join('\t'),
        )
        .join('\n');
    return body ? `${header}\n${body}` : header;
}
