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
  const emptyCatalog = { schema: {}, qualifiedTables: [], routines: [] };

  it('does not offer a closing parenthesis for incomplete function calls', () => {
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

    const actions = buildQuickFixes(document, diagnostics, emptyCatalog);

    expect(actions).toEqual([]);
  });

  it('offers to close an unterminated backtick identifier', () => {
    const document = TextDocument.create(
      'file:///tmp/query.sql',
      'bigquery',
      1,
      'SELECT `hello',
    );
    const diagnostics = [
      {
        range: {
          start: { line: 0, character: 12 },
          end: { line: 0, character: 13 },
        },
        severity: DiagnosticSeverity.Error,
        message: 'Syntax error: Expected "`" but got end of script at [1:14]',
        source: 'emulator',
      },
    ];

    const actions = buildQuickFixes(document, diagnostics, emptyCatalog);

    expect(actions.some((action) => action.title === 'Insert missing "`"')).toBe(true);
  });

  it('does not suggest uppercasing keywords that are already uppercase', () => {
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

    const actions = buildQuickFixes(document, diagnostics, emptyCatalog);

    expect(actions.some((action) => action.title.includes('uppercase keyword'))).toBe(false);
  });

  it('suggests uppercasing a lowercase keyword', () => {
    const document = TextDocument.create(
      'file:///tmp/query.sql',
      'bigquery',
      1,
      'select 1',
    );
    const diagnostics = [
      {
        range: {
          start: { line: 0, character: 0 },
          end: { line: 0, character: 6 },
        },
        severity: DiagnosticSeverity.Warning,
        message: 'Style warning: keyword should be uppercase',
        source: 'emulator',
      },
    ];

    const actions = buildQuickFixes(document, diagnostics, emptyCatalog);

    expect(actions.some((action) => action.title === 'Use uppercase keyword SELECT')).toBe(true);
  });
});
