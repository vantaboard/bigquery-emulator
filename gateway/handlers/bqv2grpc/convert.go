package bqv2grpc

import (
	"strconv"
	"strings"
	"time"

	"cloud.google.com/go/bigquery/v2/apiv2/bigquerypb"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/routines"
	"google.golang.org/protobuf/types/known/wrapperspb"
)

const (
	datasetKind = "bigquery#dataset"
	tableKind   = "bigquery#table"
	jobKind     = "bigquery#job"
)

func nowMillis() int64 {
	return time.Now().UnixMilli()
}

func datasetFromREST(projectID, datasetID string, ds bqtypes.Dataset) *bigquerypb.Dataset {
	if ds.Labels == nil {
		ds.Labels = bqtypes.ResourceLabels{}
	}
	if ds.Access == nil {
		ds.Access = []map[string]any{}
	}
	if ds.Location == "" {
		ds.Location = "US"
	}
	ct := parseMillis(ds.CreationTime)
	if ct == 0 {
		ct = nowMillis()
	}
	lmt := parseMillis(ds.LastModifiedTime)
	if lmt == 0 {
		lmt = nowMillis()
	}
	out := &bigquerypb.Dataset{
		Kind:             datasetKind,
		Id:               projectID + ":" + datasetID,
		DatasetReference: &bigquerypb.DatasetReference{ProjectId: projectID, DatasetId: datasetID},
		Location:         ds.Location,
		Labels:           map[string]string(ds.Labels),
		CreationTime:     ct,
		LastModifiedTime: lmt,
		Etag:             ds.Etag,
	}
	if ds.FriendlyName != "" {
		out.FriendlyName = wrapperspb.String(ds.FriendlyName)
	}
	if ds.Description != "" {
		out.Description = wrapperspb.String(ds.Description)
	}
	return out
}

func datasetToREST(ds *bigquerypb.Dataset) bqtypes.Dataset {
	if ds == nil {
		return bqtypes.Dataset{}
	}
	out := bqtypes.Dataset{
		Kind:             ds.GetKind(),
		ID:               ds.GetId(),
		FriendlyName:     ds.GetFriendlyName().GetValue(),
		Description:      ds.GetDescription().GetValue(),
		Location:         ds.GetLocation(),
		Etag:             ds.GetEtag(),
		CreationTime:     formatMillis(ds.GetCreationTime()),
		LastModifiedTime: formatMillis(ds.GetLastModifiedTime()),
		Labels:           bqtypes.ResourceLabels(ds.GetLabels()),
		Access:           []map[string]any{},
	}
	if ref := ds.GetDatasetReference(); ref != nil {
		out.DatasetReference = bqtypes.DatasetReference{
			ProjectID: ref.GetProjectId(),
			DatasetID: ref.GetDatasetId(),
		}
	}
	return out
}

func listDatasetFromRef(projectID, datasetID string, labels map[string]string) *bigquerypb.ListFormatDataset {
	if labels == nil {
		labels = map[string]string{}
	}
	return &bigquerypb.ListFormatDataset{
		Kind: datasetKind,
		Id:   projectID + ":" + datasetID,
		DatasetReference: &bigquerypb.DatasetReference{
			ProjectId: projectID,
			DatasetId: datasetID,
		},
		Labels: labels,
	}
}

