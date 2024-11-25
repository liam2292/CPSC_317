#include <arpa/inet.h>
#include <stdio.h>
#include "tcp.h"

#define MAXLENGTH 128
#define NBUFFERS   20

char *tcpHdrToString(tcpheader *hdr) {
    static char buff[NBUFFERS][MAXLENGTH];
    static int next = 0;
    char *buf = &buff[next][0];
    next = (next + 1) % NBUFFERS;
    
    snprintf(buf, MAXLENGTH, "%s%s%s%s%d->%d CkSum: 0x%04x Seq: %d (%08x) Ack: %d (%08x) Win: %d",
	     getSyn(hdr) ? "SYN " : "", 
	     getAck(hdr) ? "ACK " : "", 
	     getFin(hdr) ? "FIN " : "", 
	     getRst(hdr) ? "RST " : "", hdr->srcPort, hdr->dstPort, hdr->checksum, hdr->seqNo, hdr->seqNo, hdr->ackNo, hdr->ackNo, hdr->windowSize);
    return buf;
}
	     
void ntohHdr(tcpheader *hdr) {
    hdr->srcPort = ntohs(hdr->srcPort);
    hdr->dstPort = ntohs(hdr->dstPort);
    hdr->seqNo = ntohl(hdr->seqNo);
    hdr->ackNo = ntohl(hdr->ackNo);
    hdr->windowSize = ntohs(hdr->windowSize);
    hdr->checksum = ntohs(hdr->checksum);
    hdr->urgentPointer = ntohs(hdr->urgentPointer);
}
    
void htonHdr(tcpheader *hdr) {
    hdr->srcPort = htons(hdr->srcPort);
    hdr->dstPort = htons(hdr->dstPort);
    hdr->seqNo = htonl(hdr->seqNo);
    hdr->ackNo = htonl(hdr->ackNo);
    hdr->windowSize = htons(hdr->windowSize);
    hdr->checksum = htons(hdr->checksum);
    hdr->urgentPointer = htons(hdr->urgentPointer);
}
    
