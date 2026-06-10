package handlers

import (
	"context"
	"net/http"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

type engineScriptFinalResult struct {
	schema        *enginepb.TableSchema
	rows          []bqtypes.Row
	statementType string
	emulatorRoute string
}

func registerEngineScriptChildJobs(
	ctx context.Context,
	deps Dependencies,
	r *http.Request,
	projectID, defaultDataset string,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	sql string,
	useLegacy bool,
	final engineScriptFinalResult,
) (int, error) {
	childCount := 0
	for _, raw := range splitScriptStatements(unwrapBeginEndBlock(sql)) {
		st := classifyScriptStatement(raw)
		switch st.kind {
		case scriptStmtDeclare, scriptStmtCall:
			continue
		case scriptStmtSet, scriptStmtQuery:
			childPosted := *posted
			childPosted.JobReference.JobID = ""
			childCfg := *cfg
			qCopy := *cfg.Query
			stmtSQL := st.sql
			if st.kind == scriptStmtQuery {
				stmtSQL = substituteScriptVars(stmtSQL, nil)
			}
			qCopy.Query = stmtSQL
			childCfg.Query = &qCopy
			child := newPendingJob(deps, projectID, &childPosted, &childCfg)
			stampChildJobParent(child, parent.JobReference.JobID)
			childStart := time.Now().UTC()
			var childSchema *enginepb.TableSchema
			var childRows []bqtypes.Row
			var childStmtType, childRoute string
			var err error
			if st.kind == scriptStmtQuery {
				childSchema = final.schema
				childRows = final.rows
				childStmtType = final.statementType
				childRoute = final.emulatorRoute
			} else {
				childSchema, childRows, childStmtType, childRoute, err = executeScriptStatement(
					ctx, deps, projectID, defaultDataset, stmtSQL, useLegacy)
				if err != nil {
					return 0, err
				}
			}
			childEnd := time.Now().UTC()
			finalizeDoneJob(deps, child, childStart, childEnd,
				childSchema, nil, childRows, childStmtType, childRoute, nil, nil, r)
			stampChildJobParent(child, parent.JobReference.JobID)
			childCount++
		}
	}
	return childCount, nil
}

func runEngineScript(
	ctx context.Context,
	deps Dependencies,
	r *http.Request,
	projectID string,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	defaultDataset, sql string,
	useLegacy bool,
) (*scriptExecOutcome, error) {
	engineSQL := transformScriptDeclares(sql)
	schema, rows, statementType, emulatorRoute, err := executeScriptStatement(
		ctx, deps, projectID, defaultDataset, engineSQL, useLegacy)
	if err != nil {
		return nil, err
	}
	childCount, err := registerEngineScriptChildJobs(
		ctx, deps, r, projectID, defaultDataset, parent, posted, cfg, sql, useLegacy,
		engineScriptFinalResult{
			schema:        schema,
			rows:          rows,
			statementType: statementType,
			emulatorRoute: emulatorRoute,
		})
	if err != nil {
		return nil, err
	}
	return &scriptExecOutcome{
		childCount:    childCount,
		finalSchema:   schema,
		finalRows:     rows,
		finalStmtType: statementType,
		finalRoute:    emulatorRoute,
	}, nil
}
