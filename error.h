#ifndef ERROR_H
#define ERROR_H

extern const char *progname;

void error(int status, int errnum, const char *fmt, ...);

#endif /* ERROR_H */
