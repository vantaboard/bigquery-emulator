import { autocompletion, type Completion, type CompletionContext } from '@codemirror/autocomplete';
import {
    lintKeymap,
    linter,
    openLintPanel,
    previousDiagnostic,
    type Diagnostic,
} from '@codemirror/lint';
import { sql } from '@codemirror/lang-sql';
import type { Extension } from '@codemirror/state';
import { EditorView, keymap } from '@codemirror/view';

import {
    completeSql,
    completionKindToType,
    parseSql,
    type SqlDiagnostic,
} from '@/lib/sqlTools';

import type { SqlCatalog } from './sqlCatalog';

const COMPLETE_DEBOUNCE_MS = 150;
const PARSE_DEBOUNCE_MS = 400;
const POSITION_SUFFIX_RE = /\s+at\s+\[\d+:\d+\]\s*$/;

export interface EditorDiagnostic {
    message: string;
    severity: Diagnostic['severity'];
    line: number;
    column: number;
}

export function formatDiagnosticMessage(d: Pick<SqlDiagnostic, 'message' | 'line' | 'column'>): string {
    const message = d.message.replace(/\bend of statement\b/gi, 'end of script');

    if (POSITION_SUFFIX_RE.test(message)) {
        return message;
    }

    return `${message} at [${d.line}:${d.column}]`;
}

export function createDiagnosticRenderMessage(message: string): (view: EditorView) => Node {
    return (view) => {
        const wrapper = document.createElement('div');
        wrapper.className = 'cm-bq-diagnosticMessage';

        const messageEl = document.createElement('div');
        messageEl.className = 'cm-bq-diagnosticMessage-text';
        messageEl.textContent = message;

        const footer = document.createElement('div');
        footer.className = 'cm-bq-diagnosticMessage-footer';

        const viewProblem = document.createElement('button');
        viewProblem.type = 'button';
        viewProblem.className = 'cm-bq-diagnosticMessage-viewProblem';
        viewProblem.textContent = 'View Problem (Alt+F8)';
        viewProblem.addEventListener('mousedown', (event) => {
            event.preventDefault();
        });
        viewProblem.addEventListener('click', (event) => {
            event.preventDefault();
            openLintPanel(view);
        });

        const noFixes = document.createElement('span');
        noFixes.className = 'cm-bq-diagnosticMessage-noFixes';
        noFixes.textContent = 'No quick fixes available';

        footer.append(viewProblem, noFixes);
        wrapper.append(messageEl, footer);
        return wrapper;
    };
}

function diagnosticSeverity(severity: string): Diagnostic['severity'] {
    if (severity === 'error') return 'error';
    if (severity === 'warning') return 'warning';
    return 'info';
}

function diagnosticRange(doc: EditorView['state']['doc'], d: SqlDiagnostic): { from: number; to: number } {
    if (d.startUtf16 !== undefined && d.endUtf16 !== undefined) {
        return {
            from: Math.max(0, d.startUtf16),
            to: Math.max(d.startUtf16 ?? 0, d.endUtf16),
        };
    }

    const fromLine = doc.line(Math.min(Math.max(1, d.line), doc.lines));
    const from = fromLine.from + Math.max(0, d.column - 1);

    if (d.endLine !== undefined && d.endColumn !== undefined) {
        const toLine = doc.line(Math.min(Math.max(1, d.endLine), doc.lines));
        const to = toLine.from + Math.max(0, d.endColumn - 1);
        return { from, to: Math.max(from, to) };
    }

    return { from, to: from + 1 };
}

