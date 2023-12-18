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
#define DATA_OPCODE 3
#define ACK_OPCODE 4


void main2(int argc, char** argv) {

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
        memcpy(rrq_buffer + 2, argv[3], strlen(argv[3]));  // Filename
        rrq_buffer[2 + strlen(argv[3])] = 0;  // Null-terminate the filename
        memcpy(rrq_buffer + 2 + strlen(argv[3]) + 1, RRQ_MODE, strlen(RRQ_MODE));  // Mode
        rrq_buffer[2 + strlen(argv[3]) + 1 + strlen(RRQ_MODE)] = 0;  // Null-terminate the mode
        int rrq_length = 2 + strlen(argv[3]) + 1 + strlen(RRQ_MODE) + 1;

        // Send the RRQ packet
        if (sendto(sockfd, rrq_buffer, rrq_length, 0, res->ai_addr, res->ai_addrlen) == -1) {
            perror(SEND_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            exit(EXIT_FAILURE);
        }

        // Q4b - Receive Data
        char data_buffer[MAX_BUFFER_SIZE];
        int block_number = 1;  // Initial block number
        socklen_t server_addr_len = sizeof(struct sockaddr);

        // Q4c - Multiple Packets
        while(1) {
            ssize_t recv_length = recvfrom(sockfd, data_buffer, MAX_BUFFER_SIZE, 0, res->ai_addr, &server_addr_len);
            if (recv_length == -1) {
                perror(RECEIVE_ERROR);
                close(sockfd);
                freeaddrinfo(res);
                exit(EXIT_FAILURE);
            }

            if (data_buffer[1] == DATA_OPCODE) {
                int received_block_number = (data_buffer[2] << 8) | data_buffer[3];

                if (received_block_number == block_number) {
                    // Ack
                    char ack_buffer[4] = {0x00, ACK_OPCODE, data_buffer[2], data_buffer[3]};
                    if (sendto(sockfd, ack_buffer, 4, 0, res->ai_addr, res->ai_addrlen) == -1) {
                        perror(SEND_ERROR);
                        close(sockfd);
                        freeaddrinfo(res);
                        exit(EXIT_FAILURE);
                    }
                    // Print Data
                    write(STDOUT_FILENO, data_buffer + 4, recv_length - 4);

                    block_number++;

                    // Check if it's the last data block
                    if (recv_length < MAX_BUFFER_SIZE) {
                        break;
                    }
                }
            }
        }

        close(sockfd);
        freeaddrinfo(res);
    }

    exit(EXIT_SUCCESS);
}