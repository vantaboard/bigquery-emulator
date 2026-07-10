import { PanelLeft, PanelRight, Plus, X, XCircle } from 'lucide-react';
import { useQuery } from '@tanstack/react-query';
import { useCallback, useState } from 'react';
import { useNavigate } from 'react-router';

import { ContextMenu, type ContextMenuState } from '@/components/ui/ContextMenu';
import { TabBar } from '@/components/ui/Tabs';
import { explorerQueries } from '@/features/explorer/api';
import { ResourceIcon, resourceIconForTab } from '@/features/resource/ResourceIcon';

import { tabRoutePath, useWorkspace } from '@/features/workspace/store';
import { tabLabel, type WorkspaceTab } from '@/features/workspace/types';

function navigateToTab(navigate: ReturnType<typeof useNavigate>, tab: WorkspaceTab | undefined) {
    if (tab) navigate(tabRoutePath(tab));
    else navigate('/');
}

export function WorkspaceTabBar() {
    const navigate = useNavigate();
    const {
        tabs,
        activeTab,
        session,
        activateTab,
        closeTab,
        closeOtherTabs,
        splitTab,
        openBlankQuery,
    } = useWorkspace();
    const [menu, setMenu] = useState<(ContextMenuState & { tabId: string }) | null>(null);
    const { data: projects = [] } = useQuery({
        queryKey: ['explorer', 'projects'],
        queryFn: explorerQueries.projects,
    });

    const onNewQuery = () => {
        const defaultProject = projects[0] ?? '';
        const id = openBlankQuery(defaultProject);
        navigate(`/query/${encodeURIComponent(id)}`);
    };

    const onActivateTab = useCallback(
        (id: string) => {
            activateTab(id);
            const tab = tabs.find((t) => t.id === id);
            if (tab) navigate(tabRoutePath(tab));
        },
        [activateTab, navigate, tabs],
    );

    const onCloseTab = useCallback(
        (id: string) => {
            const idx = tabs.findIndex((t) => t.id === id);
            const remaining = tabs.filter((t) => t.id !== id);
            const closingActive = activeTab?.id === id;
            const closingSecondary = session.split?.secondaryTabId === id;

            closeTab(id);

            if (closingActive && session.split) {
                const survivor = session.split.secondaryTabId;
                const survivorTab = remaining.find((t) => t.id === survivor);
                navigateToTab(navigate, survivorTab);
                return;
            }

            if (closingActive) {
                const next = remaining[Math.min(idx, remaining.length - 1)] ?? remaining[0];
                navigateToTab(navigate, next);
                return;
            }

            if (closingSecondary) {
                const focused = remaining.find((t) => t.id === session.activeTabId);
                navigateToTab(navigate, focused);
            }
        },
        [activeTab?.id, closeTab, navigate, session.activeTabId, session.split, tabs],
    );

    const menuTabId = menu?.tabId ?? null;
    const canBulk = tabs.length >= 2;

    return (
        <div className="flex items-center gap-2 border-b border-[var(--bq-border)] bg-[var(--bq-surface)] px-2 py-1">
            <TabBar
                variant="workspace"
                className="min-w-0 flex-1 border-0 bg-transparent p-0"
                activeId={activeTab?.id ?? ''}
                onChange={onActivateTab}
                onClose={onCloseTab}
                onContextMenu={(id, event) => {
                    setMenu({ tabId: id, x: event.clientX, y: event.clientY });
                }}
                tabs={tabs.map((tab) => ({
                    id: tab.id,
                    label: tabLabel(tab),
                    icon: <ResourceIcon kind={resourceIconForTab(tab)} className="size-3.5" />,
                    closable: true,
                }))}
            />
            <button
                type="button"
                data-testid="new-query-tab"
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
                        disabled: !canBulk,
                        onClick: () => {
                            if (!menuTabId) return;
                            splitTab(menuTabId, 'left');
                            navigateToTab(
                                navigate,
                                tabs.find((t) => t.id === menuTabId),
                            );
                        },
                    },
                    {
                        label: 'Split tab to right',
                        icon: PanelRight,
                        disabled: !canBulk,
                        onClick: () => {
                            if (!menuTabId) return;
                            splitTab(menuTabId, 'right');
                            navigateToTab(
                                navigate,
                                tabs.find((t) => t.id === menuTabId),
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
                                tabs.find((t) => t.id === menuTabId),
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
