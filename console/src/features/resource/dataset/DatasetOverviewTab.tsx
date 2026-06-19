import { useState } from 'react';

import { TabBar } from '@/components/ui/TabBar';
import { UnplannedTab } from '@/components/ui/UnplannedTab';

import { DatasetRoutinesSubTab } from './DatasetRoutinesSubTab';
import { DatasetTablesSubTab } from './DatasetTablesSubTab';

const OVERVIEW_TABS = [
    { id: 'tables', label: 'Tables', testId: 'dataset-overview-tab-tables' },
    { id: 'routines', label: 'Routines', testId: 'dataset-overview-tab-routines' },
    { id: 'graphs', label: 'Graphs', testId: 'dataset-overview-tab-graphs' },
    { id: 'models', label: 'Models', testId: 'dataset-overview-tab-models' },
] as const;

type OverviewSubTab = (typeof OVERVIEW_TABS)[number]['id'];

interface DatasetOverviewTabProps {
    projectId: string;
    datasetId: string;
}

export function DatasetOverviewTab({ projectId, datasetId }: DatasetOverviewTabProps) {
    const [subTab, setSubTab] = useState<OverviewSubTab>('tables');

    return (
        <div data-testid="dataset-tab-overview">
            <TabBar tabs={[...OVERVIEW_TABS]} activeId={subTab} onChange={(id) => setSubTab(id as OverviewSubTab)} />
            <div className="mt-4">
                {subTab === 'tables' ? (
                    <DatasetTablesSubTab projectId={projectId} datasetId={datasetId} />
                ) : null}
                {subTab === 'routines' ? (
                    <DatasetRoutinesSubTab projectId={projectId} datasetId={datasetId} />
                ) : null}
                {subTab === 'graphs' ? (
                    <UnplannedTab title="Graphs" message="Graph visualization is not planned yet." />
                ) : null}
                {subTab === 'models' ? (
                    <UnplannedTab title="Models" message="Model management is not planned yet." />
                ) : null}
            </div>
        </div>
    );
}
