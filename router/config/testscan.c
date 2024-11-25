#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scan.h"
#include "parse.h"

int main(int argc, char **argv) {
    cscanstate state, *s = &state;
    configScanInit(s);
    configScanTarget(s, "port 3\n");
    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "3"));
    assert(configScanNext(s) == TCNL);

    configScanTarget(s, "forward 142.103.10.0/16 port 2\n");
    assert(configScanNext(s) == TCFORWARD);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "142"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "103"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "10"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCSLASH);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "16"));
    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "2"));
    assert(configScanNext(s) == TCNL);

    configScanTargetFile(s, "example");

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "1"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "2"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "3"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "4"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "5"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "6"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "7"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCFORWARD);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "142"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "103"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "10"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCSLASH);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "16"));
    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "2"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCFORWARD);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCSLASH);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCFORWARD);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "142"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "103"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "10"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCSLASH);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "17"));
    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "3"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCFORWARD);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "17"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCDOT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "0"));
    assert(configScanNext(s) == TCSLASH);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "8"));
    assert(configScanNext(s) == TCPORT);
    assert(configScanNext(s) == TCNUM);
    assert(!strcmp(configScanLexeme(s), "1"));
    assert(configScanNext(s) == TCNL);

    assert(configScanNext(s) == TCEOF);
    configScanTarget(s, "ack\n");
    assert(configScanNext(s) == TCERROR);
    return 0;
}
