import { useQuery } from '@tanstack/react-query';
import { X } from 'lucide-react';

import { explorerQueries } from '@/features/explorer/api';
import { cn } from '@/lib/utils';

interface ReferencePanelProps {
    projectId?: string;
    datasetId?: string;
    tableId?: string;
    open: boolean;
    onClose: () => void;
    className?: string;
}

export function ReferencePanel({ projectId, datasetId, tableId, open, onClose, className }: ReferencePanelProps) {
    const { data, isLoading, isError } = useQuery({
        queryKey: ['reference-panel', projectId, datasetId, tableId],
        queryFn: () => explorerQueries.tableSchema(projectId!, datasetId!, tableId!),
        enabled: open && Boolean(projectId && datasetId && tableId),
    });

    if (!open) return null;

    const title =
        projectId && datasetId && tableId ? `${projectId}.${datasetId}.${tableId}` : 'Schema reference';

    return (
        <aside
            data-testid="query-reference-panel"
            className={cn(
                'flex w-64 shrink-0 flex-col border-l border-[var(--bq-border)] bg-[var(--bq-surface)]',
                className,
            )}
        >
            <div className="flex items-center justify-between border-b border-[var(--bq-border)] px-3 py-2">
                <div className="truncate text-xs font-medium uppercase tracking-wide text-white/60">Reference</div>
                <button
                    type="button"
                    aria-label="Close reference panel"
                    className="rounded p-1 hover:bg-white/5"
                    onClick={onClose}
                >
                    <X className="size-4" />
                </button>
            </div>
            <div className="min-h-0 flex-1 overflow-auto p-3">
                <div className="mb-2 truncate text-sm font-medium">{title}</div>
                {!projectId || !datasetId || !tableId ? (
                    <p className="text-sm text-white/50">Open a table query tab to see its schema.</p>
                ) : isLoading ? (
                    <p className="text-sm text-white/50">Loading schema…</p>
                ) : isError ? (
                    <p className="text-sm text-red-300">Could not load schema.</p>
                ) : (
                    <ul className="space-y-1 text-sm">
                        {data?.schema.map((field) => (
                            <li key={field.name} data-testid={`reference-field-${field.name}`}>
                                <span className="font-mono text-blue-300">{field.name}</span>
                                <span className="text-white/50"> · {field.type}</span>
                                {field.mode !== 'NULLABLE' ? (
                                    <span className="text-white/40"> ({field.mode})</span>
                                ) : null}
                            </li>
                        ))}
                    </ul>
                )}
            </div>
        </aside>
    );
}
