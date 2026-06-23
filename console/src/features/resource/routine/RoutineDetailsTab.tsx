import { useQuery } from '@tanstack/react-query';

import { explorerQueries } from '@/features/explorer/api';

interface RoutineDetailsTabProps {
    projectId: string;
    datasetId: string;
    routineId: string;
}

function formatRoutineType(routineType: string): string {
    return routineType.replace(/_/g, ' ').toLowerCase().replace(/^\w/, (c) => c.toUpperCase());
}

export function RoutineDetailsTab({ projectId, datasetId, routineId }: RoutineDetailsTabProps) {
    const { data, isLoading, isError, error } = useQuery({
        queryKey: ['explorer', 'routine', projectId, datasetId, routineId],
        queryFn: () => explorerQueries.routine(projectId, datasetId, routineId),
    });

    if (isLoading) {
        return <p className="p-4 text-sm text-[var(--bq-muted)]">Loading routine…</p>;
    }

    if (isError || !data) {
        return (
            <p className="p-4 text-sm text-red-400" data-testid="routine-details-error">
                Failed to load routine: {error instanceof Error ? error.message : 'Unknown error'}
            </p>
        );
    }

    return (
        <div className="space-y-6" data-testid="routine-details">
            <dl className="grid gap-4 text-sm sm:grid-cols-2">
                <div>
                    <dt className="text-[var(--bq-muted)]">Routine ID</dt>
                    <dd className="font-medium">{data.id}</dd>
                </div>
                <div>
                    <dt className="text-[var(--bq-muted)]">Type</dt>
                    <dd data-testid="routine-type">{formatRoutineType(data.routineType) || '—'}</dd>
                </div>
                <div>
                    <dt className="text-[var(--bq-muted)]">Language</dt>
                    <dd data-testid="routine-language">{data.language || '—'}</dd>
                </div>
                <div>
                    <dt className="text-[var(--bq-muted)]">Return type</dt>
                    <dd data-testid="routine-return-type">{data.returnType || '—'}</dd>
                </div>
                <div>
                    <dt className="text-[var(--bq-muted)]">Created</dt>
                    <dd>{data.creationTime || '—'}</dd>
                </div>
                <div>
                    <dt className="text-[var(--bq-muted)]">Last modified</dt>
                    <dd>{data.lastModifiedTime || '—'}</dd>
                </div>
            </dl>

            {data.arguments.length > 0 ? (
                <section data-testid="routine-arguments">
                    <h3 className="mb-2 text-sm font-medium">Arguments</h3>
                    <table className="w-full border-collapse text-sm">
                        <thead>
                            <tr className="border-b border-[var(--bq-border)] text-left text-[var(--bq-muted)]">
                                <th className="px-3 py-2 font-medium">Name</th>
                                <th className="px-3 py-2 font-medium">Type</th>
                                <th className="px-3 py-2 font-medium">Kind</th>
                            </tr>
                        </thead>
                        <tbody>
                            {data.arguments.map((arg) => (
                                <tr
                                    key={`${arg.name}-${arg.argumentKind}`}
                                    className="border-b border-[var(--bq-border)]/50"
                                    data-testid={`routine-arg-${arg.name}`}
                                >
                                    <td className="px-3 py-2">{arg.name || '—'}</td>
                                    <td className="px-3 py-2 text-[var(--bq-muted)]">{arg.dataType || '—'}</td>
                                    <td className="px-3 py-2 text-[var(--bq-muted)]">{arg.argumentKind || '—'}</td>
                                </tr>
                            ))}
                        </tbody>
                    </table>
                </section>
            ) : null}

            <section data-testid="routine-definition">
                <h3 className="mb-2 text-sm font-medium">Definition</h3>
                <pre className="overflow-x-auto rounded-md border border-[var(--bq-border)] bg-black/30 p-4 font-mono text-sm whitespace-pre-wrap">
                    {data.definitionBody || '—'}
                </pre>
            </section>
        </div>
    );
}
