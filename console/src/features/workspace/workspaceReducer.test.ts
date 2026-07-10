import { beforeEach, describe, expect, it, vi } from 'vitest';

import { loadWorkspaceSession, workspaceReducer } from './store';
import { SESSION_KEY, type QueryTabState, type WorkspaceSession } from './types';

function createStorage(): Storage {
    const data = new Map<string, string>();
    return {
        get length() {
            return data.size;
        },
        clear() {
            data.clear();
        },
        getItem(key: string) {
            return data.get(key) ?? null;
        },
        key(index: number) {
            return [...data.keys()][index] ?? null;
        },
        removeItem(key: string) {
            data.delete(key);
        },
        setItem(key: string, value: string) {
            data.set(key, value);
        },
    };
}

function queryTab(id: string, title = id): QueryTabState {
    return {
        type: 'query',
        id,
        title,
        sql: '',
        subTab: 'results',
        projectId: 'p',
    };
}

function baseSession(overrides: Partial<WorkspaceSession> = {}): WorkspaceSession {
    return {
        tabs: [queryTab('q1'), queryTab('q2')],
        tabOrder: ['q1', 'q2'],
        activeTabId: 'q1',
        split: null,
        ui: loadWorkspaceSession().ui,
        savedQueriesClassic: [],
        savedQueriesVersioned: [],
        ...overrides,
    };
}

describe('workspaceReducer split and close actions', () => {
    it('closeOtherTabs keeps one tab and clears split', () => {
        const state = baseSession({
            split: { secondaryTabId: 'q2', primarySide: 'left', ratio: 0.5 },
        });
        const next = workspaceReducer(state, { type: 'CLOSE_OTHER_TABS', keepId: 'q2' });
        expect(next.tabs).toHaveLength(1);
        expect(next.tabs[0]?.id).toBe('q2');
        expect(next.activeTabId).toBe('q2');
        expect(next.split).toBeNull();
    });

    it('splitTab to left puts tab on left with previous active as secondary', () => {
        const state = baseSession();
        const next = workspaceReducer(state, { type: 'SPLIT_TAB', tabId: 'q2', side: 'left' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split).toEqual({
            secondaryTabId: 'q1',
            primarySide: 'left',
            ratio: 0.5,
        });
    });

    it('splitTab to right puts tab on right pane as focused', () => {
        const state = baseSession();
        const next = workspaceReducer(state, { type: 'SPLIT_TAB', tabId: 'q2', side: 'right' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split?.primarySide).toBe('right');
        expect(next.split?.secondaryTabId).toBe('q1');
    });

    it('closing focused tab in split collapses to secondary', () => {
        const state = baseSession({
            activeTabId: 'q1',
            split: { secondaryTabId: 'q2', primarySide: 'left', ratio: 0.5 },
        });
        const next = workspaceReducer(state, { type: 'CLOSE_TAB', id: 'q1' });
        expect(next.split).toBeNull();
        expect(next.activeTabId).toBe('q2');
        expect(next.tabs).toHaveLength(1);
    });

    it('closing secondary tab clears split but keeps focused tab', () => {
        const state = baseSession({
            activeTabId: 'q1',
            split: { secondaryTabId: 'q2', primarySide: 'left', ratio: 0.5 },
        });
        const next = workspaceReducer(state, { type: 'CLOSE_TAB', id: 'q2' });
        expect(next.split).toBeNull();
        expect(next.activeTabId).toBe('q1');
    });

    it('assigns clicked tab to focused pane while split', () => {
        const state = baseSession({
            activeTabId: 'q1',
            split: { secondaryTabId: 'q2', primarySide: 'left', ratio: 0.5 },
        });
        const next = workspaceReducer(state, { type: 'ACTIVATE_TAB', id: 'q2' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split?.secondaryTabId).toBe('q1');
    });

    it('focusPane swaps pane assignments', () => {
        const state = baseSession({
            activeTabId: 'q1',
            split: { secondaryTabId: 'q2', primarySide: 'left', ratio: 0.5 },
        });
        const next = workspaceReducer(state, { type: 'FOCUS_PANE', side: 'right' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split).toEqual({
            secondaryTabId: 'q1',
            primarySide: 'right',
            ratio: 0.5,
        });
    });
});

describe('workspace session split hydration', () => {
    beforeEach(() => {
        vi.stubGlobal('localStorage', createStorage());
    });

    it('clears split when secondary tab is missing', () => {
        localStorage.setItem(
            SESSION_KEY,
            JSON.stringify({
                tabs: [queryTab('q1')],
                tabOrder: ['q1'],
                activeTabId: 'q1',
                split: { secondaryTabId: 'missing', primarySide: 'left', ratio: 0.5 },
            }),
        );
        const session = loadWorkspaceSession();
        expect(session.split).toBeNull();
    });
});
