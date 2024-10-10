#include "rudp-protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

// Server port
const int16_t SERVER_PORT = 8080;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    uint32_t sequence_num = 0;
    rudp_packet_t packet;
    struct timeval timestamp_sent;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        // Get current time as timestamp_sent
        gettimeofday(&timestamp_sent, NULL);
        snprintf((char *)packet.data, sizeof(packet.data), "%ld.%06ld", timestamp_sent.tv_sec, timestamp_sent.tv_usec);
        packet.sequence_num = sequence_num++;
        packet.data_length = strlen((char *)packet.data);
        packet.retry_count = 0;
        packet.timestamp_sent = timestamp_sent;

        // Send message using RUDP protocol
        if (rudp_send(sockfd, &packet, &server_addr) < 0) {
            perror("Send failed");
        }

        printf("Sent packet with sequence number %d\n", packet.sequence_num);

        // Wait 5 seconds before sending the next packet
        sleep(5);
    }

    close(sockfd);
    return 0;
}
