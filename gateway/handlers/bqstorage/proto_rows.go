package bqstorage

import (
	"errors"
	"fmt"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/reflect/protodesc"
	"google.golang.org/protobuf/reflect/protoreflect"
	"google.golang.org/protobuf/types/descriptorpb"
	"google.golang.org/protobuf/types/dynamicpb"
)

func protoDataToEngineRows(
	data *storagepb.AppendRowsRequest_ProtoData,
) ([]*enginepb.DataRow, error) {
	if data == nil {
		return nil, nil
	}
	rows := data.GetRows()
	if rows == nil || len(rows.GetSerializedRows()) == 0 {
		return nil, nil
	}
	desc := data.GetWriterSchema().GetProtoDescriptor()
	if desc == nil {
		return nil, errors.New("proto_rows missing writer_schema.proto_descriptor")
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
