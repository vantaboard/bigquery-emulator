package main

import (
	"encoding/json"
	"fmt"
	"io"
	"os"
	"strconv"
)

// badgeJSON matches the shape shields.io's "endpoint" badge consumer
// requires. See https://shields.io/badges/endpoint-badge for the
// schema. We deliberately keep this struct narrow (no `cacheSeconds`
// override etc) because the README badges use shields.io's defaults
// and we don't want the badge JSON to grow extra knobs without a
// reason.
type badgeJSON struct {
	SchemaVersion int    `json:"schemaVersion"`
	Label         string `json:"label"`
	Message       string `json:"message"`
	Color         string `json:"color"`
}

// Color thresholds for the badges. These match the colour bands
// codecov uses on its default badges to keep the visual transition
// from the old badges to the new ones smooth.
const (
	colorBrightGreen = "brightgreen" // [90, 100]
	colorGreen       = "green"       // [80, 90)
	colorYellowGreen = "yellowgreen" // [70, 80)
	colorYellow      = "yellow"      // [60, 70)
	colorOrange      = "orange"      // [40, 60)
	colorRed         = "red"         // [0, 40)
	colorLightgrey   = "lightgrey"   // missing data
)

// badgeColor maps a percentage to the shields.io colour string. -1
// (the missingFlag sentinel) collapses to "lightgrey" so a flag with
// no data renders as a clearly-greyed-out badge rather than a "0%
// red" badge that would imply the suite ran and failed.
func badgeColor(pct float64) string {
	switch {
	case pct < 0:
		return colorLightgrey
	case pct >= 90:
		return colorBrightGreen
	case pct >= 80:
		return colorGreen
	case pct >= 70:
		return colorYellowGreen
	case pct >= 60:
		return colorYellow
	case pct >= 40:
		return colorOrange
	default:
		return colorRed
	}
}

// badgeMessage renders the percentage with one decimal place, except
// for the missing-data sentinel which renders as the literal "n/a".
// Keeping the format tight ("71.3%") matches the codecov badges we're
// replacing.
func badgeMessage(pct float64) string {
	if pct < 0 {
		return missingMessage
	}
	return strconv.FormatFloat(pct, 'f', 1, 64) + "%"
}

// pickField extracts the percentage for a given --field name from a
// Summary. Returns an error for unrecognized fields so a typo in the
// workflow doesn't silently default to the total.
func pickField(s *Summary, field string) (float64, error) {
	switch field {
	case "total":
		return s.Total, nil
	case "go":
		return s.Go, nil
	case "cpp":
		return s.CPP, nil
	default:
		return 0, fmt.Errorf("unknown --field %q (want one of: total, go, cpp)", field)
	}
}

// runBadge implements `coverage badge`. It reads a summary.json
// previously emitted by `coverage summarize`, picks one field, and
// writes the shields.io endpoint JSON either to --out or stdout.
func runBadge(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("badge", stderr)
	in := fs.String("in", "", "input summary.json path (required)")
	out := fs.String("out", "", "output badge JSON path (default: stdout)")
	field := fs.String("field", "total", "summary field to render (total|go|cpp)")
	label := fs.String("label", "coverage", "badge label")
	if err := fs.Parse(args); err != nil {
		return err
	}
	if *in == "" {
		_, _ = fmt.Fprintln(stderr, "badge: --in is required")
		fs.Usage()
		return errUsage
	}

	summary, err := readSummary(*in)
	if err != nil {
		return err
	}
	pct, err := pickField(summary, *field)
	if err != nil {
		return err
	}

	payload := badgeJSON{
		SchemaVersion: 1,
		Label:         *label,
		Message:       badgeMessage(pct),
		Color:         badgeColor(pct),
	}
	return emitBadge(&payload, *out, stdout)
}

func emitBadge(b *badgeJSON, outPath string, stdout io.Writer) error {
	buf, err := json.MarshalIndent(b, "", "  ")
	if err != nil {
		return fmt.Errorf("marshal badge: %w", err)
	}
	buf = append(buf, '\n')

	if outPath == "" {
		_, err = stdout.Write(buf)
		return err
	}
	//nolint:gosec // 0o644 is the right mode for a CI-published JSON artifact.
	if err := os.WriteFile(outPath, buf, 0o644); err != nil {
		return fmt.Errorf("write %q: %w", outPath, err)
	}
	return nil
}
