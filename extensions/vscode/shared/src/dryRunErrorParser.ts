import type { DryRunError } from './types.js';

const POSITION_SUFFIX_RE = /\s+at\s+\[(\d+):(\d+)\]\s*$/i;
const EXPECTED_GOT_RE =
  /Syntax error:\s*Expected\s+"([^"]+)"\s+but\s+got\s+(.+?)(?:\s+at\s+\[(\d+):(\d+)\])?$/i;
const UNRECOGNIZED_NAME_RE =
  /Unrecognized name:\s+([^\s;]+)(?:\s+at\s+\[(\d+):(\d+)\])?/i;

export function parseDryRunErrorMessage(raw: string): DryRunError | null {
  const message = raw.trim();
  if (!message) {
    return null;
  }

  const positionMatch = message.match(POSITION_SUFFIX_RE);
  if (positionMatch) {
    const line = Number(positionMatch[1]);
    const column = Number(positionMatch[2]);
    const withoutSuffix = message.replace(POSITION_SUFFIX_RE, '').trim();
    return { message: withoutSuffix, line, column };
  }

  const expectedGot = message.match(EXPECTED_GOT_RE);
  if (expectedGot) {
    const line = expectedGot[3] ? Number(expectedGot[3]) : 1;
    const column = expectedGot[4] ? Number(expectedGot[4]) : 1;
    return {
      message: `Syntax error: Expected "${expectedGot[1]}" but got ${expectedGot[2]}`,
      line,
      column,
    };
  }

  const unrecognized = message.match(UNRECOGNIZED_NAME_RE);
  if (unrecognized) {
    const line = unrecognized[2] ? Number(unrecognized[2]) : 1;
    const column = unrecognized[3] ? Number(unrecognized[3]) : 1;
    return {
      message: `Unrecognized name: ${unrecognized[1]}`,
      line,
      column,
    };
  }

  return { message, line: 1, column: 1 };
}

export function normalizeProductionMessage(message: string): string {
  return message.replace(/\bend of statement\b/gi, 'end of script');
}
