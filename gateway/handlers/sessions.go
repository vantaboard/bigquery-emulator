package handlers

import (
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/session"
)

func sessionStore(deps *Dependencies) *session.Store {
	if deps.Sessions == nil {
		deps.Sessions = NewSessionStore()
	}
	return deps.Sessions
}

func queryJobConnectionProperties(cfg *jobs.JobConfiguration) []bqtypes.ConnectionProperty {
	if cfg == nil || cfg.Query == nil {
		return nil
	}
	return cfg.Query.ConnectionProperties
}

func queryJobCreateSession(cfg *jobs.JobConfiguration) bool {
	return cfg != nil && cfg.Query != nil && cfg.Query.CreateSession
}

func stampJobSessionInfo(job *jobs.Job, info *bqtypes.SessionInfo) {
	if job == nil || info == nil {
		return
	}
	job.Statistics.SessionInfo = info
}
