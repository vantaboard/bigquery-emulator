package external

import "testing"

func TestValidateBigtableURI(t *testing.T) {
	t.Parallel()
	valid := "https://googleapis.com/bigtable/projects/p/instances/i/tables/t"
	if err := ValidateBigtableURI(valid); err != nil {
		t.Fatalf("ValidateBigtableURI(%q): %v", valid, err)
	}
	if err := ValidateBigtableURI("https://example.com/foo"); err == nil {
		t.Fatal("expected error for non-bigtable uri")
	}
}

func TestIsAzureBlobURI(t *testing.T) {
	t.Parallel()
	cases := []struct {
		uri string
		ok  bool
	}{
		{"azure://account/container/blob.csv", true},
		{"https://acct.blob.core.windows.net/c/x.csv", true},
		{"gs://bkt/x.csv", false},
	}
	for _, tc := range cases {
		if got := IsAzureBlobURI(tc.uri); got != tc.ok {
			t.Errorf("IsAzureBlobURI(%q) = %v, want %v", tc.uri, got, tc.ok)
		}
	}
}
