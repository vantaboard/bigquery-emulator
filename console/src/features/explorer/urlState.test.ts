import { describe, expect, it } from 'vitest';

import { buildExplorerSearchParams, parseExplorerSearchParams, type ExplorerUrlState } from './urlState';

describe('parseExplorerSearchParams', () => {
    it('parses a full URL produced by buildExplorerSearchParams', () => {
        const state: ExplorerUrlState = {
            project: 'p',
            dataset: 'd',
            table: 't',
            results: 'results',
            query: 'SELECT 1',
        };
        const qs = buildExplorerSearchParams(state);
        expect(parseExplorerSearchParams(`?${qs}`)).toEqual(state);
    });

    it('maps legacy tab names', () => {
        const s = parseExplorerSearchParams('?project=p&dataset=d&table=t&results=infoTab');
        expect(s.results).toBe('info');
    });
});

describe('buildExplorerSearchParams', () => {
    it('round-trips minimal selection', () => {
        const state: ExplorerUrlState = {
            project: 'p',
            dataset: 'd',
            table: 't',
            results: 'info',
            query: 'SELECT 1',
        };
        const qs = buildExplorerSearchParams(state);
        const parsed = parseExplorerSearchParams(`?${qs}`);
        expect(parsed).toEqual(state);
    });
});
