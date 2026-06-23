import { Link } from 'react-router';

export interface BreadcrumbSegment {
    label: string;
    to?: string;
}

interface BreadcrumbsProps {
    segments: BreadcrumbSegment[];
}

export function Breadcrumbs({ segments }: BreadcrumbsProps) {
    return (
        <nav aria-label="Breadcrumb" className="text-sm" data-testid="breadcrumbs">
            <ol className="flex flex-wrap items-center gap-1 text-[var(--bq-muted)]">
                {segments.map((segment, index) => {
                    const isLast = index === segments.length - 1;
                    return (
                        <li key={`${segment.label}-${index}`} className="inline-flex items-center gap-1">
                            {index > 0 ? <span aria-hidden="true">/</span> : null}
                            {segment.to && !isLast ? (
                                <Link to={segment.to} className="hover:text-white">
                                    {segment.label}
                                </Link>
                            ) : (
                                <span className={isLast ? 'text-white' : undefined}>{segment.label}</span>
                            )}
                        </li>
                    );
                })}
            </ol>
        </nav>
    );
}

export function datasetBreadcrumbs(projectId: string, datasetId: string): BreadcrumbSegment[] {
    return [
        { label: projectId, to: '/' },
        { label: 'Datasets' },
        { label: datasetId },
    ];
}

export function tableBreadcrumbs(
    projectId: string,
    datasetId: string,
    tableId: string,
): BreadcrumbSegment[] {
    return [
        { label: projectId, to: '/' },
        { label: 'Datasets' },
        {
            label: datasetId,
            to: `/project/${encodeURIComponent(projectId)}/dataset/${encodeURIComponent(datasetId)}`,
        },
        { label: 'Tables' },
        { label: tableId },
    ];
}

export function routineBreadcrumbs(
    projectId: string,
    datasetId: string,
    routineId: string,
): BreadcrumbSegment[] {
    return [
        { label: projectId, to: '/' },
        { label: 'Datasets' },
        {
            label: datasetId,
            to: `/project/${encodeURIComponent(projectId)}/dataset/${encodeURIComponent(datasetId)}`,
        },
        { label: 'Routines' },
        { label: routineId },
    ];
}
