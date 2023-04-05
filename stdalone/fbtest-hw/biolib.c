/*
 * biolib.c -- basic I/O library
 */


#include "biolib.h"


char getc(void) {
  unsigned int *base;
  char c;

  base = (unsigned int *) 0xFFFFC0;
  while ((*(base + 3) & 1) == 0) ;
  c = *(base + 2);
  return c;
}


void putc(char c) {
  unsigned int *base;

  base = (unsigned int *) 0xFFFFC0;
  while ((*(base + 3) & 2) == 0) ;
  *(base + 2) = c;
}
