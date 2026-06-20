export const EXPLORER_TABLES_CHANGED = 'explorer:tables-changed';

export function notifyTablesChanged(projectId: string, datasetId: string) {
    window.dispatchEvent(
        new CustomEvent(EXPLORER_TABLES_CHANGED, { detail: { projectId, datasetId } }),
    );
}
