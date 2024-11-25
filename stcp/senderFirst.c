/************************************************************************
 * Adapted from a course at Boston University for use in CPSC 317 at UBC
 *
 *
 * The interfaces for the STCP sender (you get to implement them), and a
 * simple application-level routine to drive the sender.
 *
 * This routine reads the data to be transferred over the connection
 * from a file specified and invokes the STCP send functionality to
 * deliver the packets as an ordered sequence of datagrams.
 *
 * Version 2.0
 *
 *
 *************************************************************************/


#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/file.h>

#include "stcp.h"

#define STCP_SUCCESS 1
#define STCP_ERROR -1
typedef enum {
    CLOSED,
    SYN_SENT,
    ESTABLISHED,
    CLOSING,
    FIN_WAIT
} SenderState;

typedef struct {

    /* YOUR CODE HERE */
    int fd;
    unsigned int seqNo;
    unsigned int ackNo;
    unsigned short windowSize;
    unsigned int state;
} stcp_send_ctrl_blk;
/* ADD ANY EXTRA FUNCTIONS HERE */


/*
 * Send STCP. This routine is to send all the data (len bytes).  If more
 * than MSS bytes are to be sent, the routine breaks the data into multiple
 * packets. It will keep sending data until the send window is full or all
 * the data has been sent. At which point it reads data from the network to,
 * hopefully, get the ACKs that open the window. You will need to be careful
 * about timing your packets and dealing with the last piece of data.
 *
 * Your sender program will spend almost all of its time in either this
 * function or in tcp_close().  All input processing (you can use the
 * function readWithTimeout() defined in stcp.c to receive segments) is done
 * as a side effect of the work of this function (and stcp_close()).
 *
 * The function returns STCP_SUCCESS on success, or STCP_ERROR on error.
 */
int stcp_send(stcp_send_ctrl_blk *stcp_CB, unsigned char* data, int length) {

    /* YOUR CODE HERE */
    
    if (stcp_CB == NULL || data == NULL || length <= 0) return STCP_ERROR;
    if (stcp_CB->state != STCP_SENDER_ESTABLISHED) return STCP_ERROR;

    unsigned int bytes_sent = 0;
    while (bytes_sent < length) {

        int segment_size = min(STCP_MSS, length - bytes_sent);

        packet pkt;
        memset(&pkt, 0, sizeof(packet));

        unsigned char flags = 0;
        createSegment(&pkt, flags, stcp_CB->windowSize, stcp_CB->seqNo, stcp_CB->ackNo, &data[bytes_sent], segment_size);
        pkt.hdr->checksum = ipchecksum(pkt.data, segment_size + sizeof(tcpheader));

        if(send(stcp_CB->fd, &pkt, sizeof(tcpheader) + segment_size, 0) < 0) return STCP_ERROR;
    
        bytes_sent += segment_size;
        stcp_CB->seqNo += segment_size;
        
        unsigned char ack_buffer[sizeof(packet)];

        //issue here READDDDD
        while(1) {
        int len = readWithTimeout(stcp_CB->fd, ack_buffer, STCP_INITIAL_TIMEOUT);
        break;
        if(len > 0) {
            tcpheader *hdr = (tcpheader*) ack_buffer;
            printf("%d\n", hdr->seqNo);
            // Assuming hdr->ackNo is in host byte order after readWithTimeout
            if(hdr->ackNo == stcp_CB->seqNo+ segment_size) {
            // Correct ACK received, can send next segment
                stcp_CB->ackNo = hdr->ackNo;
                stcp_CB->windowSize = hdr->windowSize; // Assuming the receiver may include window updates in ACKs
                break;
            }
            } else if(len == STCP_READ_TIMED_OUT) {
            // No ACK received, retransmit last segment
                if(send(stcp_CB->fd, pkt.data, sizeof(tcpheader) + segment_size, 0) < 0) {
                    return STCP_ERROR;
                }
            } else if (len == STCP_READ_PERMANENT_FAILURE) {
        // Handle a permanent failure (connection closed or unrecoverable error)
                return STCP_ERROR;
            }
        }
}
    stcp_CB->state = STCP_SENDER_CLOSING;;
    return STCP_SUCCESS;
}



