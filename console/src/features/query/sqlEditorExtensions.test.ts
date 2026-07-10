import { describe, expect, it } from 'vitest';

import { formatDiagnosticMessage } from '@/features/query/sqlEditorExtensions';

describe('formatDiagnosticMessage', () => {
    it('normalizes end of statement and appends position', () => {
        expect(
            formatDiagnosticMessage({
                message: 'Syntax error: Expected ")" but got end of statement',
                line: 1,
                column: 17,
            }),
        ).toBe('Syntax error: Expected ")" but got end of script at [1:17]');
    });

    it('does not duplicate an existing position suffix', () => {
        expect(
            formatDiagnosticMessage({
                message: 'Syntax error: Expected ")" but got end of script at [1:17]',
                line: 1,
                column: 17,
            }),
        ).toBe('Syntax error: Expected ")" but got end of script at [1:17]');
    });
});
