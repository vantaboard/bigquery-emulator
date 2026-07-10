import { Plus } from 'lucide-react';
import { useQuery } from '@tanstack/react-query';
import { useNavigate } from 'react-router';

import { TabBar } from '@/components/ui/Tabs';
import { explorerQueries } from '@/features/explorer/api';
import { ResourceIcon, resourceIconForTab } from '@/features/resource/ResourceIcon';

import { tabRoutePath, useWorkspace } from '@/features/workspace/store';
import { tabLabel } from '@/features/workspace/types';

export function WorkspaceTabBar() {
    const navigate = useNavigate();
    const { tabs, activeTab, activateTab, closeTab, openBlankQuery } = useWorkspace();
    const { data: projects = [] } = useQuery({
        queryKey: ['explorer', 'projects'],
        queryFn: explorerQueries.projects,
    });

    const onNewQuery = () => {
        const defaultProject = projects[0] ?? '';
        const id = openBlankQuery(defaultProject);
        navigate(`/query/${encodeURIComponent(id)}`);
    };

    return (
        <div className="flex items-center gap-2 border-b border-[var(--bq-border)] bg-[var(--bq-surface)] px-2 py-1">
            <TabBar
                variant="workspace"
                className="min-w-0 flex-1 border-0 bg-transparent p-0"
                activeId={activeTab?.id ?? ''}
                onChange={(id) => {
                    activateTab(id);
                    const tab = tabs.find((t) => t.id === id);
                    if (tab) navigate(tabRoutePath(tab));
                }}
                onClose={(id) => {
                    const idx = tabs.findIndex((t) => t.id === id);
                    const remaining = tabs.filter((t) => t.id !== id);
                    const closingActive = activeTab?.id === id;
                    closeTab(id);
                    if (closingActive) {
                        const next = remaining[Math.min(idx, remaining.length - 1)] ?? remaining[0];
                        navigate(next ? tabRoutePath(next) : '/');
                    }
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
        </div>
    );
}
