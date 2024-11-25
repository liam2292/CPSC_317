#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "scan.h"
#include "parse.h"

#define MAXLENGTH 128
#define NBUFFERS    8

static char *iptostring(cparseip *ip) {
    static char buff[NBUFFERS][MAXLENGTH];
    static int next = 0;
    char *buf = &buff[next][0];
    next = (next + 1) % NBUFFERS;
    
    snprintf(buf, MAXLENGTH, "%d.%d.%d.%d/%d",
	     ip->part[0], ip->part[1], ip->part[2], ip->part[3], ip->netlength);
    return buf;
}
	     

static void configParseNext(cparsestate *s) {
    s->next = configScanNext(s->scan);
}

static void configParseError(cparsestate *s, ctoken t) {
    snprintf(s->errormsg, sizeof(s->errormsg), "Syntax error: expected %s found %s", configTokenName(t), configTokenName(s->next));
    longjmp(s->jmp, 1);
}

static void configParseError2(cparsestate *s, ctoken t1, ctoken t2) {
    snprintf(s->errormsg, sizeof(s->errormsg), "Syntax error: expected %s or %s found %s", configTokenName(t1), configTokenName(t2), configTokenName(s->next));
    longjmp(s->jmp, 1);
}

static void advance(cparsestate *s, ctoken t) {
    if (s->next == t) {
	configParseNext(s);
    } else {
	configParseError(s, t);
    }
}

static cparseport configParsePort(cparsestate *s);
static cparseip configParseIP(cparsestate *s);
static cparserule configParseRule(cparsestate *s);
static cparseline *configParseS(cparsestate *s);
static cparseline *configParseLine(cparsestate *s);

cparseip defaultparseip = { .part[0] = 0, .part[1] = 0, .part[2] = 0, .part[3] = 0, .netlength = 0 };
cparseport defaultparseport = { .port = 0 };
cparserule defaultparserule = { .ip = { .part[0] = 0, .part[1] = 0, .part[2] = 0, .part[3] = 0, .netlength = 0 }, .port = 0 };

void configParseInit(cparsestate *s, cscanstate *scan) {
    s->scan = scan;
    configParseNext(s);
}

static cparseip configParseIP(cparsestate *s) {
    cparseip ans = {.netlength = 0 };
    if (s->next != TCNUM) { 
	configParseError(s, TCNUM);
	return ans;
    }
    ans.part[0] = strtol(configScanLexeme(s->scan), NULL, 10);
    configParseNext(s);
    for (int i = 1; i <= 3; i++) {
	if (s->next != TCDOT) { 
	    configParseError(s, TCDOT);
	    return ans;
	}
	configParseNext(s);
	if (s->next != TCNUM) { 
	    configParseError(s, TCNUM);
	    return ans;
	} else {
	    ans.part[i] = strtol(configScanLexeme(s->scan), NULL, 10);
	}
	configParseNext(s);
    }
    if (s->next != TCSLASH) { 
	configParseError(s, TCSLASH);
	return ans;
    }
    configParseNext(s);
    if (s->next != TCNUM) { 
	configParseError(s, TCNUM);
	return ans;
    } else {
	ans.netlength = strtol(configScanLexeme(s->scan), NULL, 10);
    }
    
    configParseNext(s);
    return ans;
}
    
static cparseport configParsePort(cparsestate *s) {
    cparseport ans = { .port = 0 };
    advance(s, TCPORT);
    if (s->next != TCNUM) {
	configParseError(s, TCNUM);
	return ans;
    }
    ans.port = strtol(configScanLexeme(s->scan), NULL, 10);    
    configParseNext(s);
    return ans;
}

static cparserule configParseRule(cparsestate *s) {
    cparserule ans = { .port = 0 };
    advance(s, TCFORWARD);
    ans.ip = configParseIP(s);
    advance(s, TCPORT);
    if (s->next == TCNUM) {
	ans.port = strtol(configScanLexeme(s->scan), NULL, 10);
	advance(s, TCNUM);
    } else {
	configParseError(s, TCNUM);
    }
    return ans;
}

static cparseline *configParseLine(cparsestate *s) {
    cparseline *ans = NULL;
    cparseport p = defaultparseport;
    cparserule r = defaultparserule;
    ctoken key = s->next;
    if (s->next == TCPORT) {
	p = configParsePort(s);
    } else if (s->next == TCFORWARD) {
	r = configParseRule(s);
    } else if (s->next == TCNL) {
	advance(s, TCNL);
	return NULL;
    } else {
	configParseError2(s, TCPORT, TCFORWARD);
    }
    ans = malloc(sizeof(cparseline));
    ans->next = NULL;
    ans->key = key;
    ans->port = p;
    ans->rule = r;
    advance(s, TCNL);
    return ans;
}

static cparseline *configParseS(cparsestate *s) {
    cparseline *ans = NULL, **ansp = &ans;
    while (s->next != TCEOF) {
	cparseline *l = configParseLine(s);
	if (l != NULL) {
	    *ansp = l;
	    ansp = &l->next;
	}
    }
    return ans;
}

cparseline *configParse(cparsestate *s) {
    int x = setjmp(s->jmp);
    if (x == 0) {
	return configParseS(s);
    } else {
	fprintf(stderr, "%s\n", s->errormsg);
	return NULL;
    }
}

static void configPrintRule(cparserule r) {
    printf("forward %s port %d", iptostring(&r.ip), r.port);
}

static void configPrintPort(cparseport p) {
    printf("port %d", p.port);    
}


static void configPrintLine(cparseline *l) {
    if (l->key == TCPORT) {
	configPrintPort(l->port);
    } else if (l->key == TCFORWARD) {
	configPrintRule(l->rule);
    } else {
	assert(0);
    }
    putchar('\n');
}

void configParsePrint(cparseline *l) {
    while (l) {
	configPrintLine(l);
	l = l->next;
    }
}
