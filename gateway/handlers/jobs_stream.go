package handlers

import (
	"errors"
	"io"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// drainSyncStream is the JobInsert flavor of `streamQueryResults`:
// same proto contract, different error reporting. Whereas
// `streamQueryResults` writes an HTTP envelope and returns an
// `ok=false` short-circuit, this helper returns the raw stream error
// so the caller can fold it into the Job's status.
func drainSyncStream(stream enginepb.Query_ExecuteQueryClient) (
	*enginepb.TableSchema, *enginepb.DmlStats, []bqtypes.Row, string, string, map[string]int64, error,
) {
	var schema *enginepb.TableSchema
	var dmlStats *enginepb.DmlStats
	var statementType string
	var emulatorRoute string
	var emulatorPhases map[string]int64
	rows := make([]bqtypes.Row, 0)
	for {
		msg, err := stream.Recv()
		if err != nil {
			if errors.Is(err, io.EOF) {
				break
			}
			return nil, nil, nil, "", "", nil, err
		}
		if s := msg.GetSchema(); s != nil {
			if schema == nil {
				schema = s
			}
			continue
		}
		if d := msg.GetDmlStats(); d != nil {
			if dmlStats == nil {
				dmlStats = d
			}
			continue
		}
		if st := msg.GetStatementType(); st != "" {
			if statementType == "" {
				statementType = st
			}
			continue
		}
		if er := msg.GetEmulatorRoute(); er != "" {
			if emulatorRoute == "" {
				emulatorRoute = er
			}
			continue
		}
		if pt := msg.GetPhaseTimings(); pt != nil && len(pt.GetPhases()) > 0 {
			if emulatorPhases == nil {
				emulatorPhases = make(map[string]int64, len(pt.GetPhases()))
			}
			for _, phase := range pt.GetPhases() {
				if phase.GetName() != "" {
					emulatorPhases[phase.GetName()] = phase.GetDurationUs()
				}
			}
			continue
		}
		rows = append(rows, bqtypes.CellsToRowForSchema(msg.GetCells(), schema))
	}
	return schema, dmlStats, rows, statementType, emulatorRoute, emulatorPhases, nil
}

// defaultDatasetID extracts the dataset ID from an optional
// `defaultDataset` reference, returning empty when the field is
// absent. The wire field on the engine carries the dataset ID only;
// the project comes from `project_id`.
func defaultDatasetID(ref *bqtypes.DatasetReference) string {
	if ref == nil {
		return ""
	}
	return ref.DatasetID
}
