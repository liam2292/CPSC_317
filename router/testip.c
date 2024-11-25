#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include "ip.h"

int main(int argc, char **argv) {
    ipheader hdr;
    bzero(&hdr, sizeof(hdr));
    hdr.length = 34;
    hdr.protocol = 0x80;
    hdr.ttl = 23;
    hdr.srcipaddr = 0xc0a80a1f;
    hdr.dstipaddr = 0xc0a80a08;
    hdr.checksum = ipchecksum(&hdr, sizeof(hdr));
    assert(ipchecksum(&hdr, sizeof(hdr)) == 0);

    printf("%s\n", ipHdrToString(&hdr));
    ntohHdr(&hdr);
    printf("%s\n", ipHdrToString(&hdr));
    ntohHdr(&hdr);
    printf("%s\n", ipHdrToString(&hdr));
    return 0;
}
