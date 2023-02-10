/*
 * promlib.c -- PROM library
 */


#include "types.h"
#include "promlib.h"


#define PROM_DELAY		((void (*) (Word)) 0xFFE004)
#define PROM_SWITCH_READ	((Word (*) (void)) 0xFFE008)
#define PROM_LED_WRITE		((void (*) (Word)) 0xFFE00C)
#define PROM_SERIAL_READ_RDY	((Bool (*) (void)) 0xFFE010)
#define PROM_SERIAL_READ	((Byte (*) (void)) 0xFFE014)
#define PROM_SERIAL_WRITE_RDY	((Bool (*) (void)) 0xFFE018)
#define PROM_SERIAL_WRITE	((void (*) (Byte)) 0xFFE01C)
#define PROM_SDCARD_READ	((void (*) (Word, Word *)) 0xFFE020)
#define PROM_SDCARD_WRITE	((void (*) (Word, Word *)) 0xFFE024)
#define PROM_SDCARD_CAPACITY	((Word (*) (void)) 0xFFE028)


void delay(Word msec) {
  PROM_DELAY(msec);
}


Word switchRead(void) {
  return PROM_SWITCH_READ();
}


void ledWrite(Word pattern) {
  PROM_LED_WRITE(pattern);
}


Bool serialReadRdy(void) {
  return PROM_SERIAL_READ_RDY();
}


Byte serialRead(void) {
  return PROM_SERIAL_READ();
}


Bool serialWriteRdy(void) {
  return PROM_SERIAL_WRITE_RDY();
}


void serialWrite(Byte c) {
  PROM_SERIAL_WRITE(c);
}


void sdcardRead(Word sector, Word *buffer) {
  PROM_SDCARD_READ(sector, buffer);
}


void sdcardWrite(Word sector, Word *buffer) {
  PROM_SDCARD_WRITE(sector, buffer);
}


Word sdcardCapacity(void) {
  return PROM_SDCARD_CAPACITY();
}
