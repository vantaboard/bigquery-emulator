import type {
  AnalyzeRequest,
  AnalyzeResponse,
  CompleteRequest,
  CompleteResponse,
  FormatRequest,
  FormatResponse,
  ParseRequest,
  ParseResponse,
  SqlCapabilities,
} from './types.js';

export interface SqlToolsClientOptions {
  baseUrl: string;
  token?: string;
  fetchImpl?: typeof fetch;
}

export class SqlToolsClient {
  private readonly baseUrl: string;
  private readonly token?: string;
  private readonly fetchImpl: typeof fetch;
  private probeCache: { available: boolean; capabilities?: SqlCapabilities } | null = null;

  constructor(options: SqlToolsClientOptions) {
    this.baseUrl = options.baseUrl.replace(/\/$/, '');
    this.token = options.token?.trim() || undefined;
    this.fetchImpl = options.fetchImpl ?? fetch;
  }

  resetProbe(): void {
    this.probeCache = null;
  }

  private url(path: string): string {
    const normalized = path.startsWith('/') ? path : `/${path}`;
    return `${this.baseUrl}${normalized}`;
  }

  private headers(extra?: Record<string, string>): Record<string, string> {
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      ...extra,
    };
    if (this.token) {
      headers['X-BigQuery-Emulator-SqlTools-Token'] = this.token;
    }
    return headers;
  }

  private async request<T>(path: string, init?: RequestInit): Promise<T> {
    const response = await this.fetchImpl(this.url(path), {
      ...init,
      headers: {
        ...this.headers(),
        ...(init?.headers as Record<string, string> | undefined),
      },
    });
    const text = await response.text();
    const data = text ? (JSON.parse(text) as unknown) : null;
    if (!response.ok) {
      const message =
        data && typeof data === 'object' && data !== null && 'message' in data
          ? String((data as { message: unknown }).message)
          : `HTTP ${response.status}`;
      throw new Error(message);
    }
    return data as T;
  }

  async probeCapabilities(): Promise<boolean> {
    if (this.probeCache !== null) {
      return this.probeCache.available;
    }
    try {
      const capabilities = await this.request<SqlCapabilities>(
        '/api/emulator/sql/capabilities',
        { method: 'GET' },
      );
      this.probeCache = {
        available: capabilities.sqlTools === true,
        capabilities,
      };
      return this.probeCache.available;
    } catch {
      this.probeCache = { available: false };
      return false;
    }
  }

  getCachedCapabilities(): SqlCapabilities | null {
    return this.probeCache?.capabilities ?? null;
  }

  isAvailable(): boolean {
    return this.probeCache?.available === true;
  }

  async format(request: FormatRequest): Promise<FormatResponse> {
    return this.request<FormatResponse>('/api/emulator/sql/format', {
      method: 'POST',
      body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
  }

  async parse(request: ParseRequest): Promise<ParseResponse> {
    return this.request<ParseResponse>('/api/emulator/sql/parse', {
      method: 'POST',
      body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
  }

  async complete(request: CompleteRequest): Promise<CompleteResponse> {
    return this.request<CompleteResponse>('/api/emulator/sql/complete', {
      method: 'POST',
      body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
  }

  async analyze(request: AnalyzeRequest): Promise<AnalyzeResponse> {
    return this.request<AnalyzeResponse>('/api/emulator/sql/analyze', {
      method: 'POST',
      body: JSON.stringify({ offsetUnit: 'utf16', ...request }),
    });
  }
}

export function completionKindToLsp(kind: string): number {
  switch (kind) {
    case 'keyword':
      return 14;
    case 'function':
      return 3;
    case 'column':
      return 10;
    case 'table':
    case 'view':
      return 7;
    case 'dataset':
      return 6;
    case 'routine':
    case 'procedure':
      return 9;
    default:
      return 1;
  }
}
