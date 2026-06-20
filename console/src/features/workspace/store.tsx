import {
    createContext,
    useCallback,
    useContext,
    useEffect,
    useMemo,
    useReducer,
    type Dispatch,
    type ReactNode,
} from 'react';

import type { QueryResponse } from '@/types/api';

import {
    LEGACY_UI_KEY,
    SESSION_KEY,
    UI_DEFAULT,
    defaultSql,
    newQueryTabId,
    newSavedQueryId,
    resourceTabId,
    type DatasetTabState,
    type QuerySubTab,
    type QueryTabState,
    type SavedQueryClassic,
    type SavedQueryVersioned,
    type TableTabState,
    type UiPrefs,
    type WorkspaceSession,
    type WorkspaceTab,
} from './types';

type WorkspaceAction =
    | { type: 'HYDRATE'; session: WorkspaceSession }
    | { type: 'OPEN_QUERY_TAB'; tab: QueryTabState; activate?: boolean }
    | { type: 'OPEN_DATASET_TAB'; projectId: string; datasetId: string; activate?: boolean }
    | { type: 'OPEN_TABLE_TAB'; projectId: string; datasetId: string; tableId: string; activate?: boolean }
    | { type: 'ACTIVATE_TAB'; id: string }
    | { type: 'CLOSE_TAB'; id: string }
    | { type: 'REORDER_TAB'; id: string; toIndex: number }
    | { type: 'RENAME_TAB'; id: string; title: string }
    | { type: 'UPDATE_QUERY_TAB'; id: string; patch: Partial<Omit<QueryTabState, 'type' | 'id'>> }
    | { type: 'UPDATE_UI'; patch: Partial<UiPrefs> }
    | { type: 'SAVE_QUERY_CLASSIC'; entry: SavedQueryClassic }
    | { type: 'SAVE_QUERY_VERSIONED'; entry: SavedQueryVersioned; sql: string };

function loadLegacyUiPrefs(): UiPrefs {
    try {
        const raw = localStorage.getItem(LEGACY_UI_KEY);
        if (!raw) return { ...UI_DEFAULT };
        const p = JSON.parse(raw) as Partial<UiPrefs>;
        return {
            sidebarWidth: Number(p.sidebarWidth) || UI_DEFAULT.sidebarWidth,
            editorHeight: Number(p.editorHeight) || UI_DEFAULT.editorHeight,
            sidebarCollapsed: p.sidebarCollapsed === true,
            editorCollapsed: p.editorCollapsed === true,
            useEmulatorParser: p.useEmulatorParser !== false,
            strictFormat: p.strictFormat === true,
            referencePanelOpen: p.referencePanelOpen === true,
        };
    } catch {
        return { ...UI_DEFAULT };
    }
}

function parseSession(raw: string): WorkspaceSession | null {
    try {
        const data = JSON.parse(raw) as Partial<WorkspaceSession>;
        if (!Array.isArray(data.tabs) || !Array.isArray(data.tabOrder)) return null;
        const ui = { ...loadLegacyUiPrefs(), ...(data.ui ?? {}) };
        return {
            tabs: data.tabs as WorkspaceTab[],
            tabOrder: data.tabOrder as string[],
            activeTabId: typeof data.activeTabId === 'string' ? data.activeTabId : null,
            ui,
            savedQueriesClassic: Array.isArray(data.savedQueriesClassic)
                ? (data.savedQueriesClassic as SavedQueryClassic[])
                : [],
            savedQueriesVersioned: Array.isArray(data.savedQueriesVersioned)
                ? (data.savedQueriesVersioned as SavedQueryVersioned[])
                : [],
        };
    } catch {
        return null;
    }
}

export function loadWorkspaceSession(): WorkspaceSession {
    try {
        const raw = localStorage.getItem(SESSION_KEY);
        if (raw) {
            const session = parseSession(raw);
            if (session) return session;
        }
    } catch {
        /* ignore */
    }
    return {
        tabs: [],
        tabOrder: [],
        activeTabId: null,
        ui: loadLegacyUiPrefs(),
        savedQueriesClassic: [],
        savedQueriesVersioned: [],
    };
}

function saveWorkspaceSession(session: WorkspaceSession) {
    try {
        localStorage.setItem(SESSION_KEY, JSON.stringify(session));
        localStorage.setItem(LEGACY_UI_KEY, JSON.stringify(session.ui));
    } catch {
        /* ignore */
    }
}

function orderedTabs(tabs: WorkspaceTab[], tabOrder: string[]): WorkspaceTab[] {
    const byId = new Map(tabs.map((t) => [t.id, t]));
    const ordered = tabOrder.map((id) => byId.get(id)).filter(Boolean) as WorkspaceTab[];
    for (const tab of tabs) {
        if (!tabOrder.includes(tab.id)) ordered.push(tab);
    }
    return ordered;
}

