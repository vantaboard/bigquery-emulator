import { apiClient } from '@/lib/api';
import {
    datasetIdsFromList,
    datasetMetadataFromBq,
    jobRefFromBq,
    projectIdsFromList,
    queryResponseFromBq,
    routineFromBq,
    routineIdsFromList,
    tableDataFromBq,
    tableIdsFromList,
    tableMetadataFromBq,
    type BqDataset,
    type BqDatasetList,
    type BqJob,
    type BqProjectList,
    type BqQueryResponse,
    type BqRoutine,
    type BqRoutineList,
    type BqTable,
    type BqTableData,
    type BqTableList,
} from '@/lib/bqRest';
import type {
    DatasetMetadata,
    ExplorerConfig,
    JobRef,
    JobSubmitConfig,
    QueryResponse,
    RoutineMetadata,
    TableDataPage,
    TableMetadata,
    TableSchemaField,
} from '@/types/api';

const defaultProject = import.meta.env.VITE_DEFAULT_PROJECT?.trim() ?? '';

function datasetPath(projectId: string, datasetId: string): string {
    return `/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}`;
}

function tablePath(projectId: string, datasetId: string, tableId: string): string {
    return `${datasetPath(projectId, datasetId)}/tables/${encodeURIComponent(tableId)}`;
}

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
        const table = await apiClient.get<BqTable>(tablePath(projectId, datasetId, tableId));
        return tableMetadataFromBq(projectId, datasetId, tableId, table);
    },

    runQuery: async (query: string, projectId: string): Promise<QueryResponse> => {
        const data = await apiClient.post<BqQueryResponse>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/queries`,
            { query, useLegacySql: false },
        );
        return queryResponseFromBq(data);
    },

    datasetMetadata: async (projectId: string, datasetId: string): Promise<DatasetMetadata> => {
        const data = await apiClient.get<BqDataset>(datasetPath(projectId, datasetId));
        return datasetMetadataFromBq(projectId, datasetId, data);
    },

    routines: async (projectId: string, datasetId: string): Promise<string[]> => {
        const data = await apiClient.get<BqRoutineList>(`${datasetPath(projectId, datasetId)}/routines`);
        return routineIdsFromList(data);
    },

    routine: async (
        projectId: string,
        datasetId: string,
        routineId: string,
    ): Promise<RoutineMetadata> => {
        const data = await apiClient.get<BqRoutine>(
            `${datasetPath(projectId, datasetId)}/routines/${encodeURIComponent(routineId)}`,
        );
        return routineFromBq(projectId, datasetId, routineId, data);
    },

    tableData: async (
        projectId: string,
        datasetId: string,
        tableId: string,
        opts: { maxResults?: number; pageToken?: string } = {},
    ): Promise<TableDataPage> => {
        const table = await apiClient.get<BqTable>(tablePath(projectId, datasetId, tableId));
        const meta = tableMetadataFromBq(projectId, datasetId, tableId, table);

        const params = new URLSearchParams();
        if (opts.maxResults !== undefined) params.set('maxResults', String(opts.maxResults));
        if (opts.pageToken) params.set('pageToken', opts.pageToken);
        const qs = params.toString() ? `?${params.toString()}` : '';

        const data = await apiClient.get<BqTableData>(`${tablePath(projectId, datasetId, tableId)}/data${qs}`);
        return tableDataFromBq(data, meta.schema);
    },

    insertTable: async (
        projectId: string,
        datasetId: string,
        body: Record<string, unknown> | { tableReference: { projectId: string; datasetId: string; tableId: string } },
    ): Promise<TableMetadata> => {
        const table = await apiClient.post<BqTable>(`${datasetPath(projectId, datasetId)}/tables`, body as Record<string, unknown>);
        const tableId =
            (body.tableReference as { tableId?: string } | undefined)?.tableId ??
            table.tableReference?.tableId ??
            '';
        return tableMetadataFromBq(projectId, datasetId, tableId, table);
    },

    submitLoadJobWithUpload: async (
        projectId: string,
        file: File,
        configuration: Record<string, unknown>,
    ): Promise<JobRef> => {
        const formData = new FormData();
        formData.append(
            'job',
            new Blob([JSON.stringify({ configuration })], { type: 'application/json' }),
        );
        formData.append('file', file);
        const data = await apiClient.postMultipart<BqJob>(
            `/upload/bigquery/v2/projects/${encodeURIComponent(projectId)}/jobs?uploadType=multipart`,
            formData,
        );
        const job = jobRefFromBq(data);
        if (job.jobId) {
            let current = job;
            for (let i = 0; i < 120; i += 1) {
                if (current.state === 'DONE') break;
                await new Promise((resolve) => setTimeout(resolve, 500));
                current = await explorerQueries.getJob(projectId, job.jobId);
            }
            if (current.errorResult?.message) {
                throw new Error(current.errorResult.message);
            }
        }
        return job;
    },

    patchTableSchema: async (
        projectId: string,
        datasetId: string,
        tableId: string,
        fields: TableSchemaField[],
        opts: { defaultValues?: Record<string, string> } = {},
    ): Promise<TableMetadata> => {
        const body = {
            schema: {
                fields: fields.map((f) => {
                    const defaultValueExpression = opts.defaultValues?.[f.name]?.trim();
                    return {
                        name: f.name,
                        type: f.type,
                        mode: f.mode,
                        ...(f.description ? { description: f.description } : {}),
                        ...(defaultValueExpression ? { defaultValueExpression } : {}),
                    };
                }),
            },
        };
        const table = await apiClient.patch<BqTable>(tablePath(projectId, datasetId, tableId), body);
        return tableMetadataFromBq(projectId, datasetId, tableId, table);
    },

    deleteTable: async (projectId: string, datasetId: string, tableId: string): Promise<void> => {
        await apiClient.del(tablePath(projectId, datasetId, tableId));
    },

    deleteDataset: async (
        projectId: string,
        datasetId: string,
        opts: { deleteContents?: boolean } = {},
    ): Promise<void> => {
        const params = opts.deleteContents ? '?deleteContents=true' : '';
        await apiClient.del(`${datasetPath(projectId, datasetId)}${params}`);
    },

    insertDataset: async (projectId: string, datasetId: string, location: string): Promise<DatasetMetadata> => {
        const data = await apiClient.post<BqDataset>(`/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets`, {
            datasetReference: { projectId, datasetId },
            location,
        });
        return datasetMetadataFromBq(projectId, datasetId, data);
    },

    submitJob: async (projectId: string, jobConfig: JobSubmitConfig): Promise<JobRef> => {
        const data = await apiClient.post<BqJob>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/jobs`,
            jobConfig,
        );
        return jobRefFromBq(data);
    },

    getJob: async (projectId: string, jobId: string): Promise<JobRef> => {
        const data = await apiClient.get<BqJob>(
            `/bigquery/v2/projects/${encodeURIComponent(projectId)}/jobs/${encodeURIComponent(jobId)}`,
        );
        return jobRefFromBq(data);
    },

    createEmulatorProject: async (_id: string): Promise<{ id: string }> => {
        throw new Error('Project creation is not supported via the BigQuery REST API');
    },
};
