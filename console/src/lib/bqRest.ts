import type {
    DatasetMetadata,
    JobRef,
    QueryResponse,
    ResourceType,
    RoutineMetadata,
    StorageStats,
    TableDataPage,
    TableMetadata,
    TableSchemaField,
} from '@/types/api';

interface BqProjectList {
    projects?: Array<{ id?: string; projectReference?: { projectId?: string } }>;
}

interface BqDatasetList {
    datasets?: Array<{ datasetReference?: { datasetId?: string } }>;
}

interface BqTableList {
    tables?: Array<{ tableReference?: { tableId?: string } }>;
}

interface BqRoutineList {
    routines?: Array<{ routineReference?: { routineId?: string } }>;
}

interface BqSchemaField {
    name?: string;
    type?: string;
    mode?: string;
    description?: string;
}

interface BqPrimaryKeyColumn {
    fieldIndex?: number;
    name?: string;
}

interface BqTable {
    schema?: { fields?: BqSchemaField[] };
    numRows?: string;
    numBytes?: string;
    numLongTermBytes?: string;
    numTotalLogicalBytes?: string;
    numActiveLogicalBytes?: string;
    numPhysicalBytes?: string;
    numTotalPhysicalBytes?: string;
    numActivePhysicalBytes?: string;
    numLongTermPhysicalBytes?: string;
    numCurrentPhysicalBytes?: string;
    numTimeTravelPhysicalBytes?: string;
    creationTime?: string;
    lastModifiedTime?: string;
    expirationTime?: string;
    description?: string;
    type?: string;
    location?: string;
    defaultCollation?: string;
    defaultRoundingMode?: string;
    caseInsensitive?: boolean;
    labels?: Record<string, string>;
    tags?: Record<string, string>;
    view?: { query?: string; useLegacySql?: boolean };
    materializedView?: { query?: string };
    tableConstraints?: { primaryKey?: { columns?: BqPrimaryKeyColumn[] } };
}

interface BqDataset {
    datasetReference?: { datasetId?: string; projectId?: string };
    friendlyName?: string;
    description?: string;
    location?: string;
    creationTime?: string;
    lastModifiedTime?: string;
    defaultTableExpirationMs?: string;
    defaultCollation?: string;
    defaultRoundingMode?: string;
    maxTimeTravelHours?: string;
    isCaseInsensitive?: boolean;
    labels?: Record<string, string>;
    tags?: Record<string, string>;
    replicas?: Array<{ location?: string; [key: string]: unknown }>;
}

interface BqRoutineArgument {
    name?: string;
    dataType?: { typeKind?: string };
    argumentKind?: string;
}

interface BqRoutine {
    routineReference?: { routineId?: string };
    routineType?: string;
    language?: string;
    definitionBody?: string;
    arguments?: BqRoutineArgument[];
    returnType?: { typeKind?: string };
    creationTime?: string;
    lastModifiedTime?: string;
}

interface BqQueryResponse {
    schema?: { fields?: BqSchemaField[] };
    rows?: Array<{ f?: Array<{ v?: unknown }> }>;
    totalRows?: string;
}

interface BqTableData {
    rows?: Array<{ f?: Array<{ v?: unknown }> }>;
    pageToken?: string;
    totalRows?: string;
}

interface BqJob {
    jobReference?: { jobId?: string; projectId?: string };
    status?: {
        state?: string;
        errorResult?: { reason?: string; message?: string };
    };
}

function parseBqValue(raw: unknown, type?: string): unknown {
    if (raw === null || raw === undefined) return null;
    const upper = (type ?? '').toUpperCase();
    if (upper.includes('INT') || upper === 'NUMERIC' || upper === 'BIGNUMERIC') {
        const n = Number(raw);
        return Number.isNaN(n) ? raw : n;
    }
    if (upper === 'FLOAT' || upper === 'FLOAT64') {
        const n = Number(raw);
        return Number.isNaN(n) ? raw : n;
    }
    if (upper === 'BOOLEAN' || upper === 'BOOL') {
        return raw === 'true' || raw === true;
    }
    return raw;
}

function bqTimestamp(ms?: string): string {
    if (!ms) return new Date(0).toISOString();
    const n = Number(ms);
    return Number.isNaN(n) ? new Date(0).toISOString() : new Date(n).toISOString();
}

