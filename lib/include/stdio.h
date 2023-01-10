/*
 * stdio.h -- I/O functions
 */

#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>

#ifndef NULL
#define NULL	0
#endif

/*********************************************
 * NOTE: This is a not a full implementation 
 *       of stdio.h, just the bare minimum
 *********************************************/

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);

int vprintf(const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);

#endif /* __STDIO_H */
