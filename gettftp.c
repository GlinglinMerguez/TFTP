#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>

#define ARGUMENTS_ERROR "Error, you must specify 3 arguments."
#define SERVER_ADDR_ERROR "Error trying to get the server address."
#define SOCKET_ERROR "Socket creation error"
#define SEND_ERROR "Error sending data"
#define MAX_RRQ_SIZE 512
#define RRQ_MODE "octet"

void main(int argc, char** argv) {

    // Argument count test
    if (argc != 4) {
        printf(ARGUMENTS_ERROR);
    } else {

        // Get the server address
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof hints);  // Hints structure
        hints.ai_family = AF_INET;                 // IPv4
        hints.ai_socktype = SOCK_DGRAM;            // UDP socket

        // Try to get the server address
        if (getaddrinfo(argv[1], NULL, &hints, &res) !=0) {
            perror(SERVER_ADDR_ERROR);
            exit(EXIT_FAILURE);
        }

        // Socket Reservation
        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            perror(SOCKET_ERROR);
            exit(EXIT_FAILURE);
        }

        // Build RRQ packet
        char rrq_buffer[MAX_RRQ_SIZE];
        rrq_buffer[0] = 0x00;
        rrq_buffer[1] = 0x01;
        strcpy(rrq_buffer + 2, argv[2]);
        strcpy(rrq_buffer + 2 + strlen(argv[2]) + 1, RRQ_MODE);
        int rrq_length = 2 + strlen(rrq_buffer) + 1 + strlen(argv[2]) + 1;

        // Send the RRQ packet
        if (sendto(sockfd, rrq_buffer, rrq_length, 0, res->ai_addr, res->ai_addrlen) == -1) {
            perror(SEND_ERROR);
            exit(EXIT_FAILURE);
        }

        freeaddrinfo(res);
    }

    exit(EXIT_SUCCESS);
}