import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query';
import { format as formatClient } from 'sql-formatter';
import {
    AlignLeft,
    BookOpen,
    ChevronDown,
    Link2,
    Play,
    Save,
    Settings2,
} from 'lucide-react';
import { useCallback, useEffect, useId, useMemo, useRef, useState } from 'react';
import { useSearchParams } from 'react-router';

import { TabBar } from '@/components/ui/Tabs';
import { cn } from '@/lib/utils';
import { formatSql, isSqlToolsAvailable, parseSql, probeCapabilities } from '@/lib/sqlTools';

import { explorerQueries } from '@/features/explorer/api';
import { notifyTablesChanged } from '@/features/explorer/events';
import { JsonViewer } from '@/features/explorer/components/JsonViewer';
import { ResultsTable } from '@/features/explorer/components/ResultsTable';
import { SqlEditor } from '@/features/explorer/components/SqlEditor';
import { buildExplorerSearchParams } from '@/features/explorer/urlState';
import { ReferencePanel } from '@/features/query/ReferencePanel';
import { SaveDestinationModal, type SaveDestination } from '@/features/query/SaveDestinationModal';
import { SaveNameModal } from '@/features/query/SaveNameModal';
import { loadSqlCatalog } from '@/features/query/sqlCatalog';

import { useWorkspace } from '@/features/workspace/store';
import type { QuerySubTab, QueryTabState } from '@/features/workspace/types';

interface QueryTabProps {
    tab: QueryTabState;
}

type SaveAction = 'view' | 'table' | null;

