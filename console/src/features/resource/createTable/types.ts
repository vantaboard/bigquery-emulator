import type { SchemaFieldDraft } from '@/features/resource/schema/types';

export type CreateTableSource =
    | 'empty'
    | 'gcs'
    | 'upload'
    | 'drive'
    | 'bigtable'
    | 's3'
    | 'azure'
    | 'existing';

export type FileFormat = 'CSV' | 'NEWLINE_DELIMITED_JSON' | 'AVRO' | 'PARQUET' | 'ORC';

export type PartitionType = 'none' | 'ingestion_time' | 'field';

export interface KeyValuePair {
    id: string;
    key: string;
    value: string;
}

export interface UploadFormatOptions {
    fileFormat: FileFormat;
    writeDisposition: 'WRITE_TRUNCATE' | 'WRITE_APPEND' | 'WRITE_EMPTY';
    maxBadRecords: number;
    allowJaggedRows: boolean;
    allowQuotedNewlines: boolean;
    ignoreUnknownValues: boolean;
    fieldDelimiter: string;
    quote: string;
    skipLeadingRows: number;
    nullMarker: string;
    sourceColumnMatch: string;
    timeZone: string;
    dateFormat: string;
    datetimeFormat: string;
    timestampFormat: string;
}

export interface CreateTableFormState {
    source: CreateTableSource;
    destinationProject: string;
    destinationDataset: string;
    tableName: string;
    schemaFields: SchemaFieldDraft[];
    partitionType: PartitionType;
    partitionField: string;
    partitionExpirationDays: string;
    clusteringFields: string;
    tagScope: string;
    tags: KeyValuePair[];
    labels: KeyValuePair[];
    encryptionType: 'google_managed' | 'cloud_kms';
    kmsKeyName: string;
    defaultCollation: string;
    defaultRoundingMode: string;
    gcsUri: string;
    uploadFile: File | null;
    driveUri: string;
    bigtableUri: string;
    s3Uri: string;
    s3Connection: string;
    azureUri: string;
    azureConnection: string;
    sourceProject: string;
    sourceDataset: string;
    sourceTable: string;
    uploadFormat: UploadFormatOptions;
}

export interface CreateTableInsertBody {
    tableReference: { projectId: string; datasetId: string; tableId: string };
    schema?: { fields: unknown[] };
    description?: string;
    labels?: Record<string, string>;
    tags?: Record<string, string>;
    timePartitioning?: Record<string, unknown>;
    rangePartitioning?: Record<string, unknown>;
    clustering?: { fields: string[] };
    defaultCollation?: string;
    defaultRoundingMode?: string;
    encryptionConfiguration?: { kmsKeyName: string };
}
