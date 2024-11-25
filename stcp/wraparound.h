#ifndef __WRAPAROUND_H__
#define __WRAPAROUND_H__
extern unsigned short plus16(unsigned short val1,  unsigned short val2);
extern unsigned short minus16(unsigned short greaterVal, unsigned short lesserVal);
extern int greater16(unsigned short val1, unsigned short val2);

extern unsigned int plus32(unsigned int a,  unsigned int b);
extern unsigned int minus32(unsigned int a, unsigned int b);
extern int greater32(unsigned int a, unsigned int b);
#endif
