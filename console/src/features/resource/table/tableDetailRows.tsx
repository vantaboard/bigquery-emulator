import type { DetailRow } from '@/components/ui/DetailTable';
import type { ResourceType, StorageStats, TableMetadata } from '@/types/api';

function formatOptionalText(value: string): string | undefined {
    return value.trim() ? value : undefined;
}

function formatBytes(value: number): string | undefined {
    if (!Number.isFinite(value) || value < 0) return undefined;
    if (value === 0) return '0 B';
    const units = ['B', 'KiB', 'MiB', 'GiB', 'TiB'];
    const exponent = Math.min(Math.floor(Math.log(value) / Math.log(1024)), units.length - 1);
    const scaled = value / 1024 ** exponent;
    const digits = scaled >= 10 || exponent === 0 ? 0 : 1;
    return `${scaled.toFixed(digits)} ${units[exponent]}`;
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

function isTableLike(resourceType: ResourceType): boolean {
    return resourceType === 'TABLE' || resourceType === 'SNAPSHOT' || resourceType === 'EXTERNAL';
}

function isViewLike(resourceType: ResourceType): boolean {
    return resourceType === 'VIEW' || resourceType === 'MATERIALIZED_VIEW';
}

export function resourceInfoHeading(resourceType: ResourceType): string {
    return isViewLike(resourceType) ? 'View info' : 'Table info';
}

export function resourceIdLabel(resourceType: ResourceType): string {
    return isViewLike(resourceType) ? 'View ID' : 'Table ID';
}

export function expirationLabel(resourceType: ResourceType): string {
    return isViewLike(resourceType) ? 'View expiration' : 'Table expiration';
}

export function tableInfoRows(meta: TableMetadata): DetailRow[] {
    const labelEntries = Object.entries(meta.labels);
    const tagEntries = Object.entries(meta.tags);
    const tableLike = isTableLike(meta.resourceType);
    const viewLike = isViewLike(meta.resourceType);

    const rows: DetailRow[] = [
        { label: resourceIdLabel(meta.resourceType), value: meta.fullyQualifiedName.split('.').pop() },
        { label: 'Created', value: formatOptionalText(meta.creationTime) },
        { label: 'Last modified', value: formatOptionalText(meta.lastModified) },
        {
            label: expirationLabel(meta.resourceType),
            value: meta.expirationTime ? formatOptionalText(meta.expirationTime) : undefined,
        },
    ];

    if (tableLike) {
        rows.push(
            { label: 'Data location', value: formatOptionalText(meta.location) },
            { label: 'Default collation', value: formatOptionalText(meta.defaultCollation) },
            { label: 'Default rounding mode', value: formatOptionalText(meta.defaultRoundingMode) },
            { label: 'Case insensitive', value: meta.caseInsensitive ? 'Yes' : 'No' },
        );
    }

    if (viewLike) {
        rows.push({ label: 'Use Legacy SQL', value: meta.useLegacySql ? 'Yes' : 'No' });
    }

    rows.push(
        { label: 'Description', value: formatOptionalText(meta.description) },
        {
            label: 'Labels',
            render: () => <ChipList entries={labelEntries} />,
        },
        {
            label: 'Primary key(s)',
            value: meta.primaryKeys.length ? meta.primaryKeys.join(', ') : undefined,
        },
        {
            label: 'Tags',
            render: () => <ChipList entries={tagEntries} />,
        },
    );

    return rows;
}

export function storageInfoRows(storage: StorageStats): DetailRow[] {
    return [
        { label: 'Number of rows', value: storage.numRows.toLocaleString() },
        { label: 'Total logical bytes', value: formatBytes(storage.totalLogicalBytes) },
        { label: 'Active logical bytes', value: formatBytes(storage.activeLogicalBytes) },
        { label: 'Long term logical bytes', value: formatBytes(storage.longTermLogicalBytes) },
        { label: 'Current physical bytes', value: formatBytes(storage.currentPhysicalBytes) },
        { label: 'Total physical bytes', value: formatBytes(storage.totalPhysicalBytes) },
        { label: 'Active physical bytes', value: formatBytes(storage.activePhysicalBytes) },
        { label: 'Long term physical bytes', value: formatBytes(storage.longTermPhysicalBytes) },
        { label: 'Time travel physical bytes', value: formatBytes(storage.timeTravelPhysicalBytes) },
    ];
}

export function showStorageInfo(resourceType: ResourceType): boolean {
    return isTableLike(resourceType);
}
