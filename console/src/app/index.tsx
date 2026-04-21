import { BrowserRouter } from 'react-router';

import { AppProvider } from '@/app/provider';
import { AppRouter } from '@/app/router';

export function App() {
    return (
        <BrowserRouter>
            <AppProvider>
                <AppRouter />
            </AppProvider>
        </BrowserRouter>
    );
}
