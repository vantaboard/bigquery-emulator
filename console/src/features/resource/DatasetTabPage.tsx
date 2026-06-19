import { useQueryClient } from '@tanstack/react-query';
import { Copy, Plus, RefreshCw, Trash2 } from 'lucide-react';
import { useState } from 'react';
import { useParams } from 'react-router';

import { ActionToolbar, ToolbarButton } from '@/components/ui/ActionToolbar';
import { Modal } from '@/components/ui/Modal';
import { TabBar } from '@/components/ui/TabBar';
import { UnplannedTab } from '@/components/ui/UnplannedTab';
import { Breadcrumbs, datasetBreadcrumbs } from '@/features/workspace/components/Breadcrumbs';

import { DatasetDetailsTab } from './dataset/DatasetDetailsTab';
import { DatasetOverviewTab } from './dataset/DatasetOverviewTab';

const RESOURCE_TABS = [
    { id: 'overview', label: 'Overview', testId: 'dataset-resource-tab-overview' },
    { id: 'details', label: 'Details', testId: 'dataset-resource-tab-details' },
    { id: 'insights', label: 'Insights', testId: 'dataset-resource-tab-insights' },
] as const;

type ResourceTab = (typeof RESOURCE_TABS)[number]['id'];

type StubAction = 'create-table' | 'copy' | 'delete';

const STUB_TITLES: Record<StubAction, string> = {
    'create-table': 'Create table',
    copy: 'Copy dataset',
    delete: 'Delete dataset',
};

export function DatasetTabPage() {
    const { projectId = '', datasetId = '' } = useParams();
    const queryClient = useQueryClient();
    const [resourceTab, setResourceTab] = useState<ResourceTab>('overview');
    const [stubAction, setStubAction] = useState<StubAction | null>(null);

    const onRefresh = () => {
        void queryClient.invalidateQueries({ queryKey: ['explorer'] });
    };

    return (
        <div className="flex flex-1 flex-col p-6" data-testid="dataset-tab-page">
            <Breadcrumbs segments={datasetBreadcrumbs(projectId, datasetId)} />

            <ActionToolbar className="mt-4">
                <ToolbarButton
                    icon={Plus}
                    label="Create Table"
                    variant="primary"
                    onClick={() => setStubAction('create-table')}
                />
                <ToolbarButton icon={Copy} label="Copy" onClick={() => setStubAction('copy')} />
                <ToolbarButton icon={Trash2} label="Delete" variant="danger" onClick={() => setStubAction('delete')} />
                <ToolbarButton icon={RefreshCw} label="Refresh" onClick={onRefresh} />
            </ActionToolbar>

            <TabBar
                className="mt-4"
                tabs={[...RESOURCE_TABS]}
                activeId={resourceTab}
                onChange={(id) => setResourceTab(id as ResourceTab)}
            />

            <div className="mt-4 min-h-0 flex-1">
                {resourceTab === 'overview' ? (
                    <DatasetOverviewTab projectId={projectId} datasetId={datasetId} />
                ) : null}
                {resourceTab === 'details' ? (
                    <DatasetDetailsTab projectId={projectId} datasetId={datasetId} />
                ) : null}
                {resourceTab === 'insights' ? (
                    <div data-testid="dataset-tab-insights">
                        <UnplannedTab title="Insights" message="Dataset insights are not planned yet." />
                    </div>
                ) : null}
            </div>

            <Modal
                open={stubAction !== null}
                onClose={() => setStubAction(null)}
                title={stubAction ? STUB_TITLES[stubAction] : ''}
                footer={
                    <button
                        type="button"
                        className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                        onClick={() => setStubAction(null)}
                    >
                        Close
                    </button>
                }
            >
                <p className="text-sm text-[var(--bq-muted)]">
                    TODO (M3): {stubAction ? STUB_TITLES[stubAction] : 'Action'} workflow will be implemented in
                    milestone M3.
                </p>
            </Modal>
        </div>
    );
}
