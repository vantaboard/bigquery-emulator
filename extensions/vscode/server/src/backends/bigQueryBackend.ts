import { GoogleAuth } from 'google-auth-library';
import { format as formatSql } from 'sql-formatter';

import {
  parseDryRunErrorMessage,
  type ConnectionSettings,
  type SqlCatalog,
  type SqlDiagnostic,
  type TableMetadata,
} from '@bigquery-emulator/vscode-shared';

import { CatalogService, catalogCompletions } from '../catalog.js';
import type { BackendContext, LanguageBackend } from './types.js';

interface QueryJobResponse {
  error?: {
    message?: string;
    errors?: Array<{ message?: string; location?: string }>;
  };
}

export class BigQueryBackend implements LanguageBackend {
  readonly name = 'bigquery';
  private settings: ConnectionSettings;
  private readonly auth: GoogleAuth;
  private readonly catalog: CatalogService;

  constructor(settings: ConnectionSettings) {
    this.settings = settings;
    this.auth = new GoogleAuth({
      scopes: ['https://www.googleapis.com/auth/bigquery'],
    });
    this.catalog = new CatalogService(
      'https://bigquery.googleapis.com',
      (input, init) => this.authorizedFetch(String(input), init),
    );
  }

  async initialize(settings: ConnectionSettings): Promise<void> {
    this.updateSettings(settings);
    await this.auth.getClient();
  }

  updateSettings(settings: ConnectionSettings): void {
    this.settings = settings;
  }

  private async authorizedFetch(url: string, init?: RequestInit): Promise<Response> {
    const client = await this.auth.getClient();
    const headers = await client.getRequestHeaders(url);
    return fetch(url, {
      ...init,
      headers: {
        'Content-Type': 'application/json',
        ...headers,
        ...(init?.headers as Record<string, string> | undefined),
      },
    });
  }

  async getDiagnostics(sql: string, _context: BackendContext): Promise<SqlDiagnostic[]> {
    if (!sql.trim()) {
      return [];
    }

    const projectId = this.settings.projectId;
    const url = `https://bigquery.googleapis.com/bigquery/v2/projects/${encodeURIComponent(projectId)}/queries`;
    const body = {
      query: sql,
      useLegacySql: false,
      dryRun: true,
      defaultDataset: this.settings.defaultDatasetId
        ? {
            projectId,
            datasetId: this.settings.defaultDatasetId,
          }
        : undefined,
    };

    const response = await this.authorizedFetch(url, {
      method: 'POST',
      body: JSON.stringify(body),
    });
    const payload = (await response.json()) as QueryJobResponse;

    if (response.ok) {
      return [];
    }

    const rawMessage =
      payload.error?.message ??
      payload.error?.errors?.[0]?.message ??
      `HTTP ${response.status}`;
    const parsed = parseDryRunErrorMessage(rawMessage);
    if (!parsed) {
      return [
        {
          line: 1,
          column: 1,
          message: rawMessage,
          severity: 'error',
        },
      ];
    }

    return [
      {
        line: parsed.line,
        column: parsed.column,
        endLine: parsed.endLine,
        endColumn: parsed.endColumn,
        message: parsed.message,
        severity: 'error',
      },
    ];
  }

  async getCompletions(sql: string, cursorOffset: number, _context: BackendContext) {
    const prefix = extractCompletionPrefix(sql, cursorOffset);
    const catalog = await this.getCatalog(this.settings.projectId);
    const candidates = catalogCompletions(catalog, prefix);
    if (candidates.length === 0) {
      return null;
    }

    const replacementStart = cursorOffset - prefix.length;
    return {
      candidates,
      replacementStart,
      replacementEnd: cursorOffset,
    };
  }

  async formatDocument(sql: string) {
    const formattedSql = formatSql(sql, { language: 'bigquery' });
    return { formattedSql, diagnostics: [] as SqlDiagnostic[] };
  }

  async analyze(): Promise<null> {
    return null;
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

function extractCompletionPrefix(sql: string, cursorOffset: number): string {
  const before = sql.slice(0, cursorOffset);
  const match = before.match(/[\w`.]*$/);
  return match?.[0] ?? '';
}
