import { format as formatSql } from 'sql-formatter';

import type { ConnectionSettings } from '@bigquery-emulator/vscode-shared';

import { SqlToolsBackend } from './sqlToolsBackend.js';
import type { LanguageBackend } from './types.js';

/**
 * Browser-safe backend manager: emulator SQL Tools only.
 * Production BigQuery (ADC / google-auth-library) is Node-only.
 */
export class BrowserBackendManager {
  private settings: ConnectionSettings;
  private readonly emulatorBackend: SqlToolsBackend;
  private activeName = 'emulator';

  constructor(settings: ConnectionSettings) {
    this.settings = { ...settings, backendMode: 'emulator' };
    this.emulatorBackend = new SqlToolsBackend(this.settings);
  }

  async initialize(): Promise<void> {
    await this.emulatorBackend.initialize(this.settings);
  }

  async updateSettings(settings: ConnectionSettings): Promise<void> {
    this.settings = { ...settings, backendMode: 'emulator' };
    this.emulatorBackend.updateSettings(this.settings);
    await this.emulatorBackend.initialize(this.settings);
  }

  getActiveName(): string {
    return this.activeName;
  }

  getBackend(): LanguageBackend {
    return this.emulatorBackend;
  }
}

export async function formatWithFallbackBrowser(
  backend: LanguageBackend,
  sql: string,
  settings: ConnectionSettings,
): Promise<string> {
  try {
    const result = await backend.formatDocument(sql, { settings });
    return result.formattedSql;
  } catch {
    return formatSql(sql, { language: 'bigquery' });
  }
}
