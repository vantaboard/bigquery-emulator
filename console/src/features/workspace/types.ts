import type { QueryResponse } from '@/types/api';

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
}

export type WorkspaceTab = QueryTabState | DatasetTabState | TableTabState;

export interface UiPrefs {
    sidebarWidth: number;
    editorHeight: number;
    sidebarCollapsed: boolean;
    editorCollapsed: boolean;
}

export interface WorkspaceSession {
    tabs: WorkspaceTab[];
    tabOrder: string[];
    activeTabId: string | null;
    ui: UiPrefs;
}

export const UI_DEFAULT: UiPrefs = {
    sidebarWidth: 320,
    editorHeight: 280,
    sidebarCollapsed: false,
    editorCollapsed: false,
};

export const LEGACY_UI_KEY = 'bigqueryExplorerUILayout';
export const SESSION_KEY = 'bigqueryWorkspaceSession';

export function resourceTabId(kind: 'dataset' | 'table', projectId: string, datasetId: string, tableId?: string): string {
    if (kind === 'dataset') return `dataset:${projectId}:${datasetId}`;
    return `table:${projectId}:${datasetId}:${tableId}`;
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
    }
}

export function newQueryTabId(): string {
    return `query-${crypto.randomUUID()}`;
}
