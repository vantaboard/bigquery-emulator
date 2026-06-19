import { useQuery } from '@tanstack/react-query';

import { DetailTable } from '@/components/ui/DetailTable';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { explorerQueries } from '@/features/explorer/api';
import type { ResourceType } from '@/types/api';

import {
    resourceInfoHeading,
    showStorageInfo,
    storageInfoRows,
    tableInfoRows,
} from './tableDetailRows';

interface TableDetailsTabProps {
    projectId: string;
    datasetId: string;
    tableId: string;
}

function isViewLike(resourceType: ResourceType): boolean {
    return resourceType === 'VIEW' || resourceType === 'MATERIALIZED_VIEW';
}

export function TableDetailsTab({ projectId, datasetId, tableId }: TableDetailsTabProps) {
    const { data, isLoading, isError, error } = useQuery({
        queryKey: ['explorer', 'tableSchema', projectId, datasetId, tableId],
        queryFn: () => explorerQueries.tableSchema(projectId, datasetId, tableId),
    });

    if (isLoading) {
        return <p className="text-sm text-[var(--bq-muted)]">Loading table details…</p>;
    }

    if (isError || !data) {
        return (
            <p className="text-sm text-red-400">
                Failed to load table details: {error instanceof Error ? error.message : 'Unknown error'}
            </p>
        );
    }

    return (
        <div data-testid="table-tab-details">
            <SectionHeading>{resourceInfoHeading(data.resourceType)}</SectionHeading>
            <DetailTable rows={tableInfoRows(data)} />

            <SectionHeading className="mt-6">Storage info</SectionHeading>
            {showStorageInfo(data.resourceType) ? (
                <div data-testid="table-storage-info">
                    <DetailTable rows={storageInfoRows(data.storage)} />
                </div>
            ) : (
                <p className="text-sm text-[var(--bq-muted)]" data-testid="table-storage-unavailable">
                    Storage statistics are not available for views and materialized views.
                </p>
            )}

            {isViewLike(data.resourceType) ? (
                <div className="mt-6" data-testid="table-view-query">
                    <div className="mb-2 flex items-center justify-between gap-2">
                        <SectionHeading className="mb-0">Query</SectionHeading>
                        <button
                            type="button"
                            className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm opacity-50"
                            disabled
                            title="Edit Query is planned for M4"
                        >
                            Edit Query
                        </button>
                    </div>
                    <pre className="overflow-x-auto rounded-md border border-[var(--bq-border)] bg-black/20 p-3 text-sm">
                        {data.viewQuery.trim() || '—'}
                    </pre>
                </div>
            ) : null}
        </div>
    );
}
