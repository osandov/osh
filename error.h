#ifndef ERROR_H
#define ERROR_H

/**
 * The name of the program to be used when printing error messages, set to a
 * default value
 */
extern const char *progname;

/**
 * Flush standard out and then print an error message consisting of the program
 * name followed by a colon, then, if errnum is non-zero, the string
 * corresponding to errnum (i.e., strerror), then, if format is non-null, the
 * corresponding printf-style * output. Then, if status is not zero, exit the
 * program with the given exit status
 */
void error(int status, int errnum, const char *format, ...);

#endif /* ERROR_H */
