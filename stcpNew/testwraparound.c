#include "wraparound.h"
#include <assert.h>

int main(int argc, char **argv) {
    unsigned int a, b, c, d;

    for (long i = (1L << 32) - (1L << 16); i < 1L << 32; i++) {
	for (long j = 1; j < 0x7fffffff; j += 0xffff) {
	    a = i;
	    b = j;
	    c = plus32(a, b);
	    assert(greater32(c, a));
	}
	b = 0x7fffffff;
	c = plus32(a, b);
	assert(greater32(c, a));	
    }

    for (long i = (1L << 32) - (1L << 16); i < 1L << 32; i++) {
	for (long j = 1; j < 0x7fffffff; j += 0xffff) {
	    a = i;
	    b = j;
	    c = plus32(a, b);
	    d = minus32(c, a);
	    assert(d == b);
	    d = minus32(c, b);
	    assert(d == a);
	}
    }
}
