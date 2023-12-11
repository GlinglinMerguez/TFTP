#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


#define ARGUMENTS_ERROR "Error, you must specify 3 arguments."
#define SERVER_ADDR_ERROR "Error trying to get the server address."
#define SOCKET_ERROR "Socket creation error"
#define SEND_ERROR "Error sending data"
#define RECEIVE_ERROR "Error receiving data"

#define MAX_BUFFER_SIZE 516
#define RRQ_MODE "octet"


void main(int argc, char** argv) {

    // Q1 - Argument count test
    if (argc != 4) {
        printf(ARGUMENTS_ERROR);
    } else {

        // Q2 - Initialise the hints structure for getaddrinfo
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof hints);  // Hints structure
        hints.ai_family = AF_INET;                 // IPv4
        hints.ai_socktype = SOCK_DGRAM;            // UDP socket

        // Try to get the server address
        if (getaddrinfo(argv[1], argv[2], &hints, &res) !=0) {
            perror(SERVER_ADDR_ERROR);
            freeaddrinfo(res);
            exit(EXIT_FAILURE);
        }

        // Q3 - Socket Reservation
        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            perror(SOCKET_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            exit(EXIT_FAILURE);
        }

        // Q4a - Build RRQ packet
        char rrq_buffer[MAX_BUFFER_SIZE];
        rrq_buffer[0] = 0x00;  // Opcode (2 bytes)
        rrq_buffer[1] = 0x01;
        strcpy(rrq_buffer + 2, argv[3]);  // Filename
        strcpy(rrq_buffer + 2 + strlen(argv[3]) + 1, RRQ_MODE);  // 1 byte of zeros + the mode
        int rrq_length = 2 + strlen(rrq_buffer) + 1 + strlen(argv[3]) + 1;

        // Send the RRQ packet
        if (sendto(sockfd, rrq_buffer, rrq_length, 0, res->ai_addr, res->ai_addrlen) == -1) {
            perror(SEND_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            exit(EXIT_FAILURE);
        }

        // Q4b - Receive Data
        char data_buffer[MAX_BUFFER_SIZE];
        socklen_t server_addr_len = sizeof(struct sockaddr);
        ssize_t recv_length = recvfrom(sockfd, data_buffer, MAX_BUFFER_SIZE, 0, res->ai_addr, &server_addr_len);
        if (recv_length == -1) {
            perror(RECEIVE_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            exit(EXIT_FAILURE);
        }

        // Re-sending the Block number to Acknowledge
        char block[MAX_BUFFER_SIZE];
        block[0] = 0x00;
        block[1] = 0x04;
        strcpy(block, data_buffer + 2);  // Getting the Block number
        if (sendto(sockfd, block, 4, 0, res->ai_addr, res->ai_addrlen) == -1) {
            perror(SEND_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            exit(EXIT_FAILURE);
        }

        // Writing the Data into the shell
        write(STDOUT_FILENO, data_buffer+4, strlen(data_buffer+4));

        // Q4c -

        close(sockfd);
        freeaddrinfo(res);
    }

    exit(EXIT_SUCCESS);
}