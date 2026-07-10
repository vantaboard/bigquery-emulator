import { explorerQueries } from '@/features/explorer/api';

import { pollJobUntilDone } from '../createTable/submitCreateTable';

export interface TableReference {
    projectId: string;
    datasetId: string;
    tableId: string;
}

export function buildCopyTableJobConfig(opts: {
    source: TableReference;
    destination: TableReference;
    writeDisposition?: string;
    createDisposition?: string;
    encryption?: { kmsKeyName: string };
}): Record<string, unknown> {
    const copy: Record<string, unknown> = {
        sourceTable: opts.source,
        destinationTable: opts.destination,
        createDisposition: opts.createDisposition ?? 'CREATE_IF_NEEDED',
        writeDisposition: opts.writeDisposition ?? 'WRITE_EMPTY',
    };
    if (opts.encryption) {
        copy.destinationEncryptionConfiguration = opts.encryption;
    }
    return { copy };
}

export function buildSnapshotJobConfig(opts: {
    source: TableReference;
    destination: TableReference;
    destinationExpirationTime?: string;
    snapshotTimeOffsetMs?: number;
}): Record<string, unknown> {
    const sourceTable: TableReference = { ...opts.source };
    if (opts.snapshotTimeOffsetMs !== undefined && opts.snapshotTimeOffsetMs !== 0) {
        sourceTable.tableId = `${opts.source.tableId}@${opts.snapshotTimeOffsetMs}`;
    }
    const copy: Record<string, unknown> = {
        sourceTable,
        destinationTable: opts.destination,
        operationType: 'SNAPSHOT',
        writeDisposition: 'WRITE_EMPTY',
        createDisposition: 'CREATE_IF_NEEDED',
    };
    if (opts.destinationExpirationTime) {
        copy.destinationExpirationTime = opts.destinationExpirationTime;
    }
    return { copy };
}

export async function submitCopyJobAndWait(
    projectId: string,
    configuration: Record<string, unknown>,
): Promise<void> {
    const job = await explorerQueries.submitJob(projectId, { configuration });
    await pollJobUntilDone(projectId, job.jobId);
}

async function ensureDataset(projectId: string, datasetId: string, location: string): Promise<void> {
    try {
        await explorerQueries.datasetMetadata(projectId, datasetId);
    } catch {
        await explorerQueries.insertDataset(projectId, datasetId, location);
    }
}

export async function submitCopyDataset(opts: {
    projectId: string;
    sourceProject: string;
    sourceDataset: string;
    destProject: string;
    destDataset: string;
    location: string;
    overwrite: boolean;
}): Promise<void> {
    const tables = await explorerQueries.tables(opts.sourceProject, opts.sourceDataset);
    await ensureDataset(opts.destProject, opts.destDataset, opts.location);

    const writeDisposition = opts.overwrite ? 'WRITE_TRUNCATE' : 'WRITE_EMPTY';
    for (const { tableId } of tables) {
        await submitCopyJobAndWait(
            opts.projectId,
            buildCopyTableJobConfig({
                source: {
                    projectId: opts.sourceProject,
                    datasetId: opts.sourceDataset,
                    tableId,
                },
                destination: {
                    projectId: opts.destProject,
                    datasetId: opts.destDataset,
                    tableId,
                },
                writeDisposition,
            }),
        );
    }
}
