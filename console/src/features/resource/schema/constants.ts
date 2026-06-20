export const SCHEMA_FIELD_TYPES = [
    'STRING',
    'BYTES',
    'INTEGER',
    'INT64',
    'FLOAT',
    'FLOAT64',
    'BOOLEAN',
    'BOOL',
    'TIMESTAMP',
    'DATE',
    'TIME',
    'DATETIME',
    'GEOGRAPHY',
    'NUMERIC',
    'BIGNUMERIC',
    'JSON',
    'RECORD',
    'RANGE',
] as const;

export const SCHEMA_FIELD_MODES = ['NULLABLE', 'REQUIRED', 'REPEATED'] as const;

export const RANGE_ELEMENT_TYPES = ['DATE', 'DATETIME', 'TIMESTAMP'] as const;

export const TYPES_WITH_MAX_LENGTH = new Set(['STRING', 'BYTES']);
