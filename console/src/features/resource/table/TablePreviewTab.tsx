import { useQuery } from '@tanstack/react-query';
import { useMemo, useRef, useState } from 'react';

import { ResultsTable } from '@/features/explorer/components/ResultsTable';
import { explorerQueries } from '@/features/explorer/api';
import type { QueryResponse } from '@/types/api';

interface TablePreviewTabProps {
    projectId: string;
    datasetId: string;
    tableId: string;
}

const PAGE_SIZES = [10, 25, 50, 100] as const;

type PreviewSource = 'tabledata' | 'query';

interface PreviewResult {
    source: PreviewSource;
    data: QueryResponse;
    pageToken: string | null;
    totalRows: number;
}

function toQueryResponse(columns: string[], rows: Record<string, unknown>[], totalRows: number): QueryResponse {
    return { columns, rows, total_rows: totalRows };
}

export function TablePreviewTab({ projectId, datasetId, tableId }: TablePreviewTabProps) {
    const [pageSize, setPageSize] = useState<(typeof PAGE_SIZES)[number]>(25);
    const [pageToken, setPageToken] = useState<string | null>(null);
    const [pageIndex, setPageIndex] = useState(0);
    const tokenStackRef = useRef<string[]>([]);

    const fqn = `\`${projectId}.${datasetId}.${tableId}\``;

    const { data, isLoading, isError, error, isFetching } = useQuery({
        queryKey: ['explorer', 'tablePreview', projectId, datasetId, tableId, pageSize, pageToken],
        queryFn: async (): Promise<PreviewResult> => {
            try {
                const page = await explorerQueries.tableData(projectId, datasetId, tableId, {
                    maxResults: pageSize,
                    pageToken: pageToken ?? undefined,
                });
                const columns = page.rows.length
                    ? Object.keys(page.rows[0] ?? {})
                    : (
                          await explorerQueries.tableSchema(projectId, datasetId, tableId)
                      ).schema.map((field) => field.name);
                return {
                    source: 'tabledata',
                    pageToken: page.pageToken,
                    totalRows: page.totalRows,
                    data: toQueryResponse(columns, page.rows, page.totalRows),
                };
            } catch {
                const sql = `SELECT * FROM ${fqn} LIMIT ${pageSize}`;
                const result = await explorerQueries.runQuery(sql, projectId);
                return {
                    source: 'query',
                    pageToken: null,
                    totalRows: result.total_rows,
                    data: result,
                };
            }
        },
    });

    const canPaginate = data?.source === 'tabledata' && data.pageToken !== null;
    const hasPrevious = pageIndex > 0;

    const statusLabel = useMemo(() => {
        if (!data) return '';
        const start = pageIndex * pageSize + 1;
        const end = start + data.data.rows.length - 1;
        if (data.data.rows.length === 0) return 'No rows';
        return `Rows ${start.toLocaleString()}–${end.toLocaleString()} of ${data.totalRows.toLocaleString()}`;
    }, [data, pageIndex, pageSize]);

    const onPageSizeChange = (next: number) => {
        setPageSize(next as (typeof PAGE_SIZES)[number]);
        setPageToken(null);
        setPageIndex(0);
        tokenStackRef.current = [];
    };

    const onNext = () => {
        if (!data?.pageToken) return;
        tokenStackRef.current = [...tokenStackRef.current, pageToken ?? ''];
        setPageToken(data.pageToken);
        setPageIndex((value) => value + 1);
    };

    const onPrevious = () => {
        if (!hasPrevious) return;
        const next = [...tokenStackRef.current];
        const previousToken = next.pop();
        tokenStackRef.current = next;
        setPageToken(previousToken === '' ? null : (previousToken ?? null));
        setPageIndex((value) => Math.max(0, value - 1));
    };

    if (isLoading) {
        return <p className="text-sm text-[var(--bq-muted)]">Loading preview…</p>;
    }

    if (isError || !data) {
        return (
            <p className="text-sm text-red-400">
                Failed to load preview: {error instanceof Error ? error.message : 'Unknown error'}
            </p>
        );
    }

    return (
        <div data-testid="table-tab-preview">
            {data.source === 'query' ? (
                <p className="mb-3 text-xs text-[var(--bq-muted)]" data-testid="table-preview-fallback-note">
                    Table data API is unavailable; showing results from a SELECT query fallback.
                </p>
            ) : null}

            <div className="mb-3 flex flex-wrap items-center gap-3 text-sm">
                <label className="inline-flex items-center gap-2 text-[var(--bq-muted)]">
                    Rows per page
                    <select
                        data-testid="table-preview-page-size"
                        className="rounded-md border border-[var(--bq-border)] bg-transparent px-2 py-1 text-white"
                        value={pageSize}
                        onChange={(event) => onPageSizeChange(Number(event.target.value))}
                    >
                        {PAGE_SIZES.map((size) => (
                            <option key={size} value={size}>
                                {size}
                            </option>
                        ))}
                    </select>
                </label>
                <span data-testid="table-preview-status">{statusLabel}</span>
                {data.source === 'tabledata' ? (
                    <div className="flex items-center gap-2">
                        <button
                            type="button"
                            data-testid="table-preview-prev"
                            className="rounded-md border border-[var(--bq-border)] px-2 py-1 disabled:opacity-50"
                            disabled={!hasPrevious || isFetching}
                            onClick={onPrevious}
                        >
                            Previous
                        </button>
                        <button
                            type="button"
                            data-testid="table-preview-next"
                            className="rounded-md border border-[var(--bq-border)] px-2 py-1 disabled:opacity-50"
                            disabled={!canPaginate || isFetching}
                            onClick={onNext}
                        >
                            Next
                        </button>
                    </div>
                ) : null}
            </div>

            <ResultsTable data={data.data} />
        </div>
    );
}
