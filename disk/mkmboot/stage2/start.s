//
// start.s -- startup code
//

	.SET	VBR_LD_ADDR,0

	.GLOBAL	_bcode
	.GLOBAL	_ecode
	.GLOBAL	_bdata
	.GLOBAL	_edata
	.GLOBAL	_bbss
	.GLOBAL	_ebss
	.GLOBAL	main
	.GLOBAL	partStart
	.GLOBAL	partSize

	.CODE

start:
	MOV	R6,_bdata		// copy data segment
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
	MOV	R4,_bbss		// clear bss
	MOV	R5,_ebss
	MOV	R0,0
	B	clrtest
clrloop:
	STW	R0,R4,0
	ADD	R4,R4,4
clrtest:
	SUB	R12,R4,R5
	BCS	clrloop
	SUB	R14,R14,(1+0)*4		// create stack frame
	STW	R15,R14,0		// save return address
	C	main			// do some useful work
	LDW	R15,R14,0		// restore return address
	ADD	R14,R14,(1+0)*4		// release stack frame
	SUB	R0,R0,0			// test main's return value
	BNE	R15			// not zero means error
	MOV	R1,partStart		// else load partition info
	LDW	R1,R1,0
	MOV	R2,partSize
	LDW	R2,R2,0
	B	VBR_LD_ADDR		// and jump to VBR
