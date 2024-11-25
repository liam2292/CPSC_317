#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scan.h"
#include "parse.h"


void configScanInit(cscanstate *s) {
    assert(!regcomp(&s->regex, "^(port|forward|\\.|/|[0-9]+|\n)", REG_EXTENDED));
}

void configScanTarget(cscanstate *s, char *input) {
    strcpy(s->line, input);
    s->infile = NULL;
    s->pos = 0;
}

void configScanClose(cscanstate *s) {
    regfree(&s->regex);
}

static int configScanFill(cscanstate *s);

// Return < 0 for error
int configScanTargetFile(cscanstate *s, char *fn) {
    s->infile = fopen(fn, "r");
    if (s->infile == NULL) return -1;
    s->line[0] = '\0';
    return configScanFill(s);
}

struct lexeme {
    char *l;
    int len;
    ctoken t;
} configLexemes[] = { { "port", 4, TCPORT },
		{ "forward", 7, TCFORWARD },
		{ ".", 1, TCDOT },
		{ "/", 1, TCSLASH },
		{ "\n", 1, TCNL } };

// Return < 0 if no more data
static int configScanFill(cscanstate *s) {
    if (s->infile != NULL) {
	char *ans = fgets(s->line, sizeof(s->line), s->infile);
	s->pos = 0;
	return ans == NULL ? -1 : 1;
    }
    return -1;
}
    
static int configScanEOL(cscanstate *s) {
    return s->line[s->pos] == '\0';
}

static void configScanSkip(cscanstate *s) {
    char c;
    while ((c = s->line[s->pos]) != '\0') {
	if (c == '\n' || !isspace(c)) break;
	s->pos++;
    }
    if (s->pos < sizeof(s->line) - 2 && s->line[s->pos] == '/' && s->line[s->pos + 1] == '/') {
	while (s->line[s->pos]) {
	    s->pos++;
	}
    }
}
    
ctoken configScanNext(cscanstate *s) {
    ctoken ans = TCEOF;
    configScanSkip(s);
    while (configScanEOL(s)) {
	if (configScanFill(s) >= 0) {
	    configScanSkip(s);
	} else {
	    return ans;
	}
    }
    if (!regexec(&s->regex, &s->line[s->pos], 10, s->matches, 0)) {
	int len = s->matches[0].rm_eo - s->matches[0].rm_so;
	if (isdigit(s->line[s->pos + s->matches[0].rm_so])) {
	    ans = TCNUM;
	} else {
	    for (int i = 0; i < sizeof(configLexemes) / sizeof(struct lexeme); i++) {
		if (len == configLexemes[i].len && !strncmp(&s->line[s->pos + s->matches[0].rm_so], configLexemes[i].l, len)) {
		    ans = configLexemes[i].t;
		    break;
		}
	    }
	}
	strncpy(s->lexeme, &s->line[s->pos + s->matches[0].rm_so], len);
	s->lexeme[len] = '\0';
	s->pos += s->matches[0].rm_eo;
	return ans;
    } else {
	return TCERROR;
    }
}

char *configScanLexeme(cscanstate *s) {
    return s->lexeme;
}
    
char *configTokenNames[] = {
    "port",
    "forward",
    ".",
    "/",
    "nl",
    "num",
    "eof",
    "error",
};

char *configTokenName(ctoken t) {
    return configTokenNames[t];
}
