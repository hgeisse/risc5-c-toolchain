/*
 * combine.c -- combine MBR and boot manager to the master boot block
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#define SECTOR_SIZE	512
#define MIN_RSV_SECTS	64	/* min # sectors before first partition */


/**************************************************************/


void error(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  printf("Error: ");
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
  exit(1);
}


void *memAlloc(unsigned int size) {
  void *p;

  p = malloc(size);
  if (p == NULL) {
    error("out of memory");
  }
  memset(p, 0, size);
  return p;
}


void memFree(void *p) {
  if (p == NULL) {
    error("memFree() detected NULL pointer");
  }
  free(p);
}


/**************************************************************/


int main(int argc, char *argv[]) {
  char *stage1Name;
  char *stage2Name;
  char *mbootblkName;
  FILE *stage1;
  unsigned int size1;
  int sectors1;
  unsigned char *bytes1;
  FILE *stage2;
  unsigned int size2;
  int sectors2;
  unsigned char *bytes2;
  FILE *mbootblk;

  /* check command line */
  if (argc != 4) {
    printf("Usage: %s <stage1.bin> <stage2.bin> <mbootblk>\n",
           argv[0]);
    exit(1);
  }
  stage1Name = argv[1];
  stage2Name = argv[2];
  mbootblkName = argv[3];
  /* load stage 1 */
  stage1 = fopen(stage1Name, "rb");
  if (stage1 == NULL) {
    error("cannot open '%s'", stage1Name);
  }
  fseek(stage1, 0, SEEK_END);
  size1 = ftell(stage1);
  fseek(stage1, 0, SEEK_SET);
  if (size1 != SECTOR_SIZE) {
    error("stage1 has size != %d", SECTOR_SIZE);
  }
  sectors1 = 1;
  bytes1 = memAlloc(sectors1 * SECTOR_SIZE);
  if (fread(bytes1, 1, size1, stage1) != size1) {
    error("cannot read '%s'", stage1Name);
  }
  fclose(stage1);
  /* load stage 2 */
  stage2 = fopen(stage2Name, "rb");
  if (stage2 == NULL) {
    error("cannot open '%s'", stage2Name);
  }
  fseek(stage2, 0, SEEK_END);
  size2 = ftell(stage2);
  fseek(stage2, 0, SEEK_SET);
  sectors2 = (size2 + SECTOR_SIZE - 1) / SECTOR_SIZE;
  if (sectors1 + sectors2 > MIN_RSV_SECTS) {
    error("stage2 too big");
  }
  bytes2 = memAlloc(sectors2 * SECTOR_SIZE);
  if (fread(bytes2, 1, size2, stage2) != size2) {
    error("cannot read '%s'", stage2Name);
  }
  fclose(stage2);
  /* now do the single important task this program has */
  * (unsigned int *) &bytes1[4] = sectors2;
  /* write output */
  mbootblk = fopen(mbootblkName, "wb");
  if (mbootblk == NULL) {
    error("cannot open '%s'", mbootblkName);
  }
  if (fwrite(bytes1, SECTOR_SIZE, sectors1, mbootblk) != sectors1) {
    error("cannot write stage1 to '%s'", mbootblkName);
  }
  if (fwrite(bytes2, SECTOR_SIZE, sectors2, mbootblk) != sectors2) {
    error("cannot write stage2 to '%s'", mbootblkName);
  }
  fclose(mbootblk);
  memFree(bytes1);
  memFree(bytes2);
  printf("master boot block '%s' written (%d sectors)\n",
         mbootblkName, sectors1 + sectors2);
  return 0;
}
