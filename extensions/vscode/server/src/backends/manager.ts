import { format as formatSql } from 'sql-formatter';

import { SqlToolsClient, type ConnectionSettings } from '@bigquery-emulator/vscode-shared';

import { BigQueryBackend } from './bigQueryBackend.js';
import { SqlToolsBackend } from './sqlToolsBackend.js';
import type { LanguageBackend } from './types.js';

export class BackendManager {
  private settings: ConnectionSettings;
  private activeBackend: LanguageBackend;
  private readonly emulatorBackend: SqlToolsBackend;
  private readonly bigQueryBackend: BigQueryBackend;
  private activeName = 'emulator';

  constructor(settings: ConnectionSettings) {
    this.settings = settings;
    this.emulatorBackend = new SqlToolsBackend(settings);
    this.bigQueryBackend = new BigQueryBackend(settings);
    this.activeBackend = this.emulatorBackend;
  }

  async initialize(): Promise<void> {
    await this.selectBackend();
  }

  async updateSettings(settings: ConnectionSettings): Promise<void> {
    this.settings = settings;
    this.emulatorBackend.updateSettings(settings);
    this.bigQueryBackend.updateSettings(settings);
    await this.selectBackend();
  }

  getActiveName(): string {
    return this.activeName;
  }

  getBackend(): LanguageBackend {
    return this.activeBackend;
  }

  private async selectBackend(): Promise<void> {
    if (this.settings.backendMode === 'emulator') {
      this.activeBackend = this.emulatorBackend;
      this.activeName = 'emulator';
      await this.emulatorBackend.initialize(this.settings);
      return;
    }

    if (this.settings.backendMode === 'bigquery') {
      this.activeBackend = this.bigQueryBackend;
      this.activeName = 'bigquery';
      await this.bigQueryBackend.initialize(this.settings);
      return;
    }

    const client = new SqlToolsClient({
      baseUrl: this.settings.emulatorBaseUrl,
      token: this.settings.sqlToolsToken,
    });
    const available = await client.probeCapabilities();
    if (available) {
      this.activeBackend = this.emulatorBackend;
      this.activeName = 'emulator';
      await this.emulatorBackend.initialize(this.settings);
      return;
    }

    this.activeBackend = this.bigQueryBackend;
    this.activeName = 'bigquery';
    await this.bigQueryBackend.initialize(this.settings);
  }
}

export async function formatWithFallback(
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
