export interface ExplorerConfig {
    allowEmulatorProjectAdmin: boolean;
}

export interface TableSchemaField {
    name: string;
    type: string;
    mode: string;
    description?: string | null;
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
}

export interface QueryResponse {
    columns: string[];
    rows: Record<string, unknown>[];
    total_rows: number;
}
