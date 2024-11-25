#ifndef __TCP_H__
#define __TCP_H__
typedef struct tcpheader {
    unsigned short srcPort;                     // Not used (should always be 0)
    unsigned short dstPort;                     // Not used (should always be 0)
    unsigned int seqNo;
    unsigned int ackNo;
    unsigned char dataOffset;                   // Not used (should always be 5)
    unsigned char flags;
    unsigned short windowSize;
    unsigned short checksum;
    unsigned short urgentPointer;               // Not used (should always be 0)
} tcpheader;

typedef enum tcpflags {
    FIN = 0b000000001,
    SYN = 0b000000010,
    RST = 0b000000100,
    ACK = 0b000010000
} tcpflags;
    
static inline void setFin(tcpheader *hdr) { hdr->flags |= FIN; }
static inline void setSyn(tcpheader *hdr) { hdr->flags |= SYN; }
static inline void setRst(tcpheader *hdr) { hdr->flags |= RST; }
static inline void setAck(tcpheader *hdr) { hdr->flags |= ACK; }
static inline int getFin(tcpheader *hdr) { return (hdr->flags & FIN) >> 0; }
static inline int getSyn(tcpheader *hdr) { return (hdr->flags & SYN) >> 1; }
static inline int getRst(tcpheader *hdr) { return (hdr->flags & RST) >> 2; }
static inline int getAck(tcpheader *hdr) { return (hdr->flags & ACK) >> 4; }

extern char *tcpHdrToString(tcpheader *hdr);
extern void ntohHdr(tcpheader *hdr);
extern void htonHdr(tcpheader *hdr);
#endif
