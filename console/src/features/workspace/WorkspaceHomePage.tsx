import { useWorkspace } from '@/features/workspace/store';

export function WorkspaceHomePage() {
    const { tabs } = useWorkspace();

    if (tabs.length === 0) {
        return (
            <div className="flex flex-1 items-center justify-center p-8 text-[var(--bq-muted)]">
                Select a table from the tree or open a new query tab (+).
            </div>
        );
    }

    return (
        <div className="flex flex-1 items-center justify-center p-8 text-[var(--bq-muted)]">
            Select a workspace tab above.
        </div>
    );
}
