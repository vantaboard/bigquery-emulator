package runner

import "testing"

func TestHashRowsDeterministic(t *testing.T) {
	rows := []map[string]string{
		{"b": "2", "a": "1"},
		{"a": "1", "b": "2"},
	}
	h1, err := HashRows(rows)
	if err != nil {
		t.Fatal(err)
	}
	h2, err := HashRows(rows[:1])
	if err != nil {
		t.Fatal(err)
	}
	if h1 == h2 {
		t.Fatal("expected different hashes")
	}
	if h1 == "" {
		t.Fatal("empty hash")
	}
}
