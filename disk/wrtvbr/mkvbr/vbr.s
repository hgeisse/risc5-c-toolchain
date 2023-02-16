//
// vbr.s -- volume boot record
//

	.SET	serialOut,0xFFE01C	// serial output

	// get some addresses listed in the load map
	.GLOBAL	vbr
	.GLOBAL	leave
	.GLOBAL	msg

	// the volume boot record should load the OS loader into memory
	// this VBR is only a dummy which prints a message and exits
vbr:
	SUB	R14,R14,(3+2)*4		// create stack frame
	STW	R15,R14,8		// save return address
	STW	R8,R14,12		// save register variable
	STW	R9,R14,16		// and another one
	MOV	R8,msg			// pointer to string
strloop:
	LDB	R1,R8,0			// get char
	BEQ	leave			// null - finished, leave
	C	serialOut		// output char
	ADD	R8,R8,1			// bump pointer
	B	strloop			// next char
leave:
	LDW	R15,R14,8		// restore return address
	LDW	R8,R14,12		// restore register variable
	LDW	R9,R14,16		// and another one
	ADD	R14,R14,(3+2)*4		// release stack frame
	B	R15			// return

	// say what is going on
msg:
	.BYTE	0x0D, 0x0A
	.BYTE	"VBR executing..."
	.BYTE	0x0D, 0x0A
	.BYTE	0x0D, 0x0A
	.BYTE	"This is only a sample VBR which does not load anything."
	.BYTE	0x0D, 0x0A
	.BYTE	"Please install a proper operating system on this partition."
	.BYTE	0x0D, 0x0A
	.BYTE	0x0D, 0x0A
	.BYTE	"Bootstrap halted."
	.BYTE	0x0D, 0x0A, 0

	.SPACE	276			// adjust for sizeof(vbr) = 512

	.BYTE	0x55, 0xAA		// boot record signature
