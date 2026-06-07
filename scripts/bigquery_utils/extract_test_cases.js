#!/usr/bin/env node
/**
 * Extract pure-SQL scalar UDF test cases from a bigquery-utils clone.
 *
 * Walks udfs/community/ and udfs/migration/<dialect>/test_cases.js, evaluates each
 * file in a vm sandbox with a unit_test_utils stub, pairs cases with sibling
 * <name>.sqlx bodies, and emits a JSON manifest of emitted vs skipped UDFs.
 *
 * Usage:
 *   node scripts/bigquery_utils/extract_test_cases.js \
 *     --src /path/to/bigquery-utils \
 *     --sha abc123 \
 *     --out /tmp/bqutils.json
 */

"use strict";

const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

function usage(msg) {
  if (msg) {
    console.error(`extract_test_cases.js: ${msg}`);
  }
  console.error(
    "usage: extract_test_cases.js --src PATH [--sha SHA] [--out PATH]"
  );
  process.exit(msg ? 2 : 0);
}

function parseArgs(argv) {
  let src = "";
  let sha = "";
  let out = "";
  for (let i = 2; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === "--src" && argv[i + 1]) {
      src = argv[++i];
    } else if (arg === "--sha" && argv[i + 1]) {
      sha = argv[++i];
    } else if (arg === "--out" && argv[i + 1]) {
      out = argv[++i];
    } else if (arg === "-h" || arg === "--help") {
      usage();
    } else {
      usage(`unknown arg ${arg}`);
    }
  }
  if (!src) {
    usage("--src is required");
  }
  return { src, sha, out };
}

function stripConfigBlock(text) {
  return text.replace(/^config\s*\{[^}]*\}\s*/m, "");
}

function stripOptionsClause(text) {
  const marker = "OPTIONS";
  let idx = 0;
  while (true) {
    const start = text.indexOf(marker, idx);
    if (start === -1) {
      return text;
    }
    let i = start + marker.length;
    while (i < text.length && /\s/.test(text[i])) {
      i++;
    }
    if (text[i] !== "(") {
      idx = start + marker.length;
      continue;
    }
    let depth = 0;
    let inTriple = false;
    let j = i;
    for (; j < text.length; j++) {
      if (!inTriple && text.startsWith('"""', j)) {
        inTriple = true;
        j += 2;
        continue;
      }
      if (inTriple) {
        if (text.startsWith('"""', j)) {
          inTriple = false;
          j += 2;
        }
        continue;
      }
      const ch = text[j];
      if (ch === "(") {
        depth++;
      } else if (ch === ")") {
        depth--;
        if (depth === 0) {
          j++;
          break;
        }
      }
    }
    text = text.slice(0, start) + text.slice(j);
    idx = start;
  }
}

function prepareSqlxBody(raw, udfName) {
  let body = stripConfigBlock(raw);
  body = stripOptionsClause(body);
  body = body.replace(/\$\{self\(\)\}/g, udfName);
  body = body.trim();
  body = body.replace(
    /CREATE\s+OR\s+REPLACE\s+FUNCTION/gi,
    "CREATE FUNCTION"
  );
  return body;
}

function hasLanguageJs(text) {
  return /\bLANGUAGE\s+js\b/i.test(text);
}

function hasDataformTemplate(text) {
  return /\$\{[^}]+\}/.test(text);
}

function isHardExcluded(name, sqlBody, cases) {
  if (/^exif/i.test(name)) {
    return "exif family (GCS/ObjectRef)";
  }
  const blob = [sqlBody, JSON.stringify(cases)].join("\n");
  if (/\bOBJ\.MAKE_REF\b/.test(blob)) {
    return "OBJ.MAKE_REF dependency";
  }
  if (/\bGCS\b|gs:\/\//i.test(blob)) {
    return "GCS dependency";
  }
  if (/\bCREATE\s+MODEL\b/i.test(sqlBody)) {
    return "BQML CREATE MODEL";
  }
  if (/\bREMOTE\s+WITH\s+CONNECTION\b/i.test(sqlBody)) {
    return "remote function";
  }
  return "";
}

function discoverTestCaseFiles(srcRoot) {
  const files = [];
  const community = path.join(srcRoot, "udfs", "community", "test_cases.js");
  if (fs.existsSync(community)) {
    files.push({ family: "community", file: community });
  }
  const migrationRoot = path.join(srcRoot, "udfs", "migration");
  if (fs.existsSync(migrationRoot)) {
    for (const dialect of fs.readdirSync(migrationRoot)) {
      const tc = path.join(migrationRoot, dialect, "test_cases.js");
      if (fs.existsSync(tc)) {
        files.push({ family: `migration/${dialect}`, file: tc });
      }
    }
  }
  return files;
}

