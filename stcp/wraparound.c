
/*
 * This code has been adapted from a course at Boston University
 * for use in CPSC 317 at UBC
 *
 * Version 2.0
 * 
 */

#include "wraparound.h"

/* 
    Adds modulo 2^32 so can have seqno wraparound
    Unsigned 32 bit arithmetic just works.
 */
unsigned int plus32(unsigned int a,  unsigned int b) {
    return a + b;            
}

/* 
    Subtracts b from a, modulo 2 ^ 32

    This is useful for sequence wraparound cases, when a packet
    with a seqno 15 can actually be greater than the packet with
    a seqno 65530 because of the wraparound.

    Unsigned 32 bit arithmetic just works.
*/
unsigned int minus32(unsigned int a, unsigned int b) {
    return a - b;
}

/* Is a > b ? */
int greater32(unsigned int a, unsigned int b) {
    unsigned int maxVal = (0xffffffff / 2) + 1;

    /* How does this code work ? 
     
       The sequence numbers cannot be more than MAX_UNSIGNED_INT/2 apart.
       If they are, that means that one of the numbers has wrapped around.

       To figure out which one, we recast these numbers into signed longs
       and see if the difference is greater than MAX_UNSIGNED_INT/2 apart.

       So suppose we have a = 100 and b = 65500 and MAX_UNSIGNED_INT/2 = 2^15
       Then, we do the first comparison:
       100 - 65500 > 32678 ? No, -65400 < 32768
       Second comparison:
       65500 - 100 > 32768 ? Yes, 65400 > 32678
       So there has been some wraparound, and 100 is the greater value 
    */

    if (((long)a - (long)b) > (long)maxVal) {
	/* There has been some wraparound, b is the greater value */
	return 0;
    } else if (((long)b - (long)a) > (long)maxVal) {
	/* There's been some wraparound, a is the greater value */
	return 1;
    } else {
	return a > b;
    }
}
