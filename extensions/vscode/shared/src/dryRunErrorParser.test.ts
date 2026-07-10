import { describe, expect, it } from 'vitest';

import { parseDryRunErrorMessage } from './dryRunErrorParser.js';

describe('parseDryRunErrorMessage', () => {
  it('parses production syntax errors with position suffix', () => {
    const parsed = parseDryRunErrorMessage(
      'Syntax error: Expected ")" but got end of script at [1:17]',
    );
    expect(parsed).toEqual({
      message: 'Syntax error: Expected ")" but got end of script',
      line: 1,
      column: 17,
    });
  });

  it('parses unrecognized name errors', () => {
    const parsed = parseDryRunErrorMessage('Unrecognized name: missing_col at [2:8]');
    expect(parsed).toEqual({
      message: 'Unrecognized name: missing_col',
      line: 2,
      column: 8,
    });
  });
});
