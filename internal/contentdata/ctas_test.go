package contentdata

import "testing"

func TestIsCTASQuery(t *testing.T) {
	t.Parallel()
	cases := []struct {
		name string
		q    string
		want bool
	}{
		{
			"ctas or replace",
			"create or replace table `p.d.t` as select 1 as x",
			true,
		},
		{
			"ctas paren",
			"CREATE TABLE a.b.c AS (SELECT 1)",
			true,
		},
		{
			"not create view",
			"create or replace view v as select 1",
			false,
		},
		{
			"select only",
			"select 1",
			false,
		},
		{
			"create table with columns not CTAS",
			"create table d.t (x int64, y string)",
			false,
		},
		{
			"invalid sql",
			"this is not valid sql ~~~",
			false,
		},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			if g := IsCTASQuery(tc.q); g != tc.want {
				t.Errorf("IsCTASQuery() = %v, want %v", g, tc.want)
			}
		})
	}
}
