package bqstorage

import (
	"context"
	"testing"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

func TestCreateReadSessionRequiresEngine(t *testing.T) {
	srv := &ReadServer{}
	_, err := srv.CreateReadSession(context.Background(), &storagepb.CreateReadSessionRequest{
		Parent: "projects/p",
		ReadSession: &storagepb.ReadSession{
			Table: testTableResource,
		},
	})
	if status.Code(err) != codes.Unavailable {
		t.Fatalf("CreateReadSession err = %v, want Unavailable", err)
	}
}

func TestPublicReadSessionFromEngineArrow(t *testing.T) {
	session, err := publicReadSessionFromEngine(&enginepb.ReadSession{
		Name:  "projects/p/locations/-/sessions/s1",
		Table: testTableResource,
		Schema: &enginepb.TableSchema{
			Fields: []*enginepb.FieldSchema{
				{Name: "id", Type: bqTypeINT64, Mode: bqModeRequired},
				{Name: "name", Type: bqTypeSTRING, Mode: bqModeNullable},
			},
		},
		Streams: []*enginepb.ReadStream{{Name: "projects/p/locations/-/sessions/s1/streams/0"}},
	}, storagepb.DataFormat_ARROW)
	if err != nil {
		t.Fatalf("publicReadSessionFromEngine: %v", err)
	}
	if session.GetArrowSchema() == nil || len(session.GetArrowSchema().GetSerializedSchema()) == 0 {
		t.Fatal("expected non-empty arrow_schema")
	}
	if len(session.GetStreams()) != 1 {
		t.Fatalf("streams = %d, want 1", len(session.GetStreams()))
	}
}
