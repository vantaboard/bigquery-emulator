import { useQueryClient } from '@tanstack/react-query';
import { Copy, Plus, RefreshCw, Trash2 } from 'lucide-react';
import { useState } from 'react';
import { useParams } from 'react-router';

import { ActionToolbar, ToolbarButton } from '@/components/ui/ActionToolbar';
import { TabBar } from '@/components/ui/TabBar';
import { UnplannedTab } from '@/components/ui/UnplannedTab';
import { Breadcrumbs, datasetBreadcrumbs } from '@/features/workspace/components/Breadcrumbs';

import { CopyDatasetModal } from './copyDataset/CopyDatasetModal';
import { CreateTableModal } from './createTable/CreateTableModal';
import { DeleteDatasetDialog } from './delete/DeleteResourceDialogs';
import { DatasetDetailsTab } from './dataset/DatasetDetailsTab';
import { DatasetOverviewTab } from './dataset/DatasetOverviewTab';

const RESOURCE_TABS = [
    { id: 'overview', label: 'Overview', testId: 'dataset-resource-tab-overview' },
    { id: 'details', label: 'Details', testId: 'dataset-resource-tab-details' },
    { id: 'insights', label: 'Insights', testId: 'dataset-resource-tab-insights' },
] as const;

type ResourceTab = (typeof RESOURCE_TABS)[number]['id'];

export function DatasetTabPage() {
    const { projectId = '', datasetId = '' } = useParams();
    const queryClient = useQueryClient();
    const [resourceTab, setResourceTab] = useState<ResourceTab>('overview');
    const [createTableOpen, setCreateTableOpen] = useState(false);
    const [copyOpen, setCopyOpen] = useState(false);
    const [deleteOpen, setDeleteOpen] = useState(false);

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
                    testId="create-table-button"
                    onClick={() => setCreateTableOpen(true)}
                />
                <ToolbarButton
                    icon={Copy}
                    label="Copy"
                    testId="copy-dataset-button"
                    onClick={() => setCopyOpen(true)}
                />
                <ToolbarButton
                    icon={Trash2}
                    label="Delete"
                    variant="danger"
                    testId="delete-dataset-button"
                    onClick={() => setDeleteOpen(true)}
                />
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

            <CreateTableModal
                open={createTableOpen}
                projectId={projectId}
                datasetId={datasetId}
                onClose={() => setCreateTableOpen(false)}
            />
            <CopyDatasetModal
                open={copyOpen}
                projectId={projectId}
                datasetId={datasetId}
                onClose={() => setCopyOpen(false)}
            />
            <DeleteDatasetDialog
                open={deleteOpen}
                projectId={projectId}
                datasetId={datasetId}
                onClose={() => setDeleteOpen(false)}
            />
        </div>
    );
}
