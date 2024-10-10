#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <time.h>
#include <uuid/uuid.h>

// Function to initialize the connection to ScyllaDB
int storage_init();

// Function to store packet data in ScyllaDB
int storage_save_packet(uuid_t primary_key, int packet_id, struct timespec timestamp_sent,
                        struct timespec timestamp_received, bool retried, int retry_count);

// Function to close the ScyllaDB connection
void storage_close();

#endif // STORAGE_H
