#ifndef RUDP_PROTOCOL_H
#define RUDP_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <stddef.h>


const int16_t RUDP_MAX_DATA_SIZE = 512;

// RUDP packet structure
typedef struct rudp_packet {
    uint32_t sequence_num;   // Sequence number for ordering
    bool ack;                // ACK flag to acknowledge packet receipt
    uint32_t retry_count;    // Number of retries
    struct timeval timestamp_sent; // Timestamp when the packet was sent
    size_t data_length;      // Length of the data payload
    uint8_t data[RUDP_MAX_DATA_SIZE]; // Data payload
} rudp_packet_t;

// Function to send a packet
int rudp_send(int sockfd, const rudp_packet_t *packet, const struct sockaddr_in *dest_addr);

// Function to receive a packet
int rudp_recv(int sockfd, rudp_packet_t *packet, struct sockaddr_in *src_addr);

// Function to handle retries
int rudp_retry_send(int sockfd, const rudp_packet_t *packet, const struct sockaddr_in *dest_addr);

#endif // RUDP_PROTOCOL_H
