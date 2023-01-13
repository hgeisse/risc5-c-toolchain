/*
 * stdio.c -- I/O functions
 */


#include <stdio.h>
#include <stdint.h>


#define FLAG_ALT	0x01		/* alternate form of output */
#define FLAG_ZERO	0x02		/* pad with leading zeroes */
#define FLAG_PLUS	0x04		/* always print with a sign */
#define FLAG_LEFT	0x08		/* left adjustment within field */
#define FLAG_SPACE	0x10		/* if no sign, prefix a space */
#define FLAG_PREC	0x20		/* a precision was specified */
#define FLAG_CAPS	0x40		/* use A-F instead of a-f */

#define SIZE_NORMAL	1		/* no length modifier */
#define SIZE_SHORT	2		/* 'h' length modifier */
#define SIZE_LONG	3		/* 'l' length modifier */


/**************************************************************/


typedef struct {
  char *p;		/* buffer pointer - if NULL use function below */
  size_t n;		/* number of bytes free in buffer */
  void (*f)(int);	/* character output function */
} FILE;


/**************************************************************/


extern void putchar(int c);


static void putc(int c, FILE *stream) {
  if (stream->p != NULL) {
    /* use pointer to write character to buffer */
    if (stream->n > 0) {
      stream->n--;
      *(stream->p)++ = c;
    }
  } else {
    /* use more general character output function */
    (*stream->f)(c);
  }
}


/**************************************************************/


#define CHECK_PUTC(s)	if (s->p != NULL && s->n == 0) return -1


static int printString(char *str, int flags,
                       int width, int prec, FILE *stream) {
  int length;
  int i;

  /* determine the length of the string */
  length = 0;
  while (str[length] != '\0') {
    /* if a precision is given, it limits the number of chars */
    if (flags & FLAG_PREC) {
      if (length == prec) {
        break;
      }
    }
    length++;
  }
  /* pad to the left if necessary */
  if (!(flags & FLAG_LEFT)) {
    for (i = 0; i < width - length; i++) {
      CHECK_PUTC(stream);
      putc(' ', stream);
    }
  }
  /* print the string proper */
  for (i = 0; i < length; i++) {
    CHECK_PUTC(stream);
    putc(str[i], stream);
  }
  /* pad to the right if necessary */
  if (flags & FLAG_LEFT) {
    for (i = 0; i < width - length; i++) {
      CHECK_PUTC(stream);
      putc(' ', stream);
    }
  }
  /* return number of chars actually printed */
  return length >= width ? length : width;
}


static int printSigned(signed long snum, int flags,
                       int width, int prec, FILE *stream) {
  int isZero;
  int isNegative;
  int numDigits;
  int digits[10];
  int numZeroes;	/* number of leading zeroes */
  int length;		/* output length, without padding */
  int i;

  /* remember if number is equal to 0 */
  isZero = (snum == 0);
  /* remember if number is less than 0; if so, negate */
  isNegative = (snum < 0);
  if (isNegative) {
    snum = -snum;
  }
  /* if snum and prec are both zero, print nothing but padding spaces */
  if (isZero && (flags & FLAG_PREC) && prec == 0) {
    for (i = 0; i < width; i++) {
      CHECK_PUTC(stream);
      putc(' ', stream);
    }
    return width;
  }
  /* compute digits in reverse order */
  numDigits = 0;
  do {
    digits[numDigits++] = snum % 10;
    snum /= 10;
  } while (snum);
  /* determine the length of the string */
  numZeroes = 0;
  length = numDigits;
  /* if more digits are needed, these are zeroes */
  i = numZeroes + numDigits;
  if ((flags & FLAG_PREC) && prec > i) {
    numZeroes += prec - i;
    length += prec - i;
  }
  /* don't forget the sign */
  if (isNegative || (flags & (FLAG_PLUS | FLAG_SPACE))) {
    length++;
  }
  /* pad to the left if necessary */
  if (!(flags & FLAG_LEFT) && width > length) {
    if (flags & FLAG_ZERO) {
      /* pad with zeroes, after the sign */
      numZeroes += width - length;
    } else {
      /* pad with spaces, now */
      for (i = 0; i < width - length; i++) {
        CHECK_PUTC(stream);
        putc(' ', stream);
      }
    }
  }
  /* print the sign if necessary */
  if (isNegative) {
    CHECK_PUTC(stream);
    putc('-', stream);
  } else
  if (flags & FLAG_PLUS) {
    CHECK_PUTC(stream);
    putc('+', stream);
  } else
  if (flags & FLAG_SPACE) {
    CHECK_PUTC(stream);
    putc(' ', stream);
  }
  /* print the leading zeroes */
  for (i = 0; i < numZeroes; i++) {
    CHECK_PUTC(stream);
    putc('0', stream);
  }
  /* print the digits proper */
  while (numDigits--) {
    CHECK_PUTC(stream);
    putc('0' + digits[numDigits], stream);
  }
  /* pad to the right if necessary */
  if (flags & FLAG_LEFT) {
    for (i = 0; i < width - length; i++) {
      CHECK_PUTC(stream);
      putc(' ', stream);
    }
  }
  /* return number of chars actually printed */
  return width > length ? width : length;
}


