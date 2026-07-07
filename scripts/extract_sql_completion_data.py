#!/usr/bin/env python3
"""Extract SQL completion metadata from BigQuery console JS into C++ data files."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


def cpp_string(value: str) -> str:
    escaped = (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    )
    return f'"{escaped}"'


def extract_balanced(text: str, start: int, open_char: str, close_char: str) -> str:
    if text[start] != open_char:
        raise ValueError(f"Expected {open_char} at {start}, got {text[start]!r}")
    depth = 0
    in_string = False
    string_char = ""
    escape = False
    for i in range(start, len(text)):
        ch = text[i]
        if in_string:
            if escape:
                escape = False
                continue
            if ch == "\\":
                escape = True
                continue
            if ch == string_char:
                in_string = False
            continue
        if ch in ("'", '"'):
            in_string = True
            string_char = ch
            continue
        if ch == open_char:
            depth += 1
        elif ch == close_char:
            depth -= 1
            if depth == 0:
                return text[start : i + 1]
    raise ValueError("Unbalanced brackets")


def extract_clause_words(text: str) -> list[tuple[str, str]]:
    marker = "this.CLAUSE_WORDS_WITH_TYPES = "
    idx = text.find(marker)
    if idx < 0:
        raise ValueError("CLAUSE_WORDS_WITH_TYPES not found")
    start = text.find("[", idx)
    array_text = extract_balanced(text, start, "[", "]")
    entries = re.findall(
        r'name:\s*"([^"]+)"\s*,\s*type:\s*"([^"]+)"',
        array_text,
    )
    if not entries:
        raise ValueError("No clause words extracted")
    return entries


def extract_keywords(text: str) -> list[str]:
    marker = 'this.KEYWORDS = "'
    idx = text.find(marker)
    if idx < 0:
        raise ValueError("KEYWORDS not found")
    start = idx + len(marker)
    end = text.find('"', start)
    if end < 0:
        raise ValueError("KEYWORDS string not terminated")
    return text[start:end].split()


def extract_function_objects(text: str, marker: str) -> list[dict[str, str]]:
    idx = text.find(marker)
    if idx < 0:
        raise ValueError(f"{marker!r} not found")
    start = text.find("[", idx)
    array_text = extract_balanced(text, start, "[", "]")
    functions: list[dict[str, str]] = []
    for block in re.finditer(r"\{[^{}]*\}", array_text):
        obj = block.group(0)
        name_match = re.search(r'name:\s*"([^"]+)"', obj)
        if not name_match:
            continue
        args_match = re.search(r'args:\s*"([^"]*)"', obj)
        desc_match = re.search(r'description:\s*"([^"]*)"', obj)
        url_match = re.search(r'url:\s*"([^"]*)"', obj)
        functions.append(
            {
                "name": name_match.group(1),
                "args": args_match.group(1) if args_match else "",
                "description": desc_match.group(1) if desc_match else "",
                "url": url_match.group(1) if url_match else "",
            }
        )
    return functions


def merge_functions(primary: list[dict[str, str]], fallback: list[dict[str, str]]) -> list[dict[str, str]]:
    seen = {item["name"].upper() for item in primary}
    merged = list(primary)
    for item in fallback:
        if item["name"].upper() not in seen:
            merged.append(item)
            seen.add(item["name"].upper())
    return merged


def extract_type_names(text: str) -> list[str]:
    marker = 'this.TYPE_NAMES = "'
    idx = text.find(marker)
    if idx < 0:
        raise ValueError("TYPE_NAMES not found")
    start = idx + len(marker)
    end = text.find('"', start)
    if end < 0:
        raise ValueError("TYPE_NAMES string not terminated")
    return text[start:end].split()


def write_header(out_path: Path) -> None:
    out_path.write_text(
        """\
#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_H_

#include <string>
#include <vector>

