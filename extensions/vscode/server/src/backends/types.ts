import type {
  AnalyzeResponse,
  CompleteResponse,
  ConnectionSettings,
  FormatResponse,
  SqlCatalog,
  SqlDiagnostic,
  TableMetadata,
} from '@bigquery-emulator/vscode-shared';
import type { TextDocument } from 'vscode-languageserver-textdocument';

export interface BackendContext {
  settings: ConnectionSettings;
  document?: TextDocument;
}

export interface LanguageBackend {
  readonly name: string;
  initialize(settings: ConnectionSettings): Promise<void>;
  updateSettings(settings: ConnectionSettings): void;
  getDiagnostics(sql: string, context: BackendContext): Promise<SqlDiagnostic[]>;
  getCompletions(
    sql: string,
    cursorOffset: number,
    context: BackendContext,
  ): Promise<CompleteResponse | null>;
  formatDocument(sql: string, context: BackendContext): Promise<FormatResponse>;
  analyze(sql: string, context: BackendContext): Promise<AnalyzeResponse | null>;
  getCatalog(projectId: string): Promise<SqlCatalog>;
  getTableMetadata(
    projectId: string,
    datasetId: string,
    tableId: string,
  ): Promise<TableMetadata | null>;
}
