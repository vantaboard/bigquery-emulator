#!/usr/bin/env node
/**
 * One-shot codemod: export async main from sample scripts for in-process tests.
 * Run from repo: node third_party/node-bigquery-tests/scripts/codemod-export-main.js
 */
'use strict';

const fs = require('fs');
const path = require('path');

const rootDir = path.resolve(__dirname, '..');

const skipFiles = new Set([
  'codemod-export-main.js',
  'package.json',
]);

const newTail = `if (require.main === module) {
  void main(...process.argv.slice(2)).catch(err => {
    console.error(err);
    process.exit(1);
  });
}
module.exports = {main};
`;

function transformStandard(s, fname) {
  // Pattern A: closing inner }, // region comments, inner();, close main, main(...
  const patA =
    /\n( {2})\}\n((?:  \/\/[^\n]*\n)+)( {2})([A-Za-z_$][\w$]*)\(([^)]*)\);\n}\n(?:\n)*main\(\.\.\.process\.argv\.slice\(2\)\);\s*$/;
  const ma = s.match(patA);
  if (ma) {
    let out = s;
    if (!out.includes('async function main')) {
      out = out.replace(/\bfunction main\b/, 'async function main');
    }
    const innerIndent = ma[3];
    const inner = ma[4];
    const innerArgs = ma[5];
    const regionComments = ma[2];
    out = out.replace(
      patA,
      `\n${ma[1]}}\n${regionComments}${innerIndent}await ${inner}(${innerArgs});\n}\n\n${newTail.trimEnd()}\n`,
    );
    return out;
  }
  // Pattern B: closing inner }, inner();, optional // lines, close main, main(...
  const patB =
    /\n( {2})\}\n( {2})([A-Za-z_$][\w$]*)\(([^)]*)\);\n((?:  \/\/[^\n]*\n)*)}\n(?:\n)*main\(\.\.\.process\.argv\.slice\(2\)\);\s*$/;
  const mb = s.match(patB);
  if (!mb) return null;
  let out = s;
  if (!out.includes('async function main')) {
    out = out.replace(/\bfunction main\b/, 'async function main');
  }
  const innerIndent = mb[2];
  const inner = mb[3];
  const innerArgs = mb[4];
  const midComments = mb[5];
  out = out.replace(
    patB,
    `\n${mb[1]}}\n${innerIndent}await ${inner}(${innerArgs});\n${midComments}}\n\n${newTail.trimEnd()}\n`,
  );
  return out;
}

function transformSyncTail(s, fname) {
  if (
    s.includes('module.exports = {main}') ||
    s.includes('module.exports={main}')
  ) {
    return null;
  }
  if (!/\bfunction main\b/.test(s)) return null;
  if (s.includes('async function')) return null;
  const pat = /\}\n(?:\n)*main\(\.\.\.process\.argv\.slice\(2\)\);\s*$/;
  if (!pat.test(s)) return null;
  let out = s.replace(/\bfunction main\b/, 'async function main');
  out = out.replace(pat, `}\n\n${newTail.trimEnd()}\n`);
  return out;
}

function transformAsyncCatch(s) {
  if (!s.includes('async function main')) return null;
  const old =
    /\nmain\(\.\.\.process\.argv\.slice\(2\)\)\.catch\(err => \{\n\s*console\.error\(err\);\n\s*process\.exitCode = 1;\n\}\);\s*$/;
  if (!old.test(s)) return null;
  return s.replace(old, `\n\n${newTail.trimEnd()}\n`);
}

/** quickstart: main(...args) */
const quickstartEnd =
  /\nconst args = process\.argv\.slice\(2\);\nmain\(\.\.\.args\);\s*$/;

function transformQuickstart(s) {
  if (!quickstartEnd.test(s)) return null;
  let out = s.replace(/\bfunction main\b/, 'async function main');
  const innerCall = /\n( {2})([A-Za-z_$][\w$]*)\(\);\n  \/\/ \[END bigquery_quickstart\]\n}/;
  const m = out.match(innerCall);
  if (!m) return null;
  out = out.replace(innerCall, `\n${m[1]}await ${m[2]}();\n  // [END bigquery_quickstart]\n}`);
  out = out.replace(
    quickstartEnd,
    `\nif (require.main === module) {\n  const args = process.argv.slice(2);\n  void main(...args).catch(err => {\n    console.error(err);\n    process.exit(1);\n  });\n}\nmodule.exports = {main};\n`,
  );
  return out;
}

function main() {
  const names = fs
    .readdirSync(rootDir)
    .filter(f => f.endsWith('.js') && !skipFiles.has(f));
  let ok = 0;
  let skip = 0;
  for (const f of names) {
    const p = path.join(rootDir, f);
    let s = fs.readFileSync(p, 'utf8');
    if (
      s.includes('module.exports = {main}') ||
      s.includes('module.exports={main}')
    ) {
      skip++;
      continue;
    }
    let next = transformQuickstart(s);
    if (next) {
      fs.writeFileSync(p, next);
      ok++;
      continue;
    }
    next = transformAsyncCatch(s);
    if (next && next !== s) {
      fs.writeFileSync(p, next);
      ok++;
      continue;
    }
    next = transformSyncTail(s, f);
    if (next) {
      fs.writeFileSync(p, next);
      ok++;
      continue;
    }
    next = transformStandard(s, f);
    if (next) {
      fs.writeFileSync(p, next);
      ok++;
      continue;
    }
    console.error('SKIP (no pattern):', f);
  }
  console.error('codemod: updated', ok, 'skipped-already-or-no-match', skip);
}

main();
