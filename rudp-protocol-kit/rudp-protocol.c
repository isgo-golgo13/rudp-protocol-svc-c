#include "rudp-protocol.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

// Send a packet
int rudp_send(int sockfd, const rudp_packet_t *packet, const struct sockaddr_in *dest_addr) {
    return sendto(sockfd, packet, sizeof(rudp_packet_t), 0, (const struct sockaddr *)dest_addr, sizeof(struct sockaddr_in));
}

// Receive a packet
int rudp_recv(int sockfd, rudp_packet_t *packet, struct sockaddr_in *src_addr) {
    socklen_t addr_len = sizeof(struct sockaddr_in);
    return recvfrom(sockfd, packet, sizeof(rudp_packet_t), 0, (struct sockaddr *)src_addr, &addr_len);
}

// Retry sending packet until ACK is received or retries are exhausted
int rudp_retry_send(int sockfd, const rudp_packet_t *packet, const struct sockaddr_in *dest_addr) {
    int retries = 0;
    while (retries < packet->retry_count) {
        int sent = rudp_send(sockfd, packet, dest_addr);
        if (sent == -1) {
            perror("Send failed");
            return -1;
        }

        rudp_packet_t ack_packet;
        struct sockaddr_in src_addr;
        struct timeval timeout;
        fd_set read_fds;

        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        timeout.tv_sec = 2;  // Timeout after 2 seconds
        timeout.tv_usec = 0;

        int rv = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);

        if (rv == -1) {
            perror("Select error");
            return -1;
        } else if (rv == 0) {
            retries++;
            printf("Timeout, retrying... (%d/%d)\n", retries, packet->retry_count);
        } else {
            if (FD_ISSET(sockfd, &read_fds)) {
                if (rudp_recv(sockfd, &ack_packet, &src_addr) > 0 && ack_packet.ack && ack_packet.sequence_num == packet->sequence_num) {
                    printf("Packet %d ACK received\n", packet->sequence_num);
                    return 0;
                }
            }
        }
    }
    printf("Max retries reached. Packet %d failed.\n", packet->sequence_num);
    return -1;
}
