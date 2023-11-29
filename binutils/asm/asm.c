/*
 * asm.c -- RISC5 assembler
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "../include/exec.h"


/**************************************************************/

/* constant definitions */


#define DFLT_OUT_NAME	"a.out"

#define LINE_SIZE	200


/**************************************************************/

/* tokens */


#define TOK_EOL		0
#define TOK_LABEL	1
#define TOK_IDENT	2
#define TOK_STRING	3
#define TOK_NUMBER	4
#define TOK_REGISTER	5
#define TOK_COMMA	6
#define TOK_LPAREN	7
#define TOK_RPAREN	8
#define TOK_PLUS	9
#define TOK_MINUS	10
#define TOK_TILDE	11
#define TOK_STAR	12
#define TOK_SLASH	13
#define TOK_PERCENT	14
#define TOK_LSHIFT	15
#define TOK_RSHIFT	16
#define TOK_AMPER	17
#define TOK_CARET	18
#define TOK_BAR		19


/**************************************************************/

/* opcodes */


#define OP_MOV		0x00000000
#define OP_MOVH		0x60000000
#define OP_PUTS		0x20000000
#define OP_GETS		0x30000000

#define OP_LSL		0x00010000
#define OP_ASR		0x00020000
#define OP_LSR		0x20020000
#define OP_ROR		0x00030000
#define OP_AND		0x00040000
#define OP_ANN		0x00050000
#define OP_IOR		0x00060000
#define OP_XOR		0x00070000
#define OP_ADD		0x00080000
#define OP_ADDC		0x20080000
#define OP_SUB		0x00090000
#define OP_SUBB		0x20090000
#define OP_MUL		0x000A0000
#define OP_MULU		0x200A0000
#define OP_DIV		0x000B0000
#define OP_DIVU		0x200B0000
#define OP_FAD		0x000C0000
#define OP_FSB		0x000D0000
#define OP_FML		0x000E0000
#define OP_FDV		0x000F0000
#define OP_FLR		0x100C0000
#define OP_FLT		0x200C0000

#define OP_LDW		0x80000000
#define OP_LDH		0x80000001
#define OP_LDB		0x90000000
#define OP_STW		0xA0000000
#define OP_STH		0xA0000001
#define OP_STB		0xB0000000

#define OP_BMI		0xC0000000
#define OP_BEQ		0xC1000000
#define OP_BCS		0xC2000000
#define OP_BVS		0xC3000000
#define OP_BLS		0xC4000000
#define OP_BLT		0xC5000000
#define OP_BLE		0xC6000000
#define OP_B		0xC7000000
#define OP_BPL		0xC8000000
#define OP_BNE		0xC9000000
#define OP_BCC		0xCA000000
#define OP_BVC		0xCB000000
#define OP_BHI		0xCC000000
#define OP_BGE		0xCD000000
#define OP_BGT		0xCE000000
#define OP_BNVR		0xCF000000

#define OP_CMI		0xD0000000
#define OP_CEQ		0xD1000000
#define OP_CCS		0xD2000000
#define OP_CVS		0xD3000000
#define OP_CLS		0xD4000000
#define OP_CLT		0xD5000000
#define OP_CLE		0xD6000000
#define OP_C		0xD7000000
#define OP_CPL		0xD8000000
#define OP_CNE		0xD9000000
#define OP_CCC		0xDA000000
#define OP_CVC		0xDB000000
#define OP_CHI		0xDC000000
#define OP_CGE		0xDD000000
#define OP_CGT		0xDE000000
#define OP_CNVR		0xDF000000

#define OP_RTI		0xC7000010
#define OP_CLI		0xCF000020
#define OP_STI		0xCF000021


/**************************************************************/

/* type definitions */


typedef enum { false, true } Bool;


#define STAT_DEFINED	0x01	/* symbol is defined */
#define STAT_GLOBAL	0x02	/* symbol is global */
#define STAT_USED	0x04	/* symbol is used by fixups */

#define SEG_ABS		-1	/* absolute values */
#define SEG_CODE	0	/* code segment */
#define SEG_DATA	1	/* initialized data segment */
#define SEG_BSS		2	/* uninitialized data segment */


typedef struct symbol {
  char *name;			/* name of symbol */
  int status;			/* status of symbol */
  int segment;			/* the symbol's segment, -1: absolute */
  int value;			/* the symbol's value */
  int number;			/* the symbol's number (only for output) */
  struct symbol *left;		/* left son in binary search tree */
  struct symbol *right;		/* right son in binary search tree */
} Symbol;


typedef struct fixup {
  int segment;			/* in which segment */
  unsigned int offset;		/* at which offset */
  int method;			/* what coding method is to be used */
  struct symbol *refSym;	/* which symbol is referenced */
  int refSeg;			/* alternatively, if sym = NULL: */
				/* which segment is referenced */
  int add;			/* additive part of value */
  struct fixup *next;		/* next fixup */
} Fixup;


/**************************************************************/

/* global variables */


Bool debugToken = false;
Bool debugEmit = false;
Bool debugFixup = false;
Bool debugIAS = false;

FILE *inFile;
FILE *outFile;

char line[LINE_SIZE];
char *lineptr;
int lineno;

int token;
char tokenvalString[LINE_SIZE];
int tokenvalNumber;

int currSeg = SEG_CODE;
unsigned int segPtr[3] = { 0, 0, 0 };

Symbol *symbolTable = NULL;


/**************************************************************/

/* error and memory handling */


