#ifndef __LOG_H__
#define __LOG_H__
#include <sys/time.h>
#include <sys/errno.h>
extern void logConfig(char *name, char *channels);
extern void logLog(char *channel, char *format, ...);
extern void logPerror(char *who);
extern long now();


#endif
