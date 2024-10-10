#ifndef STORAGE_REPOSITORY_H
#define STORAGE_REPOSITORY_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>

// Function to store packet information via the repository
int repository_save_packet(uint32_t packet_id, struct timeval timestamp_sent, struct timeval timestamp_received,
                           bool retried, uint32_t retry_count);

#endif // STORAGE_REPOSITORY_H
