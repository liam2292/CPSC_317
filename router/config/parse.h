#ifndef __CONFIG_PARSE_H__
#define __CONFIG_PARSE_H__
#include <setjmp.h>
#include "scan.h"

typedef struct cparsestate {
    cscanstate *scan;
    ctoken next;
    jmp_buf jmp;
    char errormsg[MAXLINELENGTH];
} cparsestate;

typedef struct cparseport {
    int port;
} cparseport, scriptport;

typedef struct cparseip {
    unsigned int part[4];
    int netlength;
} cparseip, scriptip;

typedef struct cparserule {
    cparseip ip;
    int port;
} cparserule, scriptrule;

typedef struct cparseline {
    struct cparseline *next;
    ctoken key;
    cparserule rule;
    cparseport port;
} cparseline, cscript;
    
extern void configParseInit(cparsestate *s, cscanstate *scan);
extern cscript *configParse(cparsestate *s);
extern void configParsePrint(cparseline *);
#endif
