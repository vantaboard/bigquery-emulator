import type { SplitPaneSide, WorkspaceSession, WorkspaceSplit } from './types';

export function paneTabIds(
    split: WorkspaceSplit | null,
    activeTabId: string | null,
): { leftTabId: string | null; rightTabId: string | null; focusedSide: SplitPaneSide } {
    if (!split || !activeTabId) {
        return { leftTabId: activeTabId, rightTabId: null, focusedSide: 'left' };
    }
    if (split.primarySide === 'left') {
        return {
            leftTabId: activeTabId,
            rightTabId: split.secondaryTabId,
            focusedSide: 'left',
        };
    }
    return {
        leftTabId: split.secondaryTabId,
        rightTabId: activeTabId,
        focusedSide: 'right',
    };
}

export function pickSecondaryTabId(state: WorkspaceSession, excludeId: string): string | null {
    const openIds = new Set(state.tabs.map((t) => t.id));
    const candidates = state.tabOrder.filter((id) => id !== excludeId && openIds.has(id));
    if (candidates.length === 0) return null;

    if (
        state.activeTabId &&
        state.activeTabId !== excludeId &&
        candidates.includes(state.activeTabId)
    ) {
        return state.activeTabId;
    }

    const excludeIdx = state.tabOrder.indexOf(excludeId);
    if (excludeIdx >= 0) {
        for (let i = excludeIdx - 1; i >= 0; i -= 1) {
            const id = state.tabOrder[i];
            if (candidates.includes(id)) return id;
        }
        for (let i = excludeIdx + 1; i < state.tabOrder.length; i += 1) {
            const id = state.tabOrder[i];
            if (candidates.includes(id)) return id;
        }
    }

    return candidates[0] ?? null;
}

export function sanitizeSplit(session: WorkspaceSession): WorkspaceSplit | null {
    const { split, activeTabId, tabs } = session;
    if (!split || !activeTabId) return null;
    const openIds = new Set(tabs.map((t) => t.id));
    if (!openIds.has(activeTabId) || !openIds.has(split.secondaryTabId)) return null;
    if (activeTabId === split.secondaryTabId) return null;
    return split;
}

export function focusedTab(session: WorkspaceSession): string | null {
    return session.activeTabId;
}
