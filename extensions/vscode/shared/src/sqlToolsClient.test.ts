import { describe, expect, it } from 'vitest';

import { SqlToolsClient } from './sqlToolsClient.js';

describe('SqlToolsClient', () => {
  it('probes capabilities and caches availability', async () => {
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
      return new Response('not found', { status: 404 });
    };

    const client = new SqlToolsClient({
      baseUrl: 'http://127.0.0.1:9050',
      fetchImpl: fetchImpl as typeof fetch,
    });

    await expect(client.probeCapabilities()).resolves.toBe(true);
    expect(client.isAvailable()).toBe(true);
    await expect(client.probeCapabilities()).resolves.toBe(true);
  });

  it('sends the SQL Tools token header when configured', async () => {
    let capturedHeaders: Record<string, string> | undefined;
    const fetchImpl = async (_url: string | URL | Request, init?: RequestInit) => {
      capturedHeaders = init?.headers as Record<string, string>;
      return new Response(
        JSON.stringify({ statementKinds: [], diagnostics: [] }),
        { status: 200 },
      );
    };

    const client = new SqlToolsClient({
      baseUrl: 'http://127.0.0.1:9050',
      token: 'secret',
      fetchImpl: fetchImpl as typeof fetch,
    });

    await client.parse({ sql: 'SELECT 1' });
    expect(capturedHeaders?.['X-BigQuery-Emulator-SqlTools-Token']).toBe('secret');
  });
});
