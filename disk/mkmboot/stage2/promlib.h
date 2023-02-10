/*
 * promlib.c -- PROM library
 */


#ifndef _PROMLIB_H_
#define _PROMLIB_H_


void delay(Word msec);
Word switchRead(void);
void ledWrite(Word pattern);
Bool serialReadRdy(void);
Byte serialRead(void);
Bool serialWriteRdy(void);
void serialWrite(Byte c);
void sdcardRead(Word sector, Word *buffer);
void sdcardWrite(Word sector, Word *buffer);
Word sdcardCapacity(void);


#endif /* _PROMLIB_H_ */
