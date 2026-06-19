import http from 'node:http';
import { URL } from 'node:url';

const BQ_URL = (process.env.BIGQUERY_EMULATOR_URL ?? 'http://bigquery:9050').replace(/\/$/, '');
const PORT = Number(process.env.EXPLORER_API_PORT ?? 8090);
const DEFAULT_PROJECT = process.env.EXPLORER_DEFAULT_PROJECT ?? 'local-project';

async function bqJson(path, init) {
    const res = await fetch(`${BQ_URL}${path}`, {
        ...init,
        headers: { 'Content-Type': 'application/json', ...init?.headers },
    });
    const text = await res.text();
    const data = text ? JSON.parse(text) : null;
    if (!res.ok) {
        const msg =
            data?.error?.message ??
            (typeof data?.error === 'string' ? data.error : null) ??
            `HTTP ${res.status}`;
        const err = new Error(msg);
        err.status = res.status;
        throw err;
    }
    return data;
}

async function listProjects() {
    try {
        const data = await bqJson('/bigquery/v2/projects');
        const ids = (data.projects ?? []).map((p) => p.id).filter(Boolean);
        if (!ids.includes(DEFAULT_PROJECT)) {
            ids.unshift(DEFAULT_PROJECT);
        }
        return [...new Set(ids)];
    } catch {
        return [DEFAULT_PROJECT];
    }
}

async function listDatasets(projectId) {
    const data = await bqJson(`/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets`);
    return (data.datasets ?? []).map((d) => d.datasetReference?.datasetId).filter(Boolean);
}

async function listTables(projectId, datasetId) {
    const data = await bqJson(
        `/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables`,
    );
    return (data.tables ?? []).map((t) => t.tableReference?.tableId).filter(Boolean);
}

function parseBqValue(raw, type) {
    if (raw === null || raw === undefined) return null;
    const upper = (type ?? '').toUpperCase();
    if (upper.includes('INT') || upper === 'NUMERIC' || upper === 'BIGNUMERIC') {
        const n = Number(raw);
        return Number.isNaN(n) ? raw : n;
    }
    if (upper === 'FLOAT' || upper === 'FLOAT64') {
        const n = Number(raw);
        return Number.isNaN(n) ? raw : n;
    }
    if (upper === 'BOOLEAN' || upper === 'BOOL') {
        return raw === 'true' || raw === true;
    }
    return raw;
}

function tableMetadata(projectId, datasetId, tableId, table) {
    const fields = table.schema?.fields ?? [];
    const schema = fields.map((f) => ({
        name: f.name,
        type: f.type,
        mode: f.mode ?? 'NULLABLE',
        description: f.description ?? null,
    }));
    const ms = (t) => (t ? new Date(Number(t)).toISOString() : new Date(0).toISOString());
    return {
        schema,
        numRows: Number(table.numRows ?? 0),
        numBytes: Number(table.numBytes ?? 0),
        creationTime: ms(table.creationTime),
        lastModified: ms(table.lastModifiedTime ?? table.creationTime),
        description: table.description ?? '',
        type: table.type ?? 'TABLE',
        location: table.location ?? '',
        fullyQualifiedName: `${projectId}.${datasetId}.${tableId}`,
    };
}

async function runQuery(query, projectId) {
    const data = await bqJson(`/bigquery/v2/projects/${encodeURIComponent(projectId)}/queries`, {
        method: 'POST',
        body: JSON.stringify({ query, useLegacySql: false }),
    });
    const fields = data.schema?.fields ?? [];
    const columns = fields.map((f) => f.name);
    const rows = (data.rows ?? []).map((row) => {
        const out = {};
        columns.forEach((col, i) => {
            const cell = row.f?.[i];
            const raw = cell?.v ?? null;
            out[col] = parseBqValue(raw, fields[i]?.type);
        });
        return out;
    });
    return {
        columns,
        rows,
        total_rows: Number(data.totalRows ?? rows.length),
    };
}

function sendJson(res, status, body) {
    const payload = JSON.stringify(body);
    res.writeHead(status, {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(payload),
    });
    res.end(payload);
}

function sendError(res, err) {
    sendJson(res, err.status && err.status >= 400 && err.status < 600 ? err.status : 500, {
        error: err.message ?? 'Internal error',
    });
}

const server = http.createServer(async (req, res) => {
    try {
        const url = new URL(req.url ?? '/', `http://${req.headers.host ?? 'localhost'}`);
        const path = url.pathname;

        if (req.method === 'GET' && path === '/healthz') {
            sendJson(res, 200, { status: 'ok' });
            return;
        }

        if (req.method === 'GET' && path === '/api/config') {
            sendJson(res, 200, { allowEmulatorProjectAdmin: false });
            return;
        }

        if (req.method === 'GET' && path === '/api/projects') {
            sendJson(res, 200, await listProjects());
            return;
        }

        const datasetsMatch = path.match(/^\/api\/projects\/([^/]+)\/datasets$/);
        if (req.method === 'GET' && datasetsMatch) {
            sendJson(res, 200, await listDatasets(decodeURIComponent(datasetsMatch[1])));
            return;
        }

        const tablesMatch = path.match(/^\/api\/projects\/([^/]+)\/datasets\/([^/]+)\/tables$/);
        if (req.method === 'GET' && tablesMatch) {
            sendJson(
                res,
                200,
                await listTables(decodeURIComponent(tablesMatch[1]), decodeURIComponent(tablesMatch[2])),
            );
            return;
        }

        const schemaMatch = path.match(/^\/api\/projects\/([^/]+)\/datasets\/([^/]+)\/tables\/([^/]+)\/schema$/);
        if (req.method === 'GET' && schemaMatch) {
            const [, projectId, datasetId, tableId] = schemaMatch.map(decodeURIComponent);
            const table = await bqJson(
                `/bigquery/v2/projects/${encodeURIComponent(projectId)}/datasets/${encodeURIComponent(datasetId)}/tables/${encodeURIComponent(tableId)}`,
            );
            sendJson(res, 200, tableMetadata(projectId, datasetId, tableId, table));
            return;
        }

        if (req.method === 'POST' && path === '/api/query') {
            const chunks = [];
            for await (const chunk of req) {
                chunks.push(chunk);
            }
            const body = JSON.parse(Buffer.concat(chunks).toString('utf8') || '{}');
            const query = String(body.query ?? '');
            const projectId = String(body.project_id ?? DEFAULT_PROJECT);
            if (!query.trim()) {
                sendJson(res, 400, { error: 'query is required' });
                return;
            }
            sendJson(res, 200, await runQuery(query, projectId));
            return;
        }

        sendJson(res, 404, { error: `No route matches ${req.method} ${path}.` });
    } catch (err) {
        sendError(res, err);
    }
});

server.listen(PORT, '0.0.0.0', () => {
    console.log(`explorer-api bridge listening on :${PORT} -> ${BQ_URL}`);
});
