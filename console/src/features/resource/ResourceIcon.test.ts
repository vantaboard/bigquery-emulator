import { describe, expect, it } from 'vitest';

import { resourceIconForTab } from './ResourceIcon';
import type { WorkspaceTab } from '@/features/workspace/types';

describe('resourceIconForTab', () => {
    it('maps query, dataset, and routine tabs', () => {
        expect(
            resourceIconForTab({
                type: 'query',
                id: 'q1',
                title: 'Untitled query',
                sql: '',
                subTab: 'results',
                projectId: 'p',
            }),
        ).toEqual({ kind: 'query' });
        expect(
            resourceIconForTab({ type: 'dataset', id: 'd1', projectId: 'p', datasetId: 'ds' }),
        ).toEqual({ kind: 'dataset' });
        expect(
            resourceIconForTab({
                type: 'routine',
                id: 'r1',
                projectId: 'p',
                datasetId: 'ds',
                routineId: 'fn',
            }),
        ).toEqual({ kind: 'routine' });
    });

    it('maps table tabs by resourceType', () => {
        const base = {
            type: 'table' as const,
            id: 't1',
            projectId: 'p',
            datasetId: 'ds',
            tableId: 'tbl',
        };
        expect(resourceIconForTab(base)).toEqual({ kind: 'table', resourceType: undefined });
        expect(resourceIconForTab({ ...base, resourceType: 'VIEW' })).toEqual({
            kind: 'table',
            resourceType: 'VIEW',
        });
        expect(resourceIconForTab({ ...base, resourceType: 'MATERIALIZED_VIEW' })).toEqual({
            kind: 'table',
            resourceType: 'MATERIALIZED_VIEW',
        });
        expect(resourceIconForTab({ ...base, resourceType: 'SNAPSHOT' })).toEqual({
            kind: 'table',
            resourceType: 'SNAPSHOT',
        });
        expect(resourceIconForTab({ ...base, resourceType: 'EXTERNAL' })).toEqual({
            kind: 'table',
            resourceType: 'EXTERNAL',
        });
    });

    it('covers all WorkspaceTab variants exhaustively', () => {
        const tabs: WorkspaceTab[] = [
            { type: 'query', id: 'q', title: 't', sql: '', subTab: 'results', projectId: 'p' },
            { type: 'dataset', id: 'd', projectId: 'p', datasetId: 'ds' },
            {
                type: 'table',
                id: 't',
                projectId: 'p',
                datasetId: 'ds',
                tableId: 'tbl',
                resourceType: 'TABLE',
            },
            { type: 'routine', id: 'r', projectId: 'p', datasetId: 'ds', routineId: 'fn' },
        ];
        expect(tabs.map(resourceIconForTab).map((k) => k.kind)).toEqual([
            'query',
            'dataset',
            'table',
            'routine',
        ]);
    });
});
