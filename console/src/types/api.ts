export interface ExplorerConfig {
    allowEmulatorProjectAdmin: boolean;
}

export type ResourceType = 'TABLE' | 'VIEW' | 'MATERIALIZED_VIEW' | 'SNAPSHOT' | 'EXTERNAL';

export interface TableSchemaField {
    name: string;
    type: string;
    mode: string;
    description?: string | null;
}

export interface StorageStats {
    numRows: number;
    totalLogicalBytes: number;
    activeLogicalBytes: number;
    longTermLogicalBytes: number;
    currentPhysicalBytes: number;
    totalPhysicalBytes: number;
    activePhysicalBytes: number;
    longTermPhysicalBytes: number;
    timeTravelPhysicalBytes: number;
}

export interface TableMetadata {
    schema: TableSchemaField[];
    numRows: number;
    numBytes: number;
    creationTime: string;
    lastModified: string;
    description: string;
    type: string;
    location: string;
    fullyQualifiedName: string;
    resourceType: ResourceType;
    expirationTime: string | null;
    defaultCollation: string;
    defaultRoundingMode: string;
    caseInsensitive: boolean;
    useLegacySql: boolean;
    viewQuery: string;
    labels: Record<string, string>;
    primaryKeys: string[];
    tags: Record<string, string>;
    storage: StorageStats;
}

export interface QueryResponse {
    columns: string[];
    rows: Record<string, unknown>[];
    total_rows: number;
}

export interface DatasetReplica {
    location?: string;
    [key: string]: unknown;
}

export interface DatasetMetadata {
    id: string;
    friendlyName: string;
    description: string;
    location: string;
    creationTime: string;
    lastModifiedTime: string;
    defaultTableExpirationMs: number | null;
    defaultCollation: string;
    defaultRoundingMode: string;
    maxTimeTravelHours: number | null;
    isCaseInsensitive: boolean;
    labels: Record<string, string>;
    tags: Record<string, string>;
    replicas: DatasetReplica[];
}

export interface RoutineArgument {
    name: string;
    dataType: string;
    argumentKind: string;
}

export interface RoutineMetadata {
    id: string;
    routineType: string;
    language: string;
    definitionBody: string;
    arguments: RoutineArgument[];
    returnType: string;
    creationTime: string;
    lastModifiedTime: string;
}

export interface JobErrorResult {
    reason?: string;
    message?: string;
}

export interface JobRef {
    jobId: string;
    projectId: string;
    state: string;
    errorResult: JobErrorResult | null;
}

export interface TableDataPage {
    rows: Record<string, unknown>[];
    pageToken: string | null;
    totalRows: number;
}

export interface JobSubmitConfig {
    configuration: Record<string, unknown>;
    jobReference?: { jobId?: string };
}
