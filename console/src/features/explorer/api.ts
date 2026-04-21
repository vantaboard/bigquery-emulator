import { apiClient } from '@/lib/api';
import type { ExplorerConfig, QueryResponse, TableMetadata } from '@/types/api';

export const explorerQueries = {
    config: () => apiClient.get<ExplorerConfig>('/api/config'),
    projects: () => apiClient.get<string[]>('/api/projects'),
    datasets: (projectId: string) => apiClient.get<string[]>(`/api/projects/${encodeURIComponent(projectId)}/datasets`),
    tables: (projectId: string, datasetId: string) =>
        apiClient.get<string[]>(
            `/api/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables`,
        ),
    tableSchema: (projectId: string, datasetId: string, tableId: string) =>
        apiClient.get<TableMetadata>(
            `/api/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables/${encodeURIComponent(tableId)}/schema`,
        ),
    runQuery: (query: string) => apiClient.post<QueryResponse>('/api/query', { query }),
    createEmulatorProject: (id: string) => apiClient.post<{ id: string }>('/api/emulator/projects', { id }),
};
