import type { QueryResponse, ResourceType } from '@/types/api';

export type QuerySubTab = 'results' | 'json';

export interface QueryTabState {
    type: 'query';
    id: string;
    title: string;
    sql: string;
    subTab: QuerySubTab;
    projectId: string;
    datasetId?: string;
    tableId?: string;
    queryResult?: QueryResponse | null;
}

export interface DatasetTabState {
    type: 'dataset';
    id: string;
    projectId: string;
    datasetId: string;
}

export interface TableTabState {
    type: 'table';
    id: string;
    projectId: string;
    datasetId: string;
    tableId: string;
    /** Present when known from tables.list or tables.get; defaults to TABLE for icons. */
    resourceType?: ResourceType;
}

export interface RoutineTabState {
    type: 'routine';
    id: string;
    projectId: string;
    datasetId: string;
    routineId: string;
}

export type WorkspaceTab = QueryTabState | DatasetTabState | TableTabState | RoutineTabState;

export interface UiPrefs {
    sidebarWidth: number;
    editorHeight: number;
    sidebarCollapsed: boolean;
    editorCollapsed: boolean;
    /** Prefer emulator SQL Tools parser over client fallback. */
    useEmulatorParser: boolean;
    /** Pass strict: true to SQL Tools /format. */
    strictFormat: boolean;
    /** Show schema reference side panel in query tabs. */
    referencePanelOpen: boolean;
}

export interface SavedQueryClassic {
    id: string;
    title: string;
    sql: string;
    projectId: string;
    datasetId?: string;
    tableId?: string;
    savedAt: string;
}

export interface SavedQueryVersionEntry {
    sql: string;
    savedAt: string;
}

export interface SavedQueryVersioned {
    id: string;
    title: string;
    projectId: string;
    datasetId?: string;
    tableId?: string;
    versions: SavedQueryVersionEntry[];
}

export interface WorkspaceSession {
    tabs: WorkspaceTab[];
    tabOrder: string[];
    activeTabId: string | null;
    ui: UiPrefs;
    savedQueriesClassic: SavedQueryClassic[];
    savedQueriesVersioned: SavedQueryVersioned[];
}

export const UI_DEFAULT: UiPrefs = {
    sidebarWidth: 320,
    editorHeight: 280,
    sidebarCollapsed: false,
    editorCollapsed: false,
    useEmulatorParser: true,
    strictFormat: false,
    referencePanelOpen: false,
};

export const LEGACY_UI_KEY = 'bigqueryExplorerUILayout';
export const SESSION_KEY = 'bigqueryWorkspaceSession';

export function resourceTabId(
    kind: 'dataset' | 'table' | 'routine',
    projectId: string,
    datasetId: string,
    resourceId?: string,
): string {
    if (kind === 'dataset') return `dataset:${projectId}:${datasetId}`;
    if (kind === 'table') return `table:${projectId}:${datasetId}:${resourceId}`;
    return `routine:${projectId}:${datasetId}:${resourceId}`;
}

export function defaultSql(project: string, dataset: string, table: string): string {
    return `SELECT * FROM \`${project}.${dataset}.${table}\` LIMIT 1000`;
}

export function tabLabel(tab: WorkspaceTab): string {
    switch (tab.type) {
        case 'query':
            return tab.title;
        case 'dataset':
            return tab.datasetId;
        case 'table':
            return tab.tableId;
        case 'routine':
            return tab.routineId;
    }
}

export function newQueryTabId(): string {
    return `query-${crypto.randomUUID()}`;
}

export function newSavedQueryId(): string {
    return `saved-${crypto.randomUUID()}`;
}
