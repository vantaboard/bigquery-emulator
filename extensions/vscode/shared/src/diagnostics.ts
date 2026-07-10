import type { SqlDiagnostic } from './types.js';

const POSITION_SUFFIX_RE = /\s+at\s+\[\d+:\d+\]\s*$/;

export interface LspRange {
  start: { line: number; character: number };
  end: { line: number; character: number };
}

export interface TextDocumentLike {
  lineCount: number;
  line(lineNumber: number): { text: string; range: { start: number; end: number } };
  offsetAt(position: { line: number; character: number }): number;
  positionAt(offset: number): { line: number; character: number };
}

export function formatDiagnosticMessage(
  diagnostic: Pick<SqlDiagnostic, 'message' | 'line' | 'column'>,
): string {
  const message = diagnostic.message.replace(/\bend of statement\b/gi, 'end of script');

  if (POSITION_SUFFIX_RE.test(message)) {
    return message;
  }

  return `${message} at [${diagnostic.line}:${diagnostic.column}]`;
}

export function diagnosticSeverity(
  severity: string,
): 1 | 2 | 3 | 4 {
  if (severity === 'error') return 1;
  if (severity === 'warning') return 2;
  if (severity === 'information') return 3;
  return 4;
}

export function diagnosticRange(
  document: TextDocumentLike,
  diagnostic: SqlDiagnostic,
): LspRange {
  if (diagnostic.startUtf16 !== undefined && diagnostic.endUtf16 !== undefined) {
    return {
      start: document.positionAt(Math.max(0, diagnostic.startUtf16)),
      end: document.positionAt(Math.max(diagnostic.startUtf16, diagnostic.endUtf16)),
    };
  }

  const fromLine = Math.min(Math.max(1, diagnostic.line), document.lineCount);
  const fromCharacter = Math.max(0, diagnostic.column - 1);

  if (diagnostic.endLine !== undefined && diagnostic.endColumn !== undefined) {
    const toLine = Math.min(Math.max(1, diagnostic.endLine), document.lineCount);
    let toCharacter = Math.max(0, diagnostic.endColumn - 1);
    if (toLine === fromLine && toCharacter <= fromCharacter) {
      if (fromCharacter > 0) {
        return {
          start: { line: fromLine - 1, character: fromCharacter - 1 },
          end: { line: toLine - 1, character: fromCharacter },
        };
      }
      toCharacter = fromCharacter + 1;
    }
    return {
      start: { line: fromLine - 1, character: fromCharacter },
      end: { line: toLine - 1, character: toCharacter },
    };
  }

  return {
    start: { line: fromLine - 1, character: fromCharacter },
    end: { line: fromLine - 1, character: fromCharacter + 1 },
  };
}

export function rangeFromLineColumn(
  line: number,
  column: number,
  endLine?: number,
  endColumn?: number,
): LspRange {
  const start = { line: Math.max(0, line - 1), character: Math.max(0, column - 1) };
  if (endLine !== undefined && endColumn !== undefined) {
    return {
      start,
      end: {
        line: Math.max(0, endLine - 1),
        character: Math.max(0, endColumn - 1),
      },
    };
  }
  return { start, end: { line: start.line, character: start.character + 1 } };
}
