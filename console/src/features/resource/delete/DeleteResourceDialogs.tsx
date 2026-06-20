import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useNavigate } from 'react-router';

import { ConfirmDialog } from '@/components/ui/ConfirmDialog';
import { explorerQueries } from '@/features/explorer/api';
import { notifyDatasetsChanged, notifyTablesChanged } from '@/features/explorer/events';
import { useWorkspace } from '@/features/workspace/store';
import { resourceTabId } from '@/features/workspace/types';

export interface DeleteDatasetDialogProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
}

export function DeleteDatasetDialog({ open, onClose, projectId, datasetId }: DeleteDatasetDialogProps) {
    const queryClient = useQueryClient();
    const navigate = useNavigate();
    const { session, closeTab } = useWorkspace();

    const deleteMutation = useMutation({
        mutationFn: (deleteContents: boolean) =>
            explorerQueries.deleteDataset(projectId, datasetId, { deleteContents }),
        onSuccess: async () => {
            const tabIds = session.tabs
                .filter(
                    (tab) =>
                        (tab.type === 'dataset' &&
                            tab.projectId === projectId &&
                            tab.datasetId === datasetId) ||
                        (tab.type === 'table' &&
                            tab.projectId === projectId &&
                            tab.datasetId === datasetId),
                )
                .map((tab) => tab.id);
            tabIds.forEach((id) => closeTab(id));
            await queryClient.invalidateQueries({ queryKey: ['explorer'] });
            notifyDatasetsChanged(projectId);
            onClose();
            navigate('/');
        },
        onError: () => {
            /* error shown via loading state reset */
        },
    });

    return (
        <ConfirmDialog
            open={open}
            onClose={onClose}
            title="Delete dataset"
            confirmLabel="Delete dataset"
            confirmTestId="delete-dataset-confirm"
            loading={deleteMutation.isPending}
            onConfirm={() => deleteMutation.mutate(true)}
            message={
                <div className="space-y-3">
                    <p>
                        Delete dataset <strong className="text-white">{datasetId}</strong> from project{' '}
                        <strong className="text-white">{projectId}</strong>? This cannot be undone.
                    </p>
                    <p className="text-xs">All tables in this dataset will be deleted.</p>
                    {deleteMutation.isError ? (
                        <p className="text-red-400" data-testid="delete-dataset-error">
                            {deleteMutation.error instanceof Error
                                ? deleteMutation.error.message
                                : 'Delete failed'}
                        </p>
                    ) : null}
                </div>
            }
        />
    );
}

export interface DeleteTableDialogProps {
    open: boolean;
    onClose: () => void;
    projectId: string;
    datasetId: string;
    tableId: string;
}

export function DeleteTableDialog({
    open,
    onClose,
    projectId,
    datasetId,
    tableId,
}: DeleteTableDialogProps) {
    const queryClient = useQueryClient();
    const navigate = useNavigate();
    const { closeTab } = useWorkspace();

    const deleteMutation = useMutation({
        mutationFn: () => explorerQueries.deleteTable(projectId, datasetId, tableId),
        onSuccess: async () => {
            closeTab(resourceTabId('table', projectId, datasetId, tableId));
            await queryClient.invalidateQueries({ queryKey: ['explorer'] });
            notifyTablesChanged(projectId, datasetId);
            onClose();
            navigate(`/project/${encodeURIComponent(projectId)}/dataset/${encodeURIComponent(datasetId)}`);
        },
    });

    return (
        <ConfirmDialog
            open={open}
            onClose={onClose}
            title="Delete table"
            confirmLabel="Delete table"
            confirmTestId="delete-table-confirm"
            loading={deleteMutation.isPending}
            onConfirm={() => deleteMutation.mutate()}
            message={
                <div className="space-y-3">
                    <p>
                        Delete table <strong className="text-white">{tableId}</strong> from{' '}
                        <strong className="text-white">
                            {projectId}.{datasetId}
                        </strong>
                        ? This cannot be undone.
                    </p>
                    {deleteMutation.isError ? (
                        <p className="text-red-400" data-testid="delete-table-error">
                            {deleteMutation.error instanceof Error
                                ? deleteMutation.error.message
                                : 'Delete failed'}
                        </p>
                    ) : null}
                </div>
            }
        />
    );
}
