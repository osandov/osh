#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#ifndef DEFAULT_PROGRAM_NAME
#define DEFAULT_PROGRAM_NAME "osh"
#endif

const char *progname = DEFAULT_PROGRAM_NAME;

/* See error.h */
void error(int status, int errnum, const char *fmt, ...)
{
    fflush(stdout);
    fprintf(stderr, "%s", progname);
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, ": ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errnum)
        fprintf(stderr, ": %s", strerror(errnum));
    fprintf(stderr, "\n");
    if (status)
        exit(status);
}
