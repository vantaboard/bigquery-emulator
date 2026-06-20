import { explorerQueries } from '@/features/explorer/api';
import { schemaFieldsToBqPayload } from '@/features/resource/schema/schemaJson';
import type { JobRef } from '@/types/api';

import type { CreateTableFormState, CreateTableInsertBody } from './types';

const JOB_POLL_MS = 500;
const JOB_POLL_MAX = 120;

function keyValueRecord(pairs: { key: string; value: string }[]): Record<string, string> {
    const out: Record<string, string> = {};
    for (const pair of pairs) {
        const key = pair.key.trim();
        const value = pair.value.trim();
        if (key && value) out[key] = value;
    }
    return out;
}

function clusteringFieldList(raw: string): string[] {
    return raw
        .split(',')
        .map((s) => s.trim())
        .filter(Boolean);
}

export function buildInsertTableBody(form: CreateTableFormState): CreateTableInsertBody {
    const body: CreateTableInsertBody = {
        tableReference: {
            projectId: form.destinationProject.trim(),
            datasetId: form.destinationDataset.trim(),
            tableId: form.tableName.trim(),
        },
    };

    const schemaFields = schemaFieldsToBqPayload(form.schemaFields);
    if (schemaFields.length > 0) {
        body.schema = { fields: schemaFields };
    }

    const labels = keyValueRecord(form.labels);
    if (Object.keys(labels).length > 0) body.labels = labels;

    const tags = keyValueRecord(form.tags);
    if (Object.keys(tags).length > 0) body.tags = tags;

    if (form.partitionType === 'ingestion_time') {
        body.timePartitioning = { type: 'DAY' };
        if (form.partitionExpirationDays.trim()) {
            const days = Number(form.partitionExpirationDays);
            if (!Number.isNaN(days) && days > 0) {
                body.timePartitioning.expirationMs = days * 24 * 60 * 60 * 1000;
            }
        }
    } else if (form.partitionType === 'field' && form.partitionField.trim()) {
        body.timePartitioning = {
            type: 'DAY',
            field: form.partitionField.trim(),
        };
        if (form.partitionExpirationDays.trim()) {
            const days = Number(form.partitionExpirationDays);
            if (!Number.isNaN(days) && days > 0) {
                body.timePartitioning.expirationMs = days * 24 * 60 * 60 * 1000;
            }
        }
    }

    const clusterFields = clusteringFieldList(form.clusteringFields);
    if (clusterFields.length > 0) {
        body.clustering = { fields: clusterFields };
    }

    if (form.defaultCollation.trim()) body.defaultCollation = form.defaultCollation.trim();
    if (form.defaultRoundingMode.trim()) body.defaultRoundingMode = form.defaultRoundingMode.trim();

    if (form.encryptionType === 'cloud_kms' && form.kmsKeyName.trim()) {
        body.encryptionConfiguration = { kmsKeyName: form.kmsKeyName.trim() };
    }

    return body;
}

function buildLoadJobConfig(form: CreateTableFormState, sourceUris: string[]): Record<string, unknown> {
    const fmt = form.uploadFormat;
    const load: Record<string, unknown> = {
        destinationTable: {
            projectId: form.destinationProject.trim(),
            datasetId: form.destinationDataset.trim(),
            tableId: form.tableName.trim(),
        },
        sourceUris,
        sourceFormat: fmt.fileFormat,
        writeDisposition: fmt.writeDisposition,
        maxBadRecords: fmt.maxBadRecords,
        ignoreUnknownValues: fmt.ignoreUnknownValues,
    };

    const schemaFields = schemaFieldsToBqPayload(form.schemaFields);
    if (schemaFields.length > 0) {
        load.schema = { fields: schemaFields };
    }

    if (fmt.fileFormat === 'CSV') {
        load.fieldDelimiter = fmt.fieldDelimiter;
        load.quote = fmt.quote;
        load.skipLeadingRows = fmt.skipLeadingRows;
        load.allowJaggedRows = fmt.allowJaggedRows;
        load.allowQuotedNewlines = fmt.allowQuotedNewlines;
        if (fmt.nullMarker.trim()) load.nullMarker = fmt.nullMarker.trim();
        if (fmt.sourceColumnMatch.trim()) load.sourceColumnMatch = fmt.sourceColumnMatch.trim();
    }

    if (fmt.timeZone.trim()) load.timeZone = fmt.timeZone.trim();
    if (fmt.dateFormat.trim()) load.dateFormat = fmt.dateFormat.trim();
    if (fmt.datetimeFormat.trim()) load.datetimeFormat = fmt.datetimeFormat.trim();
    if (fmt.timestampFormat.trim()) load.timestampFormat = fmt.timestampFormat.trim();

    return { load };
}

function sourceUrisForForm(form: CreateTableFormState): string[] {
    switch (form.source) {
        case 'gcs':
            return form.gcsUri
                .split(/[\n,]/)
                .map((s) => s.trim())
                .filter(Boolean);
        case 'drive':
            return form.driveUri.trim() ? [form.driveUri.trim()] : [];
        case 'bigtable':
            return form.bigtableUri.trim() ? [form.bigtableUri.trim()] : [];
        case 's3':
            return form.s3Uri.trim() ? [form.s3Uri.trim()] : [];
        case 'azure':
            return form.azureUri.trim() ? [form.azureUri.trim()] : [];
        default:
            return [];
    }
}

function buildExistingTableQuery(form: CreateTableFormState): string {
    const dest = `\`${form.destinationProject.trim()}.${form.destinationDataset.trim()}.${form.tableName.trim()}\``;
    const src = `\`${form.sourceProject.trim()}.${form.sourceDataset.trim()}.${form.sourceTable.trim()}\``;
    return `CREATE TABLE ${dest} AS SELECT * FROM ${src}`;
}

export async function pollJobUntilDone(projectId: string, jobId: string): Promise<JobRef> {
    for (let attempt = 0; attempt < JOB_POLL_MAX; attempt += 1) {
        const job = await explorerQueries.getJob(projectId, jobId);
        if (job.state === 'DONE') {
            if (job.errorResult?.message) {
                throw new Error(job.errorResult.message);
            }
            return job;
        }
        await new Promise((resolve) => setTimeout(resolve, JOB_POLL_MS));
    }
    throw new Error('Job timed out');
}

export async function submitCreateTable(form: CreateTableFormState): Promise<void> {
    const projectId = form.destinationProject.trim();

    if (form.source === 'empty') {
        await explorerQueries.insertTable(projectId, form.destinationDataset.trim(), buildInsertTableBody(form));
        return;
    }

    if (form.source === 'existing') {
        const query = buildExistingTableQuery(form);
        const result = await explorerQueries.runQuery(query, projectId);
        if (result.rows.length === 0 && result.total_rows === 0) {
            return;
        }
        return;
    }

    if (form.source === 'upload' && form.uploadFile) {
        await explorerQueries.submitLoadJobWithUpload(projectId, form.uploadFile, buildLoadJobConfig(form, []));
        return;
    }

    const uris = sourceUrisForForm(form);
    const job = await explorerQueries.submitJob(projectId, {
        configuration: buildLoadJobConfig(form, uris),
    });
    await pollJobUntilDone(projectId, job.jobId);
}

export function isFileSource(source: CreateTableFormState['source']): boolean {
    return ['upload', 'gcs', 'drive', 's3', 'azure'].includes(source);
}

export function showUploadFormatOptions(source: CreateTableFormState['source']): boolean {
    return ['upload', 'gcs', 'drive', 's3', 'azure'].includes(source);
}