static int printUnsigned(unsigned long unum, int base, int flags,
                         int width, int prec, FILE *stream) {
  static char *lowerCase = "0123456789abcdef";
  static char *upperCase = "0123456789ABCDEF";
  char *dp;
  int isZero;
  int numDigits;
  int digits[11];
  int numZeroes;	/* number of leading zeroes */
  int length;		/* output length, without padding */
  int i;

  /* choose upper or lower case hex digits */
  dp = flags & FLAG_CAPS ? upperCase : lowerCase;
  /* remember if number is equal to 0 */
  isZero = (unum == 0);
  /* if unum and prec are both zero, print nothing but padding spaces */
  if (isZero && (flags & FLAG_PREC) && prec == 0) {
    for (i = 0; i < width; i++) {
      CHECK_PUTC(stream);
      putc(' ', stream);
    }
    return width;
  }
  /* compute digits in reverse order */
  numDigits = 0;
  do {
    digits[numDigits++] = unum % base;
    unum /= base;
  } while (unum);
  /* determine the length of the string */
  numZeroes = 0;
  length = numDigits;
  /* consider alternate output form */
  if ((flags & FLAG_ALT) && !isZero) {
    if (base == 8) {
      /* prepend zero */
      numZeroes++;
      length++;
    } else
    if (base == 16) {
      /* prepend 0x or 0X */
      length += 2;
    }
  }
  /* if more digits are needed, these are zeroes */
  i = numZeroes + numDigits;
  if ((flags & FLAG_PREC) && prec > i) {
    numZeroes += prec - i;
    length += prec - i;
  }
  /* pad to the left if necessary */
  if (!(flags & FLAG_LEFT) && width > length) {
    if (flags & FLAG_ZERO) {
      /* pad with zeroes, after the prefix */
      numZeroes += width - length;
    } else {
      /* pad with spaces, now */
      for (i = 0; i < width - length; i++) {
        CHECK_PUTC(stream);
        putc(' ', stream);
      }
    }
  }
  /* print 0x or 0X if necessary */
  if ((flags & FLAG_ALT) && !isZero && base == 16) {
    CHECK_PUTC(stream);
    putc('0', stream);
    if (flags & FLAG_CAPS) {
      CHECK_PUTC(stream);
      putc('X', stream);
    } else {
      CHECK_PUTC(stream);
      putc('x', stream);
    }
  }
  /* print the leading zeroes */
  for (i = 0; i < numZeroes; i++) {
    CHECK_PUTC(stream);
    putc('0', stream);
  }
  /* print the digits proper */
  while (numDigits--) {
    CHECK_PUTC(stream);
    putc(dp[digits[numDigits]], stream);
  }
  /* pad to the right if necessary */
  if (flags & FLAG_LEFT) {
    for (i = 0; i < width - length; i++) {
      CHECK_PUTC(stream);
      putc(' ', stream);
    }
  }
  /* return number of chars actually printed */
  return width > length ? width : length;
}


