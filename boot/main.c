/*
 * main.c -- bootstrap loader
 */


typedef enum { false = 0, true = 1 } Bool;

typedef unsigned int Word;
typedef unsigned short Half;
typedef unsigned char Byte;


/**************************************************************/


#define MST_ADDR	((unsigned int *) 0xFFFFC0)


void msdelay(Word ms) {
  Word t;

  t = *MST_ADDR;
  t += ms;
  while (*MST_ADDR < t) ;
}


/**************************************************************/


#define LED_ADDR	((unsigned int *) 0xFFFFC4)
#define SWT_ADDR	((unsigned int *) 0xFFFFC4)


void wrled(unsigned int val) {
  *LED_ADDR = val;
}


unsigned int rdswt(void) {
  return *SWT_ADDR;
}


/**************************************************************/


#define SER_DATA	((unsigned int *) 0xFFFFC8)
#define SER_CTRL	((unsigned int *) 0xFFFFCC)


Bool rcvReady(void) {
  return (*SER_CTRL & 1) != 0;
}


unsigned char rcvByte(void) {
  while ((*SER_CTRL & 1) == 0) ;
  return *SER_DATA;
}


Bool sndReady(void) {
  return (*SER_CTRL & 2) != 0;
}


void sndByte(unsigned char b) {
  while ((*SER_CTRL & 2) == 0) ;
  *(SER_DATA) = b;
}


/**************************************************************/


#define ACK		((unsigned char) 0x10)
#define NAK		((unsigned char) 0x11)


static unsigned int rcvInt(void) {
  unsigned int val;

  val = 0;
  val |= ((unsigned int) rcvByte() <<  0);
  val |= ((unsigned int) rcvByte() <<  8);
  val |= ((unsigned int) rcvByte() << 16);
  val |= ((unsigned int) rcvByte() << 24);
  return val;
}


static Bool loadFromLine(void) {
  unsigned int len;
  unsigned char *addr;

  while (1) {
    len = rcvInt();
    if (len == 0) {
      break;
    }
    addr = (unsigned char *) rcvInt();
    do {
      *addr++ = rcvByte();
      len--;
    } while (len != 0);
  }
  sndByte(ACK);
  return true;
}


/**************************************************************/


#define SPI_DATA	((unsigned int *) 0xFFFFD0)
#define SPI_CTRL	((unsigned int *) 0xFFFFD4)


/*
 * send n FFs slowly with no card selected
 */
static void spiIdle(int n) {
  *SPI_CTRL = 0;
  while (n > 0) {
    n--;
    *SPI_DATA = 0xFFFFFFFF;
    while ((*SPI_CTRL & 1) == 0) ;
  }
}


/*
 * send & rcv byte slowly with card selected
 */
static void spi(int n) {
  *SPI_CTRL = 1;
  *SPI_DATA = n;
  while ((*SPI_CTRL & 1) == 0) ;
}


/*
 * send command
 */
static void spiCmd(int n, int arg) {
  int data;
  int crc;
  int i;

  /* flush while deselected */
  do {
    spiIdle(1);
    data = *SPI_DATA;
  } while (data != 255) ;
  /* flush while selected */
  do {
    spi(255);
    data = *SPI_DATA;
  } while (data != 255) ;
  /* select precomputed CRC */
  crc = (n == 8) ? 135 : ((n == 0) ? 149 : 255);
  /* send command */
  spi(0x40 | (n & 0x3F));
  /* send arg */
  for (i = 24; i >= 0; i -= 8) {
    spi(arg >> i);
  }
  /* send CRC */
  spi(crc);
  /* wait for response */
  i = 32;
  do {
    spi(255);
    data = *SPI_DATA;
    i--;
  } while (data >= 0x80 && i != 0);
}


/*
 * initialize SPI
 */
static void spiInit(void) {
  int res;

  /* first, idle for at least 80 clks */
  spiIdle(9);
  /* CMD0 when card selected, sets MMC SPI mode */
  spiCmd(0, 0);
  /* CMD8 for SD cards */
  spiCmd(8, 0x1AA);
  spi(-1);
  spi(-1);
  spi(-1);
  /* wait for card getting ready */
  do {
    /* ACMD41, optionally with high-capacity (HCS) bit set, starts init */
    spiCmd(55, 0);  /* APP cmd follows */
    spiCmd(41, 1 << 30);  /* HCS */
    res = *SPI_DATA;
    spi(-1);
    spi(-1);
    spi(-1);
    spiIdle(10000);
  } while (res != 0);
  /* CMD16: set block size as a precaution (should default) */
  spiCmd(16, 512);
  spiIdle(1);
}


