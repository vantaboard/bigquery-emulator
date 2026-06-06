package bqstorage

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/reflect/protodesc"
	"google.golang.org/protobuf/reflect/protoreflect"
	"google.golang.org/protobuf/types/descriptorpb"
	"google.golang.org/protobuf/types/dynamicpb"
)

func protoDataToEngineRows(
	ctx context.Context,
	engineClient *engine.Client,
	writeStream string,
	data *storagepb.AppendRowsRequest_ProtoData,
	cachedDesc **descriptorpb.DescriptorProto,
) ([]*enginepb.DataRow, error) {
	if data == nil {
		return nil, nil
	}
	rows := data.GetRows()
	if rows == nil || len(rows.GetSerializedRows()) == 0 {
		return nil, nil
	}

	desc, err := resolveProtoDescriptor(ctx, engineClient, writeStream, data, cachedDesc)
	if err != nil {
		return nil, err
	}
	msgDesc, err := messageDescriptor(desc)
	if err != nil {
		return nil, err
	}
	out := make([]*enginepb.DataRow, 0, len(rows.GetSerializedRows()))
	for i, raw := range rows.GetSerializedRows() {
		msg := dynamicpb.NewMessage(msgDesc)
		if err := proto.Unmarshal(raw, msg); err != nil {
			return nil, fmt.Errorf("row %d unmarshal: %w", i, err)
		}
		row, err := dynamicMessageToDataRow(msg)
		if err != nil {
			return nil, fmt.Errorf("row %d decode: %w", i, err)
		}
		out = append(out, row)
	}
	return out, nil
}

func resolveProtoDescriptor(
	ctx context.Context,
	engineClient *engine.Client,
	writeStream string,
	data *storagepb.AppendRowsRequest_ProtoData,
	cachedDesc **descriptorpb.DescriptorProto,
) (*descriptorpb.DescriptorProto, error) {
	if cachedDesc == nil {
		return nil, errors.New("proto_rows missing writer_schema.proto_descriptor")
	}
	if desc := data.GetWriterSchema().GetProtoDescriptor(); desc != nil {
		*cachedDesc = desc
		return desc, nil
	}
	if *cachedDesc != nil {
		return *cachedDesc, nil
	}
	if engineClient == nil || engineClient.StorageWrite == nil || writeStream == "" {
		return nil, errors.New("proto_rows missing writer_schema.proto_descriptor")
	}
	stream, err := engineClient.StorageWrite.GetWriteStream(ctx, &enginepb.GetWriteStreamRequest{
		Name: writeStream,
	})
	if err != nil {
		return nil, fmt.Errorf("proto_rows missing writer_schema.proto_descriptor (GetWriteStream: %w)", err)
	}
	desc := descriptorFromEngineTableSchema(stream.GetSchema())
	if desc == nil {
		return nil, errors.New("proto_rows missing writer_schema.proto_descriptor")
	}
	*cachedDesc = desc
	return desc, nil
}

func descriptorFromEngineTableSchema(schema *enginepb.TableSchema) *descriptorpb.DescriptorProto {
	if schema == nil || len(schema.GetFields()) == 0 {
		return nil
	}
	desc := &descriptorpb.DescriptorProto{Name: new("Row")}
	for i, field := range schema.GetFields() {
		if field == nil {
			continue
		}
		desc.Field = append(desc.Field, &descriptorpb.FieldDescriptorProto{
			Name:   new(field.GetName()),
			Number: new(int32(i + 1)),
			Label:  engineModeToProtoLabel(field.GetMode()),
			Type:   engineTypeToProtoType(field.GetType()),
		})
	}
	if len(desc.Field) == 0 {
		return nil
	}
	return desc
}

func engineModeToProtoLabel(mode string) *descriptorpb.FieldDescriptorProto_Label {
	switch strings.ToUpper(strings.TrimSpace(mode)) {
	case "REQUIRED":
		return descriptorpb.FieldDescriptorProto_LABEL_REQUIRED.Enum()
	case "REPEATED":
		return descriptorpb.FieldDescriptorProto_LABEL_REPEATED.Enum()
	default:
		return descriptorpb.FieldDescriptorProto_LABEL_OPTIONAL.Enum()
	}
}

func engineTypeToProtoType(t string) *descriptorpb.FieldDescriptorProto_Type {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case "BOOL":
		return descriptorpb.FieldDescriptorProto_TYPE_BOOL.Enum()
	case "INT64":
		return descriptorpb.FieldDescriptorProto_TYPE_INT64.Enum()
	case "FLOAT64", "DOUBLE":
		return descriptorpb.FieldDescriptorProto_TYPE_DOUBLE.Enum()
	case "BYTES":
		return descriptorpb.FieldDescriptorProto_TYPE_BYTES.Enum()
	case "TIMESTAMP", "DATETIME", "DATE", "TIME":
		return descriptorpb.FieldDescriptorProto_TYPE_STRING.Enum()
	default:
		return descriptorpb.FieldDescriptorProto_TYPE_STRING.Enum()
	}
}

func messageDescriptor(desc *descriptorpb.DescriptorProto) (protoreflect.MessageDescriptor, error) {
	if desc == nil {
		return nil, errors.New("nil descriptor")
	}
	fileDesc := &descriptorpb.FileDescriptorProto{
		Name:    new("bqstorage_row.proto"),
		Package: new("bqstorage"),
		MessageType: []*descriptorpb.DescriptorProto{
			desc,
		},
	}
	fd, err := protodesc.NewFile(fileDesc, nil)
	if err != nil {
		return nil, err
	}
	md := fd.Messages().ByName(protoreflect.Name(desc.GetName()))
	if md == nil {
		return nil, fmt.Errorf("descriptor %q not found in file", desc.GetName())
	}
	return md, nil
}

func dynamicMessageToDataRow(msg protoreflect.Message) (*enginepb.DataRow, error) {
	fields := msg.Descriptor().Fields()
	cells := make([]*enginepb.Cell, 0, fields.Len())
	for i := 0; i < fields.Len(); i++ {
		fd := fields.Get(i)
		if !msg.Has(fd) {
			cells = append(cells, &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}})
			continue
		}
		cells = append(cells, protoValueToCell(msg.Get(fd).Interface()))
	}
	return &enginepb.DataRow{Cells: cells}, nil
}
