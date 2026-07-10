import { useParams } from 'react-router';

import { DatasetTabView } from '@/features/resource/DatasetTabView';

export function DatasetTabPage() {
    const { projectId = '', datasetId = '' } = useParams();
    return <DatasetTabView projectId={projectId} datasetId={datasetId} />;
}
