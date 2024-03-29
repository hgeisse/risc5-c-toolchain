/*
 * string.h -- string operations
 */

#ifndef __STRING_H
#define __STRING_H

/*********************************************
 * NOTE: This is a not a full implementation 
 *       of string.h, just the bare minimum
 *********************************************/

#include <stddef.h>

void* memcpy( void *dest, const void *src, size_t count );
void *memset( void *dest, int ch, size_t count );
int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);

#endif //__STRING_H
