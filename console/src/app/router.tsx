import { Route, Routes } from 'react-router';

import { QueryTabPage } from '@/features/query/QueryTabPage';
import { DatasetTabPage } from '@/features/resource/DatasetTabPage';
import { TableTabPage } from '@/features/resource/TableTabPage';
import { WorkspaceHomePage } from '@/features/workspace/WorkspaceHomePage';
import { WorkspaceLayout } from '@/features/workspace/WorkspaceLayout';
import { WorkspaceProvider } from '@/features/workspace/store';

export function AppRouter() {
    return (
        <WorkspaceProvider>
            <Routes>
                <Route element={<WorkspaceLayout />}>
                    <Route path="/" element={<WorkspaceHomePage />} />
                    <Route path="/query/:tabId" element={<QueryTabPage />} />
                    <Route path="/project/:projectId/dataset/:datasetId" element={<DatasetTabPage />} />
                    <Route
                        path="/project/:projectId/dataset/:datasetId/table/:tableId"
                        element={<TableTabPage />}
                    />
                </Route>
            </Routes>
        </WorkspaceProvider>
    );
}
