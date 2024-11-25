#include <arpa/inet.h>
#include <stdio.h>
#include "ip.h"

#define MAXLENGTH 128
#define NBUFFERS    2

char *ipHdrToString(ipheader *hdr) {
    static char buff[NBUFFERS][MAXLENGTH];
    static int next = 0;
    char *buf = &buff[next][0];
    next = (next + 1) % NBUFFERS;
    
    snprintf(buf, MAXLENGTH, "src: %d.%d.%d.%d dst: %d.%d.%d.%d CkSum: 0x%04x Len: %4d TTL: %3d Prot: 0x%02x",
	     (hdr->srcipaddr & 0xFF000000) >> 24,
	     (hdr->srcipaddr & 0x00FF0000) >> 16,
	     (hdr->srcipaddr & 0x0000FF00) >> 8,
	     (hdr->srcipaddr & 0x000000FF),
	     (hdr->dstipaddr & 0xFF000000) >> 24,
	     (hdr->dstipaddr & 0x00FF0000) >> 16,
	     (hdr->dstipaddr & 0x0000FF00) >> 8,
	     (hdr->dstipaddr & 0x000000FF),
	     hdr->checksum, hdr->length, hdr->ttl, hdr->protocol);
    return buf;
}
	     
void ntohHdr(ipheader *hdr) {
    hdr->length = ntohs(hdr->length);
}
    
void htonHdr(ipheader *hdr) {
    hdr->length = htons(hdr->length);
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

