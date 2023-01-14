/*
 * main.c -- test program
 */


#include <stdio.h>
#include <string.h>


#define TEST_STRING	"hello"
#define TEST_LENGTH	((int) strlen(TEST_STRING))
#define TEST_START	2


/**************************************************************/


#define TEST_BUF_LEN	14


char testBuffer[TEST_BUF_LEN];


void clearBuffer(void) {
  int i;

  for (i = 0; i < TEST_BUF_LEN; i++) {
    testBuffer[i] = '?';
  }
}


void showBuffer(void) {
  int i;

  for (i = 0; i < TEST_BUF_LEN; i++) {
    printf("0x%02X ", testBuffer[i]);
  }
  printf("\r\n");
}


/**************************************************************/


void test1(void) {
  printf("empty buffer\r\n");
  clearBuffer();
  showBuffer();
  printf("\r\n");
}


void test2(void) {
  int n;

  printf("str = '%s', strlen = %d, start = %d, infinite buffer\r\n",
         TEST_STRING, TEST_LENGTH, TEST_START);
  clearBuffer();
  n = sprintf(testBuffer + TEST_START, "%s", TEST_STRING);
  showBuffer();
  printf("num chars written = %d\r\n", n);
  printf("\r\n");
}


void test3(int bufsize) {
  int n;

  printf("str = '%s', strlen = %d, start = %d, buffer size = %d\r\n",
         TEST_STRING, TEST_LENGTH, TEST_START, bufsize);
  clearBuffer();
  n = snprintf(testBuffer + TEST_START, bufsize, "%s", TEST_STRING);
  showBuffer();
  printf("num chars written = %d\r\n", n);
  printf("\r\n");
}


/**************************************************************/


int main(void) {
  int bufsize;

  test1();
  test2();
  for (bufsize = 0; bufsize <= 8; bufsize++) {
    test3(bufsize);
  }
  return 0;
}