function bqTimestampOrNull(ms?: string): string | null {
    if (!ms) return null;
    const n = Number(ms);
    return Number.isNaN(n) ? null : new Date(n).toISOString();
}

function labelsFromBq(labels?: Record<string, string>): Record<string, string> {
    return labels ?? {};
}

export function projectIdsFromList(data: BqProjectList): string[] {
    return (data.projects ?? [])
        .map((p) => p.id ?? p.projectReference?.projectId)
        .filter((id): id is string => Boolean(id));
}

export function datasetIdsFromList(data: BqDatasetList): string[] {
    return (data.datasets ?? [])
        .map((d) => d.datasetReference?.datasetId)
        .filter((id): id is string => Boolean(id));
}

export function tableIdsFromList(data: BqTableList): string[] {
    return (data.tables ?? [])
        .map((t) => t.tableReference?.tableId)
        .filter((id): id is string => Boolean(id));
}

export function routineIdsFromList(data: BqRoutineList): string[] {
    return (data.routines ?? [])
        .map((r) => r.routineReference?.routineId)
        .filter((id): id is string => Boolean(id));
}

export function resourceTypeFromBq(raw: BqTable): ResourceType {
    const t = (raw.type ?? 'TABLE').toUpperCase();
    if (
        t === 'TABLE' ||
        t === 'VIEW' ||
        t === 'MATERIALIZED_VIEW' ||
        t === 'SNAPSHOT' ||
        t === 'EXTERNAL'
    ) {
        return t;
    }
    return 'TABLE';
}

function storageStatsFromBq(table: BqTable): StorageStats {
    const numRows = Number(table.numRows ?? 0);
    return {
        numRows,
        totalLogicalBytes: Number(table.numTotalLogicalBytes ?? table.numBytes ?? 0),
        activeLogicalBytes: Number(table.numActiveLogicalBytes ?? 0),
        longTermLogicalBytes: Number(table.numLongTermBytes ?? 0),
        currentPhysicalBytes: Number(table.numCurrentPhysicalBytes ?? 0),
        totalPhysicalBytes: Number(table.numPhysicalBytes ?? table.numTotalPhysicalBytes ?? 0),
        activePhysicalBytes: Number(table.numActivePhysicalBytes ?? 0),
        longTermPhysicalBytes: Number(table.numLongTermPhysicalBytes ?? 0),
        timeTravelPhysicalBytes: Number(table.numTimeTravelPhysicalBytes ?? 0),
    };
}

function primaryKeysFromBq(table: BqTable): string[] {
    const columns = table.tableConstraints?.primaryKey?.columns ?? [];
    const fields = table.schema?.fields ?? [];
    return columns
        .map((col) => {
            const idx = col.fieldIndex ?? -1;
            if (idx >= 0 && idx < fields.length) {
                return fields[idx].name ?? '';
            }
            return col.name ?? '';
        })
        .filter((name): name is string => Boolean(name));
}

export function datasetMetadataFromBq(_projectId: string, datasetId: string, raw: BqDataset): DatasetMetadata {
    const id = raw.datasetReference?.datasetId ?? datasetId;
    const defaultTableExpirationMs = raw.defaultTableExpirationMs
        ? Number(raw.defaultTableExpirationMs)
        : null;
    const maxTimeTravelHours = raw.maxTimeTravelHours ? Number(raw.maxTimeTravelHours) : null;

    return {
        id,
        friendlyName: raw.friendlyName ?? id,
        description: raw.description ?? '',
        location: raw.location ?? '',
        creationTime: bqTimestamp(raw.creationTime),
        lastModifiedTime: bqTimestamp(raw.lastModifiedTime ?? raw.creationTime),
        defaultTableExpirationMs: Number.isNaN(defaultTableExpirationMs as number) ? null : defaultTableExpirationMs,
        defaultCollation: raw.defaultCollation ?? '',
        defaultRoundingMode: raw.defaultRoundingMode ?? '',
        maxTimeTravelHours: Number.isNaN(maxTimeTravelHours as number) ? null : maxTimeTravelHours,
        isCaseInsensitive: raw.isCaseInsensitive ?? false,
        labels: labelsFromBq(raw.labels),
        tags: labelsFromBq(raw.tags),
        replicas: raw.replicas ?? [],
    };
}

