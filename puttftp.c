#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>


#define ARGUMENTS_ERROR "Error, you must specify 3 arguments."
#define SERVER_ADDR_ERROR "Error trying to get the server address."
#define SOCKET_ERROR "Socket creation error"
#define SEND_ERROR "Error sending data"
#define RECEIVE_ERROR "Error receiving data"

#define MAX_BUFFER_SIZE 516
#define WRQ_MODE "octet"
#define DATA_OPCODE 3
#define ACK_OPCODE 4


void main(int argc, char** argv) {

    // Argument count test
    if (argc != 4) {
        printf(ARGUMENTS_ERROR);
        exit(EXIT_FAILURE);
    }

    // Initialise the hints structure for getaddrinfo
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

    // Socket Reservation
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror(SOCKET_ERROR);
        close(sockfd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Build WRQ packet
    char wrq_buffer[MAX_BUFFER_SIZE];
    wrq_buffer[0] = 0x00;  // Opcode (2 bytes)
    wrq_buffer[1] = 0x02;
    memcpy(wrq_buffer + 2, argv[3], strlen(argv[3]));  // Filename
    wrq_buffer[2 + strlen(argv[3])] = 0;  // Null-terminate the filename
    memcpy(wrq_buffer + 2 + strlen(argv[3]) + 1, WRQ_MODE, strlen(WRQ_MODE));  // Mode
    wrq_buffer[2 + strlen(argv[3]) + 1 + strlen(WRQ_MODE)] = 0;  // Null-terminate the mode
    int wrq_length = 2 + strlen(argv[3]) + 1 + strlen(WRQ_MODE) + 1;

    // Send the WRQ packet
    if (sendto(sockfd, wrq_buffer, wrq_length, 0, res->ai_addr, res->ai_addrlen) == -1) {
        perror(SEND_ERROR);
        close(sockfd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // Send Data
    char data_buffer[MAX_BUFFER_SIZE];
    int block_number = 0;  // Initial block number

    // Opening or Creating the File
    int fd = open(argv[3], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        close(sockfd);
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    // While there's still Data
    ssize_t read_length;
    while ((read_length = read(fd, data_buffer + 4, MAX_BUFFER_SIZE - 4)) > 0) {

        // Opcode
        data_buffer[0] = 0x00;
        data_buffer[1] = 0x03;

        // Block Number
        data_buffer[2] = (block_number >> 8) & 0xFF;
        data_buffer[3] = block_number & 0xFF;

        // Send Data to Server
        if (sendto(sockfd, data_buffer, read_length + 4, 0, res->ai_addr, res->ai_addrlen) == -1) {
            perror(SEND_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Receive Acknowledgment
        char ack_buffer[4];
        ssize_t recv_length = recvfrom(sockfd, ack_buffer, 4, 0, res->ai_addr, &res->ai_addrlen);
        if (recv_length == -1) {
            perror(RECEIVE_ERROR);
            close(sockfd);
            freeaddrinfo(res);
            close(fd);
            exit(EXIT_FAILURE);
        }

        int received_block_number = (ack_buffer[2] << 8) | ack_buffer[3];

        // Check for acknowledgment with correct block number
        if (received_block_number != block_number) {
            // Mismatched block numbers (retransmission, error handling, etc.)
            fprintf(stderr, "Mismatched block numbers.\n");
            close(sockfd);
            freeaddrinfo(res);
            close(fd);
            exit(EXIT_FAILURE);
        }

        block_number++;
    }

    if (read_length == -1) {
        perror(RECEIVE_ERROR);
    }

    close(sockfd);
    freeaddrinfo(res);
    close(fd);

    exit(EXIT_SUCCESS);
}