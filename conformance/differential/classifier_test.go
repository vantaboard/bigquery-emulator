package differential

import "testing"

func TestClassifyDivergence(t *testing.T) {
	t.Parallel()
	cases := []struct {
		name string
		in   ClassifyInput
		want DivergenceKind
	}{
		{
			name: "match",
			in: ClassifyInput{
				OracleSuccess: true, EmulatorSuccess: true,
			},
			want: KindMatch,
		},
		{
			name: "semantic",
			in: ClassifyInput{
				OracleSuccess: true, EmulatorSuccess: true,
				Diff: "row mismatch",
			},
			want: KindSemanticDivergence,
		},
		{
			name: "error divergence",
			in: ClassifyInput{
				OracleSuccess: true, EmulatorSuccess: false,
				EmulatorStatus: 400,
			},
			want: KindErrorDivergence,
		},
		{
			name: "feature gap",
			in: ClassifyInput{
				OracleSuccess: true, EmulatorSuccess: false,
				RunnerMessage: "SetOperationScan op is not UNION ALL",
			},
			want: KindFeatureGap,
		},
		{
			name: "crash",
			in: ClassifyInput{
				RunnerMessage: "engine process exited with signal: aborted",
			},
			want: KindCrash,
		},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			if got := ClassifyDivergence(tc.in); got != tc.want {
				t.Fatalf("ClassifyDivergence() = %q, want %q", got, tc.want)
			}
		})
	}
}
