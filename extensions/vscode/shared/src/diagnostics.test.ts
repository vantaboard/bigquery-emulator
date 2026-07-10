import { describe, expect, it } from 'vitest';

import { formatDiagnosticMessage } from './diagnostics.js';

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
