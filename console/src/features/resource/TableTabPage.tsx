import { useQuery, useQueryClient } from '@tanstack/react-query';
import { Camera, Copy, Play, RefreshCw, Trash2 } from 'lucide-react';
import { useState } from 'react';
import { useNavigate, useParams } from 'react-router';

import { ActionToolbar, ToolbarButton } from '@/components/ui/ActionToolbar';
import { TabBar } from '@/components/ui/TabBar';
import { UnplannedTab } from '@/components/ui/UnplannedTab';
import { explorerQueries } from '@/features/explorer/api';
import { Breadcrumbs, tableBreadcrumbs } from '@/features/workspace/components/Breadcrumbs';
import { useWorkspace } from '@/features/workspace/store';
import type { ResourceType } from '@/types/api';

import { CopyTableModal } from './copyTable/CopyTableModal';
import { DeleteTableDialog } from './delete/DeleteResourceDialogs';
import { CreateSnapshotModal } from './snapshot/CreateSnapshotModal';
import { TableDetailsTab } from './table/TableDetailsTab';
import { TablePreviewTab } from './table/TablePreviewTab';
import { TableSchemaTab } from './table/TableSchemaTab';

const RESOURCE_TABS = [
    { id: 'schema', label: 'Schema', testId: 'table-resource-tab-schema' },
    { id: 'details', label: 'Details', testId: 'table-resource-tab-details' },
    { id: 'preview', label: 'Preview', testId: 'table-resource-tab-preview' },
    { id: 'table-explorer', label: 'Table Explorer', testId: 'table-resource-tab-table-explorer' },
    { id: 'insights', label: 'Insights', testId: 'table-resource-tab-insights' },
    { id: 'lineage', label: 'Lineage', testId: 'table-resource-tab-lineage' },
    { id: 'data-profile', label: 'Data Profile', testId: 'table-resource-tab-data-profile' },
    { id: 'data-quality', label: 'Data Quality', testId: 'table-resource-tab-data-quality' },
] as const;

type ResourceTab = (typeof RESOURCE_TABS)[number]['id'];

const UNPLANNED_MESSAGES: Record<
    Extract<ResourceTab, 'table-explorer' | 'insights' | 'lineage' | 'data-profile' | 'data-quality'>,
    { title: string; message: string }
> = {
    'table-explorer': {
        title: 'Table Explorer',
        message: 'Table explorer is not planned yet.',
    },
    insights: {
        title: 'Insights',
        message: 'Table insights are not planned yet.',
    },
    lineage: {
        title: 'Lineage',
        message: 'Table lineage is not planned yet.',
    },
    'data-profile': {
        title: 'Data Profile',
        message: 'Data profile is not planned yet.',
    },
    'data-quality': {
        title: 'Data Quality',
        message: 'Data quality checks are not planned yet.',
    },
};

function showSnapshotButton(resourceType: ResourceType | undefined): boolean {
    return resourceType === 'TABLE';
}

export function TableTabPage() {
    const { projectId = '', datasetId = '', tableId = '' } = useParams();
    const navigate = useNavigate();
    const queryClient = useQueryClient();
    const { openQueryForTable } = useWorkspace();
    const [resourceTab, setResourceTab] = useState<ResourceTab>('schema');
    const [copyOpen, setCopyOpen] = useState(false);
    const [snapshotOpen, setSnapshotOpen] = useState(false);
    const [deleteOpen, setDeleteOpen] = useState(false);

    const { data: metadata } = useQuery({
        queryKey: ['explorer', 'tableSchema', projectId, datasetId, tableId],
        queryFn: () => explorerQueries.tableSchema(projectId, datasetId, tableId),
    });

    const onQuery = () => {
        const id = openQueryForTable(projectId, datasetId, tableId);
        navigate(`/query/${encodeURIComponent(id)}`);
    };

    const onRefresh = () => {
        void queryClient.invalidateQueries({ queryKey: ['explorer'] });
    };

    return (
        <div className="flex flex-1 flex-col p-6" data-testid="table-tab-page">
            <Breadcrumbs segments={tableBreadcrumbs(projectId, datasetId, tableId)} />

            <ActionToolbar className="mt-4">
                <ToolbarButton
                    icon={Play}
                    label="Query"
                    variant="primary"
                    testId="open-query-from-table"
                    onClick={onQuery}
                />
                <ToolbarButton
                    icon={Copy}
                    label="Copy"
                    testId="copy-table-button"
                    onClick={() => setCopyOpen(true)}
                />
                {showSnapshotButton(metadata?.resourceType) ? (
                    <ToolbarButton
                        icon={Camera}
                        label="Snapshot"
                        testId="create-snapshot-button"
                        onClick={() => setSnapshotOpen(true)}
                    />
                ) : null}
                <ToolbarButton
                    icon={Trash2}
                    label="Delete"
                    variant="danger"
                    testId="delete-table-button"
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
                {resourceTab === 'schema' ? (
                    <TableSchemaTab projectId={projectId} datasetId={datasetId} tableId={tableId} />
                ) : null}
                {resourceTab === 'details' ? (
                    <TableDetailsTab projectId={projectId} datasetId={datasetId} tableId={tableId} />
                ) : null}
                {resourceTab === 'preview' ? (
                    <TablePreviewTab projectId={projectId} datasetId={datasetId} tableId={tableId} />
                ) : null}
                {resourceTab === 'table-explorer' ? (
                    <div data-testid="table-tab-table-explorer">
                        <UnplannedTab {...UNPLANNED_MESSAGES['table-explorer']} />
                    </div>
                ) : null}
                {resourceTab === 'insights' ? (
                    <div data-testid="table-tab-insights">
                        <UnplannedTab {...UNPLANNED_MESSAGES.insights} />
                    </div>
                ) : null}
                {resourceTab === 'lineage' ? (
                    <div data-testid="table-tab-lineage">
                        <UnplannedTab {...UNPLANNED_MESSAGES.lineage} />
                    </div>
                ) : null}
                {resourceTab === 'data-profile' ? (
                    <div data-testid="table-tab-data-profile">
                        <UnplannedTab {...UNPLANNED_MESSAGES['data-profile']} />
                    </div>
                ) : null}
                {resourceTab === 'data-quality' ? (
                    <div data-testid="table-tab-data-quality">
                        <UnplannedTab {...UNPLANNED_MESSAGES['data-quality']} />
                    </div>
                ) : null}
            </div>

            <CopyTableModal
                open={copyOpen}
                projectId={projectId}
                datasetId={datasetId}
                tableId={tableId}
                onClose={() => setCopyOpen(false)}
            />
            {showSnapshotButton(metadata?.resourceType) ? (
                <CreateSnapshotModal
                    open={snapshotOpen}
                    projectId={projectId}
                    datasetId={datasetId}
                    tableId={tableId}
                    onClose={() => setSnapshotOpen(false)}
                />
            ) : null}
            <DeleteTableDialog
                open={deleteOpen}
                projectId={projectId}
                datasetId={datasetId}
                tableId={tableId}
                onClose={() => setDeleteOpen(false)}
            />
        </div>
    );
}
