import { describe, expect, it } from 'vitest';

import { SqlToolsBackend } from './sqlToolsBackend.js';

const settings = {
  backendMode: 'emulator' as const,
  emulatorBaseUrl: 'http://127.0.0.1:9050',
  projectId: 'local-project',
  strictFormat: false,
  formatIndentationSpaces: 2,
  formatLineLengthLimit: 80,
};

describe('SqlToolsBackend', () => {
  it('returns empty diagnostics when parse omits diagnostics on success', async () => {
    const fetchImpl = async (url: string | URL | Request) => {
      const path = String(url);
      if (path.endsWith('/capabilities')) {
        return new Response(
          JSON.stringify({
            sqlTools: true,
            version: '1.0',
            endpoints: ['parse'],
            offsetUnits: ['utf16'],
          }),
          { status: 200 },
        );
      }
      if (path.endsWith('/parse')) {
        return new Response(JSON.stringify({ statementKinds: ['QueryStatement'] }), {
          status: 200,
        });
      }
      return new Response('not found', { status: 404 });
    };

    const backend = new SqlToolsBackend(settings, {
      fetchImpl: fetchImpl as typeof fetch,
    });

    await expect(backend.getDiagnostics('SELECT 1', { settings })).resolves.toEqual([]);
  });
});
