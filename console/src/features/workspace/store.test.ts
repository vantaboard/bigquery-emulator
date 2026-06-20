import { beforeEach, describe, expect, it, vi } from 'vitest';

import { loadWorkspaceSession } from './store';
import { LEGACY_UI_KEY, SESSION_KEY, UI_DEFAULT } from './types';

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

describe('workspace session persistence', () => {
    beforeEach(() => {
        vi.stubGlobal('localStorage', createStorage());
    });

    it('loads defaults when storage is empty', () => {
        const session = loadWorkspaceSession();
        expect(session.tabs).toEqual([]);
        expect(session.tabOrder).toEqual([]);
        expect(session.activeTabId).toBeNull();
        expect(session.ui.sidebarWidth).toBe(UI_DEFAULT.sidebarWidth);
        expect(session.ui.useEmulatorParser).toBe(true);
        expect(session.savedQueriesClassic).toEqual([]);
        expect(session.savedQueriesVersioned).toEqual([]);
    });

    it('hydrates saved session from bigqueryWorkspaceSession', () => {
        localStorage.setItem(
            SESSION_KEY,
            JSON.stringify({
                tabs: [
                    {
                        type: 'query',
                        id: 'q1',
                        title: 'Untitled query',
                        sql: 'SELECT 1',
                        subTab: 'results',
                        projectId: 'p',
                    },
                ],
                tabOrder: ['q1'],
                activeTabId: 'q1',
                ui: { sidebarWidth: 400, editorHeight: 280, sidebarCollapsed: false, editorCollapsed: false },
            }),
        );
        const session = loadWorkspaceSession();
        expect(session.tabs).toHaveLength(1);
        expect(session.activeTabId).toBe('q1');
        expect(session.ui.sidebarWidth).toBe(400);
    });

    it('migrates legacy bigqueryExplorerUILayout prefs when session is missing', () => {
        localStorage.setItem(
            LEGACY_UI_KEY,
            JSON.stringify({ sidebarWidth: 280, editorHeight: 200, sidebarCollapsed: true, editorCollapsed: true }),
        );
        const session = loadWorkspaceSession();
        expect(session.ui.sidebarWidth).toBe(280);
        expect(session.ui.sidebarCollapsed).toBe(true);
    });
});
