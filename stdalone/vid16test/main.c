/*
 * main.c -- main program
 */


#include "types.h"
#include "stdarg.h"
#include "iolib.h"
#include "promlib.h"


void vid16Test1(void) {
  int row, col;
  Word *p;

  p = (Word *) 0xC00000;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      *p++ = 0x00000000;
    }
  }
}


void vid16Test2(void) {
  int row, col;
  Word *p;

  p = (Word *) 0xC00000;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      *p++ = 0x00FFFFFF;
    }
  }
}


void vid16Test3(void) {
  int row, col;
  Word *p;

  p = (Word *) 0xC00000;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      switch (row >> 8) {
        case 0:
          *p++ = 0x00FF0000;
          break;
        case 1:
          *p++ = 0x0000FF00;
          break;
        case 2:
          *p++ = 0x000000FF;
          break;
      }
    }
  }
}


void vid16Test4(void) {
  int row, col;
  Word *p;

  p = (Word *) 0xC00000;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      switch (row / 96) {
        case 0:
          *p++ = 0x00000000;
          break;
        case 1:
          *p++ = 0x00FF0000;
          break;
        case 2:
          *p++ = 0x0000FF00;
          break;
        case 3:
          *p++ = 0x00FFFF00;
          break;
        case 4:
          *p++ = 0x000000FF;
          break;
        case 5:
          *p++ = 0x00FF00FF;
          break;
        case 6:
          *p++ = 0x0000FFFF;
          break;
        case 7:
          *p++ = 0x00FFFFFF;
          break;
      }
    }
  }
}


void vid16Test5(void) {
  int row, col;
  Word *p;

  p = (Word *) 0xC00000;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      switch (col >> 7) {
        case 0:
          *p++ = 0x00000000;
          break;
        case 1:
          *p++ = 0x00FF0000;
          break;
        case 2:
          *p++ = 0x0000FF00;
          break;
        case 3:
          *p++ = 0x00FFFF00;
          break;
        case 4:
          *p++ = 0x000000FF;
          break;
        case 5:
          *p++ = 0x00FF00FF;
          break;
        case 6:
          *p++ = 0x0000FFFF;
          break;
        case 7:
          *p++ = 0x00FFFFFF;
          break;
      }
    }
  }
}


int main(void) {
  char c;

  printf("\nPROM Functions Test\n\n");
  while (1) {
    printf("please choose one of the following actions:\n");
    printf("    0   quit\n");
    printf("    1   video test 1\n");
    printf("    2   video test 2\n");
    printf("    3   video test 3\n");
    printf("    4   video test 4\n");
    printf("    5   video test 5\n");
    c = serialRead();
    printf("%c\n\n", c);
    if (c == '0') {
      break;
    }
    if (c == '1') {
      vid16Test1();
      continue;
    }
    if (c == '2') {
      vid16Test2();
      continue;
    }
    if (c == '3') {
      vid16Test3();
      continue;
    }
    if (c == '4') {
      vid16Test4();
      continue;
    }
    if (c == '5') {
      vid16Test5();
      continue;
    }
    printf("unknown action, please try again\n");
  }
  printf("End of PROM Functions Test\n");
  return 0;
}
