package connection

import (
	"context"
	"database/sql"
	"testing"

	_ "github.com/vantaboard/go-googlesql-engine"
)

func TestElasticPoolGrowsToMax(t *testing.T) {
	db, err := sql.Open("googlesqlengine", ":memory:")
	if err != nil {
		t.Skip(err)
	}
	defer db.Close()

	ctx := context.Background()
	mgr, err := NewManager(ctx, db, WithElasticPool(2, 5, false))
	if err != nil {
		t.Fatal(err)
	}
	defer mgr.Close()

	acquired := make([]*ManagedConnection, 0, 5)
	for i := 0; i < 5; i++ {
		c, err := mgr.acquireConn(ctx)
		if err != nil {
			t.Fatalf("acquire %d: %v", i, err)
		}
		acquired = append(acquired, c)
	}
	if mgr.live != 5 {
		t.Fatalf("live=%d want 5", mgr.live)
	}
	for _, c := range acquired {
		mgr.releaseConnAfterUse(c)
	}
}

func TestSetCurrentMaxEvictsIdle(t *testing.T) {
	db, err := sql.Open("googlesqlengine", ":memory:")
	if err != nil {
		t.Skip(err)
	}
	defer db.Close()

	ctx := context.Background()
	mgr, err := NewManager(ctx, db, WithElasticPool(2, 6, false))
	if err != nil {
		t.Fatal(err)
	}
	defer mgr.Close()

	var held []*ManagedConnection
	for i := 0; i < 4; i++ {
		c, err := mgr.acquireConn(ctx)
		if err != nil {
			t.Fatal(err)
		}
		held = append(held, c)
	}
	for _, c := range held {
		mgr.releaseConnAfterUse(c)
	}
	mgr.SetCurrentMax(2)
	if mgr.live != 2 {
		t.Fatalf("live=%d want 2", mgr.live)
	}
}
