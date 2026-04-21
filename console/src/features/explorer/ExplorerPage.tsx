import { useMutation, useQuery, useQueryClient } from '@tanstack/react-query';
import { format } from 'sql-formatter';
import {
    ChevronLeft,
    ChevronRight,
    Folder,
    FolderOpen,
    Link2,
    Play,
    RefreshCw,
    Table2,
    AlignLeft,
} from 'lucide-react';
import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { useSearchParams } from 'react-router';

import { cn } from '@/lib/utils';
import type { QueryResponse, TableMetadata } from '@/types/api';

import { explorerQueries } from './api';
import { JsonViewer } from './components/JsonViewer';
import { ResultsTable } from './components/ResultsTable';
import { SqlEditor } from './components/SqlEditor';
import { buildExplorerSearchParams, parseExplorerSearchParams, type ResultsTab } from './urlState';

const UI_KEY = 'bigqueryExplorerUILayout';

interface UiPrefs {
    sidebarWidth: number;
    editorHeight: number;
    sidebarCollapsed: boolean;
    editorCollapsed: boolean;
}

const UI_DEFAULT: UiPrefs = {
    sidebarWidth: 320,
    editorHeight: 280,
    sidebarCollapsed: false,
    editorCollapsed: false,
};

function loadUiPrefs(): UiPrefs {
    try {
        const raw = localStorage.getItem(UI_KEY);
        if (!raw) return { ...UI_DEFAULT };
        const p = JSON.parse(raw) as Partial<UiPrefs>;
        return {
            sidebarWidth: Number(p.sidebarWidth) || UI_DEFAULT.sidebarWidth,
            editorHeight: Number(p.editorHeight) || UI_DEFAULT.editorHeight,
            sidebarCollapsed: p.sidebarCollapsed === true,
            editorCollapsed: p.editorCollapsed === true,
        };
    } catch {
        return { ...UI_DEFAULT };
    }
}

function saveUiPrefs(p: UiPrefs) {
    try {
        localStorage.setItem(UI_KEY, JSON.stringify(p));
    } catch {
        /* ignore */
    }
}

function defaultSql(project: string, dataset: string, table: string) {
    return `SELECT * FROM \`${project}.${dataset}.${table}\` LIMIT 100`;
}

