//
// mbr.s -- master boot record
//

	.SET	startSector,1		// disk location of boot manager
	.SET	numSectors,63		// maximum size of boot manager
	.SET	loadAddr,0xB00000	// where to load the boot manager

	.SET	serialOut,0xFFE01C	// serial output
	.SET	readSector,0xFFE020	// read sector from SD card

	// get some addresses listed in the load map
	.GLOBAL	mbr
	.GLOBAL	load
	.GLOBAL	liftoff
	.GLOBAL	msg

	// the master boot record loads the boot manager
	// from a fixed disk position into high memory
mbr:
	SUB	R14,R14,(3+2)*4		// create stack frame
	STW	R15,R14,8		// save return address
	STW	R8,R14,12		// save register variable
	STW	R9,R14,16		// and another one
	MOV	R8,msg			// pointer to string
strloop:
	LDB	R1,R8,0			// get char
	BEQ	load			// null - finished, go loading
	C	serialOut		// output char
	ADD	R8,R8,1			// bump pointer
	B	strloop			// next char
load:
	MOV	R8,startSector		// first sector to load
	MOV	R9,loadAddr		// gets loaded here
ldloop:
	MOV	R1,R8			// first argument: sector
	MOV	R2,R9			// second argument: address
	C	readSector		// load a single sector
	ADD	R9,R9,512		// bump load address
	ADD	R8,R8,1			// and sector number
	SUB	R4,R8,startSector+numSectors
	BNE	ldloop			// not yet finished?
	LDW	R15,R14,8		// restore return address
	LDW	R8,R14,12		// restore register variable
	LDW	R9,R14,16		// and another one
	ADD	R14,R14,(3+2)*4		// release stack frame
liftoff:
	B	loadAddr		// jump to loaded program

	// say what is going on
msg:
	.BYTE	0x0D, 0x0A
	.BYTE	"MBR executing..."
	.BYTE	0x0D, 0x0A, 0

	.SPACE	369			// adjust for sizeof(mbr) = 512

	.BYTE	0x55,0xAA		// boot record signature
