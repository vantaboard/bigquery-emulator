package bqstorage

import (
	"testing"

	"google.golang.org/protobuf/types/descriptorpb"
)

func TestEngineTypeToProtoType(t *testing.T) {
	t.Parallel()
	cases := []struct {
		bqType string
		want   descriptorpb.FieldDescriptorProto_Type
	}{
		{bqType: bqTypeDATE, want: descriptorpb.FieldDescriptorProto_TYPE_INT32},
		{bqType: bqTypeTIMESTAMP, want: descriptorpb.FieldDescriptorProto_TYPE_INT64},
		{bqType: bqTypeTIME, want: descriptorpb.FieldDescriptorProto_TYPE_STRING},
		{bqType: bqTypeDATETIME, want: descriptorpb.FieldDescriptorProto_TYPE_STRING},
	}
	for _, tc := range cases {
		t.Run(tc.bqType, func(t *testing.T) {
			t.Parallel()
			got := engineTypeToProtoType(tc.bqType)
			if got == nil || *got != tc.want {
				t.Fatalf("engineTypeToProtoType(%q) = %v, want %v", tc.bqType, got, tc.want)
			}
		})
	}
}
