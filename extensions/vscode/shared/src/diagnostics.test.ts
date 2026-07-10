import { describe, expect, it } from 'vitest';

import { diagnosticRange, formatDiagnosticMessage } from './diagnostics.js';
import type { SqlDiagnostic } from './types.js';

const document = {
  lineCount: 1,
  line: () => ({ text: 'SELECT SAFE_ADD(', range: { start: 0, end: 16 } }),
  offsetAt: (position: { line: number; character: number }) => position.character,
  positionAt: (offset: number) => ({ line: 0, character: offset }),
};

describe('diagnosticRange', () => {
  it('expands zero-width EOF diagnostics to cover the preceding character', () => {
    const diagnostic: SqlDiagnostic = {
      line: 1,
      column: 17,
      endLine: 1,
      endColumn: 17,
      message: 'Syntax error',
      severity: 'error',
    };

    expect(diagnosticRange(document, diagnostic)).toEqual({
      start: { line: 0, character: 15 },
      end: { line: 0, character: 16 },
    });
  });
});

describe('formatDiagnosticMessage', () => {
  it('normalizes end of statement to end of script', () => {
    const message = formatDiagnosticMessage({
      message: 'Syntax error: Expected ")" but got end of statement',
      line: 1,
      column: 17,
    });
    expect(message).toBe('Syntax error: Expected ")" but got end of script at [1:17]');
  });

  it('preserves messages that already include a position suffix', () => {
    const message = formatDiagnosticMessage({
      message: 'Syntax error: Expected ")" but got end of script at [1:17]',
      line: 1,
      column: 17,
    });
    expect(message).toBe('Syntax error: Expected ")" but got end of script at [1:17]');
  });
});
