import type { ConnectionSettings } from '@bigquery-emulator/vscode-shared';

export interface ServerSettings {
  backendMode: ConnectionSettings['backendMode'];
  emulatorBaseUrl: string;
  sqlToolsToken?: string;
  projectId: string;
  defaultDatasetId?: string;
  strictFormat: boolean;
  formatIndentationSpaces: number;
  formatLineLengthLimit: number;
}

export const defaultSettings: ServerSettings = {
  backendMode: 'auto',
  emulatorBaseUrl: 'http://127.0.0.1:9050',
  projectId: 'local-project',
  strictFormat: false,
  formatIndentationSpaces: 2,
  formatLineLengthLimit: 80,
};

export function settingsFromInit(args: unknown): ServerSettings {
  if (!args || typeof args !== 'object') {
    return defaultSettings;
  }

  const record = args as { bigquery?: Partial<ServerSettings> };
  const input = record.bigquery ?? (args as Partial<ServerSettings>);
  return {
    backendMode: input.backendMode ?? defaultSettings.backendMode,
    emulatorBaseUrl: input.emulatorBaseUrl ?? defaultSettings.emulatorBaseUrl,
    sqlToolsToken: input.sqlToolsToken,
    projectId: input.projectId ?? defaultSettings.projectId,
    defaultDatasetId: input.defaultDatasetId,
    strictFormat: input.strictFormat ?? defaultSettings.strictFormat,
    formatIndentationSpaces:
      input.formatIndentationSpaces ?? defaultSettings.formatIndentationSpaces,
    formatLineLengthLimit:
      input.formatLineLengthLimit ?? defaultSettings.formatLineLengthLimit,
  };
}

export function toConnectionSettings(settings: ServerSettings): ConnectionSettings {
  return {
    backendMode: settings.backendMode,
    emulatorBaseUrl: settings.emulatorBaseUrl,
    sqlToolsToken: settings.sqlToolsToken,
    projectId: settings.projectId,
    defaultDatasetId: settings.defaultDatasetId,
    strictFormat: settings.strictFormat,
    formatIndentationSpaces: settings.formatIndentationSpaces,
    formatLineLengthLimit: settings.formatLineLengthLimit,
  };
}
