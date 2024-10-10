#include "storage.h"
#include <stdio.h>
#include <cassandra.h>
#include <uuid/uuid.h>

static CassCluster* cluster;
static CassSession* session;

// Initialize ScyllaDB connection
int storage_init() {
    cluster = cass_cluster_new();
    session = cass_session_new();

    // Add contact points (ScyllaDB node)
    cass_cluster_set_contact_points(cluster, "scylla-db");

    // Connect to ScyllaDB keyspace
    CassFuture* connect_future = cass_session_connect_keyspace(session, cluster, "rudp_keyspace");
    if (cass_future_error_code(connect_future) != CASS_OK) {
        fprintf(stderr, "Unable to connect to ScyllaDB\n");
        cass_future_free(connect_future);
        return -1;
    }

    cass_future_free(connect_future);
    return 0;
}

// Save packet information to ScyllaDB
int storage_save_packet(uuid_t primary_key, int packet_id, struct timespec timestamp_sent,
                        struct timespec timestamp_received, bool retried, int retry_count) {
    const char* query = "INSERT INTO packet_logs (primary_key_id, packet_id, timestamp_sent, "
                        "timestamp_received, retried, retried_count) VALUES (?, ?, ?, ?, ?, ?);";

    CassStatement* statement = cass_statement_new(query, 6);

    // Bind values
    cass_statement_bind_uuid(statement, 0, primary_key);
    cass_statement_bind_int32(statement, 1, packet_id);
    cass_statement_bind_int64(statement, 2, timestamp_sent.tv_sec * 1000 + timestamp_sent.tv_nsec / 1000000);
    cass_statement_bind_int64(statement, 3, timestamp_received.tv_sec * 1000 + timestamp_received.tv_nsec / 1000000);
    cass_statement_bind_bool(statement, 4, retried);
    cass_statement_bind_int32(statement, 5, retry_count);

    CassFuture* result_future = cass_session_execute(session, statement);
    CassError rc = cass_future_error_code(result_future);

    if (rc != CASS_OK) {
        fprintf(stderr, "Failed to insert packet data: %s\n", cass_error_desc(rc));
        cass_future_free(result_future);
        return -1;
    }

    cass_future_free(result_future);
    cass_statement_free(statement);
    return 0;
}

// Close the ScyllaDB connection
void storage_close() {
    CassFuture* close_future = cass_session_close(session);
    cass_future_wait(close_future);
    cass_future_free(close_future);
    cass_cluster_free(cluster);
    cass_session_free(session);
}
