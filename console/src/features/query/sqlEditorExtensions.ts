import { autocompletion, type Completion, type CompletionContext } from '@codemirror/autocomplete';
import { linter, type Diagnostic } from '@codemirror/lint';
import type { Extension } from '@codemirror/state';
import { sql } from '@codemirror/lang-sql';
import type { EditorView } from '@codemirror/view';

import {
    completeSql,
    completionKindToType,
    parseSql,
    type SqlDiagnostic,
} from '@/lib/sqlTools';

import type { SqlCatalog } from './sqlCatalog';

const COMPLETE_DEBOUNCE_MS = 150;
const PARSE_DEBOUNCE_MS = 400;

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
}

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
            if (!sqlText.trim()) return [];

            if (!useSqlTools) return [];

            const gen = ++parseGeneration;
            await new Promise((resolve) => setTimeout(resolve, PARSE_DEBOUNCE_MS));
            if (gen !== parseGeneration) return [];

            try {
                const result = await parseSql({ sql: sqlText, offsetUnit: 'utf16' });
                if (gen !== parseGeneration) return [];

                return result.diagnostics.map((d) => {
                    const { from, to } = diagnosticRange(view.state.doc, d);
                    return {
                        from,
                        to,
                        severity: diagnosticSeverity(d.severity),
                        message: d.message,
                    } satisfies Diagnostic;
                });
            } catch {
                return [];
            }
        },
        { delay: PARSE_DEBOUNCE_MS },
    );

    return [sql({ schema: catalog.schema, upperCaseKeywords: true }), completionExt, lintExt];
}

export function bumpSqlEditorGeneration(completeGeneration: { current: number }): void {
    completeGeneration.current += 1;
}
