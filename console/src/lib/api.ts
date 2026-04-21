import { webEnv } from './env';

const base = webEnv.apiBaseUrl;

export class ApiClient {
    private readonly baseUrl: string;

    constructor(baseUrl: string = base) {
        this.baseUrl = baseUrl;
    }

    private url(path: string): string {
        if (path.startsWith('http')) return path;
        const p = path.startsWith('/') ? path : `/${path}`;
        return `${this.baseUrl}${p}`;
    }

    async fetchJson<T>(path: string, init?: RequestInit): Promise<T> {
        const res = await fetch(this.url(path), {
            ...init,
            headers: {
                'Content-Type': 'application/json',
                ...init?.headers,
            },
        });
        const text = await res.text();
        const data = text ? (JSON.parse(text) as unknown) : null;
        if (!res.ok) {
            const msg =
                data && typeof data === 'object' && data !== null && 'error' in data
                    ? String((data as { error: unknown }).error)
                    : `HTTP ${res.status}`;
            throw new Error(msg);
        }
        return data as T;
    }

    get<T>(path: string) {
        return this.fetchJson<T>(path, { method: 'GET' });
    }

    post<T>(path: string, body?: unknown) {
        return this.fetchJson<T>(path, {
            method: 'POST',
            body: body !== undefined ? JSON.stringify(body) : undefined,
        });
    }
}

export const apiClient = new ApiClient();
