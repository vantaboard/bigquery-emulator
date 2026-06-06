package bqstorage

import (
	"bytes"
	"errors"
	"fmt"
	"io"

	"github.com/apache/arrow/go/v18/arrow"
	"github.com/apache/arrow/go/v18/arrow/ipc"
)

type countingReader struct {
	r   io.Reader
	pos int64
}

func (c *countingReader) Read(p []byte) (int, error) {
	n, err := c.r.Read(p)
	c.pos += int64(n)
	return n, err
}

func ipcMessageAt(data []byte, index int) ([]byte, error) {
	r := &countingReader{r: bytes.NewReader(data)}
	msgRdr := ipc.NewMessageReader(r)
	defer msgRdr.Release()

	var (
		start int64
		end   int64
	)
	for i := 0; ; i++ {
		msgStart := r.pos
		msg, err := msgRdr.Message()
		if err != nil {
			if errors.Is(err, io.EOF) {
				return nil, fmt.Errorf("arrow ipc: message index %d out of range", index)
			}
			return nil, err
		}
		msgEnd := r.pos
		msg.Release()

		if i == index {
			start = msgStart
			end = msgEnd
			break
		}
	}
	return data[start:end], nil
}

func serializeArrowIPCSchema(as *arrow.Schema) ([]byte, error) {
	var stream bytes.Buffer
	w := ipc.NewWriter(&stream, ipc.WithSchema(as))
	if err := w.Close(); err != nil {
		return nil, err
	}
	return ipcMessageAt(stream.Bytes(), 0)
}

func serializeArrowIPCRecordBatch(as *arrow.Schema, rec arrow.Record) ([]byte, error) {
	var stream bytes.Buffer
	w := ipc.NewWriter(&stream, ipc.WithSchema(as))
	if err := w.Write(rec); err != nil {
		_ = w.Close()
		return nil, err
	}
	if err := w.Close(); err != nil {
		return nil, err
	}
	// Index 0 is schema; index 1 is the record batch (EOS follows).
	return ipcMessageAt(stream.Bytes(), 1)
}
