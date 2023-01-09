#include <string.h>

void* memcpy( void *dest, const void *src, size_t count )
{
    while(count--)
    {
        ((unsigned char*)dest)[count] = ((const unsigned char*)src)[count];
    }
    return dest;
}

void* memset( void *dest, int ch, size_t count ){
    unsigned char *destB = (unsigned char*)dest;
    
    if (count >= 8)
    {
        unsigned int ch4 = (unsigned int)((unsigned char)ch << 24) | (unsigned int)((unsigned char)ch << 16) | (unsigned int)((unsigned char)ch << 8) | (unsigned char)ch;

        unsigned int *destW;
        size_t countW;

        /* set 0-3 bytes to align to 4 byte boundary */
        /* x & 3 == x % 4 */
        while((((size_t)destB) & 3)) 
        {   
            *(destB++) = (unsigned char)ch;
            --count;
        }

        /* set 4 bytes at once */
        /* x >> 2 == x / 4 */
        destW = (unsigned int*)destB;
        countW = count >> 2;
        while(countW--){
            *(destW++) = ch4;
        }

        /* leftover bytes*/
        count = count & 3;
        destB = (unsigned char*)destW;
    }
    
    /* set leftover bytes */
    while(count--){
        *(destB++) = (unsigned char)ch;
    }

    return dest;
}


int memcmp(const void *s1, const void *s2, size_t n)
{
    unsigned char *string1 = (unsigned char *)s1;
    unsigned char *string2 = (unsigned char *)s2;

    size_t i;
    for (i = 0; i < n; ++i)
    {
        if(string1[i] != string2[i])
        {
            return string1[i] < string2[-1] ? -1 : 1;
        }
    }
    return 0;
}


/*
 * Count the length of a string (without terminating null character).
 */
size_t strlen(const char *s) {
  const char *p;

  p = s;
  while (*p != '\0') {
    p++;
  }
  return p - s;
}


/*
 * Compare two strings.
 * Return a number < 0, = 0, or > 0 iff the first string is less
 * than, equal to, or greater than the second one, respectively.
 */
int strcmp(const char *s, const char *t) {
  while (*s == *t) {
    if (*s == '\0') {
      return 0;
    }
    s++;
    t++;
  }
  return *s - *t;
}


/*
 * Compare two strings, up to at most n bytes.
 * Return a number < 0, = 0, or > 0 iff the first string is less
 * than, equal to, or greater than the second one, respectively.
 */
int strncmp(const char *s, const char *t, size_t n) {
  while (*s == *t && n > 0) {
    if (*s == '\0') {
      return 0;
    }
    s++;
    t++;
    n--;
  }
  return n == 0 ? 0 : *s - *t;
}


/*
 * Copy string t to string s (includes terminating null character).
 */
char *strcpy(char *s, const char *t) {
  char *p;

  p = s;
  while ((*p = *t) != '\0') {
    p++;
    t++;
  }
  return s;
}


/*
 * Copy string t to string s, up to at most n bytes.
 */
char *strncpy(char *s, const char *t, size_t n) {
  char *p;

  p = s;
  while ((*p = *t) != '\0' && n > 0) {
    p++;
    t++;
    n--;
  }
  return s;
}