void error(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  fprintf(stderr, "Error: ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}


void warning(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  fprintf(stderr, "Warning: ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}


void *memAlloc(unsigned int size) {
  void *p;

  p = malloc(size);
  if (p == NULL) {
    error("out of memory");
  }
  return p;
}


void memFree(void *p) {
  if (p == NULL) {
    error("memFree() got NULL pointer");
  }
  free(p);
}


/**************************************************************/

/* endianness conversion */


unsigned int read4FromTarget(unsigned char *p) {
  return (unsigned int) p[0] <<  0 |
         (unsigned int) p[1] <<  8 |
         (unsigned int) p[2] << 16 |
         (unsigned int) p[3] << 24;
}


void write4ToTarget(unsigned char *p, unsigned int data) {
  p[0] = data >>  0;
  p[1] = data >>  8;
  p[2] = data >> 16;
  p[3] = data >> 24;
}


void conv4FromTargetToNative(unsigned char *p) {
  unsigned int data;

  data = read4FromTarget(p);
  * (unsigned int *) p = data;
}


void conv4FromNativeToTarget(unsigned char *p) {
  unsigned int data;

  data = * (unsigned int *) p;
  write4ToTarget(p, data);
}


unsigned short read2FromTarget(unsigned char *p) {
  return (unsigned short) p[0] << 0 |
         (unsigned short) p[1] << 8;
}


void write2ToTarget(unsigned char *p, unsigned short data) {
  p[0] = data >> 0;
  p[1] = data >> 8;
}


void conv2FromTargetToNative(unsigned char *p) {
  unsigned short data;

  data = read2FromTarget(p);
  * (unsigned short *) p = data;
}


void conv2FromNativeToTarget(unsigned char *p) {
  unsigned short data;

  data = * (unsigned short *) p;
  write2ToTarget(p, data);
}


/**************************************************************/

/* other helper functions */


#define CODE_NAME	".code"
#define DATA_NAME	".data"
#define BSS_NAME	".bss"


char *segName(int segment) {
  char *name;

  switch (segment) {
    case SEG_ABS:
      name = "<abs>";
      break;
    case SEG_CODE:
      name = CODE_NAME;
      break;
    case SEG_DATA:
      name = DATA_NAME;
      break;
    case SEG_BSS:
      name = BSS_NAME;
      break;
    default:
      name = "<unknown>";
      break;
  }
  return name;
}


#define H16_NAME	"H16"
#define L16_NAME	"L16"
#define R22_NAME	"R22"
#define W32_NAME	"W32"


char *methodName(int method) {
  char *name;

  switch (method) {
    case RELOC_H16:
      name = H16_NAME;
      break;
    case RELOC_L16:
      name = L16_NAME;
      break;
    case RELOC_R22:
      name = R22_NAME;
      break;
    case RELOC_W32:
      name = W32_NAME;
      break;
    default:
      name = "<unknown>";
      break;
  }
  return name;
}


/**************************************************************/

/* symbol table */


Symbol *newSymbol(char *name) {
  Symbol *p;

  p = memAlloc(sizeof(Symbol));
  p->name = memAlloc(strlen(name) + 1);
  strcpy(p->name, name);
  p->status = 0;
  p->segment = SEG_ABS;
  p->value = 0;
  p->number = -1;
  p->left = NULL;
  p->right = NULL;
  return p;
}


Symbol *lookupEnter(char *name) {
  Symbol *p, *q, *r;
  int cmp;

  p = symbolTable;
  if (p == NULL) {
    /* the very first symbol */
    r = newSymbol(name);
    symbolTable = r;
    return r;
  }
  /* try to look up symbol in binary search tree */
  while (1) {
    q = p;
    cmp = strcmp(name, q->name);
    if (cmp == 0) {
      /* found */
      return q;
    }
    if (cmp < 0) {
      p = q->left;
    } else {
      p = q->right;
    }
    if (p == NULL) {
      /* symbol is not in tree, enter */
      r = newSymbol(name);
      if (cmp < 0) {
        q->left = r;
      } else {
        q->right = r;
      }
      return r;
    }
  }
  /* never reached */
  return NULL;
}


void walkSymbolTree(Symbol *s, void (*fp)(Symbol *sp)) {
  if (s == NULL) {
    return;
  }
  walkSymbolTree(s->left, fp);
  (*fp)(s);
  walkSymbolTree(s->right, fp);
}


void walkSymbolTable(void (*fp)(Symbol *sp)) {
  walkSymbolTree(symbolTable, fp);
}


/**************************************************************/

/* fixups */


Fixup *fixupList = NULL;
Fixup *fixupLast;


void addFixup(int segment, unsigned int offset,
              int method, Symbol *refSym, int add) {
  Fixup *f;

  if (debugFixup) {
    printf("DEBUG: addFixup (segment: %s, offset: %08X,\n"
           "                 method: %s, refSym: '%s', add: %08X)\n",
           segName(segment), offset,
           methodName(method), refSym->name, add);
  }
  f = memAlloc(sizeof(Fixup));
  f->segment = segment;
  f->offset = offset;
  f->method = method;
  f->refSym = refSym;
  f->refSeg = -1;
  f->add = add;
  /* append fixup to list (prepending should also be OK) */
  f->next = NULL;
  if (fixupList == NULL) {
    fixupList = f;
  } else {
    fixupLast->next = f;
  }
  fixupLast = f;
  /* mark symbol as used */
  refSym->status |= STAT_USED;
}


void doFixup(Fixup *f) {
  Symbol *refSym;

  if (debugFixup) {
    printf("DEBUG: doFixup (segment: %s, offset: %08X,\n"
           "                method: %s, refSym: '%s', add: %08X)\n",
           segName(f->segment), f->offset,
           methodName(f->method), f->refSym->name, f->add);
  }
  refSym = f->refSym;
  if ((refSym->status & STAT_GLOBAL) == 0) {
    /* this is a local symbol */
    if ((refSym->status & STAT_DEFINED) == 0) {
      /* at this point, local symbols have to be defined */
      error("undefined symbol '%s'", refSym->name);
    }
    /* mutate the symbol-based fixup into a segment-based relocation */
    f->refSym = NULL;
    f->refSeg = refSym->segment;
    f->add += refSym->value;
  } else {
    /* this is a global symbol: nothing to do here */
  }
}


void doFixups(void) {
  Fixup *f;

  f = fixupList;
  while (f != NULL) {
    doFixup(f);
    f = f->next;
  }
}


/**************************************************************/

/* code array management */


#define MAX_CODE_INIT		256
#define MAX_CODE_MULT		4


unsigned char *codeArray = NULL;	/* the code proper */
unsigned int codeSize = 0;		/* the current code size */
unsigned int codeMaxSize = 0;		/* the code array's current size */


void growCodeArray(void) {
  unsigned int newMaxSize;
  unsigned char *newCodeArray;
  unsigned int i;

  if (codeMaxSize == 0) {
    /* first allocation */
    newMaxSize = MAX_CODE_INIT;
  } else {
    /* subsequent allocation */
    newMaxSize = codeMaxSize * MAX_CODE_MULT;
  }
  newCodeArray = memAlloc(newMaxSize);
  for (i = 0; i < codeSize; i++) {
    newCodeArray[i] = codeArray[i];
  }
  if (codeArray != NULL) {
    memFree(codeArray);
  }
  codeArray = newCodeArray;
  codeMaxSize = newMaxSize;
}


void putCodeWord(unsigned int data) {
  if (codeSize + 4 > codeMaxSize) {
    growCodeArray();
  }
  write4ToTarget(codeArray + codeSize, data);
  codeSize += 4;
}


void putCodeHalf(unsigned short data) {
  if (codeSize + 2 > codeMaxSize) {
    growCodeArray();
  }
  write2ToTarget(codeArray + codeSize, data);
  codeSize += 2;
}


void putCodeByte(unsigned char data) {
  if (codeSize + 1 > codeMaxSize) {
    growCodeArray();
  }
  *(codeArray + codeSize) = data;
  codeSize += 1;
}


void writeCodeBytes(void) {
  int i;

  for (i = 0; i < codeSize; i++) {
    fputc(codeArray[i], outFile);
  }
}


/**************************************************************/

/* data array management */


#define MAX_DATA_INIT		256
#define MAX_DATA_MULT		4


unsigned char *dataArray = NULL;	/* the data proper */
unsigned int dataSize = 0;		/* the current data size */
unsigned int dataMaxSize = 0;		/* the data array's current size */


void growDataArray(void) {
  unsigned int newMaxSize;
  unsigned char *newDataArray;
  unsigned int i;

  if (dataMaxSize == 0) {
    /* first allocation */
    newMaxSize = MAX_DATA_INIT;
  } else {
    /* subsequent allocation */
    newMaxSize = dataMaxSize * MAX_DATA_MULT;
  }
  newDataArray = memAlloc(newMaxSize);
  for (i = 0; i < dataSize; i++) {
    newDataArray[i] = dataArray[i];
  }
  if (dataArray != NULL) {
    memFree(dataArray);
  }
  dataArray = newDataArray;
  dataMaxSize = newMaxSize;
}


void putDataWord(unsigned int data) {
  if (dataSize + 4 > dataMaxSize) {
    growDataArray();
  }
  write4ToTarget(dataArray + dataSize, data);
  dataSize += 4;
}


void putDataHalf(unsigned short data) {
  if (dataSize + 2 > dataMaxSize) {
    growDataArray();
  }
  write2ToTarget(dataArray + dataSize, data);
  dataSize += 2;
}


void putDataByte(unsigned char data) {
  if (dataSize + 1 > dataMaxSize) {
    growDataArray();
  }
  *(dataArray + dataSize) = data;
  dataSize += 1;
}


void writeDataBytes(void) {
  int i;

  for (i = 0; i < dataSize; i++) {
    fputc(dataArray[i], outFile);
  }
}


/**************************************************************/

/* code and data emitter */


void emitWord(unsigned int word) {
  if (debugEmit) {
    printf("DEBUG: word @ segment = %s, offset = %08X",
           segName(currSeg), segPtr[currSeg]);
    printf(", value = %02X%02X%02X%02X\n",
           (word >> 24) & 0xFF, (word >> 16) & 0xFF,
           (word >> 8) & 0xFF, word & 0xFF);
  }
  switch (currSeg) {
    case SEG_ABS:
      error("illegal segment in emitWord()");
      break;
    case SEG_CODE:
      putCodeWord(word);
      break;
    case SEG_DATA:
      putDataWord(word);
      break;
    case SEG_BSS:
      break;
  }
  segPtr[currSeg] += 4;
}


void emitHalf(unsigned int half) {
  half &= 0x0000FFFF;
  if (debugEmit) {
    printf("DEBUG: half @ segment = %s, offset = %08X",
           segName(currSeg), segPtr[currSeg]);
    printf(", value = %02X%02X\n",
           (half >> 8) & 0xFF, half & 0xFF);
  }
  switch (currSeg) {
    case SEG_ABS:
      error("illegal segment in emitHalf()");
      break;
    case SEG_CODE:
      putCodeHalf(half);
      break;
    case SEG_DATA:
      putDataHalf(half);
      break;
    case SEG_BSS:
      break;
  }
  segPtr[currSeg] += 2;
}


void emitByte(unsigned int byte) {
  byte &= 0x000000FF;
  if (debugEmit) {
    printf("DEBUG: byte @ segment = %s, offset = %08X",
           segName(currSeg), segPtr[currSeg]);
    printf(", value = %02X\n", byte);
  }
  switch (currSeg) {
    case SEG_ABS:
      error("illegal segment in emitByte()");
      break;
    case SEG_CODE:
      putCodeByte(byte);
      break;
    case SEG_DATA:
      putDataByte(byte);
      break;
    case SEG_BSS:
      break;
  }
  segPtr[currSeg] += 1;
}


/**************************************************************/

/* scanner */


Bool isReg(char *str) {
  int num;

  if (*str != 'R') {
    return false;
  }
  num = 0;
  str++;
  do {
    if (!isdigit(*str)) {
      return false;
    }
    num *= 10;
    num += (*str - '0');
    str++;
  } while (*str != '\0');
  if (num < 0 || num > 15) {
    error("register number out of bounds in line %d", lineno);
  }
  tokenvalNumber = num;
  return true;
}


int getNextToken(void) {
  char *p;
  int base;
  int digit;

  while (*lineptr == ' ' || *lineptr == '\t') {
    lineptr++;
  }
  if (*lineptr == '\n' || *lineptr == '\0' ||
      (lineptr[0] == '/' && lineptr[1] == '/')) {
    return TOK_EOL;
  }
  if (isalpha((int) *lineptr) || *lineptr == '_' || *lineptr == '.') {
    p = tokenvalString;
    while (isalnum((int) *lineptr) || *lineptr == '_' || *lineptr == '.') {
      *p++ = *lineptr++;
    }
    *p = '\0';
    if (*lineptr == ':') {
      lineptr++;
      return TOK_LABEL;
    } else {
      if (isReg(tokenvalString)) {
        return TOK_REGISTER;
      } else {
        return TOK_IDENT;
      }
    }
  }
  if (isdigit((int) *lineptr)) {
    base = 10;
    tokenvalNumber = 0;
    if (*lineptr == '0') {
      lineptr++;
      if (*lineptr == 'x' || *lineptr == 'X') {
        base = 16;
        lineptr++;
      } else
      if (isdigit((int) *lineptr)) {
        base = 8;
      } else {
        return TOK_NUMBER;
      }
    }
    while (isxdigit((int) *lineptr)) {
      digit = *lineptr++ - '0';
      if (digit >= 'A' - '0') {
        if (digit >= 'a' - '0') {
          digit += '0' - 'a' + 10;
        } else {
          digit += '0' - 'A' + 10;
        }
      }
      if (digit >= base) {
        error("illegal digit value %d in line %d", digit, lineno);
      }
      tokenvalNumber *= base;
      tokenvalNumber += digit;
    }
    return TOK_NUMBER;
  }
  if (*lineptr == '\'') {
    lineptr++;
    if (!isprint((int) *lineptr)) {
      error("cannot quote character 0x%02X in line %d", *lineptr, lineno);
    }
    tokenvalNumber = *lineptr;
    lineptr++;
    if (*lineptr != '\'') {
      error("unbalanced quote in line %d", lineno);
    }
    lineptr++;
    return TOK_NUMBER;
  }
  if (*lineptr == '\"') {
    lineptr++;
    p = tokenvalString;
    while (1) {
      if (*lineptr == '\n' || *lineptr == '\0') {
        error("unterminated string constant in line %d", lineno);
      }
      if (!isprint((int) *lineptr)) {
        error("string contains illegal character 0x%02X in line %d",
              *lineptr, lineno);
      }
      if (*lineptr == '\"') {
        break;
      }
      *p++ = *lineptr++;
    }
    lineptr++;
    *p = '\0';
    return TOK_STRING;
  }
  if (*lineptr == '<' && *(lineptr + 1) == '<') {
    lineptr += 2;
    return TOK_LSHIFT;
  }
  if (*lineptr == '>' && *(lineptr + 1) == '>') {
    lineptr += 2;
    return TOK_RSHIFT;
  }
  switch (*lineptr) {
    case ',':
      lineptr++;
      return TOK_COMMA;
    case '(':
      lineptr++;
      return TOK_LPAREN;
    case ')':
      lineptr++;
      return TOK_RPAREN;
    case '+':
      lineptr++;
      return TOK_PLUS;
    case '-':
      lineptr++;
      return TOK_MINUS;
    case '~':
      lineptr++;
      return TOK_TILDE;
    case '*':
      lineptr++;
      return TOK_STAR;
    case '/':
      lineptr++;
      return TOK_SLASH;
    case '%':
      lineptr++;
      return TOK_PERCENT;
    case '&':
      lineptr++;
      return TOK_AMPER;
    case '^':
      lineptr++;
      return TOK_CARET;
    case '|':
      lineptr++;
      return TOK_BAR;
  }
  /* no match */
  error("illegal character 0x%02X in line %d", *lineptr, lineno);
  /* not reached */
  return 0;
}


void showToken(void) {
  printf("DEBUG: line = %d, ", lineno);
  switch (token) {
    case TOK_EOL:
      printf("token = TOK_EOL\n");
      break;
    case TOK_LABEL:
      printf("token = TOK_LABEL, value = '%s'\n", tokenvalString);
      break;
    case TOK_IDENT:
      printf("token = TOK_IDENT, value = '%s'\n", tokenvalString);
      break;
    case TOK_STRING:
      printf("token = TOK_STRING, value = '%s'\n", tokenvalString);
      break;
    case TOK_NUMBER:
      printf("token = TOK_NUMBER, value = 0x%08X\n", tokenvalNumber);
      break;
    case TOK_REGISTER:
      printf("token = TOK_REGISTER, value = %d\n", tokenvalNumber);
      break;
    case TOK_COMMA:
      printf("token = TOK_COMMA\n");
      break;
    case TOK_LPAREN:
      printf("token = TOK_LPAREN\n");
      break;
    case TOK_RPAREN:
      printf("token = TOK_RPAREN\n");
      break;
    case TOK_PLUS:
      printf("token = TOK_PLUS\n");
      break;
    case TOK_MINUS:
      printf("token = TOK_MINUS\n");
      break;
    case TOK_TILDE:
      printf("token = TOK_TILDE\n");
      break;
    case TOK_STAR:
      printf("token = TOK_STAR\n");
      break;
    case TOK_SLASH:
      printf("token = TOK_SLASH\n");
      break;
    case TOK_PERCENT:
      printf("token = TOK_PERCENT\n");
      break;
    case TOK_LSHIFT:
      printf("token = TOK_LSHIFT\n");
      break;
    case TOK_RSHIFT:
      printf("token = TOK_RSHIFT\n");
      break;
    case TOK_AMPER:
      printf("token = TOK_AMPER\n");
      break;
    case TOK_CARET:
      printf("token = TOK_CARET\n");
      break;
    case TOK_BAR:
      printf("token = TOK_BAR\n");
      break;
    default:
      error("illegal token %d in showToken()", token);
  }
}


void getToken(void) {
  token = getNextToken();
  if (debugToken) {
    showToken();
  }
}


char *tok2str[] = {
  "end-of-line",
  "label",
  "identifier",
  "string",
  "number",
  "register",
  ",",
  "(",
  ")",
  "+",
  "-",
  "~",
  "*",
  "/",
  "%",
  "<<",
  ">>",
  "&",
  "^",
  "|",
};


void expect(int expected) {
  if (token != expected) {
    error("%s expected, got %s in line %d",
          tok2str[expected], tok2str[token], lineno);
  }
}


/**************************************************************/

/* expression parser */


typedef struct {
  int con;
  Symbol *sym;
} Value;


Value parseExpression(void);


Value parsePrimaryExpression(void) {
  Value v;
  Symbol *s;

  if (token == TOK_NUMBER) {
    v.con = tokenvalNumber;
    v.sym = NULL;
    getToken();
  } else
  if (token == TOK_IDENT) {
    s = lookupEnter(tokenvalString);
    if ((s->status & STAT_DEFINED) != 0 && s->segment == SEG_ABS) {
      v.con = s->value;
      v.sym = NULL;
    } else {
      v.con = 0;
      v.sym = s;
    }
    getToken();
  } else
  if (token == TOK_LPAREN) {
    getToken();
    v = parseExpression();
    expect(TOK_RPAREN);
    getToken();
  } else {
    error("illegal primary expression, line %d", lineno);
  }
  return v;
}


Value parseUnaryExpression(void) {
  Value v;

  if (token == TOK_PLUS) {
    getToken();
    v = parseUnaryExpression();
  } else
  if (token == TOK_MINUS) {
    getToken();
    v = parseUnaryExpression();
    if (v.sym != NULL) {
      error("cannot negate symbol '%s' in line %d", v.sym->name, lineno);
    }
    v.con = -v.con;
  } else
  if (token == TOK_TILDE) {
    getToken();
    v = parseUnaryExpression();
    if (v.sym != NULL) {
      error("cannot complement symbol '%s' in line %d", v.sym->name, lineno);
    }
    v.con = ~v.con;
  } else {
    v = parsePrimaryExpression();
  }
  return v;
}


Value parseMultiplicativeExpression(void) {
  Value v1, v2;

  v1 = parseUnaryExpression();
  while (token == TOK_STAR || token == TOK_SLASH || token == TOK_PERCENT) {
    if (token == TOK_STAR) {
      getToken();
      v2 = parseUnaryExpression();
      if (v1.sym != NULL || v2.sym != NULL) {
        error("multiplication of symbols not supported, line %d", lineno);
      }
      v1.con *= v2.con;
    } else
    if (token == TOK_SLASH) {
      getToken();
      v2 = parseUnaryExpression();
      if (v1.sym != NULL || v2.sym != NULL) {
        error("division of symbols not supported, line %d", lineno);
      }
      if (v2.con == 0) {
        error("division by zero, line %d", lineno);
      }
      v1.con /= v2.con;
    } else
    if (token == TOK_PERCENT) {
      getToken();
      v2 = parseUnaryExpression();
      if (v1.sym != NULL || v2.sym != NULL) {
        error("division of symbols not supported, line %d", lineno);
      }
      if (v2.con == 0) {
        error("division by zero, line %d", lineno);
      }
      v1.con %= v2.con;
    }
  }
  return v1;
}


Value parseAdditiveExpression(void) {
  Value v1, v2;

  v1 = parseMultiplicativeExpression();
  while (token == TOK_PLUS || token == TOK_MINUS) {
    if (token == TOK_PLUS) {
      getToken();
      v2 = parseMultiplicativeExpression();
      if (v1.sym != NULL && v2.sym != NULL) {
        error("addition of symbols not supported, line %d", lineno);
      }
      if (v2.sym != NULL) {
        v1.sym = v2.sym;
      }
      v1.con += v2.con;
    } else
    if (token == TOK_MINUS) {
      getToken();
      v2 = parseMultiplicativeExpression();
      if (v2.sym != NULL) {
        error("subtraction of symbols not supported, line %d", lineno);
      }
      v1.con -= v2.con;
    }
  }
  return v1;
}


Value parseShiftExpression(void) {
  Value v1, v2;

  v1 = parseAdditiveExpression();
  while (token == TOK_LSHIFT || token == TOK_RSHIFT) {
    if (token == TOK_LSHIFT) {
      getToken();
      v2 = parseAdditiveExpression();
      if (v1.sym != NULL || v2.sym != NULL) {
        error("shifting of symbols not supported, line %d", lineno);
      }
      v1.con = (unsigned) v1.con << v2.con;
    } else
    if (token == TOK_RSHIFT) {
      getToken();
      v2 = parseAdditiveExpression();
      if (v1.sym != NULL || v2.sym != NULL) {
        error("shifting of symbols not supported, line %d", lineno);
      }
      v1.con = (unsigned) v1.con >> v2.con;
    }
  }
  return v1;
}


Value parseAndExpression(void) {
  Value v1, v2;

  v1 = parseShiftExpression();
  while (token == TOK_AMPER) {
    getToken();
    v2 = parseShiftExpression();
    if (v2.sym != NULL) {
      error("bitwise 'and' of symbols not supported, line %d", lineno);
    }
    v1.con &= v2.con;
  }
  return v1;
}


Value parseExclusiveOrExpression(void) {
  Value v1, v2;

  v1 = parseAndExpression();
  while (token == TOK_CARET) {
    getToken();
    v2 = parseAndExpression();
    if (v2.sym != NULL) {
      error("bitwise 'xor' of symbols not supported, line %d", lineno);
    }
    v1.con ^= v2.con;
  }
  return v1;
}


Value parseInclusiveOrExpression(void) {
  Value v1, v2;

  v1 = parseExclusiveOrExpression();
  while (token == TOK_BAR) {
    getToken();
    v2 = parseExclusiveOrExpression();
    if (v2.sym != NULL) {
      error("bitwise 'or' of symbols not supported, line %d", lineno);
    }
    v1.con |= v2.con;
  }
  return v1;
}


Value parseExpression(void) {
  Value v;

  v = parseInclusiveOrExpression();
  return v;
}


/**************************************************************/

/* assemblers for the different formats */


#define Q_BIT		0x40000000		/* immediate operand bit */
#define U_BIT		0x20000000		/* branch rel distance bit */
#define V_BIT		0x10000000		/* filler for high 16 bits */
#define OP_IORI		(OP_IOR | Q_BIT)	/* inclusive OR immediate */
#define AUX		12			/* aux assembler register */


/*
 * format_0 operands: register, register or immediate
 */
void format_0(unsigned int code) {
  int reg1;
  int reg2;
  Value v;
  unsigned int imm;

  expect(TOK_REGISTER);
  reg1 = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  if (token == TOK_REGISTER) {
    reg2 = tokenvalNumber;
    getToken();
    emitWord(code | (reg1 << 24) | reg2);
  } else {
    v = parseExpression();
    imm = (unsigned) v.con;
    if (v.sym == NULL) {
      if ((imm & 0xFFFF0000) == 0x00000000) {
        /* set q-bit only */
        code |= Q_BIT;
        emitWord(code | (reg1 << 24) | (imm & 0x0000FFFF));
      } else
      if ((imm & 0xFFFF0000) == 0xFFFF0000) {
        /* set q-bit and v-bit */
        code |= Q_BIT | V_BIT;
        emitWord(code | (reg1 << 24) | (imm & 0x0000FFFF));
      } else {
        /* must synthesize the value */
        emitWord(OP_MOVH | (reg1 << 24) | (imm >> 16));
        if ((imm & 0x0000FFFF) != 0x00000000) {
          emitWord(OP_IORI | (reg1 << 24) | (reg1 << 20) | (imm & 0x0000FFFF));
        }
      }
    } else {
      /* must synthesize the value in any case */
      addFixup(currSeg, segPtr[currSeg], RELOC_H16, v.sym, v.con);
      emitWord(OP_MOVH | (reg1 << 24));
      addFixup(currSeg, segPtr[currSeg], RELOC_L16, v.sym, v.con);
      emitWord(OP_IORI | (reg1 << 24) | (reg1 << 20));
    }
  }
}


/*
 * format_1 operands: register, immediate
 * ATTENTION: high-order 16 bits encoded in instruction
 */
void format_1(unsigned int code) {
  int reg;
  Value v;
  unsigned int imm;

  expect(TOK_REGISTER);
  reg = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  v = parseExpression();
  imm = (unsigned) v.con;
  if (v.sym == NULL) {
    emitWord(code | (reg << 24) | (imm >> 16));
  } else {
    addFixup(currSeg, segPtr[currSeg], RELOC_H16, v.sym, v.con);
    emitWord(code | (reg << 24));
  }
}


/*
 * format_2 operands: register, special register number
 */
void format_2(unsigned int code) {
  int reg;
  Value v;
  unsigned int imm;

  expect(TOK_REGISTER);
  reg = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  v = parseExpression();
  if (v.sym != NULL) {
    error("absolute expression expected in line %d", lineno);
  }
  imm = (unsigned) v.con;
  if (imm > 15) {
    error("special register number out of bounds in line %d", lineno);
  }
  emitWord(code | (reg << 24) | imm);
}


/*
 * format_3 operands: register, register, register or immediate
 */
void format_3(unsigned int code) {
  int reg1;
  int reg2;
  int reg3;
  Value v;
  unsigned int imm;

  expect(TOK_REGISTER);
  reg1 = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  expect(TOK_REGISTER);
  reg2 = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  if (token == TOK_REGISTER) {
    reg3 = tokenvalNumber;
    getToken();
    emitWord(code | (reg1 << 24) | (reg2 << 20) | reg3);
  } else {
    v = parseExpression();
    imm = (unsigned) v.con;
    if (v.sym == NULL) {
      if ((imm & 0xFFFF0000) == 0x00000000) {
        /* set q-bit only */
        code |= Q_BIT;
        emitWord(code | (reg1 << 24) | (reg2 << 20) | (imm & 0x0000FFFF));
      } else
      if ((imm & 0xFFFF0000) == 0xFFFF0000) {
        /* set q-bit and v-bit */
        code |= Q_BIT | V_BIT;
        emitWord(code | (reg1 << 24) | (reg2 << 20) | (imm & 0x0000FFFF));
      } else {
        /* must synthesize the value */
        emitWord(OP_MOVH | (AUX << 24) | (imm >> 16));
        if ((imm & 0x0000FFFF) != 0x00000000) {
          emitWord(OP_IORI | (AUX << 24) | (AUX << 20) | (imm & 0x0000FFFF));
        }
        emitWord(code | (reg1 << 24) | (reg2 << 20) | AUX);
      }
    } else {
      /* must synthesize the value in any case */
      addFixup(currSeg, segPtr[currSeg], RELOC_H16, v.sym, v.con);
      emitWord(OP_MOVH | (AUX << 24));
      addFixup(currSeg, segPtr[currSeg], RELOC_L16, v.sym, v.con);
      emitWord(OP_IORI | (AUX << 24) | (AUX << 20));
      emitWord(code | (reg1 << 24) | (reg2 << 20) | AUX);
    }
  }
}


/*
 * format_4 operands: register, register
 */
void format_4(unsigned int code) {
  int reg1;
  int reg2;

  expect(TOK_REGISTER);
  reg1 = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  expect(TOK_REGISTER);
  reg2 = tokenvalNumber;
  getToken();
  emitWord(code | (reg1 << 24) | (reg2 << 20));
}


/*
 * format_5 operands: register, register, offset
 */
void format_5(unsigned int code) {
  int reg1;
  int reg2;
  Value v;
  unsigned int off;

  expect(TOK_REGISTER);
  reg1 = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  expect(TOK_REGISTER);
  reg2 = tokenvalNumber;
  getToken();
  expect(TOK_COMMA);
  getToken();
  v = parseExpression();
  off = v.con;
  if (v.sym == NULL) {
    if (((int) off) >= -(1 << 19) && ((int) off) < (1 << 19)) {
      emitWord(code | (reg1 << 24) | (reg2 << 20) | (off & 0x000FFFFF));
    } else {
      /* must synthesize the address */
      emitWord(OP_MOVH | (AUX << 24) | (off >> 16));
      if ((off & 0x0000FFFF) != 0x00000000) {
        emitWord(OP_IORI | (AUX << 24) | (AUX << 20) | (off & 0x0000FFFF));
      }
      emitWord(OP_ADD | (AUX << 24) | (AUX << 20) | reg2);
      emitWord(code | (reg1 << 24) | (AUX << 20));
    }
  } else {
    /* must synthesize the address in any case */
    addFixup(currSeg, segPtr[currSeg], RELOC_H16, v.sym, v.con);
    emitWord(OP_MOVH | (AUX << 24));
    addFixup(currSeg, segPtr[currSeg], RELOC_L16, v.sym, v.con);
    emitWord(OP_IORI | (AUX << 24) | (AUX << 20));
    emitWord(OP_ADD | (AUX << 24) | (AUX << 20) | reg2);
    emitWord(code | (reg1 << 24) | (AUX << 20));
  }
}


/*
 * format_6 operand: register or target address
 */
void format_6(unsigned int code) {
  int reg;
  Value v;
  unsigned int tgt;

  if (token == TOK_REGISTER) {
    reg = tokenvalNumber;
    getToken();
    emitWord(code | reg);
  } else {
    v = parseExpression();
    tgt = v.con;
    if (v.sym == NULL) {
      /* numeric target */
      emitWord(OP_MOVH | (AUX << 24) | (tgt >> 16));
      if ((tgt & 0x0000FFFF) != 0x00000000) {
        emitWord(OP_IORI | (AUX << 24) | (AUX << 20) | (tgt & 0x0000FFFF));
      }
      emitWord(code | AUX);
    } else {
      /* symbolic target */
      addFixup(currSeg, segPtr[currSeg], RELOC_R22, v.sym, v.con);
      emitWord(code | U_BIT);
    }
  }
}


/*
 * format_7 operands: none
 */
void format_7(unsigned int code) {
  emitWord(code);
}


/**************************************************************/

/* assemblers for the directives */


int countBits(unsigned int x) {
  int n;

  n = 0;
  while (x != 0) {
    x &= x - 1;
    n++;
  }
  return n;
}


/*
 * .CODE
 * set the current segment to code
 */
void dotCode(unsigned int code) {
  currSeg = SEG_CODE;
}


/*
 * .DATA
 * set the current segment to data
 */
void dotData(unsigned int code) {
  currSeg = SEG_DATA;
}


/*
 * .BSS
 * set the current segment to bss
 */
void dotBss(unsigned int code) {
  currSeg = SEG_BSS;
}


/*
 * .GLOBAL <comma-separated list of names>
 * make the names globally accessible
 * works in both directions (import and export)
 */
void dotGlobal(unsigned int code) {
  Symbol *symbol;

  while (1) {
    expect(TOK_IDENT);
    symbol = lookupEnter(tokenvalString);
    symbol->status |= STAT_GLOBAL;
    getToken();
    if (token != TOK_COMMA) {
      break;
    }
    getToken();
  }
}


/*
 * .ALIGN n
 * align the current address to a multiple of n
 * by emitting zero-bytes (n must be a power of 2)
 */
void dotAlign(unsigned int code) {
  Value v;
  unsigned int mask;

  v = parseExpression();
  if (v.sym != NULL) {
    error("absolute expression expected in line %d", lineno);
  }
  if (countBits(v.con) != 1) {
    error("argument must be a power of 2 in line %d", lineno);
  }
  mask = v.con - 1;
  while ((segPtr[currSeg] & mask) != 0) {
    emitByte(0);
  }
}


/*
 * .SPACE n
 * emit n zero-bytes
 */
void dotSpace(unsigned int code) {
  Value v;
  int i;

  v = parseExpression();
  if (v.sym != NULL) {
    error("absolute expression expected in line %d", lineno);
  }
  for (i = 0; i < v.con; i++) {
    emitByte(0);
  }
}


/*
 * .WORD <comma-separated list of values>
 * deposit the 32-bit values in memory
 */
void dotWord(unsigned int code) {
  Value v;

  while (1) {
    v = parseExpression();
    if (v.sym == NULL) {
      emitWord(v.con);
    } else {
      addFixup(currSeg, segPtr[currSeg], RELOC_W32, v.sym, v.con);
      emitWord(0);
    }
    if (token != TOK_COMMA) {
      break;
    }
    getToken();
  }
}


/*
 * .HALF <comma-separated list of values>
 * deposit the 16-bit values in memory
 */
void dotHalf(unsigned int code) {
  Value v;

  while (1) {
    v = parseExpression();
    if (v.sym != NULL) {
      error("absolute expression expected in line %d", lineno);
    }
    emitHalf(v.con);
    if (token != TOK_COMMA) {
      break;
    }
    getToken();
  }
}


/*
 * .BYTE <comma-separated list of values>
 * deposit the 8-bit values in memory
 * strings are stored without a trailing zero
 */
void dotByte(unsigned int code) {
  Value v;
  char *p;

  while (1) {
    if (token == TOK_STRING) {
      p = tokenvalString;
      while (*p != '\0') {
        emitByte(*p);
        p++;
      }
      getToken();
    } else {
      v = parseExpression();
      if (v.sym != NULL) {
        error("absolute expression expected in line %d", lineno);
      }
      emitByte(v.con);
    }
    if (token != TOK_COMMA) {
      break;
    }
    getToken();
  }
}


/*
 * .SET <name>,<value>
 * define name to denote the given value
 */
void dotSet(unsigned int code) {
  Value v;
  Symbol *symbol;

  expect(TOK_IDENT);
  symbol = lookupEnter(tokenvalString);
  if ((symbol->status & STAT_DEFINED) != 0) {
    error("symbol '%s' multiply defined in line %d",
          symbol->name, lineno);
  }
  getToken();
  expect(TOK_COMMA);
  getToken();
  v = parseExpression();
  if (v.sym == NULL) {
    symbol->status |= STAT_DEFINED;
    symbol->segment = SEG_ABS;
    symbol->value = v.con;
  } else {
    error("illegal type of symbol '%s' in expression, line %d",
          v.sym->name, lineno);
  }
}


/**************************************************************/

/* instruction table */


typedef struct {
  char *name;
  void (*func)(unsigned int code);
  unsigned int code;
} Instr;


Instr instrTable[] = {
  /* register data move */
  { "MOV",     format_0,  OP_MOV	},
  { "MOVH",    format_1,  OP_MOVH	},
  { "PUTS",    format_2,  OP_PUTS	},
  { "GETS",    format_2,  OP_GETS	},
  /* shift */
  { "LSL",     format_3,  OP_LSL	},
  { "ASR",     format_3,  OP_ASR	},
  { "LSR",     format_3,  OP_LSR	},
  { "ROR",     format_3,  OP_ROR	},
  /* logic */
  { "AND",     format_3,  OP_AND	},
  { "ANN",     format_3,  OP_ANN	},
  { "IOR",     format_3,  OP_IOR	},
  { "XOR",     format_3,  OP_XOR	},
  /* integer arithmetic */
  { "ADD",     format_3,  OP_ADD	},
  { "ADDC",    format_3,  OP_ADDC	},
  { "SUB",     format_3,  OP_SUB	},
  { "SUBB",    format_3,  OP_SUBB	},
  { "MUL",     format_3,  OP_MUL	},
  { "MULU",    format_3,  OP_MULU	},
  { "DIV",     format_3,  OP_DIV	},
  { "DIVU",    format_3,  OP_DIVU	},
  /* floating-point arithmetic */
  { "FAD",     format_3,  OP_FAD	},
  { "FSB",     format_3,  OP_FSB	},
  { "FML",     format_3,  OP_FML	},
  { "FDV",     format_3,  OP_FDV	},
  /* floating-point conversions */
  { "FLR",     format_4,  OP_FLR	},
  { "FLT",     format_4,  OP_FLT	},
  /* load/store memory */
  { "LDW",     format_5,  OP_LDW	},
  { "LDH",     format_5,  OP_LDH	},
  { "LDB",     format_5,  OP_LDB	},
  { "STW",     format_5,  OP_STW	},
  { "STH",     format_5,  OP_STH	},
  { "STB",     format_5,  OP_STB	},
  /* branch */
  { "BMI",     format_6,  OP_BMI	},
  { "BEQ",     format_6,  OP_BEQ	},
  { "BCS",     format_6,  OP_BCS	},
  { "BVS",     format_6,  OP_BVS	},
  { "BLS",     format_6,  OP_BLS	},
  { "BLT",     format_6,  OP_BLT	},
  { "BLE",     format_6,  OP_BLE	},
  { "B",       format_6,  OP_B		},
  { "BPL",     format_6,  OP_BPL	},
  { "BNE",     format_6,  OP_BNE	},
  { "BCC",     format_6,  OP_BCC	},
  { "BVC",     format_6,  OP_BVC	},
  { "BHI",     format_6,  OP_BHI	},
  { "BGE",     format_6,  OP_BGE	},
  { "BGT",     format_6,  OP_BGT	},
  { "BNVR",    format_6,  OP_BNVR	},
  /* call */
  { "CMI",     format_6,  OP_CMI	},
  { "CEQ",     format_6,  OP_CEQ	},
  { "CCS",     format_6,  OP_CCS	},
  { "CVS",     format_6,  OP_CVS	},
  { "CLS",     format_6,  OP_CLS	},
  { "CLT",     format_6,  OP_CLT	},
  { "CLE",     format_6,  OP_CLE	},
  { "C",       format_6,  OP_C		},
  { "CPL",     format_6,  OP_CPL	},
  { "CNE",     format_6,  OP_CNE	},
  { "CCC",     format_6,  OP_CCC	},
  { "CVC",     format_6,  OP_CVC	},
  { "CHI",     format_6,  OP_CHI	},
  { "CGE",     format_6,  OP_CGE	},
  { "CGT",     format_6,  OP_CGT	},
  { "CNVR",    format_6,  OP_CNVR	},
  /* interrupt control */
  { "RTI",     format_7,  OP_RTI	},
  { "CLI",     format_7,  OP_CLI	},
  { "STI",     format_7,  OP_STI	},
  /* assembler directives */
  { ".CODE",   dotCode,   0		},
  { ".DATA",   dotData,   0		},
  { ".BSS",    dotBss,    0		},
  { ".GLOBAL", dotGlobal, 0		},
  { ".ALIGN",  dotAlign,  0		},
  { ".SPACE",  dotSpace,  0		},
  { ".WORD",   dotWord,   0		},
  { ".HALF",   dotHalf,   0		},
  { ".BYTE",   dotByte,   0		},
  { ".SET",    dotSet,    0		},
};


static int cmpInstr(const void *instr1, const void *instr2) {
  return strcmp(((Instr *) instr1)->name, ((Instr *) instr2)->name);
}


void sortInstrTable(void) {
  qsort(instrTable, sizeof(instrTable)/sizeof(instrTable[0]),
        sizeof(instrTable[0]), cmpInstr);
}


Instr *lookupInstr(char *name) {
  int lo, hi, tst;
  int res;

  lo = 0;
  hi = sizeof(instrTable) / sizeof(instrTable[0]) - 1;
  while (lo <= hi) {
    tst = (lo + hi) / 2;
    res = strcmp(instrTable[tst].name, name);
    if (res == 0) {
      return &instrTable[tst];
    }
    if (res < 0) {
      lo = tst + 1;
    } else {
      hi = tst - 1;
    }
  }
  return NULL;
}


/**************************************************************/

/* assembler for a whole module */


void asmModule(void) {
  Symbol *label;
  Instr *instr;

  currSeg = SEG_CODE;
  lineno = 0;
  while (fgets(line, LINE_SIZE, inFile) != NULL) {
    lineno++;
    lineptr = line;
    getToken();
    while (token == TOK_LABEL) {
      label = lookupEnter(tokenvalString);
      if ((label->status & STAT_DEFINED) != 0) {
        error("label '%s' multiply defined in line %d",
              label->name, lineno);
      }
      label->status |= STAT_DEFINED;
      label->segment = currSeg;
      label->value = segPtr[currSeg];
      getToken();
    }
    if (token == TOK_IDENT) {
      instr = lookupInstr(tokenvalString);
      if (instr == NULL) {
        error("unknown instruction '%s' in line %d",
              tokenvalString, lineno);
      }
      getToken();
      (*instr->func)(instr->code);
    }
    if (token != TOK_EOL) {
      error("garbage in line %d", lineno);
    }
  }
  doFixups();
}


/**************************************************************/

/* object file writer */


static ExecHeader execHeader;

static int fileOffset;
static int fileDataSize;
static int fileStringSize;

static int nsyms;
static int nrels;


static void writeDummyHeader(void) {
  fwrite(&execHeader, sizeof(ExecHeader), 1, outFile);
  /* update file offset */
  fileOffset += sizeof(ExecHeader);
}


static void writeRealHeader(void) {
  rewind(outFile);
  execHeader.magic = EXEC_MAGIC;
  execHeader.entry = 0;
  conv4FromNativeToTarget((unsigned char *) &execHeader.magic);
  conv4FromNativeToTarget((unsigned char *) &execHeader.osegs);
  conv4FromNativeToTarget((unsigned char *) &execHeader.nsegs);
  conv4FromNativeToTarget((unsigned char *) &execHeader.osyms);
  conv4FromNativeToTarget((unsigned char *) &execHeader.nsyms);
  conv4FromNativeToTarget((unsigned char *) &execHeader.orels);
  conv4FromNativeToTarget((unsigned char *) &execHeader.nrels);
  conv4FromNativeToTarget((unsigned char *) &execHeader.odata);
  conv4FromNativeToTarget((unsigned char *) &execHeader.sdata);
  conv4FromNativeToTarget((unsigned char *) &execHeader.ostrs);
  conv4FromNativeToTarget((unsigned char *) &execHeader.sstrs);
  conv4FromNativeToTarget((unsigned char *) &execHeader.entry);
  fwrite(&execHeader, sizeof(ExecHeader), 1, outFile);
  conv4FromTargetToNative((unsigned char *) &execHeader.magic);
  conv4FromTargetToNative((unsigned char *) &execHeader.osegs);
  conv4FromTargetToNative((unsigned char *) &execHeader.nsegs);
  conv4FromTargetToNative((unsigned char *) &execHeader.osyms);
  conv4FromTargetToNative((unsigned char *) &execHeader.nsyms);
  conv4FromTargetToNative((unsigned char *) &execHeader.orels);
  conv4FromTargetToNative((unsigned char *) &execHeader.nrels);
  conv4FromTargetToNative((unsigned char *) &execHeader.odata);
  conv4FromTargetToNative((unsigned char *) &execHeader.sdata);
  conv4FromTargetToNative((unsigned char *) &execHeader.ostrs);
  conv4FromTargetToNative((unsigned char *) &execHeader.sstrs);
  conv4FromTargetToNative((unsigned char *) &execHeader.entry);
}


static void writeSegment(SegmentRecord *p) {
  conv4FromNativeToTarget((unsigned char *) &p->name);
  conv4FromNativeToTarget((unsigned char *) &p->offs);
  conv4FromNativeToTarget((unsigned char *) &p->addr);
  conv4FromNativeToTarget((unsigned char *) &p->size);
  conv4FromNativeToTarget((unsigned char *) &p->attr);
  fwrite(p, sizeof(SegmentRecord), 1, outFile);
  conv4FromTargetToNative((unsigned char *) &p->name);
  conv4FromTargetToNative((unsigned char *) &p->offs);
  conv4FromTargetToNative((unsigned char *) &p->addr);
  conv4FromTargetToNative((unsigned char *) &p->size);
  conv4FromTargetToNative((unsigned char *) &p->attr);
}


static void writeSegmentTable(void) {
  SegmentRecord segment;

  /* record file offset */
  execHeader.osegs = fileOffset;
  /* write code segment descriptor */
  segment.name = fileStringSize;
  fileStringSize += strlen(CODE_NAME) + 1;
  segment.offs = fileDataSize;
  segment.addr = 0;
  segment.size = segPtr[SEG_CODE];
  segment.attr = SEG_ATTR_A | SEG_ATTR_P | SEG_ATTR_X;
  writeSegment(&segment);
  fileDataSize += segment.size;
  /* write data segment descriptor */
  segment.name = fileStringSize;
  fileStringSize += strlen(DATA_NAME) + 1;
  segment.offs = fileDataSize;
  segment.addr = 0;
  segment.size = segPtr[SEG_DATA];
  segment.attr = SEG_ATTR_A | SEG_ATTR_P | SEG_ATTR_W;
  writeSegment(&segment);
  fileDataSize += segment.size;
  /* write bss segment descriptor */
  segment.name = fileStringSize;
  fileStringSize += strlen(BSS_NAME) + 1;
  segment.offs = fileDataSize;
  segment.addr = 0;
  segment.size = segPtr[SEG_BSS];
  segment.attr = SEG_ATTR_A | SEG_ATTR_W;
  writeSegment(&segment);
  fileDataSize += 0;  /* segment not present */
  /* update file offset */
  execHeader.nsegs = 3;
  fileOffset += execHeader.nsegs * sizeof(SegmentRecord);
}


static void writeSymbol(Symbol *s) {
  SymbolRecord symRec;

  if ((s->status & STAT_GLOBAL) == 0) {
    /* this symbol isn't global: skip */
    return;
  }
  if ((s->status & STAT_DEFINED) == 0 && (s->status & STAT_USED) == 0) {
    /* this symbol is neither defined nor used here: skip */
    return;
  }
  symRec.name = fileStringSize;
  fileStringSize += strlen(s->name) + 1;
  if ((s->status & STAT_DEFINED) == 0) {
    symRec.val = 0;
    symRec.seg = -1;
    symRec.attr = SYM_ATTR_U;
  } else {
    symRec.val = s->value;
    symRec.seg = s->segment;
    symRec.attr = 0;
  }
  conv4FromNativeToTarget((unsigned char *) &symRec.name);
  conv4FromNativeToTarget((unsigned char *) &symRec.val);
  conv4FromNativeToTarget((unsigned char *) &symRec.seg);
  conv4FromNativeToTarget((unsigned char *) &symRec.attr);
  fwrite(&symRec, sizeof(SymbolRecord), 1, outFile);
  conv4FromTargetToNative((unsigned char *) &symRec.name);
  conv4FromTargetToNative((unsigned char *) &symRec.val);
  conv4FromTargetToNative((unsigned char *) &symRec.seg);
  conv4FromTargetToNative((unsigned char *) &symRec.attr);
  /* the symbols get consecutive numbers */
  s->number = nsyms;
  nsyms++;
}


static void writeSymbolTable(void) {
  /* record file offset */
  execHeader.osyms = fileOffset;
  /* write symbol table */
  nsyms = 0;
  walkSymbolTable(writeSymbol);
  /* update file offset */
  execHeader.nsyms = nsyms;
  fileOffset += execHeader.nsyms * sizeof(SymbolRecord);
}


static void writeReloc(Fixup *f) {
  RelocRecord relRec;

  if (f->segment != SEG_CODE && f->segment != SEG_DATA) {
    /* this should never happen */
    error("fixup found in a segment other than code or data");
  }
  relRec.loc = f->offset;
  relRec.seg = f->segment;
  if (f->refSym != NULL) {
    /* relocation references a symbol */
    relRec.typ = f->method | RELOC_SYM;
    relRec.ref = f->refSym->number;
  } else {
    /* relocation references a segment */
    relRec.typ = f->method;
    relRec.ref = f->refSeg;
  }
  relRec.add = f->add;
  conv4FromNativeToTarget((unsigned char *) &relRec.loc);
  conv4FromNativeToTarget((unsigned char *) &relRec.seg);
  conv4FromNativeToTarget((unsigned char *) &relRec.typ);
  conv4FromNativeToTarget((unsigned char *) &relRec.ref);
  conv4FromNativeToTarget((unsigned char *) &relRec.add);
  fwrite(&relRec, sizeof(RelocRecord), 1, outFile);
  conv4FromTargetToNative((unsigned char *) &relRec.loc);
  conv4FromTargetToNative((unsigned char *) &relRec.seg);
  conv4FromTargetToNative((unsigned char *) &relRec.typ);
  conv4FromTargetToNative((unsigned char *) &relRec.ref);
  conv4FromTargetToNative((unsigned char *) &relRec.add);
  nrels++;
}


static void writeRelocTable(void) {
  Fixup *f;

  /* record file offset */
  execHeader.orels = fileOffset;
  /* write reloc table */
  nrels = 0;
  f = fixupList;
  while (f != NULL) {
    writeReloc(f);
    f = f->next;
  }
  /* update file offset */
  execHeader.nrels = nrels;
  fileOffset += execHeader.nrels * sizeof(RelocRecord);
}


static void writeData(void) {
  /* record file offset */
  execHeader.odata = fileOffset;
  /* write segment data */
  writeCodeBytes();
  writeDataBytes();
  /* update file offset */
  execHeader.sdata = fileDataSize;
  fileOffset += execHeader.sdata;
}


static void writeSymbolName(Symbol *s) {
  if ((s->status & STAT_GLOBAL) == 0) {
    /* this symbol isn't global: skip */
    return;
  }
  if ((s->status & STAT_DEFINED) == 0 && (s->status & STAT_USED) == 0) {
    /* this symbol is neither defined nor used here: skip */
    return;
  }
  fputs(s->name, outFile);
  fputc('\0', outFile);
}


static void writeStrings(void) {
  /* record file offset */
  execHeader.ostrs = fileOffset;
  /* write segment names */
  fputs(CODE_NAME, outFile);
  fputc('\0', outFile);
  fputs(DATA_NAME, outFile);
  fputc('\0', outFile);
  fputs(BSS_NAME, outFile);
  fputc('\0', outFile);
  /* write symbol names */
  walkSymbolTable(writeSymbolName);
  /* update file offsets */
  execHeader.sstrs = fileStringSize;
  fileOffset += execHeader.sstrs;
}


void writeAll(void) {
  fileOffset = 0;
  fileDataSize = 0;
  fileStringSize = 0;
  writeDummyHeader();
  writeSegmentTable();
  writeSymbolTable();
  writeRelocTable();
  writeData();
  writeStrings();
  writeRealHeader();
}


/**************************************************************/

/* inline assembler substitution */


static char *awkText =
"#\n"
"# asm.awk -- implement a poor man's variant of inline assembler\n"
"#\n"
"\n"
"BEGIN {\n"
"def[\"CLI\"] = \"\\tCLI\\n\"\n"
"def[\"STI\"] = \"\\tSTI\\n\"\n"
"def[\"NOP\"] = \"\\tBNVR\\tR0\\n\"\n"
"@\n"
"}\n"
"\n"
"{\n"
"  if ($1 != \".GLOBAL\" || substr($2,1,5) != \"_asm_\")\n"
"  {\n"
"    if ($1 == \"C\" && substr($2,1,5) == \"_asm_\")\n"
"    {\n"
"      printf def[substr($2,6)]\n"
"    }\n"
"    else\n"
"    {\n"
"      print $0\n"
"    }\n"
"  }\n"
"  # else do nothing: suppress .GLOBAL declarations for _asm_xyz\n"
"}\n"
;


char *doInlineAsmSubst(char *inName, char *iadName) {
  static char tmpName[LINE_SIZE];
  char awkName[LINE_SIZE];
  FILE *iadFile;
  FILE *awkFile;
  char *p;
  int c;
  char awkCommand[LINE_SIZE];

  sprintf(tmpName, "%s.tmp", inName);
  sprintf(awkName, "%s.awk", inName);
  if (debugIAS) {
    printf("DEBUG: inline assembler substitution files\n");
    printf("DEBUG: inName = '%s', tmpName = '%s'\n", inName, tmpName);
    printf("DEBUG: iadName = '%s', awkName = '%s'\n", (iadName != NULL) ? iadName : "(None)", awkName);
  }
  if (iadName != NULL) {  
    iadFile = fopen(iadName, "r");
    if (iadFile == NULL) {
      error("cannot open inline definition file '%s' for read", iadName);
    }
  }
  awkFile = fopen(awkName, "w");
  if (awkFile == NULL) {
    error("cannot open AWK file '%s' for write", awkName);
  }
  /* copy AWK text to AWK file */
  p = awkText;
  while ((c = *p++) != '\0') {
    if (c == '@') {
      /* at character '@' include definition file */
      if (iadName != NULL) {
        while ((c = fgetc(iadFile)) != EOF) {
          fputc(c, awkFile);
        }
      }
    } else {
      fputc(c, awkFile);
    }
  }
  if (iadName != NULL) {
    fclose(iadFile);
  }
  fclose(awkFile);
  /* build up AWK command line */
  sprintf(awkCommand, "awk -f %s %s >%s", awkName, inName, tmpName);
  if (debugIAS) {
    printf("DEBUG: inline assembler substitution command\n");
    printf("DEBUG: %s\n", awkCommand);
  }
  /* execute */
  system(awkCommand);
  if (!debugIAS) {
    /* remove AWK file */
    remove(awkName);
  }
  return tmpName;
}


/**************************************************************/

/* main program */


void usage(char *myself) {
  fprintf(stderr, "Usage: %s\n", myself);
  fprintf(stderr, "         [-o file]     set object file\n");
  fprintf(stderr, "         [-i file]     set inline asm definition file\n");
  fprintf(stderr, "         file          set input file\n");
  exit(1);
}


int main(int argc, char *argv[]) {
  int i;
  char *inName;
  char *outName;
  char *iadName;

  sortInstrTable();
  inName = NULL;
  outName = DFLT_OUT_NAME;
  iadName = NULL;
  for (i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      /* option */
      if (strcmp(argv[i], "-o") == 0) {
        if (i == argc - 1) {
          usage(argv[0]);
        }
        outName = argv[++i];
      } else
      if (strcmp(argv[i], "-i") == 0) {
        if (i == argc - 1) {
          usage(argv[0]);
        }
        iadName = argv[++i];
      } else {
        usage(argv[0]);
      }
    } else {
      /* file */
      if (inName != NULL) {
        usage(argv[0]);
      }
      inName = argv[i];
    }
  }
  if (inName == NULL) {
    error("no input file");
  }
  inName = doInlineAsmSubst(inName, iadName);
  inFile = fopen(inName, "r");
  if (inFile == NULL) {
    error("cannot open input file '%s'", inName);
  }
  outFile = fopen(outName, "w");
  if (outFile == NULL) {
    error("cannot open output file '%s'", outName);
  }
  asmModule();
  writeAll();
  fclose(inFile);
  fclose(outFile);
  if (!debugIAS) {
    /* remove temporary file */
    remove(inName);
  }
  return 0;
}
