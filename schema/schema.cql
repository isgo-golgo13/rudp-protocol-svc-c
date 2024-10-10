CREATE KEYSPACE IF NOT EXISTS rudp_keyspace
WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};

CREATE TABLE IF NOT EXISTS rudp_keyspace.packet_logs (
    primary_key_id UUID PRIMARY KEY,
    packet_id INT,
    timestamp_sent TIMESTAMP,
    timestamp_received TIMESTAMP,
    retried BOOLEAN,
    retried_count INT
);
