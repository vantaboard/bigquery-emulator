import { useQuery } from '@tanstack/react-query';

import { explorerQueries } from '@/features/explorer/api';

interface DatasetRoutinesSubTabProps {
    projectId: string;
    datasetId: string;
}

export function DatasetRoutinesSubTab({ projectId, datasetId }: DatasetRoutinesSubTabProps) {
    const { data: routineIds = [], isLoading, isError, error } = useQuery({
        queryKey: ['explorer', 'routines', projectId, datasetId],
        queryFn: () => explorerQueries.routines(projectId, datasetId),
        retry: false,
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
                    </tr>
                </thead>
                <tbody>
                    {routineIds.map((routineId) => (
                        <tr key={routineId} className="border-b border-[var(--bq-border)]/50">
                            <td className="px-4 py-2 text-[var(--bq-muted)]">{routineId}</td>
                        </tr>
                    ))}
                </tbody>
            </table>
            <p className="px-4 py-2 text-xs text-[var(--bq-muted)]">
                Routine detail view is planned for M5.
            </p>
        </div>
    );
}
