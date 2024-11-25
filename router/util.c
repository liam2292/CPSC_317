/*
 * Developed for use in CPSC 317 at UBC
 * 
 * Version 1.0
 */

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ip.h"
#include "util.h"
int localport;

/*
 * Return a port number based on the uid of the caller.  This will
 * with reasonably high probability return a port number different from
 * that chosen for other uses on the undergraduate Linux systems.
 *
 * This port is used if ports are not specified on the command line.
 */
int getDefaultPort() {
    uid_t uid = getuid();
    int port = (uid % (65536 / 8 - 1024 / 8)) * 8 + 1024;
    assert(port >= 1024 && port <= 65535 - 1);
    return port;
}

/*
 * Convert a DNS name or numeric IP address into an integer value
 * (in network byte order).  This is more general-purpose than
 * inet_addr() which maps dotted pair notation to uint. 
 */
unsigned int hostname_to_ipaddr(const char *s) {
    if (isdigit(*s)) {
	return (unsigned int)inet_addr(s);
    } else {
	struct hostent *hp = gethostbyname(s);
	if (hp == 0) {
	    /* Error */
	    return 0;
	} 
	return *((unsigned int **)hp->h_addr_list)[0];
    }
}

/*
 * Print an IP packet to standard output. 
 *   dir is either 's'ent or 'r'eceived.  
 *   interface is the interface that it was received on or will be sent on
 */
void dump(char dir, int interface, void *pkt, int len) {
    ipheader *sipHeader = (ipheader *) pkt;
    logLog("packet", "%c [%d] %s payload %d bytes", dir, interface, ipHdrToString(sipHeader), len - sizeof(ipheader));
    fflush(stdout);
}

/*
 * Helper function to read a IP packet from the network.
 * As a side effect print the packet header to standard output.
 */
int readpkt(int fd, packet *pkt, int *interface) {
    struct sockaddr address;
    socklen_t addrlen = sizeof(address);
    struct sockaddr_in *sinp = (struct sockaddr_in *) &address;
    int cc = recvfrom(fd, &pkt->data, sizeof(pkt->data), 0, &address, &addrlen);
    if (cc > 0) {
	pkt->len = cc;
	ipheader *hdr = (ipheader *)&pkt->data;
	assert(addrlen == sizeof(*sinp));
	int port = ntohs(sinp->sin_port);
	*interface = port - localport;
	ntohHdr(hdr);
	dump('r', *interface, pkt, cc);
	htonHdr(hdr);
    } else {
	logPerror("readpkt");
	if (errno == ECONNREFUSED) return UTIL_READ_PERMANENT_FAILURE;
    }
    return cc;
}

/*
 * Helper function to send a packet on an interface
 */
int sendpkt(int fd, int interface, packet *pkt) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    address.sin_port = htons(localport + interface);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(0x7f000001);
    ipheader *hdr = (ipheader *)pkt;
    ntohHdr(hdr);
    dump('s', interface, pkt, pkt->len);
    htonHdr(hdr);
    int cc = sendto(fd, pkt, pkt->len, 0, (struct sockaddr *)&address, addrlen);
    if (cc < 0) {
	logPerror("sendpkt");
    }
    return cc;
}

/*
 * Open a UDP connection.
 */
int udp_open(int local_port) {
    int      fd;
    struct   sockaddr_in sin;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
	logPerror("Error creating UDP socket");
	return -1;
    }

    /* Bind the local socket to listen at the local_port. */
    logLog("init", "Binding locally to port %d", local_port);
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0x7f000001);
    sin.sin_port = htons(local_port);

    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	logPerror("Bind failed");
	return -2;
    }
    localport = local_port;
    return fd;
}
