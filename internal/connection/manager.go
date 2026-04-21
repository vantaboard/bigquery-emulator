package connection

import (
	"context"
	"database/sql"
	"fmt"
	"log/slog"
	"sync"
	"sync/atomic"
	"time"

	"github.com/vantaboard/go-googlesqlite"
)

const (
	// DefaultPoolSize is used only when legacy callers omit options (see NewManager).
	DefaultPoolSize = 5
)

// ManagerOption configures [NewManager].
type ManagerOption func(*PoolConfig)

// WithPoolSize sets a fixed pool size (disables elastic growth and autoscale loop).
func WithPoolSize(n int) ManagerOption {
	return func(cfg *PoolConfig) {
		if n > 0 {
			cfg.FixedSize = n
			cfg.PoolMin = n
			cfg.PoolMaxHard = n
			cfg.AutoscaleLoop = false
		}
	}
}

// WithElasticPool overrides computed bounds (tests or advanced tuning).
func WithElasticPool(poolMin, poolMaxHard int, autoscaleLoop bool) ManagerOption {
	return func(cfg *PoolConfig) {
		cfg.FixedSize = 0
		cfg.PoolMin = poolMin
		cfg.PoolMaxHard = poolMaxHard
		cfg.AutoscaleLoop = autoscaleLoop
	}
}

// WithPoolConfig replaces the merged [PoolConfig] (use after [PoolConfigFromEnv] in the caller if needed).
func WithPoolConfig(cfg PoolConfig) ManagerOption {
	return func(dst *PoolConfig) {
		*dst = cfg
	}
}

type Manager struct {
	db *sql.DB

	preparedQueryStrings []string

	mu       sync.Mutex
	allConns []*ManagedConnection
	idleCh   chan *ManagedConnection
	live     int

	poolMin     int
	poolMaxHard int
	currentMax  atomic.Int32

	closeOnce sync.Once
	closed    bool

	autosMu     sync.Mutex
	lastRaiseAt time.Time

	txConnMap map[*sql.Tx]*ManagedConnection
	txMu      sync.RWMutex
}

// NewManager creates a connection manager. Options merge with [PoolConfigFromEnv] unless you pass
// only [WithPoolSize] / [WithElasticPool] after defaults.
func NewManager(ctx context.Context, db *sql.DB, opts ...ManagerOption) (*Manager, error) {
	db.SetConnMaxLifetime(-1) // Keep connections alive
	db.SetConnMaxIdleTime(-1)

	cfg := PoolConfigFromEnv()
	for _, o := range opts {
		if o != nil {
			o(&cfg)
		}
	}

	poolMin := cfg.PoolMin
	poolMaxHard := cfg.PoolMaxHard
	if cfg.FixedSize > 0 {
		poolMin = cfg.FixedSize
		poolMaxHard = cfg.FixedSize
	}
	if poolMin <= 0 {
		poolMin = DefaultPoolSize
	}
	if poolMaxHard < poolMin {
		poolMaxHard = poolMin
	}

	manager := &Manager{
		db:        db,
		idleCh:    make(chan *ManagedConnection, poolMaxHard),
		poolMin:   poolMin,
		poolMaxHard: poolMaxHard,
		txConnMap: make(map[*sql.Tx]*ManagedConnection),
	}
	manager.currentMax.Store(int32(poolMaxHard))

	for i := 0; i < poolMin; i++ {
		if err := manager.createConnAndIdle(ctx); err != nil {
			_ = manager.Close()
			return nil, fmt.Errorf("failed to warm pool connection %d: %w", i, err)
		}
	}

	return manager, nil
}

func (m *Manager) createConnAndIdle(ctx context.Context) error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if m.closed {
		return fmt.Errorf("repository is closed")
	}
	c, err := m.createConnLocked(ctx)
	if err != nil {
		return err
	}
	select {
	case m.idleCh <- c:
		return nil
	default:
		m.removeConnLocked(c)
		_ = c.Close()
		return fmt.Errorf("idle channel full (poolMaxHard=%d)", m.poolMaxHard)
	}
}

func (m *Manager) createConnLocked(ctx context.Context) (*ManagedConnection, error) {
	sc, err := m.db.Conn(ctx)
	if err != nil {
		return nil, err
	}
	mc := &ManagedConnection{
		googlesqliteConnection: sc,
		stmts:                  make(map[string]*sql.Stmt),
		manager:                m,
	}
	m.allConns = append(m.allConns, mc)
	m.live++
	if len(m.preparedQueryStrings) > 0 {
		if err := m.prepareQueriesOnConnLocked(mc, m.preparedQueryStrings); err != nil {
			m.removeConnLocked(mc)
			_ = mc.Close()
			return nil, err
		}
	}
	return mc, nil
}

