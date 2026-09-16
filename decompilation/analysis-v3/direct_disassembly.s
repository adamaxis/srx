0x00000200	STOP	; size=4 opcode=0x4E72 flow=stop
0x00000208	STOP	; size=4 opcode=0x4E72 flow=stop
0x00000210	STOP	; size=4 opcode=0x4E72 flow=stop
0x00000218	STOP	; size=4 opcode=0x4E72 flow=stop
0x00000220	STOP	; size=4 opcode=0x4E72 flow=stop
0x00000228	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0000022E	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00000234	RTE	; size=2 opcode=0x4E73 flow=return
0x00000236	unary-op	; size=6 opcode=0x4AB9 flow=fallthrough
0x0000023C	Bcc[6] 0x000244	; size=2 opcode=0x6606 flow=conditional
0x0000023E	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00000244	Bcc[6] 0x0002C2	; size=2 opcode=0x667C flow=conditional
0x00000246	LEA	; size=4 opcode=0x4BFA flow=fallthrough
0x0000024A	MOVEM	; size=4 opcode=0x4C9D flow=fallthrough
0x0000024E	MOVEM	; size=4 opcode=0x4CDD flow=fallthrough
0x00000252	MOVE	; size=4 opcode=0x1029 flow=fallthrough
0x00000256	immediate-op	; size=4 opcode=0x0200 flow=fallthrough
0x0000025A	Bcc[7] 0x000264	; size=2 opcode=0x6708 flow=conditional
0x0000025C	MOVE	; size=8 opcode=0x237C flow=fallthrough
0x00000264	MOVE	; size=2 opcode=0x3014 flow=fallthrough
0x00000266	MOVEQ	; size=2 opcode=0x7000 flow=fallthrough
0x00000268	MOVE	; size=2 opcode=0x2C40 flow=fallthrough
0x0000026A	MOVE USP	; size=2 opcode=0x4E66 flow=fallthrough
0x0000026C	MOVEQ	; size=2 opcode=0x7217 flow=fallthrough
0x0000026E	MOVE	; size=2 opcode=0x1A1D flow=fallthrough
0x00000270	MOVE	; size=2 opcode=0x3885 flow=fallthrough
0x00000272	arithmetic-op	; size=2 opcode=0xDA47 flow=fallthrough
0x00000274	DBcc 0x00026E	; size=4 opcode=0x51C9 flow=conditional
0x00000278	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x0000027A	MOVE	; size=2 opcode=0x3680 flow=fallthrough
0x0000027C	MOVE	; size=2 opcode=0x3287 flow=fallthrough
0x0000027E	MOVE	; size=2 opcode=0x3487 flow=fallthrough
0x00000280	bit-op	; size=2 opcode=0x0111 flow=fallthrough
0x00000282	Bcc[6] 0x000280	; size=2 opcode=0x66FC flow=conditional
0x00000284	MOVEQ	; size=2 opcode=0x7425 flow=fallthrough
0x00000286	MOVE	; size=2 opcode=0x10DD flow=fallthrough
0x00000288	DBcc 0x000286	; size=4 opcode=0x51CA flow=conditional
0x0000028C	MOVE	; size=2 opcode=0x3480 flow=fallthrough
0x0000028E	MOVE	; size=2 opcode=0x3280 flow=fallthrough
0x00000290	MOVE	; size=2 opcode=0x3487 flow=fallthrough
0x00000292	MOVE	; size=2 opcode=0x2D00 flow=fallthrough
0x00000294	DBcc 0x000292	; size=4 opcode=0x51CE flow=conditional
0x00000298	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x0000029A	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x0000029C	MOVEQ	; size=2 opcode=0x761F flow=fallthrough
0x0000029E	MOVE	; size=2 opcode=0x2680 flow=fallthrough
0x000002A0	DBcc 0x00029E	; size=4 opcode=0x51CB flow=conditional
0x000002A4	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x000002A6	MOVEQ	; size=2 opcode=0x7813 flow=fallthrough
0x000002A8	MOVE	; size=2 opcode=0x2680 flow=fallthrough
0x000002AA	DBcc 0x0002A8	; size=4 opcode=0x51CC flow=conditional
0x000002AE	MOVEQ	; size=2 opcode=0x7A03 flow=fallthrough
0x000002B0	MOVE	; size=4 opcode=0x175D flow=fallthrough
0x000002B4	DBcc 0x0002B0	; size=4 opcode=0x51CD flow=conditional
0x000002B8	MOVE	; size=2 opcode=0x3480 flow=fallthrough
0x000002BA	MOVEM	; size=4 opcode=0x4CD6 flow=fallthrough
0x000002BE	MOVE #imm,SR	; size=4 opcode=0x46FC flow=fallthrough
0x000002C2	BRA 0x000330	; size=2 opcode=0x606C flow=branch
0x00000330	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00000336	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00000338	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0000033E	arithmetic-op	; size=2 opcode=0xEC08 flow=fallthrough
0x00000340	immediate-op	; size=4 opcode=0x0200 flow=fallthrough
0x00000344	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x00000348	MOVE	; size=4 opcode=0x1030 flow=fallthrough
0x0000034C	unary-op	; size=2 opcode=0x4A00 flow=fallthrough
0x0000034E	Bcc[7] 0x000436	; size=4 opcode=0x6700 flow=conditional
0x00000352	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00000358	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000035C	arithmetic-op	; size=2 opcode=0xB010 flow=fallthrough
0x0000035E	Bcc[7] 0x0006C8	; size=4 opcode=0x6700 flow=conditional
0x00000362	ADDQ/SUBQ	; size=2 opcode=0x5288 flow=fallthrough
0x00000364	DBcc 0x00035C	; size=4 opcode=0x51C9 flow=conditional
0x00000368	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x0000036E	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x00000374	MOVE	; size=4 opcode=0x3ABC flow=fallthrough
0x00000378	MOVE	; size=4 opcode=0x3ABC flow=fallthrough
0x0000037C	MOVE	; size=4 opcode=0x3ABC flow=fallthrough
0x00000380	MOVE	; size=4 opcode=0x3ABC flow=fallthrough
0x00000384	MOVE	; size=4 opcode=0x3ABC flow=fallthrough
0x00000388	MOVE	; size=6 opcode=0x2ABC flow=fallthrough
0x0000038E	MOVE	; size=4 opcode=0x38BC flow=fallthrough
0x00000392	MOVE	; size=6 opcode=0x2ABC flow=fallthrough
0x00000398	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x0000039C	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000003A0	MOVE	; size=6 opcode=0x243C flow=fallthrough
0x000003A6	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000003AA	MOVE	; size=2 opcode=0x1218 flow=fallthrough
0x000003AC	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000003B2	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x000003B6	arithmetic-op	; size=2 opcode=0xE99A flow=fallthrough
0x000003B8	arithmetic-op	; size=2 opcode=0xE219 flow=fallthrough
0x000003BA	Bcc[4] 0x0003BE	; size=2 opcode=0x6402 flow=conditional
0x000003BC	arithmetic-op	; size=2 opcode=0x8882 flow=fallthrough
0x000003BE	DBcc 0x0003B6	; size=4 opcode=0x51CD flow=conditional
0x000003C2	MOVE	; size=2 opcode=0x2884 flow=fallthrough
0x000003C4	DBcc 0x0003AA	; size=4 opcode=0x51CE flow=conditional
0x000003C8	DBcc 0x0003A6	; size=4 opcode=0x51C8 flow=conditional
0x000003CC	MOVE	; size=4 opcode=0x123C flow=fallthrough
0x000003D0	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x000003D4	MOVE	; size=2 opcode=0x1018 flow=fallthrough
0x000003D6	BSR 0x000438	; size=4 opcode=0x6100 flow=call
0x000003DA	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000003E0	immediate-op	; size=4 opcode=0x0C11 flow=fallthrough
0x000003E4	Bcc[7] 0x000428	; size=2 opcode=0x6742 flow=conditional
0x000003E6	LEA	; size=4 opcode=0x45FA flow=fallthrough
0x000003EA	MOVE	; size=2 opcode=0x381A flow=fallthrough
0x000003EC	unary-op	; size=2 opcode=0x4A04 flow=fallthrough
0x000003EE	Bcc[7] 0x000424	; size=2 opcode=0x6734 flow=conditional
0x000003F0	arithmetic-op	; size=2 opcode=0xB811 flow=fallthrough
0x000003F2	Bcc[6] 0x000420	; size=2 opcode=0x662C flow=conditional
0x000003F4	immediate-op	; size=6 opcode=0x0C29 flow=fallthrough
0x000003FA	Bcc[6] 0x000410	; size=2 opcode=0x6614 flow=conditional
0x000003FC	SUBA/CMPA/ADDA	; size=6 opcode=0xB3FC flow=fallthrough
0x00000402	Bcc[7] 0x000410	; size=2 opcode=0x670C flow=conditional
0x00000404	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x00000408	MOVE	; size=2 opcode=0x1018 flow=fallthrough
0x0000040A	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x0000040C	BSR 0x000438	; size=4 opcode=0x6100 flow=call
0x00000410	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x00000414	SUBA/CMPA/ADDA	; size=2 opcode=0xD1DA flow=fallthrough
0x00000416	MOVE	; size=2 opcode=0x1018 flow=fallthrough
0x00000418	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x0000041A	BSR 0x000438	; size=4 opcode=0x6100 flow=call
0x0000041E	BRA 0x000424	; size=2 opcode=0x6004 flow=branch
0x00000420	ADDQ/SUBQ	; size=2 opcode=0x588A flow=fallthrough
0x00000422	BRA 0x0003EA	; size=2 opcode=0x60C6 flow=branch
0x00000424	ADDQ/SUBQ	; size=2 opcode=0x5289 flow=fallthrough
0x00000426	BRA 0x0003E0	; size=2 opcode=0x60B8 flow=branch
0x00000428	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x0000042C	MOVE	; size=2 opcode=0x1018 flow=fallthrough
0x0000042E	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00000430	BSR 0x000438	; size=4 opcode=0x6100 flow=call
0x00000434	BRA 0x000434	; size=2 opcode=0x60FE flow=branch
0x00000436	MOVE	; size=2 opcode=0x1401 flow=fallthrough
0x00000438	immediate-op	; size=6 opcode=0x0282 flow=fallthrough
0x0000043E	SWAP	; size=2 opcode=0x4842 flow=fallthrough
0x00000440	arithmetic-op	; size=2 opcode=0xEF8A flow=fallthrough
0x00000442	MOVE	; size=2 opcode=0x1600 flow=fallthrough
0x00000444	immediate-op	; size=6 opcode=0x0283 flow=fallthrough
0x0000044A	SWAP	; size=2 opcode=0x4843 flow=fallthrough
0x0000044C	arithmetic-op	; size=2 opcode=0xE383 flow=fallthrough
0x0000044E	arithmetic-op	; size=2 opcode=0xD483 flow=fallthrough
0x00000450	immediate-op	; size=6 opcode=0x0682 flow=fallthrough
0x00000456	MOVE	; size=2 opcode=0x2A82 flow=fallthrough
0x00000458	unary-op	; size=2 opcode=0x4A10 flow=fallthrough
0x0000045A	Bcc[7] 0x00046A	; size=2 opcode=0x670E flow=conditional
0x0000045C	MOVE	; size=2 opcode=0x1418 flow=fallthrough
0x0000045E	immediate-op	; size=4 opcode=0x0402 flow=fallthrough
0x00000462	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x00000466	MOVE	; size=2 opcode=0x3882 flow=fallthrough
0x00000468	BRA 0x000458	; size=2 opcode=0x60EE flow=branch
0x0000046A	RTS	; size=2 opcode=0x4E75 flow=return
0x000006C8	illegal-line-F	; size=2 opcode=0xFBFC flow=fallthrough
0x00000E36	MOVE #imm,SR	; size=4 opcode=0x46FC flow=fallthrough
0x00000E3A	MOVE	; size=2 opcode=0x3F00 flow=fallthrough
0x00000E3C	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00000E42	BSR 0x000E9E	; size=4 opcode=0x6100 flow=call
0x00000E46	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000E50	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000E5A	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000E64	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000E6C	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000E76	MOVE	; size=2 opcode=0x301F flow=fallthrough
0x00000E78		; size=2 opcode=0x46DF flow=fallthrough
0x00000E9E	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x00000EA0	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00000EA4	immediate-op	; size=4 opcode=0x0041 flow=fallthrough
0x00000EA8	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00000EAE	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x00000EB0	arithmetic-op	; size=2 opcode=0xE559 flow=fallthrough
0x00000EB2	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00000EB6	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00000EBC	MOVE	; size=2 opcode=0x221F flow=fallthrough
0x00000EBE	RTS	; size=2 opcode=0x4E75 flow=return
0x00000EC2		; size=2 opcode=0x00FF flow=fallthrough
0x00000EE8	immediate-op	; size=4 opcode=0x0050 flow=fallthrough
0x00000EEC	arithmetic-op	; size=2 opcode=0xDD02 flow=fallthrough
0x00000EEE	Bcc[5] 0x000EFA	; size=4 opcode=0x6500 flow=conditional
0x00000EF2	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00000EF8	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x00000EFA		; size=2 opcode=0x00FF flow=fallthrough
0x00000EFE	ADDQ/SUBQ	; size=2 opcode=0x5242 flow=fallthrough
0x00000F00	Bcc[7] 0x000F16	; size=2 opcode=0x6714 flow=conditional
0x00000F02	arithmetic-op	; size=2 opcode=0xE74A flow=fallthrough
0x00000F04	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00000F0A	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00000F10	BSR 0x000F9A	; size=4 opcode=0x6100 flow=call
0x00000F14	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F16	BSR 0x000E36	; size=4 opcode=0x6100 flow=call
0x00000F1A	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F1E		; size=2 opcode=0x00FF flow=fallthrough
0x00000F3E	unary-op	; size=2 opcode=0x4000 flow=fallthrough
0x00000F40	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00000F44	immediate-op	; size=4 opcode=0x0004 flow=fallthrough
0x00000F48		; size=2 opcode=0x00FF flow=fallthrough
0x00000F5C	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x00000F5E	MOVE	; size=2 opcode=0x3802 flow=fallthrough
0x00000F60	arithmetic-op	; size=2 opcode=0xD880 flow=fallthrough
0x00000F62	arithmetic-op	; size=2 opcode=0xB184 flow=fallthrough
0x00000F64	arithmetic-op	; size=2 opcode=0x0804 flow=fallthrough
0x00000F66	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00000F6A	immediate-op	; size=6 opcode=0x002E flow=fallthrough
0x00000F70	immediate-op	; size=6 opcode=0x0282 flow=fallthrough
0x00000F76	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00000F7A	MOVE	; size=2 opcode=0x3400 flow=fallthrough
0x00000F7C	unary-op	; size=2 opcode=0x4442 flow=fallthrough
0x00000F7E	MOVE	; size=2 opcode=0x2802 flow=fallthrough
0x00000F80	BSR 0x000F9A	; size=4 opcode=0x6100 flow=call
0x00000F84	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00000F88	arithmetic-op	; size=2 opcode=0xD084 flow=fallthrough
0x00000F8A	arithmetic-op	; size=2 opcode=0xD284 flow=fallthrough
0x00000F8C	arithmetic-op	; size=2 opcode=0x9444 flow=fallthrough
0x00000F8E	BSR 0x000F9A	; size=4 opcode=0x6100 flow=call
0x00000F92	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00000F96	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F9A	MOVE #imm,SR	; size=4 opcode=0x46FC flow=fallthrough
0x00000F9E	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00000FA2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00000FA8	MOVE	; size=2 opcode=0x2040 flow=fallthrough
0x00000FAA	MOVE	; size=2 opcode=0x2001 flow=fallthrough
0x00000FAC	BSR 0x000E9E	; size=4 opcode=0x6100 flow=call
0x00000FB0	MOVE	; size=2 opcode=0x3002 flow=fallthrough
0x00000FB2	arithmetic-op	; size=2 opcode=0xEC48 flow=fallthrough
0x00000FB4	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x00000FB6	Bcc[11] 0x000FDC	; size=2 opcode=0x6B24 flow=conditional
0x00000FB8	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FBA	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FBC	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FBE	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FC0	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FC2	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FC4	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FC6	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FC8	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FCA	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FCC	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FCE	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FD0	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FD2	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FD4	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FD6	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FD8	DBcc 0x000FB8	; size=4 opcode=0x51C8 flow=conditional
0x00000FDC	MOVE	; size=2 opcode=0x3002 flow=fallthrough
0x00000FDE	arithmetic-op	; size=2 opcode=0xE448 flow=fallthrough
0x00000FE0	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00000FE4	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x00000FE6	Bcc[11] 0x000FEE	; size=2 opcode=0x6B06 flow=conditional
0x00000FE8	MOVE	; size=2 opcode=0x2298 flow=fallthrough
0x00000FEA	DBcc 0x000FE8	; size=4 opcode=0x51C8 flow=conditional
0x00000FEE	arithmetic-op	; size=2 opcode=0x0802 flow=fallthrough
0x00000FF0	immediate-op	; size=4 opcode=0x0001 flow=fallthrough
0x00000FF4	MOVE	; size=2 opcode=0x3298 flow=fallthrough
0x00000FF6	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00000FFA		; size=2 opcode=0x46DF flow=fallthrough
0x00001070	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001074	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000107A	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x0000107E	RTE	; size=2 opcode=0x4E73 flow=return
0x00001080	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001086	RTE	; size=2 opcode=0x4E73 flow=return
0x00001872	ADDQ/SUBQ	; size=6 opcode=0x52B9 flow=fallthrough
0x00001878	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000187E	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x00001884	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x00001886	Bcc[7] 0x001894	; size=2 opcode=0x670C flow=conditional
0x00001888	BSR 0x000EE8	; size=4 opcode=0x6100 flow=call
0x0000188C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001894	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x0000189A	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x0000189C	Bcc[7] 0x0018A2	; size=2 opcode=0x6704 flow=conditional
0x0000189E	BSR 0x000EC2	; size=4 opcode=0x6100 flow=call
0x000018A2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018A8	Bcc[6] 0x0018B4	; size=4 opcode=0x6600 flow=conditional
0x000018AC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018B2	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x000018B4	immediate-op	; size=4 opcode=0x0002 flow=fallthrough
0x000018B8	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x000018BA	Bcc[7] 0x0018C0	; size=2 opcode=0x6704 flow=conditional
0x000018BC	BSR 0x000F1E	; size=4 opcode=0x6100 flow=call
0x000018C0	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x000018C6	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x000018C8	Bcc[7] 0x0018CE	; size=2 opcode=0x6704 flow=conditional
0x000018CA	BSR 0x000F3E	; size=4 opcode=0x6100 flow=call
0x000018CE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018D4	Bcc[7] 0x0018E0	; size=4 opcode=0x6700 flow=conditional
0x000018D8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018DE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018E0		; size=2 opcode=0x00FF flow=fallthrough
0x000018E4	Bcc[7] 0x0018F0	; size=4 opcode=0x6700 flow=conditional
0x000018E8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018EE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018F0		; size=2 opcode=0x00FF flow=fallthrough
0x000018F4	Bcc[7] 0x001900	; size=4 opcode=0x6700 flow=conditional
0x000018F8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018FE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00001900		; size=2 opcode=0x00FF flow=fallthrough
0x00001904	Bcc[7] 0x001910	; size=4 opcode=0x6700 flow=conditional
0x00001908	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000190E	RTS	; size=2 opcode=0x4E75 flow=return
0x00001910	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001914	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0000191E	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00001924	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000192A	illegal-line-F	; size=2 opcode=0xF9F1 flow=fallthrough
0x00002C92	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00002C98	MOVE	; size=6 opcode=0x3A39 flow=fallthrough
0x00002C9E	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00002CA4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00002CAA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002CB0	RTS	; size=2 opcode=0x4E75 flow=return
0x00002CB2	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00002CB8	MOVE	; size=6 opcode=0x3A39 flow=fallthrough
0x00002CBE	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00002CC4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00002CCA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00002CD2	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002CD8	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00002CDE	RTS	; size=2 opcode=0x4E75 flow=return
0x00003D18	BSR 0x0041B8	; size=4 opcode=0x6100 flow=call
0x00003D1C	BSR 0x004002	; size=4 opcode=0x6100 flow=call
0x00003D20	arithmetic-op	; size=2 opcode=0xD845 flow=fallthrough
0x00003D22	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003D26	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00003D2A	BSR 0x003D4C	; size=4 opcode=0x6100 flow=call
0x00003D2E	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00003D32	MOVE	; size=2 opcode=0x3604 flow=fallthrough
0x00003D34	immediate-op	; size=4 opcode=0x0243 flow=fallthrough
0x00003D38	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x00003D3C	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00003D40	arithmetic-op	; size=2 opcode=0x8843 flow=fallthrough
0x00003D42	ADDQ/SUBQ	; size=2 opcode=0x5041 flow=fallthrough
0x00003D44	DBcc 0x003D26	; size=4 opcode=0x51CA flow=conditional
0x00003D48	RTS	; size=2 opcode=0x4E75 flow=return
0x00003D4C	immediate-op	; size=6 opcode=0x028C flow=fallthrough
0x00003D52		; size=2 opcode=0x007F flow=fallthrough
0x00004002	immediate-op	; size=4 opcode=0x0243 flow=fallthrough
0x00004006	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x0000400A	arithmetic-op	; size=2 opcode=0xE44B flow=fallthrough
0x0000400C	arithmetic-op	; size=2 opcode=0xE94C flow=fallthrough
0x0000400E	arithmetic-op	; size=2 opcode=0xD843 flow=fallthrough
0x00004010	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00004014	EXT	; size=2 opcode=0x48C4 flow=fallthrough
0x00004016	RTS	; size=2 opcode=0x4E75 flow=return
0x000041B8		; size=2 opcode=0x00FF flow=fallthrough
0x00006148	MOVE	; size=6 opcode=0x3E39 flow=fallthrough
0x0000614E	MOVE	; size=2 opcode=0x3007 flow=fallthrough
0x00006150	ADDQ/SUBQ	; size=2 opcode=0x5347 flow=fallthrough
0x00006152	MOVE	; size=6 opcode=0x2239 flow=fallthrough
0x00006158	arithmetic-op	; size=2 opcode=0xE508 flow=fallthrough
0x0000615A	arithmetic-op	; size=2 opcode=0x9200 flow=fallthrough
0x0000615C	MOVE	; size=2 opcode=0x2241 flow=fallthrough
0x0000615E	MOVE	; size=2 opcode=0x2051 flow=fallthrough
0x00006160	MOVE	; size=4 opcode=0x2028 flow=fallthrough
0x00006164	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x00006168	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x0000616C	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00006170	immediate-op	; size=4 opcode=0x0044 flow=fallthrough
0x00006174	immediate-op	; size=4 opcode=0x0005 flow=fallthrough
0x00006178	Bcc[7] 0x006182	; size=4 opcode=0x6700 flow=conditional
0x0000617C	MOVE	; size=4 opcode=0x2028 flow=fallthrough
0x00006180	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00006182	immediate-op	; size=4 opcode=0x0026 flow=fallthrough
0x00006184	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00006186	immediate-op	; size=6 opcode=0x0028 flow=fallthrough
0x00006188	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000618C	bit-op	; size=2 opcode=0x0F5C flow=fallthrough
0x0000618E	MOVE	; size=2 opcode=0x2009 flow=fallthrough
0x00006190	ADDQ/SUBQ	; size=2 opcode=0x5800 flow=fallthrough
0x00006192	MOVE	; size=2 opcode=0x2240 flow=fallthrough
0x00006194	DBcc 0x00615E	; size=4 opcode=0x51CF flow=conditional
0x00006198	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000619E	RTS	; size=2 opcode=0x4E75 flow=return
0x000061F4	MOVE	; size=6 opcode=0x3E39 flow=fallthrough
0x000061FA	MOVE	; size=2 opcode=0x3007 flow=fallthrough
0x000061FC	ADDQ/SUBQ	; size=2 opcode=0x5347 flow=fallthrough
0x000061FE	MOVE	; size=6 opcode=0x2239 flow=fallthrough
0x00006204	arithmetic-op	; size=2 opcode=0xE708 flow=fallthrough
0x00006206	arithmetic-op	; size=2 opcode=0x9200 flow=fallthrough
0x00006208	MOVE	; size=2 opcode=0x2241 flow=fallthrough
0x0000620A	MOVE	; size=2 opcode=0x2011 flow=fallthrough
0x0000620C	MOVE	; size=4 opcode=0x3429 flow=fallthrough
0x00006210	MOVE	; size=4 opcode=0x3229 flow=fallthrough
0x00006214	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000621A	MOVE	; size=2 opcode=0x2009 flow=fallthrough
0x0000621C	ADDQ/SUBQ	; size=2 opcode=0x5000 flow=fallthrough
0x0000621E	MOVE	; size=2 opcode=0x2240 flow=fallthrough
0x00006220	DBcc 0x00620A	; size=4 opcode=0x51CF flow=conditional
0x00006224	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000622A	RTS	; size=2 opcode=0x4E75 flow=return
0x0005672C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00056732	Bcc[7] 0x05673A	; size=4 opcode=0x6700 flow=conditional
0x00056736	RTS	; size=2 opcode=0x4E75 flow=return
0x0005673A	immediate-op	; size=4 opcode=0x0014 flow=fallthrough
0x0005673E	SUBA/CMPA/ADDA	; size=2 opcode=0xDDC4 flow=fallthrough
0x00056740	Bcc[7] 0x05696C	; size=4 opcode=0x6700 flow=conditional
0x00056744	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005674C	Bcc[7] 0x05689C	; size=4 opcode=0x6700 flow=conditional
0x00056750	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056758	Bcc[7] 0x05696E	; size=4 opcode=0x6700 flow=conditional
0x0005675C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00056762	Bcc[6] 0x05696C	; size=4 opcode=0x6600 flow=conditional
0x00056766	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005676E	Bcc[7] 0x05690A	; size=4 opcode=0x6700 flow=conditional
0x00056772	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005677A	Bcc[7] 0x05690A	; size=4 opcode=0x6700 flow=conditional
0x0005677E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056786	Bcc[7] 0x05690A	; size=4 opcode=0x6700 flow=conditional
0x0005678A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056792	Bcc[7] 0x05690A	; size=4 opcode=0x6700 flow=conditional
0x00056796	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005679E	Bcc[7] 0x0569FA	; size=4 opcode=0x6700 flow=conditional
0x000567A2	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567AA	Bcc[7] 0x056A5E	; size=4 opcode=0x6700 flow=conditional
0x000567AE	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567B6	Bcc[7] 0x0567D8	; size=4 opcode=0x6700 flow=conditional
0x000567BA	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567C2	Bcc[12] 0x05696C	; size=4 opcode=0x6C00 flow=conditional
0x000567C6	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567CE	Bcc[12] 0x05682E	; size=4 opcode=0x6C00 flow=conditional
0x000567D2	BRA 0x05696C	; size=4 opcode=0x6000 flow=branch
0x000567D8	immediate-op	; size=4 opcode=0x0018 flow=fallthrough
0x000567DC	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000567E2	Bcc[10] 0x05696C	; size=4 opcode=0x6A00 flow=conditional
0x000567E6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000567EE	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000567F4	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567FC	Bcc[5] 0x056808	; size=4 opcode=0x6500 flow=conditional
0x00056800	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00056806	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056808		; size=2 opcode=0x00FF flow=fallthrough
0x0005680C	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x0005680E	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00056812	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00056818	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056820	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056828	BRA 0x05696C	; size=4 opcode=0x6000 flow=branch
0x0005682E	immediate-op	; size=4 opcode=0x0018 flow=fallthrough
0x00056832	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056838	Bcc[10] 0x05696C	; size=4 opcode=0x6A00 flow=conditional
0x0005683C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056844	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005684A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056852	Bcc[5] 0x05685E	; size=4 opcode=0x6500 flow=conditional
0x00056856	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005685C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005685E		; size=2 opcode=0x00FF flow=fallthrough
0x00056862	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x00056864	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00056868	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005686E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056876	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005687E	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056886	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005688E	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056896	BRA 0x05696C	; size=4 opcode=0x6000 flow=branch
0x0005689C	immediate-op	; size=4 opcode=0x0018 flow=fallthrough
0x000568A0	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000568A6	Bcc[10] 0x05696C	; size=4 opcode=0x6A00 flow=conditional
0x000568AA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000568B2	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000568B8	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000568C0	Bcc[5] 0x0568CC	; size=4 opcode=0x6500 flow=conditional
0x000568C4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000568CA	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000568CC		; size=2 opcode=0x00FF flow=fallthrough
0x000568D0	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x000568D2	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000568D6	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x000568DC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000568E4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000568EC	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000568F4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000568FC	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056904	BRA 0x05696C	; size=4 opcode=0x6000 flow=branch
0x0005690A	arithmetic-op	; size=2 opcode=0xC01A flow=fallthrough
0x0005690C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056912	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056918	Bcc[10] 0x05696C	; size=4 opcode=0x6A00 flow=conditional
0x0005691C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056924	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005692A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056932	Bcc[5] 0x05693E	; size=4 opcode=0x6500 flow=conditional
0x00056936	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005693C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005693E		; size=2 opcode=0x00FF flow=fallthrough
0x00056942	arithmetic-op	; size=2 opcode=0xE749 flow=fallthrough
0x00056944	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005694A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056952	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005695A	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056962	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005696A	RTS	; size=2 opcode=0x4E75 flow=return
0x0005696C	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005696E		; size=2 opcode=0x00FF flow=fallthrough
0x00056972	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00056976	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005697A	Bcc[6] 0x05696A	; size=2 opcode=0x66EE flow=conditional
0x0005697C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00056982	Bcc[10] 0x0569B2	; size=4 opcode=0x6A00 flow=conditional
0x00056986	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005698C	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x00056994	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005699C	Bcc[6] 0x0569D4	; size=4 opcode=0x6600 flow=conditional
0x000569A0	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000569A6	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000569AC	BRA 0x0569D4	; size=4 opcode=0x6000 flow=branch
0x000569B2		; size=2 opcode=0x00FF flow=fallthrough
0x000569D4		; size=2 opcode=0x00FF flow=fallthrough
0x000569FA	arithmetic-op	; size=2 opcode=0xC00C flow=fallthrough
0x000569FC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056A02	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056A08	Bcc[10] 0x05696C	; size=4 opcode=0x6A00 flow=conditional
0x00056A0C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056A14	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00056A1A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056A22	Bcc[5] 0x056A2E	; size=4 opcode=0x6500 flow=conditional
0x00056A26	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00056A2C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056A2E		; size=2 opcode=0x00FF flow=fallthrough
0x00056A32	arithmetic-op	; size=2 opcode=0xE749 flow=fallthrough
0x00056A34	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00056A3A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056A42	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056A4A	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056A52	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056A5A	RTS	; size=2 opcode=0x4E75 flow=return
0x00056A5E	arithmetic-op	; size=2 opcode=0xC014 flow=fallthrough
0x00056A60	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056A66	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056A6C	Bcc[10] 0x05696C	; size=4 opcode=0x6A00 flow=conditional
0x00056A70	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056A78	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00056A7E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056A86	Bcc[5] 0x056A92	; size=4 opcode=0x6500 flow=conditional
0x00056A8A	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00056A90	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056A92		; size=2 opcode=0x00FF flow=fallthrough
0x00056A96	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x00056A98	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00056A9E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056AA6	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056AAE	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056AB6	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056ABE	RTS	; size=2 opcode=0x4E75 flow=return
