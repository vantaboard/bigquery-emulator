package runner

import (
	"encoding/json"
	"fmt"
	"io"
	"os"
	"time"
)

// CaseResult is the aggregated outcome for one (case, target) pair.
type CaseResult struct {
	CaseName       string        `json:"case_name"`
	Target         TargetName    `json:"target"`
	ContentHash    string        `json:"content_hash,omitempty"`
	Outcome        Outcome       `json:"outcome"`
	Error          string        `json:"error,omitempty"`
	Latency        LatencyStats  `json:"latency"`
	ExecutionP50   time.Duration `json:"execution_p50,omitempty"`
	BytesProcessed int64         `json:"bytes_processed,omitempty"`
	Route          string        `json:"route,omitempty"`
	Phases         PhaseStats    `json:"phases,omitempty"`
	ResultHash     string        `json:"result_hash,omitempty"`
	RowCount       int           `json:"row_count,omitempty"`
	Pass           *bool         `json:"pass,omitempty"`
	CompareReason  string        `json:"compare_reason,omitempty"`
	BQTotalP50MS   int64         `json:"bq_total_p50_ms,omitempty"`
	Ratio          float64       `json:"ratio_vs_bq,omitempty"`
}

// RunReport is the machine-readable benchmark output.
type RunReport struct {
	Timestamp  time.Time    `json:"timestamp"`
	CommitSHA  string       `json:"commit_sha,omitempty"`
	Host       string       `json:"host,omitempty"`
	GoccyImage string       `json:"goccy_image,omitempty"`
	Targets    []TargetName `json:"targets"`
	Results    []CaseResult `json:"results"`
}

// SaveReport writes JSON results.
func SaveReport(path string, r RunReport) error {
	r.Timestamp = r.Timestamp.UTC()
	raw, err := json.MarshalIndent(r, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(path, append(raw, '\n'), 0o644)
}

// LoadReport reads a results JSON file.
func LoadReport(path string) (RunReport, error) {
	raw, err := os.ReadFile(path)
	if err != nil {
		return RunReport{}, err
	}
	var r RunReport
	if err := json.Unmarshal(raw, &r); err != nil {
		return RunReport{}, err
	}
	return r, nil
}

// PrintTextReport renders a human-readable summary.
func PrintTextReport(w io.Writer, report RunReport, baseline *BaselineFile) {
	_, _ = fmt.Fprintf(w, "benchmark report @ %s\n", report.Timestamp.Format(time.RFC3339))
	if report.CommitSHA != "" {
		_, _ = fmt.Fprintf(w, "commit: %s\n", report.CommitSHA)
	}
	if report.GoccyImage != "" {
		_, _ = fmt.Fprintf(w, "goccy image: %s\n", report.GoccyImage)
	}
	_, _ = fmt.Fprintf(w, "\n%-24s %-10s %-8s %-10s %-12s %-8s %s\n",
		"case", "target", "outcome", "p50", "route", "rows", "notes")
	for _, r := range report.Results {
		notes := r.CompareReason
		if notes == "" && r.Error != "" {
			notes = r.Error
		}
		_, _ = fmt.Fprintf(w, "%-24s %-10s %-8s %-10s %-12s %-8d %s\n",
			r.CaseName, r.Target, r.Outcome, r.Latency.P50, r.Route, r.RowCount, notes)
	}
	if baseline != nil {
		ok, total := 0, 0
		for _, r := range report.Results {
			if r.Target == TargetEmulator {
				total++
				if r.Pass != nil && *r.Pass {
					ok++
				}
			}
		}
		_, _ = fmt.Fprintf(w, "\nemulator vs baseline: %d/%d passed\n", ok, total)
	}
}