function catalogCompletions(catalog: SqlCatalog, context: CompletionContext): Completion[] | null {
    const word = context.matchBefore(/[\w`.]*$/);
    if (!word || (word.from === word.to && !context.explicit)) return null;

    const prefix = word.text.toLowerCase();
    const options: Completion[] = [];

    for (const [table, columns] of Object.entries(catalog.schema)) {
        if (table.toLowerCase().includes(prefix) || prefix === '') {
            options.push({
                label: table,
                type: 'class',
                detail: 'table',
            });
        }
        for (const col of columns) {
            if (col.toLowerCase().startsWith(prefix) || prefix.endsWith('.')) {
                options.push({
                    label: col,
                    type: 'property',
                    detail: table,
                });
            }
        }
    }

    for (const q of catalog.qualifiedTables) {
        if (q.toLowerCase().includes(prefix)) {
            options.push({ label: q, type: 'class', detail: 'table' });
        }
    }

    for (const routine of catalog.routines) {
        if (routine.toLowerCase().includes(prefix)) {
            options.push({ label: routine, type: 'method', detail: 'routine' });
        }
    }

    if (options.length === 0) return null;
    return options;
}

function mergeCatalogRoutineCompletions(
    options: Completion[],
    catalog: SqlCatalog,
    prefix: string,
): Completion[] {
    const labels = new Set(options.map((option) => option.label));
    const normalized = prefix.toLowerCase();
    const extras = catalog.routines
        .filter((routine) => !labels.has(routine) && routine.toLowerCase().includes(normalized))
        .map(
            (routine): Completion => ({
                label: routine,
                type: 'method',
                detail: 'routine',
            }),
        );
    return extras.length ? [...options, ...extras] : options;
}

function createDebouncedComplete(
    opts: SqlEditorExtensionOptions,
    generationRef: { current: number },
) {
    return async (context: CompletionContext) => {
        const sqlText = context.state.doc.toString();
        const pos = context.pos;
        const useSqlTools = opts.useEmulatorParser && opts.sqlToolsAvailable;
        const catalog = opts.catalog ?? { schema: {}, qualifiedTables: [], routines: [] };

        const gen = ++generationRef.current;
        await new Promise((resolve) => setTimeout(resolve, COMPLETE_DEBOUNCE_MS));
        if (gen !== generationRef.current) return null;

        if (useSqlTools) {
            try {
                const result = await completeSql({
                    sql: sqlText,
                    cursorByteOffset: pos,
                    projectId: opts.projectId,
                    defaultDatasetId: opts.defaultDatasetId,
                    offsetUnit: 'utf16',
                });

                if (gen !== generationRef.current) return null;
                if (result.candidates.length === 0) {
                    const fallback = catalogCompletions(catalog, context);
                    if (!fallback?.length) return null;
                    const word = context.matchBefore(/[\w`.]*$/);
                    if (!word) return null;
                    return { from: word.from, to: word.to, options: fallback };
                }

                const word = context.matchBefore(/[\w`.]*$/);
                const prefix = word?.text ?? '';
                const mergedOptions = mergeCatalogRoutineCompletions(
                    result.candidates.map((c) => ({
                        label: c.label,
                        type: completionKindToType(c.kind),
                        detail: c.detail ?? c.kind,
                        apply: c.insertText,
                    })),
                    catalog,
                    prefix,
                );

                return {
                    from: result.replacementStart,
                    to: result.replacementEnd,
                    options: mergedOptions,
                };
            } catch {
                /* fall through to catalog */
            }
        }

        const options = catalogCompletions(catalog, context);
        if (!options?.length) return null;
        const word = context.matchBefore(/[\w`.]*$/);
        if (!word) return null;
        return { from: word.from, to: word.to, options };
    };
}

export interface SqlEditorExtensionOptions {
    projectId?: string;
    defaultDatasetId?: string;
    useEmulatorParser: boolean;
    sqlToolsAvailable: boolean;
    catalog: SqlCatalog | null;
    onDiagnostics?: (diagnostics: EditorDiagnostic[]) => void;
}

