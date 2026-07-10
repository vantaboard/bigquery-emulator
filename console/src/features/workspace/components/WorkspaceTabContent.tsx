import { QueryTab } from '@/features/query/QueryTab';
import { DatasetTabView } from '@/features/resource/DatasetTabView';
import { RoutineTabView } from '@/features/resource/RoutineTabView';
import { TableTabView } from '@/features/resource/TableTabView';
import type { WorkspaceTab } from '@/features/workspace/types';

interface WorkspaceTabContentProps {
    tab: WorkspaceTab;
}

export function WorkspaceTabContent({ tab }: WorkspaceTabContentProps) {
    switch (tab.type) {
        case 'query':
            return <QueryTab tab={tab} />;
        case 'dataset':
            return <DatasetTabView projectId={tab.projectId} datasetId={tab.datasetId} />;
        case 'table':
            return (
                <TableTabView
                    projectId={tab.projectId}
                    datasetId={tab.datasetId}
                    tableId={tab.tableId}
                />
            );
        case 'routine':
            return (
                <RoutineTabView
                    projectId={tab.projectId}
                    datasetId={tab.datasetId}
                    routineId={tab.routineId}
                />
            );
    }
}
