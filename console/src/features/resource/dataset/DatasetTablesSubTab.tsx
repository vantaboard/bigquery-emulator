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
        data: tableEntries = [],
        isLoading,
        isError,
        error,
    } = useQuery({
        queryKey: ['explorer', 'tables', projectId, datasetId],
        queryFn: () => explorerQueries.tables(projectId, datasetId),
    });

    const metaQueries = useQueries({
        queries: tableEntries.map((entry) => ({
            queryKey: ['explorer', 'tableSchema', projectId, datasetId, entry.tableId],
            queryFn: () => explorerQueries.tableSchema(projectId, datasetId, entry.tableId),
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

    if (tableEntries.length === 0) {
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
                    {tableEntries.map((entry, index) => {
                        const meta = metaQueries[index]?.data;
                        return (
                            <tr key={entry.tableId} className="border-b border-[var(--bq-border)]/50 hover:bg-white/5">
                                <td className="px-4 py-2">
                                    <Link
                                        to={tableRoute(projectId, datasetId, entry.tableId)}
                                        className="text-blue-400 hover:underline"
                                    >
                                        {entry.tableId}
                                    </Link>
                                </td>
                                <td className="px-4 py-2 text-[var(--bq-muted)]">
                                    {meta?.resourceType ?? meta?.type ?? entry.resourceType ?? '—'}
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
