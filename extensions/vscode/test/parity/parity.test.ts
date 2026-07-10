import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import { parse as parseYaml } from 'yaml';
import { describe, expect, it } from 'vitest';

import {
  formatDiagnosticMessage,
  parseDryRunErrorMessage,
} from '@bigquery-emulator/vscode-shared';

interface ParityFixture {
  name: string;
  sql: string;
  production: {
    message: string | null;
    line: number | null;
    column: number | null;
  };
  emulator: {
    known_failing: boolean;
  };
}

const fixturePath = path.join(path.dirname(fileURLToPath(import.meta.url)), 'errors.yaml');

function loadFixtures(): ParityFixture[] {
  const raw = fs.readFileSync(fixturePath, 'utf8');
  return parseYaml(raw) as ParityFixture[];
}

describe('production parity fixtures', () => {
  const fixtures = loadFixtures();

  for (const fixture of fixtures) {
    it(`parses production message for ${fixture.name}`, () => {
      if (!fixture.production.message) {
        return;
      }

      const parsed = parseDryRunErrorMessage(fixture.production.message);
      expect(parsed).not.toBeNull();
      expect(parsed?.line).toBe(fixture.production.line);
      expect(parsed?.column).toBe(fixture.production.column);
      expect(formatDiagnosticMessage({
        message: parsed!.message,
        line: parsed!.line,
        column: parsed!.column,
      })).toBe(fixture.production.message);
    });
  }
});
