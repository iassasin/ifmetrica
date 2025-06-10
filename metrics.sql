-- create table in clickhouse

CREATE TABLE network_interfaces(
	time DateTime CODEC(Delta, ZSTD(1)),
	group_id LowCardinality(String) CODEC(ZSTD(1)),
	iface LowCardinality(String) CODEC(ZSTD(1)),

	rx_bytes UInt32 CODEC(ZSTD(1)),
	rx_packets UInt32 CODEC(ZSTD(1)),
	rx_errors UInt32 CODEC(ZSTD(1)),
	rx_drops UInt32 CODEC(ZSTD(1)),

	tx_bytes UInt32 CODEC(ZSTD(1)),
	tx_packets UInt32 CODEC(ZSTD(1)),
	tx_errors UInt32 CODEC(ZSTD(1)),
	tx_drops UInt32 CODEC(ZSTD(1))
)
ENGINE = MergeTree()
PARTITION BY toYYYYMM(time)
ORDER BY (group_id, iface, time)
TTL time + INTERVAL 2 YEAR DELETE;