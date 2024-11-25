#include <assert.h>
#include <stdint.h>
#include "ip.h"
#include "config/scan.h"
#include "config/parse.h"

void configLoad(char *filename, void (*addForwardEntry)(uint32_t network, int netlength, int interface), void (*addInterface)(int interface)) {
    cscanstate sstate, *ss = &sstate;
    cparsestate pstate, *s = &pstate;
    cparseline *l;

    configScanInit(ss);
    configScanTargetFile(ss, filename);
    configParseInit(s, ss);
    l = configParse(s);
    assert(l != NULL);
    while (l) {
	if (l->key == TCFORWARD) {
	    uint32_t ip = (l->rule.ip.part[0] << 24) +
		(l->rule.ip.part[1] << 16) +
		(l->rule.ip.part[2] << 8) +
		(l->rule.ip.part[3] << 0);
	    addForwardEntry(ip, l->rule.ip.netlength, l->rule.port);
	} else if (l->key == TCPORT) {
	    addInterface(l->port.port);
	}
	l = l->next;
    }
    configParsePrint(l);
    configScanClose(ss);
}
