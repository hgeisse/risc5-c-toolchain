	.GLOBAL	C_extern
	.GLOBAL	D_extern
	.GLOBAL	B_extern

	.CODE

	ADD	R3,R2,R1
	ADD	R3,R2,R1
	ADD	R3,R2,R1
C_extern:
	ADD	R3,R2,R1
	ADD	R3,R2,R1
	ADD	R3,R2,R1

	.DATA

	.WORD	0x55AA55AA
	.WORD	0x55AA55AA
	.WORD	0x55AA55AA
D_extern:
	.WORD	0x55AA55AA
	.WORD	0x55AA55AA
	.WORD	0x55AA55AA

	.BSS
	.SPACE	0x100
B_extern:
	.SPACE	0x100
