//
// start.s -- startup code
//

	.SET	stack,0xFFE000

	.GLOBAL	_bcode
	.GLOBAL	_ecode
	.GLOBAL	_bdata
	.GLOBAL	_edata
	.GLOBAL	_bbss
	.GLOBAL	_ebss

	.GLOBAL	main

	.GLOBAL	start
	.GLOBAL	msdelay
	.GLOBAL	rdswt
	.GLOBAL	wrled
	.GLOBAL	rcvReady
	.GLOBAL	rcvByte
	.GLOBAL	sndReady
	.GLOBAL	sndByte
	.GLOBAL	sdRead
	.GLOBAL	sdWrite
	.GLOBAL	sdCapacity

//--------------------------------------------------------------

	.GLOBAL	reset
	.GLOBAL	delay
	.GLOBAL	switchRead
	.GLOBAL	ledWrite
	.GLOBAL	serialReadRdy
	.GLOBAL	serialRead
	.GLOBAL	serialWriteRdy
	.GLOBAL	serialWrite
	.GLOBAL	sdcardRead
	.GLOBAL	sdcardWrite
	.GLOBAL	sdcardCapacity

	.CODE

reset:
	B	start

delay:
	B	msdelay

switchRead:
	B	rdswt

ledWrite:
	B	wrled

serialReadRdy:
	B	rcvReady

serialRead:
	B	rcvByte

serialWriteRdy:
	B	sndReady

serialWrite:
	B	sndByte

sdcardRead:
	B	sdRead

sdcardWrite:
	B	sdWrite

sdcardCapacity:
	B	sdCapacity

//--------------------------------------------------------------

	.CODE

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
