package bqstorage

import (
	"strings"
	"time"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/protobuf/types/known/timestamppb"
)

func engineCreateReadSessionRequest(
	in *storagepb.CreateReadSessionRequest,
) *enginepb.CreateReadSessionRequest {
	if in == nil {
		return nil
	}
	out := &enginepb.CreateReadSessionRequest{
		Parent:         in.GetParent(),
		MaxStreamCount: in.GetMaxStreamCount(),
	}
	if rs := in.GetReadSession(); rs != nil {
		out.ReadSession = &enginepb.ReadSession{
			Table: rs.GetTable(),
		}
		if opts := rs.GetReadOptions(); opts != nil {
			out.ReadSession.ReadOptions = &enginepb.ReadOptions{
				SelectedFields: append([]string(nil), opts.GetSelectedFields()...),
				RowRestriction: opts.GetRowRestriction(),
			}
		}
	}
	return out
}

func publicReadSessionFromEngine(
	in *enginepb.ReadSession,
	dataFormat storagepb.DataFormat,
) (*storagepb.ReadSession, error) {
	if in == nil {
		return nil, nil
	}
	out := &storagepb.ReadSession{
		Name:  in.GetName(),
		Table: in.GetTable(),
	}
	if opts := in.GetReadOptions(); opts != nil {
		out.ReadOptions = &storagepb.ReadSession_TableReadOptions{
			SelectedFields: append([]string(nil), opts.GetSelectedFields()...),
			RowRestriction: opts.GetRowRestriction(),
		}
	}
	for _, st := range in.GetStreams() {
		out.Streams = append(out.Streams, &storagepb.ReadStream{Name: st.GetName()})
	}
	switch dataFormat {
	case storagepb.DataFormat_ARROW:
		arrowSchema, err := serializeArrowSchema(in.GetSchema())
		if err != nil {
			return nil, err
		}
		out.Schema = &storagepb.ReadSession_ArrowSchema{ArrowSchema: arrowSchema}
		out.DataFormat = storagepb.DataFormat_ARROW
	case storagepb.DataFormat_AVRO:
		avroSchema, err := serializeAvroSchema(in.GetSchema())
		if err != nil {
			return nil, err
		}
		out.Schema = &storagepb.ReadSession_AvroSchema{AvroSchema: avroSchema}
		out.DataFormat = storagepb.DataFormat_AVRO
	default:
		out.DataFormat = storagepb.DataFormat_ARROW
		if arrowSchema, err := serializeArrowSchema(in.GetSchema()); err == nil {
			out.Schema = &storagepb.ReadSession_ArrowSchema{ArrowSchema: arrowSchema}
		}
	}
	return out, nil
}

func engineTableSchemaToPublic(in *enginepb.TableSchema) *storagepb.TableSchema {
	if in == nil {
		return nil
	}
	out := &storagepb.TableSchema{}
	for _, f := range in.GetFields() {
		out.Fields = append(out.Fields, engineFieldToPublic(f))
	}
	return out
}

func engineFieldToPublic(f *enginepb.FieldSchema) *storagepb.TableFieldSchema {
	if f == nil {
		return nil
	}
	return &storagepb.TableFieldSchema{
		Name:        f.GetName(),
		Type:        engineTypeToPublic(f.GetType()),
		Mode:        engineModeToPublic(f.GetMode()),
		Description: f.GetDescription(),
	}
}

func engineTypeToPublic(t string) storagepb.TableFieldSchema_Type {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case bqTypeSTRING:
		return storagepb.TableFieldSchema_STRING
	case bqTypeBYTES:
		return storagepb.TableFieldSchema_BYTES
	case bqTypeINT64:
		return storagepb.TableFieldSchema_INT64
	case bqTypeFLOAT64:
		return storagepb.TableFieldSchema_DOUBLE
	case bqTypeBOOL:
		return storagepb.TableFieldSchema_BOOL
	case bqTypeTIMESTAMP:
		return storagepb.TableFieldSchema_TIMESTAMP
	case bqTypeDATE:
		return storagepb.TableFieldSchema_DATE
	case bqTypeTIME:
		return storagepb.TableFieldSchema_TIME
	case bqTypeDATETIME:
		return storagepb.TableFieldSchema_DATETIME
	case bqTypeNUMERIC:
		return storagepb.TableFieldSchema_NUMERIC
	case bqTypeBIGNUMERIC:
		return storagepb.TableFieldSchema_BIGNUMERIC
	case bqTypeJSON:
		return storagepb.TableFieldSchema_JSON
	case bqTypeGEOGRAPHY:
		return storagepb.TableFieldSchema_GEOGRAPHY
	case bqTypeSTRUCT, bqTypeRECORD:
		return storagepb.TableFieldSchema_STRUCT
	default:
		return storagepb.TableFieldSchema_STRING
	}
}

func engineModeToPublic(m string) storagepb.TableFieldSchema_Mode {
	switch strings.ToUpper(strings.TrimSpace(m)) {
	case bqModeRequired:
		return storagepb.TableFieldSchema_REQUIRED
	case bqModeRepeated:
		return storagepb.TableFieldSchema_REPEATED
	default:
		return storagepb.TableFieldSchema_NULLABLE
	}
}

func publicWriteTypeToEngine(t storagepb.WriteStream_Type) enginepb.WriteStream_Type {
	switch t {
	case storagepb.WriteStream_TYPE_COMMITTED:
		return enginepb.WriteStream_TYPE_COMMITTED
	case storagepb.WriteStream_TYPE_PENDING:
		return enginepb.WriteStream_TYPE_PENDING
	case storagepb.WriteStream_TYPE_BUFFERED:
		return enginepb.WriteStream_TYPE_BUFFERED
	default:
		return enginepb.WriteStream_TYPE_COMMITTED
	}
}

func engineWriteTypeToPublic(t enginepb.WriteStream_Type) storagepb.WriteStream_Type {
	switch t {
	case enginepb.WriteStream_TYPE_COMMITTED:
		return storagepb.WriteStream_TYPE_COMMITTED
	case enginepb.WriteStream_TYPE_PENDING:
		return storagepb.WriteStream_TYPE_PENDING
	case enginepb.WriteStream_TYPE_BUFFERED:
		return storagepb.WriteStream_TYPE_BUFFERED
	default:
		return storagepb.WriteStream_TYPE_UNSPECIFIED
	}
}

func publicWriteStreamFromEngine(in *enginepb.WriteStream) *storagepb.WriteStream {
	if in == nil {
		return nil
	}
	out := &storagepb.WriteStream{
		Name:        in.GetName(),
		Type:        engineWriteTypeToPublic(in.GetType()),
		TableSchema: engineTableSchemaToPublic(in.GetSchema()),
	}
	if ts := in.GetCreateTime(); ts != "" {
		if t, err := time.Parse(time.RFC3339, ts); err == nil {
			out.CreateTime = timestamppb.New(t)
		}
	}
	return out
}

func engineWriteStreamFromPublic(in *storagepb.WriteStream) *enginepb.WriteStream {
	if in == nil {
		return nil
	}
	return &enginepb.WriteStream{
		Type: publicWriteTypeToEngine(in.GetType()),
	}
}
