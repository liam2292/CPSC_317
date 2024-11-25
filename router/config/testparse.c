#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scan.h"
#include "parse.h"

int main(int argc, char **argv) {
    cparsestate pstate, *s = &pstate;
    cscanstate sstate, *ss = &sstate;
    cparseline *l;
    configScanInit(ss);
    configScanTarget(ss, "\n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    configParsePrint(l);
    configScanTargetFile(ss, "example");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l != NULL);
    configParsePrint(l);
    configScanTarget(ss, "foo \n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected port or forward found error"));

    configScanTarget(ss, "forward 142.103.0. port 1\n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected num found port"));

    configScanTarget(ss, "forward 142.103.. port 1\n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected num found ."));

    configScanTarget(ss, "forward 142.103.0.0/15 port \n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected num found nl"));

    configScanTarget(ss, "forward 142.103.0.0/ port 1\n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected num found port"));

    configScanTarget(ss, "forward 142.103.0.0/16 1\n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected port found num"));

    configScanTarget(ss, "ack ack\n");
    configParseInit(s, ss);
    l = configParse(s);
    assert(l == NULL);
    assert(!strcmp(s->errormsg, "Syntax error: expected port or forward found error"));
}