/*
 * Open the sender side of the STCP connection. Returns the pointer to
 * a newly allocated control block containing the basic information
 * about the connection. Returns NULL if an error happened.
 *
 * If you use udp_open() it will use connect() on the UDP socket
 * then all packets then sent and received on the given file
 * descriptor go to and are received from the specified host. Reads
 * and writes are still completed in a datagram unit size, but the
 * application does not have to do the multiplexing and
 * demultiplexing. This greatly simplifies things but restricts the
 * number of "connections" to the number of file descriptors and isn't
 * very good for a pure request response protocol like DNS where there
 * is no long term relationship between the client and server.
 */
stcp_send_ctrl_blk *stcp_open(char *destination, int sendersPort, int receiversPort) {
    logLog("init", "Sending from port %d to <%s, %d>", sendersPort, destination, receiversPort);

    // Create UDP connection
    int fd = udp_open(destination, receiversPort, sendersPort);
    if (fd < 0) {
        perror("udp_open failed");
        return NULL;
    }

    // Allocate and initialize control block
    stcp_send_ctrl_blk *ctrl_blk = (stcp_send_ctrl_blk *)malloc(sizeof(stcp_send_ctrl_blk));
    if (ctrl_blk == NULL) {
        perror("Failed to allocate control block");
        close(fd);
        return NULL;
    }

    ctrl_blk->fd = fd;
    ctrl_blk->seqNo = 0; // Random initial sequence number
    ctrl_blk->ackNo = 0;      // Initial ACK number set to 0
    ctrl_blk->windowSize = STCP_MSS;
    ctrl_blk->state = STCP_SENDER_SYN_SENT;

    // Send SYN packet
    packet syn_pkt;
    memset(&syn_pkt, 0, sizeof(packet));
    createSegment(&syn_pkt, SYN, ctrl_blk->windowSize, ctrl_blk->seqNo, ctrl_blk->ackNo, NULL, 0);
    syn_pkt.hdr->checksum = ipchecksum(syn_pkt.data, sizeof(tcpheader));
    //printf("%s\n", tcpHdrToString(syn_pkt.hdr));
    if (send(ctrl_blk->fd, syn_pkt.data, sizeof(tcpheader), 0) < 0) {
        perror("Failed to send SYN");
        close(fd);
        free(ctrl_blk);
        return NULL;
    }

    // Wait for SYN-ACK
    unsigned char ack_buffer[sizeof(packet)];
    int len = readWithTimeout(ctrl_blk->fd, ack_buffer, STCP_INITIAL_TIMEOUT);
    if (len <= 0) {
        perror("Failed to receive SYN-ACK");
        close(fd);
        free(ctrl_blk);
        return NULL;
    }

    tcpheader *syn_ack_hdr = (tcpheader *)ack_buffer;
    if (!(getSyn(syn_ack_hdr) && getAck(syn_ack_hdr))) {
        perror("Invalid SYN-ACK received");
        close(fd);
        free(ctrl_blk);
        return NULL;
    }
    ctrl_blk->state = STCP_SENDER_ESTABLISHED;

    // Update control block with received ACK and next SEQ numbers
    // ctrl_blk->ackNo = ntohl(syn_ack_hdr->seqNo) + 1; // Expecting the next sequence number
    tcpheader *hdr = (tcpheader *) ack_buffer;
    // Send ACK packet for SYN-ACK
    packet ack_pkt;
    memset(&ack_pkt, 0, sizeof(packet));
    createSegment(&ack_pkt, ACK, ctrl_blk->windowSize, hdr->ackNo, hdr->seqNo, NULL, 0);
    ack_pkt.hdr->checksum = ipchecksum(ack_pkt.data, sizeof(tcpheader));
    printf("%s\n", tcpHdrToString(ack_pkt.hdr));
    if (send(ctrl_blk->fd, ack_pkt.data, sizeof(tcpheader), 0) < 0) {
        perror("Failed to send ACK for SYN-ACK");
        close(fd);
        free(ctrl_blk);
        return NULL;
    }

    // Update the state to ESTABLISHED
    
    printf("Connection established\n");
    return ctrl_blk;
}


/*
 * Make sure all the outstanding data has been transmitted and
 * acknowledged, and then initiate closing the connection. This
 * function is also responsible for freeing and closing all necessary
 * structures that were not previously freed, including the control
 * block itself.
 *
 * Returns STCP_SUCCESS on success or STCP_ERROR on error.
 */
