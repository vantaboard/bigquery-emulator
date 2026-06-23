export const EXPLORER_TABLES_CHANGED = 'explorer:tables-changed';
export const EXPLORER_DATASETS_CHANGED = 'explorer:datasets-changed';
export const EXPLORER_ROUTINES_CHANGED = 'explorer:routines-changed';

export function notifyTablesChanged(projectId: string, datasetId: string) {
    window.dispatchEvent(
        new CustomEvent(EXPLORER_TABLES_CHANGED, { detail: { projectId, datasetId } }),
    );
}

export function notifyDatasetsChanged(projectId: string) {
    window.dispatchEvent(new CustomEvent(EXPLORER_DATASETS_CHANGED, { detail: { projectId } }));
}

export function notifyRoutinesChanged(projectId: string, datasetId: string) {
    window.dispatchEvent(
        new CustomEvent(EXPLORER_ROUTINES_CHANGED, { detail: { projectId, datasetId } }),
    );
}
