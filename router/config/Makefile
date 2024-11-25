CC     = gcc
CFLAGS = -g -Wall

all:	testscan testparse 
	./testscan
	./testparse

testscan: testscan.o scan.o
	$(CC) -o $@ $(CFLAGS) $^

testparse: testparse.o parse.o scan.o
	$(CC)  -o $@ $(CFLAGS) $^

scan.o:	scan.h parse.h
parse.o:	scan.h parse.h
testscan.o:	scan.h parse.h
testparse.o:	scan.h parse.h

realclean: emacsClean clean

clean:
	-rm -f *.o testscan testparse

emacsClean:
	-rm -f *~

