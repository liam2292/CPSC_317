#
# This code is developed for use in CPSC 317 at UBC
#
#
# Version 1.0
#


CC     = gcc
CFLAGS = -g -Wall

all:    testip router waitForPorts tester

test:   all
	./tester tosend.script

router: router.o ip.o ip.o log.o config/parse.o config/scan.o config.o util.o
	$(CC) -o $@ $(CFLAGS) $^

util.o: util.h util.c
	$(CC) -c -o  $@  $(CFLAGS) util.c

ip.o: ip.h ip.c
	$(CC) -c -o  $@  $(CFLAGS) ip.c

config.o: config.h config.c ip.h
	$(CC) -c -o  $@  $(CFLAGS) config.c

waitForPorts:   waitForPorts.c 
	$(CC) -o $@  $(CFLAGS) $^

testip: testip.o ip.o
	$(CC)  -o $@ $(CFLAGS) $^

tester: tester_linux tester_mac_apple tester_mac_intel
	@echo "Choose one of the executables for the tester"
	@echo "i.e., ln tester_linux tester ; touch tester"
clean:
	-rm -rf *.o router tester testip waitForPorts *.dSYM
	-cd config ; make clean
