import { createEmptySchemaField } from '@/features/resource/schema/schemaJson';

import type { CreateTableFormState } from './types';

let nextKvId = 0;

export function newKeyValueId(): string {
    nextKvId += 1;
    return `kv-${nextKvId}`;
}

export function defaultCreateTableForm(projectId: string, datasetId: string): CreateTableFormState {
    return {
        source: 'empty',
        destinationProject: projectId,
        destinationDataset: datasetId,
        tableName: '',
        schemaFields: [createEmptySchemaField(), createEmptySchemaField()],
        partitionType: 'none',
        partitionField: '',
        partitionExpirationDays: '',
        clusteringFields: '',
        tagScope: '',
        tags: [],
        labels: [],
        encryptionType: 'google_managed',
        kmsKeyName: '',
        defaultCollation: '',
        defaultRoundingMode: '',
        gcsUri: '',
        uploadFile: null,
        driveUri: '',
        bigtableUri: '',
        s3Uri: '',
        s3Connection: '',
        azureUri: '',
        azureConnection: '',
        sourceProject: projectId,
        sourceDataset: datasetId,
        sourceTable: '',
        uploadFormat: {
            fileFormat: 'CSV',
            writeDisposition: 'WRITE_EMPTY',
            maxBadRecords: 0,
            allowJaggedRows: false,
            allowQuotedNewlines: false,
            ignoreUnknownValues: false,
            fieldDelimiter: ',',
            quote: '"',
            skipLeadingRows: 1,
            nullMarker: '',
            sourceColumnMatch: '',
            timeZone: '',
            dateFormat: '',
            datetimeFormat: '',
            timestampFormat: '',
        },
    };
}

export const CREATE_TABLE_SOURCES: { id: CreateTableFormState['source']; label: string }[] = [
    { id: 'empty', label: 'Empty table' },
    { id: 'gcs', label: 'Google Cloud Storage' },
    { id: 'upload', label: 'Upload' },
    { id: 'drive', label: 'Drive' },
    { id: 'bigtable', label: 'Google Bigtable' },
    { id: 's3', label: 'Amazon S3' },
    { id: 'azure', label: 'Azure Blob Storage' },
    { id: 'existing', label: 'Existing table/view' },
];

export const FILE_FORMAT_OPTIONS = [
    { value: 'CSV', label: 'CSV' },
    { value: 'NEWLINE_DELIMITED_JSON', label: 'JSON (Newline delimited)' },
    { value: 'AVRO', label: 'Avro' },
    { value: 'PARQUET', label: 'Parquet' },
    { value: 'ORC', label: 'ORC' },
] as const;

export const ROUNDING_MODES = ['', 'ROUND_HALF_AWAY_FROM_ZERO', 'ROUND_HALF_EVEN'] as const;