static int doPrintf(FILE *stream, const char *format, va_list ap) {
  int count, auxCount;
  char c;
  int flags;
  int width;
  int prec;
  int size;
  char *str;
  signed long snum;
  unsigned long unum;
  double fnum;
  int base;

  count = 0;
  while ((c = *format++) != '\0') {
    /* conversion specifier? */
    if (c != '%') {
      /* no, output character verbatim */
      CHECK_PUTC(stream);
      putc(c, stream);
      count++;
      continue;
    }
    /* else, look one character ahead */
    c = *format++;
    /* set some defaults */
    flags = 0;
    width = 0;
    prec = 0;
    size = SIZE_NORMAL;
    /* first, the flags */
    while (c == '#' || c == '0' || c == '+' || c == '-' || c == ' ') {
      switch (c) {
        case '#':
          /* alternate output form */
          flags |= FLAG_ALT;
          break;
        case '0':
          /* pad numbers with zeroes */
          flags |= FLAG_ZERO;
          break;
        case '+':
          /* print numbers always with a sign */
          flags |= FLAG_PLUS;
          break;
        case '-':
          /* left adjustment */
          flags |= FLAG_LEFT;
          break;
        case ' ':
          /* prefix a space, if first char is not a sign */
          flags |= FLAG_SPACE;
          break;
      }
      c = *format++;
    }
    /* next, the width field */
    if (c == '*') {
      /* computed value */
      width = va_arg(ap, int);
      /* negative width means left adjustment */
      if (width < 0) {
        width = -width;
        flags |= FLAG_LEFT;
      }
      /* maintain one char lookahead */
      c = *format++;
    } else {
      /* constant value */
      while (c >= '0' && c <= '9') {
        width = width * 10 + (c - '0');
        c = *format++;
      }
    }
    /* then, the precision field */
    if (c == '.') {
      c = *format++;
      flags |= FLAG_PREC;
      if (c == '*') {
        /* computed value */
        prec = va_arg(ap, int);
        /* negative precision counts as 0 */
        if (prec < 0) {
          prec = 0;
        }
        /* maintain one char lookahead */
        c = *format++;
      } else {
        /* constant value */
        while (c >= '0' && c <= '9') {
          prec = prec * 10 + (c - '0');
          c = *format++;
        }
      }
    }
    /* now, the length modifier */
    if (c == 'h') {
      size = SIZE_SHORT;
      c = *format++;
    } else
    if (c == 'l') {
      size = SIZE_LONG;
      c = *format++;
    }
    /* finally, the conversion character */
    switch (c) {
      case '%':
        CHECK_PUTC(stream);
        putc(c, stream);
        count++;
        break;
      case 'c':
        c = va_arg(ap, char);
        CHECK_PUTC(stream);
        putc(c, stream);
        count++;
        break;
      case 's':
        str = va_arg(ap, char *);
        auxCount = printString(str, flags, width, prec, stream);
        if (auxCount < 0) {
          return -1;
        }
        count += auxCount;
        break;
      case 'd':
      case 'i':
        switch (size) {
          case SIZE_NORMAL:
            snum = va_arg(ap, int);
            break;
          case SIZE_SHORT:
            snum = va_arg(ap, short);
            break;
          case SIZE_LONG:
            snum = va_arg(ap, long);
            break;
        }
        auxCount = printSigned(snum, flags, width, prec, stream);
        if (auxCount < 0) {
          return -1;
        }
        count += auxCount;
        break;
      case 'u':
      case 'o':
      case 'x':
      case 'X':
        switch (c) {
          case 'u':
            base = 10;
            break;
          case 'o':
            base = 8;
            break;
          case 'x':
            base = 16;
            break;
          case 'X':
            base = 16;
            flags |= FLAG_CAPS;
            break;
        }
        switch (size) {
          case SIZE_NORMAL:
            unum = va_arg(ap, unsigned int);
            break;
          case SIZE_SHORT:
            unum = va_arg(ap, unsigned short);
            break;
          case SIZE_LONG:
            unum = va_arg(ap, unsigned long);
            break;
        }
        auxCount = printUnsigned(unum, base, flags, width, prec, stream);
        if (auxCount < 0) {
          return -1;
        }
        count += auxCount;
        break;
      case 'p':
        flags |= FLAG_ALT | FLAG_ZERO;
        unum = (unsigned long) va_arg(ap, void *);
        auxCount = printUnsigned(unum, 16, flags, 10, 0, stream);
        if (auxCount < 0) {
          return -1;
        }
        count += auxCount;
        break;
#if 0
      case 'f':
        fnum = va_arg(ap, double);
        auxCount = printFloat(fnum, flags, width, prec, stream);
        if (auxCount < 0) {
          return -1;
        }
        count += auxCount;
        break;
#endif
      default:
        /* unknown conversion type, return error */
        return -1;
    }
  }
  return count;
}


/**************************************************************/


int printf(const char *format, ...) {
  FILE stream;
  va_list ap;
  int count;

  stream.p = NULL;
  stream.n = 0;
  stream.f = putchar;
  va_start(ap, format);
  count = doPrintf(&stream, format, ap);
  va_end(ap);
  return count;
}


int sprintf(char *str, const char *format, ...) {
  FILE stream;
  va_list ap;
  int count;

  stream.p = str;
  stream.n = SIZE_MAX;
  stream.f = 0;
  va_start(ap, format);
  count = doPrintf(&stream, format, ap);
  va_end(ap);
  putc('\0', &stream);
  return count;
}


int snprintf(char *str, size_t size, const char *format, ...) {
  FILE stream;
  va_list ap;
  int count;

  stream.p = str;
  stream.n = size;
  stream.f = 0;
  va_start(ap, format);
  count = doPrintf(&stream, format, ap);
  va_end(ap);
  if (count < 0 || stream.n == 0) {
    /* insufficient string space, but must write delimiter */
    stream.p--;
    stream.n++;
    /* nevertheless, return string space overflow indicator */
    count = -1;
  }
  putc('\0', &stream);
  return count;
}


int vprintf(const char *format, va_list ap) {
  FILE stream;
  int count;

  stream.p = NULL;
  stream.n = 0;
  stream.f = putchar;
  count = doPrintf(&stream, format, ap);
  return count;
}


int vsprintf(char *str, const char *format, va_list ap) {
  FILE stream;
  int count;

  stream.p = str;
  stream.n = SIZE_MAX;
  stream.f = 0;
  count = doPrintf(&stream, format, ap);
  putc('\0', &stream);
  return count;
}


int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  FILE stream;
  int count;

  stream.p = str;
  stream.n = size;
  stream.f = 0;
  count = doPrintf(&stream, format, ap);
  if (count < 0 || stream.n == 0) {
    /* insufficient string space, but must write delimiter */
    stream.p--;
    stream.n++;
    /* nevertheless, return string space overflow indicator */
    count = -1;
  }
  putc('\0', &stream);
  return count;
}
