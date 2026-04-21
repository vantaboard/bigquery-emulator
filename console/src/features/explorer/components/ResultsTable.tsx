import { useEffect, useRef, useState } from 'react';

import { cn } from '@/lib/utils';
import type { QueryResponse } from '@/types/api';

const BATCH = 48;
const PREVIEW_CHARS = 320;
const PREVIEW_LINES = 5;

function formatCellValue(raw: unknown): {
    fullText: string;
    previewText: string;
    expandable: boolean;
    usePre: boolean;
} {
    if (raw === null) {
        return { fullText: 'NULL', previewText: 'NULL', expandable: false, usePre: false };
    }
    if (Array.isArray(raw) || (typeof raw === 'object' && raw !== null)) {
        const fullText = JSON.stringify(raw, null, 2);
        return {
            fullText,
            previewText: buildPreview(fullText),
            expandable: isExpandable(fullText),
            usePre: true,
        };
    }
    const fullText = String(raw);
    return {
        fullText,
        previewText: buildPreview(fullText),
        expandable: isExpandable(fullText),
        usePre: fullText.includes('\n'),
    };
}

function isExpandable(text: string): boolean {
    return text.length > PREVIEW_CHARS || text.split('\n').length > PREVIEW_LINES;
}

function buildPreview(text: string): string {
    const lines = text.split('\n');
    const clipped = lines.slice(0, PREVIEW_LINES).map((line) => line.slice(0, PREVIEW_CHARS));
    let preview = clipped.join('\n');
    if (preview.length > PREVIEW_CHARS) {
        preview = `${preview.slice(0, PREVIEW_CHARS)}...`;
    } else if (lines.length > PREVIEW_LINES || text.length > preview.length) {
        preview = `${preview}...`;
    }
    return preview;
}

function CellContent({
    fullText,
    previewText,
    expandable,
    usePre,
}: {
    fullText: string;
    previewText: string;
    expandable: boolean;
    usePre: boolean;
}) {
    const [open, setOpen] = useState(false);
    if (!expandable) {
        const Tag = usePre ? 'pre' : 'span';
        return (
            <Tag className={cn('text-sm', usePre && 'm-0 whitespace-pre-wrap font-sans')}>{fullText}</Tag>
        );
    }
    return (
        <div className="flex flex-col gap-1">
            {usePre ? (
                <pre className="m-0 max-w-full whitespace-pre-wrap font-sans text-sm">{open ? fullText : previewText}</pre>
            ) : (
                <span className="text-sm">{open ? fullText : previewText}</span>
            )}
            <button
                type="button"
                className="text-left text-xs text-blue-400 hover:underline"
                onClick={() => setOpen(!open)}
            >
                {open ? 'Show less' : 'Show more'}
            </button>
        </div>
    );
}

export function ResultsTable({ data }: { data: QueryResponse | null }) {
    const [visibleCount, setVisibleCount] = useState(0);
    const genRef = useRef(0);

    useEffect(() => {
        const rows = data?.rows ?? [];
        if (rows.length === 0) {
            setVisibleCount(0);
            return;
        }
        const gen = ++genRef.current;
        const first = Math.min(BATCH, rows.length);
        setVisibleCount(first);
        let shown = first;
        const pump = () => {
            if (gen !== genRef.current) return;
            if (shown >= rows.length) return;
            shown = Math.min(shown + BATCH, rows.length);
            setVisibleCount(shown);
            if (shown < rows.length) {
                requestAnimationFrame(pump);
            }
        };
        if (rows.length > BATCH) {
            requestAnimationFrame(pump);
        }
    }, [data]);

    if (!data || !data.columns.length) {
        return (
            <div className="rounded-md border border-[var(--bq-border)] bg-[var(--bq-surface)]/50 p-4 text-sm text-[var(--bq-muted)]">
                No query results yet. Run a query above.
            </div>
        );
    }

    const rows = data.rows;
    const slice = rows.slice(0, visibleCount);

    return (
        <div className="overflow-auto rounded-md border border-[var(--bq-border)]">
            <table className="min-w-full border-collapse text-sm">
                <thead className="sticky top-0 bg-[#243044]">
                    <tr>
                        {data.columns.map((c) => (
                            <th key={c} className="border-b border-[var(--bq-border)] px-2 py-2 text-left font-medium">
                                {c}
                            </th>
                        ))}
                    </tr>
                </thead>
                <tbody>
                    {slice.map((row, i) => (
                        <tr key={i} className="odd:bg-black/10">
                            {data.columns.map((col) => {
                                const f = formatCellValue(row[col]);
                                return (
                                    <td key={col} className="max-w-[28rem] border-b border-[var(--bq-border)]/60 px-2 py-1 align-top">
                                        <CellContent
                                            fullText={f.fullText}
                                            previewText={f.previewText}
                                            expandable={f.expandable}
                                            usePre={f.usePre}
                                        />
                                    </td>
                                );
                            })}
                        </tr>
                    ))}
                </tbody>
            </table>
        </div>
    );
}
