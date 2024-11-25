#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*
 * Return a port number based on the uid of the caller.  This will
 * with reasonably high probability return a port number different from
 * that chosen for other users on the undergraduate Linux systems.
 *
 * This port is used if ports are not specified on the command line.
 */
int getDefaultPort() {
    uid_t uid = getuid();
    int port = (uid % (65536 / 8 - 1024 / 8)) * 8 + 1024;
    assert(port >= 1024 && port <= 65535 - 1);
    return port;
}

int openUdpPort(int port);

int main(int argc, char **argv) {
    int port = getDefaultPort();
    if (argc > 1) port = strtol(argv[1], NULL, 10);
    for (int i = 0; i < 8; i++) {
	openUdpPort(port + i);
    }
}

/*
 * Claim a given UDP port, if busy wait until it is free
 */
int openUdpPort(int port) {
    int      fd;
    struct   sockaddr_in sin;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
	perror("Error creating UDP socket");
	return -1;
    }

    /* Bind the local socket to listen at the port. */
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    while (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	perror("Bind failed");
	sleep(1);
    }
    return (fd);
}
