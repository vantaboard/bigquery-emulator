import { explorerQueries } from '@/features/explorer/api';

export interface SqlCatalog {
    /** Table or view id → column names (unqualified). */
    schema: Record<string, readonly string[]>;
    /** Fully qualified `project.dataset.table` names for custom completion. */
    qualifiedTables: string[];
    /** Routine names as `dataset.routine`. */
    routines: string[];
}

const catalogCache = new Map<string, Promise<SqlCatalog>>();

function cacheKey(projectId: string): string {
    return projectId;
}

export function clearSqlCatalogCache(projectId?: string): void {
    if (projectId) {
        catalogCache.delete(cacheKey(projectId));
        return;
    }
    catalogCache.clear();
}

export async function loadSqlCatalog(projectId: string): Promise<SqlCatalog> {
    const key = cacheKey(projectId);
    const existing = catalogCache.get(key);
    if (existing) return existing;

    const promise = (async (): Promise<SqlCatalog> => {
        const schema: Record<string, string[]> = {};
        const qualifiedTables: string[] = [];
        const routines: string[] = [];

        let datasets: string[] = [];
        try {
            datasets = await explorerQueries.datasets(projectId);
        } catch {
            return { schema, qualifiedTables, routines };
        }

        await Promise.all(
            datasets.map(async (datasetId) => {
                let tableIds: string[] = [];
                try {
                    tableIds = await explorerQueries.tables(projectId, datasetId);
                } catch {
                    return;
                }

                await Promise.all(
                    tableIds.map(async (tableId) => {
                        qualifiedTables.push(`${projectId}.${datasetId}.${tableId}`);
                        try {
                            const meta = await explorerQueries.tableSchema(projectId, datasetId, tableId);
                            const columns = meta.schema.map((f) => f.name);
                            schema[tableId] = columns;
                            schema[`${datasetId}.${tableId}`] = columns;
                            schema[`${projectId}.${datasetId}.${tableId}`] = columns;
                        } catch {
                            schema[tableId] = [];
                        }
                    }),
                );

                try {
                    const routineIds = await explorerQueries.routines(projectId, datasetId);
                    for (const routineId of routineIds) {
                        routines.push(`${datasetId}.${routineId}`);
                        routines.push(`${projectId}.${datasetId}.${routineId}`);
                    }
                } catch {
                    /* routines optional */
                }
            }),
        );

        return { schema, qualifiedTables, routines };
    })();

    catalogCache.set(key, promise);
    return promise;
}
