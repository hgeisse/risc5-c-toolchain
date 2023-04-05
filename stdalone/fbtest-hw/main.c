/*
 * main.c -- RISC5 framebuffer test
 * Note: This test requires the "full-test" version of the video hardware!
 */


#include "types.h"
#include "stdarg.h"
#include "iolib.h"


#define FB_ADDR		((unsigned int *) (0xC00000))

#define NUM_VERIFY	8


/**************************************************************/


unsigned int color(int row, int col) {
  unsigned int c;

  if (row == 0 || row == 767 ||
      col == 0 || col == 1023) {
    c = 0x739C;
  } else {
    c = 0x0000;
  }
  return c;
}


/**************************************************************/


int verify(void) {
  int row, col;
  unsigned int *memAddr;

  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      if (*memAddr++ != color(row, col)) {
        return 0;
      }
    }
  }
  return 1;
}


/**************************************************************/


int main(void) {
  int i;

  printf("\nRISC5 frame buffer test started\n\n");
  for (i = 0; i < NUM_VERIFY; i++) {
    printf("verifying frame buffer...\n");
    if (verify()) {
      printf("data is correct\n");
    } else {
      printf("verification failed\n");
    }
  }
  printf("\nRISC5 frame buffer test finished\n");
  return 0;
}
