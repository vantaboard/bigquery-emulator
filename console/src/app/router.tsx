import { Route, Routes } from 'react-router';

import { ExplorerPage } from '@/features/explorer/ExplorerPage';

export function AppRouter() {
    return (
        <Routes>
            <Route path="/" element={<ExplorerPage />} />
        </Routes>
    );
}
