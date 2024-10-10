#include "rudp-protocol.h"
#include "storage-repository.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

const int16_t SERVER_PORT = 8080;

// Define a structure to hold buffered packets
typedef struct packet_buffer {
    rudp_packet_t packet;
    struct packet_buffer *next;
} packet_buffer_t;

packet_buffer_t *head = NULL; // Head of the packet buffer linked list
uint32_t expected_sequence_num = 0; // The next expected sequence number

// Function to buffer out-of-order packets
void buffer_packet(rudp_packet_t *packet) {
    packet_buffer_t *new_node = (packet_buffer_t *)malloc(sizeof(packet_buffer_t));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    new_node->packet = *packet;
    new_node->next = NULL;

    // Insert the packet into the linked list (in ascending order of sequence number)
    if (head == NULL || head->packet.sequence_num > packet->sequence_num) {
        new_node->next = head;
        head = new_node;
    } else {
        packet_buffer_t *current = head;
        while (current->next != NULL && current->next->packet.sequence_num < packet->sequence_num) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}

// Function to process and remove buffered packets if they are in sequence
void process_buffered_packets() {
    while (head != NULL && head->packet.sequence_num == expected_sequence_num) {
        // Process the packet
        rudp_packet_t *packet = &head->packet;
        struct timeval timestamp_received;
        gettimeofday(&timestamp_received, NULL);

        printf("Processing buffered packet with sequence number %d: %s\n", packet->sequence_num, packet->data);

        // Store packet in ScyllaDB
        if (repository_save_packet(packet->sequence_num, packet->timestamp_sent, timestamp_received, packet->retry_count > 0, packet->retry_count) != 0) {
            fprintf(stderr, "Failed to store packet information.\n");
        }

        // Remove the processed packet from the buffer
        packet_buffer_t *temp = head;
        head = head->next;
        free(temp);

        expected_sequence_num++; // Move to the next expected sequence number
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    rudp_packet_t packet;
    struct timeval timestamp_received;

    // Initialize RUDP protocol and storage
    if (storage_init() != 0) {
        fprintf(stderr, "Failed to initialize storage\n");
        return EXIT_FAILURE;
    }

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to server address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is ready and listening on port %d...\n", SERVER_PORT);

    while (1) {
        // Receive packet
        socklen_t addr_len = sizeof(client_addr);
        if (rudp_recv(sockfd, &packet, &client_addr) > 0) {
            gettimeofday(&timestamp_received, NULL);
            printf("Received packet with sequence number %d: %s\n", packet.sequence_num, packet.data);

            // Check if the packet is the expected one
            if (packet.sequence_num == expected_sequence_num) {
                // Process the packet immediately
                printf("Processing packet with sequence number %d\n", packet.sequence_num);

                // Store packet in ScyllaDB
                if (repository_save_packet(packet.sequence_num, packet.timestamp_sent, timestamp_received, packet.retry_count > 0, packet.retry_count) != 0) {
                    fprintf(stderr, "Failed to store packet information.\n");
                }

                expected_sequence_num++; // Move to the next expected sequence number

                // Check if there are buffered packets that can be processed
                process_buffered_packets();
            } else if (packet.sequence_num > expected_sequence_num) {
                // Packet arrived out of order, buffer it
                printf("Buffering out-of-order packet with sequence number %d\n", packet.sequence_num);
                buffer_packet(&packet);
            } else {
                // Duplicate or old packet, ignore it
                printf("Ignoring duplicate or old packet with sequence number %d\n", packet.sequence_num);
            }

            // Send ACK
            rudp_packet_t ack_packet = {
                .sequence_num = packet.sequence_num,
                .ack = true,
                .data_length = 0,
                .retry_count = 0
            };
            rudp_send(sockfd, &ack_packet, &client_addr);
            printf("ACK sent for packet %d\n", packet.sequence_num);
        }
    }

    // Close storage and socket
    storage_close();
    close(sockfd);
    return 0;
}
