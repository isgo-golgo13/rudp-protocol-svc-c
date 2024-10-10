#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <uuid/uuid.h>

// Function to initialize the connection to ScyllaDB
int storage_init();

// Function to store packet data in ScyllaDB
int storage_save_packet(uuid_t primary_key, uint32_t packet_id, struct timeval timestamp_sent,
                        struct timeval timestamp_received, bool retried, uint32_t retry_count);

// Function to close the ScyllaDB connection
void storage_close();

#endif // STORAGE_H
