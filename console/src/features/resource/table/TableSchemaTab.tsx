import { useQuery } from '@tanstack/react-query';
import { Copy } from 'lucide-react';
import { useMemo, useState } from 'react';

import { ToolbarButton } from '@/components/ui/ActionToolbar';
import { explorerQueries } from '@/features/explorer/api';

import { EditSchemaModal } from '../editSchema/EditSchemaModal';
import { formatSchemaAsJson, formatSchemaAsTable, schemaGridRows } from './schemaCopy';

interface TableSchemaTabProps {
    projectId: string;
    datasetId: string;
    tableId: string;
}

const GRID_COLUMNS = [
    'Field name',
    'Type',
    'Mode',
    'Description',
    'Key',
    'Collation',
    'Default Value',
    'Policy Tags',
    'Data Policies',
] as const;

async function copyText(value: string): Promise<void> {
    await navigator.clipboard.writeText(value);
}

export function TableSchemaTab({ projectId, datasetId, tableId }: TableSchemaTabProps) {
    const [filter, setFilter] = useState('');
    const [selected, setSelected] = useState<Set<string>>(new Set());
    const [editSchemaOpen, setEditSchemaOpen] = useState(false);

    const { data, isLoading, isError, error } = useQuery({
        queryKey: ['explorer', 'tableSchema', projectId, datasetId, tableId],
        queryFn: () => explorerQueries.tableSchema(projectId, datasetId, tableId),
    });

    const rows = useMemo(
        () => (data ? schemaGridRows(data.schema, data.primaryKeys) : []),
        [data],
    );

    const filteredRows = useMemo(() => {
        const needle = filter.trim().toLowerCase();
        if (!needle) return rows;
        return rows.filter((row) =>
            [
                row.name,
                row.type,
                row.mode,
                row.description ?? '',
                row.key,
            ]
                .join(' ')
                .toLowerCase()
                .includes(needle),
        );
    }, [filter, rows]);

    const selectedFields = useMemo(() => {
        const pick = selected.size > 0 ? rows.filter((row) => selected.has(row.name)) : rows;
        return pick.map(({ name, type, mode, description }) => ({ name, type, mode, description }));
    }, [rows, selected]);

    const allSelected = filteredRows.length > 0 && filteredRows.every((row) => selected.has(row.name));

    const toggleAll = () => {
        setSelected((prev) => {
            const next = new Set(prev);
            if (allSelected) {
                filteredRows.forEach((row) => next.delete(row.name));
            } else {
                filteredRows.forEach((row) => next.add(row.name));
            }
            return next;
        });
    };

    const toggleRow = (name: string) => {
        setSelected((prev) => {
            const next = new Set(prev);
            if (next.has(name)) next.delete(name);
            else next.add(name);
            return next;
        });
    };

    if (isLoading) {
        return <p className="text-sm text-[var(--bq-muted)]">Loading schema…</p>;
    }

    if (isError || !data) {
        return (
            <p className="text-sm text-red-400">
                Failed to load schema: {error instanceof Error ? error.message : 'Unknown error'}
            </p>
        );
    }

    return (
        <div data-testid="table-tab-schema">
            <div className="mb-3 flex flex-wrap items-center gap-2">
                <input
                    type="search"
                    placeholder="Filter fields"
                    data-testid="table-schema-filter"
                    className="min-w-48 flex-1 rounded-md border border-[var(--bq-border)] bg-transparent px-3 py-1.5 text-sm"
                    value={filter}
                    onChange={(event) => setFilter(event.target.value)}
                />
                <ToolbarButton
                    icon={Copy}
                    label="Copy"
                    dropdown={[
                        {
                            label: 'Copy as Table',
                            onClick: () => void copyText(formatSchemaAsTable(schemaGridRows(selectedFields, data.primaryKeys))),
                        },
                        {
                            label: 'Copy as JSON',
                            onClick: () => void copyText(formatSchemaAsJson(selectedFields)),
                        },
                    ]}
                />
                <button
                    type="button"
                    className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm hover:bg-white/5"
                    data-testid="edit-schema-button"
                    onClick={() => setEditSchemaOpen(true)}
                >
                    Edit schema
                </button>
                <button
                    type="button"
                    className="rounded-md border border-[var(--bq-border)] px-3 py-1.5 text-sm opacity-50"
                    disabled
                    title="Row access policies are not supported yet"
                >
                    View row access policies
                </button>
            </div>

            <div className="overflow-x-auto rounded-md border border-[var(--bq-border)]" data-testid="table-schema-grid">
                <table className="min-w-full border-collapse text-sm">
                    <thead className="bg-[#243044]">
                        <tr>
                            <th className="w-10 border-b border-[var(--bq-border)] px-2 py-2">
                                <input
                                    type="checkbox"
                                    aria-label="Select all fields"
                                    checked={allSelected}
                                    onChange={toggleAll}
                                />
                            </th>
                            {GRID_COLUMNS.map((column) => (
                                <th
                                    key={column}
                                    className="border-b border-[var(--bq-border)] px-2 py-2 text-left font-medium"
                                >
                                    {column}
                                </th>
                            ))}
                        </tr>
                    </thead>
                    <tbody>
                        {filteredRows.map((row) => (
                            <tr key={row.name} className="odd:bg-black/10" data-testid={`schema-field-${row.name}`}>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">
                                    <input
                                        type="checkbox"
                                        aria-label={`Select ${row.name}`}
                                        checked={selected.has(row.name)}
                                        onChange={() => toggleRow(row.name)}
                                    />
                                </td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">{row.name}</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">{row.type}</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">{row.mode}</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1 text-[var(--bq-muted)]">
                                    {row.description?.trim() || '—'}
                                </td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">{row.key || '—'}</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">—</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">—</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">—</td>
                                <td className="border-b border-[var(--bq-border)]/60 px-2 py-1">—</td>
                            </tr>
                        ))}
                    </tbody>
                </table>
            </div>
            {filteredRows.length === 0 ? (
                <p className="mt-2 text-sm text-[var(--bq-muted)]">No fields match the current filter.</p>
            ) : null}

            <EditSchemaModal
                open={editSchemaOpen}
                projectId={projectId}
                datasetId={datasetId}
                tableId={tableId}
                onClose={() => setEditSchemaOpen(false)}
            />
        </div>
    );
}
