//
// crt0.s -- startup code and interrupt dispatch
//

	// constants
	.SET	stack,0xC00000
	.SET	interrupt_frame_size,17*4

	// import symbols
	.GLOBAL	_bcode
	.GLOBAL	_ecode
	.GLOBAL	_bdata
	.GLOBAL	_edata
	.GLOBAL	_bbss
	.GLOBAL	_ebss
	.GLOBAL	main

	// export symbols
	.GLOBAL	stack
	.GLOBAL risc5_ivt
    .GLOBAL risc5_start_irq_dispatch
    .GLOBAL risc5_default_irq_dispatcher

	.CODE

    B start
risc5_start_irq_dispatch:
    B risc5_default_irq_dispatcher
risc5_ivt:
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
	.WORD 	default_isr
risc5_default_irq_dispatcher:
	GETS 	R13,	3		// first of all save the PSW, because of the flags
	SUB	R14, R14, interrupt_frame_size		// calculate the new stack pointer for saving context
	STW R0, R14, 0*4		// save R0
	STW R1, R14, 1*4		// save R1
	STW R2, R14, 2*4		// save R2
	STW R3, R14, 3*4		// save R3
	STW R4, R14, 4*4		// save R4
	STW R5, R14, 5*4		// save R5
	STW R6, R14, 6*4		// save R6
	STW R7, R14, 7*4		// save R7
	STW R8, R14, 8*4		// save R8
	STW R9, R14, 9*4		// save R9
	STW R10, R14, 10*4		// save R10
	STW R11, R14, 11*4		// save R11
	STW R12, R14, 12*4		// save R12
	STW R15, R14, 13*4		// save R15
	GETS R1, 1				// get H register
	GETS R2, 2				// get X register
	STW R1, R14, 14*4		// save H register
	STW R2, R14, 15*4		// save X register
	STW R13, R14, 16*4		// save PSW

    ROR R13, R13, 16		// shift ACK in R13
	AND R13, R13, 0xF		// mask 4 LSBs of ACK
	LSL R13, R13, 2			// multiply by 4
	LDW R13, R13, risc5_ivt	// load address of ISR from risc5_ivt
	
	C R13		// call ISR
    CLI			// enter critical section
	
	// combine saved flags and most recent interrupt mask:
	LDW R13, R14, 16*4			// load PSW from stack
	AND R13, R13, 0xFFFF0000	// clear mask in saved PSW
	GETS R0, 3					// LOAD current PSW into R0
	AND R0, R0, 0x0000FFFF		// clear flags in R0
	IOR R13, R13, R0			// OR new flags and old mask together

	LDW R2, R14, 15*4		// load X register from stack
	LDW R1, R14, 14*4		// load H register from stack
	PUTS R2, 2				// restore X register
	PUTS R1, 1				// restore H register
	LDW R15, R14, 13*4		// load R15 register from stack
	LDW R12, R14, 12*4		// load R12 register from stack
	LDW R11, R14, 11*4		// load R11 register from stack
	LDW R10, R14, 10*4		// load R10 register from stack
	LDW R9, R14, 9*4		// load R9 register from stack
	LDW R8, R14, 8*4		// load R8 register from stack
	LDW R7, R14, 7*4		// load R7 register from stack
	LDW R6, R14, 6*4		// load R6 register from stack
	LDW R5, R14, 5*4		// load R5 register from stack
	LDW R4, R14, 4*4		// load R4 register from stack
	LDW R3, R14, 3*4		// load R3 register from stack
	LDW R2, R14, 2*4		// load R2 register from stack
	LDW R1, R14, 1*4		// load R1 register from stack
	LDW R0, R14, 0*4		// load R0 register from stack
	ADD R14, R14, interrupt_frame_size	// restore the old stack pointer
	PUTS R13, 3		// restore PSW
    RTI
default_isr:		// infinite loop, never returns
	B default_isr

start:
	MOV	R14,stack	// set sp
	MOV	R6,_bdata	// copy data segment
	MOV	R4,_edata
	SUB	R5,R4,R6
	ADD	R5,R5,_ecode
	B	cpytest
cpyloop:
	LDW	R7,R5,0
	STW	R7,R4,0
cpytest:
	SUB	R4,R4,4
	SUB	R5,R5,4
	SUB	R12,R4,R6
	BCC	cpyloop
	MOV	R4,_bbss	// clear bss
	MOV	R5,_ebss
	MOV	R0,0
	B	clrtest
clrloop:
	STW	R0,R4,0
	ADD	R4,R4,4
clrtest:
	SUB	R12,R4,R5
	BCS	clrloop
	C	main		// call 'main'
start1:
	B	start1		// loop
