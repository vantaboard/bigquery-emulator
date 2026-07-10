import { useParams } from 'react-router';

import { RoutineTabView } from '@/features/resource/RoutineTabView';

export function RoutineTabPage() {
    const { projectId = '', datasetId = '', routineId = '' } = useParams();
    return <RoutineTabView projectId={projectId} datasetId={datasetId} routineId={routineId} />;
}
