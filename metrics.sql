-- create table in clickhouse

CREATE TABLE network_interfaces(
	time DateTime,
	group_id LowCardinality(String),
	iface LowCardinality(String),

	rx_bytes UInt32,
	rx_packets UInt32,
	rx_errors UInt32,
	rx_drops UInt32,

	tx_bytes UInt32,
	tx_packets UInt32,
	tx_errors UInt32,
	tx_drops UInt32
)
ENGINE = MergeTree()
PARTITION BY toYYYYMM(time)
ORDER BY (group_id, iface, time)
TTL time + INTERVAL 2 YEAR DELETE;