//go:build integration

package e2e

import (
	"database/sql"
	"testing"
)

// Regression tests for SQL patterns reported by vibecart-backend against
// an earlier emulator build:
//
//   - merge staging tables with SELECT * EXCEPT(rn) + ROW_NUMBER() dedup
//   - segment profile counts with correlated subqueries, table aliases,
//     and NOT IN UNNEST(@p0) array parameters
//   - INSERT ... SELECT ... QUALIFY ROW_NUMBER() dedup
//   - campaign summary INSERT with COUNT(DISTINCT IF(...)) GROUP BY
//
// SELECT-shaped subtests cover the same SQL the production jobs emit where
// the DuckDB fast path is available today. Exact DML/CTAS subtests mirror
// the customer statements and guard the control_op / semantic DML routes.

func TestVibecartRegressionQueries(t *testing.T) {
	db, ctx := openQueryPortTestDB(t)
	defer db.Close()
	conn, err := db.Conn(ctx)
	if err != nil {
		t.Fatalf("Conn: %v", err)
	}
	defer conn.Close()

	mustExec := func(t *testing.T, q string, args ...any) {
		t.Helper()
		if _, err := conn.ExecContext(ctx, q, args...); err != nil {
			t.Fatalf("exec %q: %v", q, err)
		}
	}
	mustQuery := func(t *testing.T, q string, args ...any) *emulatorRows {
		t.Helper()
		rows, err := conn.QueryContext(ctx, q, args...)
		if err != nil {
			t.Fatalf("query %q: %v", q, err)
		}
		return rows
	}

	t.Run("merge_dedup_select_except_row_number", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.merge_source (
			id STRING,
			tie_break INT64,
			value INT64
		)`)
		mustExec(t, `INSERT INTO ds.merge_source (id, tie_break, value) VALUES
			('a', 1, 10),
			('a', 2, 20),
			('b', 1, 30)`)

		rows := mustQuery(t, `
			SELECT * EXCEPT(rn) FROM (
				SELECT *,
					ROW_NUMBER() OVER (
						PARTITION BY id ORDER BY tie_break DESC
					) AS rn
				FROM ds.merge_source
			)
			WHERE rn = 1
			ORDER BY id`)
		got := rowDump(t, rows)
		if len(got) != 2 {
			t.Fatalf("row count = %d; want 2", len(got))
		}
		if val, _ := got[0][2].(int64); val != 20 {
			t.Fatalf("deduped value for a = %v; want 20", got[0][2])
		}
	})

	t.Run("merge_dedup_create_or_replace_table", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.merge_source_cr (
			id STRING,
			tie_break INT64,
			value INT64
		)`)
		mustExec(t, `INSERT INTO ds.merge_source_cr (id, tie_break, value) VALUES
			('a', 1, 10),
			('a', 2, 20),
			('b', 1, 30)`)

		mustExec(t, `CREATE OR REPLACE TABLE ds.merge_deduped_cr AS
			SELECT * EXCEPT(rn) FROM (
				SELECT *,
					ROW_NUMBER() OVER (
						PARTITION BY id ORDER BY tie_break DESC
					) AS rn
				FROM ds.merge_source_cr
			)
			WHERE rn = 1`)

		rows := mustQuery(t, `SELECT id, value FROM ds.merge_deduped_cr ORDER BY id`)
		got := rowDump(t, rows)
		if len(got) != 2 {
			t.Fatalf("row count = %d; want 2", len(got))
		}
		if val, _ := got[0][1].(int64); val != 20 {
			t.Fatalf("deduped value for a = %v; want 20", got[0][1])
		}
	})

	t.Run("segment_profile_count_correlated_subquery", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.profiles (
			id STRING,
			organization_id STRING,
			is_deleted BOOL
		)`)
		mustExec(t, `CREATE TABLE ds.orders (
			id STRING,
			profile_id STRING,
			is_deleted BOOL,
			financial_status STRING
		)`)
		mustExec(t, `INSERT INTO ds.profiles (id, organization_id, is_deleted) VALUES
			('p1', '2', FALSE),
			('p2', '2', FALSE),
			('p3', '2', TRUE),
			('p4', '1', FALSE)`)
		mustExec(t, `INSERT INTO ds.orders (id, profile_id, is_deleted, financial_status) VALUES
			('o1', 'p1', FALSE, 'paid'),
			('o2', 'p1', FALSE, 'voided'),
			('o3', 'p2', FALSE, 'refunded')`)

		baseSQL := `
			SELECT DISTINCT id
			FROM ds.profiles profiles
			WHERE organization_id = '2'
			AND COALESCE(is_deleted, FALSE) = FALSE
			AND ((
				SELECT COUNT(*)
				FROM ds.orders o
				WHERE o.profile_id = profiles.id
				AND COALESCE(o.is_deleted, FALSE) = FALSE
				AND o.financial_status NOT IN UNNEST(@p0)
			) >= @p1)`

		rows := mustQuery(t, `
			WITH profiles_cte AS (`+baseSQL+`)
			SELECT COUNT(pcte.id) AS profile_count
			FROM profiles_cte pcte`,
			sql.Named("p0", []string{"voided", "refunded"}),
			sql.Named("p1", int64(1)),
		)
		defer rows.Close()
		if !rows.Next() {
			t.Fatal("expected one row")
		}
		var count int64
		if err := rows.Scan(&count); err != nil {
			t.Fatalf("Scan: %v", err)
		}
		if count != 1 {
			t.Fatalf("profile_count = %d; want 1 (only p1 has a qualifying order)", count)
		}
	})

	t.Run("insert_select_qualify_row_number", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.product_staging_q (
			id STRING,
			seq INT64,
			payload STRING
		)`)
		mustExec(t, `INSERT INTO ds.product_staging_q (id, seq, payload) VALUES
			('x', 1, 'old'),
			('x', 2, 'new'),
			('y', 1, 'only')`)

		rows := mustQuery(t, `
			SELECT id, payload
			FROM ds.product_staging_q
			QUALIFY ROW_NUMBER() OVER (
				PARTITION BY id
				ORDER BY seq DESC
			) = 1
			ORDER BY id`)
		got := rowDump(t, rows)
		if len(got) != 2 {
			t.Fatalf("row count = %d; want 2", len(got))
		}
		if payload, _ := got[0][1].(string); payload != "new" {
			t.Fatalf("deduped payload for x = %v; want new", got[0][1])
		}
	})

	t.Run("insert_into_qualify_row_number", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.product_main_q (
			id STRING,
			seq INT64,
			payload STRING
		)`)
		mustExec(t, `CREATE TABLE ds.product_staging_ins (
			id STRING,
			seq INT64,
			payload STRING
		)`)
		mustExec(t, `INSERT INTO ds.product_staging_ins (id, seq, payload) VALUES
			('x', 1, 'old'),
			('x', 2, 'new'),
			('y', 1, 'only')`)

		mustExec(t, `
			INSERT INTO ds.product_main_q
			SELECT *
			FROM ds.product_staging_ins
			QUALIFY ROW_NUMBER() OVER (
				PARTITION BY id
				ORDER BY seq DESC
			) = 1`)

		rows := mustQuery(t, `SELECT id, payload FROM ds.product_main_q ORDER BY id`)
		got := rowDump(t, rows)
		if len(got) != 2 {
			t.Fatalf("row count = %d; want 2", len(got))
		}
		if payload, _ := got[0][1].(string); payload != "new" {
			t.Fatalf("deduped payload for x = %v; want new", got[0][1])
		}
	})

	t.Run("campaign_summary_performance_stats", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.campaign_events (
			organization_id STRING,
			campaign_id STRING,
			profile_public_id STRING,
			event_type STRING,
			channel STRING,
			cost FLOAT64
		)`)
		mustExec(t, `INSERT INTO ds.campaign_events VALUES
			('org1', 'c1', 'pub1', 'targeted', 'viber', 0.1),
			('org1', 'c1', 'pub1', 'sent', 'viber', 0.2),
			('org1', 'c1', 'pub1', 'delivered', 'viber', 0.3),
			('org1', 'c1', 'pub1', 'opened', 'viber', NULL),
			('org1', 'c1', 'pub1', 'clicked', 'viber', NULL)`)

		rows := mustQuery(t, `
			WITH CampaignEvents AS (
				SELECT organization_id, campaign_id, profile_public_id, event_type, channel, cost
				FROM ds.campaign_events
			),
			PerformanceStats AS (
				SELECT
					organization_id,
					campaign_id,
					channel,
					COUNT(DISTINCT IF(event_type = 'targeted', profile_public_id, NULL)) AS targeted,
					COUNT(DISTINCT IF(event_type = 'sent', profile_public_id, NULL)) AS sent,
					COUNTIF(event_type = 'delivered') AS delivered_total,
					COUNTIF(event_type = 'delivered' AND channel = 'viber') AS delivered_viber,
					COUNTIF(event_type = 'delivered' AND channel = 'sms') AS delivered_sms,
					COALESCE(SUM(cost), 0) AS total_cost,
					COUNT(DISTINCT IF(event_type = 'opened', profile_public_id, NULL)) AS unique_opens,
					COUNT(DISTINCT IF(event_type = 'clicked', profile_public_id, NULL)) AS unique_clicks,
					COUNT(DISTINCT IF(event_type = 'clicked' AND channel = 'viber', profile_public_id, NULL)) AS unique_clicks_viber,
					COUNT(DISTINCT IF(event_type = 'clicked' AND channel = 'sms', profile_public_id, NULL)) AS unique_clicks_sms
				FROM CampaignEvents
				GROUP BY organization_id, campaign_id, channel
			)
			SELECT targeted, delivered_total, unique_clicks
			FROM PerformanceStats`)
		got := rowDump(t, rows)
		if len(got) != 1 {
			t.Fatalf("row count = %d; want 1", len(got))
		}
		if targeted, _ := got[0][0].(int64); targeted != 1 {
			t.Fatalf("targeted = %v; want 1", got[0][0])
		}
		if delivered, _ := got[0][1].(int64); delivered != 1 {
			t.Fatalf("delivered_total = %v; want 1", got[0][1])
		}
	})

	t.Run("campaign_summary_full_pipeline", func(t *testing.T) {
		mustExec(t, `CREATE TABLE ds.campaign_events_full (
			organization_id STRING,
			campaign_id STRING,
			profile_public_id STRING,
			event_type STRING,
			channel STRING,
			occurred_at INT64,
			cost FLOAT64
		)`)
		mustExec(t, `CREATE TABLE ds.campaign_profiles_full (
			id STRING,
			public_id STRING
		)`)
		mustExec(t, `CREATE TABLE ds.campaign_orders_full (
			id STRING,
			profile_id STRING,
			total_price FLOAT64,
			total_refunded FLOAT64,
			source_created_at INT64,
			is_deleted BOOL,
			financial_status STRING
		)`)
		mustExec(t, `INSERT INTO ds.campaign_events_full VALUES
			('org1', 'c1', 'pub1', 'targeted', 'viber', 1, 0.1),
			('org1', 'c1', 'pub1', 'delivered', 'viber', 2, 0.3),
			('org1', 'c1', 'pub1', 'clicked', 'viber', 4, NULL)`)
		mustExec(t, `INSERT INTO ds.campaign_profiles_full VALUES ('prof1', 'pub1')`)
		mustExec(t, `INSERT INTO ds.campaign_orders_full VALUES
			('ord1', 'prof1', 100.0, 10.0, 5, FALSE, 'paid')`)

		rows := mustQuery(t, `
			WITH CampaignEvents AS (
				SELECT organization_id, campaign_id, profile_public_id, event_type, channel, occurred_at, cost
				FROM ds.campaign_events_full
			),
			PerformanceStats AS (
				SELECT
					organization_id,
					campaign_id,
					channel,
					COUNT(DISTINCT IF(event_type = 'targeted', profile_public_id, NULL)) AS targeted,
					COUNTIF(event_type = 'delivered') AS delivered_total
				FROM CampaignEvents
				GROUP BY organization_id, campaign_id, channel
			),
			ValidCampaignMessages AS (
				SELECT profile_public_id, campaign_id, channel, occurred_at AS sent_at
				FROM CampaignEvents
				WHERE event_type = 'delivered'
			),
			LatestOrders AS (
				SELECT * FROM ds.campaign_orders_full
			),
			AttributedOrdersRaw AS (
				SELECT
					msgs.campaign_id,
					msgs.channel,
					orders.id AS order_id,
					orders.total_price,
					COALESCE(orders.total_refunded, 0) AS refunded_amount,
					ROW_NUMBER() OVER (PARTITION BY orders.id ORDER BY msgs.sent_at DESC) AS attribution_rank
				FROM ValidCampaignMessages msgs
				JOIN ds.campaign_profiles_full profiles ON msgs.profile_public_id = profiles.public_id
				JOIN LatestOrders orders ON profiles.id = orders.profile_id
				WHERE orders.source_created_at > msgs.sent_at
				  AND orders.source_created_at <= msgs.sent_at + 7
				  AND COALESCE(orders.is_deleted, FALSE) = FALSE
				  AND orders.financial_status NOT IN ('voided', 'refunded')
			),
			RevenueStats AS (
				SELECT
					campaign_id,
					channel,
					COUNT(order_id) AS total_orders,
					COALESCE(SUM(total_price - refunded_amount), 0) AS net_revenue
				FROM AttributedOrdersRaw
				WHERE attribution_rank = 1
				GROUP BY campaign_id, channel
			)
			SELECT
				CONCAT(p.campaign_id, ':', p.channel) AS id,
				COALESCE(r.total_orders, 0) AS total_orders,
				COALESCE(r.net_revenue, 0) AS net_revenue
			FROM PerformanceStats p
			LEFT JOIN RevenueStats r
				ON p.campaign_id = r.campaign_id AND p.channel = r.channel`)

		got := rowDump(t, rows)
		if len(got) != 1 {
			t.Fatalf("row count = %d; want 1", len(got))
		}
		if orders, _ := got[0][1].(int64); orders != 1 {
			t.Fatalf("total_orders = %v; want 1", got[0][1])
		}
	})
}
