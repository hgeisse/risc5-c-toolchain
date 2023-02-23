/*
 * mkpart.c -- make partitions on a disk
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>


#define SECTOR_SIZE	512
#define SECTS_PER_MIB	2048
#define NEXT_MIB(x)	(((x) + SECTS_PER_MIB - 1) & ~(SECTS_PER_MIB - 1))

#define MIN_RSV_SECTS	64	/* min # sectors before first partition */

#define LINE_SIZE	100
#define MAX_TOKENS	30


/**************************************************************/


unsigned char buf[MIN_RSV_SECTS * SECTOR_SIZE];


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


int tokenize(char *line, char *tokens[], int maxTokens) {
  int n;
  char *p;

  n = 0;
  p = strtok(line, " \t\n\r");
  while (p != NULL) {
    if (n < maxTokens) {
      tokens[n++] = p;
    }
    p = strtok(NULL, " \t\n\r");
  }
  return n;
}


int main(int argc, char *argv[]) {
  char *diskName;
  char *confName;
  FILE *disk;
  FILE *conf;
  unsigned long diskSize;
  unsigned int numSectors;
  unsigned int diskIdent;
  char line[LINE_SIZE];
  char *p, *q;
  int lineNumber;
  FILE *mbootblk;
  unsigned long mbootblkSize;
  int i;
  int currPart;
  unsigned int currSect;
  char *tokens[MAX_TOKENS];
  int n;
  char *endp;
  unsigned int start;
  unsigned int size;
  unsigned int type;
  unsigned int boot;

  /* check command line arguments */
  if (argc != 3) {
    printf("Usage: %s <disk image file> <configuration file>\n", argv[0]);
    exit(1);
  }
  diskName = argv[1];
  confName = argv[2];
  /* determine disk size */
  disk = fopen(diskName, "rb");
  if (disk == NULL) {
    error("cannot open disk image '%s'", diskName);
  }
  fseek(disk, 0, SEEK_END);
  diskSize = ftell(disk);
  numSectors = diskSize / SECTOR_SIZE;
  fclose(disk);
  printf("Disk '%s' has %u (0x%X) sectors.\n",
         diskName, numSectors, numSectors);
  if (numSectors < MIN_RSV_SECTS) {
    error("disk is too small");
  }
  if (diskSize % SECTOR_SIZE != 0) {
    printf("Warning: disk size is not a multiple of sector size!\n");
  }
  /* generate disk identifier (aka disk signature) */
  srand((unsigned int) time(NULL));
  diskIdent = (unsigned int) rand();
  printf("Disk identifier = 0x%08X\n", diskIdent);
  /*
   * create partition table (and possibly
   * master boot block) from config file
   */
  conf = fopen(confName, "rt");
  if (conf == NULL) {
    error("cannot open configuration file '%s'", confName);
  }
  lineNumber = 0;
  /* first, handle master boot block specification */
  while (fgets(line, LINE_SIZE, conf) != NULL) {
    lineNumber++;
    p = line;
    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '\0' || *p == '\n' || *p == '#') {
      continue;
    }
    q = p;
    while (*q > 0x20 && *q < 0x7F) {
      q++;
    }
    *q = '\0';
    if (strcmp(p, "-noboot-") == 0) {
      /* master boot block not wanted */
    } else {
      /* p points to name of master boot block file */
      mbootblk = fopen(p, "rb");
      if (mbootblk == NULL) {
        error("cannot open master boot block file '%s'", p);
      }
      fseek(mbootblk, 0, SEEK_END);
      mbootblkSize = ftell(mbootblk);
      fseek(mbootblk, 0, SEEK_SET);
      if (mbootblkSize > MIN_RSV_SECTS * SECTOR_SIZE) {
        error("master boot block file '%s' is bigger than %d sectors",
              p, MIN_RSV_SECTS);
      }
      for (i = 0; i < MIN_RSV_SECTS * SECTOR_SIZE; i++) {
        buf[i] = '\0';
      }
      if (fread(buf, 1, mbootblkSize, mbootblk) != mbootblkSize) {
        error("cannot read master boot block file '%s'", p);
      }
      fclose(mbootblk);
      disk = fopen(diskName, "r+b");
      if (disk == NULL) {
        error("cannot open disk image '%s'", diskName);
      }
      if (fwrite(buf, 1, MIN_RSV_SECTS * SECTOR_SIZE, disk) !=
                         MIN_RSV_SECTS * SECTOR_SIZE) {
        error("cannot write master boot block to disk image '%s'", diskName);
      }
      fclose(disk);
    }
    break;
  }
  /* then, handle partition table entries */
  currPart = 0;
  currSect = MIN_RSV_SECTS;
  while (fgets(line, LINE_SIZE, conf) != NULL) {
    lineNumber++;
    p = line;
    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '\0' || *p == '\n' || *p == '#') {
      continue;
    }
    n = tokenize(p, tokens, MAX_TOKENS);
    if (n < 4) {
      error("too few tokens in config file '%s', line %d",
            confName, lineNumber);
    }
    if (n > 4 && *tokens[4] != '#') {
      error("garbage at end of line in config file '%s', line %d",
            confName, lineNumber);
    }
    /* type */
    type = strtoul(tokens[0], &endp, 0);
    if (*endp != '\0') {
      error("cannot read partition type in config file '%s', line %d",
            confName, lineNumber);
    }
    if (type >= 0x100) {
      error("illegal partition type in config file '%s', line %d",
            confName, lineNumber);
    }
    /* boot */
    if (strcmp(tokens[1], "*") == 0) {
      boot = 0x80;
    } else
    if (strcmp(tokens[1], "-") == 0) {
      boot = 0x00;
    } else {
      error("illegal boot flag in config file '%s', line %d",
            confName, lineNumber);
    }
    /* start */
    if (strcmp(tokens[2], "+") == 0) {
      start = NEXT_MIB(currSect);
    } else {
      start = strtoul(tokens[2], &endp, 0);
      if (*endp != '\0') {
        if (strcmp(endp, "M") != 0) {
          error("cannot read start sector in config file '%s', line %d",
                confName, lineNumber);
        }
        start *= SECTS_PER_MIB;
      }
    }
    if (type != 0) {
      if (start < currSect) {
        error("start sector too low in config file '%s', line %d",
              confName, lineNumber);
      }
      currSect = start;
    }
    /* size */
    size = strtoul(tokens[3], &endp, 0);
    if (*endp != '\0') {
      if (strcmp(endp, "M") != 0) {
        error("cannot read partition size in config file '%s', line %d",
              confName, lineNumber);
      }
      size *= SECTS_PER_MIB;
    }
    if (type != 0) {
      currSect += size;
      if (currSect > numSectors) {
        error("not enough space on disk in config file '%s', line %d",
              confName, lineNumber);
      }
    }
    /* correct values if entry is empty */
    if (type == 0) {
      if (boot != 0 || size != 0) {
        printf("Warning: null entry corrected in config file '%s', line %d\n",
               confName, lineNumber);
      }
      boot = 0;
      start = 0;
      size = 0;
    }
    /* fill entry */
    if (currPart > 3) {
      error("too many partitions in config file '%s', line %d",
            confName, lineNumber);
    }
    partTbl[currPart].boot = boot;
    partTbl[currPart].firstSectorCHS[0] = 0xFE;  /* dummy value, not used */
    partTbl[currPart].firstSectorCHS[1] = 0xFF;  /* dummy value, not used */
    partTbl[currPart].firstSectorCHS[2] = 0xFF;  /* dummy value, not used */
    partTbl[currPart].type = type;
    partTbl[currPart].lastSectorCHS[0] = 0xFE;  /* dummy value, not used */
    partTbl[currPart].lastSectorCHS[1] = 0xFF;  /* dummy value, not used */
    partTbl[currPart].lastSectorCHS[2] = 0xFF;  /* dummy value, not used */
    partTbl[currPart].start = start;
    partTbl[currPart].size = size;
    currPart++;
  }
  fclose(conf);
  /* next, show partition table */
  printf("Partitions:\n");
  printf("  #  type  b  start       size\n");
  for (currPart = 0; currPart < 4; currPart++) {
    printf("  %d  0x%02X  %c  0x%08X  0x%08X\n",
           currPart,
           partTbl[currPart].type,
           partTbl[currPart].boot & 0x80 ? '*' : '-',
           partTbl[currPart].start,
           partTbl[currPart].size);
  }
  /* finally, write the partition table to the master boot record */
  /* also set the disk identifier (but *NOT* the boot signature!) */
  disk = fopen(diskName, "r+b");
  if (disk == NULL) {
    error("cannot open disk image '%s'", diskName);
  }
  if (fread(buf, 1, SECTOR_SIZE, disk) != SECTOR_SIZE) {
    error("cannot read master boot record from disk image '%s'", diskName);
  }
  * (unsigned int *) &buf[0x1B8] = diskIdent;  /* disk identifier */
  buf[0x1BC] = 0;  /* no copy protection */
  buf[0x1BD] = 0;
  memcpy(&buf[0x1BE], partTbl, sizeof(partTbl));  /* partition table */
  fseek(disk, 0, SEEK_SET);
  if (fwrite(buf, 1, SECTOR_SIZE, disk) != SECTOR_SIZE) {
    error("cannot write master boot record to disk image '%s'", diskName);
  }
  fclose(disk);
  /* done */
  return 0;
}
