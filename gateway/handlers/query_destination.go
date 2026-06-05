package handlers

import (
	"regexp"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// createTempTableDestinationRE matches `CREATE TEMP TABLE `_SESSION`.table`
// and `CREATE TEMP TABLE `_SESSION`.`table“ shapes bigframes emits.
var createTempTableDestinationRE = regexp.MustCompile(
	"(?i)CREATE\\s+TEMP\\s+TABLE\\s+`([^`]+)`\\.(?:`([^`]+)`|([^\\s(]+))")

// stampQueryJobDestination fills configuration.query.destinationTable on
// CREATE TABLE / CREATE TEMP TABLE jobs so BigQuery clients (e.g.
// bigframes SessionResourceManager.create_temp_table) can read
// query_job.destination after jobs.insert.
func stampQueryJobDestination(projectID string, job *jobs.Job, statementType string) {
	if job == nil || job.Configuration == nil || job.Configuration.Query == nil {
		return
	}
	switch statementType {
	case "CREATE_TABLE", "CREATE_TABLE_AS_SELECT":
	default:
		return
	}
	dest := parseCreateTableDestination(projectID, job.Configuration.Query.Query)
	if dest != nil {
		job.Configuration.Query.DestinationTable = dest
	}
}

func parseCreateTableDestination(projectID, sql string) *bqtypes.TableReference {
	sql = strings.TrimSpace(sql)
	if m := createTempTableDestinationRE.FindStringSubmatch(sql); len(m) >= 3 {
		datasetID := m[1]
		tableID := m[2]
		if tableID == "" && len(m) > 3 {
			tableID = m[3]
		}
		if datasetID == "" || tableID == "" {
			return nil
		}
		return &bqtypes.TableReference{
			ProjectID: projectID,
			DatasetID: datasetID,
			TableID:   tableID,
		}
	}
	return nil
}