function workspaceReducer(state: WorkspaceSession, action: WorkspaceAction): WorkspaceSession {
    switch (action.type) {
        case 'HYDRATE':
            return action.session;

        case 'OPEN_QUERY_TAB': {
            const exists = state.tabs.find((t) => t.id === action.tab.id);
            if (exists) {
                return {
                    ...state,
                    activeTabId: action.activate === false ? state.activeTabId : action.tab.id,
                };
            }
            return {
                ...state,
                tabs: [...state.tabs, action.tab],
                tabOrder: [...state.tabOrder, action.tab.id],
                activeTabId: action.activate === false ? state.activeTabId : action.tab.id,
            };
        }

        case 'OPEN_DATASET_TAB': {
            const id = resourceTabId('dataset', action.projectId, action.datasetId);
            const existing = state.tabs.find((t) => t.id === id);
            if (existing) {
                return {
                    ...state,
                    activeTabId: action.activate === false ? state.activeTabId : id,
                };
            }
            const tab: DatasetTabState = {
                type: 'dataset',
                id,
                projectId: action.projectId,
                datasetId: action.datasetId,
            };
            return {
                ...state,
                tabs: [...state.tabs, tab],
                tabOrder: [...state.tabOrder, id],
                activeTabId: action.activate === false ? state.activeTabId : id,
            };
        }

        case 'OPEN_TABLE_TAB': {
            const id = resourceTabId('table', action.projectId, action.datasetId, action.tableId);
            const existing = state.tabs.find((t) => t.id === id);
            if (existing) {
                return {
                    ...state,
                    activeTabId: action.activate === false ? state.activeTabId : id,
                };
            }
            const tab: TableTabState = {
                type: 'table',
                id,
                projectId: action.projectId,
                datasetId: action.datasetId,
                tableId: action.tableId,
            };
            return {
                ...state,
                tabs: [...state.tabs, tab],
                tabOrder: [...state.tabOrder, id],
                activeTabId: action.activate === false ? state.activeTabId : id,
            };
        }

        case 'ACTIVATE_TAB':
            if (!state.tabs.some((t) => t.id === action.id)) return state;
            return { ...state, activeTabId: action.id };

        case 'CLOSE_TAB': {
            if (!state.tabs.some((t) => t.id === action.id)) return state;
            const tabs = state.tabs.filter((t) => t.id !== action.id);
            const tabOrder = state.tabOrder.filter((id) => id !== action.id);
            let activeTabId = state.activeTabId;
            if (activeTabId === action.id) {
                const idx = state.tabOrder.indexOf(action.id);
                activeTabId = tabOrder[idx] ?? tabOrder[idx - 1] ?? tabOrder[0] ?? null;
            }
            return { ...state, tabs, tabOrder, activeTabId };
        }

        case 'REORDER_TAB': {
            const fromIndex = state.tabOrder.indexOf(action.id);
            if (fromIndex < 0) return state;
            const tabOrder = [...state.tabOrder];
            tabOrder.splice(fromIndex, 1);
            tabOrder.splice(Math.max(0, Math.min(action.toIndex, tabOrder.length)), 0, action.id);
            return { ...state, tabOrder };
        }

        case 'RENAME_TAB':
            return {
                ...state,
                tabs: state.tabs.map((t) =>
                    t.id === action.id && t.type === 'query' ? { ...t, title: action.title } : t,
                ),
            };

        case 'UPDATE_QUERY_TAB':
            return {
                ...state,
                tabs: state.tabs.map((t) =>
                    t.id === action.id && t.type === 'query' ? { ...t, ...action.patch } : t,
                ),
            };

        case 'UPDATE_UI':
            return { ...state, ui: { ...state.ui, ...action.patch } };

        case 'SAVE_QUERY_CLASSIC':
            return {
                ...state,
                savedQueriesClassic: [
                    action.entry,
                    ...state.savedQueriesClassic.filter((q) => q.id !== action.entry.id),
                ],
            };

        case 'SAVE_QUERY_VERSIONED': {
            const now = new Date().toISOString();
            const existing = state.savedQueriesVersioned.find((q) => q.id === action.entry.id);
            const versions = existing
                ? [{ sql: action.sql, savedAt: now }, ...existing.versions]
                : [{ sql: action.sql, savedAt: now }];
            const entry: SavedQueryVersioned = { ...action.entry, versions };
            return {
                ...state,
                savedQueriesVersioned: [
                    entry,
                    ...state.savedQueriesVersioned.filter((q) => q.id !== entry.id),
                ],
            };
        }

        default:
            return state;
    }
}