export function tableMetadataFromBq(
    projectId: string,
    datasetId: string,
    tableId: string,
    table: BqTable,
): TableMetadata {
    const schema: TableSchemaField[] = (table.schema?.fields ?? []).map((f) => ({
        name: f.name ?? '',
        type: f.type ?? 'STRING',
        mode: f.mode ?? 'NULLABLE',
        description: f.description ?? null,
    }));
    const storage = storageStatsFromBq(table);
    const resourceType = resourceTypeFromBq(table);
    const viewQuery = table.view?.query ?? table.materializedView?.query ?? '';

    return {
        schema,
        numRows: storage.numRows,
        numBytes: Number(table.numBytes ?? storage.totalLogicalBytes),
        creationTime: bqTimestamp(table.creationTime),
        lastModified: bqTimestamp(table.lastModifiedTime ?? table.creationTime),
        description: table.description ?? '',
        type: table.type ?? 'TABLE',
        location: table.location ?? '',
        fullyQualifiedName: `${projectId}.${datasetId}.${tableId}`,
        resourceType,
        expirationTime: bqTimestampOrNull(table.expirationTime),
        defaultCollation: table.defaultCollation ?? '',
        defaultRoundingMode: table.defaultRoundingMode ?? '',
        caseInsensitive: table.caseInsensitive ?? false,
        useLegacySql: table.view?.useLegacySql ?? false,
        viewQuery,
        labels: labelsFromBq(table.labels),
        primaryKeys: primaryKeysFromBq(table),
        tags: labelsFromBq(table.tags),
        storage,
    };
}

export function routineFromBq(
    _projectId: string,
    _datasetId: string,
    routineId: string,
    raw: BqRoutine,
): RoutineMetadata {
    const id = raw.routineReference?.routineId ?? routineId;
    return {
        id,
        routineType: raw.routineType ?? '',
        language: raw.language ?? '',
        definitionBody: raw.definitionBody ?? '',
        arguments: (raw.arguments ?? []).map((arg) => ({
            name: arg.name ?? '',
            dataType: arg.dataType?.typeKind ?? '',
            argumentKind: arg.argumentKind ?? '',
        })),
        returnType: raw.returnType?.typeKind ?? '',
        creationTime: bqTimestamp(raw.creationTime),
        lastModifiedTime: bqTimestamp(raw.lastModifiedTime ?? raw.creationTime),
    };
}

export function tableDataFromBq(raw: BqTableData, schema: TableSchemaField[]): TableDataPage {
    const columns = schema.map((f) => f.name);
    const rows = (raw.rows ?? []).map((row) => {
        const out: Record<string, unknown> = {};
        columns.forEach((col, i) => {
            const cell = row.f?.[i];
            out[col] = parseBqValue(cell?.v ?? null, schema[i]?.type);
        });
        return out;
    });
    return {
        rows,
        pageToken: raw.pageToken ?? null,
        totalRows: Number(raw.totalRows ?? rows.length),
    };
}

export function queryResponseFromBq(data: BqQueryResponse): QueryResponse {
    const fields = data.schema?.fields ?? [];
    const columns = fields.map((f) => f.name ?? '');
    const rows = (data.rows ?? []).map((row) => {
        const out: Record<string, unknown> = {};
        columns.forEach((col, i) => {
            const cell = row.f?.[i];
            out[col] = parseBqValue(cell?.v ?? null, fields[i]?.type);
        });
        return out;
    });
    return {
        columns,
        rows,
        total_rows: Number(data.totalRows ?? rows.length),
    };
}

export function jobRefFromBq(raw: BqJob): JobRef {
    return {
        jobId: raw.jobReference?.jobId ?? '',
        projectId: raw.jobReference?.projectId ?? '',
        state: raw.status?.state ?? '',
        errorResult: raw.status?.errorResult ?? null,
    };
}

export type {
    BqDataset,
    BqDatasetList,
    BqJob,
    BqProjectList,
    BqQueryResponse,
    BqRoutine,
    BqRoutineList,
    BqTable,
    BqTableData,
    BqTableList,
};
