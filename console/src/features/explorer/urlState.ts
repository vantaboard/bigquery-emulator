export type ResultsTab = 'info' | 'results' | 'json';

export interface ExplorerUrlState {
    project: string;
    dataset: string;
    table: string;
    results: ResultsTab;
    query: string;
}

const LEGACY_TAB_MAP: Record<string, ResultsTab> = {
    infoTab: 'info',
    resultsTab: 'results',
    jsonTab: 'json',
};

function decodeBase64Utf8(b64: string): string {
    try {
        const binary = atob(b64);
        const bytes = new Uint8Array(binary.length);
        for (let i = 0; i < binary.length; i += 1) {
            bytes[i] = binary.charCodeAt(i);
        }
        return new TextDecoder().decode(bytes);
    } catch {
        return '';
    }
}

function encodeBase64Utf8(text: string): string {
    try {
        const bytes = new TextEncoder().encode(text);
        let binary = '';
        for (let i = 0; i < bytes.length; i += 1) {
            binary += String.fromCharCode(bytes[i]);
        }
        return btoa(binary);
    } catch {
        return '';
    }
}

export function parseExplorerSearchParams(search: string): ExplorerUrlState {
    const params = new URLSearchParams(search);
    const project = params.get('project') ?? '';
    const dataset = params.get('dataset') ?? '';
    const table = params.get('table') ?? '';
    let resultsRaw = params.get('results') ?? 'info';
    if (resultsRaw in LEGACY_TAB_MAP) {
        resultsRaw = LEGACY_TAB_MAP[resultsRaw];
    }
    const results: ResultsTab =
        resultsRaw === 'results' || resultsRaw === 'json' || resultsRaw === 'info' ? resultsRaw : 'info';

    let query = '';
    const q = params.get('query');
    if (q) {
        query = decodeBase64Utf8(q);
    }

    return { project, dataset, table, results, query };
}

export function buildExplorerSearchParams(state: ExplorerUrlState): string {
    if (!state.project || !state.dataset || !state.table) {
        return '';
    }
    const params = new URLSearchParams();
    params.set('project', state.project);
    params.set('dataset', state.dataset);
    params.set('table', state.table);
    params.set('results', state.results);
    if (state.query.trim()) {
        params.set('query', encodeBase64Utf8(state.query));
    }
    return params.toString();
}