namespace bigquery_emulator {
namespace backend {
namespace sqltools {

struct ClauseWordEntry {
  std::string name;
  std::string type;
};

struct FunctionInfoEntry {
  std::string name;
  std::string args;
  std::string description;
  std::string url;
};

const std::vector<ClauseWordEntry>& ClauseWords();
const std::vector<std::string>& ExpressionKeywords();
const std::vector<FunctionInfoEntry>& FunctionInfo();
const std::vector<FunctionInfoEntry>& TvfFunctionInfo();
const std::vector<std::string>& TypeNames();

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_H_
""",
        encoding="utf-8",
    )


def write_source(
    out_path: Path,
    clause_words: list[tuple[str, str]],
    expression_keywords: list[str],
    functions: list[dict[str, str]],
    tvf_functions: list[dict[str, str]],
    type_names: list[str],
) -> None:
    lines = [
        '#include "backend/sqltools/sql_completion_data.h"',
        "",
        "namespace bigquery_emulator {",
        "namespace backend {",
        "namespace sqltools {",
        "namespace {",
        "",
        "const std::vector<ClauseWordEntry>& BuildClauseWords() {",
        "  static const std::vector<ClauseWordEntry>* kEntries =",
        "      new std::vector<ClauseWordEntry>{",
    ]
    for name, clause_type in clause_words:
        lines.append(
            f"          {{{cpp_string(name)}, {cpp_string(clause_type)}}},"
        )
    lines.extend(
        [
            "      };",
            "  return *kEntries;",
            "}",
            "",
            "const std::vector<std::string>& BuildExpressionKeywords() {",
            "  static const std::vector<std::string>* kEntries =",
            "      new std::vector<std::string>{",
        ]
    )
    for keyword in expression_keywords:
        lines.append(f"          {cpp_string(keyword)},")
    lines.extend(
        [
            "      };",
            "  return *kEntries;",
            "}",
            "",
            "const std::vector<FunctionInfoEntry>& BuildFunctionInfo() {",
            "  static const std::vector<FunctionInfoEntry>* kEntries =",
            "      new std::vector<FunctionInfoEntry>{",
        ]
    )
    for fn in functions:
        lines.append(
            "          {"
            f"{cpp_string(fn['name'])}, "
            f"{cpp_string(fn['args'])}, "
            f"{cpp_string(fn['description'])}, "
            f"{cpp_string(fn['url'])}"
            "},"
        )
    lines.extend(
        [
            "      };",
            "  return *kEntries;",
            "}",
            "",
            "const std::vector<FunctionInfoEntry>& BuildTvfFunctionInfo() {",
            "  static const std::vector<FunctionInfoEntry>* kEntries =",
            "      new std::vector<FunctionInfoEntry>{",
        ]
    )
    for fn in tvf_functions:
        lines.append(
            "          {"
            f"{cpp_string(fn['name'])}, "
            f"{cpp_string(fn['args'])}, "
            f"{cpp_string(fn['description'])}, "
            f"{cpp_string(fn['url'])}"
            "},"
        )
    lines.extend(
        [
            "      };",
            "  return *kEntries;",
            "}",
            "",
            "const std::vector<std::string>& BuildTypeNames() {",
            "  static const std::vector<std::string>* kEntries =",
            "      new std::vector<std::string>{",
        ]
    )
    for type_name in type_names:
        lines.append(f"          {cpp_string(type_name)},")
    lines.extend(
        [
            "      };",
            "  return *kEntries;",
            "}",
            "",
            "}  // namespace",
            "",
            "const std::vector<ClauseWordEntry>& ClauseWords() {",
            "  return BuildClauseWords();",
            "}",
            "",
            "const std::vector<std::string>& ExpressionKeywords() {",
            "  return BuildExpressionKeywords();",
            "}",
            "",
            "const std::vector<FunctionInfoEntry>& FunctionInfo() {",
            "  return BuildFunctionInfo();",
            "}",
            "",
            "const std::vector<FunctionInfoEntry>& TvfFunctionInfo() {",
            "  return BuildTvfFunctionInfo();",
            "}",
            "",
            "const std::vector<std::string>& TypeNames() {",
            "  return BuildTypeNames();",
            "}",
            "",
            "}  // namespace sqltools",
            "}  // namespace backend",
            "}  // namespace bigquery_emulator",
            "",
        ]
    )
    out_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("js_file", type=Path, help="Path to BigQuery console JS bundle")
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("backend/sqltools"),
        help="Output directory for generated C++ files",
    )
    args = parser.parse_args()

    text = args.js_file.read_text(encoding="utf-8")
    clause_words = extract_clause_words(text)
    expression_keywords = extract_keywords(text)
    functions_with_info = extract_function_objects(text, "this.FUNCTIONS_WITH_INFO = ")
    fallback_functions = extract_function_objects(text, "var Fzd = ")
    tvf_functions = extract_function_objects(text, "this.TVF_FUNCTIONS_WITH_INFO = ")
    type_names = extract_type_names(text)
    functions = merge_functions(functions_with_info, fallback_functions)

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    write_header(out_dir / "sql_completion_data.h")
    write_source(
        out_dir / "sql_completion_data.cc",
        clause_words,
        expression_keywords,
        functions,
        tvf_functions,
        type_names,
    )

    print(
        f"Generated {out_dir / 'sql_completion_data.h'} and "
        f"{out_dir / 'sql_completion_data.cc'} "
        f"({len(clause_words)} clause words, "
        f"{len(expression_keywords)} expression keywords, "
        f"{len(functions)} functions, "
        f"{len(tvf_functions)} TVFs, "
        f"{len(type_names)} type names)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
