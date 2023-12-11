#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>

#define ARGUMENTSERROR "Error, you must specify 3 arguments."
#define SERVERERROR "Error trying to get the server adress."

void main(int argc, char** argv) {

    // Argument count test
    if (argc != 4) {
        printf(ARGUMENTSERROR);
    } else {

        // Get the server address
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof hints);  // Hints structure
        hints.ai_family = AF_INET;                 // IPv4
        hints.ai_socktype = SOCK_DGRAM;            // UDP socket

        // Try to get the server address
        if (getaddrinfo(argv[1], NULL, &hints, &res) !=0) {
            perror(SERVERERROR);
            exit(EXIT_FAILURE);
        }

        // Socket Reservation
        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        freeaddrinfo(res);
    }

    exit(EXIT_SUCCESS);
}