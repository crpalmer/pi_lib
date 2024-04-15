#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "mem.h"
#include "string-utils.h"

char *
maprintf(const char *fmt, ...)
{
    int n;
    int size = 100;     /* Guess we need no more than 100 bytes. */
    char *p, *np;
    va_list ap;

    if ((p = fatal_malloc(size)) == NULL) return NULL;

    while (1) {

	/* Try to print in the allocated space. */

	va_start(ap, fmt);
	n = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	/* If that worked, return the string. */

	if (n > -1 && n < size) return p;

	/* Else try again with more space. */

	if (n > -1) {    /* glibc 2.1 */
	    size = n+1; /* precisely what is needed */
	} else {          /* glibc 2.0 */
	    size *= 2;  /* twice the old size */
	}

	if ((np = realloc (p, size)) == NULL) {
	    free(p);
	    return NULL;
	} else {
	    p = np;
	}
    }
}