func (m *Manager) removeConnLocked(conn *ManagedConnection) {
	for i, c := range m.allConns {
		if c == conn {
			m.allConns = append(m.allConns[:i], m.allConns[i+1:]...)
			m.live--
			return
		}
	}
}

// PoolMaxHard returns the configured hard ceiling for live connections.
func (m *Manager) PoolMaxHard() int {
	return m.poolMaxHard
}

// PoolMin returns the configured minimum warm connections.
func (m *Manager) PoolMin() int {
	return m.poolMin
}

// CurrentMax returns the soft cap (may be lowered by autoscale).
func (m *Manager) CurrentMax() int {
	return int(m.currentMax.Load())
}

// SetCurrentMax sets the soft cap for live connections and evicts idle connections above the target.
func (m *Manager) SetCurrentMax(newMax int) {
	if newMax < m.poolMin {
		newMax = m.poolMin
	}
	if newMax > m.poolMaxHard {
		newMax = m.poolMaxHard
	}
	m.currentMax.Store(int32(newMax))
	m.evictIdleConnectionsToTarget(newMax)
}

func (m *Manager) evictIdleConnectionsToTarget(targetLive int) {
	for {
		m.mu.Lock()
		if m.live <= targetLive {
			m.mu.Unlock()
			return
		}
		m.mu.Unlock()
		select {
		case c := <-m.idleCh:
			m.mu.Lock()
			m.removeConnLocked(c)
			m.mu.Unlock()
			_ = c.Close()
		default:
			return
		}
	}
}

// Close gracefully shuts down the repository.
func (m *Manager) Close() (err error) {
	m.closeOnce.Do(func() {
		m.mu.Lock()
		m.closed = true
		conns := append([]*ManagedConnection(nil), m.allConns...)
		m.allConns = nil
		m.mu.Unlock()

		for _, conn := range conns {
			if closeErr := conn.Close(); closeErr != nil && err == nil {
				err = closeErr
			}
		}
	})
	return
}

type Tx struct {
	tx        *sql.Tx
	conn      *ManagedConnection
	manager   *Manager
	committed bool
	finalized bool
}

func (t *Tx) Tx() *sql.Tx {
	return t.tx
}

// unregister removes this transaction from the manager's tracking map.
// It is idempotent and safe to call multiple times.
func (t *Tx) unregister() {
	if t.finalized || t.manager == nil {
		return
	}
	t.manager.txMu.Lock()
	delete(t.manager.txConnMap, t.tx)
	t.manager.txMu.Unlock()
	t.finalized = true
}

func (t *Tx) Conn() *ManagedConnection { return t.conn }

func (t *Tx) RollbackIfNotCommitted() error {
	defer t.unregister()
	if t.committed {
		return nil
	}
	return t.tx.Rollback()
}

func (t *Tx) Commit() error {
	defer t.unregister()
	if err := t.tx.Commit(); err != nil {
		return err
	}
	t.committed = true
	return nil
}

func (t *Tx) SetProjectAndDataset(projectID, datasetID string) {
	t.conn.ProjectID = projectID
	t.conn.DatasetID = datasetID
}

func (t *Tx) MetadataRepoMode() error {
	if err := t.conn.googlesqliteConnection.Raw(func(c interface{}) error {
		googlesqliteConn, ok := c.(*googlesqlite.GoogleSQLiteConn)
		if !ok {
			return fmt.Errorf("failed to get GoogleSQLiteConn from %T", c)
		}
		_ = googlesqliteConn.SetNamePath([]string{})
		return nil
	}); err != nil {
		return fmt.Errorf("failed to setup connection: %w", err)
	}
	return nil
}

func (t *Tx) ContentRepoMode() error {
	if err := t.conn.googlesqliteConnection.Raw(func(c interface{}) error {
		googlesqliteConn, ok := c.(*googlesqlite.GoogleSQLiteConn)
		if !ok {
			return fmt.Errorf("failed to get GoogleSQLiteConn from %T", c)
		}
		if t.conn.DatasetID == "" {
			_ = googlesqliteConn.SetNamePath([]string{t.conn.ProjectID})
		} else {
			_ = googlesqliteConn.SetNamePath([]string{t.conn.ProjectID, t.conn.DatasetID})
		}
		const maxNamePath = 3 // projectID and datasetID and tableID
		googlesqliteConn.SetMaxNamePath(maxNamePath)
		return nil
	}); err != nil {
		return fmt.Errorf("failed to setup connection: %w", err)
	}
	return nil
}

type ManagedConnection struct {
	googlesqliteConnection *sql.Conn
	stmts                  map[string]*sql.Stmt
	queries                []string
	mu                     sync.RWMutex
	manager                *Manager // immutable after construction, safe for concurrent reads
	ProjectID              string
	DatasetID              string
}

