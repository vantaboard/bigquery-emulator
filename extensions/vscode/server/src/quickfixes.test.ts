import { describe, expect, it } from 'vitest';
import { TextDocument } from 'vscode-languageserver-textdocument';
import { DiagnosticSeverity } from 'vscode-languageserver';

import { catalogCompletions } from './catalog.js';
import { buildQuickFixes } from './quickfixes.js';

describe('catalogCompletions', () => {
  it('returns table and column matches for a prefix', () => {
    const catalog = {
      schema: {
        events: ['id', 'name'],
        'analytics.events': ['id'],
      },
      qualifiedTables: ['proj.analytics.events'],
      routines: ['analytics.add_one'],
    };

    const results = catalogCompletions(catalog, 'ev');
    expect(results.some((item) => item.label === 'events')).toBe(true);
    expect(results.some((item) => item.label === 'proj.analytics.events')).toBe(true);
  });
});

describe('buildQuickFixes', () => {
  it('offers to insert a missing closing parenthesis', () => {
    const document = TextDocument.create(
      'file:///tmp/query.sql',
      'bigquery',
      1,
      'SELECT SAFE_ADD(',
    );
    const diagnostics = [
      {
        range: {
          start: { line: 0, character: 16 },
          end: { line: 0, character: 17 },
        },
        severity: DiagnosticSeverity.Error,
        message: 'Syntax error: Expected ")" but got end of script at [1:17]',
        source: 'emulator',
      },
    ];

    const actions = buildQuickFixes(document, diagnostics, {
      schema: {},
      qualifiedTables: [],
      routines: [],
    });

    expect(actions.some((action) => action.title === 'Insert missing ")"')).toBe(true);
  });
});
