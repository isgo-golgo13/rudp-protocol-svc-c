#include "storage-repository.h"
#include "storage.h"
#include <uuid/uuid.h>
#include <sys/time.h>
#include <stdio.h>

// Repository function to save packet info
int repository_save_packet(uint32_t packet_id, struct timeval timestamp_sent, struct timeval timestamp_received,
                           bool retried, uint32_t retry_count) {
    uuid_t primary_key;
    uuid_generate(primary_key);

    // Call storage API to persist data
    return storage_save_packet(primary_key, packet_id, timestamp_sent, timestamp_received, retried, retry_count);
}
