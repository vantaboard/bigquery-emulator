import { PanelLeft, PanelRight, Plus, X, XCircle } from 'lucide-react';
import { useQuery } from '@tanstack/react-query';
import { useCallback, useMemo, useState } from 'react';
import { useNavigate } from 'react-router';

import { ContextMenu, type ContextMenuState } from '@/components/ui/ContextMenu';
import { TabBar } from '@/components/ui/Tabs';
import { explorerQueries } from '@/features/explorer/api';
import { ResourceIcon, resourceIconForTab } from '@/features/resource/ResourceIcon';
import { getPaneGroup, orderedPaneTabs } from '@/features/workspace/splitUtils';
import { tabRoutePath, useWorkspace } from '@/features/workspace/store';
import { tabLabel, type SplitPaneSide, type WorkspaceTab } from '@/features/workspace/types';

function navigateToTab(navigate: ReturnType<typeof useNavigate>, tab: WorkspaceTab | undefined) {
    if (tab) navigate(tabRoutePath(tab));
    else navigate('/');
}

export interface WorkspacePaneTabBarProps {
    paneSide?: SplitPaneSide | null;
}

export function WorkspacePaneTabBar({ paneSide = null }: WorkspacePaneTabBarProps) {
    const navigate = useNavigate();
    const {
        tabs: allTabs,
        session,
        activateTab,
        closeTab,
        closeOtherTabs,
        splitTab,
        focusPane,
        openBlankQuery,
    } = useWorkspace();
    const [menu, setMenu] = useState<(ContextMenuState & { tabId: string }) | null>(null);
    const { data: projects = [] } = useQuery({
        queryKey: ['explorer', 'projects'],
        queryFn: explorerQueries.projects,
    });

    const { paneTabs, activeId } = useMemo(() => {
        if (!paneSide || !session.split) {
            return {
                paneTabs: allTabs,
                activeId: session.activeTabId ?? '',
            };
        }
        const group = getPaneGroup(session.split, paneSide);
        return {
            paneTabs: orderedPaneTabs(allTabs, group.tabOrder),
            activeId: group.activeTabId ?? '',
        };
    }, [allTabs, paneSide, session.activeTabId, session.split]);

    const onNewQuery = () => {
        const defaultProject = projects[0] ?? '';
        const id = openBlankQuery(defaultProject, paneSide ?? undefined);
        if (paneSide) focusPane(paneSide);
        navigate(`/query/${encodeURIComponent(id)}`);
    };

    const onActivateTab = useCallback(
        (id: string) => {
            if (paneSide) focusPane(paneSide);
            activateTab(id);
            const tab = allTabs.find((t) => t.id === id);
            if (tab) navigate(tabRoutePath(tab));
        },
        [activateTab, allTabs, focusPane, navigate, paneSide],
    );

    const onCloseTab = useCallback(
        (id: string) => {
            const idx = paneTabs.findIndex((t) => t.id === id);
            const remaining = paneTabs.filter((t) => t.id !== id);
            const closingActive = activeId === id;

            closeTab(id);

            if (closingActive) {
                const next = remaining[Math.min(idx, remaining.length - 1)] ?? remaining[0];
                navigateToTab(navigate, next);
            }
        },
        [activeId, closeTab, navigate, paneTabs],
    );

    const menuTabId = menu?.tabId ?? null;
    const canBulk = paneTabs.length >= 2;
    const canSplit = allTabs.length >= 2;

    return (
        <div
            className="flex items-center gap-2 border-b border-[var(--bq-border)] bg-[var(--bq-surface)] px-2 py-1"
            data-testid={paneSide ? `workspace-pane-tabbar-${paneSide}` : undefined}
        >
            <TabBar
                variant="workspace"
                className="min-w-0 flex-1 border-0 bg-transparent p-0"
                activeId={activeId}
                onChange={onActivateTab}
                onClose={onCloseTab}
                onContextMenu={(id, event) => {
                    setMenu({ tabId: id, x: event.clientX, y: event.clientY });
                }}
                tabs={paneTabs.map((tab) => ({
                    id: tab.id,
                    label: tabLabel(tab),
                    icon: <ResourceIcon kind={resourceIconForTab(tab)} className="size-3.5" />,
                    closable: true,
                }))}
            />
            <button
                type="button"
                data-testid={paneSide ? `new-query-tab-${paneSide}` : 'new-query-tab'}
                className="inline-flex shrink-0 items-center gap-1 rounded border border-[var(--bq-border)] px-2 py-1 text-sm hover:bg-white/5"
                title="New query tab"
                onClick={onNewQuery}
            >
                <Plus className="size-4" />
            </button>

            <ContextMenu
                open={menu}
                onClose={() => setMenu(null)}
                items={[
                    {
                        label: 'Split tab to left',
                        icon: PanelLeft,
                        disabled: !canSplit,
                        onClick: () => {
                            if (!menuTabId) return;
                            splitTab(menuTabId, 'left');
                            navigateToTab(
                                navigate,
                                allTabs.find((t) => t.id === menuTabId),
                            );
                        },
                    },
                    {
                        label: 'Split tab to right',
                        icon: PanelRight,
                        disabled: !canSplit,
                        onClick: () => {
                            if (!menuTabId) return;
                            splitTab(menuTabId, 'right');
                            navigateToTab(
                                navigate,
                                allTabs.find((t) => t.id === menuTabId),
                            );
                        },
                    },
                    {
                        label: 'Close other tabs',
                        icon: XCircle,
                        disabled: !canBulk,
                        onClick: () => {
                            if (!menuTabId) return;
                            closeOtherTabs(menuTabId);
                            navigateToTab(
                                navigate,
                                allTabs.find((t) => t.id === menuTabId),
                            );
                        },
                    },
                    {
                        label: 'Close tab',
                        icon: X,
                        onClick: () => {
                            if (!menuTabId) return;
                            onCloseTab(menuTabId);
                        },
                    },
                ]}
            />
        </div>
    );
}
