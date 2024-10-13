#include "storage.h"
#include <stdio.h>
#include <cassandra.h>
#include <uuid/uuid.h>
#include <stdlib.h>
#include <string.h>

static CassCluster* cluster;
static CassSession* session;

// Helper function to load the schema from the .cql file
int load_schema(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open schema file: %s\n", file_path);
        return -1;
    }

    // Get file size and allocate memory for schema script
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* schema_script = (char*)malloc(file_size + 1);
    if (schema_script == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return -1;
    }

    // Read the file into the schema_script buffer
    fread(schema_script, 1, file_size, file);
    schema_script[file_size] = '\0';  // Null-terminate the string
    fclose(file);

    // Execute the schema script using the ScyllaDB session
    CassStatement* statement = cass_statement_new(schema_script, 0);
    CassFuture* result_future = cass_session_execute(session, statement);
    CassError rc = cass_future_error_code(result_future);

    if (rc != CASS_OK) {
        fprintf(stderr, "Failed to execute schema: %s\n", cass_error_desc(rc));
        free(schema_script);
        cass_statement_free(statement);
        cass_future_free(result_future);
        return -1;
    }

    printf("Successfully loaded and executed schema from %s\n", file_path);
    
    free(schema_script);
    cass_statement_free(statement);
    cass_future_free(result_future);

    return 0;
}

// Initialize ScyllaDB connection and load schema
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

    // Load schema from file
    if (load_schema("schema/schema.cql") != 0) {
        fprintf(stderr, "Schema loading failed\n");
        return -1;
    }

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
