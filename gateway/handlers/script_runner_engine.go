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
	useLegacy bool,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	sql string,
	final engineScriptFinalResult,
) int {
	inner := unwrapBeginEndBlock(sql)
	if scriptNeedsGoogleSQLExecutor(inner) {
		if final.schema == nil && len(final.rows) == 0 {
			return 0
		}
		registerFinalSelectChildJob(
			deps, r, projectID, parent, posted, cfg, final)
		return 1
	}
	statements := splitScriptStatements(inner)
	lastQueryIdx := -1
	for i, raw := range statements {
		if classifyScriptStatement(raw).kind == scriptStmtQuery {
			lastQueryIdx = i
		}
	}
	childCount := 0
	for i, raw := range statements {
		st := classifyScriptStatement(raw)
		switch st.kind {
		case scriptStmtDeclare, scriptStmtCall:
			continue
		case scriptStmtSet:
			registerReExecutedEngineScriptChild(
				ctx, deps, r, projectID, defaultDataset, useLegacy,
				parent, posted, cfg, st.sql)
			childCount++
		case scriptStmtQuery:
			if i == lastQueryIdx {
				registerFinalSelectChildJob(
					deps, r, projectID, parent, posted, cfg, final)
				childCount++
			} else {
				registerReExecutedEngineScriptChild(
					ctx, deps, r, projectID, defaultDataset, useLegacy,
					parent, posted, cfg, st.sql)
				childCount++
			}
		}
	}
	return childCount
}

func registerReExecutedEngineScriptChild(
	ctx context.Context,
	deps Dependencies,
	r *http.Request,
	projectID, defaultDataset string,
	useLegacy bool,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	stmtSQL string,
) {
	childPosted := *posted
	childPosted.JobReference.JobID = ""
	childCfg := *cfg
	qCopy := *cfg.Query
	qCopy.Query = stmtSQL
	childCfg.Query = &qCopy
	child := newPendingJob(deps, projectID, &childPosted, &childCfg)
	stampChildJobParent(child, parent.JobReference.JobID)
	childStart := time.Now().UTC()
	schema, rows, statementType, emulatorRoute, err := executeScriptStatement(
		ctx, deps, projectID, defaultDataset, stmtSQL, useLegacy)
	childEnd := time.Now().UTC()
	if err != nil {
		finalizeFailedJob(deps, child, childStart, err)
		stampChildJobParent(child, parent.JobReference.JobID)
		return
	}
	finalizeDoneJob(deps, child, childStart, childEnd,
		schema, nil, rows, statementType, emulatorRoute, nil, nil, r)
	stampChildJobParent(child, parent.JobReference.JobID)
}

func registerFinalSelectChildJob(
	deps Dependencies,
	r *http.Request,
	projectID string,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	final engineScriptFinalResult,
) {
	childPosted := *posted
	childPosted.JobReference.JobID = ""
	childCfg := *cfg
	qCopy := *cfg.Query
	childCfg.Query = &qCopy
	child := newPendingJob(deps, projectID, &childPosted, &childCfg)
	stampChildJobParent(child, parent.JobReference.JobID)
	childStart := time.Now().UTC()
	childEnd := time.Now().UTC()
	finalizeDoneJob(deps, child, childStart, childEnd,
		final.schema, nil, final.rows, final.statementType, final.emulatorRoute,
		nil, nil, r)
	stampChildJobParent(child, parent.JobReference.JobID)
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
		ctx, deps, r, projectID, defaultDataset, useLegacy,
		parent, posted, cfg, sql,
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
