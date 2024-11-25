#include <stdio.h>
#include <strings.h>
#include "tcp.h"

int main(int argc, char **argv) {
    tcpheader hdr;
    bzero(&hdr, sizeof(hdr));
    hdr.srcPort = 513;
    hdr.dstPort = 1027;
    hdr.checksum = 6 * 256 + 5;
    hdr.windowSize = 8 * 256 + 7;
    hdr.ackNo = 7;
    hdr.seqNo = 23;
    setSyn(&hdr);
    printf("%s\n", tcpHdrToString(&hdr));
    setFin(&hdr);
    setRst(&hdr);
    setAck(&hdr);
    printf("%s\n", tcpHdrToString(&hdr));
    ntohHdr(&hdr);
    printf("%s\n", tcpHdrToString(&hdr));
    ntohHdr(&hdr);
    printf("%s\n", tcpHdrToString(&hdr));
    return 0;
}
