import { useParams } from 'react-router';

import { TableTabView } from '@/features/resource/TableTabView';

export function TableTabPage() {
    const { projectId = '', datasetId = '', tableId = '' } = useParams();
    return <TableTabView projectId={projectId} datasetId={datasetId} tableId={tableId} />;
}