const lintPanelTheme = EditorView.theme({
    '.cm-tooltip-lint': {
        backgroundColor: 'var(--bq-surface, #1e1e1e)',
        border: '1px solid #5c2b29',
        borderRadius: '2px',
        padding: 0,
        maxWidth: '36rem',
    },
    '.cm-tooltip-lint .cm-diagnostic': {
        alignItems: 'stretch',
        padding: 0,
    },
    '.cm-tooltip-lint .cm-diagnostic-error::before': {
        display: 'none',
    },
    '.cm-bq-diagnosticMessage-text': {
        padding: '0.5rem 0.75rem',
        color: '#e8eaed',
        fontSize: '0.8125rem',
        lineHeight: 1.4,
        borderLeft: '3px solid #f28b82',
    },
    '.cm-bq-diagnosticMessage-footer': {
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        gap: '1rem',
        borderTop: '1px solid #3c4043',
        padding: '0.35rem 0.75rem',
        backgroundColor: '#2d2e30',
    },
    '.cm-bq-diagnosticMessage-viewProblem': {
        border: 'none',
        background: 'transparent',
        color: '#8ab4f8',
        cursor: 'pointer',
        font: 'inherit',
        fontSize: '0.75rem',
        padding: 0,
        textAlign: 'left',
    },
    '.cm-bq-diagnosticMessage-viewProblem:hover': {
        textDecoration: 'underline',
    },
    '.cm-bq-diagnosticMessage-noFixes': {
        color: '#9aa0a6',
        fontSize: '0.75rem',
        whiteSpace: 'nowrap',
    },
    '.cm-panel-lint .cm-bq-diagnosticMessage-footer': {
        display: 'none',
    },
    '.cm-panel-lint': {
        backgroundColor: 'var(--bq-surface, #1e1e1e)',
        borderTop: '1px solid #5c2b29',
        color: '#f28b82',
        maxHeight: '8rem',
        overflow: 'auto',
    },
    '.cm-panel-lint ul': {
        margin: 0,
        padding: '0.25rem 0',
        listStyle: 'none',
    },
    '.cm-panel-lint ul:focus': {
        outline: 'none',
    },
    '.cm-panel-lint [name="close"]': {
        position: 'absolute',
        top: '0.25rem',
        right: '0.5rem',
        border: 'none',
        background: 'transparent',
        color: 'inherit',
        cursor: 'pointer',
        fontSize: '1.1rem',
        lineHeight: 1,
    },
    '.cm-panel-lint .cm-diagnostic': {
        alignItems: 'flex-start',
        padding: '0.35rem 0.75rem',
        gap: '0.5rem',
    },
    '.cm-panel-lint .cm-diagnosticText': {
        color: '#f28b82',
        fontSize: '0.8125rem',
        lineHeight: 1.4,
    },
    '.cm-panel-lint .cm-diagnostic-error::before': {
        content: '"✕"',
        color: '#f28b82',
        fontWeight: 700,
    },
});

const diagnosticNavigationKeymap = keymap.of([
    { key: 'Alt-F8', run: openLintPanel },
    { key: 'Shift-F8', run: previousDiagnostic },
    ...lintKeymap,
]);

export function buildSqlEditorExtensions(opts: SqlEditorExtensionOptions): Extension[] {
    const useSqlTools = opts.useEmulatorParser && opts.sqlToolsAvailable;
    const catalog = opts.catalog ?? { schema: {}, qualifiedTables: [], routines: [] };
    const completeGeneration = { current: 0 };
    let parseGeneration = 0;

    const completionExt = autocompletion({
        activateOnTyping: true,
        override: [createDebouncedComplete(opts, completeGeneration)],
    });

    const lintExt = linter(
        async (view) => {
            const sqlText = view.state.doc.toString();
            if (!sqlText.trim() || !useSqlTools) {
                opts.onDiagnostics?.([]);
                return [];
            }

            const gen = ++parseGeneration;
            await new Promise((resolve) => setTimeout(resolve, PARSE_DEBOUNCE_MS));
            if (gen !== parseGeneration) return [];

            try {
                const result = await parseSql({ sql: sqlText, offsetUnit: 'utf16' });
                if (gen !== parseGeneration) return [];

                const editorDiagnostics: EditorDiagnostic[] = result.diagnostics.map((d) => ({
                    message: formatDiagnosticMessage(d),
                    severity: diagnosticSeverity(d.severity),
                    line: d.line,
                    column: d.column,
                }));
                opts.onDiagnostics?.(editorDiagnostics);

                return result.diagnostics.map((d) => {
                    const { from, to } = diagnosticRange(view.state.doc, d);
                    const message = formatDiagnosticMessage(d);
                    return {
                        from,
                        to,
                        severity: diagnosticSeverity(d.severity),
                        message,
                        renderMessage: createDiagnosticRenderMessage(message),
                    } satisfies Diagnostic;
                });
            } catch {
                opts.onDiagnostics?.([]);
                return [];
            }
        },
        { delay: PARSE_DEBOUNCE_MS },
    );

    return [
        sql({ schema: catalog.schema, upperCaseKeywords: true }),
        completionExt,
        lintExt,
        lintPanelTheme,
        diagnosticNavigationKeymap,
    ];
}

export function bumpSqlEditorGeneration(completeGeneration: { current: number }): void {
    completeGeneration.current += 1;
}
