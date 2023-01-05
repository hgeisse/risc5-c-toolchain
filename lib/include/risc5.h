/*
 * risc5.h -- RISC5 specific declarations and constants
 */

#ifndef __RISC5_H
#define __RISC5_H

#include <stdint.h>

/*********************************************************************************************************************/
/*** Interrupts ***/

/* interrupt vector table */
extern void (*risc5_ivt[16])(void);  /* defined in crt0.s */

/* IRQ numbers */
#define RISC5_IRQ_HPTMR_0       (15U)
#define RISC5_IRQ_HPTMR_1       (14U)
#define RISC5_IRQ_MSTMR         (11U)
#define RISC5_IRQ_RS232_0_RX    (7U)
#define RISC5_IRQ_RS232_0_TX    (6U)
#define RISC5_IRQ_RS232_1_RX    (5U)
#define RISC5_IRQ_RS232_1_TX    (4U)
#define RISC5_IRQ_BTNSWT        (3U)

/*********************************************************************************************************************/
/*** Devices ***/

/* Device base addresses */

#define RISC5_IO_BASE		    (0xFFFFC0U)
#define RISC5_XIO_BASE	        (0xFFFF80U)

/* Standard device addresses */

#define RISC5_IO_MSTMR	                ((volatile uint32_t*)(0xFFFFC0 + 0))    /* read: counter    write: control  */

#define RISC5_IO_SWTLED                 ((volatile uint32_t*)(0xFFFFC4 + 0))    /* read: status     write: LEDs     */

#define RISC5_IO_RS232_0_DATA           ((volatile uint32_t*)(0xFFFFC8 + 0))    /* read: rcv data   write: xmt data */
#define RISC5_IO_RS232_0_STAT_CTRL      ((volatile uint32_t*)(0xFFFFC8 + 4))    /* read: status     write: control  */

#define RISC5_IO_RS232_1_DATA           ((volatile uint32_t*)(0xFFFFA0 + 0))    /* read: rcv data   write: xmt data */
#define RISC5_IO_RS232_1_STAT_CTRL      ((volatile uint32_t*)(0xFFFFA0 + 4))    /* read: status     write: control  */

#define RISC5_IO_SPI_DATA               ((volatile uint32_t*)(0xFFFFD0 + 0))    /* read: data       write: data     */
#define RISC5_IO_SPI_STAT_CTRL          ((volatile uint32_t*)(0xFFFFD0 + 4))    /* read: status     write: control  */

#define RISC5_IO_KBD_MDATA_KSTAT        ((volatile uint32_t*)(0xFFFFD8 + 0))
#define RISC5_IO_KBD_KDATA              ((volatile uint32_t*)(0xFFFFD8 + 4))

#define RISC5_IO_GPIO_DATA              ((volatile uint32_t*)(0xFFFFE0 + 0))
#define RISC5_IO_GPIO_DIR               ((volatile uint32_t*)(0xFFFFE0 + 4))

#define RISC5_IO_SIM_SHUTDOWN           ((volatile uint32_t*)(0xFFFFFC + 0))

/* Extended device addresses */

#define RISC5_IO_HPTMR_0_DATA           ((volatile uint32_t*)(0xFFFF80 + 0))    /* read: counter    write: divisor */
#define RISC5_IO_HPTMR_0_CTRL           ((volatile uint32_t*)(0xFFFF80 + 4))    /* read: status     write: control  */

#define RISC5_IO_HPTMR_1_DATA           ((volatile uint32_t*)(0xFFFF98 + 0))    /* read: counter    write: divisor */
#define RISC5_IO_HPTMR_1_CTRL           ((volatile uint32_t*)(0xFFFF98 + 4))    /* read: status     write: control  */

#define RISC5_IO_LCD_DATA               ((volatile uint32_t*)(0xFFFF88 + 0))    /* read: data/status    write: data/instr    */
#define RISC5_IO_LCD_STAT_CTRL          ((volatile uint32_t*)(0xFFFF88 + 4))    /* read: control lines  write: control lines */

#define RISC5_IO_BTNSWT                 ((volatile uint32_t*)(0xFFFF88 + 0))    /* read: status     write: control  */

/*********************************************************************************************************************/
/*** Cpu Intrinsics ***/

extern void _asm_CLI(void);
extern void _asm_STI(void);
extern void _asm_NOP(void);

#define RISC5_DISABLE_INTERRUPTS() _asm_CLI()
#define RISC5_ENABLE_INTERRUPTS() _asm_STI()
#define RISC5_NOP() _asm_NOP()

/*********************************************************************************************************************/
/*** Interrupt dispatch ***/

/*
 * The default interrupt dispatching behaviour can be overwritten
 * by writing a jump instruction to a user provided dispatcher at
 * address 4 (risc5_start_irq_dispatch).
 * 
 * To do so, the macro RISC5_SET_IRQ_DISPATCHER(dispatcher_adress) 
 * is provided, which correctly encodes the jump to the new dispatcher.
 * 
 * Example:
 *      extern void(*my_dispatcher)(void);
 *      uint32_t old_jump_instruction = risc5_start_irq_dispatch;
 *      RISC5_SET_IRQ_DISPATCHER(my_dispatcher);
 * 
 * NOTE: The dispatcher is not a normal function and should be written 
 *       in assembly language. See `risc5_default_irq_dispatcher` in
 *       crt0.s for a example dispatcher. 
 * NOTE: Changing the interrupt dispatcher should only be necessary 
 *       when writing very low level code like an an operating system. 
 *       Normal code wishing to install a interrupt service routine
 *       for a particular interrupt should do so by writing the address 
 *       of the isr to the corresponding entry in risc5_ivt[].
 */

extern uint32_t risc5_start_irq_dispatch;
extern uint32_t risc5_default_irq_dispatcher;

/* 
 * Decode address of current irq dispatcher from branch instruction at address risc5_start_irq_dispatch.
 * B Instruction: 
 *             4      4     2       22
 *      +------+------+------+------+------+------+------+------+
 * F3:  | 1110 | 0111 | 00      off                             |
 *      +------+------+------+------+------+------+------+------+
 * Destination address = PC + 4 + off * 4, where PC is the current program counter.
 */
#define RISC5_GET_IRQ_DISPATCHER() ((&risc5_start_irq_dispatch) + 1 + (risc5_start_irq_dispatch & 0x3fffff))

/* 
 * Write a branch instruction to dispatcher_adress at address risc5_start_irq_dispatch.
 */
#define RISC5_SET_IRQ_DISPATCHER(dispatcher_adress)                                                                 \
    do{                                                                                                             \
        risc5_start_irq_dispatch = (0xe7000000 | ((uint32_t *)(dispatcher_adress) - &risc5_start_irq_dispatch - 1));   \
    } while(0)

#endif /* __RISC5_H */
