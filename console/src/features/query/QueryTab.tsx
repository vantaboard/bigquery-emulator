import { useMutation } from '@tanstack/react-query';
import { format } from 'sql-formatter';
import { AlignLeft, Link2, Play } from 'lucide-react';
import { useCallback, useMemo, useRef } from 'react';
import { useSearchParams } from 'react-router';

import { TabBar } from '@/components/ui/Tabs';
import { cn } from '@/lib/utils';

import { explorerQueries } from '@/features/explorer/api';
import { JsonViewer } from '@/features/explorer/components/JsonViewer';
import { ResultsTable } from '@/features/explorer/components/ResultsTable';
import { SqlEditor } from '@/features/explorer/components/SqlEditor';
import { buildExplorerSearchParams } from '@/features/explorer/urlState';

import { useWorkspace } from '@/features/workspace/store';
import type { QuerySubTab, QueryTabState } from '@/features/workspace/types';

interface QueryTabProps {
    tab: QueryTabState;
}

export function QueryTab({ tab }: QueryTabProps) {
    const { ui, updateQueryTab, updateUi } = useWorkspace();
    const [, setSearchParams] = useSearchParams();
    const debounceUrl = useRef<ReturnType<typeof setTimeout> | null>(null);

    const runMutation = useMutation({
        mutationFn: ({ q, projectId }: { q: string; projectId: string }) => explorerQueries.runQuery(q, projectId),
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

    const onFormat = () => {
        try {
            const formatted = format(tab.sql, { language: 'bigquery' });
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

    const jsonText = useMemo(
        () => (tab.queryResult?.rows ? JSON.stringify(tab.queryResult.rows, null, 2) : '[]'),
        [tab.queryResult],
    );

    const title =
        tab.datasetId && tab.tableId
            ? `${tab.projectId}.${tab.datasetId}.${tab.tableId}`
            : tab.title;

    return (
        <div className="flex min-h-0 flex-1 flex-col">
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
                            onChange={(v) => {
                                updateQueryTab(tab.id, { sql: v });
                                syncShareUrl({ sql: v });
                            }}
                        />
                        <div className="mt-2 flex flex-wrap gap-2">
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
                                className="inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                                onClick={onFormat}
                            >
                                <AlignLeft className="size-4" />
                                Format SQL
                            </button>
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
    );
}