function evalTestCases(filePath) {
  const source = fs.readFileSync(filePath, "utf8");
  const udfCases = new Map();
  const udafCases = new Map();

  const captureUdf = (name, cases) => {
    if (!udfCases.has(name)) {
      udfCases.set(name, []);
    }
    udfCases.get(name).push(...cases);
  };
  const captureUdaf = (name, testCase) => {
    if (!udafCases.has(name)) {
      udafCases.set(name, []);
    }
    udafCases.get(name).push(testCase);
  };

  const sandbox = {
    unit_test_utils: {
      generate_udf_test: captureUdf,
      generate_udaf_test: captureUdaf,
    },
    publish: () => {},
    ctx: {},
    dataform: {
      projectConfig: {
        vars: {},
        defaultLocation: "US",
      },
    },
    uuidv4: () => "test",
    console: { log: () => {}, error: () => {} },
  };
  vm.createContext(sandbox);
  vm.runInContext(source, sandbox, { filename: filePath });
  return { udfCases, udafCases };
}

function relPath(srcRoot, absPath) {
  return path.relative(srcRoot, absPath).split(path.sep).join("/");
}

function main() {
  const { src, sha, out } = parseArgs(process.argv);
  const srcRoot = path.resolve(src);
  const udfsRoot = path.join(srcRoot, "udfs");
  if (!fs.existsSync(udfsRoot)) {
    console.error(`extract_test_cases.js: udfs/ missing under ${srcRoot}`);
    process.exit(1);
  }

  const emitted = [];
  const skipped = [];

  for (const { family, file } of discoverTestCaseFiles(srcRoot)) {
    let udfCases;
    let udafCases;
    try {
      ({ udfCases, udafCases } = evalTestCases(file));
    } catch (err) {
      skipped.push({
        family,
        name: path.basename(path.dirname(file)),
        reason: `test_cases.js eval failed: ${err.message}`,
      });
      continue;
    }

    const dir = path.dirname(file);
    for (const [name, cases] of udafCases) {
      const sqlxPath = path.join(dir, `${name}.sqlx`);
      if (!fs.existsSync(sqlxPath)) {
        skipped.push({
          family,
          name,
          reason: `missing sibling ${name}.sqlx`,
        });
        continue;
      }

      const rawSqlx = fs.readFileSync(sqlxPath, "utf8");
      const createSql = prepareSqlxBody(rawSqlx, name);

      if (hasLanguageJs(createSql)) {
        skipped.push({ family, name, reason: "LANGUAGE js" });
        continue;
      }
      if (hasDataformTemplate(createSql)) {
        skipped.push({ family, name, reason: "Dataform templating (${...})" });
        continue;
      }

      const excludeReason = isHardExcluded(name, createSql, cases);
      if (excludeReason) {
        skipped.push({ family, name, reason: excludeReason });
        continue;
      }

      if (!cases.length) {
        skipped.push({ family, name, reason: "no UDAF test cases" });
        continue;
      }

      emitted.push({
        family,
        name,
        kind: "udaf",
        upstream_sqlx: relPath(srcRoot, sqlxPath),
        upstream_test_cases: relPath(srcRoot, file),
        create_sql: createSql,
        cases: cases.map((tc) => ({
          input_columns: tc.input_columns,
          input_rows: tc.input_rows,
          expected_output: tc.expected_output,
        })),
      });
    }

    for (const [name, cases] of udfCases) {
      const sqlxPath = path.join(dir, `${name}.sqlx`);
      if (!fs.existsSync(sqlxPath)) {
        skipped.push({
          family,
          name,
          reason: `missing sibling ${name}.sqlx`,
        });
        continue;
      }

      const rawSqlx = fs.readFileSync(sqlxPath, "utf8");
      const createSql = prepareSqlxBody(rawSqlx, name);

      if (hasLanguageJs(createSql)) {
        skipped.push({ family, name, reason: "LANGUAGE js" });
        continue;
      }
      if (hasDataformTemplate(createSql)) {
        skipped.push({ family, name, reason: "Dataform templating (${...})" });
        continue;
      }

      const excludeReason = isHardExcluded(name, createSql, cases);
      if (excludeReason) {
        skipped.push({ family, name, reason: excludeReason });
        continue;
      }

      if (!cases.length) {
        skipped.push({ family, name, reason: "no test cases" });
        continue;
      }

      emitted.push({
        family,
        name,
        upstream_sqlx: relPath(srcRoot, sqlxPath),
        upstream_test_cases: relPath(srcRoot, file),
        create_sql: createSql,
        cases: cases.map((tc) => ({
          inputs: tc.inputs,
          expected_output: tc.expected_output,
        })),
      });
    }
  }

  emitted.sort((a, b) =>
    `${a.family}/${a.name}`.localeCompare(`${b.family}/${b.name}`)
  );
  skipped.sort((a, b) =>
    `${a.family}/${a.name}`.localeCompare(`${b.family}/${b.name}`)
  );

  const manifest = { source_sha: sha, emitted, skipped };
  const json = JSON.stringify(manifest, null, 2) + "\n";
  if (out) {
    fs.writeFileSync(out, json);
  } else {
    process.stdout.write(json);
  }

  console.error(
    `extract_test_cases.js: emitted=${emitted.length} skipped=${skipped.length}`
  );
}

main();
