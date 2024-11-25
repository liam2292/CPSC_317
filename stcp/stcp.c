/*
 * Adapted from a course at Boston University for use in CPSC 317 at
 * UBC
 * 
 * Version 1.0
 */

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

#include "stcp.h"


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
 * Print an STCP packet to standard output. dir is either 's'ent or
 * 'r'eceived packet
 */
void dump(char dir, void *pkt, int len) {
    tcpheader *stcpHeader = (tcpheader *) pkt;
    logLog("packet", "%c %s payload %d bytes", dir, tcpHdrToString(stcpHeader), len - sizeof(tcpheader));
    fflush(stdout);
}

// Compute Internet Checksum for "len" bytes beginning at location "data".
unsigned short ipchecksum(void *data, int len) {
    register int sum = 0;

    while (len > 1)  {
        sum += * (unsigned short *) data;
        data += 2;
        len -= 2;
    }

    /*  Add left-over byte, if any */
    if (len > 0) {
        sum += * (unsigned char *) data;
    }
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;
}

/*
 * Helper function to prepare an STCP segment for sending.  Initializes all
 * the fields of the header except the checksum.
 */
void createSegment(packet *pkt, int flags, unsigned short rwnd, unsigned int seq, unsigned int ack, unsigned char *data, int len) {
    initPacket(pkt, data, len + sizeof(tcpheader));
    tcpheader *hdr = pkt->hdr;
    hdr->srcPort = 0;
    hdr->dstPort = 0;
    hdr->seqNo = seq;
    hdr->ackNo = ack;
    hdr->windowSize = rwnd;
    hdr->flags = (5 << 12) | flags;
    hdr->checksum = 0;
}

/*
 * Helper function to read a STCP packet from the network.
 * As a side effect print the packet header to standard output.
 */
static int readpkt(int fd, void *pkt, int len) {
    int cc = recv(fd, pkt, len, 0);
    if (cc > 0) {
        tcpheader *hdr = (tcpheader *)pkt;
        ntohHdr(hdr);
        dump('r', pkt, cc);
        htonHdr(hdr);
    } else {
        logPerror("readpkt");
        if (errno == ECONNREFUSED) return STCP_READ_PERMANENT_FAILURE;
    }
    return cc;
}

/*
 * Read a packet from the network or timeout if no packet is received within ms milliseconds.
 * Returns:
 *   The length of the packet if one is received, or
 *   STCP_READ_TIMED_OUT if "ms" milliseconds transpire before a packet arrives
 *   STCP_READ_PERMANENT_FAILURE if reads will never work again (socket closed)
 */
int readWithTimeout(int fd, unsigned char *pkt, int ms) {
    int s;
    fd_set fds;
    struct timeval tv;
  
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms - tv.tv_sec * 1000) * 1000;
  
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    s = select(fd + 1, &fds, 0, 0, &tv);
    if (s > 0 && FD_ISSET(fd, &fds)) {
        int res = readpkt(fd, pkt, STCP_MTU);
        if (res < 0) {
            logPerror("readWithTimeout");
            return errno == ECONNREFUSED ? STCP_READ_PERMANENT_FAILURE :  STCP_READ_TIMED_OUT;
        }
        return res;
    } else {
        return STCP_READ_TIMED_OUT;
    }
}

/*
 * Set an I/O channel (file descriptor) to non-blocking mode.
 */
void nonblock(int fd) {       
    int flags = fcntl(fd, F_GETFL, 0);
#if defined(hpux) || defined(__hpux)
    flags |= O_NONBLOCK;
#else
    flags |= O_NONBLOCK|O_NDELAY;
#endif
    if (fcntl(fd, F_SETFL, flags) == -1) {
        logPerror("fcntl: F_SETFL");
        exit(1);
    }
}

/*
 * Open a UDP connection.
 */
int udp_open(char *remote_IP_str, int remote_port, int local_port) {
    int      fd;
    uint32_t dst;
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
    sin.sin_port = htons(local_port);

    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        logPerror("Bind failed");
        return -2;
    }

    /* Connect, i.e. prepare to accept UDP packets from <remote_host, remote_port>.  */
    /* Listen() and accept() are not necessary with UDP connection setup.            */
    dst = hostname_to_ipaddr(remote_IP_str);

    if (!dst) {
        fprintf(stderr, "Invalid host name: %s\n", remote_IP_str);
        return -4;
    }
    logLog("init", "Configuring  UDP \"connection\" to <%u.%u.%u.%u, port %d>",
	   (ntohl(dst)>>24) & 0xFF, (ntohl(dst)>>16) & 0xFF,
	   (ntohl(dst)>>8) & 0XFF, ntohl(dst) & 0XFF, remote_port);

    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(remote_port);
    sin.sin_addr.s_addr = dst;
    if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        logPerror("connect");
        return -1;
    }
    logLog("init", "UDP \"connection\" to <%u.%u.%u.%u, port %d> configured",
	   (ntohl(dst)>>24) & 0xFF, (ntohl(dst)>>16) & 0xFF,
	   (ntohl(dst)>>8) & 0XFF, ntohl(dst) & 0XFF , remote_port);

    return (fd);
}
