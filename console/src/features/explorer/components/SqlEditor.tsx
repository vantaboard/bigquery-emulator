import Editor, { loader, type OnMount } from '@monaco-editor/react';
import { useEffect, useMemo, useRef } from 'react';

import {
    attachGooglesqlLanguageClient,
    type EditorDiagnostic,
    type GooglesqlLanguageSession,
} from '@/features/query/languageClient';
import { monaco } from '@/lib/monacoSetup';
import { cn } from '@/lib/utils';

loader.config({ monaco });

const LANGUAGE_ID = 'googlesql';

let languageRegistered = false;

function ensureGooglesqlLanguage(): void {
    if (languageRegistered) {
        return;
    }
    monaco.languages.register({ id: LANGUAGE_ID, extensions: ['.sql', '.bqsql'] });
    monaco.languages.setMonarchTokensProvider(LANGUAGE_ID, {
        defaultToken: '',
        ignoreCase: true,
        tokenizer: {
            root: [
                [/--.*$/, 'comment'],
                [/\/\*/, 'comment', '@comment'],
                [/"[^"]*"/, 'string'],
                [/'[^']*'/, 'string'],
                [/`[^`]*`/, 'identifier'],
                [/\b\d+(\.\d+)?\b/, 'number'],
                [
                    /\b(SELECT|FROM|WHERE|AND|OR|NOT|JOIN|LEFT|RIGHT|INNER|OUTER|ON|GROUP|BY|ORDER|LIMIT|AS|WITH|INSERT|UPDATE|DELETE|CREATE|REPLACE|TABLE|VIEW|FUNCTION|PROCEDURE|RETURNS|BEGIN|END|IF|THEN|ELSE|CASE|WHEN|NULL|TRUE|FALSE|DISTINCT|UNION|ALL|HAVING|PARTITION|OVER|WINDOW|SAFE_ADD|SAFE_CAST)\b/,
                    'keyword',
                ],
            ],
            comment: [
                [/[^/*]+/, 'comment'],
                [/\*\//, 'comment', '@pop'],
                [/./, 'comment'],
            ],
        },
    });
    languageRegistered = true;
}

export type { EditorDiagnostic };

interface SqlEditorProps {
    value: string;
    onChange: (v: string) => void;
    className?: string;
    readOnly?: boolean;
    projectId?: string;
    defaultDatasetId?: string;
    useEmulatorParser?: boolean;
    sqlToolsAvailable?: boolean;
    onDiagnostics?: (diagnostics: EditorDiagnostic[]) => void;
    onEditorReady?: (editor: monaco.editor.IStandaloneCodeEditor) => void;
}

export function SqlEditor({
    value,
    onChange,
    className,
    readOnly,
    projectId,
    defaultDatasetId,
    useEmulatorParser = true,
    sqlToolsAvailable = false,
    onDiagnostics,
    onEditorReady,
}: SqlEditorProps) {
    const editorRef = useRef<monaco.editor.IStandaloneCodeEditor | null>(null);
    const sessionRef = useRef<GooglesqlLanguageSession | null>(null);
    const onDiagnosticsRef = useRef(onDiagnostics);
    onDiagnosticsRef.current = onDiagnostics;
    const onEditorReadyRef = useRef(onEditorReady);
    onEditorReadyRef.current = onEditorReady;

    const settings = useMemo(
        () => ({
            projectId,
            defaultDatasetId,
            emulatorBaseUrl: window.location.origin,
            useEmulatorParser,
            sqlToolsAvailable,
        }),
        [projectId, defaultDatasetId, useEmulatorParser, sqlToolsAvailable],
    );
    const settingsRef = useRef(settings);
    settingsRef.current = settings;

    useEffect(() => {
        void sessionRef.current?.updateSettings(settings);
    }, [settings]);

    // Apply parent-driven SQL updates (format API, tab restore) without using the
    // controlled `value` prop, which fights Monaco on every keystroke and corrupts
    // LSP document sync.
    useEffect(() => {
        const editor = editorRef.current;
        if (!editor) {
            return;
        }
        const model = editor.getModel();
        if (!model || model.getValue() === value) {
            return;
        }
        editor.setValue(value);
    }, [value]);

    useEffect(() => {
        return () => {
            void sessionRef.current?.dispose();
            sessionRef.current = null;
        };
    }, []);

    const handleMount: OnMount = (editor) => {
        editorRef.current = editor;
        ensureGooglesqlLanguage();
        onEditorReadyRef.current?.(editor);

        void attachGooglesqlLanguageClient({
            monaco,
            editor,
            languageId: LANGUAGE_ID,
            settings,
            onDiagnostics: (diagnostics) => onDiagnosticsRef.current?.(diagnostics),
        })
            .then((session) => {
                sessionRef.current = session;
                // Probe may finish after mount; apply the latest settings once the
                // LSP session exists (the mount-time useEffect often runs too early).
                void session.updateSettings(settingsRef.current);
            })
            .catch((err: unknown) => {
                console.error('Failed to attach BigQuery language client', err);
            });
    };

    return (
        <div
            data-testid="sql-editor"
            className={cn('overflow-hidden rounded-md border border-[var(--bq-border)]', className)}
        >
            <Editor
                height="220px"
                language={LANGUAGE_ID}
                theme="vs-dark"
                defaultValue={value}
                onChange={(next) => onChange(next ?? '')}
                onMount={handleMount}
                options={{
                    readOnly: Boolean(readOnly),
                    minimap: { enabled: false },
                    fontSize: 13,
                    lineNumbers: 'on',
                    glyphMargin: true,
                    scrollBeyondLastLine: false,
                    automaticLayout: true,
                    wordWrap: 'on',
                    tabSize: 2,
                    folding: false,
                    renderValidationDecorations: 'on',
                    hover: { enabled: true, sticky: true, delay: 100 },
                    lightbulb: { enabled: 'on' },
                    quickSuggestions: { other: true, comments: false, strings: false },
                    suggestOnTriggerCharacters: true,
                    wordBasedSuggestions: 'off',
                }}
            />
        </div>
    );
}