func (c *ManagedConnection) GetStmt(name string) (*sql.Stmt, error) {
	c.mu.RLock()
	defer c.mu.RUnlock()

	stmt, ok := c.stmts[name]
	if !ok {
		return nil, fmt.Errorf("statement not found: %s", name)
	}
	return stmt, nil
}

func (c *ManagedConnection) Close() (err error) {
	c.mu.Lock()
	defer c.mu.Unlock()

	for _, stmt := range c.stmts {
		if closeErr := stmt.Close(); closeErr != nil && err == nil {
			err = closeErr
		}
	}
	if closeErr := c.googlesqliteConnection.Close(); closeErr != nil && err == nil {
		err = closeErr
	}
	return
}

func (c *ManagedConnection) ConfigureScope(projectID, datasetID string) *ManagedConnection {
	c.ProjectID = projectID
	c.DatasetID = datasetID
	return c
}

// Raw executes the given function with the underlying ite connection.
func (c *ManagedConnection) Raw(fn func(interface{}) error) error {
	return c.googlesqliteConnection.Raw(fn)
}

func (c *ManagedConnection) Begin(ctx context.Context) (*Tx, error) {
	tx, err := c.googlesqliteConnection.BeginTx(ctx, nil)
	if err != nil {
		return nil, err
	}

	wrappedTx := &Tx{tx: tx, conn: c, manager: c.manager}

	// Register transaction with manager if available
	if c.manager != nil {
		c.manager.txMu.Lock()
		c.manager.txConnMap[tx] = c
		c.manager.txMu.Unlock()
	}

	return wrappedTx, nil
}

// GetStatement retrieves a prepared statement for use within a transaction
func (m *Manager) GetStatement(ctx context.Context, tx *sql.Tx, name string) (*sql.Stmt, error) {
	if tx == nil {
		return nil, fmt.Errorf("transaction is nil")
	}

	m.txMu.RLock()
	conn, ok := m.txConnMap[tx]
	m.txMu.RUnlock()

	if ok {
		// Transaction is tracked in txConnMap, use pre-prepared statement
		return conn.GetStmt(name)
	}

	// Transaction was created externally, prepare statement on the fly (this ideally never happens)
	ctx = googlesqlite.WithQueryFormattingDisabled(ctx)
	return tx.PrepareContext(ctx, name)
}

func (m *Manager) prepareQueriesOnConnLocked(conn *ManagedConnection, queries []string) error {
	ctx := googlesqlite.WithQueryFormattingDisabled(context.Background())
	for _, query := range queries {
		stmt, err := conn.googlesqliteConnection.PrepareContext(ctx, query)
		if err != nil {
			return fmt.Errorf("failed to prepare statement %s: %w", query, err)
		}
		conn.stmts[query] = stmt
	}
	return nil
}

func (m *Manager) PrepareQueries(queries []string) error {
	m.mu.Lock()
	m.preparedQueryStrings = append([]string(nil), queries...)
	conns := append([]*ManagedConnection(nil), m.allConns...)
	m.mu.Unlock()

	for _, conn := range conns {
		if err := m.prepareQueriesOnConnLocked(conn, queries); err != nil {
			return err
		}
	}
	return nil
}

// releaseConnAfterUse returns a pooled connection after a request. If the underlying
// *sql.Conn was closed (e.g. because the HTTP request context was canceled while a
// query was running), database/sql leaves it unusable; we replace the sqlite Conn and
// re-prepare statements so the pool does not hand out dead connections.
func (m *Manager) releaseConnAfterUse(conn *ManagedConnection) {
	if conn == nil {
		return
	}
	pingCtx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	if err := conn.googlesqliteConnection.PingContext(pingCtx); err != nil {
		if rebErr := m.rebindManagedConnection(conn); rebErr != nil {
			slog.Error("failed to rebind dead pooled connection",
				slog.Any("rebind_err", rebErr),
				slog.Any("ping_err", err),
			)
		}
	}

	m.mu.Lock()
	if m.closed {
		m.removeConnLocked(conn)
		m.mu.Unlock()
		_ = conn.Close()
		return
	}
	if m.live > int(m.currentMax.Load()) {
		m.removeConnLocked(conn)
		m.mu.Unlock()
		_ = conn.Close()
		return
	}
	m.mu.Unlock()

	select {
	case m.idleCh <- conn:
	default:
		m.mu.Lock()
		m.removeConnLocked(conn)
		m.mu.Unlock()
		_ = conn.Close()
	}
}

