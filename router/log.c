#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

static char **enabledChannels = NULL;
static char *prefix;

static int enabled(char *channel) {
    for (int i = 0; enabledChannels[i]; i++) {
        char *c = enabledChannels[i];
        if (!strcmp(c, channel)) return 1;
    }
    return 0;
}

long now() {
    struct timeval t;
    long ans;
    gettimeofday(&t, NULL);
    ans = t.tv_sec * 1000 + t.tv_usec / 1000;
    return ans;
}

static int countOccurances(char *s, char c) {
    int count = 0;
    while (*s) {
        if (*s == c) count++;
        s++;
    }
    return count;
}

static char *strsave(char *s) {
    char *ans = malloc(strlen(s) + 1);
    strcpy(ans, s);
    return ans;
}

static char *strnsave(char *s, int len) {
    char *ans = malloc(len + 1);
    strncpy(ans, s, len);
    ans[len] = '\0';
    return ans;
}

/*
 * prefixx is an arbitrary string that will be displayed at the beginning of
 * each logged message. 
 *
 * channels is a string containing a comma-separated list of words, each of
 * which defines a channel that is enabled:  messages logged with that
 * channel name will be displayed.  messages logged with any other channel
 * names will not be displayed.
 */
void logConfig(char *prefixx, char *channels) {
    prefix = strsave(prefixx);
    int len = countOccurances(channels, ',') + 1;
    enabledChannels = calloc(len + 1, sizeof(char *));
    int i = 0;
    while (*channels) {
        char *pos = index(channels, ',');
        if (pos == NULL) {
            len = strlen(channels);
        } else {
            len = pos - channels;
        }
        char *one = strnsave(channels, len);
        enabledChannels[i] = one;
        i++;
        channels += len + (pos == NULL ? 0 : 1);
    }
    enabledChannels[i] = NULL;
}

void logLog(char *channel, char *format, ...) {
    va_list al;

    if (enabled(channel)) {
        long t = now() % 100000000;
        printf("%4ld.%03ld %s: ", t / 1000, t % 1000, prefix);
        va_start(al, format);
        vprintf(format, al);
        va_end(al);
        putchar('\n');
    }
}

void logPerror(char *who) {
    const char *const error = strerror(errno);
    logLog("failure", "%s %s", who, error);
}
