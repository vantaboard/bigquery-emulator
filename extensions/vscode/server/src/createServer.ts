import {
  type Connection,
  TextDocuments,
  TextDocumentSyncKind,
  CompletionItem,
  CompletionItemKind,
  CompletionList,
  Diagnostic,
  DiagnosticSeverity,
  Hover,
  MarkupContent,
  CodeAction,
  TextEdit,
} from 'vscode-languageserver/node.js';
import { TextDocument } from 'vscode-languageserver-textdocument';

import {
  completionKindToLsp,
  diagnosticRange,
  diagnosticSeverity,
  formatDiagnosticMessage,
  lookupFunctionDoc,
  type ConnectionSettings,
} from '@bigquery-emulator/vscode-shared';

import type { LanguageBackend } from './backends/types.js';
import { buildQuickFixes } from './quickfixes.js';
import { defaultSettings, settingsFromInit, toConnectionSettings } from './settings.js';

const PARSE_DEBOUNCE_MS = 400;

export interface BackendManagerLike {
  initialize(): Promise<void>;
  updateSettings(settings: ConnectionSettings): Promise<void>;
  getActiveName(): string;
  getBackend(): LanguageBackend;
}

export interface CreateServerOptions {
  createBackendManager: (settings: ConnectionSettings) => BackendManagerLike;
  formatDocument: (
    backend: LanguageBackend,
    sql: string,
    settings: ConnectionSettings,
  ) => Promise<string>;
}

