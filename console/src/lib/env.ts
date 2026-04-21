export interface WebEnv {
    apiBaseUrl: string;
    dev: boolean;
}

export function getWebEnv(): WebEnv {
    const raw = import.meta.env.VITE_API_URL;
    const trimmed = typeof raw === 'string' ? raw.trim() : '';
    return {
        apiBaseUrl: trimmed === '' ? '' : trimmed.replace(/\/$/, ''),
        dev: import.meta.env.DEV,
    };
}

export const webEnv = getWebEnv();
