import type { SqlCatalog, TableMetadata } from '@bigquery-emulator/vscode-shared';

interface DatasetListResponse {
  datasets?: Array<{ datasetReference?: { datasetId?: string } }>;
}

interface TableListResponse {
  tables?: Array<{ tableReference?: { tableId?: string } }>;
}

interface RoutineListResponse {
  routines?: Array<{ routineReference?: { routineId?: string } }>;
}

export class CatalogService {
  private baseUrl: string;
  private readonly fetchImpl: typeof fetch;
  private readonly cache = new Map<string, Promise<SqlCatalog>>();
  private readonly tableCache = new Map<string, Promise<TableMetadata | null>>();

  constructor(baseUrl: string, fetchImpl: typeof fetch = fetch) {
    this.baseUrl = baseUrl.replace(/\/$/, '');
    this.fetchImpl = fetchImpl;
  }

  updateBaseUrl(baseUrl: string): void {
    this.baseUrl = baseUrl.replace(/\/$/, '');
    this.cache.clear();
    this.tableCache.clear();
  }

  private projectBase(projectId: string): string {
    return `${this.baseUrl}/bigquery/v2/projects/${encodeURIComponent(projectId)}`;
  }

  async load(projectId: string): Promise<SqlCatalog> {
    const existing = this.cache.get(projectId);
    if (existing) {
      return existing;
    }

    const promise = this.buildCatalog(projectId);
    this.cache.set(projectId, promise);
    return promise;
  }

  async getTable(
    projectId: string,
    datasetId: string,
    tableId: string,
  ): Promise<TableMetadata | null> {
    const key = `${projectId}.${datasetId}.${tableId}`;
    const existing = this.tableCache.get(key);
    if (existing) {
      return existing;
    }

    const promise = this.fetchTable(projectId, datasetId, tableId);
    this.tableCache.set(key, promise);
    return promise;
  }

  private async fetchJson<T>(url: string, init?: RequestInit): Promise<T> {
    const response = await this.fetchImpl(url, {
      ...init,
      headers: {
        'Content-Type': 'application/json',
        ...(init?.headers as Record<string, string> | undefined),
      },
    });
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    return (await response.json()) as T;
  }

  private async fetchTable(
    projectId: string,
    datasetId: string,
    tableId: string,
  ): Promise<TableMetadata | null> {
    try {
      const url = `${this.projectBase(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables/${encodeURIComponent(tableId)}`;
      return await this.fetchJson<TableMetadata>(url, { method: 'GET' });
    } catch {
      return null;
    }
  }

  private async buildCatalog(projectId: string): Promise<SqlCatalog> {
    const schema: Record<string, string[]> = {};
    const qualifiedTables: string[] = [];
    const routines: string[] = [];

    let datasets: string[] = [];
    try {
      const response = await this.fetchJson<DatasetListResponse>(
        `${this.projectBase(projectId)}/datasets`,
        { method: 'GET' },
      );
      datasets =
        response.datasets
          ?.map((entry) => entry.datasetReference?.datasetId)
          .filter((id): id is string => Boolean(id)) ?? [];
    } catch {
      return { schema, qualifiedTables, routines };
    }

    await Promise.all(
      datasets.map(async (datasetId) => {
        let tableIds: string[] = [];
        try {
          const response = await this.fetchJson<TableListResponse>(
            `${this.projectBase(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables`,
            { method: 'GET' },
          );
          tableIds =
            response.tables
              ?.map((entry) => entry.tableReference?.tableId)
              .filter((id): id is string => Boolean(id)) ?? [];
        } catch {
          return;
        }

        await Promise.all(
          tableIds.map(async (tableId) => {
            qualifiedTables.push(`${projectId}.${datasetId}.${tableId}`);
            try {
              const metadata = await this.getTable(projectId, datasetId, tableId);
              const columns = metadata?.schema?.fields?.map((field) => field.name) ?? [];
              schema[tableId] = columns;
              schema[`${datasetId}.${tableId}`] = columns;
              schema[`${projectId}.${datasetId}.${tableId}`] = columns;
            } catch {
              schema[tableId] = [];
            }
          }),
        );

        try {
          const response = await this.fetchJson<RoutineListResponse>(
            `${this.projectBase(projectId)}/datasets/${encodeURIComponent(datasetId)}/routines`,
            { method: 'GET' },
          );
          for (const entry of response.routines ?? []) {
            const routineId = entry.routineReference?.routineId;
            if (!routineId) {
              continue;
            }
            routines.push(`${datasetId}.${routineId}`);
            routines.push(`${projectId}.${datasetId}.${routineId}`);
          }
        } catch {
          /* routines optional */
        }
      }),
    );

    return { schema, qualifiedTables, routines };
  }
}

export function catalogCompletions(
  catalog: SqlCatalog,
  prefix: string,
): Array<{ label: string; kind: string; insertText: string; detail?: string }> {
  const normalized = prefix.toLowerCase();
  const options: Array<{ label: string; kind: string; insertText: string; detail?: string }> =
    [];

  for (const [table, columns] of Object.entries(catalog.schema)) {
    if (table.toLowerCase().includes(normalized) || normalized === '') {
      options.push({ label: table, kind: 'table', insertText: table, detail: 'table' });
    }
    for (const column of columns) {
      if (column.toLowerCase().startsWith(normalized) || normalized.endsWith('.')) {
        options.push({
          label: column,
          kind: 'column',
          insertText: column,
          detail: table,
        });
      }
    }
  }

  for (const qualified of catalog.qualifiedTables) {
    if (qualified.toLowerCase().includes(normalized)) {
      options.push({
        label: qualified,
        kind: 'table',
        insertText: qualified,
        detail: 'table',
      });
    }
  }

  for (const routine of catalog.routines) {
    if (routine.toLowerCase().includes(normalized)) {
      options.push({
        label: routine,
        kind: 'routine',
        insertText: `${routine}(`,
        detail: 'routine',
      });
    }
  }

  return options;
}
