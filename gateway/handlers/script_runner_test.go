package handlers

import (
	"strings"
	"testing"
)

func TestIsMultiStatementScriptBqutilsFromHexSetup(t *testing.T) {
	const sql = `/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

-- from_hex:
CREATE FUNCTION from_hex(value STRING) 

AS
(
  (
    SELECT 
      SUM(
      	CAST(
      	  CONCAT('0x', SUBSTR(value, byte * 2 + 1, 2)) 
      	    AS INT64) << ((LENGTH(value) - (byte + 1) * 2) * 4))
    FROM UNNEST(GENERATE_ARRAY(1, LENGTH(value) / 2)) WITH OFFSET byte
  )
);`
	if isMultiStatementScript(sql) {
		t.Fatal("from_hex CREATE FUNCTION setup must not be treated as a script")
	}
}

func TestIsMultiStatementScriptDetectsSetScript(t *testing.T) {
	const sql = `SET x = 1; SELECT x;`
	if !isMultiStatementScript(sql) {
		t.Fatal("expected DECLARE/SET script detection")
	}
}

func TestNeedsEngineScriptExecutionDetectsDeclare(t *testing.T) {
	const sql = `DECLARE x INT64; SET x = 1; SELECT x;`
	if !needsEngineScriptExecution(sql) {
		t.Fatal("expected DECLARE script to use engine path")
	}
}

func TestTransformScriptDeclaresLowersDeclarePreservesSet(t *testing.T) {
	const sql = `DECLARE top_names ARRAY<STRING>;
SET top_names = (SELECT ['a']);
SELECT name FROM UNNEST(top_names) AS name;`
	got := transformScriptDeclares(sql)
	if strings.Contains(strings.ToUpper(got), "DECLARE ") {
		t.Fatalf("DECLARE not lowered: %q", got)
	}
	if !strings.Contains(strings.ToUpper(got), "SET TOP_NAMES") {
		t.Fatalf("SET statement removed: %q", got)
	}
}

func TestCountExecutableScriptChildStatements(t *testing.T) {
	const sql = `DECLARE top_names ARRAY<STRING>;
SET top_names = (SELECT ['a']);
SELECT name FROM UNNEST(top_names) AS name;`
	var childCount int
	for _, raw := range splitScriptStatements(unwrapBeginEndBlock(sql)) {
		st := classifyScriptStatement(raw)
		switch st.kind {
		case scriptStmtDeclare, scriptStmtCall:
			continue
		case scriptStmtSet, scriptStmtQuery:
			childCount++
		}
	}
	if childCount != 2 {
		t.Fatalf("childCount = %d, want 2", childCount)
	}
}