export function createServer(connection: Connection, options: CreateServerOptions): void {
  const documents = new TextDocuments(TextDocument);
  const { createBackendManager, formatDocument: formatFn } = options;

  let currentSettings = defaultSettings;
  let backendManager: BackendManagerLike = createBackendManager(toConnectionSettings(currentSettings));
  const diagnosticTimers = new Map<string, ReturnType<typeof setTimeout>>();
  const diagnosticVersions = new Map<string, number>();

  connection.onInitialize(async (params) => {
    currentSettings = settingsFromInit(params.initializationOptions);
    backendManager = createBackendManager(toConnectionSettings(currentSettings));
    await backendManager.initialize();

    return {
      capabilities: {
        textDocumentSync: TextDocumentSyncKind.Incremental,
        completionProvider: {
          resolveProvider: false,
          triggerCharacters: ['.', '`', '_', '('],
        },
        documentFormattingProvider: true,
        codeActionProvider: {
          codeActionKinds: ['quickfix'],
        },
        hoverProvider: true,
      },
    };
  });

  connection.onDidChangeConfiguration(async (change) => {
    currentSettings = settingsFromInit(change.settings);
    await backendManager.updateSettings(toConnectionSettings(currentSettings));
    for (const document of documents.all()) {
      void publishDiagnostics(document);
    }
  });

  documents.onDidChangeContent((event) => {
    const existing = diagnosticTimers.get(event.document.uri);
    if (existing) {
      clearTimeout(existing);
    }

    const version = (diagnosticVersions.get(event.document.uri) ?? 0) + 1;
    diagnosticVersions.set(event.document.uri, version);

    const timer = setTimeout(() => {
      void publishDiagnostics(event.document, version);
    }, PARSE_DEBOUNCE_MS);
    diagnosticTimers.set(event.document.uri, timer);
  });

  documents.onDidClose((event) => {
    const timer = diagnosticTimers.get(event.document.uri);
    if (timer) {
      clearTimeout(timer);
      diagnosticTimers.delete(event.document.uri);
    }
    diagnosticVersions.delete(event.document.uri);
    connection.sendDiagnostics({ uri: event.document.uri, diagnostics: [] });
  });

  connection.onCompletion(async (params) => {
    const document = documents.get(params.textDocument.uri);
    if (!document) {
      return [];
    }

    const backend = backendManager.getBackend();
    const position = params.position;
    const offset = document.offsetAt(position);
    const result = await backend.getCompletions(document.getText(), offset, {
      settings: toConnectionSettings(currentSettings),
      document,
    });

    if (!result || result.candidates.length === 0) {
      return [];
    }

    const items: CompletionItem[] = result.candidates.map((candidate) => ({
      label: candidate.label,
      kind: completionKindToLsp(candidate.kind) as CompletionItemKind,
      detail: candidate.detail ?? candidate.kind,
      insertText: candidate.insertText,
      textEdit: {
        range: {
          start: document.positionAt(result.replacementStart),
          end: document.positionAt(result.replacementEnd),
        },
        newText: candidate.insertText,
      },
    }));

    return CompletionList.create(items, false);
  });

  connection.onDocumentFormatting(async (params) => {
    const document = documents.get(params.textDocument.uri);
    if (!document) {
      return [];
    }

    const settings = toConnectionSettings(currentSettings);
    const formatted = await formatFn(backendManager.getBackend(), document.getText(), settings);

    const fullRange = {
      start: document.positionAt(0),
      end: document.positionAt(document.getText().length),
    };

    return [TextEdit.replace(fullRange, formatted)];
  });

  connection.onCodeAction(async (params) => {
    const document = documents.get(params.textDocument.uri);
    if (!document) {
      return [];
    }

    const catalog = await backendManager.getBackend().getCatalog(currentSettings.projectId);
    return buildQuickFixes(document, params.context.diagnostics, catalog) as CodeAction[];
  });

  connection.onHover(async (params) => {
    const document = documents.get(params.textDocument.uri);
    if (!document) {
      return null;
    }

    const wordRange = getWordRangeAtPosition(document, params.position);
    if (!wordRange) {
      return null;
    }

    const word = document.getText(wordRange).replace(/[`"]/g, '');
    const functionDoc = lookupFunctionDoc(word);
    if (functionDoc) {
      return hoverMarkdown(
        `**${functionDoc.name}**\n\n\`${functionDoc.signature}\`\n\n${functionDoc.description}`,
      );
    }

    const backend = backendManager.getBackend();
    const analyze = await backend.analyze(document.getText(), {
      settings: toConnectionSettings(currentSettings),
      document,
    });

    if (analyze?.referencedTables?.length) {
      for (const table of analyze.referencedTables) {
        const fqn = `${table.projectId}.${table.datasetId}.${table.tableId}`;
        if (word === table.tableId || word === fqn || word === `${table.datasetId}.${table.tableId}`) {
          const metadata = await backend.getTableMetadata(
            table.projectId,
            table.datasetId,
            table.tableId,
          );
          if (metadata?.schema?.fields?.length) {
            const lines = metadata.schema.fields.map(
              (field) => `- \`${field.name}\` ${field.type}${field.mode ? ` (${field.mode})` : ''}`,
            );
            return hoverMarkdown(`**${fqn}**\n\n${lines.join('\n')}`);
          }
          return hoverMarkdown(`**${fqn}** (${table.kind})`);
        }
      }
    }

    return null;
  });

  async function publishDiagnostics(document: TextDocument, expectedVersion?: number): Promise<void> {
    if (expectedVersion !== undefined) {
      const current = diagnosticVersions.get(document.uri);
      if (current !== expectedVersion) {
        return;
      }
    }

    const backend = backendManager.getBackend();
    const rawDiagnostics = await backend.getDiagnostics(document.getText(), {
      settings: toConnectionSettings(currentSettings),
      document,
    });

    const diagnostics: Diagnostic[] = rawDiagnostics.map((item) => ({
      range: diagnosticRange(document, item),
      severity: diagnosticSeverity(item.severity) as DiagnosticSeverity,
      message: formatDiagnosticMessage(item),
      source: backendManager.getActiveName(),
    }));

    connection.sendDiagnostics({ uri: document.uri, diagnostics });
  }

  function getWordRangeAtPosition(
    document: TextDocument,
    position: { line: number; character: number },
  ) {
    const line = document.getText({
      start: { line: position.line, character: 0 },
      end: { line: position.line, character: Number.MAX_SAFE_INTEGER },
    });
    const before = line.slice(0, position.character);
    const after = line.slice(position.character);
    const prefix = before.match(/[`\w.]*$/)?.[0] ?? '';
    const suffix = after.match(/^[`\w.]*/)?.[0] ?? '';
    const word = `${prefix}${suffix}`;
    if (!word) {
      return null;
    }
    const startCharacter = position.character - prefix.length;
    const endCharacter = startCharacter + word.length;
    return {
      start: { line: position.line, character: startCharacter },
      end: { line: position.line, character: endCharacter },
    };
  }

  function hoverMarkdown(value: string): Hover {
    const contents: MarkupContent = {
      kind: 'markdown',
      value,
    };
    return { contents };
  }

  documents.listen(connection);
  connection.listen();
}
