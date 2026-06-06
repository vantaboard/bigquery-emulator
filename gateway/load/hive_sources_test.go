package load

import "testing"

func TestParseHiveCustomPrefix(t *testing.T) {
	t.Parallel()
	bucket, prefix, fields, err := parseHiveCustomPrefix(
		"gs://bkt/hive/customlayout/{pkey:STRING}/",
	)
	if err != nil {
		t.Fatalf("parseHiveCustomPrefix: %v", err)
	}
	if bucket != "bkt" {
		t.Fatalf("bucket = %q", bucket)
	}
	if prefix != "hive/customlayout/" {
		t.Fatalf("prefix = %q, want hive/customlayout/", prefix)
	}
	if len(fields) != 1 || fields[0].Name != "pkey" || fields[0].Type != "STRING" {
		t.Fatalf("fields = %#v", fields)
	}
}

func TestExtractCustomPartitions(t *testing.T) {
	t.Parallel()
	got, err := extractCustomPartitions(
		"gs://cloud-samples-data/bigquery/hive-partitioning-samples/customlayout/pkey=bar/file1.parquet",
		"gs://cloud-samples-data/bigquery/hive-partitioning-samples/customlayout/{pkey:STRING}/",
	)
	if err != nil {
		t.Fatalf("extractCustomPartitions: %v", err)
	}
	if got["pkey"] != "bar" {
		t.Fatalf("pkey = %q, want bar", got["pkey"])
	}
}
