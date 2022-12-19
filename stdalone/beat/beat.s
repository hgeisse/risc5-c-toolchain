//
// beat.s -- LED, interrupt-driven by two high precision timers
//

	.CODE

	// program starts here, at location 0
start:
	B	main

	// interrupts arrive here, at location 4
intr:
	GETS	R0,3		// get PSW
	ROR	R0,R0,16	// which IRQ?
	AND	R0,R0,0xF
	SUB	R1,R0,15
	BEQ	led_off		// timer 0: switch off
	SUB	R1,R0,14
	BEQ	led_on		// timer 1: switch on
	RTI			// ignore other IRQs
led_off:
	MOV	R0,-128		// reset timer interrupt
	LDW	R0,R0,4		// by reading its status
	MOV	R0,-60
	MOV	R1,0
	STW	R1,R0,0
	RTI			// return from interrupt
led_on:
	MOV	R0,-104		// reset timer interrupt
	LDW	R0,R0,4		// by reading its status
	MOV	R0,-60
	MOV	R1,1
	STW	R1,R0,0
	RTI			// return from interrupt

main:
	MOV	R0,-128		// set divisor
	MOV	R1,100000-200	// for 501 Hz
	STW	R1,R0,0
	MOV	R1,1		// enable device interrupt
	STW	R1,R0,4
	MOV	R0,-104		// set divisor
	MOV	R1,100000	// for 500 Hz
	STW	R1,R0,0
	MOV	R1,1		// enable device interrupt
	STW	R1,R0,4
	GETS	R0,3		// get PSW
	IOR	R0,R0,1<<15	// enable high precision timer 0 interrupt
	IOR	R0,R0,1<<14	// enable high precision timer 1 interrupt
	PUTS	R0,3		// put PSW
	STI			// enable interrupts
loop:
	B	loop		// and wait indefinitely
