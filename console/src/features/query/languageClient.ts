import type * as Monaco from 'monaco-editor';
import {
    BrowserMessageReader,
    BrowserMessageWriter,
} from 'vscode-languageserver-protocol/browser';
import {
    createProtocolConnection,
    type ProtocolConnection,
    type Diagnostic as LspDiagnostic,
    type CompletionItem,
    type Hover,
    type TextEdit,
    type MarkupContent,
} from 'vscode-languageserver-protocol';
import type { InitializeParams, InitializeResult } from 'vscode-languageserver-protocol';

export interface EditorDiagnostic {
    message: string;
    severity: 'error' | 'warning' | 'info' | 'hint';
    line: number;
    column: number;
}

export interface LanguageClientSettings {
    projectId?: string;
    defaultDatasetId?: string;
    emulatorBaseUrl?: string;
    useEmulatorParser?: boolean;
    sqlToolsAvailable?: boolean;
}

export interface GooglesqlLanguageSession {
    uri: string;
    connection: ProtocolConnection;
    dispose: () => Promise<void>;
    updateSettings: (settings: LanguageClientSettings) => Promise<void>;
    formatDocument: () => Promise<string | null>;
}

let worker: Worker | null = null;
let sharedConnection: ProtocolConnection | null = null;
let sharedInit: Promise<void> | null = null;
let documentVersion = 1;

function severityToMonaco(
    monaco: typeof Monaco,
    severity?: number,
): Monaco.MarkerSeverity {
    switch (severity) {
        case 1:
            return monaco.MarkerSeverity.Error;
        case 2:
            return monaco.MarkerSeverity.Warning;
        case 3:
            return monaco.MarkerSeverity.Info;
        default:
            return monaco.MarkerSeverity.Hint;
    }
}

function lspSeverityToEditor(severity?: number): EditorDiagnostic['severity'] {
    switch (severity) {
        case 1:
            return 'error';
        case 2:
            return 'warning';
        case 3:
            return 'info';
        default:
            return 'hint';
    }
}

function ensureWorkerConnection(): ProtocolConnection {
    if (sharedConnection) {
        return sharedConnection;
    }

    worker = new Worker(
        new URL('@bigquery-emulator/vscode-server/browser', import.meta.url),
        { type: 'module', name: 'bigquery-lsp' },
    );
    const reader = new BrowserMessageReader(worker);
    const writer = new BrowserMessageWriter(worker);
    sharedConnection = createProtocolConnection(reader, writer);
    sharedConnection.listen();
    return sharedConnection;
}

async function ensureInitialized(settings: LanguageClientSettings): Promise<ProtocolConnection> {
    const connection = ensureWorkerConnection();
    if (!sharedInit) {
        sharedInit = (async () => {
            const params: InitializeParams = {
                processId: null,
                rootUri: null,
                capabilities: {
                    textDocument: {
                        synchronization: { dynamicRegistration: false },
                        completion: { completionItem: { snippetSupport: true } },
                        hover: { contentFormat: ['markdown', 'plaintext'] },
                        formatting: { dynamicRegistration: false },
                        publishDiagnostics: { relatedInformation: false },
                        codeAction: {
                            codeActionLiteralSupport: {
                                codeActionKind: { valueSet: ['quickfix'] },
                            },
                        },
                    },
                },
                initializationOptions: {
                    backendMode: 'emulator',
                    emulatorBaseUrl: settings.emulatorBaseUrl ?? window.location.origin,
                    projectId: settings.projectId ?? 'local-project',
                    defaultDatasetId: settings.defaultDatasetId,
                    strictFormat: false,
                },
                workspaceFolders: null,
            };
            await connection.sendRequest('initialize', params) as InitializeResult;
            await connection.sendNotification('initialized', {});
        })();
    }
    await sharedInit;
    return connection;
}

function markupToString(contents: Hover['contents']): string {
    if (typeof contents === 'string') {
        return contents;
    }
    if (Array.isArray(contents)) {
        return contents
            .map((part) => (typeof part === 'string' ? part : part.value))
            .join('\n\n');
    }
    return (contents as MarkupContent).value;
}

