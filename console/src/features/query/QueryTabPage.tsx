import { useParams } from 'react-router';

import { QueryTab } from '@/features/query/QueryTab';
import { useWorkspace } from '@/features/workspace/store';

export function QueryTabPage() {
    const { tabId = '' } = useParams();
    const { tabs } = useWorkspace();
    const tab = tabs.find((t) => t.type === 'query' && t.id === tabId);

    if (!tab || tab.type !== 'query') {
        return (
            <div className="flex flex-1 items-center justify-center p-8 text-[var(--bq-muted)]">
                Query tab not found.
            </div>
        );
    }

    return <QueryTab tab={tab} />;
}