/**************************************************************/


static void sdShift(int *p) {
  int data;

  /* CMD58: get card capacity bit */
  spiCmd(58, 0);
  data = *SPI_DATA;
  spi(-1);
  if (data != 0 || (*SPI_DATA & 0x40) == 0) {
    *p *= 512;
  }
  spi(-1);
  spi(-1);
  spiIdle(1);
}


void sdRead(int src, int dst) {
  int data;
  int i;

  sdShift(&src);
  /* CMD17: read one block */
  spiCmd(17, src);
  /* wait for start data marker */
  do {
    spi(-1);
    data = *SPI_DATA;
  } while (data != 254);
  /* switch to fast */
  *SPI_CTRL = 4 + 1;
  /* get data */
  for (i = 0; i <= 508; i += 4) {
    *SPI_DATA = 0xFFFFFFFF;
    while ((*SPI_CTRL & 1) == 0) ;
    data = *SPI_DATA;
    * (unsigned int *) dst = data;
    dst += 4;
  }
  /* may be a checksum */
  spi(255);
  spi(255);
  /* deselect card */
  spiIdle(1);
}


void sdWrite(int dst, int src) {
  int data;
  int i;

  sdShift(&dst);
  /* CMD24: write one block */
  spiCmd(24, dst);
  /* write start data marker */
  spi(254);
  /* switch to fast */
  *SPI_CTRL = 4 + 1;
  for (i = 0; i <= 508; i += 4) {
    data = * (unsigned int *) src;
    src += 4;
    *SPI_DATA = data;
    while ((*SPI_CTRL & 1) == 0) ;
  }
  /* dummy checksum */
  spi(255);
  spi(255);
  i = 0;
  do {
    spi(-1);
    data = *SPI_DATA;
    i++;
  } while ((data & 31) != 5 && i != 10000);
  /* deselect card */
  spiIdle(1);
}


Word sdCapacity(void) {
  Word data;
  int i;
  Byte csd[16];
  Word csize;
  Word numSectors;

  /* CMD9: send card-specific data (CSD) */
  spiCmd(9, 0);
  /* wait for start data marker */
  do {
    spi(-1);
    data = *SPI_DATA;
  } while (data != 254);
  /* get data */
  for (i = 0; i < 16; i++) {
    *SPI_DATA = 0xFFFFFFFF;
    while ((*SPI_CTRL & 1) == 0) ;
    data = *SPI_DATA;
    csd[i] = data;
  }
  /* may be a checksum */
  spi(255);
  spi(255);
  /* deselect card */
  spiIdle(1);
  /* verify correct CSD version */
  if ((csd[0] & 0xC0) != 0x40) {
    /* wrong CSD structure version */
    return 0;
  }
  /* extract size information */
  csize = (((Word) csd[7] & 0x3F) << 16) |
          (((Word) csd[8] & 0xFF) <<  8) |
          (((Word) csd[9] & 0xFF) <<  0);
  /* compute number of sectors */
  numSectors = (csize + 1) << 10;
  return numSectors;
}


/**************************************************************/


static Bool loadFromDisk(void) {
  unsigned char sb0, sb1;

  /* read boot sector */
  sdRead(0, 0);
  /* check boot signature */
  sb0 = * (unsigned char *) 0x1FE;
  sb1 = * (unsigned char *) 0x1FF;
  return sb0 == 0x55 && sb1 == 0xAA;
}


/**************************************************************/


void main(void) {
  Bool loaded;

  wrled(0x80);
  spiInit();
  if (rdswt() & 1) {
    /* loading from serial line requested */
    wrled(0x81);
    loaded = loadFromLine();
  } else {
    /* loading from disk requested */
    wrled(0x82);
    loaded = loadFromDisk();
  }
  if (loaded) {
    /* loading succeeded, start program */
    wrled(0x83);
    (* (void (*)(void)) 0)();
  }
  /* loading failed */
  wrled(0xFF);
}
