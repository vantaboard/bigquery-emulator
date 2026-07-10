import { useCallback, useRef, type PointerEvent } from 'react';

import { cn } from '@/lib/utils';

import { paneTabIds } from '@/features/workspace/splitUtils';
import { useWorkspace } from '@/features/workspace/store';
import type { SplitPaneSide, WorkspaceTab } from '@/features/workspace/types';

import { WorkspaceTabContent } from './WorkspaceTabContent';

function tabById(tabs: WorkspaceTab[], id: string | null): WorkspaceTab | null {
    if (!id) return null;
    return tabs.find((t) => t.id === id) ?? null;
}

export function WorkspaceSplitPanes() {
    const { session, tabs, focusPane, setSplitRatio } = useWorkspace();
    const dragRef = useRef<{ startX: number; startRatio: number } | null>(null);

    const { leftTabId, rightTabId, focusedSide } = paneTabIds(session.split, session.activeTabId);
    const leftTab = tabById(tabs, leftTabId);
    const rightTab = tabById(tabs, rightTabId);
    const ratio = session.split?.ratio ?? 0.5;

    const onDividerPointerDown = useCallback(
        (event: PointerEvent<HTMLDivElement>) => {
            if (!session.split) return;
            event.preventDefault();
            dragRef.current = { startX: event.clientX, startRatio: ratio };
            event.currentTarget.setPointerCapture(event.pointerId);
        },
        [ratio, session.split],
    );

    const onDividerPointerMove = useCallback(
        (event: PointerEvent<HTMLDivElement>) => {
            const drag = dragRef.current;
            if (!drag) return;
            const container = event.currentTarget.parentElement;
            if (!container) return;
            const width = container.clientWidth;
            if (width <= 0) return;
            const delta = (event.clientX - drag.startX) / width;
            setSplitRatio(drag.startRatio + delta);
        },
        [setSplitRatio],
    );

    const onDividerPointerUp = useCallback((event: PointerEvent<HTMLDivElement>) => {
        dragRef.current = null;
        event.currentTarget.releasePointerCapture(event.pointerId);
    }, []);

    const renderPane = (tab: WorkspaceTab | null, side: SplitPaneSide) => {
        const focused = focusedSide === side;
        return (
            <div
                data-testid={side === 'left' ? 'workspace-pane-left' : 'workspace-pane-right'}
                className={cn(
                    'flex min-h-0 min-w-0 flex-1 flex-col overflow-hidden',
                    focused ? 'ring-1 ring-inset ring-blue-500/60' : 'opacity-95',
                )}
                onMouseDown={() => focusPane(side)}
            >
                {tab ? (
                    <WorkspaceTabContent tab={tab} />
                ) : (
                    <div className="flex flex-1 items-center justify-center text-sm text-[var(--bq-muted)]">
                        No tab
                    </div>
                )}
            </div>
        );
    };

    return (
        <div className="flex min-h-0 flex-1">
            <div className="flex min-h-0 min-w-0 flex-col" style={{ width: `${ratio * 100}%` }}>
                {renderPane(leftTab, 'left')}
            </div>
            <div
                role="separator"
                aria-orientation="vertical"
                className="w-1 shrink-0 cursor-col-resize bg-[var(--bq-border)] hover:bg-blue-500/50"
                onPointerDown={onDividerPointerDown}
                onPointerMove={onDividerPointerMove}
                onPointerUp={onDividerPointerUp}
            />
            <div className="flex min-h-0 min-w-0 flex-1 flex-col">{renderPane(rightTab, 'right')}</div>
        </div>
    );
}
