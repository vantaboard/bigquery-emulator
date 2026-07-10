import type { SqlCatalog } from '@bigquery-emulator/vscode-shared';
import type { CodeAction, Diagnostic, Range, TextDocument } from 'vscode-languageserver';

const EXPECTED_DELIMITER_RE =
  /Syntax error:\s*Expected\s+"([^"]+)"\s+but\s+got\s+end of script/i;
const UNRECOGNIZED_NAME_RE = /Unrecognized name:\s+([^\s;]+)/i;
const LOWERCASE_KEYWORD_RE = /\b(select|from|where|join|group by|order by)\b/i;

export function buildQuickFixes(
  document: TextDocument,
  diagnostics: Diagnostic[],
  catalog: SqlCatalog,
): CodeAction[] {
  const actions: CodeAction[] = [];

  for (const diagnostic of diagnostics) {
    actions.push(...delimiterFixes(document, diagnostic));
    actions.push(...unrecognizedNameFixes(document, diagnostic, catalog));
    actions.push(...keywordCasingFix(document, diagnostic));
  }

  return actions;
}

function delimiterFixes(document: TextDocument, diagnostic: Diagnostic): CodeAction[] {
  const match = diagnostic.message.match(EXPECTED_DELIMITER_RE);
  if (!match) {
    return [];
  }

  const expected = match[1];
  const closingPairs: Record<string, string> = {
    ')': ')',
    '"': '"',
    "'": "'",
    '`': '`',
  };
  const insertText = closingPairs[expected];
  if (!insertText) {
    return [];
  }

  const range: Range = diagnostic.range;
  const insertPosition = range.end;
  return [
    {
      title: `Insert missing "${expected}"`,
      kind: 'quickfix',
      diagnostics: [diagnostic],
      edit: {
        changes: {
          [document.uri]: [
            {
              range: {
                start: insertPosition,
                end: insertPosition,
              },
              newText: insertText,
            },
          ],
        },
      },
    },
  ];
}

function unrecognizedNameFixes(
  document: TextDocument,
  diagnostic: Diagnostic,
  catalog: SqlCatalog,
): CodeAction[] {
  const match = diagnostic.message.match(UNRECOGNIZED_NAME_RE);
  if (!match) {
    return [];
  }

  const badName = match[1];
  const candidates = [
    ...Object.keys(catalog.schema),
    ...catalog.qualifiedTables,
    ...catalog.routines,
  ];
  const suggestion = nearestMatch(badName, candidates);
  if (!suggestion) {
    return [];
  }

  return [
    {
      title: `Did you mean "${suggestion}"?`,
      kind: 'quickfix',
      diagnostics: [diagnostic],
      edit: {
        changes: {
          [document.uri]: [
            {
              range: diagnostic.range,
              newText: suggestion,
            },
          ],
        },
      },
    },
    {
      title: `Quote identifier as \`${badName}\``,
      kind: 'quickfix',
      diagnostics: [diagnostic],
      edit: {
        changes: {
          [document.uri]: [
            {
              range: diagnostic.range,
              newText: `\`${badName}\``,
            },
          ],
        },
      },
    },
  ];
}

function keywordCasingFix(document: TextDocument, diagnostic: Diagnostic): CodeAction[] {
  const lineText = document.getText({
    start: { line: diagnostic.range.start.line, character: 0 },
    end: {
      line: diagnostic.range.start.line,
      character: Number.MAX_SAFE_INTEGER,
    },
  });
  const match = lineText.match(LOWERCASE_KEYWORD_RE);
  if (!match) {
    return [];
  }

  const keyword = match[1];
  const upper = keyword.toUpperCase();
  const startCharacter = lineText.indexOf(keyword);
  if (startCharacter < 0) {
    return [];
  }

  return [
    {
      title: `Use uppercase keyword ${upper}`,
      kind: 'quickfix',
      diagnostics: [diagnostic],
      edit: {
        changes: {
          [document.uri]: [
            {
              range: {
                start: { line: diagnostic.range.start.line, character: startCharacter },
                end: {
                  line: diagnostic.range.start.line,
                  character: startCharacter + keyword.length,
                },
              },
              newText: upper,
            },
          ],
        },
      },
    },
  ];
}

function nearestMatch(target: string, candidates: string[]): string | undefined {
  const normalized = target.toLowerCase();
  let best: string | undefined;
  let bestScore = Number.POSITIVE_INFINITY;

  for (const candidate of candidates) {
    const score = levenshtein(normalized, candidate.toLowerCase());
    if (score < bestScore) {
      bestScore = score;
      best = candidate;
    }
  }

  return bestScore <= 3 ? best : undefined;
}

function levenshtein(a: string, b: string): number {
  const matrix = Array.from({ length: a.length + 1 }, () => Array<number>(b.length + 1).fill(0));
  for (let i = 0; i <= a.length; i += 1) matrix[i][0] = i;
  for (let j = 0; j <= b.length; j += 1) matrix[0][j] = j;

  for (let i = 1; i <= a.length; i += 1) {
    for (let j = 1; j <= b.length; j += 1) {
      const cost = a[i - 1] === b[j - 1] ? 0 : 1;
      matrix[i][j] = Math.min(
        matrix[i - 1][j] + 1,
        matrix[i][j - 1] + 1,
        matrix[i - 1][j - 1] + cost,
      );
    }
  }

  return matrix[a.length][b.length];
}
