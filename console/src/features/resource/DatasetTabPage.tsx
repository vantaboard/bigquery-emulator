import { useParams } from 'react-router';

import { Breadcrumbs, datasetBreadcrumbs } from '@/features/workspace/components/Breadcrumbs';

export function DatasetTabPage() {
    const { projectId = '', datasetId = '' } = useParams();

    return (
        <div className="flex flex-1 flex-col p-6" data-testid="dataset-tab-page">
            <Breadcrumbs segments={datasetBreadcrumbs(projectId, datasetId)} />
            <p className="mt-4 text-[var(--bq-muted)]">
                Dataset detail page coming in M2 ({projectId}.{datasetId}).
            </p>
        </div>
    );
}
