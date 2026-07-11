import { useEffect, useRef } from 'react';
import { Outlet, useNavigate, useParams, useSearchParams } from 'react-router';

import { parseExplorerSearchParams } from '@/features/explorer/urlState';

import { ResourceSidebar } from './components/ResourceSidebar';
import { WorkspaceSplitPanes } from './components/WorkspaceSplitPanes';
import { WorkspaceTabBar } from './components/WorkspaceTabBar';
import { tabRoutePath, useWorkspace } from './store';
import { defaultSql, type QuerySubTab } from './types';

export function WorkspaceLayout() {
    const navigate = useNavigate();
    const params = useParams();
    const [searchParams] = useSearchParams();
    const { tabs, session, activateTab, openTableTab, openDatasetTab, openRoutineTab, openQueryFromShare } =
        useWorkspace();
    const shareHandled = useRef(false);
    const initialRouteHandled = useRef(false);

    useEffect(() => {
        if (shareHandled.current) return;
        const st = parseExplorerSearchParams(searchParams.toString());
        if (!st.project || !st.dataset || !st.table) return;
        shareHandled.current = true;
        const subTab: QuerySubTab = st.results === 'json' ? 'json' : 'results';
        const sql = st.query.trim() ? st.query : defaultSql(st.project, st.dataset, st.table);
        const tabId = openQueryFromShare({
            projectId: st.project,
            datasetId: st.dataset,
            tableId: st.table,
            sql,
            subTab,
        });
        navigate(`/query/${encodeURIComponent(tabId)}`, { replace: true });
    }, [navigate, openQueryFromShare, searchParams]);

    useEffect(() => {
        if (shareHandled.current && !params.tabId && !params.projectId) return;

        if (params.tabId) {
            const tab = tabs.find((t) => t.type === 'query' && t.id === params.tabId);
            if (tab) activateTab(tab.id);
            return;
        }

        if (params.projectId && params.datasetId && params.tableId) {
            openTableTab(params.projectId, params.datasetId, params.tableId);
            return;
        }

        if (params.projectId && params.datasetId && params.routineId) {
            openRoutineTab(params.projectId, params.datasetId, params.routineId);
            return;
        }

        if (params.projectId && params.datasetId) {
            openDatasetTab(params.projectId, params.datasetId);
        }
    }, [
        activateTab,
        openDatasetTab,
        openRoutineTab,
        openTableTab,
        params.datasetId,
        params.projectId,
        params.routineId,
        params.tabId,
        params.tableId,
        tabs,
    ]);

    useEffect(() => {
        if (initialRouteHandled.current) return;
        if (shareHandled.current) {
            initialRouteHandled.current = true;
            return;
        }
        if (params.tabId || params.projectId) {
            initialRouteHandled.current = true;
            return;
        }
        if (!session.activeTabId) {
            initialRouteHandled.current = true;
            return;
        }
        const tab = tabs.find((t) => t.id === session.activeTabId);
        if (tab) {
            navigate(tabRoutePath(tab), { replace: true });
        }
        initialRouteHandled.current = true;
    }, [navigate, params.projectId, params.tabId, session.activeTabId, tabs]);

    return (
        <div className="flex min-h-screen flex-col">
            <header className="flex items-center gap-3 border-b border-[var(--bq-border)] bg-[var(--bq-surface)] px-4 py-3">
                <h1 className="text-lg font-semibold">BigQuery Explorer</h1>
            </header>

            <div className="flex min-h-0 flex-1">
                <ResourceSidebar />
                <div className="flex min-w-0 flex-1 flex-col">
                    {!session.split ? <WorkspaceTabBar /> : null}
                    <main className="flex min-h-0 flex-1 flex-col">
                        {session.split ? <WorkspaceSplitPanes /> : <Outlet />}
                    </main>
                </div>
            </div>
        </div>
    );
}
