package googlesqlcorpus

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
)

const (
	corpusValNaN  = "nan"
	corpusValInf  = "inf"
	corpusValNInf = "-inf"
)

// ExpectedResult is the parsed ARRAY<...>[...] expectation from a
// .test case.
type ExpectedResult struct {
	Ordered bool
	Rows    [][]any
}

// ParseExpected parses the GoogleSQL compliance result stanza.
func ParseExpected(raw string) (ExpectedResult, error) {
	raw = strings.TrimSpace(raw)
	if raw == "" {
		return ExpectedResult{}, errors.New("empty expected block")
	}
	p := &expectedParser{s: raw}
	rows, ordered, err := p.parseTopArray()
	if err != nil {
		return ExpectedResult{}, err
	}
	if p.pos < len(p.s) && strings.TrimSpace(p.s[p.pos:]) != "" {
		return ExpectedResult{}, fmt.Errorf("trailing input at %d", p.pos)
	}
	return ExpectedResult{Ordered: ordered, Rows: rows}, nil
}

type expectedParser struct {
	s   string
	pos int
}

func (p *expectedParser) parseTopArray() ([][]any, bool, error) {
	p.skipSpace()
	if !p.consume("ARRAY<") {
		return nil, false, fmt.Errorf("expected ARRAY< at %d", p.pos)
	}
	if err := p.skipTypeExpr(); err != nil {
		return nil, false, err
	}
	if !p.consume(">") {
		return nil, false, fmt.Errorf("expected > at %d", p.pos)
	}
	p.skipSpace()
	if !p.consume("[") {
		return nil, false, fmt.Errorf("expected [ at %d", p.pos)
	}
	ordered := p.consume("known order:")

	rows, err := p.parseRows()
	if err != nil {
		return nil, false, err
	}
	if !p.consume("]") {
		return nil, false, fmt.Errorf("expected ] at %d", p.pos)
	}
	return rows, ordered, nil
}

func (p *expectedParser) skipTypeExpr() error {
	depth := 1
	for p.pos < len(p.s) && depth > 0 {
		switch p.s[p.pos] {
		case '<':
			depth++
			p.pos++
		case '>':
			depth--
			if depth > 0 {
				p.pos++
			}
		default:
			p.pos++
		}
	}
	if depth != 0 {
		return fmt.Errorf("unterminated type expr at %d", p.pos)
	}
	return nil
}

func (p *expectedParser) parseRows() ([][]any, error) {
	p.skipSpace()
	if p.peek() == ']' {
		return nil, nil
	}
	var rows [][]any
	for {
		p.skipSpace()
		cells, err := p.parseRow()
		if err != nil {
			return nil, err
		}
		rows = append(rows, cells)
		p.skipSpace()
		if p.peek() == ']' || p.peek() == 0 {
			break
		}
		if !p.consume(",") {
			return nil, fmt.Errorf("expected , between rows at %d", p.pos)
		}
	}
	return rows, nil
}

func (p *expectedParser) parseRow() ([]any, error) {
	if !p.consume("{") {
		return nil, fmt.Errorf("expected { at %d", p.pos)
	}
	var cells []any
	for {
		p.skipSpace()
		if p.consume("}") {
			return cells, nil
		}
		if len(cells) > 0 {
			if !p.consume(",") {
				return nil, fmt.Errorf("expected , between cells at %d", p.pos)
			}
			p.skipSpace()
		}
		val, err := p.parseValue()
		if err != nil {
			return nil, err
		}
		cells = append(cells, val)
	}
}

func (p *expectedParser) parseValue() (any, error) {
	p.skipSpace()
	if p.peek() == '{' {
		nested, err := p.parseRow()
		if err != nil {
			return nil, err
		}
		return nested, nil
	}
	if p.consume("NULL") {
		return nil, nil
	}
	if p.consume("true") {
		return true, nil
	}
	if p.consume("false") {
		return false, nil
	}
	if p.consume(corpusValNaN) {
		return corpusValNaN, nil
	}
	if p.consume(corpusValNInf) {
		return corpusValNInf, nil
	}
	if p.consume(corpusValInf) {
		return corpusValInf, nil
	}
	if v, ok, err := p.tryParseQuotedValue(); ok || err != nil {
		return v, err
	}
	if v, ok, err := p.tryParseTypedLiteral(); ok || err != nil {
		return v, err
	}
	if isDigit(p.peek()) || p.peek() == '-' {
		return p.readBareToken(), nil
	}
	return nil, fmt.Errorf("unexpected value at %d", p.pos)
}

