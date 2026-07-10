import {
  createConnection,
  TextDocuments,
  ProposedFeatures,
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
  diagnosticSeverity,
  formatDiagnosticMessage,
  lookupFunctionDoc,
} from '@bigquery-emulator/vscode-shared';

import { BackendManager, formatWithFallback } from './backends/manager.js';
import { buildQuickFixes } from './quickfixes.js';
import { defaultSettings, settingsFromInit, toConnectionSettings } from './settings.js';

const PARSE_DEBOUNCE_MS = 400;

const connection = createConnection(ProposedFeatures.all);
const documents = new TextDocuments(TextDocument);

let currentSettings = defaultSettings;
let backendManager = new BackendManager(toConnectionSettings(currentSettings));
const diagnosticTimers = new Map<string, ReturnType<typeof setTimeout>>();
const diagnosticVersions = new Map<string, number>();

connection.onInitialize(async (params) => {
  currentSettings = settingsFromInit(params.initializationOptions);
  backendManager = new BackendManager(toConnectionSettings(currentSettings));
  await backendManager.initialize();

  return {
    capabilities: {
      textDocumentSync: TextDocumentSyncKind.Incremental,
      completionProvider: {
        resolveProvider: false,
        triggerCharacters: ['.', '`', '_'],
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
  const formatted = await formatWithFallback(
    backendManager.getBackend(),
    document.getText(),
    settings,
  );

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

function diagnosticRangeForDocument(
  document: TextDocument,
  diagnostic: import('@bigquery-emulator/vscode-shared').SqlDiagnostic,
) {
  if (diagnostic.startUtf16 !== undefined && diagnostic.endUtf16 !== undefined) {
    return {
      start: document.positionAt(Math.max(0, diagnostic.startUtf16)),
      end: document.positionAt(Math.max(diagnostic.startUtf16, diagnostic.endUtf16)),
    };
  }

  const fromLine = Math.min(Math.max(1, diagnostic.line), document.lineCount);
  const fromCharacter = Math.max(0, diagnostic.column - 1);
  if (diagnostic.endLine !== undefined && diagnostic.endColumn !== undefined) {
    const toLine = Math.min(Math.max(1, diagnostic.endLine), document.lineCount);
    return {
      start: { line: fromLine - 1, character: fromCharacter },
      end: {
        line: toLine - 1,
        character: Math.max(0, diagnostic.endColumn - 1),
      },
    };
  }

  return {
    start: { line: fromLine - 1, character: fromCharacter },
    end: { line: fromLine - 1, character: fromCharacter + 1 },
  };
}

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

  const diagnostics: Diagnostic[] = (rawDiagnostics ?? []).map((item) => ({
    range: diagnosticRangeForDocument(document, item),
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
