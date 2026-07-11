import type { SplitPaneSide, SplitPaneGroup, WorkspaceSession, WorkspaceSplit, WorkspaceTab } from './types';

export function getPaneGroup(split: WorkspaceSplit, side: SplitPaneSide): SplitPaneGroup {
    return side === 'left' ? split.left : split.right;
}

export function setPaneGroup(
    split: WorkspaceSplit,
    side: SplitPaneSide,
    group: SplitPaneGroup,
): WorkspaceSplit {
    return side === 'left' ? { ...split, left: group } : { ...split, right: group };
}

export function findPaneForTab(split: WorkspaceSplit, tabId: string): SplitPaneSide | null {
    if (split.left.tabOrder.includes(tabId)) return 'left';
    if (split.right.tabOrder.includes(tabId)) return 'right';
    return null;
}

export function orderedPaneTabs(allTabs: WorkspaceTab[], tabOrder: string[]): WorkspaceTab[] {
    const byId = new Map(allTabs.map((t) => [t.id, t]));
    return tabOrder.map((id) => byId.get(id)).filter(Boolean) as WorkspaceTab[];
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

function nextActiveInGroup(tabOrder: string[], closingId: string): string | null {
    const remaining = tabOrder.filter((id) => id !== closingId);
    if (remaining.length === 0) return null;
    const idx = tabOrder.indexOf(closingId);
    return remaining[Math.min(idx, remaining.length - 1)] ?? remaining[0] ?? null;
}

function isLegacySplit(split: unknown): split is {
    secondaryTabId: string;
    primarySide: SplitPaneSide;
    ratio: number;
} {
    if (!split || typeof split !== 'object') return false;
    return 'secondaryTabId' in split && !('left' in split);
}

export function migrateSplit(session: WorkspaceSession): WorkspaceSplit | null {
    const { split, activeTabId, tabOrder } = session;
    if (!split) return null;

    if (!isLegacySplit(split)) {
        return sanitizeSplit(session);
    }

    if (!activeTabId || !split.secondaryTabId) return null;

    const others = tabOrder.filter((id) => id !== activeTabId);
    const focusedGroup: SplitPaneGroup = {
        tabOrder: [activeTabId],
        activeTabId,
    };
    const otherGroup: SplitPaneGroup = {
        tabOrder: others.length > 0 ? others : [split.secondaryTabId],
        activeTabId: split.secondaryTabId,
    };

    const migrated: WorkspaceSplit = {
        left: split.primarySide === 'left' ? focusedGroup : otherGroup,
        right: split.primarySide === 'right' ? focusedGroup : otherGroup,
        focusedSide: split.primarySide,
        ratio: split.ratio ?? 0.5,
    };

    return sanitizeSplit({ ...session, split: migrated });
}

export function sanitizeSplit(session: WorkspaceSession): WorkspaceSplit | null {
    const { split, tabs } = session;
    if (!split) return null;

    const openIds = new Set(tabs.map((t) => t.id));
    const sanitizeGroup = (group: SplitPaneGroup): SplitPaneGroup | null => {
        const tabOrder = group.tabOrder.filter((id) => openIds.has(id));
        if (tabOrder.length === 0) return null;
        const activeTabId =
            group.activeTabId && tabOrder.includes(group.activeTabId)
                ? group.activeTabId
                : tabOrder[0]!;
        return { tabOrder, activeTabId };
    };

    const left = sanitizeGroup(split.left);
    const right = sanitizeGroup(split.right);
    if (!left || !right) return null;

    const focusedSide = split.focusedSide === 'right' ? 'right' : 'left';
    const focusedActive = focusedSide === 'left' ? left.activeTabId : right.activeTabId;

    if (!focusedActive) return null;

    return {
        left,
        right,
        focusedSide,
        ratio: split.ratio ?? 0.5,
    };
}

export function collapseSplitIfNeeded(state: WorkspaceSession): WorkspaceSession {
    if (!state.split) return state;

    const leftCount = state.split.left.tabOrder.length;
    const rightCount = state.split.right.tabOrder.length;

    if (leftCount > 0 && rightCount > 0) {
        const focusedActive =
            state.split.focusedSide === 'left'
                ? state.split.left.activeTabId
                : state.split.right.activeTabId;
        return { ...state, activeTabId: focusedActive ?? state.activeTabId };
    }

    const surviving = leftCount > 0 ? state.split.left : state.split.right;
    return {
        ...state,
        tabOrder: surviving.tabOrder,
        activeTabId: surviving.activeTabId,
        split: null,
    };
}

export function mergeSplitToGlobal(state: WorkspaceSession): WorkspaceSession {
    if (!state.split) return state;
    const tabOrder = [...state.split.left.tabOrder, ...state.split.right.tabOrder];
    const focusedActive =
        state.split.focusedSide === 'left'
            ? state.split.left.activeTabId
            : state.split.right.activeTabId;
    return {
        ...state,
        tabOrder,
        activeTabId: focusedActive ?? state.activeTabId,
        split: null,
    };
}

export function activateTabInSplit(state: WorkspaceSession, tabId: string): WorkspaceSession {
    if (!state.split) {
        return { ...state, activeTabId: tabId };
    }

    const pane = findPaneForTab(state.split, tabId);
    if (!pane) {
        const side = state.split.focusedSide;
        const group = getPaneGroup(state.split, side);
        const tabOrder = group.tabOrder.includes(tabId) ? group.tabOrder : [...group.tabOrder, tabId];
        const split = setPaneGroup(state.split, side, { tabOrder, activeTabId: tabId });
        return { ...state, activeTabId: tabId, split };
    }

    const group = getPaneGroup(state.split, pane);
    const split = setPaneGroup(state.split, pane, { ...group, activeTabId: tabId });
    return { ...state, activeTabId: tabId, split: { ...split, focusedSide: pane } };
}

export function closeTabInSplit(state: WorkspaceSession, tabId: string): WorkspaceSession {
    if (!state.split) return state;

    const pane = findPaneForTab(state.split, tabId);
    if (!pane) return state;

    const group = getPaneGroup(state.split, pane);
    const tabOrder = group.tabOrder.filter((id) => id !== tabId);
    const activeTabId =
        group.activeTabId === tabId ? nextActiveInGroup(group.tabOrder, tabId) : group.activeTabId;

    let split = setPaneGroup(state.split, pane, { tabOrder, activeTabId });
    const tabs = state.tabs.filter((t) => t.id !== tabId);

    let next: WorkspaceSession = {
        ...state,
        tabs,
        split,
        activeTabId:
            state.activeTabId === tabId
                ? activeTabId ?? split[pane === 'left' ? 'right' : 'left'].activeTabId
                : state.activeTabId,
    };

    next = collapseSplitIfNeeded(next);
    if (next.split) {
        next = {
            ...next,
            activeTabId:
                next.split.focusedSide === 'left'
                    ? next.split.left.activeTabId
                    : next.split.right.activeTabId,
        };
    } else {
        next = {
            ...next,
            tabOrder: next.tabOrder.filter((id) => id !== tabId),
            activeTabId:
                next.activeTabId === tabId
                    ? nextActiveInGroup(state.tabOrder, tabId)
                    : next.activeTabId,
        };
    }

    return next;
}

export { nextActiveInGroup };
