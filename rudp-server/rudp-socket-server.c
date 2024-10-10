#include "rudp-protocol.h"
#include "storage-repository.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 8080

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

            // Store packet in ScyllaDB
            if (repository_save_packet(packet.sequence_num, packet.timestamp_sent, timestamp_received, packet.retry_count > 0, packet.retry_count) != 0) {
                fprintf(stderr, "Failed to store packet information.\n");
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
