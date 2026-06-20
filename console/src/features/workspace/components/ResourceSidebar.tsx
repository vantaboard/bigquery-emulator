import { useQuery } from '@tanstack/react-query';
import {
    ChevronDown,
    ChevronLeft,
    ChevronRight,
    Folder,
    FolderOpen,
    RefreshCw,
    Table2,
} from 'lucide-react';
import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router';

import { cn } from '@/lib/utils';

import { explorerQueries } from '@/features/explorer/api';
import { EXPLORER_DATASETS_CHANGED, EXPLORER_TABLES_CHANGED } from '@/features/explorer/events';
import { tabRoutePath, useWorkspace } from '@/features/workspace/store';
import { resourceTabId } from '@/features/workspace/types';

export function ResourceSidebar() {
    const navigate = useNavigate();
    const { ui, updateUi, openDatasetTab, openTableTab, activeTab } = useWorkspace();

    const [expandedProjects, setExpandedProjects] = useState<string[]>([]);
    const [expandedDatasets, setExpandedDatasets] = useState<string[]>([]);
    const [projectDatasets, setProjectDatasets] = useState<Record<string, string[]>>({});
    const [datasetTables, setDatasetTables] = useState<Record<string, string[]>>({});

    const [newProjectId, setNewProjectId] = useState('');
    const [adminErr, setAdminErr] = useState<string | null>(null);

    const { data: config } = useQuery({ queryKey: ['explorer', 'config'], queryFn: explorerQueries.config });
    const { data: projects = [], refetch: refetchProjects } = useQuery({
        queryKey: ['explorer', 'projects'],
        queryFn: explorerQueries.projects,
    });

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

    const onSelectDataset = (project: string, dataset: string) => {
        openDatasetTab(project, dataset);
        const id = resourceTabId('dataset', project, dataset);
        navigate(tabRoutePath({ type: 'dataset', id, projectId: project, datasetId: dataset }));
    };

    const onSelectTable = (project: string, dataset: string, table: string) => {
        openTableTab(project, dataset, table);
        const id = resourceTabId('table', project, dataset, table);
        navigate(tabRoutePath({ type: 'table', id, projectId: project, datasetId: dataset, tableId: table }));
    };

    const onRefreshResources = async () => {
        setExpandedProjects([]);
        setExpandedDatasets([]);
        setProjectDatasets({});
        setDatasetTables({});
        await refetchProjects();
    };

    useEffect(() => {
        const onTablesChanged = (event: Event) => {
            const detail = (event as CustomEvent<{ projectId: string; datasetId: string }>).detail;
            if (!detail?.projectId || !detail?.datasetId) return;
            const key = `${detail.projectId}-${detail.datasetId}`;
            void (async () => {
                const [datasets, tables] = await Promise.all([
                    explorerQueries.datasets(detail.projectId),
                    explorerQueries.tables(detail.projectId, detail.datasetId),
                ]);
                setProjectDatasets((m) => ({ ...m, [detail.projectId]: datasets }));
                setDatasetTables((m) => ({ ...m, [key]: tables }));
                setExpandedProjects((s) => (s.includes(detail.projectId) ? s : [...s, detail.projectId]));
                setExpandedDatasets((s) => (s.includes(key) ? s : [...s, key]));
            })();
        };
        const onDatasetsChanged = (event: Event) => {
            const detail = (event as CustomEvent<{ projectId: string }>).detail;
            if (!detail?.projectId) return;
            void (async () => {
                const datasets = await explorerQueries.datasets(detail.projectId);
                setProjectDatasets((m) => ({ ...m, [detail.projectId]: datasets }));
                setExpandedProjects((s) => (s.includes(detail.projectId) ? s : [...s, detail.projectId]));
            })();
        };
        window.addEventListener(EXPLORER_TABLES_CHANGED, onTablesChanged);
        window.addEventListener(EXPLORER_DATASETS_CHANGED, onDatasetsChanged);
        return () => {
            window.removeEventListener(EXPLORER_TABLES_CHANGED, onTablesChanged);
            window.removeEventListener(EXPLORER_DATASETS_CHANGED, onDatasetsChanged);
        };
    }, []);

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

    return (
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
                    onClick={() => updateUi({ sidebarCollapsed: !ui.sidebarCollapsed })}
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
                                    data-testid={`project-${p}`}
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
                                                    <div className="flex w-full items-center gap-0.5 rounded px-0.5 hover:bg-white/5">
                                                        <button
                                                            type="button"
                                                            data-testid={`dataset-toggle-${d}`}
                                                            className="rounded p-0.5 hover:bg-white/10"
                                                            aria-label={
                                                                expandedDatasets.includes(dk)
                                                                    ? `Collapse ${d}`
                                                                    : `Expand ${d}`
                                                            }
                                                            onClick={() => void onToggleDataset(p, d)}
                                                        >
                                                            {expandedDatasets.includes(dk) ? (
                                                                <ChevronDown className="size-4 shrink-0" />
                                                            ) : (
                                                                <ChevronRight className="size-4 shrink-0" />
                                                            )}
                                                        </button>
                                                        <button
                                                            type="button"
                                                            data-testid={`dataset-${d}`}
                                                            className="flex min-w-0 flex-1 items-center gap-1 rounded px-1 py-0.5 text-left"
                                                            onClick={() => onSelectDataset(p, d)}
                                                        >
                                                            {expandedDatasets.includes(dk) ? (
                                                                <FolderOpen className="size-4 shrink-0" />
                                                            ) : (
                                                                <Folder className="size-4 shrink-0" />
                                                            )}
                                                            <span className="truncate">{d}</span>
                                                        </button>
                                                    </div>
                                                    {expandedDatasets.includes(dk) && (
                                                        <ul className="ml-4">
                                                            {(datasetTables[dk] ?? []).map((t) => {
                                                                const active =
                                                                    activeTab?.type === 'table' &&
                                                                    activeTab.projectId === p &&
                                                                    activeTab.datasetId === d &&
                                                                    activeTab.tableId === t;
                                                                return (
                                                                    <li key={t}>
                                                                        <button
                                                                            type="button"
                                                                            data-testid={`table-${t}`}
                                                                            className={cn(
                                                                                'flex w-full items-center gap-1 rounded px-1 py-0.5 text-left hover:bg-white/5',
                                                                                active && 'bg-blue-600/30',
                                                                            )}
                                                                            onClick={() => onSelectTable(p, d, t)}
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
    );
}
