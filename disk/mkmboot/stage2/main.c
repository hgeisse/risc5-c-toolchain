/*
 * main.c -- main program
 */


#include "types.h"
#include "stdarg.h"
#include "iolib.h"
#include "promlib.h"


#define VBR_LD_ADDR	((Word *) 0x000000)

#define PARTTBL_OFF	0x1BE

#define LINE_SIZE	100


/**************************************************************/


typedef struct {
  unsigned char boot;			/* MSB is 'bootable' flag */
  unsigned char firstSectorCHS[3];	/* not used */
  unsigned char type;			/* type of partition */
  unsigned char lastSectorCHS[3];	/* not used */
  unsigned int start;			/* first sector LBA */
  unsigned int size;			/* number of sectors */
} PartEntry;

PartEntry partTbl[4];


/**************************************************************/


int main(void) {
  Word buf[512 / sizeof(Word)];
  int i;
  char line[LINE_SIZE];
  char *endp;
  int n;

  printf("Bootstrap manager executing...\n");
  sdcardRead(0, buf);
  memcpy((Byte *) partTbl, ((Byte *) buf) + PARTTBL_OFF, sizeof(partTbl));
  while (1) {
    printf("\nPartitions:\n");
    printf("  #  type  b  start       size\n");
    for (i = 0; i < 4; i++) {
      printf("  %d  0x%02X  %c  0x%08X  0x%08X\n",
             i,
             partTbl[i].type,
             partTbl[i].boot & 0x80 ? '*' : '-',
             partTbl[i].start,
             partTbl[i].size);
    }
    getLine("\nBoot partition #: ", line, LINE_SIZE);
    if (line[0] == '\0') {
      continue;
    }
    n = str2int(line, &endp);
    if (*endp != '\0' || n < 0 || n > 3) {
      printf("illegal partition number\n");
      continue;
    }
    if (partTbl[n].type == 0) {
      printf("partition %d does not contain a file system\n", n);
      continue;
    }
    if ((partTbl[n].boot & 0x80) == 0) {
      printf("partition %d is not bootable\n", n);
      continue;
    }
    /* load boot sector of selected partition (aka VBR) */
    sdcardRead(partTbl[n].start, VBR_LD_ADDR);
    /* check for signature */
    if ((*((unsigned char *) VBR_LD_ADDR + 0x1FE) != 0x55) ||
        (*((unsigned char *) VBR_LD_ADDR + 0x1FF) != 0xAA)) {
      printf("VBR of partition %d has no signature\n", n);
      continue;
    }
    /* we have a valid VBR: success */
    return 0;
  }
  /* if we ever get here, the bootstrap failed */
  return 1;
}
