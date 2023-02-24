/*
 * wrtvbr.c -- write a VBR to a partition of a disk image
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


#define SECTOR_SIZE	512
#define PARTTBL_OFF	0x1BE


/**************************************************************/


typedef struct {
  unsigned char boot;			/* MSB is 'bootable' flag */
  unsigned char firstSectorCHS[3];	/* not used */
  unsigned char type;			/* type of partition */
  unsigned char lastSectorCHS[3];	/* not used */
  unsigned int start;			/* first sector LBA */
  unsigned int size;			/* number of sectors */
} PartEntry;


/**************************************************************/


unsigned char vbr[SECTOR_SIZE] = {
  #include "vbr.dump"
};


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


/**************************************************************/


int main(int argc, char *argv[]) {
  char *diskName;
  char *partNmbr;
  FILE *disk;
  unsigned char mbr[SECTOR_SIZE];
  PartEntry *partTbl;
  int i;
  char *endp;
  int pn, pi;
  unsigned int partStart;
  unsigned int partSize;

  /* check command line arguments */
  if (argc != 3) {
    printf("Usage: %s <disk image> <partition number>\n", argv[0]);
    exit(1);
  }
  diskName = argv[1];
  partNmbr = argv[2];
  /* read MBR */
  disk = fopen(diskName, "r+b");
  if (disk == NULL) {
    error("cannot open disk image '%s'", diskName);
  }
  if (fread(mbr, 1, SECTOR_SIZE, disk) != SECTOR_SIZE) {
    error("cannot read MBR from disk image '%s'", diskName);
  }
  /* check boot signature */
  if (mbr[0x1FE] != 0x55 || mbr[0x1FF] != 0xAA) {
    error("MBR of disk '%s' has no boot signature", diskName);
  }
  /* show partitions */
  partTbl = (PartEntry *) &mbr[PARTTBL_OFF];
  printf("Partitions:\n");
  printf("  #  type  b  start       size\n");
  for (i = 0; i < 4; i++) {
    printf("  %d  0x%02X  %c  0x%08X  0x%08X\n",
           i + 1,
           partTbl[i].type,
           partTbl[i].boot & 0x80 ? '*' : '-',
           partTbl[i].start,
           partTbl[i].size);
  }
  /* get partition number, check type and bootable flag */
  pn = strtol(partNmbr, &endp, 10);
  if (*endp != '\0') {
    error("cannot read partition number");
  }
  if (pn < 1 || pn > 4) {
    error("illegal partition number %d", pn);
  }
  pi = pn - 1;
  if (partTbl[pi].type == 0) {
    error("partition %d does not contain a file system", pn);
  }
  if ((partTbl[pi].boot & 0x80) == 0) {
    error("partition %d is not bootable", pn);
  }
  /* determine start and size of partition */
  partStart = partTbl[pi].start;
  partSize = partTbl[pi].size;
  if (partSize == 0) {
    error("partition %d has no sectors", pn);
  }
  /* copy VBR to partition on disk image */
  printf("Now going to copy the VBR to partition %d...\n", pn);
  fseek(disk, partStart * SECTOR_SIZE, SEEK_SET);
  if (fwrite(vbr, 1, SECTOR_SIZE, disk) != SECTOR_SIZE) {
    error("cannot write VBR to disk image '%s'", diskName);
  }
  printf("...done\n");
  fclose(disk);
  return 0;
}