int stcp_close(stcp_send_ctrl_blk *cb) {
    /* YOUR CODE HERE */
    printf("hello \n");
    // if (cb == NULL) {
    //     return STCP_ERROR;
    // }
    
    // if (cb->state != STCP_SENDER_CLOSED) {
    //     return STCP_ERROR;
    // }
    
    // cb->state = STCP_SENDER_CLOSING;

    // packet fin_pkt;
    // memset(&fin_pkt, 0, sizeof(packet));
    // createSegment(&fin_pkt, FIN, cb->windowSize, cb->seqNo, cb->ackNo, NULL, 0);
    // fin_pkt.hdr->checksum = ipchecksum(fin_pkt.data, sizeof(tcpheader));

    // if (send(cb->fd, fin_pkt.data, sizeof(tcpheader), 0) < 0) {
    //     return STCP_ERROR; 
    // }

    // cb->state = STCP_SENDER_FIN_WAIT;

    // unsigned char ack_buffer[sizeof(packet)];
    // int len = readWithTimeout(cb->fd, ack_buffer, STCP_READ_TIMED_OUT);
    // if (len <= 0) {
    //     return STCP_ERROR; 
    // }

    // tcpheader* ack_hdr = (tcpheader*) ack_buffer;
    // if (!(getAck(ack_hdr))) {
    //     cb->state = STCP_SENDER_CLOSED;
    //     return STCP_ERROR;
    // }
    // if (close(cb->fd) < 0) {
    //     cb->state = STCP_SENDER_CLOSED;
    //     return STCP_ERROR; 
    // }

    // cb->state = STCP_SENDER_CLOSED;
    // free(cb);
    return STCP_SUCCESS;
}

/*
 * Return a port number based on the uid of the caller.  This will
 * with reasonably high probability return a port number different from
 * that chosen for other uses on the undergraduate Linux systems.
 *
 * This port is used if ports are not specified on the command line.
 */
int getDefaultPort() {
    uid_t uid = getuid();
    int port = (uid % (32768 - 512) * 2) + 1024;
    assert(port >= 1024 && port <= 65535 - 1);
    return port;
}

/*
 * This application is to invoke the send-side functionality.
 */
int main(int argc, char **argv) {
    stcp_send_ctrl_blk *cb;

    char *destinationHost;
    int receiversPort, sendersPort;
    char *filename = NULL;
    int file;
    /* You might want to change the size of this buffer to test how your
     * code deals with different packet sizes.
     */
    unsigned char buffer[STCP_MSS];
    int num_read_bytes;

    logConfig("sender", "init,segment,error,failure");
    /* Verify that the arguments are right */
    if (argc > 5 || argc == 1) {
        fprintf(stderr, "usage: sender DestinationIPAddress/Name receiveDataOnPort sendDataToPort filename\n");
        fprintf(stderr, "or   : sender filename\n");
        exit(1);
    }
    if (argc == 2) {
        filename = argv[1];
        argc--;
    }

    // Extract the arguments
    destinationHost = argc > 1 ? argv[1] : "localhost";
    receiversPort = argc > 2 ? atoi(argv[2]) : getDefaultPort();
    sendersPort = argc > 3 ? atoi(argv[3]) : getDefaultPort() + 1;
    if (argc > 4) filename = argv[4];

    /* Open file for transfer */
    file = open(filename, O_RDONLY);
    if (file < 0) {
        logPerror(filename);
        exit(1);
    }

    /*
     * Open connection to destination.  If stcp_open succeeds the
     * control block should be correctly initialized.
     */
    cb = stcp_open(destinationHost, sendersPort, receiversPort);
    if (cb == NULL) {
        /* YOUR CODE HERE */
    }

    /* Start to send data in file via STCP to remote receiver. Chop up
     * the file into pieces as large as max packet size and transmit
     * those pieces.
     */
    while (1) {
        num_read_bytes = read(file, buffer, sizeof(buffer));

        /* Break when EOF is reached */
        if (num_read_bytes <= 0)
            break;

        if (stcp_send(cb, buffer, num_read_bytes) == STCP_ERROR) {
            /* YOUR CODE HERE */
        }
    }

    /* Close the connection to remote receiver */
    if (stcp_close(cb) == STCP_ERROR) {
        /* YOUR CODE HERE */
    }

    return 0;
}
