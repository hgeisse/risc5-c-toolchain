//
// blinkintr2.s -- blinking LED, interrupt-driven by high precision timer
//

	.CODE

	// program starts here, at location 0
start:
	B	main

	// interrupts arrive here, at location 4
intr:
	MOV	R0,-124		// reset timer interrupt
	LDW	R0,R0,0		// by reading its status
	MOV	R0,leds		// and toggle LED state (LSB)
	LDW	R1,R0,0
	XOR	R1,R1,1
	STW	R1,R0,0
	MOV	R0,-60
	STW	R1,R0,0
	RTI			// return from interrupt

main:
	MOV	R0,-128		// set divisor
	MOV	R1,25000000	// to clock cycles per 500 msec
	STW	R1,R0,0
	MOV	R1,1		// enable device interrupt
	STW	R1,R0,4
	GETS	R0,3		// get PSW
	IOR	R0,R0,1<<15	// enable high precision timer interrupt
	PUTS	R0,3		// put PSW
	STI			// enable interrupts
loop:
	B	loop		// and wait indefinitely

	.DATA

leds:
	.WORD	0		// LED state