func (m *Manager) rebindManagedConnection(mc *ManagedConnection) error {
	m.mu.Lock()
	queries := m.preparedQueryStrings
	m.mu.Unlock()

	mc.mu.Lock()
	defer mc.mu.Unlock()

	for _, stmt := range mc.stmts {
		_ = stmt.Close()
	}
	mc.stmts = make(map[string]*sql.Stmt)
	if mc.googlesqliteConnection != nil {
		_ = mc.googlesqliteConnection.Close()
	}
	newConn, err := m.db.Conn(context.Background())
	if err != nil {
		return fmt.Errorf("db.Conn: %w", err)
	}
	mc.googlesqliteConnection = newConn
	mc.ProjectID = ""
	mc.DatasetID = ""

	return m.prepareQueriesOnConnLocked(mc, queries)
}

func (m *Manager) acquireConn(ctx context.Context) (*ManagedConnection, error) {
	waitStart := time.Now()
	for {
		m.mu.Lock()
		closed := m.closed
		m.mu.Unlock()
		if closed {
			return nil, fmt.Errorf("repository is closed")
		}

		select {
		case c := <-m.idleCh:
			if connMetricsEnabled() {
				recordConnAcquireSuccess(time.Since(waitStart))
			}
			return c, nil
		default:
		}

		m.mu.Lock()
		if m.closed {
			m.mu.Unlock()
			return nil, fmt.Errorf("repository is closed")
		}
		if m.live < int(m.currentMax.Load()) {
			c, err := m.createConnLocked(ctx)
			if err != nil {
				m.mu.Unlock()
				return nil, err
			}
			m.mu.Unlock()
			if connMetricsEnabled() {
				recordConnAcquireSuccess(time.Since(waitStart))
			}
			return c, nil
		}
		m.mu.Unlock()

		select {
		case c := <-m.idleCh:
			if connMetricsEnabled() {
				recordConnAcquireSuccess(time.Since(waitStart))
			}
			return c, nil
		case <-ctx.Done():
			if connMetricsEnabled() {
				recordConnAcquireCanceled()
			}
			return nil, fmt.Errorf("context cancelled: %w", ctx.Err())
		}
	}
}

func (m *Manager) WithManagedConnection(ctx context.Context, fn func(ctx context.Context, conn *ManagedConnection) error) error {
	conn, err := m.acquireConn(ctx)
	if err != nil {
		return err
	}
	defer m.releaseConnAfterUse(conn)

	if !connMetricsEnabled() {
		return fn(ctx, conn)
	}
	t0 := time.Now()
	err = fn(ctx, conn)
	recordConnFnHold(time.Since(t0))
	return err
}

func (m *Manager) ExecuteWithTransaction(ctx context.Context, fn func(ctx context.Context, tx *Tx) error) error {
	return m.WithManagedConnection(ctx, func(ctx context.Context, conn *ManagedConnection) error {
		tx, err := conn.Begin(ctx)
		if err != nil {
			return fmt.Errorf("failed to begin transaction: %w", err)
		}
		// Transaction is automatically registered in Begin() and will be unregistered in Commit()/RollbackIfNotCommitted()

		// Execute the user's function with the transaction
		err = fn(ctx, tx)
		if err != nil {
			if rbErr := tx.RollbackIfNotCommitted(); rbErr != nil {
				return fmt.Errorf("transaction failed: %w (rollback also failed: %v)", err, rbErr)
			}
			return err
		}

		if err = tx.Commit(); err != nil {
			return fmt.Errorf("failed to commit transaction: %w", err)
		}

		return nil
	})
}

func WithManagedConnection[T any](m *Manager, ctx context.Context,
	fn func(context.Context, *ManagedConnection) (T, error)) (T, error) {

	var zero T

	conn, err := m.acquireConn(ctx)
	if err != nil {
		return zero, err
	}
	defer m.releaseConnAfterUse(conn)

	if !connMetricsEnabled() {
		return fn(ctx, conn)
	}
	t0 := time.Now()
	v, err := fn(ctx, conn)
	recordConnFnHold(time.Since(t0))
	return v, err
}

// ExecuteWithTransaction is a convenience wrapper for executing a function with a transaction
// This is useful for migrating code that used getConnection + BeginTx pattern
func ExecuteWithTransaction[T any](m *Manager, ctx context.Context, fn func(context.Context, *sql.Tx) (T, error)) (T, error) {
	return WithManagedConnection[T](m, ctx, func(ctx context.Context, conn *ManagedConnection) (T, error) {
		var zero T
		tx, err := conn.Begin(ctx)
		if err != nil {
			return zero, fmt.Errorf("failed to begin transaction: %w", err)
		}

		// Execute the user's function with the transaction
		result, err := fn(ctx, tx.Tx())
		if err != nil {
			if rbErr := tx.RollbackIfNotCommitted(); rbErr != nil {
				return zero, fmt.Errorf("transaction failed: %w (rollback also failed: %v)", err, rbErr)
			}
			return zero, err
		}

		if err = tx.Commit(); err != nil {
			return zero, fmt.Errorf("failed to commit transaction: %w", err)
		}

		return result, nil
	})
}