export async function attachGooglesqlLanguageClient(options: {
    monaco: typeof Monaco;
    editor: Monaco.editor.IStandaloneCodeEditor;
    languageId: string;
    settings: LanguageClientSettings;
    onDiagnostics?: (diagnostics: EditorDiagnostic[]) => void;
}): Promise<GooglesqlLanguageSession> {
    const { monaco, editor, languageId, onDiagnostics } = options;
    let settings = options.settings;
    const model = editor.getModel();
    if (!model) {
        throw new Error('Monaco model missing');
    }

    const uri = model.uri.toString();
    const connection = await ensureInitialized(settings);

    const publishHandler = connection.onNotification(
        'textDocument/publishDiagnostics',
        (params: { uri: string; diagnostics: LspDiagnostic[] }) => {
            if (params.uri !== uri) {
                return;
            }
            const markers: Monaco.editor.IMarkerData[] = params.diagnostics.map((d) => ({
                severity: severityToMonaco(monaco, d.severity),
                message: typeof d.message === 'string' ? d.message : markupToString(d.message),
                startLineNumber: d.range.start.line + 1,
                startColumn: d.range.start.character + 1,
                endLineNumber: d.range.end.line + 1,
                endColumn: Math.max(d.range.end.character + 1, d.range.start.character + 2),
                source: d.source,
            }));
            monaco.editor.setModelMarkers(model, 'bigquery-lsp', markers);
            onDiagnostics?.(
                params.diagnostics.map((d) => ({
                    message: typeof d.message === 'string' ? d.message : markupToString(d.message),
                    severity: lspSeverityToEditor(d.severity),
                    line: d.range.start.line + 1,
                    column: d.range.start.character + 1,
                })),
            );
        },
    );

    await connection.sendNotification('textDocument/didOpen', {
        textDocument: {
            uri,
            languageId,
            version: documentVersion,
            text: model.getValue(),
        },
    });

    const changeSub = model.onDidChangeContent(() => {
        documentVersion += 1;
        void connection.sendNotification('textDocument/didChange', {
            textDocument: { uri, version: documentVersion },
            contentChanges: [{ text: model.getValue() }],
        });
    });

    const completionProvider = monaco.languages.registerCompletionItemProvider(languageId, {
        triggerCharacters: ['.', '`', '_', '('],
        provideCompletionItems: async (_model, position) => {
            if (!settings.useEmulatorParser) {
                return { suggestions: [] };
            }
            try {
                const result = (await connection.sendRequest('textDocument/completion', {
                    textDocument: { uri },
                    position: {
                        line: position.lineNumber - 1,
                        character: position.column - 1,
                    },
                })) as { items?: CompletionItem[] } | CompletionItem[] | null;

                const items = Array.isArray(result) ? result : (result?.items ?? []);
                return {
                    suggestions: items.map((item) => {
                        const insert =
                            typeof item.insertText === 'string'
                                ? item.insertText
                                : item.label;
                        const range = item.textEdit && 'range' in item.textEdit
                            ? {
                                  startLineNumber: item.textEdit.range.start.line + 1,
                                  startColumn: item.textEdit.range.start.character + 1,
                                  endLineNumber: item.textEdit.range.end.line + 1,
                                  endColumn: item.textEdit.range.end.character + 1,
                              }
                            : {
                                  startLineNumber: position.lineNumber,
                                  startColumn: position.column,
                                  endLineNumber: position.lineNumber,
                                  endColumn: position.column,
                              };
                        return {
                            label: item.label,
                            kind: monaco.languages.CompletionItemKind.Function,
                            insertText: insert,
                            detail: item.detail,
                            range,
                        };
                    }),
                };
            } catch {
                return { suggestions: [] };
            }
        },
    });

    const hoverProvider = monaco.languages.registerHoverProvider(languageId, {
        provideHover: async (m, position) => {
            const markers = monaco.editor.getModelMarkers({ resource: m.uri });
            const atPos = markers.filter(
                (marker) =>
                    marker.startLineNumber <= position.lineNumber &&
                    marker.endLineNumber >= position.lineNumber &&
                    marker.startColumn <= position.column &&
                    marker.endColumn >= position.column,
            );
            if (atPos.length > 0) {
                const primary = atPos[0]!;
                return {
                    range: new monaco.Range(
                        primary.startLineNumber,
                        primary.startColumn,
                        primary.endLineNumber,
                        primary.endColumn,
                    ),
                    contents: [
                        { value: primary.message },
                        {
                            value: 'View Problem (Alt+F8)\n\nNo quick fixes available',
                        },
                    ],
                };
            }

            try {
                const hover = (await connection.sendRequest('textDocument/hover', {
                    textDocument: { uri },
                    position: {
                        line: position.lineNumber - 1,
                        character: position.column - 1,
                    },
                })) as Hover | null;
                if (!hover) {
                    return null;
                }
                return {
                    contents: [{ value: markupToString(hover.contents) }],
                };
            } catch {
                return null;
            }
        },
    });

    const formattingProvider = monaco.languages.registerDocumentFormattingEditProvider(languageId, {
        provideDocumentFormattingEdits: async (_model) => {
            try {
                const edits = (await connection.sendRequest('textDocument/formatting', {
                    textDocument: { uri },
                    options: { tabSize: 2, insertSpaces: true },
                })) as TextEdit[] | null;
                if (!edits?.length) {
                    return [];
                }
                return edits.map((edit) => ({
                    range: new monaco.Range(
                        edit.range.start.line + 1,
                        edit.range.start.character + 1,
                        edit.range.end.line + 1,
                        edit.range.end.character + 1,
                    ),
                    text: edit.newText,
                }));
            } catch {
                return [];
            }
        },
    });

    // Match VS Code / BigQuery Studio: Alt+F8 opens / jumps to next problem.
    editor.addCommand(monaco.KeyMod.Alt | monaco.KeyCode.F8, () => {
        void editor.getAction('editor.action.marker.next')?.run();
    });

    return {
        uri,
        connection,
        updateSettings: async (next) => {
            settings = next;
            await connection.sendNotification('workspace/didChangeConfiguration', {
                settings: {
                    backendMode: 'emulator',
                    emulatorBaseUrl: next.emulatorBaseUrl ?? window.location.origin,
                    projectId: next.projectId ?? 'local-project',
                    defaultDatasetId: next.defaultDatasetId,
                    strictFormat: false,
                },
            });
        },
        formatDocument: async () => {
            const action = editor.getAction('editor.action.formatDocument');
            if (action) {
                await action.run();
                return editor.getValue();
            }
            return null;
        },
        dispose: async () => {
            changeSub.dispose();
            completionProvider.dispose();
            hoverProvider.dispose();
            formattingProvider.dispose();
            publishHandler.dispose();
            monaco.editor.setModelMarkers(model, 'bigquery-lsp', []);
            onDiagnostics?.([]);
            try {
                await connection.sendNotification('textDocument/didClose', {
                    textDocument: { uri },
                });
            } catch {
                /* ignore */
            }
        },
    };
}