export function QueryTab({ tab }: QueryTabProps) {
    const queryClient = useQueryClient();
    const { ui, session, updateQueryTab, updateUi, saveQueryClassic, saveQueryVersioned } = useWorkspace();
    const [, setSearchParams] = useSearchParams();
    const debounceUrl = useRef<ReturnType<typeof setTimeout> | null>(null);

    const toolsMenuId = useId();
    const saveMenuId = useId();
    const toolsRef = useRef<HTMLDivElement>(null);
    const saveRef = useRef<HTMLDivElement>(null);
    const [toolsOpen, setToolsOpen] = useState(false);
    const [saveOpen, setSaveOpen] = useState(false);
    const [saveAction, setSaveAction] = useState<SaveAction>(null);

    const { data: sqlToolsAvailable = false } = useQuery({
        queryKey: ['sql-tools', 'capabilities'],
        queryFn: probeCapabilities,
        staleTime: Infinity,
    });

    const useEmulatorParser = ui.useEmulatorParser && sqlToolsAvailable;
    const needsCatalog = !useEmulatorParser && Boolean(tab.projectId);

    const { data: catalog = null } = useQuery({
        queryKey: ['sql-catalog', tab.projectId],
        queryFn: () => loadSqlCatalog(tab.projectId),
        enabled: needsCatalog,
        staleTime: 60_000,
    });

    useEffect(() => {
        if (!toolsOpen && !saveOpen) return;

        const onPointerDown = (event: MouseEvent) => {
            if (toolsOpen && !toolsRef.current?.contains(event.target as Node)) {
                setToolsOpen(false);
            }
            if (saveOpen && !saveRef.current?.contains(event.target as Node)) {
                setSaveOpen(false);
            }
        };

        document.addEventListener('mousedown', onPointerDown);
        return () => document.removeEventListener('mousedown', onPointerDown);
    }, [toolsOpen, saveOpen]);

    const runMutation = useMutation({
        mutationFn: ({ q, projectId }: { q: string; projectId: string }) => explorerQueries.runQuery(q, projectId),
    });

    const saveDdlMutation = useMutation({
        mutationFn: ({ ddl, projectId }: { ddl: string; projectId: string }) => explorerQueries.runQuery(ddl, projectId),
    });

    const syncShareUrl = useCallback(
        (patch: Partial<{ subTab: QuerySubTab; sql: string }>) => {
            if (!tab.projectId || !tab.datasetId || !tab.tableId) return;
            const subTab = patch.subTab ?? tab.subTab;
            const sql = patch.sql ?? tab.sql;
            const qs = buildExplorerSearchParams({
                project: tab.projectId,
                dataset: tab.datasetId,
                table: tab.tableId,
                results: subTab,
                query: sql,
            });
            if (debounceUrl.current) clearTimeout(debounceUrl.current);
            debounceUrl.current = setTimeout(() => {
                setSearchParams(qs ? new URLSearchParams(qs) : {}, { replace: true });
            }, 350);
        },
        [setSearchParams, tab.datasetId, tab.projectId, tab.sql, tab.subTab, tab.tableId],
    );

    const onRun = () => {
        if (!tab.sql.trim() || !tab.projectId) return;
        const q = tab.sql;
        runMutation.mutate(
            { q, projectId: tab.projectId },
            {
                onSuccess: (data) => {
                    updateQueryTab(tab.id, { queryResult: data, subTab: 'results' });
                    syncShareUrl({ subTab: 'results', sql: q });
                },
            },
        );
    };

    const onFormat = async () => {
        const useSqlToolsFormat = ui.useEmulatorParser && (sqlToolsAvailable || isSqlToolsAvailable());
        if (useSqlToolsFormat) {
            try {
                const result = await formatSql({ sql: tab.sql, strict: ui.strictFormat, offsetUnit: 'utf16' });
                updateQueryTab(tab.id, { sql: result.formattedSql });
                syncShareUrl({ sql: result.formattedSql });
                return;
            } catch {
                /* fall through to client formatter */
            }
        }

        try {
            const formatted = formatClient(tab.sql, { language: 'bigquery' });
            updateQueryTab(tab.id, { sql: formatted });
            syncShareUrl({ sql: formatted });
        } catch {
            /* ignore */
        }
    };

    const onShare = async () => {
        if (!tab.projectId || !tab.datasetId || !tab.tableId) return;
        const qs = buildExplorerSearchParams({
            project: tab.projectId,
            dataset: tab.datasetId,
            table: tab.tableId,
            results: tab.subTab,
            query: tab.sql,
        });
        const url = `${window.location.origin}/?${qs}`;
        try {
            await navigator.clipboard.writeText(url);
        } catch {
            /* ignore */
        }
    };

    const validateSelectQuery = async (): Promise<boolean> => {
        if (!ui.useEmulatorParser || !sqlToolsAvailable) return true;
        try {
            const parsed = await parseSql({ sql: tab.sql, offsetUnit: 'utf16' });
            if (parsed.diagnostics.some((d) => d.severity === 'error')) return false;
            return parsed.statementKinds.every((k) => k === 'QueryStatement');
        } catch {
            return true;
        }
    };

    const onSaveQueryClassic = () => {
        if (!tab.projectId || !tab.sql.trim()) return;
        saveQueryClassic({
            title: tab.title,
            sql: tab.sql,
            projectId: tab.projectId,
            datasetId: tab.datasetId,
            tableId: tab.tableId,
        });
        setSaveOpen(false);
    };

    const onSaveQueryVersioned = () => {
        if (!tab.projectId || !tab.sql.trim()) return;
        saveQueryVersioned({
            title: tab.title,
            sql: tab.sql,
            projectId: tab.projectId,
            datasetId: tab.datasetId,
            tableId: tab.tableId,
        });
        setSaveOpen(false);
    };

    const onSaveView = async ({ projectId, datasetId, name }: SaveDestination) => {
        if (!projectId || !datasetId) return;
        const ok = await validateSelectQuery();
        if (!ok) return;

        const ddl = `CREATE OR REPLACE VIEW \`${projectId}.${datasetId}.${name}\` AS\n${tab.sql.trim()}`;
        saveDdlMutation.mutate(
            { ddl, projectId },
            {
                onSuccess: () => {
                    notifyTablesChanged(projectId, datasetId);
                    void queryClient.invalidateQueries({ queryKey: ['explorer'] });
                },
            },
        );
    };

    const onSaveAsTable = async (tableName: string) => {
        if (!tab.projectId || !tab.datasetId) return;
        const ok = await validateSelectQuery();
        if (!ok) return;

        const ddl = `CREATE OR REPLACE TABLE \`${tab.projectId}.${tab.datasetId}.${tableName}\` AS\n${tab.sql.trim()}`;
        saveDdlMutation.mutate(
            { ddl, projectId: tab.projectId },
            {
                onSuccess: () => {
                    notifyTablesChanged(tab.projectId, tab.datasetId!);
                    void queryClient.invalidateQueries({ queryKey: ['explorer'] });
                },
            },
        );
    };

    const jsonText = useMemo(
        () => (tab.queryResult?.rows ? JSON.stringify(tab.queryResult.rows, null, 2) : '[]'),
        [tab.queryResult],
    );

    const title =
        tab.datasetId && tab.tableId
            ? `${tab.projectId}.${tab.datasetId}.${tab.tableId}`
            : tab.title;

    const savedClassicCount = session.savedQueriesClassic.filter((q) => q.title === tab.title).length;
    const savedVersioned = session.savedQueriesVersioned.find(
        (q) => q.title === tab.title && q.projectId === tab.projectId,
    );

    return (
        <div className="flex min-h-0 flex-1">
            <div className="flex min-h-0 min-w-0 flex-1 flex-col">
                <section
                    className={cn('border-b border-[var(--bq-border)] p-3', ui.editorCollapsed && 'max-h-14')}
                    style={!ui.editorCollapsed ? { minHeight: ui.editorHeight } : { minHeight: undefined }}
                >
                    <div className="mb-2 flex items-center justify-between gap-2">
                        <div className="truncate text-sm font-medium">Query: {title}</div>
                        <div className="flex items-center gap-2">
                            {tab.datasetId && tab.tableId ? (
                                <button
                                    type="button"
                                    className="inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                                    onClick={() => void onShare()}
                                >
                                    <Link2 className="size-4" />
                                    Share
                                </button>
                            ) : null}
                            <button
                                type="button"
                                data-testid="toggle-reference-panel"
                                className={cn(
                                    'inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5',
                                    ui.referencePanelOpen && 'bg-white/5',
                                )}
                                onClick={() => updateUi({ referencePanelOpen: !ui.referencePanelOpen })}
                            >
                                <BookOpen className="size-4" />
                                Reference
                            </button>
                            <button
                                type="button"
                                className="rounded border border-[var(--bq-border)] px-2 py-1 text-xs"
                                onClick={() => updateUi({ editorCollapsed: !ui.editorCollapsed })}
                            >
                                {ui.editorCollapsed ? 'Show editor' : 'Hide editor'}
                            </button>
                        </div>
                    </div>
                    {!ui.editorCollapsed && (
                        <>
                            <SqlEditor
                                value={tab.sql}
                                projectId={tab.projectId}
                                defaultDatasetId={tab.datasetId}
                                useEmulatorParser={ui.useEmulatorParser}
                                sqlToolsAvailable={sqlToolsAvailable}
                                catalog={catalog}
                                onChange={(v) => {
                                    updateQueryTab(tab.id, { sql: v });
                                    syncShareUrl({ sql: v });
                                }}
                            />
                            <div className="mt-2 flex flex-wrap items-center gap-2">
                                <button
                                    type="button"
                                    data-testid="run-query"
                                    className="inline-flex items-center gap-2 rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium disabled:opacity-50"
                                    disabled={runMutation.isPending || !tab.projectId}
                                    onClick={onRun}
                                >
                                    <Play className="size-4" />
                                    {runMutation.isPending ? 'Running…' : 'Run query'}
                                </button>
                                <button
                                    type="button"
                                    data-testid="format-sql"
                                    className="inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                                    onClick={() => void onFormat()}
                                >
                                    <AlignLeft className="size-4" />
                                    Format SQL
                                </button>

                                <div ref={saveRef} className="relative inline-flex">
                                    <button
                                        type="button"
                                        data-testid="save-query-menu"
                                        className="inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                                        aria-haspopup="menu"
                                        aria-expanded={saveOpen}
                                        aria-controls={saveMenuId}
                                        disabled={!tab.projectId || !tab.sql.trim()}
                                        onClick={() => setSaveOpen((v) => !v)}
                                    >
                                        <Save className="size-4" />
                                        Save
                                        <ChevronDown className="size-3" />
                                    </button>
                                    {saveOpen ? (
                                        <div
                                            id={saveMenuId}
                                            role="menu"
                                            className="absolute left-0 top-full z-20 mt-1 min-w-48 rounded-md border border-[var(--bq-border)] bg-[var(--bq-surface)] py-1 shadow-lg"
                                        >
                                            <button
                                                type="button"
                                                role="menuitem"
                                                data-testid="save-query-versioned"
                                                className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5"
                                                onClick={onSaveQueryVersioned}
                                            >
                                                Save query
                                                {savedVersioned ? ` (${savedVersioned.versions.length + 1})` : ''}
                                            </button>
                                            <button
                                                type="button"
                                                role="menuitem"
                                                data-testid="save-query-classic"
                                                className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5"
                                                onClick={onSaveQueryClassic}
                                            >
                                                Save query (Classic)
                                                {savedClassicCount > 0 ? ` (${savedClassicCount + 1})` : ''}
                                            </button>
                                            <button
                                                type="button"
                                                role="menuitem"
                                                data-testid="save-view"
                                                className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5 disabled:opacity-50"
                                                disabled={saveDdlMutation.isPending}
                                                onClick={() => {
                                                    setSaveOpen(false);
                                                    setSaveAction('view');
                                                }}
                                            >
                                                Save view (DDL)
                                            </button>
                                            <button
                                                type="button"
                                                role="menuitem"
                                                data-testid="save-as-table"
                                                className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5 disabled:opacity-50"
                                                disabled={!tab.datasetId || saveDdlMutation.isPending}
                                                onClick={() => {
                                                    setSaveOpen(false);
                                                    setSaveAction('table');
                                                }}
                                            >
                                                Save as…
                                            </button>
                                        </div>
                                    ) : null}
                                </div>

                                <div ref={toolsRef} className="relative inline-flex">
                                    <button
                                        type="button"
                                        data-testid="tools-menu"
                                        className="inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                                        aria-haspopup="menu"
                                        aria-expanded={toolsOpen}
                                        aria-controls={toolsMenuId}
                                        onClick={() => setToolsOpen((v) => !v)}
                                    >
                                        <Settings2 className="size-4" />
                                        Tools
                                        <ChevronDown className="size-3" />
                                    </button>
                                    {toolsOpen ? (
                                        <div
                                            id={toolsMenuId}
                                            role="menu"
                                            className="absolute left-0 top-full z-20 mt-1 min-w-56 rounded-md border border-[var(--bq-border)] bg-[var(--bq-surface)] py-1 shadow-lg"
                                        >
                                            <button
                                                type="button"
                                                role="menuitem"
                                                data-testid="toggle-emulator-parser"
                                                className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5"
                                                onClick={() =>
                                                    updateUi({ useEmulatorParser: !ui.useEmulatorParser })
                                                }
                                            >
                                                {ui.useEmulatorParser ? '✓ ' : ''}
                                                Emulator parser (SQL Tools)
                                                {!sqlToolsAvailable ? ' — unavailable' : ''}
                                            </button>
                                            <button
                                                type="button"
                                                role="menuitem"
                                                data-testid="toggle-strict-format"
                                                className="flex w-full px-3 py-1.5 text-left text-sm hover:bg-white/5"
                                                onClick={() => updateUi({ strictFormat: !ui.strictFormat })}
                                            >
                                                {ui.strictFormat ? '✓ ' : ''}
                                                Strict format
                                            </button>
                                        </div>
                                    ) : null}
                                </div>
                            </div>
                        </>
                    )}
                </section>

                <section className="flex min-h-0 flex-1 flex-col p-3">
                    <TabBar
                        variant="underline"
                        activeId={tab.subTab}
                        onChange={(id) => {
                            const subTab = id as QuerySubTab;
                            updateQueryTab(tab.id, { subTab });
                            syncShareUrl({ subTab });
                        }}
                        tabs={[
                            {
                                id: 'results',
                                label: 'Results',
                                testId: 'results-tab',
                                badge:
                                    tab.queryResult != null
                                        ? `(${tab.queryResult.total_rows ?? tab.queryResult.rows.length})`
                                        : undefined,
                            },
                            { id: 'json', label: 'JSON' },
                        ]}
                        className="mb-2"
                    />
                    <div className="min-h-0 flex-1 overflow-auto">
                        {tab.subTab === 'results' ? (
                            <ResultsTable data={tab.queryResult ?? null} />
                        ) : (
                            <JsonViewer value={jsonText} />
                        )}
                    </div>
                </section>
            </div>

            <ReferencePanel
                projectId={tab.projectId}
                datasetId={tab.datasetId}
                tableId={tab.tableId}
                open={ui.referencePanelOpen}
                onClose={() => updateUi({ referencePanelOpen: false })}
            />

            <SaveDestinationModal
                open={saveAction === 'view'}
                title="Save view"
                nameLabel="View name"
                defaultProjectId={tab.projectId}
                defaultDatasetId={tab.datasetId}
                defaultName={tab.tableId ? `${tab.tableId}_view` : ''}
                submitLabel="Save view"
                testId="save-view-modal"
                onClose={() => setSaveAction(null)}
                onSubmit={(destination) => void onSaveView(destination)}
            />

            <SaveNameModal
                open={saveAction === 'table'}
                title="Save as table"
                label="Table name"
                defaultValue=""
                submitLabel="Save as table"
                testId="save-as-modal"
                onClose={() => setSaveAction(null)}
                onSubmit={(name) => void onSaveAsTable(name)}
            />
        </div>
    );
}
