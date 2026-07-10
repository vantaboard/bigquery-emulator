export type OffsetUnit = 'utf8' | 'utf16';

export type BackendMode = 'auto' | 'emulator' | 'bigquery';

export interface SqlCapabilities {
  sqlTools: boolean;
  version: string;
  endpoints: string[];
  offsetUnits: string[];
}

export interface SqlDiagnostic {
  line: number;
  column: number;
  message: string;
  severity: string;
  endLine?: number;
  endColumn?: number;
  startByte?: number;
  endByte?: number;
  startUtf16?: number;
  endUtf16?: number;
}

export interface FormatRequest {
  sql: string;
  strict?: boolean;
  lineLengthLimit?: number;
  indentationSpaces?: number;
  offsetUnit?: OffsetUnit;
}

export interface FormatResponse {
  formattedSql: string;
  diagnostics: SqlDiagnostic[];
}

export interface ParseRequest {
  sql: string;
  offsetUnit?: OffsetUnit;
}

export interface ParseResponse {
  statementKinds: string[];
  diagnostics: SqlDiagnostic[];
}

export interface CompleteRequest {
  sql: string;
  cursorByteOffset: number;
  projectId?: string;
  defaultDatasetId?: string;
  offsetUnit?: OffsetUnit;
}

export interface CompletionCandidate {
  label: string;
  kind: string;
  insertText: string;
  detail?: string;
  fqn?: string;
}

export interface CompleteResponse {
  candidates: CompletionCandidate[];
  replacementStart: number;
  replacementEnd: number;
}

export interface AnalyzeRequest {
  sql: string;
  projectId?: string;
  defaultDatasetId?: string;
  offsetUnit?: OffsetUnit;
}

export interface ReferencedTable {
  projectId: string;
  datasetId: string;
  tableId: string;
  alias?: string;
  kind: string;
}

export interface AnalyzeResponse {
  referencedTables: ReferencedTable[];
  statementKinds: string[];
  diagnostics: SqlDiagnostic[];
}

export interface SqlCatalog {
  schema: Record<string, readonly string[]>;
  qualifiedTables: string[];
  routines: string[];
}

export interface ConnectionSettings {
  backendMode: BackendMode;
  emulatorBaseUrl: string;
  sqlToolsToken?: string;
  projectId: string;
  defaultDatasetId?: string;
  strictFormat: boolean;
  formatIndentationSpaces: number;
  formatLineLengthLimit: number;
}

export interface TableField {
  name: string;
  type: string;
  mode?: string;
}

export interface TableSchema {
  fields: TableField[];
}

export interface TableMetadata {
  tableReference: {
    projectId: string;
    datasetId: string;
    tableId: string;
  };
  schema?: TableSchema;
  type?: string;
}

export interface DryRunError {
  message: string;
  line: number;
  column: number;
  endLine?: number;
  endColumn?: number;
}