export interface WorkspaceContextValue {
    session: WorkspaceSession;
    tabs: WorkspaceTab[];
    activeTab: WorkspaceTab | null;
    ui: UiPrefs;
    dispatch: Dispatch<WorkspaceAction>;
    openBlankQuery: (projectId?: string) => string;
    openQueryForTable: (projectId: string, datasetId: string, tableId: string, sql?: string) => string;
    openDatasetTab: (projectId: string, datasetId: string) => void;
    openTableTab: (projectId: string, datasetId: string, tableId: string) => void;
    activateTab: (id: string) => void;
    closeTab: (id: string) => void;
    renameTab: (id: string, title: string) => void;
    reorderTab: (id: string, toIndex: number) => void;
    updateQueryTab: (id: string, patch: Partial<Omit<QueryTabState, 'type' | 'id'>>) => void;
    updateUi: (patch: Partial<UiPrefs>) => void;
    saveQueryClassic: (opts: {
        title: string;
        sql: string;
        projectId: string;
        datasetId?: string;
        tableId?: string;
    }) => SavedQueryClassic;
    saveQueryVersioned: (opts: {
        title: string;
        sql: string;
        projectId: string;
        datasetId?: string;
        tableId?: string;
    }) => SavedQueryVersioned;
    openQueryFromShare: (opts: {
        projectId: string;
        datasetId: string;
        tableId: string;
        sql: string;
        subTab: QuerySubTab;
        queryResult?: QueryResponse | null;
    }) => string;
}

const WorkspaceContext = createContext<WorkspaceContextValue | null>(null);

