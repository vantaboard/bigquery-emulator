import { useNavigate, useParams } from 'react-router';

import { Breadcrumbs, tableBreadcrumbs } from '@/features/workspace/components/Breadcrumbs';
import { useWorkspace } from '@/features/workspace/store';

export function TableTabPage() {
    const { projectId = '', datasetId = '', tableId = '' } = useParams();
    const navigate = useNavigate();
    const { openQueryForTable } = useWorkspace();

    const onQuery = () => {
        const id = openQueryForTable(projectId, datasetId, tableId);
        navigate(`/query/${encodeURIComponent(id)}`);
    };

    return (
        <div className="flex flex-1 flex-col p-6" data-testid="table-tab-page">
            <Breadcrumbs segments={tableBreadcrumbs(projectId, datasetId, tableId)} />
            <p className="mt-4 text-[var(--bq-muted)]">
                Table detail page coming in M2 ({projectId}.{datasetId}.{tableId}).
            </p>
            <button
                type="button"
                data-testid="open-query-from-table"
                className="mt-4 inline-flex w-fit items-center rounded-md bg-blue-600 px-3 py-1.5 text-sm font-medium"
                onClick={onQuery}
            >
                Query
            </button>
        </div>
    );
}
