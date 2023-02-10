/*
 * main.c -- main program
 */


#include "types.h"
#include "stdarg.h"
#include "iolib.h"
#include "promlib.h"


#define CHECK_SECTOR	1234


void delayTest(void) {
  printf("please check the delay, it should last 30 seconds\n");
  delay(30000);
  printf("\n");
}


void ledTest(void) {
  int i;

  printf("please watch the LEDs\n");
  for (i = 0; i < 5; i++) {
    ledWrite(0xFF);
    delay(500);
    ledWrite(0x00);
    delay(500);
  }
  printf("\n");
}


void switchTest(void) {
  Word pat;
  int i;

  pat = switchRead();
  printf("switches : ");
  for (i = 7; i >= 0; i--) {
    printf("%c ", (pat & (1 << i)) ? '1' : '0');
  }
  printf("\n");
  printf("\n");
}


void sdcardTest(void) {
  Word buffer[512/4];
  Word save[512/4];
  Byte *p;
  int i;

  /* read sector 0 */
  printf("read sector 0\n");
  sdcardRead(0, buffer);
  printf("0x%08X  0x%08X  0x%08X  0x%08X ... 0x%08X  0x%08X\n",
         buffer[0], buffer[1], buffer[2], buffer[3],
         buffer[126], buffer[127]);
  /* read sector CHECK_SECTOR */
  printf("read sector %d\n", CHECK_SECTOR);
  sdcardRead(CHECK_SECTOR, buffer);
  printf("0x%08X  0x%08X  0x%08X  0x%08X ... 0x%08X  0x%08X\n",
         buffer[0], buffer[1], buffer[2], buffer[3],
         buffer[126], buffer[127]);
  /* save sector CHECK_SECTOR */
  printf("save sector %d\n", CHECK_SECTOR);
  for (i = 0; i < 512; i++) {
    save[i] = buffer[i];
  }
  /* write sector CHECK_SECTOR */
  printf("write sector %d\n", CHECK_SECTOR);
  p = (Byte *) buffer;
  for (i = 0; i < 512; i++) {
    *p++ = i;
  }
  sdcardWrite(CHECK_SECTOR, buffer);
  /* read sector 0 */
  printf("read sector 0\n");
  sdcardRead(0, buffer);
  printf("0x%08X  0x%08X  0x%08X  0x%08X ... 0x%08X  0x%08X\n",
         buffer[0], buffer[1], buffer[2], buffer[3],
         buffer[126], buffer[127]);
  /* read sector CHECK_SECTOR */
  printf("read sector %d\n", CHECK_SECTOR);
  sdcardRead(CHECK_SECTOR, buffer);
  printf("0x%08X  0x%08X  0x%08X  0x%08X ... 0x%08X  0x%08X\n",
         buffer[0], buffer[1], buffer[2], buffer[3],
         buffer[126], buffer[127]);
  /* restore sector CHECK_SECTOR */
  printf("restore sector %d\n", CHECK_SECTOR);
  sdcardWrite(CHECK_SECTOR, save);
  /* read sector CHECK_SECTOR */
  printf("read sector %d\n", CHECK_SECTOR);
  sdcardRead(CHECK_SECTOR, buffer);
  printf("0x%08X  0x%08X  0x%08X  0x%08X ... 0x%08X  0x%08X\n",
         buffer[0], buffer[1], buffer[2], buffer[3],
         buffer[126], buffer[127]);
  /* finally, show capacity */
  printf("capacity = ");
  p = (Byte *) sdcardCapacity();
  for (i = 0; i < 8; i++) {
    printf("0x%02X  ", p[i]);
  }
  printf("\n           ");
  for (i = 8; i < 16; i++) {
    printf("0x%02X  ", p[i]);
  }
  printf("\n");
  printf("\n");
}


int main(void) {
  char c;

  printf("\nPROM Functions Test\n\n");
  while (1) {
    printf("please choose one of the following actions:\n");
    printf("    0   quit\n");
    printf("    1   delay test\n");
    printf("    2   LED test\n");
    printf("    3   switch test\n");
    printf("    4   SD card test\n");
    c = serialRead();
    printf("%c\n\n", c);
    if (c == '0') {
      break;
    }
    if (c == '1') {
      delayTest();
      continue;
    }
    if (c == '2') {
      ledTest();
      continue;
    }
    if (c == '3') {
      switchTest();
      continue;
    }
    if (c == '4') {
      sdcardTest();
      continue;
    }
    printf("unknown action, please try again\n");
  }
  printf("End of PROM Functions Test\n");
  return 0;
}