func tableFromREST(projectID, datasetID, tableID string, t bqtypes.Table) *bigquerypb.Table {
	if t.Labels == nil {
		t.Labels = bqtypes.ResourceLabels{}
	}
	if t.Type == "" {
		t.Type = tableTypeTable
	}
	if t.Location == "" {
		t.Location = "US"
	}
	ct := parseMillis(t.CreationTime)
	if ct == 0 {
		ct = nowMillis()
	}
	lmt := parseMillis(t.LastModifiedTime)
	if lmt == 0 {
		lmt = nowMillis()
	}
	out := &bigquerypb.Table{
		Kind: tableKind,
		Id:   projectID + ":" + datasetID + "." + tableID,
		TableReference: &bigquerypb.TableReference{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
		Type:             t.Type,
		Labels:           map[string]string(t.Labels),
		CreationTime:     ct,
		LastModifiedTime: uint64FromNonNegativeInt64(lmt),
		Etag:             t.Etag,
		Location:         t.Location,
		Schema:           schemaToProto(t.Schema),
	}
	if n := parseInt64(t.NumRows); n > 0 {
		out.NumRows = wrapperspb.UInt64(uint64(n))
	}
	if n := parseInt64(t.NumBytes); n > 0 {
		out.NumBytes = wrapperspb.Int64(n)
	}
	if t.FriendlyName != "" {
		out.FriendlyName = wrapperspb.String(t.FriendlyName)
	}
	if t.Description != "" {
		out.Description = wrapperspb.String(t.Description)
	}
	return out
}

func tableToREST(t *bigquerypb.Table) bqtypes.Table {
	if t == nil {
		return bqtypes.Table{}
	}
	out := bqtypes.Table{
		Kind:             t.GetKind(),
		ID:               t.GetId(),
		FriendlyName:     t.GetFriendlyName().GetValue(),
		Description:      t.GetDescription().GetValue(),
		Type:             t.GetType(),
		Etag:             t.GetEtag(),
		CreationTime:     formatMillis(t.GetCreationTime()),
		LastModifiedTime: formatMillis(int64FromUint64(t.GetLastModifiedTime())),
		NumRows:          formatUInt64(t.GetNumRows().GetValue()),
		NumBytes:         formatInt64(t.GetNumBytes().GetValue()),
		Location:         t.GetLocation(),
		Labels:           bqtypes.ResourceLabels(t.GetLabels()),
		Schema:           schemaFromProto(t.GetSchema()),
	}
	if ref := t.GetTableReference(); ref != nil {
		out.TableReference = bqtypes.TableReference{
			ProjectID: ref.GetProjectId(),
			DatasetID: ref.GetDatasetId(),
			TableID:   ref.GetTableId(),
		}
	}
	return out
}

func listTableFromRef(
	projectID, datasetID, tableID, tableType string,
	labels map[string]string,
) *bigquerypb.ListFormatTable {
	if labels == nil {
		labels = map[string]string{}
	}
	if tableType == "" {
		tableType = "TABLE"
	}
	return &bigquerypb.ListFormatTable{
		Kind: tableKind,
		Id:   projectID + ":" + datasetID + "." + tableID,
		TableReference: &bigquerypb.TableReference{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		},
		Type:   tableType,
		Labels: labels,
	}
}

func schemaToProto(s *bqtypes.TableSchema) *bigquerypb.TableSchema {
	if s == nil {
		return nil
	}
	out := &bigquerypb.TableSchema{Fields: make([]*bigquerypb.TableFieldSchema, 0, len(s.Fields))}
	for i := range s.Fields {
		out.Fields = append(out.Fields, fieldToProto(s.Fields[i]))
	}
	return out
}

func fieldToProto(f bqtypes.TableFieldSchema) *bigquerypb.TableFieldSchema {
	out := &bigquerypb.TableFieldSchema{
		Name: f.Name,
		Type: f.Type,
		Mode: f.Mode,
	}
	if f.Description != "" {
		out.Description = wrapperspb.String(f.Description)
	}
	for i := range f.Fields {
		out.Fields = append(out.Fields, fieldToProto(f.Fields[i]))
	}
	return out
}

func schemaFromProto(s *bigquerypb.TableSchema) *bqtypes.TableSchema {
	if s == nil || len(s.GetFields()) == 0 {
		return nil
	}
	out := &bqtypes.TableSchema{Fields: make([]bqtypes.TableFieldSchema, 0, len(s.GetFields()))}
	for _, f := range s.GetFields() {
		out.Fields = append(out.Fields, fieldFromProto(f))
	}
	return out
}

func fieldFromProto(f *bigquerypb.TableFieldSchema) bqtypes.TableFieldSchema {
	fieldType := normalizeRESTFieldType(f.GetType())
	if strings.EqualFold(fieldType, "STRUCT") {
		fieldType = "RECORD"
	}
	out := bqtypes.TableFieldSchema{
		Name:        f.GetName(),
		Type:        fieldType,
		Mode:        f.GetMode(),
		Description: f.GetDescription().GetValue(),
	}
	for _, sub := range f.GetFields() {
		out.Fields = append(out.Fields, fieldFromProto(sub))
	}
	return out
}

func normalizeRESTFieldType(t string) string {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case "INT64":
		return "INTEGER"
	case "FLOAT64":
		return "FLOAT"
	case "BOOL":
		return "BOOLEAN"
	default:
		return t
	}
}

func schemaToEngine(s *bigquerypb.TableSchema) *enginepb.TableSchema {
	if s == nil {
		return nil
	}
	out := &enginepb.TableSchema{Fields: make([]*enginepb.FieldSchema, 0, len(s.GetFields()))}
	for _, f := range s.GetFields() {
		out.Fields = append(out.Fields, engineFieldFromProto(f))
	}
	return out
}

func engineFieldFromProto(f *bigquerypb.TableFieldSchema) *enginepb.FieldSchema {
	out := &enginepb.FieldSchema{
		Name:        f.GetName(),
		Type:        f.GetType(),
		Mode:        f.GetMode(),
		Description: f.GetDescription().GetValue(),
	}
	for _, sub := range f.GetFields() {
		out.Fields = append(out.Fields, engineFieldFromProto(sub))
	}
	return out
}

