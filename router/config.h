#ifndef __CONFIG_H__
#define __CONFIG_H__
extern void configLoad(char *fileName, void (*addForwardEntry)(uint32_t network, int netlength, int interface), void (*addInterface)(int interface));

#endif
