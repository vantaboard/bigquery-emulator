import { useQuery } from '@tanstack/react-query';

import { DetailTable } from '@/components/ui/DetailTable';
import { SectionHeading } from '@/components/ui/SectionHeading';
import { explorerQueries } from '@/features/explorer/api';

import { datasetDetailRows } from './datasetDetailRows';

interface DatasetDetailsTabProps {
    projectId: string;
    datasetId: string;
}

export function DatasetDetailsTab({ projectId, datasetId }: DatasetDetailsTabProps) {
    const { data, isLoading, isError, error } = useQuery({
        queryKey: ['explorer', 'datasetMetadata', projectId, datasetId],
        queryFn: () => explorerQueries.datasetMetadata(projectId, datasetId),
    });

    if (isLoading) {
        return <p className="text-sm text-[var(--bq-muted)]">Loading dataset info…</p>;
    }

    if (isError || !data) {
        return (
            <p className="text-sm text-red-400">
                Failed to load dataset info: {error instanceof Error ? error.message : 'Unknown error'}
            </p>
        );
    }

    return (
        <div data-testid="dataset-tab-details">
            <SectionHeading>Dataset info</SectionHeading>
            <DetailTable rows={datasetDetailRows(data)} />
            <p className="mt-4 text-xs text-[var(--bq-muted)]">
                Empty fields may be omitted by the upstream emulator; they are shown as — above.
            </p>
        </div>
    );
}