func jobReferenceToProto(ref bqtypes.JobReference) *bigquerypb.JobReference {
	out := &bigquerypb.JobReference{
		ProjectId: ref.ProjectID,
		JobId:     ref.JobID,
	}
	if ref.Location != "" {
		out.Location = wrapperspb.String(ref.Location)
	}
	return out
}

func jobListEntryToProto(j *jobs.Job) *bigquerypb.ListFormatJob {
	if j == nil {
		return nil
	}
	out := &bigquerypb.ListFormatJob{
		Kind:         jobKind,
		Id:           j.ID,
		JobReference: jobReferenceToProto(j.JobReference),
		State:        j.Status.State,
		Status: &bigquerypb.JobStatus{
			State: j.Status.State,
		},
		Statistics: &bigquerypb.JobStatistics{
			CreationTime: parseMillis(j.Statistics.CreationTime),
			StartTime:    parseMillis(j.Statistics.StartTime),
			EndTime:      parseMillis(j.Statistics.EndTime),
		},
	}
	if j.Configuration != nil {
		out.Configuration = &bigquerypb.JobConfiguration{
			JobType: j.Configuration.JobType,
		}
	}
	return out
}

func routineFromREST(projectID, datasetID, routineID string, rt bqtypes.Routine) *bigquerypb.Routine {
	ct := parseMillis(rt.CreationTime)
	if ct == 0 {
		ct = nowMillis()
	}
	lmt := parseMillis(rt.LastModifiedTime)
	if lmt == 0 {
		lmt = nowMillis()
	}
	out := &bigquerypb.Routine{
		Etag: rt.Etag,
		RoutineReference: &bigquerypb.RoutineReference{
			ProjectId: projectID,
			DatasetId: datasetID,
			RoutineId: routineID,
		},
		RoutineType:      routineTypeToProto(string(rt.RoutineType)),
		Language:         routineLanguageToProto(string(rt.Language)),
		DefinitionBody:   rt.DefinitionBody,
		CreationTime:     ct,
		LastModifiedTime: lmt,
	}
	if rt.Etag == "" {
		out.Etag = routines.MintEtag()
	}
	return out
}

func routineToREST(rt *bigquerypb.Routine) bqtypes.Routine {
	if rt == nil {
		return bqtypes.Routine{}
	}
	out := bqtypes.Routine{
		Etag:             rt.GetEtag(),
		RoutineType:      bqtypes.RoutineType(routineTypeFromProto(rt.GetRoutineType())),
		Language:         bqtypes.RoutineLanguage(routineLanguageFromProto(rt.GetLanguage())),
		DefinitionBody:   rt.GetDefinitionBody(),
		CreationTime:     formatMillis(rt.GetCreationTime()),
		LastModifiedTime: formatMillis(rt.GetLastModifiedTime()),
	}
	if ref := rt.GetRoutineReference(); ref != nil {
		out.RoutineReference = bqtypes.RoutineReference{
			ProjectID: ref.GetProjectId(),
			DatasetID: ref.GetDatasetId(),
			RoutineID: ref.GetRoutineId(),
		}
	}
	return out
}

func routineTypeToProto(s string) bigquerypb.Routine_RoutineType {
	switch strings.ToUpper(strings.TrimSpace(s)) {
	case "PROCEDURE":
		return bigquerypb.Routine_PROCEDURE
	case "TABLE_VALUED_FUNCTION":
		return bigquerypb.Routine_TABLE_VALUED_FUNCTION
	default:
		return bigquerypb.Routine_SCALAR_FUNCTION
	}
}

func routineTypeFromProto(t bigquerypb.Routine_RoutineType) string {
	switch t {
	case bigquerypb.Routine_PROCEDURE:
		return "PROCEDURE"
	case bigquerypb.Routine_TABLE_VALUED_FUNCTION:
		return "TABLE_VALUED_FUNCTION"
	default:
		return "SCALAR_FUNCTION"
	}
}

func routineLanguageToProto(s string) bigquerypb.Routine_Language {
	if strings.EqualFold(s, "JAVASCRIPT") {
		return bigquerypb.Routine_JAVASCRIPT
	}
	return bigquerypb.Routine_SQL
}

func routineLanguageFromProto(l bigquerypb.Routine_Language) string {
	if l == bigquerypb.Routine_JAVASCRIPT {
		return "JAVASCRIPT"
	}
	return "SQL"
}

func parseInt64(s string) int64 {
	n, err := strconv.ParseInt(s, 10, 64)
	if err != nil {
		return 0
	}
	return n
}

func formatInt64(n int64) string {
	if n == 0 {
		return "0"
	}
	return strconv.FormatInt(n, 10)
}

func formatUInt64(n uint64) string {
	if n == 0 {
		return "0"
	}
	return strconv.FormatUint(n, 10)
}