func (p *expectedParser) tryParseQuotedValue() (any, bool, error) {
	if p.consume("b\"") || p.consume("b'") {
		quote := p.s[p.pos-1]
		s, err := p.readQuoted(quote)
		if err != nil {
			return nil, true, err
		}
		return "b:" + s, true, nil
	}
	if p.peek() == '"' || p.peek() == '\'' || p.peek() == '`' {
		q := p.peek()
		p.pos++
		s, err := p.readQuoted(q)
		if err != nil {
			return nil, true, err
		}
		return s, true, nil
	}
	return nil, false, nil
}

func (p *expectedParser) tryParseTypedLiteral() (any, bool, error) {
	for _, spec := range []struct {
		prefix string
		tag    string
	}{
		{"DATE ", "DATE:"},
		{"TIMESTAMP ", "TIMESTAMP:"},
		{"TIME ", "TIME:"},
		{"DATETIME ", "DATETIME:"},
	} {
		if !p.consume(spec.prefix) {
			continue
		}
		p.skipSpace()
		s, err := p.readDateOrTimestampLiteral()
		if err != nil {
			return nil, true, err
		}
		return spec.tag + s, true, nil
	}
	return nil, false, nil
}

func (p *expectedParser) readQuoted(quote byte) (string, error) {
	var b strings.Builder
	for p.pos < len(p.s) {
		c := p.s[p.pos]
		p.pos++
		if c == '\\' && p.pos < len(p.s) {
			b.WriteByte(p.s[p.pos])
			p.pos++
			continue
		}
		if c == quote {
			return b.String(), nil
		}
		b.WriteByte(c)
	}
	return "", fmt.Errorf("unterminated string at %d", p.pos)
}

func (p *expectedParser) readDateOrTimestampLiteral() (string, error) {
	p.skipSpace()
	if p.peek() == '"' || p.peek() == '\'' {
		q := p.peek()
		p.pos++
		return p.readQuoted(q)
	}
	return p.readBareToken(), nil
}

func (p *expectedParser) readBareToken() string {
	start := p.pos
	for p.pos < len(p.s) {
		c := p.s[p.pos]
		if c == ',' || c == '}' || c == ']' || c == ' ' || c == '\n' || c == '\t' {
			break
		}
		p.pos++
	}
	return strings.TrimSpace(p.s[start:p.pos])
}

func (p *expectedParser) skipSpace() {
	for p.pos < len(p.s) {
		switch p.s[p.pos] {
		case ' ', '\t', '\n', '\r':
			p.pos++
		default:
			return
		}
	}
}

func (p *expectedParser) peek() byte {
	if p.pos >= len(p.s) {
		return 0
	}
	return p.s[p.pos]
}

func (p *expectedParser) consume(tok string) bool {
	p.skipSpace()
	if !strings.HasPrefix(p.s[p.pos:], tok) {
		return false
	}
	p.pos += len(tok)
	return true
}

func isDigit(c byte) bool {
	return c >= '0' && c <= '9'
}

// ToRunnerRows maps positional expected cells onto the gateway schema's
// column names for typed comparison in the fixture diff engine.
func ToRunnerRows(cells [][]any, colNames []string) []map[string]any {
	out := make([]map[string]any, 0, len(cells))
	for _, row := range cells {
		m := make(map[string]any, len(colNames))
		for i, col := range colNames {
			if i < len(row) {
				m[col] = normalizeExpectedCell(row[i])
			}
		}
		out = append(out, m)
	}
	return out
}

func normalizeExpectedCell(v any) any {
	switch x := v.(type) {
	case string:
		if strings.HasPrefix(x, "b:") {
			return x[2:]
		}
		if strings.HasPrefix(x, "DATE:") {
			return x[5:]
		}
		if strings.HasPrefix(x, "TIMESTAMP:") {
			return x[10:]
		}
		if strings.HasPrefix(x, "TIME:") {
			return x[5:]
		}
		if strings.HasPrefix(x, "DATETIME:") {
			return x[9:]
		}
		if x == corpusValNaN {
			return "NaN"
		}
		if x == corpusValInf {
			return corpusValInf
		}
		if x == corpusValNInf {
			return corpusValNInf
		}
		if f, err := strconv.ParseFloat(x, 64); err == nil {
			return f
		}
		return x
	case []any:
		// Nested struct rendered as JSON-ish for STRUCT columns until
		// the lane grows struct-aware corpus cases.
		return fmt.Sprint(x)
	default:
		return v
	}
}
