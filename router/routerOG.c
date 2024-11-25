#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "ip.h"
#include "config.h"
#include "util.h"
#include "log.h"

#define MAX_INTERFACES 8

typedef struct forward_entry
{
    uint32_t network;
    uint32_t netmask;
    int netlength;
    int interface;
    struct forward_entry *next;
} forward_entry_t;

forward_entry_t *table = NULL;

int interfaces[MAX_INTERFACES] = {0};

/*
 * Add a forwarding entry to your forwarding table.  The network address is
 * given by the network and netlength arguments.  Datagrams destined for
 * this network are to be sent out on the indicated interface.
 */
void addForwardEntry(uint32_t network, int netlength, int interface)
{
    // TODO: Implement this
    if (interface < 0 || interface >= MAX_INTERFACES || interfaces[interface] != 1)
    {
        return;
    }
    forward_entry_t *newEntry = (forward_entry_t *)malloc(sizeof(forward_entry_t));
    if (!newEntry)
    {
        return;
    }
    newEntry->network = network;
    newEntry->netlength = netlength;
    newEntry->interface = interface;
    if (netlength == 0)
    {
        newEntry->netmask = 0;
    }
    else
    {
        newEntry->netmask = (~((1U << (32 - netlength)) - 1));
    }
    newEntry->next = table;
    table = newEntry;
}

/*
 * Add an interface to your router.
 */
void addInterface(int interface)
{
    // TODO: Implement this
    if (interface >= 0 && interface < MAX_INTERFACES)
    {
        interfaces[interface] = 1;
    }
}

int forwardInterface(uint32_t dstIp)
{
    forward_entry_t *current = table;
    int match = -1;
    int outInter = -1;
    int defaultInter = -1;

    while (current != NULL)
    {
        if(current->netlength == 0) defaultInter = current->interface;
        uint32_t maskedDst = dstIp & current->netmask;
        uint32_t maskedNet = current->network & current->netmask;

        if (maskedDst == maskedNet && current->netlength > match)
        {
            match = current->netlength;
            outInter = current->interface;
        }
        current = current->next;
    }
    //printf("outInter:%d \n", outInter);
    return (outInter != -1) ? outInter : defaultInter;
}

void run()
{
    int port = getDefaultPort();
    int fd = udp_open(port);
    // TODO: Implement this

    packet pkt;
    int interface;

    while (1)
    {
        int len = readpkt(fd, &pkt, &interface);
        if(interface == 0) {
            break;
        }
        if (len > 0)
        {
            initPacket(&pkt, (unsigned char *)pkt.data, len);
            ntohHdr(pkt.hdr);

            if (pkt.hdr->ttl > 0)
            {
                pkt.hdr->ttl -= 1;
                
            }

            int newInter = forwardInterface(pkt.hdr->dstipaddr);

            if (interface != newInter && pkt.hdr->ttl > 0)
            {
                htonHdr(pkt.hdr);
                pkt.hdr->checksum = 0;
                pkt.hdr->checksum = ipchecksum(pkt.data, sizeof(ipheader));
                sendpkt(fd, newInter, &pkt);
            }
        }
        else if (len == UTIL_READ_PERMANENT_FAILURE)
        {
            logLog("error", "Permanent failure reading packet");
            break;
        }
    }
}

int main(int argc, char **argv)
{
    logConfig("router", "packet,error,failure");

    char *configFileName = "router.config";
    if (argc > 1)
    {
        configFileName = argv[1];
    }
    configLoad(configFileName, addForwardEntry, addInterface);
    run();
    return 0;
}