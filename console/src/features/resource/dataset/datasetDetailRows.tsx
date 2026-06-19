import type { DetailRow } from '@/components/ui/DetailTable';
import type { DatasetMetadata } from '@/types/api';

function formatOptionalText(value: string): string | undefined {
    return value.trim() ? value : undefined;
}

function formatMs(value: number | null): string | undefined {
    if (value === null) return undefined;
    return `${value.toLocaleString()} ms`;
}

function formatHours(value: number | null): string | undefined {
    if (value === null) return undefined;
    return `${value} hours`;
}

function ChipList({ entries }: { entries: [string, string][] }) {
    if (entries.length === 0) return undefined;
    return (
        <div className="flex flex-wrap gap-1">
            {entries.map(([key, val]) => (
                <span key={key} className="rounded bg-white/10 px-2 py-0.5 text-xs">
                    {key}: {val}
                </span>
            ))}
        </div>
    );
}

function ReplicasSection({ replicas }: { replicas: DatasetMetadata['replicas'] }) {
    if (!replicas.length) return undefined;
    return (
        <table className="w-full border-collapse text-xs">
            <thead>
                <tr className="border-b border-[var(--bq-border)]/50 text-left text-[var(--bq-muted)]">
                    <th className="py-1 pr-4 font-medium">Location</th>
                </tr>
            </thead>
            <tbody>
                {replicas.map((replica, index) => (
                    <tr key={index} className="border-b border-[var(--bq-border)]/30">
                        <td className="py-1">{replica.location?.trim() || '—'}</td>
                    </tr>
                ))}
            </tbody>
        </table>
    );
}

export function datasetDetailRows(meta: DatasetMetadata): DetailRow[] {
    const labelEntries = Object.entries(meta.labels);
    const tagEntries = Object.entries(meta.tags);

    return [
        { label: 'Dataset ID', value: meta.id },
        { label: 'Created', value: formatOptionalText(meta.creationTime) },
        { label: 'Default table expiration', value: formatMs(meta.defaultTableExpirationMs) },
        { label: 'Last modified', value: formatOptionalText(meta.lastModifiedTime) },
        { label: 'Data location', value: formatOptionalText(meta.location) },
        { label: 'Description', value: formatOptionalText(meta.description) },
        { label: 'Default collation', value: formatOptionalText(meta.defaultCollation) },
        { label: 'Default rounding mode', value: formatOptionalText(meta.defaultRoundingMode) },
        { label: 'Time travel window', value: formatHours(meta.maxTimeTravelHours) },
        { label: 'Case insensitive', value: meta.isCaseInsensitive ? 'Yes' : 'No' },
        {
            label: 'Labels',
            render: () => <ChipList entries={labelEntries} />,
        },
        {
            label: 'Tags',
            render: () => <ChipList entries={tagEntries} />,
        },
        {
            label: 'Replicas',
            render: () => <ReplicasSection replicas={meta.replicas} />,
        },
    ];
}
