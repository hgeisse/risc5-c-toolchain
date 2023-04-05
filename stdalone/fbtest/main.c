/*
 * main.c -- RISC5 framebuffer test
 */


#include "types.h"
#include "stdarg.h"
#include "iolib.h"


#define FB_ADDR		((unsigned int *) (0xC00000))

#define NUM_VERIFY	4


/**************************************************************/


unsigned int randomNumber;


void setRandomNumber(unsigned int seed) {
  randomNumber = seed;
}


unsigned int getRandomNumber(void) {
  randomNumber = randomNumber * (unsigned) 1103515245 + (unsigned) 12345;
  return randomNumber;
}


/**************************************************************/


void fbWriteColor(unsigned int color) {
  int row, col;
  unsigned int *memAddr;

  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      *memAddr++ = color;
    }
  }
}


int fbVerifyColor(unsigned int color) {
  int row, col;
  unsigned int *memAddr;

  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      if (*memAddr++ != color) {
        return 0;
      }
    }
  }
  return 1;
}


/**************************************************************/


void fbWriteRandom(unsigned int seed) {
  int row, col;
  unsigned int *memAddr;

  setRandomNumber(seed);
  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      *memAddr++ = (getRandomNumber() >> 13) & 0x7FFF;
    }
  }
}


int fbVerifyRandom(unsigned int seed) {
  int row, col;
  unsigned int *memAddr;

  setRandomNumber(seed);
  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      if (*memAddr++ != ((getRandomNumber() >> 13) & 0x7FFF)) {
        return 0;
      }
    }
  }
  return 1;
}


/**************************************************************/


unsigned int borderColor(int row, int col) {
  unsigned int c;

  if (row == 0 || row == 767 ||
      col == 0 || col == 1023) {
    c = 0x739C;
  } else {
    c = 0x0000;
  }
  return c;
}


void fbWriteBorder(void) {
  int row, col;
  unsigned int *memAddr;

  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      *memAddr++ = borderColor(row, col);
    }
  }
}


int fbVerifyBorder(void) {
  int row, col;
  unsigned int *memAddr;

  memAddr = FB_ADDR;
  for (row = 0; row < 768; row++) {
    for (col = 0; col < 1024; col++) {
      if (*memAddr++ != borderColor(row, col)) {
        return 0;
      }
    }
  }
  return 1;
}


/**************************************************************/


int main(void) {
  int i;

  printf("\nRISC5 frame buffer test 2 started\n\n");
  printf("writing frame buffer, value = 0x0000...\n");
  fbWriteColor(0x0000);
  for (i = 0; i < NUM_VERIFY; i++) {
    printf("verifying frame buffer...\n");
    if (fbVerifyColor(0x0000)) {
      printf("data is correct\n");
    } else {
      printf("verification failed\n");
    }
  }
  printf("\n");
  printf("writing frame buffer, value = 0x739C\n");
  fbWriteColor(0x739C);
  for (i = 0; i < NUM_VERIFY; i++) {
    printf("verifying frame buffer...\n");
    if (fbVerifyColor(0x739C)) {
      printf("data is correct\n");
    } else {
      printf("verification failed\n");
    }
  }
  printf("\n");
  printf("writing frame buffer, random values\n");
  fbWriteRandom(0xDEADBEEF);
  for (i = 0; i < NUM_VERIFY; i++) {
    printf("verifying frame buffer...\n");
    if (fbVerifyRandom(0xDEADBEEF)) {
      printf("data is correct\n");
    } else {
      printf("verification failed\n");
    }
  }
  printf("\n");
  printf("writing frame buffer, 1-pixel border\n");
  fbWriteBorder();
  for (i = 0; i < NUM_VERIFY; i++) {
    printf("verifying frame buffer...\n");
    if (fbVerifyBorder()) {
      printf("data is correct\n");
    } else {
      printf("verification failed\n");
    }
  }
  printf("\nRISC5 frame buffer test 2 finished\n");
  return 0;
}
