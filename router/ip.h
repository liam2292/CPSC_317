#ifndef __IP_H__
#define __IP_H__

#ifndef __LOG_H__
#include "log.h"
#endif // __LOG_H__


#define IP_MAX_DATAGRAM_LENGTH 512

typedef struct ipheader {
    unsigned char verslen;	// version in first 4 bits (== 4), len in second 4 bits (== 5) (header length in 32 bit words)
    unsigned char typeOfService;// Not used, must be 0
    unsigned short length;	// Length in bytes of the entire datagram
    unsigned short id;		// Not used, must be 0
    unsigned short flagsoffset;	// Not used, must be 0
    unsigned char ttl;		// Time to live in hops
    unsigned char protocol;	// Upper level protocol.  May be any value
    unsigned short checksum;	// Checksum of the IP header
    unsigned int srcipaddr;	// Src IP address
    unsigned int dstipaddr;	// Dst IP address
} ipheader;

typedef struct packet {
    char data[IP_MAX_DATAGRAM_LENGTH];
    ipheader *hdr;
    int len;
} packet;

extern unsigned short ipchecksum(void *data, int len);
extern char *ipHdrToString(ipheader *hdr);
extern void ntohHdr(ipheader *hdr);
extern void htonHdr(ipheader *hdr);

#endif
