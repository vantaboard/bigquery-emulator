import type { ReactNode } from 'react';

import { cn } from '@/lib/utils';

const EMPTY = '—';

export interface DetailRow {
    label: string;
    value?: ReactNode;
    render?: () => ReactNode;
}

export interface DetailTableProps {
    rows: DetailRow[];
    className?: string;
    labelClassName?: string;
    valueClassName?: string;
}

function isEmptyValue(value: ReactNode): boolean {
    if (value === null || value === undefined) return true;
    if (typeof value === 'string') return value.trim() === '';
    return false;
}

function renderRowValue(row: DetailRow): ReactNode {
    if (row.render) return row.render();
    if (isEmptyValue(row.value)) return EMPTY;
    return row.value;
}

export function DetailTable({ rows, className, labelClassName, valueClassName }: DetailTableProps) {
    return (
        <table className={cn('w-full border-collapse text-sm', className)}>
            <tbody>
                {rows.map((row) => {
                    const content = renderRowValue(row);
                    const usePre = typeof content === 'string' && content.includes('\n');

                    return (
                        <tr key={row.label} className="border-b border-[var(--bq-border)]/50">
                            <th
                                className={cn(
                                    'w-48 py-1 text-left font-medium text-[var(--bq-muted)]',
                                    labelClassName,
                                )}
                            >
                                {row.label}
                            </th>
                            <td className={cn('py-1', valueClassName)}>
                                {usePre ? <pre className="whitespace-pre-wrap font-sans">{content}</pre> : content}
                            </td>
                        </tr>
                    );
                })}
            </tbody>
        </table>
    );
}
