import { useQueries, useQuery } from '@tanstack/react-query';
import { Link } from 'react-router';

import { explorerQueries } from '@/features/explorer/api';

interface DatasetRoutinesSubTabProps {
    projectId: string;
    datasetId: string;
}

function routineRoute(projectId: string, datasetId: string, routineId: string): string {
    return `/project/${encodeURIComponent(projectId)}/dataset/${encodeURIComponent(datasetId)}/routine/${encodeURIComponent(routineId)}`;
}

function formatRoutineType(routineType: string): string {
    if (!routineType) return '—';
    return routineType.replace(/_/g, ' ');
}

export function DatasetRoutinesSubTab({ projectId, datasetId }: DatasetRoutinesSubTabProps) {
    const {
        data: routineIds = [],
        isLoading,
        isError,
        error,
    } = useQuery({
        queryKey: ['explorer', 'routines', projectId, datasetId],
        queryFn: () => explorerQueries.routines(projectId, datasetId),
        retry: false,
    });

    const metaQueries = useQueries({
        queries: routineIds.map((routineId) => ({
            queryKey: ['explorer', 'routine', projectId, datasetId, routineId],
            queryFn: () => explorerQueries.routine(projectId, datasetId, routineId),
        })),
    });

    if (isLoading) {
        return <p className="p-4 text-sm text-[var(--bq-muted)]">Loading routines…</p>;
    }

    if (isError) {
        return (
            <div className="p-4 text-sm" data-testid="dataset-overview-routines-error">
                <p className="text-[var(--bq-muted)]">Routines are not available from the emulator.</p>
                <p className="mt-1 text-xs text-red-400/80">
                    {error instanceof Error ? error.message : 'Upstream routines.list may be unsupported.'}
                </p>
            </div>
        );
    }

    if (routineIds.length === 0) {
        return (
            <p className="p-4 text-sm text-[var(--bq-muted)]" data-testid="dataset-overview-routines-empty">
                No routines in this dataset.
            </p>
        );
    }

    return (
        <div className="overflow-x-auto" data-testid="dataset-overview-routines">
            <table className="w-full border-collapse text-sm">
                <thead>
                    <tr className="border-b border-[var(--bq-border)] text-left text-[var(--bq-muted)]">
                        <th className="px-4 py-2 font-medium">Routine ID</th>
                        <th className="px-4 py-2 font-medium">Type</th>
                        <th className="px-4 py-2 font-medium">Language</th>
                        <th className="px-4 py-2 font-medium">Return type</th>
                    </tr>
                </thead>
                <tbody>
                    {routineIds.map((routineId, index) => {
                        const meta = metaQueries[index]?.data;
                        return (
                            <tr key={routineId} className="border-b border-[var(--bq-border)]/50 hover:bg-white/5">
                                <td className="px-4 py-2">
                                    <Link
                                        to={routineRoute(projectId, datasetId, routineId)}
                                        className="text-blue-400 hover:underline"
                                        data-testid={`routine-link-${routineId}`}
                                    >
                                        {routineId}
                                    </Link>
                                </td>
                                <td className="px-4 py-2 text-[var(--bq-muted)]">
                                    {formatRoutineType(meta?.routineType ?? '')}
                                </td>
                                <td className="px-4 py-2 text-[var(--bq-muted)]">{meta?.language || '—'}</td>
                                <td className="px-4 py-2 text-[var(--bq-muted)]">{meta?.returnType || '—'}</td>
                            </tr>
                        );
                    })}
                </tbody>
            </table>
        </div>
    );
}