export function ExplorerPage() {
    const queryClient = useQueryClient();
    const [searchParams, setSearchParams] = useSearchParams();

    const [expandedProjects, setExpandedProjects] = useState<string[]>([]);
    const [expandedDatasets, setExpandedDatasets] = useState<string[]>([]);
    const [projectDatasets, setProjectDatasets] = useState<Record<string, string[]>>({});
    const [datasetTables, setDatasetTables] = useState<Record<string, string[]>>({});

    const [currentProject, setCurrentProject] = useState('');
    const [currentDataset, setCurrentDataset] = useState('');
    const [currentTable, setCurrentTable] = useState('');
    const [activeTab, setActiveTab] = useState<ResultsTab>('info');
    const [sql, setSql] = useState('');
    const [queryResult, setQueryResult] = useState<QueryResponse | null>(null);
    const [ui, setUi] = useState<UiPrefs>(() => loadUiPrefs());

    const debounceUrl = useRef<ReturnType<typeof setTimeout> | null>(null);
    /** Project/dataset/table last applied from the URL — only clear query results when this triple changes. */
    const lastUrlTableKeyRef = useRef<string | null>(null);

    const syncUrl = useCallback(
        (next: {
            project: string;
            dataset: string;
            table: string;
            results: ResultsTab;
            query: string;
        }) => {
            const qs = buildExplorerSearchParams({
                project: next.project,
                dataset: next.dataset,
                table: next.table,
                results: next.results,
                query: next.query,
            });
            setSearchParams(qs ? new URLSearchParams(qs) : {}, { replace: true });
        },
        [setSearchParams],
    );

    const scheduleUrlSync = useCallback(
        (partial: Partial<{ results: ResultsTab; query: string }>) => {
            if (!currentProject || !currentDataset || !currentTable) return;
            if (debounceUrl.current) clearTimeout(debounceUrl.current);
            debounceUrl.current = setTimeout(() => {
                syncUrl({
                    project: currentProject,
                    dataset: currentDataset,
                    table: currentTable,
                    results: partial.results ?? activeTab,
                    query: partial.query ?? sql,
                });
            }, 350);
        },
        [activeTab, currentDataset, currentProject, currentTable, sql, syncUrl],
    );

    const { data: config } = useQuery({ queryKey: ['explorer', 'config'], queryFn: explorerQueries.config });
    const { data: projects = [], refetch: refetchProjects } = useQuery({
        queryKey: ['explorer', 'projects'],
        queryFn: explorerQueries.projects,
    });

    const schemaQuery = useQuery({
        queryKey: ['explorer', 'schema', currentProject, currentDataset, currentTable],
        queryFn: () => explorerQueries.tableSchema(currentProject, currentDataset, currentTable),
        enabled: Boolean(currentProject && currentDataset && currentTable),
    });

    const runMutation = useMutation({
        mutationFn: ({ q, projectId }: { q: string; projectId: string }) => explorerQueries.runQuery(q, projectId),
    });

    const applySelection = useCallback(
        async (project: string, dataset: string, table: string, tab: ResultsTab, queryText?: string) => {
            const dk = `${project}-${dataset}`;
            const ds = await explorerQueries.datasets(project);
            setProjectDatasets((m) => ({ ...m, [project]: ds }));
            const tb = await explorerQueries.tables(project, dataset);
            setDatasetTables((m) => ({ ...m, [dk]: tb }));

            setExpandedProjects((s) => (s.includes(project) ? s : [...s, project]));
            setExpandedDatasets((s) => (s.includes(dk) ? s : [...s, dk]));

            setCurrentProject(project);
            setCurrentDataset(dataset);
            setCurrentTable(table);
            setActiveTab(tab);
            const q = queryText?.trim() ? queryText : defaultSql(project, dataset, table);
            setSql(q);
            setQueryResult(null);
            syncUrl({ project, dataset, table, results: tab, query: q });
            await queryClient.invalidateQueries({
                queryKey: ['explorer', 'schema', project, dataset, table],
            });
        },
        [queryClient, syncUrl],
    );

    // Hydrate from URL when the address bar query changes (deep links, share, back/forward).
    useEffect(() => {
        if (!projects.length) return;
        const st = parseExplorerSearchParams(searchParams.toString());
        if (!st.project || !st.dataset || !st.table) return;
        if (!projects.includes(st.project)) return;

        let cancelled = false;
        void (async () => {
            const dsList = await explorerQueries.datasets(st.project);
            if (cancelled) return;
            setProjectDatasets((m) => ({ ...m, [st.project]: dsList }));
            if (!dsList.includes(st.dataset)) return;

            const dk = `${st.project}-${st.dataset}`;
            const tbList = await explorerQueries.tables(st.project, st.dataset);
            if (cancelled) return;
            setDatasetTables((m) => ({ ...m, [dk]: tbList }));
            if (!tbList.includes(st.table)) return;

            setExpandedProjects((prev) => (prev.includes(st.project) ? prev : [...prev, st.project]));
            setExpandedDatasets((prev) => (prev.includes(dk) ? prev : [...prev, dk]));

            const q = st.query.trim() ? st.query : defaultSql(st.project, st.dataset, st.table);
            const tableKey = `${st.project}/${st.dataset}/${st.table}`;
            if (lastUrlTableKeyRef.current !== tableKey) {
                lastUrlTableKeyRef.current = tableKey;
                setQueryResult(null);
            }
            setCurrentProject(st.project);
            setCurrentDataset(st.dataset);
            setCurrentTable(st.table);
            setActiveTab(st.results);
            setSql(q);
        })();

        return () => {
            cancelled = true;
        };
    }, [searchParams, projects]);

    const onToggleProject = async (p: string) => {
        if (expandedProjects.includes(p)) {
            setExpandedProjects((s) => s.filter((x) => x !== p));
        } else {
            setExpandedProjects((s) => [...s, p]);
            if (!projectDatasets[p]) {
                const ds = await explorerQueries.datasets(p);
                setProjectDatasets((m) => ({ ...m, [p]: ds }));
            }
        }
    };

    const onToggleDataset = async (project: string, dataset: string) => {
        const key = `${project}-${dataset}`;
        if (expandedDatasets.includes(key)) {
            setExpandedDatasets((s) => s.filter((k) => k !== key));
        } else {
            setExpandedDatasets((s) => [...s, key]);
            if (!datasetTables[key]) {
                const tb = await explorerQueries.tables(project, dataset);
                setDatasetTables((m) => ({ ...m, [key]: tb }));
            }
        }
    };

    const onSelectTable = (project: string, dataset: string, table: string) => {
        void applySelection(project, dataset, table, 'info');
    };

    const onRun = () => {
        if (!sql.trim() || !currentProject || !currentDataset || !currentTable) return;
        const q = sql;
        const project = currentProject;
        const dataset = currentDataset;
        const table = currentTable;
        runMutation.mutate({ q, projectId: project }, {
            onSuccess: (data) => {
                setQueryResult(data);
                setActiveTab('results');
                syncUrl({
                    project,
                    dataset,
                    table,
                    results: 'results',
                    query: q,
                });
            },
        });
    };

    const onFormat = () => {
        try {
            setSql(format(sql, { language: 'bigquery' }));
        } catch {
            /* ignore */
        }
    };

    const onShare = async () => {
        if (!currentProject || !currentDataset || !currentTable) return;
        const qs = buildExplorerSearchParams({
            project: currentProject,
            dataset: currentDataset,
            table: currentTable,
            results: activeTab,
            query: sql,
        });
        const url = `${window.location.origin}${window.location.pathname}?${qs}`;
        try {
            await navigator.clipboard.writeText(url);
        } catch {
            /* ignore */
        }
    };

    const onRefreshResources = async () => {
        setExpandedProjects([]);
        setExpandedDatasets([]);
        setProjectDatasets({});
        setDatasetTables({});
        await refetchProjects();
    };

    const [newProjectId, setNewProjectId] = useState('');
    const [adminErr, setAdminErr] = useState<string | null>(null);
    const addProject = async () => {
        const id = newProjectId.trim();
        if (!id) return;
        setAdminErr(null);
        try {
            await explorerQueries.createEmulatorProject(id);
            setNewProjectId('');
            await refetchProjects();
        } catch (e) {
            setAdminErr(e instanceof Error ? e.message : 'Failed');
        }
    };

    const tableMeta: TableMetadata | undefined = schemaQuery.data as TableMetadata | undefined;
    const showPanels = Boolean(currentTable);

    const jsonText = useMemo(
        () => (queryResult?.rows ? JSON.stringify(queryResult.rows, null, 2) : '[]'),
        [queryResult],
    );

    return (
        <div className="flex min-h-screen flex-col">
            <header className="flex items-center gap-3 border-b border-[var(--bq-border)] bg-[var(--bq-surface)] px-4 py-3">
                <h1 className="text-lg font-semibold">BigQuery Explorer</h1>
                <div className="ml-auto flex items-center gap-2">
                    <button
                        type="button"
                        className="inline-flex items-center gap-2 rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                        onClick={() => void onShare()}
                    >
                        <Link2 className="size-4" />
                        Share
                    </button>
                </div>
            </header>

            <div className="flex min-h-0 flex-1">
                <aside
                    className={cn(
                        'flex shrink-0 flex-col border-r border-[var(--bq-border)] bg-[var(--bq-surface)] transition-[width]',
                        ui.sidebarCollapsed && 'w-12',
                    )}
                    style={!ui.sidebarCollapsed ? { width: ui.sidebarWidth } : undefined}
                >
                    <div className="flex items-center justify-between gap-2 border-b border-[var(--bq-border)] p-2">
                        <button
                            type="button"
                            className="rounded p-1 hover:bg-white/10"
                            title={ui.sidebarCollapsed ? 'Expand' : 'Collapse'}
                            onClick={() => {
                                const n = { ...ui, sidebarCollapsed: !ui.sidebarCollapsed };
                                setUi(n);
                                saveUiPrefs(n);
                            }}
                        >
                            {ui.sidebarCollapsed ? <ChevronRight className="size-5" /> : <ChevronLeft className="size-5" />}
                        </button>
                        {!ui.sidebarCollapsed && <span className="text-sm font-medium">Resources</span>}
                        {!ui.sidebarCollapsed && (
                            <button
                                type="button"
                                className="rounded p-1 hover:bg-white/10"
                                title="Refresh"
                                onClick={() => void onRefreshResources()}
                            >
                                <RefreshCw className="size-4" />
                            </button>
                        )}
                    </div>
                    {!ui.sidebarCollapsed && (
                        <div className="flex-1 overflow-auto p-2">
                            <ul className="space-y-0.5 text-sm">
                                {projects.map((p) => (
                                    <li key={p}>
                                        <button
                                            type="button"
                                            className="flex w-full items-center gap-1 rounded px-1 py-0.5 text-left hover:bg-white/5"
                                            onClick={() => void onToggleProject(p)}
                                        >
                                            {expandedProjects.includes(p) ? (
                                                <FolderOpen className="size-4 shrink-0 text-amber-200/90" />
                                            ) : (
                                                <Folder className="size-4 shrink-0 text-amber-200/90" />
                                            )}
                                            <span className="truncate">{p}</span>
                                        </button>
                                        {expandedProjects.includes(p) && (
                                            <ul className="ml-4 border-l border-[var(--bq-border)] pl-2">
                                                {(projectDatasets[p] ?? []).map((d) => {
                                                    const dk = `${p}-${d}`;
                                                    return (
                                                        <li key={dk}>
                                                            <button
                                                                type="button"
                                                                className="flex w-full items-center gap-1 rounded px-1 py-0.5 text-left hover:bg-white/5"
                                                                onClick={() => void onToggleDataset(p, d)}
                                                            >
                                                                {expandedDatasets.includes(dk) ? (
                                                                    <FolderOpen className="size-4 shrink-0" />
                                                                ) : (
                                                                    <Folder className="size-4 shrink-0" />
                                                                )}
                                                                <span className="truncate">{d}</span>
                                                            </button>
                                                            {expandedDatasets.includes(dk) && (
                                                                <ul className="ml-4">
                                                                    {(datasetTables[dk] ?? []).map((t) => {
                                                                        const active =
                                                                            currentProject === p &&
                                                                            currentDataset === d &&
                                                                            currentTable === t;
                                                                        return (
                                                                            <li key={t}>
                                                                                <button
                                                                                    type="button"
                                                                                    className={cn(
                                                                                        'flex w-full items-center gap-1 rounded px-1 py-0.5 text-left hover:bg-white/5',
                                                                                        active && 'bg-blue-600/30',
                                                                                    )}
                                                                                    onClick={() =>
                                                                                        onSelectTable(p, d, t)
                                                                                    }
                                                                                >
                                                                                    <Table2 className="size-4 shrink-0" />
                                                                                    <span className="truncate">{t}</span>
                                                                                </button>
                                                                            </li>
                                                                        );
                                                                    })}
                                                                </ul>
                                                            )}
                                                        </li>
                                                    );
                                                })}
                                            </ul>
                                        )}
                                    </li>
                                ))}
                            </ul>
                        </div>
                    )}
                    {config?.allowEmulatorProjectAdmin && !ui.sidebarCollapsed && (
                        <div className="border-t border-[var(--bq-border)] p-2">
                            <div className="text-xs font-medium text-[var(--bq-muted)]">Add emulator project</div>
                            <div className="mt-1 flex gap-1">
                                <input
                                    className="min-w-0 flex-1 rounded border border-[var(--bq-border)] bg-black/20 px-2 py-1 text-sm"
                                    value={newProjectId}
                                    onChange={(e) => setNewProjectId(e.target.value)}
                                    placeholder="project-id"
                                />
                                <button
                                    type="button"
                                    className="rounded bg-blue-600 px-2 py-1 text-sm"
                                    onClick={() => void addProject()}
                                >
                                    Add
                                </button>
                            </div>
                            {adminErr && <p className="mt-1 text-xs text-red-400">{adminErr}</p>}
                        </div>
                    )}
                </aside>

                <main className="flex min-w-0 flex-1 flex-col">
                    {showPanels && (
                        <>
                            <section
                                className={cn('border-b border-[var(--bq-border)] p-3', ui.editorCollapsed && 'max-h-14')}
                                style={
                                    !ui.editorCollapsed
                                        ? { minHeight: ui.editorHeight }
                                        : { minHeight: undefined }
                                }
                            >
                                <div className="mb-2 flex items-center justify-between gap-2">
                                    <div className="truncate text-sm font-medium">
                                        Query: {currentProject}.{currentDataset}.{currentTable}
                                    </div>
                                    <button
                                        type="button"
                                        className="rounded border border-[var(--bq-border)] px-2 py-1 text-xs"
                                        onClick={() => {
                                            const n = { ...ui, editorCollapsed: !ui.editorCollapsed };
                                            setUi(n);
                                            saveUiPrefs(n);
                                        }}
                                    >
                                        {ui.editorCollapsed ? 'Show editor' : 'Hide editor'}
                                    </button>
                                </div>
                                {!ui.editorCollapsed && (
                                    <>
                                        <SqlEditor value={sql} onChange={(v) => {
                                            setSql(v);
                                            scheduleUrlSync({ query: v });
                                        }} />
                                        <div className="mt-2 flex flex-wrap gap-2">
                                            <button
                                                type="button"
                                                className="inline-flex items-center gap-2 rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium disabled:opacity-50"
                                                disabled={runMutation.isPending}
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
                                <div className="mb-2 flex gap-1 border-b border-[var(--bq-border)] text-sm">
                                    {(
                                        [
                                            ['info', 'Table info'],
                                            ['results', 'Results'],
                                            ['json', 'JSON'],
                                        ] as const
                                    ).map(([id, label]) => (
                                        <button
                                            key={id}
                                            type="button"
                                            className={cn(
                                                '-mb-px border-b-2 px-3 py-2',
                                                activeTab === id
                                                    ? 'border-blue-500 text-white'
                                                    : 'border-transparent text-[var(--bq-muted)] hover:text-white',
                                            )}
                                            onClick={() => {
                                                setActiveTab(id);
                                                scheduleUrlSync({ results: id });
                                            }}
                                        >
                                            {label}
                                            {id === 'results' && queryResult
                                                ? ` (${queryResult.total_rows ?? queryResult.rows.length})`
                                                : null}
                                        </button>
                                    ))}
                                </div>
                                <div className="min-h-0 flex-1 overflow-auto">
                                    {activeTab === 'info' && (
                                        <div className="space-y-4 text-sm">
                                            <table className="w-full border-collapse">
                                                <tbody>
                                                    {[
                                                        ['Name', tableMeta?.fullyQualifiedName ?? ''],
                                                        ['Description', tableMeta?.description || '—'],
                                                        ['Rows', tableMeta ? String(tableMeta.numRows) : '—'],
                                                        ['Bytes', tableMeta ? String(tableMeta.numBytes) : '—'],
                                                        ['Type', tableMeta?.type || '—'],
                                                        ['Location', tableMeta?.location || '—'],
                                                        ['Created', tableMeta?.creationTime || '—'],
                                                        ['Modified', tableMeta?.lastModified || '—'],
                                                    ].map(([k, v]) => (
                                                        <tr key={String(k)} className="border-b border-[var(--bq-border)]/50">
                                                            <th className="w-48 py-1 text-left font-medium text-[var(--bq-muted)]">
                                                                {k}
                                                            </th>
                                                            <td className="py-1">{v}</td>
                                                        </tr>
                                                    ))}
                                                </tbody>
                                            </table>
                                            <div>
                                                <div className="mb-1 font-medium">Schema</div>
                                                <div className="overflow-auto rounded border border-[var(--bq-border)]">
                                                    <table className="min-w-full text-sm">
                                                        <thead className="bg-[#243044]">
                                                            <tr>
                                                                {['Name', 'Type', 'Mode', 'Description'].map((h) => (
                                                                    <th key={h} className="px-2 py-1 text-left">
                                                                        {h}
                                                                    </th>
                                                                ))}
                                                            </tr>
                                                        </thead>
                                                        <tbody>
                                                            {(tableMeta?.schema ?? []).map((f) => (
                                                                <tr key={f.name} className="odd:bg-black/10">
                                                                    <td className="px-2 py-1">{f.name}</td>
                                                                    <td className="px-2 py-1">{f.type}</td>
                                                                    <td className="px-2 py-1">{f.mode}</td>
                                                                    <td className="px-2 py-1">{f.description || '—'}</td>
                                                                </tr>
                                                            ))}
                                                        </tbody>
                                                    </table>
                                                </div>
                                            </div>
                                        </div>
                                    )}
                                    {activeTab === 'results' && <ResultsTable data={queryResult} />}
                                    {activeTab === 'json' && <JsonViewer value={jsonText} />}
                                </div>
                            </section>
                        </>
                    )}

                    {!showPanels && (
                        <div className="flex flex-1 items-center justify-center p-8 text-[var(--bq-muted)]">
                            Select a table from the tree to explore metadata and run queries.
                        </div>
                    )}
                </main>
            </div>
        </div>
    );
}
