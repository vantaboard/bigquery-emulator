import * as path from 'node:path';

import * as vscode from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
} from 'vscode-languageclient/node';

let client: LanguageClient | undefined;
let statusBarItem: vscode.StatusBarItem | undefined;

export async function activate(context: vscode.ExtensionContext): Promise<void> {
  statusBarItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
  statusBarItem.command = 'bigquery.probeBackend';
  context.subscriptions.push(statusBarItem);

  await startClient(context);

  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration(async (event) => {
      if (
        event.affectsConfiguration('bigquery.backendMode') ||
        event.affectsConfiguration('bigquery.emulatorBaseUrl') ||
        event.affectsConfiguration('bigquery.sqlToolsToken') ||
        event.affectsConfiguration('bigquery.projectId') ||
        event.affectsConfiguration('bigquery.defaultDatasetId') ||
        event.affectsConfiguration('bigquery.strictFormat') ||
        event.affectsConfiguration('bigquery.formatIndentationSpaces') ||
        event.affectsConfiguration('bigquery.formatLineLengthLimit')
      ) {
        await restartClient(context);
      }
    }),
    vscode.commands.registerCommand('bigquery.restartLanguageServer', async () => {
      await restartClient(context);
      void vscode.window.showInformationMessage('BigQuery language server restarted.');
    }),
    vscode.commands.registerCommand('bigquery.probeBackend', async () => {
      const settings = readSettings();
      void vscode.window.showInformationMessage(
        `BigQuery backend mode: ${settings.backendMode} (project: ${settings.projectId})`,
      );
      updateStatusBar(settings.backendMode);
    }),
  );

  updateStatusBar(readSettings().backendMode);
  statusBarItem.show();
}

export async function deactivate(): Promise<void> {
  if (!client) {
    return;
  }
  await client.stop();
}

async function startClient(context: vscode.ExtensionContext): Promise<void> {
  const serverModule = context.asAbsolutePath(path.join('server', 'dist', 'server.js'));
  const serverOptions: ServerOptions = {
    run: { module: serverModule, transport: TransportKind.stdio },
    debug: {
      module: serverModule,
      transport: TransportKind.stdio,
      options: { execArgv: ['--nolazy', '--inspect=6010'] },
    },
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ language: 'bigquery' }],
    synchronize: {
      configurationSection: 'bigquery',
    },
    initializationOptions: readSettings(),
  };

  client = new LanguageClient('bigqueryLanguageServer', 'BigQuery Language Server', serverOptions, clientOptions);
  await client.start();
}

async function restartClient(context: vscode.ExtensionContext): Promise<void> {
  if (client) {
    await client.stop();
    client = undefined;
  }
  await startClient(context);
  updateStatusBar(readSettings().backendMode);
}

function readSettings() {
  const config = vscode.workspace.getConfiguration('bigquery');
  return {
    backendMode: config.get<'auto' | 'emulator' | 'bigquery'>('backendMode', 'auto'),
    emulatorBaseUrl: config.get<string>('emulatorBaseUrl', 'http://127.0.0.1:9050'),
    sqlToolsToken: config.get<string>('sqlToolsToken', '') || undefined,
    projectId: config.get<string>('projectId', 'local-project'),
    defaultDatasetId: config.get<string>('defaultDatasetId', '') || undefined,
    strictFormat: config.get<boolean>('strictFormat', false),
    formatIndentationSpaces: config.get<number>('formatIndentationSpaces', 2),
    formatLineLengthLimit: config.get<number>('formatLineLengthLimit', 80),
  };
}

function updateStatusBar(backendMode: string): void {
  if (!statusBarItem) {
    return;
  }
  statusBarItem.text = `$(database) BigQuery: ${backendMode}`;
  statusBarItem.tooltip = 'BigQuery language service backend mode';
}
