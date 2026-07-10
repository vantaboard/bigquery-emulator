import {
  SqlToolsClient,
  type ConnectionSettings,
  type SqlCatalog,
  type TableMetadata,
} from '@bigquery-emulator/vscode-shared';

import { CatalogService } from '../catalog.js';
import type { BackendContext, LanguageBackend } from './types.js';

export class SqlToolsBackend implements LanguageBackend {
  readonly name = 'emulator';
  private settings: ConnectionSettings;
  private readonly client: SqlToolsClient;
  private readonly catalog: CatalogService;

  constructor(settings: ConnectionSettings) {
    this.settings = settings;
    this.client = new SqlToolsClient({
      baseUrl: settings.emulatorBaseUrl,
      token: settings.sqlToolsToken,
    });
    this.catalog = new CatalogService(settings.emulatorBaseUrl);
  }

  async initialize(settings: ConnectionSettings): Promise<void> {
    this.updateSettings(settings);
    await this.client.probeCapabilities();
  }

  updateSettings(settings: ConnectionSettings): void {
    this.settings = settings;
    this.client.resetProbe();
    this.catalog.updateBaseUrl(settings.emulatorBaseUrl);
  }

  async getDiagnostics(sql: string, _context: BackendContext): Promise<import('@bigquery-emulator/vscode-shared').SqlDiagnostic[]> {
    if (!sql.trim()) {
      return [];
    }
    const available = await this.client.probeCapabilities();
    if (!available) {
      return [];
    }
    const result = await this.client.parse({ sql, offsetUnit: 'utf16' });
    return result.diagnostics;
  }

  async getCompletions(sql: string, cursorOffset: number, _context: BackendContext) {
    const available = await this.client.probeCapabilities();
    if (!available) {
      return null;
    }
    return this.client.complete({
      sql,
      cursorByteOffset: cursorOffset,
      projectId: this.settings.projectId,
      defaultDatasetId: this.settings.defaultDatasetId,
      offsetUnit: 'utf16',
    });
  }

  async formatDocument(sql: string) {
    const available = await this.client.probeCapabilities();
    if (!available) {
      throw new Error('SQL Tools unavailable');
    }
    return this.client.format({
      sql,
      strict: this.settings.strictFormat,
      indentationSpaces: this.settings.formatIndentationSpaces,
      lineLengthLimit: this.settings.formatLineLengthLimit,
      offsetUnit: 'utf16',
    });
  }

  async analyze(sql: string) {
    const available = await this.client.probeCapabilities();
    if (!available) {
      return null;
    }
    return this.client.analyze({
      sql,
      projectId: this.settings.projectId,
      defaultDatasetId: this.settings.defaultDatasetId,
      offsetUnit: 'utf16',
    });
  }

  async getCatalog(projectId: string): Promise<SqlCatalog> {
    return this.catalog.load(projectId);
  }

  async getTableMetadata(
    projectId: string,
    datasetId: string,
    tableId: string,
  ): Promise<TableMetadata | null> {
    return this.catalog.getTable(projectId, datasetId, tableId);
  }
}
