package bqstorage

import (
	"context"
	"errors"
	"fmt"
	"strconv"
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
	case bqModeRequired:
		return descriptorpb.FieldDescriptorProto_LABEL_REQUIRED.Enum()
	case "REPEATED":
		return descriptorpb.FieldDescriptorProto_LABEL_REPEATED.Enum()
	default:
		return descriptorpb.FieldDescriptorProto_LABEL_OPTIONAL.Enum()
	}
}

func engineTypeToProtoType(t string) *descriptorpb.FieldDescriptorProto_Type {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case bqTypeBOOL:
		return descriptorpb.FieldDescriptorProto_TYPE_BOOL.Enum()
	case bqTypeINT64:
		return descriptorpb.FieldDescriptorProto_TYPE_INT64.Enum()
	case bqTypeFLOAT64, "DOUBLE":
		return descriptorpb.FieldDescriptorProto_TYPE_DOUBLE.Enum()
	case bqTypeBYTES:
		return descriptorpb.FieldDescriptorProto_TYPE_BYTES.Enum()
	case bqTypeDATE:
		return descriptorpb.FieldDescriptorProto_TYPE_INT32.Enum()
	case bqTypeTIMESTAMP:
		return descriptorpb.FieldDescriptorProto_TYPE_INT64.Enum()
	case bqTypeDATETIME, bqTypeTIME:
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
		cell, err := fieldDescriptorToCell(msg, fd)
		if err != nil {
			return nil, err
		}
		cells = append(cells, cell)
	}
	return &enginepb.DataRow{Cells: cells}, nil
}

func fieldDescriptorToCell(msg protoreflect.Message, fd protoreflect.FieldDescriptor) (*enginepb.Cell, error) {
	if fd.IsList() {
		if !msg.Has(fd) {
			return &enginepb.Cell{Value: &enginepb.Cell_Array{Array: &enginepb.Array{}}}, nil
		}
		list := msg.Get(fd).List()
		elems := make([]*enginepb.Cell, list.Len())
		for i := 0; i < list.Len(); i++ {
			elem, err := protoreflectValueToCell(list.Get(i), fd)
			if err != nil {
				return nil, err
			}
			elems[i] = elem
		}
		return &enginepb.Cell{Value: &enginepb.Cell_Array{Array: &enginepb.Array{Elements: elems}}}, nil
	}
	if fd.Kind() == protoreflect.MessageKind {
		if !msg.Has(fd) {
			return &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}}, nil
		}
		return messageToStructCell(msg.Get(fd).Message())
	}
	if !msg.Has(fd) {
		return &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}}, nil
	}
	return protoreflectValueToCell(msg.Get(fd), fd)
}

func messageToStructCell(msg protoreflect.Message) (*enginepb.Cell, error) {
	fields := msg.Descriptor().Fields()
	fieldCells := make([]*enginepb.Cell, fields.Len())
	for i := 0; i < fields.Len(); i++ {
		fd := fields.Get(i)
		cell, err := fieldDescriptorToCell(msg, fd)
		if err != nil {
			return nil, err
		}
		fieldCells[i] = cell
	}
	return &enginepb.Cell{
		Value: &enginepb.Cell_StructValue{
			StructValue: &enginepb.Struct{Fields: fieldCells},
		},
	}, nil
}

func protoreflectValueToCell(v protoreflect.Value, fd protoreflect.FieldDescriptor) (*enginepb.Cell, error) {
	switch fd.Kind() {
	case protoreflect.MessageKind:
		return messageToStructCell(v.Message())
	case protoreflect.BoolKind:
		return &enginepb.Cell{
			Value: &enginepb.Cell_StringValue{StringValue: strconv.FormatBool(v.Bool())},
		}, nil
	case protoreflect.Int32Kind, protoreflect.Sint32Kind, protoreflect.Sfixed32Kind,
		protoreflect.Int64Kind, protoreflect.Sint64Kind, protoreflect.Sfixed64Kind:
		return int64Cell(v.Int()), nil
	case protoreflect.Uint32Kind, protoreflect.Fixed32Kind,
		protoreflect.Uint64Kind, protoreflect.Fixed64Kind:
		return int64Cell(uint64ToSignedInt64(v.Uint())), nil
	case protoreflect.FloatKind:
		return &enginepb.Cell{
			Value: &enginepb.Cell_StringValue{
				StringValue: strconv.FormatFloat(float64(v.Float()), 'g', -1, 32),
			},
		}, nil
	case protoreflect.DoubleKind:
		return &enginepb.Cell{
			Value: &enginepb.Cell_StringValue{
				StringValue: strconv.FormatFloat(v.Float(), 'g', -1, 64),
			},
		}, nil
	case protoreflect.StringKind:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: v.String()}}, nil
	case protoreflect.BytesKind:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: string(v.Bytes())}}, nil
	default:
		return nil, fmt.Errorf("unsupported proto field kind %v", fd.Kind())
	}
}

func int64Cell(n int64) *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: strconv.FormatInt(n, 10)}}
}