export function WorkspaceProvider({ children }: { children: ReactNode }) {
    const [session, dispatch] = useReducer(workspaceReducer, undefined, loadWorkspaceSession);

    useEffect(() => {
        saveWorkspaceSession(session);
    }, [session]);

    const tabs = useMemo(() => orderedTabs(session.tabs, session.tabOrder), [session.tabs, session.tabOrder]);

    const activeTab = useMemo(
        () => tabs.find((t) => t.id === session.activeTabId) ?? null,
        [tabs, session.activeTabId],
    );

    const openBlankQuery = useCallback((projectId = '') => {
        const id = newQueryTabId();
        const tab: QueryTabState = {
            type: 'query',
            id,
            title: 'Untitled query',
            sql: '',
            subTab: 'results',
            projectId,
        };
        dispatch({ type: 'OPEN_QUERY_TAB', tab });
        return id;
    }, []);

    const openQueryForTable = useCallback(
        (projectId: string, datasetId: string, tableId: string, sql?: string) => {
            const id = newQueryTabId();
            const q = sql?.trim() ? sql : defaultSql(projectId, datasetId, tableId);
            const tab: QueryTabState = {
                type: 'query',
                id,
                title: tableId,
                sql: q,
                subTab: 'results',
                projectId,
                datasetId,
                tableId,
            };
            dispatch({ type: 'OPEN_QUERY_TAB', tab });
            return id;
        },
        [],
    );

    const openQueryFromShare = useCallback(
        (opts: {
            projectId: string;
            datasetId: string;
            tableId: string;
            sql: string;
            subTab: QuerySubTab;
            queryResult?: QueryResponse | null;
        }) => {
            const existing = session.tabs.find(
                (t) =>
                    t.type === 'query' &&
                    t.projectId === opts.projectId &&
                    t.datasetId === opts.datasetId &&
                    t.tableId === opts.tableId &&
                    t.sql === opts.sql,
            );
            if (existing && existing.type === 'query') {
                dispatch({
                    type: 'UPDATE_QUERY_TAB',
                    id: existing.id,
                    patch: { subTab: opts.subTab, queryResult: opts.queryResult ?? existing.queryResult },
                });
                dispatch({ type: 'ACTIVATE_TAB', id: existing.id });
                return existing.id;
            }
            const id = newQueryTabId();
            const tab: QueryTabState = {
                type: 'query',
                id,
                title: opts.tableId,
                sql: opts.sql,
                subTab: opts.subTab,
                projectId: opts.projectId,
                datasetId: opts.datasetId,
                tableId: opts.tableId,
                queryResult: opts.queryResult ?? null,
            };
            dispatch({ type: 'OPEN_QUERY_TAB', tab });
            return id;
        },
        [session.tabs],
    );

    const openDatasetTab = useCallback((projectId: string, datasetId: string) => {
        dispatch({ type: 'OPEN_DATASET_TAB', projectId, datasetId });
    }, []);

    const openTableTab = useCallback((projectId: string, datasetId: string, tableId: string) => {
        dispatch({ type: 'OPEN_TABLE_TAB', projectId, datasetId, tableId });
    }, []);

    const activateTab = useCallback((id: string) => {
        dispatch({ type: 'ACTIVATE_TAB', id });
    }, []);

    const closeTab = useCallback((id: string) => {
        dispatch({ type: 'CLOSE_TAB', id });
    }, []);

    const renameTab = useCallback((id: string, title: string) => {
        dispatch({ type: 'RENAME_TAB', id, title });
    }, []);

    const reorderTab = useCallback((id: string, toIndex: number) => {
        dispatch({ type: 'REORDER_TAB', id, toIndex });
    }, []);

    const updateQueryTab = useCallback((id: string, patch: Partial<Omit<QueryTabState, 'type' | 'id'>>) => {
        dispatch({ type: 'UPDATE_QUERY_TAB', id, patch });
    }, []);

    const updateUi = useCallback((patch: Partial<UiPrefs>) => {
        dispatch({ type: 'UPDATE_UI', patch });
    }, []);

    const saveQueryClassic = useCallback(
        (opts: {
            title: string;
            sql: string;
            projectId: string;
            datasetId?: string;
            tableId?: string;
        }) => {
            const entry: SavedQueryClassic = {
                id: newSavedQueryId(),
                title: opts.title,
                sql: opts.sql,
                projectId: opts.projectId,
                datasetId: opts.datasetId,
                tableId: opts.tableId,
                savedAt: new Date().toISOString(),
            };
            dispatch({ type: 'SAVE_QUERY_CLASSIC', entry });
            return entry;
        },
        [],
    );

    const saveQueryVersioned = useCallback(
        (opts: {
            title: string;
            sql: string;
            projectId: string;
            datasetId?: string;
            tableId?: string;
        }) => {
            const existing = session.savedQueriesVersioned.find(
                (q) => q.title === opts.title && q.projectId === opts.projectId,
            );
            const entry: SavedQueryVersioned = existing ?? {
                id: newSavedQueryId(),
                title: opts.title,
                projectId: opts.projectId,
                datasetId: opts.datasetId,
                tableId: opts.tableId,
                versions: [],
            };
            dispatch({ type: 'SAVE_QUERY_VERSIONED', entry, sql: opts.sql });
            return { ...entry, versions: [{ sql: opts.sql, savedAt: new Date().toISOString() }, ...entry.versions] };
        },
        [session.savedQueriesVersioned],
    );

    const value = useMemo<WorkspaceContextValue>(
        () => ({
            session,
            tabs,
            activeTab,
            ui: session.ui,
            dispatch,
            openBlankQuery,
            openQueryForTable,
            openDatasetTab,
            openTableTab,
            activateTab,
            closeTab,
            renameTab,
            reorderTab,
            updateQueryTab,
            updateUi,
            saveQueryClassic,
            saveQueryVersioned,
            openQueryFromShare,
        }),
        [
            session,
            tabs,
            activeTab,
            openBlankQuery,
            openQueryForTable,
            openDatasetTab,
            openTableTab,
            activateTab,
            closeTab,
            renameTab,
            reorderTab,
            updateQueryTab,
            updateUi,
            saveQueryClassic,
            saveQueryVersioned,
            openQueryFromShare,
        ],
    );

    return <WorkspaceContext.Provider value={value}>{children}</WorkspaceContext.Provider>;
}

export function useWorkspace(): WorkspaceContextValue {
    const ctx = useContext(WorkspaceContext);
    if (!ctx) throw new Error('useWorkspace must be used within WorkspaceProvider');
    return ctx;
}

export function tabRoutePath(tab: WorkspaceTab): string {
    switch (tab.type) {
        case 'query':
            return `/query/${encodeURIComponent(tab.id)}`;
        case 'dataset':
            return `/project/${encodeURIComponent(tab.projectId)}/dataset/${encodeURIComponent(tab.datasetId)}`;
        case 'table':
            return `/project/${encodeURIComponent(tab.projectId)}/dataset/${encodeURIComponent(tab.datasetId)}/table/${encodeURIComponent(tab.tableId)}`;
    }
}

export function findTabForRoute(
    tabs: WorkspaceTab[],
    params: { tabId?: string; projectId?: string; datasetId?: string; tableId?: string },
): WorkspaceTab | null {
    if (params.tabId) {
        return tabs.find((t) => t.type === 'query' && t.id === params.tabId) ?? null;
    }
    if (params.projectId && params.datasetId && params.tableId) {
        const id = resourceTabId('table', params.projectId, params.datasetId, params.tableId);
        return tabs.find((t) => t.id === id) ?? null;
    }
    if (params.projectId && params.datasetId) {
        const id = resourceTabId('dataset', params.projectId, params.datasetId);
        return tabs.find((t) => t.id === id) ?? null;
    }
    return null;
}
