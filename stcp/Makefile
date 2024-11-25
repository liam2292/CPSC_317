#
# This code is adapted from a programming course at 
# Boston University for use in CPSC 317 at UBC
#
#
# Version 1.0
#


CC     = gcc
CFLAGS = -g -Wall

all:	testwraparound testtcp sender waitForPorts 
	bash ./runnoerrors.sh

sender: sender.o stcp.o wraparound.o tcp.o log.o
	$(CC) -o $@ $(CFLAGS) $^

wraparound.o: stcp.h wraparound.c
	$(CC) -c -o  $@  $(CFLAGS) wraparound.c

stcp.o: stcp.h stcp.c
	$(CC) -c -o  $@  $(CFLAGS) stcp.c

waitForPorts:	waitForPorts.c
	$(CC) -o $@  $(CFLAGS) $^

testwraparound: testwraparound.o wraparound.o
	$(CC)  -o $@ $(CFLAGS) $^

testtcp: testtcp.o tcp.o
	$(CC)  -o $@ $(CFLAGS) $^

clean:
	-rm -f *.o sender testwraparound testtcp waitForPorts OutputFile
