#include <stdio.h>
#include "tftp.h"

#define PORTSERVER "69"
#define LOCALHOST "127.0.0.1"

int main() {
    gettftp(LOCALHOST, PORTSERVER, "ones512");
}