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
	deps Dependencies,
	r *http.Request,
	projectID string,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	sql string,
	final engineScriptFinalResult,
) int {
	childCount := 0
	for _, raw := range splitScriptStatements(unwrapBeginEndBlock(sql)) {
		st := classifyScriptStatement(raw)
		switch st.kind {
		case scriptStmtDeclare, scriptStmtCall, scriptStmtSet:
			// DECLARE/CALL/SET already ran in the parent engine script round-trip.
			continue
		case scriptStmtQuery:
			childPosted := *posted
			childPosted.JobReference.JobID = ""
			childCfg := *cfg
			qCopy := *cfg.Query
			stmtSQL := substituteScriptVars(raw, nil)
			qCopy.Query = stmtSQL
			childCfg.Query = &qCopy
			child := newPendingJob(deps, projectID, &childPosted, &childCfg)
			stampChildJobParent(child, parent.JobReference.JobID)
			childStart := time.Now().UTC()
			childSchema := final.schema
			childRows := final.rows
			childStmtType := final.statementType
			childRoute := final.emulatorRoute
			childEnd := time.Now().UTC()
			finalizeDoneJob(deps, child, childStart, childEnd,
				childSchema, nil, childRows, childStmtType, childRoute, nil, nil, r)
			stampChildJobParent(child, parent.JobReference.JobID)
			childCount++
		}
	}
	return childCount
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
	childCount := registerEngineScriptChildJobs(
		deps, r, projectID, parent, posted, cfg, sql,
		engineScriptFinalResult{
			schema:        schema,
			rows:          rows,
			statementType: statementType,
			emulatorRoute: emulatorRoute,
		})
	return &scriptExecOutcome{
		childCount:    childCount,
		finalSchema:   schema,
		finalRows:     rows,
		finalStmtType: statementType,
		finalRoute:    emulatorRoute,
	}, nil
}
