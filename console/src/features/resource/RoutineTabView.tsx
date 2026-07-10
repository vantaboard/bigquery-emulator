import { useQueryClient } from '@tanstack/react-query';
import { RefreshCw } from 'lucide-react';

import { ActionToolbar, ToolbarButton } from '@/components/ui/ActionToolbar';
import { Breadcrumbs, routineBreadcrumbs } from '@/features/workspace/components/Breadcrumbs';

import { RoutineDetailsTab } from './routine/RoutineDetailsTab';

export interface RoutineTabViewProps {
    projectId: string;
    datasetId: string;
    routineId: string;
}

export function RoutineTabView({ projectId, datasetId, routineId }: RoutineTabViewProps) {
    const queryClient = useQueryClient();

    const onRefresh = () => {
        void queryClient.invalidateQueries({ queryKey: ['explorer', 'routine', projectId, datasetId, routineId] });
    };

    return (
        <div className="flex flex-1 flex-col p-6" data-testid="routine-tab-page">
            <Breadcrumbs segments={routineBreadcrumbs(projectId, datasetId, routineId)} />

            <ActionToolbar className="mt-4">
                <ToolbarButton icon={RefreshCw} label="Refresh" onClick={onRefresh} />
            </ActionToolbar>

            <div className="mt-4 min-h-0 flex-1">
                <RoutineDetailsTab projectId={projectId} datasetId={datasetId} routineId={routineId} />
            </div>
        </div>
    );
}
