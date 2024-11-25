#ifndef __LOG_H__
#define __LOG_H__
#include <sys/time.h>
#include <sys/errno.h>

/*
 * You can create channels for diagnostic messages just by including them in the 
 * argument to logConfig and then using them in calls to logLog.  
 * 
 * See sender.c for examples.
 */

extern void logConfig(char *name, char *channels);
extern void logLog(char *channel, char *format, ...);
extern void logPerror(char *who);
extern long now();


#endif
