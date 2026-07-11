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

function splitSession(
    left: { tabOrder: string[]; activeTabId: string | null },
    right: { tabOrder: string[]; activeTabId: string | null },
    focusedSide: 'left' | 'right' = 'left',
): WorkspaceSession {
    return baseSession({
        activeTabId: focusedSide === 'left' ? left.activeTabId : right.activeTabId,
        split: { left, right, focusedSide, ratio: 0.5 },
    });
}

function baseSession(overrides: Partial<WorkspaceSession> = {}): WorkspaceSession {
    return {
        tabs: [queryTab('q1'), queryTab('q2'), queryTab('q3')],
        tabOrder: ['q1', 'q2', 'q3'],
        activeTabId: 'q1',
        split: null,
        ui: loadWorkspaceSession().ui,
        savedQueriesClassic: [],
        savedQueriesVersioned: [],
        ...overrides,
    };
}

describe('workspaceReducer split and close actions', () => {
    it('closeOtherTabs in a pane with multiple tabs keeps only the selected tab in that pane', () => {
        const state = splitSession(
            { tabOrder: ['q1'], activeTabId: 'q1' },
            { tabOrder: ['q2', 'q3'], activeTabId: 'q2' },
        );
        const next = workspaceReducer(state, { type: 'CLOSE_OTHER_TABS', keepId: 'q2' });
        expect(next.tabs.map((t) => t.id)).toEqual(['q1', 'q2']);
        expect(next.split?.left.tabOrder).toEqual(['q1']);
        expect(next.split?.right.tabOrder).toEqual(['q2']);
        expect(next.activeTabId).toBe('q2');
    });

    it('closeOtherTabs only affects the pane containing the kept tab', () => {
        const state = splitSession(
            { tabOrder: ['q1', 'q3'], activeTabId: 'q1' },
            { tabOrder: ['q2'], activeTabId: 'q2' },
        );
        const next = workspaceReducer(state, { type: 'CLOSE_OTHER_TABS', keepId: 'q1' });
        expect(next.split?.left.tabOrder).toEqual(['q1']);
        expect(next.split?.right.tabOrder).toEqual(['q2']);
        expect(next.tabs.map((t) => t.id)).toEqual(['q1', 'q2']);
    });

    it('splitTab to left creates separate pane tab groups', () => {
        const state = baseSession();
        const next = workspaceReducer(state, { type: 'SPLIT_TAB', tabId: 'q2', side: 'left' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split?.left).toEqual({ tabOrder: ['q2'], activeTabId: 'q2' });
        expect(next.split?.right.tabOrder).toEqual(['q1', 'q3']);
        expect(next.split?.focusedSide).toBe('left');
    });

    it('splitTab to right focuses the right pane group', () => {
        const state = baseSession();
        const next = workspaceReducer(state, { type: 'SPLIT_TAB', tabId: 'q2', side: 'right' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split?.right).toEqual({ tabOrder: ['q2'], activeTabId: 'q2' });
        expect(next.split?.focusedSide).toBe('right');
    });

    it('closing the last tab in a pane collapses split to the other pane', () => {
        const state = splitSession(
            { tabOrder: ['q1'], activeTabId: 'q1' },
            { tabOrder: ['q2'], activeTabId: 'q2' },
        );
        const next = workspaceReducer(state, { type: 'CLOSE_TAB', id: 'q2' });
        expect(next.split).toBeNull();
        expect(next.activeTabId).toBe('q1');
        expect(next.tabOrder).toEqual(['q1']);
    });

    it('activates tab within its pane group while split', () => {
        const state = splitSession(
            { tabOrder: ['q1', 'q3'], activeTabId: 'q1' },
            { tabOrder: ['q2'], activeTabId: 'q2' },
            'right',
        );
        const next = workspaceReducer(state, { type: 'ACTIVATE_TAB', id: 'q3' });
        expect(next.activeTabId).toBe('q3');
        expect(next.split?.left.activeTabId).toBe('q3');
        expect(next.split?.focusedSide).toBe('left');
    });

    it('focusPane switches focused side without moving tabs between groups', () => {
        const state = splitSession(
            { tabOrder: ['q1'], activeTabId: 'q1' },
            { tabOrder: ['q2'], activeTabId: 'q2' },
            'left',
        );
        const next = workspaceReducer(state, { type: 'FOCUS_PANE', side: 'right' });
        expect(next.activeTabId).toBe('q2');
        expect(next.split?.focusedSide).toBe('right');
        expect(next.split?.left.tabOrder).toEqual(['q1']);
        expect(next.split?.right.tabOrder).toEqual(['q2']);
    });
});

describe('workspace session split hydration', () => {
    beforeEach(() => {
        vi.stubGlobal('localStorage', createStorage());
    });

    it('migrates legacy split format into pane tab groups', () => {
        localStorage.setItem(
            SESSION_KEY,
            JSON.stringify({
                tabs: [queryTab('q1'), queryTab('q2')],
                tabOrder: ['q1', 'q2'],
                activeTabId: 'q2',
                split: { secondaryTabId: 'q1', primarySide: 'right', ratio: 0.5 },
            }),
        );
        const session = loadWorkspaceSession();
        expect(session.split?.left.tabOrder).toEqual(['q1']);
        expect(session.split?.right.tabOrder).toEqual(['q2']);
        expect(session.split?.focusedSide).toBe('right');
    });

    it('clears split when a pane group is empty after hydration', () => {
        localStorage.setItem(
            SESSION_KEY,
            JSON.stringify({
                tabs: [queryTab('q1')],
                tabOrder: ['q1'],
                activeTabId: 'q1',
                split: {
                    left: { tabOrder: ['q1'], activeTabId: 'q1' },
                    right: { tabOrder: ['missing'], activeTabId: 'missing' },
                    focusedSide: 'left',
                    ratio: 0.5,
                },
            }),
        );
        const session = loadWorkspaceSession();
        expect(session.split).toBeNull();
    });
});
