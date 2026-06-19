import { apiClient } from '@/lib/api';
import {
    datasetIdsFromList,
    projectIdsFromList,
    queryResponseFromBq,
    tableIdsFromList,
    tableMetadataFromBq,
    type BqDatasetList,
    type BqProjectList,
    type BqQueryResponse,
    type BqTable,
    type BqTableList,
} from '@/lib/bqRest';
import type { ExplorerConfig, QueryResponse, TableMetadata } from '@/types/api';

const defaultProject = import.meta.env.VITE_DEFAULT_PROJECT?.trim() ?? '';

export const explorerQueries = {
    config: async (): Promise<ExplorerConfig> => ({
        allowEmulatorProjectAdmin: import.meta.env.VITE_ALLOW_EMULATOR_PROJECT_ADMIN === 'true',
    }),

    projects: async (): Promise<string[]> => {
        const data = await apiClient.get<BqProjectList>('/bigquery/v2/projects');
        const ids = projectIdsFromList(data);
        if (defaultProject && !ids.includes(defaultProject)) {
            try {
                await apiClient.get<BqDatasetList>(
                    `/bigquery/v2/projects/${encodeURIComponent(defaultProject)}/datasets`,
                );
                ids.unshift(defaultProject);
            } catch {
                /* default project has no datasets */
            }
        }
        return [...new Set(ids)];
    },

    datasets: async (projectId: string): Promise<string[]> => {
        const data = await apiClient.get<BqDatasetList>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets`,
        );
        return datasetIdsFromList(data);
    },

    tables: async (projectId: string, datasetId: string): Promise<string[]> => {
        const data = await apiClient.get<BqTableList>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables`,
        );
        return tableIdsFromList(data);
    },

    tableSchema: async (projectId: string, datasetId: string, tableId: string): Promise<TableMetadata> => {
        const table = await apiClient.get<BqTable>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables/${encodeURIComponent(tableId)}`,
        );
        return tableMetadataFromBq(projectId, datasetId, tableId, table);
    },

    runQuery: async (query: string, projectId: string): Promise<QueryResponse> => {
        const data = await apiClient.post<BqQueryResponse>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/queries`,
            { query, useLegacySql: false },
        );
        return queryResponseFromBq(data);
    },

    createEmulatorProject: async (_id: string): Promise<{ id: string }> => {
        throw new Error('Project creation is not supported via the BigQuery REST API');
    },
};
