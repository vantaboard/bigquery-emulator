import { useQueries, useQuery } from '@tanstack/react-query';
import { Link } from 'react-router';

import { explorerQueries } from '@/features/explorer/api';

interface DatasetTablesSubTabProps {
    projectId: string;
    datasetId: string;
}

function tableRoute(projectId: string, datasetId: string, tableId: string): string {
    return `/project/${encodeURIComponent(projectId)}/dataset/${encodeURIComponent(datasetId)}/table/${encodeURIComponent(tableId)}`;
}

export function DatasetTablesSubTab({ projectId, datasetId }: DatasetTablesSubTabProps) {
    const {
        data: tableIds = [],
        isLoading,
        isError,
        error,
    } = useQuery({
        queryKey: ['explorer', 'tables', projectId, datasetId],
        queryFn: () => explorerQueries.tables(projectId, datasetId),
    });

    const metaQueries = useQueries({
        queries: tableIds.map((tableId) => ({
            queryKey: ['explorer', 'tableSchema', projectId, datasetId, tableId],
            queryFn: () => explorerQueries.tableSchema(projectId, datasetId, tableId),
        })),
    });

    if (isLoading) {
        return <p className="p-4 text-sm text-[var(--bq-muted)]">Loading tables…</p>;
    }

    if (isError) {
        return (
            <p className="p-4 text-sm text-red-400">
                Failed to load tables: {error instanceof Error ? error.message : 'Unknown error'}
            </p>
        );
    }

    if (tableIds.length === 0) {
        return <p className="p-4 text-sm text-[var(--bq-muted)]">No tables in this dataset.</p>;
    }

    return (
        <div className="overflow-x-auto" data-testid="dataset-overview-tables">
            <table className="w-full border-collapse text-sm">
                <thead>
                    <tr className="border-b border-[var(--bq-border)] text-left text-[var(--bq-muted)]">
                        <th className="px-4 py-2 font-medium">Table ID</th>
                        <th className="px-4 py-2 font-medium">Type</th>
                        <th className="px-4 py-2 font-medium">Created</th>
                    </tr>
                </thead>
                <tbody>
                    {tableIds.map((tableId, index) => {
                        const meta = metaQueries[index]?.data;
                        return (
                            <tr key={tableId} className="border-b border-[var(--bq-border)]/50 hover:bg-white/5">
                                <td className="px-4 py-2">
                                    <Link
                                        to={tableRoute(projectId, datasetId, tableId)}
                                        className="text-blue-400 hover:underline"
                                    >
                                        {tableId}
                                    </Link>
                                </td>
                                <td className="px-4 py-2 text-[var(--bq-muted)]">
                                    {meta?.resourceType ?? meta?.type ?? '—'}
                                </td>
                                <td className="px-4 py-2 text-[var(--bq-muted)]">{meta?.creationTime || '—'}</td>
                            </tr>
                        );
                    })}
                </tbody>
            </table>
        </div>
    );
}
