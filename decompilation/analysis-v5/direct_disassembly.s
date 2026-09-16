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
0x0000034E	Bcc[7] 0x000434	; size=4 opcode=0x6700 flow=conditional
0x00000352	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00000358	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000035C	arithmetic-op	; size=2 opcode=0xB010 flow=fallthrough
0x0000035E	Bcc[7] 0x0006C6	; size=4 opcode=0x6700 flow=conditional
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
0x000003D6	BSR 0x000436	; size=4 opcode=0x6100 flow=call
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
0x0000040C	BSR 0x000436	; size=4 opcode=0x6100 flow=call
0x00000410	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x00000414	SUBA/CMPA/ADDA	; size=2 opcode=0xD1DA flow=fallthrough
0x00000416	MOVE	; size=2 opcode=0x1018 flow=fallthrough
0x00000418	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x0000041A	BSR 0x000436	; size=4 opcode=0x6100 flow=call
0x0000041E	BRA 0x000424	; size=2 opcode=0x6004 flow=branch
0x00000420	ADDQ/SUBQ	; size=2 opcode=0x588A flow=fallthrough
0x00000422	BRA 0x0003EA	; size=2 opcode=0x60C6 flow=branch
0x00000424	ADDQ/SUBQ	; size=2 opcode=0x5289 flow=fallthrough
0x00000426	BRA 0x0003E0	; size=2 opcode=0x60B8 flow=branch
0x00000428	LEA	; size=4 opcode=0x41FA flow=fallthrough
0x0000042C	MOVE	; size=2 opcode=0x1018 flow=fallthrough
0x0000042E	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00000430	BSR 0x000436	; size=4 opcode=0x6100 flow=call
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
0x000006C6	LEA	; size=4 opcode=0x4BFA flow=fallthrough
0x000006CA	MOVEM	; size=4 opcode=0x4C9D flow=fallthrough
0x000006CE	MOVEM	; size=4 opcode=0x4CDD flow=fallthrough
0x000006D2	MOVE	; size=2 opcode=0x3014 flow=fallthrough
0x000006D4	MOVEQ	; size=2 opcode=0x7000 flow=fallthrough
0x000006D6	MOVE	; size=2 opcode=0x2C40 flow=fallthrough
0x000006D8	MOVE USP	; size=2 opcode=0x4E66 flow=fallthrough
0x000006DA	MOVEQ	; size=2 opcode=0x7217 flow=fallthrough
0x000006DC	MOVE	; size=2 opcode=0x1A1D flow=fallthrough
0x000006DE	MOVE	; size=2 opcode=0x3885 flow=fallthrough
0x000006E0	arithmetic-op	; size=2 opcode=0xDA47 flow=fallthrough
0x000006E2	DBcc 0x0006DC	; size=4 opcode=0x51C9 flow=conditional
0x000006E6	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x000006E8	MOVE	; size=2 opcode=0x3680 flow=fallthrough
0x000006EA	MOVE	; size=6 opcode=0x2C3C flow=fallthrough
0x000006F0	MOVE	; size=6 opcode=0x2C7C flow=fallthrough
0x000006F6	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000006F8	ADDQ/SUBQ	; size=2 opcode=0x598E flow=fallthrough
0x000006FA	DBcc 0x0006F8	; size=4 opcode=0x51CE flow=conditional
0x000006FE	MOVE	; size=2 opcode=0x3287 flow=fallthrough
0x00000700	MOVE	; size=2 opcode=0x3487 flow=fallthrough
0x00000702	MOVE	; size=6 opcode=0x2A7C flow=fallthrough
0x00000708	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x0000070A	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x0000070C	MOVEQ	; size=2 opcode=0x761F flow=fallthrough
0x0000070E	MOVE	; size=2 opcode=0x2680 flow=fallthrough
0x00000710	DBcc 0x00070E	; size=4 opcode=0x51CB flow=conditional
0x00000714	MOVE	; size=2 opcode=0x289D flow=fallthrough
0x00000716	MOVEQ	; size=2 opcode=0x7813 flow=fallthrough
0x00000718	MOVE	; size=2 opcode=0x2680 flow=fallthrough
0x0000071A	DBcc 0x000718	; size=4 opcode=0x51CC flow=conditional
0x0000071E	MOVEQ	; size=2 opcode=0x7A03 flow=fallthrough
0x00000720	MOVE	; size=4 opcode=0x175D flow=fallthrough
0x00000724	DBcc 0x000720	; size=4 opcode=0x51CD flow=conditional
0x00000728	MOVE #imm,SR	; size=4 opcode=0x46FC flow=fallthrough
0x0000072C	MOVEQ	; size=2 opcode=0x7C0A flow=fallthrough
0x0000072E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000734	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0000073A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000073E	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00000740	DBcc 0x00073E	; size=4 opcode=0x51CF flow=conditional
0x00000744	DBcc 0x00073A	; size=4 opcode=0x51CE flow=conditional
0x00000748	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000074E	immediate-op	; size=4 opcode=0x0005 flow=fallthrough
0x00000752	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00000756	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00000758	DBcc 0x000756	; size=4 opcode=0x51CF flow=conditional
0x0000075C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000075E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000764	MOVE #imm,SR	; size=4 opcode=0x46FC flow=fallthrough
0x00000768	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000076E	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x00000778	Bcc[7] 0x000786	; size=4 opcode=0x6700 flow=conditional
0x0000077C	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000786	BRA 0x000796	; size=4 opcode=0x6000 flow=branch
0x0000078A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000790	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00000796	MOVE	; size=6 opcode=0x2039 flow=fallthrough
0x0000079C	arithmetic-op	; size=6 opcode=0xD179 flow=fallthrough
0x000007A2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000007AA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000007B2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000007BA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007C0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007C6	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000007CC	Bcc[7] 0x00078A	; size=2 opcode=0x67BC flow=conditional
0x000007CE	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007D4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007DA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007E0	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000007E6	Bcc[6] 0x0007F0	; size=4 opcode=0x6600 flow=conditional
0x000007EA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007F6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000007FC	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000806	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x0000080C	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00000812	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00000818	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x0000081E	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00000824	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000082A	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00000830	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00000836	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000083C	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00000842	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00000848	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x0000084E	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000858	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000860	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00000868	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000086E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000876	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0000087E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000884	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000088A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000890	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000896	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000089C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000008A2	Bcc[7] 0x0008C0	; size=4 opcode=0x6700 flow=conditional
0x000008A6	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x000008A8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000008AE	JSR	; size=6 opcode=0x4EB9 flow=call
0x000008B4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000008BA	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000008C0	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000008C4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000008CA	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000008D2	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000008D8	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000008DE	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000008E6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000008EE	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x000008F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000008F6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000008FC	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000902		; size=2 opcode=0x08F9 flow=fallthrough
0x00000BD2	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00000BD8	MOVE	; size=6 opcode=0x3E39 flow=fallthrough
0x00000BDE	arithmetic-op	; size=6 opcode=0xBE79 flow=fallthrough
0x00000BE4	Bcc[7] 0x000BDE	; size=2 opcode=0x67F8 flow=conditional
0x00000BE6	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00000BF0	RTS	; size=2 opcode=0x4E75 flow=return
0x00000BF2	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00000BF8	arithmetic-op	; size=6 opcode=0x8079 flow=fallthrough
0x00000BFE	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00000C02	Bcc[7] 0x000C22	; size=4 opcode=0x6700 flow=conditional
0x00000C06	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000C0E	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00000C14	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00000C16	Bcc[7] 0x000C22	; size=4 opcode=0x6700 flow=conditional
0x00000C1A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000C22	RTS	; size=2 opcode=0x4E75 flow=return
0x00000C24	BSR 0x000BD2	; size=2 opcode=0x61AC flow=call
0x00000C26	DBcc 0x000C24	; size=4 opcode=0x51CD flow=conditional
0x00000C2A	RTS	; size=2 opcode=0x4E75 flow=return
0x00000C2C	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x00000C36	Bcc[7] 0x000C58	; size=4 opcode=0x6700 flow=conditional
0x00000C3A	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00000C3C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00000C40	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x00000C46	arithmetic-op	; size=2 opcode=0xD290 flow=fallthrough
0x00000C48	MOVE	; size=2 opcode=0x20C0 flow=fallthrough
0x00000C4A	DBcc 0x000C46	; size=4 opcode=0x51CF flow=conditional
0x00000C4E	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00000C54	BRA 0x000C6A	; size=4 opcode=0x6000 flow=branch
0x00000C58	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x00000C5A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00000C5E	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x00000C64	MOVE	; size=2 opcode=0x10C0 flow=fallthrough
0x00000C66	DBcc 0x000C64	; size=4 opcode=0x51CF flow=conditional
0x00000C6A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000C70	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00000C74	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00000C76	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00000C7A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000C80	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000C88	BSR 0x000E34	; size=4 opcode=0x6100 flow=call
0x00000C8C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00000C92	BSR 0x000CEC	; size=4 opcode=0x6100 flow=call
0x00000C96	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00000C9A	BSR 0x000E9C	; size=4 opcode=0x6100 flow=call
0x00000C9E	MOVEQ	; size=2 opcode=0x7000 flow=fallthrough
0x00000CA0	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00000CA4	MOVE	; size=6 opcode=0x23C0 flow=fallthrough
0x00000CAA	DBcc 0x000CA4	; size=4 opcode=0x51C9 flow=conditional
0x00000CAE	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000CB8	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000CC2	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x00000CCC	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000CD2	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000CD8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000CDE	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000CE4	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000CEA	RTS	; size=2 opcode=0x4E75 flow=return
0x00000CEC	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x00000CF2	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x00000CF8	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x00000CFE	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x00000D04	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00000D08	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x00000D0E	DBcc 0x000D08	; size=4 opcode=0x51CF flow=conditional
0x00000D12	RTS	; size=2 opcode=0x4E75 flow=return
0x00000D3A	BSR 0x000D44	; size=2 opcode=0x6108 flow=call
0x00000D3C	MUL/DIV	; size=2 opcode=0xC0C1 flow=fallthrough
0x00000D3E	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x00000D40	EXT	; size=2 opcode=0x48C0 flow=fallthrough
0x00000D42	RTS	; size=2 opcode=0x4E75 flow=return
0x00000D44	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00000D46	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00000D4C	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00000D50	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00000D52	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00000D58	RTS	; size=2 opcode=0x4E75 flow=return
0x00000D94	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000D9A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000DA0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000DA6	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00000DAC	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00000DB0	Bcc[7] 0x000D94	; size=2 opcode=0x67E2 flow=conditional
0x00000DB2	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000DB8	RTS	; size=2 opcode=0x4E75 flow=return
0x00000DBA	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00000DBE	ADDQ/SUBQ	; size=2 opcode=0x5346 flow=fallthrough
0x00000DC0	Bcc[10] 0x000DC6	; size=4 opcode=0x6A00 flow=conditional
0x00000DC4	unary-op	; size=2 opcode=0x4246 flow=fallthrough
0x00000DC6	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00000DCC	MOVEQ	; size=2 opcode=0x7206 flow=fallthrough
0x00000DCE	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x00000DD0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00000DD6	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00000DD8	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00000DDC	Bcc[6] 0x000DE4	; size=4 opcode=0x6600 flow=conditional
0x00000DE0	arithmetic-op	; size=2 opcode=0xD440 flow=fallthrough
0x00000DE2	BRA 0x000DD0	; size=2 opcode=0x60EC flow=branch
0x00000DE4	arithmetic-op	; size=2 opcode=0xD042 flow=fallthrough
0x00000DE6	arithmetic-op	; size=2 opcode=0xB044 flow=fallthrough
0x00000DE8	Bcc[11] 0x000DF2	; size=4 opcode=0x6B00 flow=conditional
0x00000DEC	ADDQ/SUBQ	; size=6 opcode=0x5239 flow=fallthrough
0x00000DF2	DBcc 0x000DCE	; size=4 opcode=0x51CE flow=conditional
0x00000DF6	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00000DFA	RTS	; size=2 opcode=0x4E75 flow=return
0x00000DFC		; size=2 opcode=0x40E7 flow=fallthrough
0x00000E18		; size=2 opcode=0x40E7 flow=fallthrough
0x00000E34		; size=2 opcode=0x40E7 flow=fallthrough
0x00000E9C	MOVE	; size=2 opcode=0x2F01 flow=fallthrough
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
0x00000EC0	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00000EC6	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00000ECC	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00000ED2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00000EDA	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x00000EE0	ADDQ/SUBQ	; size=2 opcode=0x5400 flow=fallthrough
0x00000EE2	Bcc[10] 0x000ECC	; size=2 opcode=0x6AE8 flow=conditional
0x00000EE4	RTS	; size=2 opcode=0x4E75 flow=return
0x00000EE6	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00000EEE	Bcc[5] 0x000EF8	; size=4 opcode=0x6500 flow=conditional
0x00000EF2	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00000EF8	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x00000EFE	ADDQ/SUBQ	; size=2 opcode=0x5242 flow=fallthrough
0x00000F00	Bcc[7] 0x000F16	; size=2 opcode=0x6714 flow=conditional
0x00000F02	arithmetic-op	; size=2 opcode=0xE74A flow=fallthrough
0x00000F04	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00000F0A	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00000F10	BSR 0x000F98	; size=4 opcode=0x6100 flow=call
0x00000F14	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F16	BSR 0x000E34	; size=4 opcode=0x6100 flow=call
0x00000F1A	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F1C	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00000F22	BSR 0x000E9C	; size=4 opcode=0x6100 flow=call
0x00000F26	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00000F30	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00000F3A	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F3C	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00000F46	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00000F50	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00000F5A	RTS	; size=2 opcode=0x4E75 flow=return
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
0x00000F80	BSR 0x000F98	; size=4 opcode=0x6100 flow=call
0x00000F84	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00000F88	arithmetic-op	; size=2 opcode=0xD084 flow=fallthrough
0x00000F8A	arithmetic-op	; size=2 opcode=0xD284 flow=fallthrough
0x00000F8C	arithmetic-op	; size=2 opcode=0x9444 flow=fallthrough
0x00000F8E	BSR 0x000F98	; size=4 opcode=0x6100 flow=call
0x00000F92	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00000F96	RTS	; size=2 opcode=0x4E75 flow=return
0x00000F98		; size=2 opcode=0x40E7 flow=fallthrough
0x00000FFE		; size=2 opcode=0x40E7 flow=fallthrough
0x0000102A		; size=2 opcode=0x40E7 flow=fallthrough
0x00001070	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001074	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000107A	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x0000107E	RTE	; size=2 opcode=0x4E73 flow=return
0x00001080	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001086	RTE	; size=2 opcode=0x4E73 flow=return
0x00001088	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000108E	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00001092	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x00001094	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x00001096	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x0000109A	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0000109E	Bcc[7] 0x0010A4	; size=4 opcode=0x6700 flow=conditional
0x000010A2	ADDQ/SUBQ	; size=2 opcode=0x5540 flow=fallthrough
0x000010A4	arithmetic-op	; size=2 opcode=0x8041 flow=fallthrough
0x000010A6	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x000010A8	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x000010AC	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000010B0	Bcc[7] 0x0010B8	; size=4 opcode=0x6700 flow=conditional
0x000010B4	immediate-op	; size=4 opcode=0x0440 flow=fallthrough
0x000010B8	arithmetic-op	; size=2 opcode=0x8041 flow=fallthrough
0x000010BA	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x000010BC	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x000010C0	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000010C4	Bcc[7] 0x0010CC	; size=4 opcode=0x6700 flow=conditional
0x000010C8	immediate-op	; size=4 opcode=0x0440 flow=fallthrough
0x000010CC	arithmetic-op	; size=2 opcode=0x8041 flow=fallthrough
0x000010CE	MOVE	; size=2 opcode=0x30C0 flow=fallthrough
0x000010D0	DBcc 0x001092	; size=4 opcode=0x51CF flow=conditional
0x000010D4	RTS	; size=2 opcode=0x4E75 flow=return
0x000010D6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000010DC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000010E2	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000010E6	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x000010E8	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x000010EA	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x000010EE	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000010F2	MOVE	; size=2 opcode=0x3411 flow=fallthrough
0x000010F4	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x000010F8	arithmetic-op	; size=2 opcode=0xB042 flow=fallthrough
0x000010FA	Bcc[7] 0x001100	; size=4 opcode=0x6700 flow=conditional
0x000010FE	ADDQ/SUBQ	; size=2 opcode=0x5440 flow=fallthrough
0x00001100	arithmetic-op	; size=2 opcode=0x8041 flow=fallthrough
0x00001102	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x00001104	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00001108	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0000110C	MOVE	; size=2 opcode=0x3411 flow=fallthrough
0x0000110E	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x00001112	arithmetic-op	; size=2 opcode=0xB042 flow=fallthrough
0x00001114	Bcc[7] 0x00111C	; size=4 opcode=0x6700 flow=conditional
0x00001118	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x0000111C	arithmetic-op	; size=2 opcode=0x8041 flow=fallthrough
0x0000111E	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x00001120	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00001124	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00001128	MOVE	; size=2 opcode=0x3419 flow=fallthrough
0x0000112A	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x0000112E	arithmetic-op	; size=2 opcode=0xB042 flow=fallthrough
0x00001130	Bcc[7] 0x001138	; size=4 opcode=0x6700 flow=conditional
0x00001134	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x00001138	arithmetic-op	; size=2 opcode=0x8041 flow=fallthrough
0x0000113A	MOVE	; size=2 opcode=0x30C0 flow=fallthrough
0x0000113C	DBcc 0x0010E6	; size=4 opcode=0x51CF flow=conditional
0x00001140	RTS	; size=2 opcode=0x4E75 flow=return
0x000011C0	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x000011C4		; size=2 opcode=0x40E7 flow=fallthrough
0x00001232	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001236		; size=2 opcode=0x40E7 flow=fallthrough
0x000012F6	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000012FA	MOVE	; size=2 opcode=0x3F06 flow=fallthrough
0x000012FC	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x00001300	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001306	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000130C		; size=2 opcode=0x08F9 flow=fallthrough
0x0000131C	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00001320	MOVE	; size=2 opcode=0x3F06 flow=fallthrough
0x00001322	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x00001326	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000132C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001332		; size=2 opcode=0x08F9 flow=fallthrough
0x00001342	MOVE	; size=2 opcode=0x3F05 flow=fallthrough
0x00001344	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000134A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001350	MOVE	; size=2 opcode=0x3A1F flow=fallthrough
0x00001352	DBcc 0x001342	; size=4 opcode=0x51CD flow=conditional
0x00001356	RTS	; size=2 opcode=0x4E75 flow=return
0x00001358	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000135E	MOVEQ	; size=2 opcode=0x701F flow=fallthrough
0x00001360	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x00001362	DBcc 0x001360	; size=4 opcode=0x51C8 flow=conditional
0x00001366	RTS	; size=2 opcode=0x4E75 flow=return
0x00001368	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000136E	MOVEQ	; size=2 opcode=0x701F flow=fallthrough
0x00001370	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x00001372	DBcc 0x001370	; size=4 opcode=0x51C8 flow=conditional
0x00001376	RTS	; size=2 opcode=0x4E75 flow=return
0x00001378	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x0000137A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000137E	Bcc[6] 0x001384	; size=4 opcode=0x6600 flow=conditional
0x00001382	RTS	; size=2 opcode=0x4E75 flow=return
0x00001384	arithmetic-op	; size=2 opcode=0xD041 flow=fallthrough
0x00001386	MOVE	; size=4 opcode=0x3740 flow=fallthrough
0x0000138A	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x0000138C	arithmetic-op	; size=2 opcode=0xD042 flow=fallthrough
0x0000138E	MOVE	; size=2 opcode=0x3680 flow=fallthrough
0x00001390	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x00001392	MOVE	; size=4 opcode=0x1740 flow=fallthrough
0x00001396	MOVE	; size=4 opcode=0x1745 flow=fallthrough
0x0000139A	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x0000139C	arithmetic-op	; size=2 opcode=0x8046 flow=fallthrough
0x0000139E	MOVE	; size=4 opcode=0x3740 flow=fallthrough
0x000013A2	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x000013A4	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000013AA	ADDQ/SUBQ	; size=2 opcode=0x508B flow=fallthrough
0x000013AC	MOVE	; size=6 opcode=0x23CB flow=fallthrough
0x000013B2	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x000013BC	BRA 0x001378	; size=2 opcode=0x60BA flow=branch
0x000013BE	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x000013C0	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x000013C4	Bcc[6] 0x0013CA	; size=4 opcode=0x6600 flow=conditional
0x000013C8	RTS	; size=2 opcode=0x4E75 flow=return
0x000013CA	arithmetic-op	; size=2 opcode=0xD041 flow=fallthrough
0x000013CC	MOVE	; size=4 opcode=0x3740 flow=fallthrough
0x000013D0	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x000013D2	arithmetic-op	; size=2 opcode=0xD042 flow=fallthrough
0x000013D4	MOVE	; size=2 opcode=0x3680 flow=fallthrough
0x000013D6	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x000013D8	MOVE	; size=4 opcode=0x1740 flow=fallthrough
0x000013DC	MOVE	; size=4 opcode=0x1745 flow=fallthrough
0x000013E0	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x000013E2	arithmetic-op	; size=2 opcode=0xD044 flow=fallthrough
0x000013E4	arithmetic-op	; size=2 opcode=0x8046 flow=fallthrough
0x000013E6	MOVE	; size=4 opcode=0x3740 flow=fallthrough
0x000013EA	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x000013EC	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000013F2	ADDQ/SUBQ	; size=2 opcode=0x508B flow=fallthrough
0x000013F4	MOVE	; size=6 opcode=0x23CB flow=fallthrough
0x000013FA	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00001404	BRA 0x0013BE	; size=2 opcode=0x60B8 flow=branch
0x00001406	BSR 0x00131C	; size=4 opcode=0x6100 flow=call
0x0000140A	MOVE	; size=2 opcode=0x205E flow=fallthrough
0x0000140C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001412	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001418	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000141E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001424	MOVE	; size=2 opcode=0x205E flow=fallthrough
0x00001426	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000142C	MOVE	; size=6 opcode=0x33DE flow=fallthrough
0x00001432	MOVE	; size=6 opcode=0x33DE flow=fallthrough
0x00001438	MOVE	; size=6 opcode=0x33DE flow=fallthrough
0x0000143E	MOVE	; size=6 opcode=0x33DE flow=fallthrough
0x00001444	MOVE	; size=2 opcode=0x201E flow=fallthrough
0x00001446	Bcc[7] 0x001476	; size=4 opcode=0x6700 flow=conditional
0x0000144A	arithmetic-op	; size=2 opcode=0x0800 flow=fallthrough
0x0000144C	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x00001450	immediate-op	; size=4 opcode=0x001A flow=fallthrough
0x00001454	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x00001458	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000145E	MOVE	; size=2 opcode=0x221E flow=fallthrough
0x00001460	MOVE	; size=2 opcode=0x241E flow=fallthrough
0x00001462	MOVE	; size=2 opcode=0x3001 flow=fallthrough
0x00001464	BSR 0x001530	; size=4 opcode=0x6100 flow=call
0x00001468	BRA 0x001444	; size=2 opcode=0x60DA flow=branch
0x00001476	MOVE	; size=2 opcode=0x201E flow=fallthrough
0x00001478	Bcc[7] 0x0014A4	; size=4 opcode=0x6700 flow=conditional
0x0000147C	MOVE	; size=2 opcode=0x2240 flow=fallthrough
0x0000147E	MOVE	; size=2 opcode=0x281E flow=fallthrough
0x00001480	MOVE	; size=2 opcode=0x3E1E flow=fallthrough
0x00001482	MOVE	; size=2 opcode=0x321E flow=fallthrough
0x00001484	MOVE	; size=2 opcode=0x341E flow=fallthrough
0x00001486	arithmetic-op	; size=2 opcode=0x0880 flow=fallthrough
0x00001488	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x0000148C	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00001490	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x00001492	MOVE	; size=2 opcode=0x3A07 flow=fallthrough
0x00001494	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x00001496	BSR 0x001728	; size=4 opcode=0x6100 flow=call
0x0000149A	BRA 0x001476	; size=2 opcode=0x60DA flow=branch
0x000014A4	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x000014A6	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x000014A8	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x000014AA		; size=2 opcode=0x08F9 flow=fallthrough
0x000014CE	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x000014D4	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x000014D6	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x000014D8	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x000014DC	MOVE	; size=2 opcode=0x12D0 flow=fallthrough
0x000014DE	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x000014E2	DBcc 0x0014DC	; size=4 opcode=0x51CF flow=conditional
0x000014E6	RTS	; size=2 opcode=0x4E75 flow=return
0x00001502	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x00001508	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0000150A	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x0000150C	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x00001510	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x00001516	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x00001518	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0000151A	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x0000151C	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00001520	MOVE	; size=2 opcode=0x1290 flow=fallthrough
0x00001522	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00001526	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x0000152A	DBcc 0x001520	; size=4 opcode=0x51CF flow=conditional
0x0000152E	RTS	; size=2 opcode=0x4E75 flow=return
0x00001530	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001534	unary-op	; size=2 opcode=0x4246 flow=fallthrough
0x00001536	MOVE	; size=2 opcode=0x341B flow=fallthrough
0x00001538	MOVE	; size=2 opcode=0x3F02 flow=fallthrough
0x0000153A		; size=2 opcode=0x40E7 flow=fallthrough
0x00001610	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001614	MOVE	; size=2 opcode=0x341B flow=fallthrough
0x00001616	MOVE	; size=2 opcode=0x3F02 flow=fallthrough
0x00001618	MOVEQ	; size=2 opcode=0x7201 flow=fallthrough
0x0000161A	ADDQ/SUBQ	; size=2 opcode=0x5341 flow=fallthrough
0x0000161C	Bcc[6] 0x001622	; size=2 opcode=0x6604 flow=conditional
0x0000161E	MOVE	; size=2 opcode=0x101B flow=fallthrough
0x00001620	MOVEQ	; size=2 opcode=0x7208 flow=fallthrough
0x00001622	arithmetic-op	; size=2 opcode=0xD000 flow=fallthrough
0x00001624	Bcc[5] 0x00162E	; size=2 opcode=0x6508 flow=conditional
0x00001626	MOVE	; size=2 opcode=0x12DB flow=fallthrough
0x00001628	DBcc 0x00161A	; size=4 opcode=0x51CA flow=conditional
0x0000162C	BRA 0x001670	; size=2 opcode=0x6042 flow=branch
0x0000162E	MOVE	; size=2 opcode=0x161B flow=fallthrough
0x00001630	Bcc[11] 0x001648	; size=2 opcode=0x6B16 flow=conditional
0x00001632	immediate-op	; size=4 opcode=0x0243 flow=fallthrough
0x00001636	ADDQ/SUBQ	; size=2 opcode=0x5443 flow=fallthrough
0x00001638	arithmetic-op	; size=2 opcode=0x9443 flow=fallthrough
0x0000163A	MOVE	; size=2 opcode=0x181B flow=fallthrough
0x0000163C	MOVE	; size=2 opcode=0x12C4 flow=fallthrough
0x0000163E	DBcc 0x00163C	; size=4 opcode=0x51CB flow=conditional
0x00001642	DBcc 0x00161A	; size=4 opcode=0x51CA flow=conditional
0x00001646	BRA 0x001670	; size=2 opcode=0x6028 flow=branch
0x00001648	MOVE	; size=2 opcode=0x181B flow=fallthrough
0x0000164A	arithmetic-op	; size=2 opcode=0xE74C flow=fallthrough
0x0000164C	MOVE	; size=2 opcode=0x1A03 flow=fallthrough
0x0000164E	arithmetic-op	; size=2 opcode=0xE80D flow=fallthrough
0x00001650	immediate-op	; size=4 opcode=0x0245 flow=fallthrough
0x00001654	arithmetic-op	; size=2 opcode=0x8805 flow=fallthrough
0x00001656	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x0000165A	immediate-op	; size=4 opcode=0x0243 flow=fallthrough
0x0000165E	ADDQ/SUBQ	; size=2 opcode=0x5443 flow=fallthrough
0x00001660	arithmetic-op	; size=2 opcode=0x9443 flow=fallthrough
0x00001662	MOVE	; size=2 opcode=0x2449 flow=fallthrough
0x00001664	SUBA/CMPA/ADDA	; size=2 opcode=0x94C4 flow=fallthrough
0x00001666	MOVE	; size=2 opcode=0x12DA flow=fallthrough
0x00001668	DBcc 0x001666	; size=4 opcode=0x51CB flow=conditional
0x0000166C	DBcc 0x00161A	; size=4 opcode=0x51CA flow=conditional
0x00001670	MOVEM	; size=4 opcode=0x4C9F flow=fallthrough
0x00001674	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00001676	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x0000167A	RTS	; size=2 opcode=0x4E75 flow=return
0x00001728		; size=2 opcode=0x40E7 flow=fallthrough
0x00001872	ADDQ/SUBQ	; size=6 opcode=0x52B9 flow=fallthrough
0x00001878	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000187E	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x00001884	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x00001886	Bcc[7] 0x001894	; size=2 opcode=0x670C flow=conditional
0x00001888	BSR 0x000EE6	; size=4 opcode=0x6100 flow=call
0x0000188C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001894	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x0000189A	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x0000189C	Bcc[7] 0x0018A2	; size=2 opcode=0x6704 flow=conditional
0x0000189E	BSR 0x000EC0	; size=4 opcode=0x6100 flow=call
0x000018A2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018A8	Bcc[6] 0x0018B2	; size=4 opcode=0x6600 flow=conditional
0x000018AC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018B2	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x000018B8	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x000018BA	Bcc[7] 0x0018C0	; size=2 opcode=0x6704 flow=conditional
0x000018BC	BSR 0x000F1C	; size=4 opcode=0x6100 flow=call
0x000018C0	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x000018C6	arithmetic-op	; size=2 opcode=0xEF14 flow=fallthrough
0x000018C8	Bcc[7] 0x0018CE	; size=2 opcode=0x6704 flow=conditional
0x000018CA	BSR 0x000F3C	; size=4 opcode=0x6100 flow=call
0x000018CE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018D4	Bcc[7] 0x0018DE	; size=4 opcode=0x6700 flow=conditional
0x000018D8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018DE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018E4	Bcc[7] 0x0018EE	; size=4 opcode=0x6700 flow=conditional
0x000018E8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018EE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000018F4	Bcc[7] 0x0018FE	; size=4 opcode=0x6700 flow=conditional
0x000018F8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000018FE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00001904	Bcc[7] 0x00190E	; size=4 opcode=0x6700 flow=conditional
0x00001908	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000190E	RTS	; size=2 opcode=0x4E75 flow=return
0x00001910	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00001914	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0000191E	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00001924	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000192A	illegal-line-F	; size=2 opcode=0xF9F1 flow=fallthrough
0x0000195C	BSR 0x0019F2	; size=4 opcode=0x6100 flow=call
0x00001960	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00001964	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0000196A	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00001970	arithmetic-op	; size=2 opcode=0xB141 flow=fallthrough
0x00001972	arithmetic-op	; size=2 opcode=0xC041 flow=fallthrough
0x00001974	arithmetic-op	; size=2 opcode=0xE148 flow=fallthrough
0x00001976	arithmetic-op	; size=6 opcode=0x8079 flow=fallthrough
0x0000197C	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00001982	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00001986	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x00001988	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000198E	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00001996	BRA 0x0019D0	; size=4 opcode=0x6000 flow=branch
0x000019D0	RTS	; size=2 opcode=0x4E75 flow=return
0x000019F2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000019F8	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00001A00	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00001A08	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001A0C	DBcc 0x001A0C	; size=4 opcode=0x51C9 flow=conditional
0x00001A10	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x00001A16	arithmetic-op	; size=2 opcode=0xE500 flow=fallthrough
0x00001A18	immediate-op	; size=4 opcode=0x0200 flow=fallthrough
0x00001A1C	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00001A24	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001A28	DBcc 0x001A28	; size=4 opcode=0x51C9 flow=conditional
0x00001A2C	MOVE	; size=6 opcode=0x1239 flow=fallthrough
0x00001A32	immediate-op	; size=4 opcode=0x0201 flow=fallthrough
0x00001A36	arithmetic-op	; size=2 opcode=0x8001 flow=fallthrough
0x00001A38	unary-op	; size=2 opcode=0x4600 flow=fallthrough
0x00001A3A	MOVE	; size=6 opcode=0x13C0 flow=fallthrough
0x00001A40	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001A46	RTS	; size=2 opcode=0x4E75 flow=return
0x00001A9E	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00001AA4	BSR 0x001D7A	; size=4 opcode=0x6100 flow=call
0x00001AA8	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001AB0	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00001ABA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001AC0	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001AC6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001ACE	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001AD4	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00001ADA	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00001AE0	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001AE6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001AEE	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001AF6	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001AFC	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00001B00	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x00001B02	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001B08	BSR 0x001C1E	; size=4 opcode=0x6100 flow=call
0x00001B0C	BSR 0x002634	; size=4 opcode=0x6100 flow=call
0x00001B10	BSR 0x0026D0	; size=4 opcode=0x6100 flow=call
0x00001B14	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001B1A	BSR 0x001BEA	; size=4 opcode=0x6100 flow=call
0x00001B1E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001B24	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00001B2A	Bcc[10] 0x001B62	; size=4 opcode=0x6A00 flow=conditional
0x00001B2E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00001B36	Bcc[14] 0x001B62	; size=4 opcode=0x6E00 flow=conditional
0x00001B3A	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00001B40	Bcc[10] 0x001B52	; size=4 opcode=0x6A00 flow=conditional
0x00001B44	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001B4C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001B52	BSR 0x001C6A	; size=4 opcode=0x6100 flow=call
0x00001B56		; size=2 opcode=0x08F9 flow=fallthrough
0x00001B62	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001B68	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x00001B6A	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00001B6E	Bcc[4] 0x001B96	; size=4 opcode=0x6400 flow=conditional
0x00001B72	MOVE	; size=2 opcode=0x3007 flow=fallthrough
0x00001B74	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00001B78	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00001B7C	Bcc[5] 0x001B88	; size=4 opcode=0x6500 flow=conditional
0x00001B80	BSR 0x001DCA	; size=4 opcode=0x6100 flow=call
0x00001B84	BRA 0x001B8C	; size=4 opcode=0x6000 flow=branch
0x00001B88	BSR 0x001DE0	; size=4 opcode=0x6100 flow=call
0x00001B8C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00001B92	Bcc[6] 0x001B9A	; size=4 opcode=0x6600 flow=conditional
0x00001B96	DBcc 0x001B00	; size=4 opcode=0x51CF flow=conditional
0x00001B9A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001BA0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001BA6	BSR 0x001BAC	; size=4 opcode=0x6100 flow=call
0x00001BAA	RTS	; size=2 opcode=0x4E75 flow=return
0x00001BAC	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x00001BB2	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x00001BB4	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00001BB8	Bcc[6] 0x001BE2	; size=4 opcode=0x6600 flow=conditional
0x00001BBC	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x00001BC6	Bcc[6] 0x001BE2	; size=4 opcode=0x6600 flow=conditional
0x00001BCA	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x00001BD4	Bcc[6] 0x001BE2	; size=4 opcode=0x6600 flow=conditional
0x00001BD8	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00001BE0	RTS	; size=2 opcode=0x4E75 flow=return
0x00001BE2	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00001BE8	RTS	; size=2 opcode=0x4E75 flow=return
0x00001BEA	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00001BF0	arithmetic-op	; size=2 opcode=0x0880 flow=fallthrough
0x00001BF2	immediate-op	; size=4 opcode=0x000F flow=fallthrough
0x00001BF6	unary-op	; size=2 opcode=0x4A00 flow=fallthrough
0x00001BF8	Bcc[7] 0x001C1C	; size=4 opcode=0x6700 flow=conditional
0x00001BFC	MOVE	; size=6 opcode=0x1239 flow=fallthrough
0x00001C02	EXT	; size=2 opcode=0x4881 flow=fallthrough
0x00001C04	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x00001C08	Bcc[14] 0x001C1C	; size=4 opcode=0x6E00 flow=conditional
0x00001C0C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00001C12	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C1 flow=fallthrough
0x00001C14	MOVE	; size=2 opcode=0x1080 flow=fallthrough
0x00001C16	ADDQ/SUBQ	; size=6 opcode=0x5239 flow=fallthrough
0x00001C1C	RTS	; size=2 opcode=0x4E75 flow=return
0x00001C1E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00001C26	Bcc[6] 0x001C68	; size=4 opcode=0x6600 flow=conditional
0x00001C2A	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00001C30	Bcc[11] 0x001C52	; size=4 opcode=0x6B00 flow=conditional
0x00001C34	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00001C3A	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00001C3E	Bcc[6] 0x001C68	; size=4 opcode=0x6600 flow=conditional
0x00001C42	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001C48		; size=2 opcode=0x08F9 flow=fallthrough
0x00001C52	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001C5A	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00001C5E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001C64	BSR 0x001DF6	; size=4 opcode=0x6100 flow=call
0x00001C68	RTS	; size=2 opcode=0x4E75 flow=return
0x00001C6A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001C72	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x00001C78	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x00001C7C	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00001C82	Bcc[10] 0x001CA6	; size=4 opcode=0x6A00 flow=conditional
0x00001C86	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001C8E	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00001C94	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00001C9C	Bcc[13] 0x001CA6	; size=4 opcode=0x6D00 flow=conditional
0x00001CA0	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001CA6	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00001CAC	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x00001CAE	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x00001CB0	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00001CB6	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x00001CBA	ADDQ/SUBQ	; size=6 opcode=0x5879 flow=fallthrough
0x00001CC0	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00001CC6	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00001CCA	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00001CCE	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001CD4	ADDQ/SUBQ	; size=2 opcode=0x514B flow=fallthrough
0x00001CD6	unary-op	; size=4 opcode=0x422B flow=fallthrough
0x00001CDA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001CE2	RTS	; size=2 opcode=0x4E75 flow=return
0x00001CFC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001D02	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x00001D08	MOVEQ	; size=2 opcode=0x7020 flow=fallthrough
0x00001D0A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001D10	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00001D1A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001D22	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001D28	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x00001D2E	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00001D34	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001D3A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001D40	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x00001D46	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00001D4C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001D52	BSR 0x001D58	; size=4 opcode=0x6100 flow=call
0x00001D56	RTS	; size=2 opcode=0x4E75 flow=return
0x00001D58	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001D5E	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00001D62	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00001D66	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001D6A	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00001D6E	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x00001D70	MOVE	; size=2 opcode=0x3A07 flow=fallthrough
0x00001D72	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x00001D74	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00001D7A	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001D80	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001D86	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001D8C	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00001D92	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x00001D98	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001D9E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001DA4	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001DAA		; size=2 opcode=0x08F9 flow=fallthrough
0x00001DCA	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00001DD0	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001DD6	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001DDA	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00001DE0	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00001DE6	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001DEC	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001DF0	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00001DF6	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00001DFC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001E02	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001E06	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001E0C	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00001E12	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001E18	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001E1C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001E22	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00001E28	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001E2E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001E32	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001E38	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00001E3E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001E44	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001E48	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00001E4E	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00001E54	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00001E58	Bcc[6] 0x001E66	; size=4 opcode=0x6600 flow=conditional
0x00001E5C	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00001E60	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001E66	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x00001E6C	MOVE	; size=6 opcode=0x3839 flow=fallthrough
0x00001E72	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x00001E76	Bcc[4] 0x001E9E	; size=4 opcode=0x6400 flow=conditional
0x00001E7A	arithmetic-op	; size=2 opcode=0xE34C flow=fallthrough
0x00001E7C	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x00001E80	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00001E84	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00001E88	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00001E8C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00001E92	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00001E98	ADDQ/SUBQ	; size=6 opcode=0x54B9 flow=fallthrough
0x00001E9E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00001EA6	Bcc[6] 0x001EC8	; size=4 opcode=0x6600 flow=conditional
0x00001EAA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00001EB2	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00001EB8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00001EBE	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00001EC2	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x00001EC4	DBcc 0x001EC2	; size=4 opcode=0x51CF flow=conditional
0x00001EC8	RTS	; size=2 opcode=0x4E75 flow=return
0x00002634	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0000263C	Bcc[7] 0x002642	; size=4 opcode=0x6700 flow=conditional
0x00002640	RTS	; size=2 opcode=0x4E75 flow=return
0x00002642	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00002646	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000264C	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00002652	Bcc[10] 0x0026CE	; size=4 opcode=0x6A00 flow=conditional
0x00002656	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0000265E	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00002664	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0000266C	Bcc[5] 0x002676	; size=4 opcode=0x6500 flow=conditional
0x00002670	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00002676	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0000267C	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x0000267E	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00002684	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0000268C	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00002694	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0000269C	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000026A4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000026AC	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000026B4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000026BC	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000026C4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000026CC	RTS	; size=2 opcode=0x4E75 flow=return
0x000026CE	RTS	; size=2 opcode=0x4E75 flow=return
0x000026D0	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000026D6	Bcc[10] 0x0026F0	; size=4 opcode=0x6A00 flow=conditional
0x000026DA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000026E2	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000026E8		; size=2 opcode=0x08F9 flow=fallthrough
0x000026F0	RTS	; size=2 opcode=0x4E75 flow=return
0x0000281E	BSR 0x0028AA	; size=4 opcode=0x6100 flow=call
0x00002822	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00002828	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00002830	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00002838	BSR 0x003074	; size=4 opcode=0x6100 flow=call
0x0000283C	BRA 0x002852	; size=4 opcode=0x6000 flow=branch
0x00002852	BSR 0x002900	; size=4 opcode=0x6100 flow=call
0x00002856	BSR 0x002B34	; size=4 opcode=0x6100 flow=call
0x0000285A	MOVE	; size=6 opcode=0x2C79 flow=fallthrough
0x00002860	BSR 0x002C6C	; size=4 opcode=0x6100 flow=call
0x00002864	BSR 0x00301E	; size=4 opcode=0x6100 flow=call
0x00002868	BSR 0x00286E	; size=4 opcode=0x6100 flow=call
0x0000286C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000286E	MOVE	; size=6 opcode=0x2839 flow=fallthrough
0x00002874	Bcc[6] 0x00287A	; size=4 opcode=0x6600 flow=conditional
0x00002878	RTS	; size=2 opcode=0x4E75 flow=return
0x0000287A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00002880	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00002884	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x00002888	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000288C	immediate-op	; size=4 opcode=0x000E flow=fallthrough
0x00002890	immediate-op	; size=4 opcode=0x0007 flow=fallthrough
0x00002894	Bcc[7] 0x00289A	; size=4 opcode=0x6700 flow=conditional
0x00002898	RTS	; size=2 opcode=0x4E75 flow=return
0x0000289A	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x0000289E	DBcc 0x002884	; size=4 opcode=0x51CF flow=conditional
0x000028A2	MOVE	; size=2 opcode=0x2C44 flow=fallthrough
0x000028A4	JMP	; size=6 opcode=0x4EF9 flow=branch
0x000028AA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000028B0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000028B6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000028BC	JMP	; size=6 opcode=0x4EF9 flow=branch
0x000028C2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000028CA	BSR 0x002900	; size=4 opcode=0x6100 flow=call
0x000028CE	BSR 0x00286E	; size=2 opcode=0x619E flow=call
0x000028D0	BSR 0x0028AA	; size=2 opcode=0x61D8 flow=call
0x000028D2	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x000028DA	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x000028E2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000028EA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000028F2	MOVE	; size=6 opcode=0x2C79 flow=fallthrough
0x000028F8	BSR 0x002C6C	; size=4 opcode=0x6100 flow=call
0x000028FC	BRA 0x00301E	; size=4 opcode=0x6000 flow=branch
0x00002900	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00002906	Bcc[7] 0x00291C	; size=4 opcode=0x6700 flow=conditional
0x0000290A	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00002910	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00002916	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000291C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00002922	Bcc[7] 0x0029BC	; size=4 opcode=0x6700 flow=conditional
0x00002926	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000292C	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x00002930	Bcc[7] 0x00293A	; size=4 opcode=0x6700 flow=conditional
0x00002934	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000293A	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00002940	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00002946	Bcc[7] 0x002956	; size=4 opcode=0x6700 flow=conditional
0x0000294A	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00002950	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002956	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000295C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002962	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002968	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000296E	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00002976	Bcc[6] 0x002986	; size=4 opcode=0x6600 flow=conditional
0x0000297A	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00002982	Bcc[6] 0x00298C	; size=4 opcode=0x6600 flow=conditional
0x00002986	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000298C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00002992	Bcc[7] 0x0029A6	; size=4 opcode=0x6700 flow=conditional
0x00002996	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000299C	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000029A2	BRA 0x0029AA	; size=4 opcode=0x6000 flow=branch
0x000029A6	BSR 0x00313A	; size=4 opcode=0x6100 flow=call
0x000029AA	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000029B2	Bcc[6] 0x0029BC	; size=4 opcode=0x6600 flow=conditional
0x000029B6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000029BC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000029C2	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x000029CA	Bcc[13] 0x0029D4	; size=4 opcode=0x6D00 flow=conditional
0x000029CE	JSR	; size=6 opcode=0x4EB9 flow=call
0x000029D4	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000029DC	Bcc[6] 0x0029EA	; size=4 opcode=0x6600 flow=conditional
0x000029E0	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000029E6	Bcc[7] 0x0029EA	; size=4 opcode=0x6700 flow=conditional
0x000029EA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000029F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000029F6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000029FC	MOVE	; size=6 opcode=0x2039 flow=fallthrough
0x00002A02	arithmetic-op	; size=2 opcode=0x0880 flow=fallthrough
0x00002A04	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x00002A08	immediate-op	; size=4 opcode=0x0024 flow=fallthrough
0x00002A0C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00002A12	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002A18	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00002A22	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00002A2C	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00002B34	unary-op	; size=6 opcode=0x4AB9 flow=fallthrough
0x00002B3A	Bcc[6] 0x002B78	; size=4 opcode=0x6600 flow=conditional
0x00002B3E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00002B44	MOVE	; size=6 opcode=0x30B9 flow=fallthrough
0x00002B4A	MOVE	; size=8 opcode=0x3179 flow=fallthrough
0x00002B52	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00002B56	MOVE	; size=6 opcode=0x30B9 flow=fallthrough
0x00002B5C	MOVE	; size=8 opcode=0x3179 flow=fallthrough
0x00002B64	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00002B68	MOVE	; size=6 opcode=0x30B9 flow=fallthrough
0x00002B6E	MOVE	; size=8 opcode=0x3179 flow=fallthrough
0x00002B76	RTS	; size=2 opcode=0x4E75 flow=return
0x00002B78	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x00002B7E	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00002B84	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00002B8A	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00002B90	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x00002B98	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x00002BA0	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00002BA8	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00002BB0	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00002BB6	MOVE	; size=2 opcode=0x3099 flow=fallthrough
0x00002BB8	MOVE	; size=4 opcode=0x3159 flow=fallthrough
0x00002BBC	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00002BC0	MOVE	; size=2 opcode=0x3099 flow=fallthrough
0x00002BC2	MOVE	; size=4 opcode=0x3159 flow=fallthrough
0x00002BC6	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00002BCA	MOVE	; size=2 opcode=0x3099 flow=fallthrough
0x00002BCC	MOVE	; size=4 opcode=0x3159 flow=fallthrough
0x00002BD0	RTS	; size=2 opcode=0x4E75 flow=return
0x00002C6C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002C72	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002C78	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00002C80	Bcc[6] 0x002C8A	; size=4 opcode=0x6600 flow=conditional
0x00002C84	JSR	; size=6 opcode=0x4EB9 flow=call
0x00002C8A	BSR 0x002C92	; size=4 opcode=0x6100 flow=call
0x00002C8E	BRA 0x002CB2	; size=4 opcode=0x6000 flow=branch
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
0x0000301E	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00003024	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003028	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0000302C	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00003032	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00003036	Bcc[7] 0x00306A	; size=4 opcode=0x6700 flow=conditional
0x0000303A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000303E	Bcc[7] 0x00306A	; size=4 opcode=0x6700 flow=conditional
0x00003042	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00003046	Bcc[7] 0x00306A	; size=4 opcode=0x6700 flow=conditional
0x0000304A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000304E	Bcc[7] 0x00306A	; size=4 opcode=0x6700 flow=conditional
0x00003052	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00003056	Bcc[13] 0x003062	; size=4 opcode=0x6D00 flow=conditional
0x0000305A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000305E	Bcc[13] 0x00306A	; size=4 opcode=0x6D00 flow=conditional
0x00003062	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00003068	RTS	; size=2 opcode=0x4E75 flow=return
0x0000306A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00003072	RTS	; size=2 opcode=0x4E75 flow=return
0x00003074	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0000307A	Bcc[6] 0x003084	; size=4 opcode=0x6600 flow=conditional
0x0000307E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00003084	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0000308A	MOVE	; size=6 opcode=0x13C0 flow=fallthrough
0x00003090	BSR 0x003032	; size=2 opcode=0x61A0 flow=call
0x00003092	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00003098	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0000309A	MOVE	; size=4 opcode=0x2070 flow=fallthrough
0x0000309E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000030A4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000030A8	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x000030AA	DBcc 0x0030A8	; size=4 opcode=0x51CF flow=conditional
0x000030AE	MOVE	; size=10 opcode=0x23F9 flow=fallthrough
0x000030B8	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000030BE	Bcc[7] 0x003134	; size=4 opcode=0x6700 flow=conditional
0x000030C2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000030C8	Bcc[6] 0x003134	; size=4 opcode=0x6600 flow=conditional
0x000030CC	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x000030D6	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x000030E0	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x000030EA	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x000030F4	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x000030FE	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00003108	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0000310E	immediate-op	; size=4 opcode=0x0441 flow=fallthrough
0x00003112	Bcc[4] 0x003118	; size=4 opcode=0x6400 flow=conditional
0x00003116	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00003118	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x0000311E	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00003124	immediate-op	; size=4 opcode=0x0441 flow=fallthrough
0x00003128	Bcc[4] 0x00312E	; size=4 opcode=0x6400 flow=conditional
0x0000312C	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0000312E	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00003134	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000313A	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00003140	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00003146	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000314A	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x0000314C	DBcc 0x00314A	; size=4 opcode=0x51CF flow=conditional
0x00003150	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00003156	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000315C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00003160	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x00003162	DBcc 0x003160	; size=4 opcode=0x51CF flow=conditional
0x00003166	RTS	; size=2 opcode=0x4E75 flow=return
0x0000334C	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00003352	MOVE	; size=6 opcode=0x223C flow=fallthrough
0x00003358	MOVE	; size=6 opcode=0x243C flow=fallthrough
0x0000335E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00003364	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00003368	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000336E	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00003372	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003376	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0000337A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00003380	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00003384	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000338A	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0000338E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00003394	BSR 0x0033AA	; size=4 opcode=0x6100 flow=call
0x00003398	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0000339C	BSR 0x0033AA	; size=4 opcode=0x6100 flow=call
0x000033A0	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000033A4	BSR 0x0033AA	; size=4 opcode=0x6100 flow=call
0x000033A8	RTS	; size=2 opcode=0x4E75 flow=return
0x000033AA	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000033AE	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000033B2	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x000033B6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000033BC	RTS	; size=2 opcode=0x4E75 flow=return
0x00003404	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00003408	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0000340E	MOVEQ	; size=2 opcode=0x7A02 flow=fallthrough
0x00003410	MOVE	; size=6 opcode=0x223C flow=fallthrough
0x00003416	MOVEQ	; size=2 opcode=0x7420 flow=fallthrough
0x00003418	MOVEQ	; size=2 opcode=0x7E03 flow=fallthrough
0x0000341A	arithmetic-op	; size=4 opcode=0x082D flow=fallthrough
0x0000341E	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00003422	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x00003426	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x0000342A	BRA 0x003438	; size=4 opcode=0x6000 flow=branch
0x00003438	MOVEQ	; size=2 opcode=0x7C03 flow=fallthrough
0x0000343A	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0000343C	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x0000343E	arithmetic-op	; size=2 opcode=0xEB80 flow=fallthrough
0x00003440	immediate-op	; size=6 opcode=0x0680 flow=fallthrough
0x00003446	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000344C	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x00003450	DBcc 0x00343A	; size=4 opcode=0x51CE flow=conditional
0x00003454	ADDQ/SUBQ	; size=2 opcode=0x504A flow=fallthrough
0x00003456	DBcc 0x003438	; size=4 opcode=0x51CF flow=conditional
0x0000345A	arithmetic-op	; size=4 opcode=0x082D flow=fallthrough
0x0000345E	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00003462	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x00003466	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x0000346A	BRA 0x0034C8	; size=4 opcode=0x6000 flow=branch
0x000034C8	MOVEQ	; size=2 opcode=0x7C05 flow=fallthrough
0x000034CA	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000034CC	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x000034CE	arithmetic-op	; size=2 opcode=0xEB80 flow=fallthrough
0x000034D0	immediate-op	; size=6 opcode=0x0680 flow=fallthrough
0x000034D6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000034DC	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x000034E0	DBcc 0x0034CA	; size=4 opcode=0x51CE flow=conditional
0x000034E4	SUBA/CMPA/ADDA	; size=6 opcode=0xDBFC flow=fallthrough
0x000034EA	DBcc 0x003418	; size=4 opcode=0x51CD flow=conditional
0x000034EE	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x000034F2	BSR 0x003548	; size=4 opcode=0x6100 flow=call
0x000034F6	RTS	; size=2 opcode=0x4E75 flow=return
0x00003548	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0000354C	MOVEQ	; size=2 opcode=0x7002 flow=fallthrough
0x0000354E	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00003550	BSR 0x003560	; size=4 opcode=0x6100 flow=call
0x00003554	MOVE	; size=2 opcode=0x201F flow=fallthrough
0x00003556	DBcc 0x00354E	; size=4 opcode=0x5BC8 flow=conditional
0x0000355A	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x0000355E	RTS	; size=2 opcode=0x4E75 flow=return
0x00003560	Bcc[6] 0x003586	; size=4 opcode=0x6600 flow=conditional
0x00003564	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000356A	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0000356E	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003574	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00003578	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0000357C	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003582	BRA 0x0035D4	; size=4 opcode=0x6000 flow=branch
0x00003586	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000358C	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00003590	Bcc[6] 0x0035B6	; size=4 opcode=0x6600 flow=conditional
0x00003594	SUBA/CMPA/ADDA	; size=6 opcode=0xD1FC flow=fallthrough
0x0000359A	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0000359E	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000035A4	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x000035A8	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x000035AC	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000035B2	BRA 0x0035D4	; size=4 opcode=0x6000 flow=branch
0x000035B6	SUBA/CMPA/ADDA	; size=6 opcode=0xD1FC flow=fallthrough
0x000035BC	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x000035C0	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000035C6	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x000035CA	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x000035CE	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000035D4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000035D8	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000035DC	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x000035E0	Bcc[6] 0x0035E8	; size=4 opcode=0x6600 flow=conditional
0x000035E4	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x000035E8	BSR 0x003648	; size=4 opcode=0x6100 flow=call
0x000035EC	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x000035F0	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000035F4	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000035F8	BSR 0x003602	; size=4 opcode=0x6100 flow=call
0x000035FC	BSR 0x003648	; size=4 opcode=0x6100 flow=call
0x00003600	RTS	; size=2 opcode=0x4E75 flow=return
0x00003602	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x00003606	Bcc[7] 0x003634	; size=4 opcode=0x6700 flow=conditional
0x0000360A	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00003610	Bcc[10] 0x00362A	; size=4 opcode=0x6A00 flow=conditional
0x00003614	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x00003618	Bcc[10] 0x00362A	; size=4 opcode=0x6A00 flow=conditional
0x0000361C	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00003622	Bcc[10] 0x00363E	; size=4 opcode=0x6A00 flow=conditional
0x00003626	BRA 0x003634	; size=4 opcode=0x6000 flow=branch
0x0000362A	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00003630	Bcc[11] 0x00363E	; size=4 opcode=0x6B00 flow=conditional
0x00003634	MOVE	; size=6 opcode=0x243C flow=fallthrough
0x0000363A	BRA 0x003644	; size=4 opcode=0x6000 flow=branch
0x0000363E	MOVE	; size=6 opcode=0x243C flow=fallthrough
0x00003644	MOVE	; size=2 opcode=0x2C02 flow=fallthrough
0x00003646	RTS	; size=2 opcode=0x4E75 flow=return
0x00003648		; size=2 opcode=0x40E7 flow=fallthrough
0x00003674	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00003678	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0000367C	Bcc[6] 0x00368A	; size=4 opcode=0x6600 flow=conditional
0x00003680	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003686	BRA 0x0036A4	; size=4 opcode=0x6000 flow=branch
0x0000368A	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x00003690	Bcc[6] 0x00369E	; size=4 opcode=0x6600 flow=conditional
0x00003694	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x0000369A	BRA 0x0036A4	; size=4 opcode=0x6000 flow=branch
0x0000369E	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000036A4	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x000036A8	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x000036AC	immediate-op	; size=4 opcode=0x000A flow=fallthrough
0x000036B0	immediate-op	; size=4 opcode=0x004C flow=fallthrough
0x000036B4	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x000036B8	immediate-op	; size=4 opcode=0x004C flow=fallthrough
0x000036BC	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x000036C0	BRA 0x0036E4	; size=4 opcode=0x6000 flow=branch
0x000036E4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000036E8	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000036EC	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x000036F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000036F6	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x000036FA	RTS	; size=2 opcode=0x4E75 flow=return
0x000036FC	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00003700	unary-op	; size=2 opcode=0x4282 flow=fallthrough
0x00003702	unary-op	; size=2 opcode=0x4285 flow=fallthrough
0x00003704	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x00003708	Bcc[6] 0x003716	; size=4 opcode=0x6600 flow=conditional
0x0000370C	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003712	BRA 0x003730	; size=4 opcode=0x6000 flow=branch
0x00003716	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0000371C	Bcc[6] 0x00372A	; size=4 opcode=0x6600 flow=conditional
0x00003720	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003726	BRA 0x003730	; size=4 opcode=0x6000 flow=branch
0x0000372A	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003730	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x00003734	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00003738	immediate-op	; size=4 opcode=0x0062 flow=fallthrough
0x0000373C	immediate-op	; size=4 opcode=0x0056 flow=fallthrough
0x00003740	immediate-op	; size=4 opcode=0x005A flow=fallthrough
0x00003744		; size=2 opcode=0x007F flow=fallthrough
0x000037EC	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x000037F0	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x000037F4	Bcc[6] 0x003802	; size=4 opcode=0x6600 flow=conditional
0x000037F8	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000037FE	BRA 0x00381C	; size=4 opcode=0x6000 flow=branch
0x00003802	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x00003808	Bcc[6] 0x003816	; size=4 opcode=0x6600 flow=conditional
0x0000380C	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00003812	BRA 0x00381C	; size=4 opcode=0x6000 flow=branch
0x00003816	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x0000381C	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x00003820	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00003824	immediate-op	; size=4 opcode=0x0012 flow=fallthrough
0x00003828	immediate-op	; size=6 opcode=0x009C flow=fallthrough
0x0000382E	MOVE	; size=4 opcode=0x1228 flow=fallthrough
0x00003832	BSR 0x003674	; size=4 opcode=0x6100 flow=call
0x00003836	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0000383A	MOVE	; size=2 opcode=0x2E08 flow=fallthrough
0x0000383C	arithmetic-op	; size=6 opcode=0xBEB9 flow=fallthrough
0x00003842	Bcc[6] 0x003850	; size=4 opcode=0x6600 flow=conditional
0x00003846	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000384C	BRA 0x003890	; size=4 opcode=0x6000 flow=branch
0x00003850	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00003856	BRA 0x003890	; size=4 opcode=0x6000 flow=branch
0x00003890	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00003894	MOVE	; size=2 opcode=0x2F01 flow=fallthrough
0x00003896	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000389A	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0000389E	JSR	; size=6 opcode=0x4EB9 flow=call
0x000038A4	MOVE	; size=2 opcode=0x221F flow=fallthrough
0x000038A6	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x000038AA	RTS	; size=2 opcode=0x4E75 flow=return
0x000038FA	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00003900	MOVEQ	; size=2 opcode=0x7E02 flow=fallthrough
0x00003902	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00003904	BSR 0x003674	; size=4 opcode=0x6100 flow=call
0x00003908	BSR 0x0037EC	; size=4 opcode=0x6100 flow=call
0x0000390C	BSR 0x0036FC	; size=4 opcode=0x6100 flow=call
0x00003910	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00003914	DBcc 0x003902	; size=4 opcode=0x51CF flow=conditional
0x00003918	RTS	; size=2 opcode=0x4E75 flow=return
0x0000391A	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00003922	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00003926	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000392C	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x00003932	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00003938	MOVE	; size=2 opcode=0x2A08 flow=fallthrough
0x0000393A	MOVE	; size=4 opcode=0x3828 flow=fallthrough
0x0000393E	MUL/DIV	; size=4 opcode=0xC8FC flow=fallthrough
0x00003942	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x00003946	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x0000394A	BSR 0x0033AA	; size=4 opcode=0x6100 flow=call
0x0000394E	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00003954	MOVE	; size=4 opcode=0x2028 flow=fallthrough
0x00003958	Bcc[7] 0x003964	; size=4 opcode=0x6700 flow=conditional
0x0000395C	MOVE	; size=2 opcode=0x2440 flow=fallthrough
0x0000395E	arithmetic-op	; size=4 opcode=0x08AA flow=fallthrough
0x00003962	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x00003964	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x00003966	immediate-op	; size=4 opcode=0x0002 flow=fallthrough
0x0000396A	Bcc[6] 0x003992	; size=4 opcode=0x6600 flow=conditional
0x0000396E	ADDQ/SUBQ	; size=6 opcode=0x5339 flow=fallthrough
0x00003974	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x00003978	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000397C		; size=2 opcode=0x00FF flow=fallthrough
0x00003992	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x00003996	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000399A	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x0000399E		; size=2 opcode=0x00FF flow=fallthrough
0x000039F2	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x000039F8	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x000039FE	MOVE	; size=4 opcode=0x3828 flow=fallthrough
0x00003A02	MUL/DIV	; size=4 opcode=0xC8FC flow=fallthrough
0x00003A06	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x00003A0A	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x00003A0E	BSR 0x0033AA	; size=4 opcode=0x6100 flow=call
0x00003A12	BSR 0x003548	; size=4 opcode=0x6100 flow=call
0x00003A16	BSR 0x0038FA	; size=4 opcode=0x6100 flow=call
0x00003A1A	RTS	; size=2 opcode=0x4E75 flow=return
0x00003A1C	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003A24	Bcc[6] 0x003A3C	; size=4 opcode=0x6600 flow=conditional
0x00003A28	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00003A30	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00003A38	BRA 0x003BAC	; size=4 opcode=0x6000 flow=branch
0x00003A3C	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00003A42	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00003A46	MOVE	; size=4 opcode=0x363C flow=fallthrough
0x00003A4A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00003A50	arithmetic-op	; size=2 opcode=0xD650 flow=fallthrough
0x00003A52	Bcc[10] 0x003A58	; size=4 opcode=0x6A00 flow=conditional
0x00003A56	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x00003A58	arithmetic-op	; size=4 opcode=0xD468 flow=fallthrough
0x00003A5C	Bcc[10] 0x003A62	; size=4 opcode=0x6A00 flow=conditional
0x00003A60	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x00003A62	MOVE	; size=2 opcode=0x3802 flow=fallthrough
0x00003A64	MOVE	; size=6 opcode=0x3C39 flow=fallthrough
0x00003A6A	MOVE	; size=6 opcode=0x3E39 flow=fallthrough
0x00003A70	immediate-op	; size=4 opcode=0x0646 flow=fallthrough
0x00003A74	immediate-op	; size=4 opcode=0x0647 flow=fallthrough
0x00003A78	BSR 0x003A90	; size=4 opcode=0x6100 flow=call
0x00003A7C	MOVE	; size=6 opcode=0x13C1 flow=fallthrough
0x00003A82	BSR 0x003B1E	; size=4 opcode=0x6100 flow=call
0x00003A86	MOVE	; size=6 opcode=0x13C2 flow=fallthrough
0x00003A8C	BRA 0x003C36	; size=4 opcode=0x6000 flow=branch
0x00003A90	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00003A92	arithmetic-op	; size=2 opcode=0x9C43 flow=fallthrough
0x00003A94	Bcc[5] 0x003ADA	; size=4 opcode=0x6500 flow=conditional
0x00003A98	immediate-op	; size=4 opcode=0x0C46 flow=fallthrough
0x00003A9C	Bcc[5] 0x003AA6	; size=4 opcode=0x6500 flow=conditional
0x00003AA0	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003AA4	RTS	; size=2 opcode=0x4E75 flow=return
0x00003AA6	immediate-op	; size=4 opcode=0x0C46 flow=fallthrough
0x00003AAA	Bcc[5] 0x003AC6	; size=4 opcode=0x6500 flow=conditional
0x00003AAE	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003AB2	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003ABA	Bcc[6] 0x003AC0	; size=4 opcode=0x6600 flow=conditional
0x00003ABE	RTS	; size=2 opcode=0x4E75 flow=return
0x00003AC0	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003AC4	RTS	; size=2 opcode=0x4E75 flow=return
0x00003AC6	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003ACE	Bcc[7] 0x003AD4	; size=4 opcode=0x6700 flow=conditional
0x00003AD2	RTS	; size=2 opcode=0x4E75 flow=return
0x00003AD4	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003AD8	RTS	; size=2 opcode=0x4E75 flow=return
0x00003ADA	unary-op	; size=2 opcode=0x4446 flow=fallthrough
0x00003ADC	immediate-op	; size=4 opcode=0x0C46 flow=fallthrough
0x00003AE0	Bcc[5] 0x003AEA	; size=4 opcode=0x6500 flow=conditional
0x00003AE4	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003AE8	RTS	; size=2 opcode=0x4E75 flow=return
0x00003AEA	immediate-op	; size=4 opcode=0x0C46 flow=fallthrough
0x00003AEE	Bcc[5] 0x003B0A	; size=4 opcode=0x6500 flow=conditional
0x00003AF2	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003AF6	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003AFE	Bcc[6] 0x003B04	; size=4 opcode=0x6600 flow=conditional
0x00003B02	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B04	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003B08	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B0A	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003B12	Bcc[7] 0x003B18	; size=4 opcode=0x6700 flow=conditional
0x00003B16	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B18	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00003B1C	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B1E	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x00003B20	arithmetic-op	; size=2 opcode=0x9E44 flow=fallthrough
0x00003B22	Bcc[5] 0x003B68	; size=4 opcode=0x6500 flow=conditional
0x00003B26	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00003B2A	Bcc[5] 0x003B34	; size=4 opcode=0x6500 flow=conditional
0x00003B2E	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B32	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B34	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00003B38	Bcc[5] 0x003B54	; size=4 opcode=0x6500 flow=conditional
0x00003B3C	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B40	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003B48	Bcc[6] 0x003B4E	; size=4 opcode=0x6600 flow=conditional
0x00003B4C	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B4E	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B52	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B54	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003B5C	Bcc[7] 0x003B62	; size=4 opcode=0x6700 flow=conditional
0x00003B60	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B62	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B66	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B68	unary-op	; size=2 opcode=0x4447 flow=fallthrough
0x00003B6A	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00003B6E	Bcc[5] 0x003B78	; size=4 opcode=0x6500 flow=conditional
0x00003B72	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B76	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B78	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00003B7C	Bcc[5] 0x003B98	; size=4 opcode=0x6500 flow=conditional
0x00003B80	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B84	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003B8C	Bcc[6] 0x003B92	; size=4 opcode=0x6600 flow=conditional
0x00003B90	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B92	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003B96	RTS	; size=2 opcode=0x4E75 flow=return
0x00003B98	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00003BA0	Bcc[7] 0x003BA6	; size=4 opcode=0x6700 flow=conditional
0x00003BA4	RTS	; size=2 opcode=0x4E75 flow=return
0x00003BA6	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003BAA	RTS	; size=2 opcode=0x4E75 flow=return
0x00003BAC	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x00003BB2	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x00003BB8	MOVE	; size=6 opcode=0x3A39 flow=fallthrough
0x00003BBE	MOVE	; size=6 opcode=0x3839 flow=fallthrough
0x00003BC4	JSR	; size=6 opcode=0x4EB9 flow=call
0x00003BCA	MOVE	; size=2 opcode=0x3803 flow=fallthrough
0x00003BCC	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x00003BD0	Bcc[4] 0x003BDC	; size=4 opcode=0x6400 flow=conditional
0x00003BD4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00003BDA	RTS	; size=2 opcode=0x4E75 flow=return
0x00003BDC	arithmetic-op	; size=2 opcode=0xE75A flow=fallthrough
0x00003BDE	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x00003BE2	MOVE	; size=2 opcode=0x3002 flow=fallthrough
0x00003BE4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00003BEA	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00003BEC	MOVE	; size=4 opcode=0x3231 flow=fallthrough
0x00003BF0	ADDQ/SUBQ	; size=2 opcode=0x5440 flow=fallthrough
0x00003BF2	MOVE	; size=4 opcode=0x3431 flow=fallthrough
0x00003BF6	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x00003BFA	Bcc[5] 0x003C36	; size=4 opcode=0x6500 flow=conditional
0x00003BFE	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00003C04	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x00003C0A	Bcc[7] 0x003C36	; size=4 opcode=0x6700 flow=conditional
0x00003C0E	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x00003C14	Bcc[7] 0x003C36	; size=4 opcode=0x6700 flow=conditional
0x00003C18	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00003C1E	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x00003C24	Bcc[7] 0x003C36	; size=4 opcode=0x6700 flow=conditional
0x00003C28	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x00003C2E	Bcc[7] 0x003C36	; size=4 opcode=0x6700 flow=conditional
0x00003C32	arithmetic-op	; size=2 opcode=0xE341 flow=fallthrough
0x00003C34	arithmetic-op	; size=2 opcode=0xE342 flow=fallthrough
0x00003C36	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00003C3C	Bcc[6] 0x003D16	; size=4 opcode=0x6600 flow=conditional
0x00003C40	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00003C46	Bcc[6] 0x003D16	; size=4 opcode=0x6600 flow=conditional
0x00003C4A	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00003C50	Bcc[6] 0x003D16	; size=4 opcode=0x6600 flow=conditional
0x00003C54	MOVE	; size=2 opcode=0x3001 flow=fallthrough
0x00003C56	arithmetic-op	; size=2 opcode=0xE949 flow=fallthrough
0x00003C58	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x00003C5A	Bcc[7] 0x003CBA	; size=4 opcode=0x6700 flow=conditional
0x00003C5E	Bcc[10] 0x003C76	; size=4 opcode=0x6A00 flow=conditional
0x00003C62	arithmetic-op	; size=6 opcode=0xD279 flow=fallthrough
0x00003C68	arithmetic-op	; size=6 opcode=0xB279 flow=fallthrough
0x00003C6E	Bcc[13] 0x003CBA	; size=4 opcode=0x6D00 flow=conditional
0x00003C72	BRA 0x003C86	; size=4 opcode=0x6000 flow=branch
0x00003C76	arithmetic-op	; size=6 opcode=0xD279 flow=fallthrough
0x00003C7C	arithmetic-op	; size=6 opcode=0xB279 flow=fallthrough
0x00003C82	Bcc[14] 0x003CBA	; size=4 opcode=0x6E00 flow=conditional
0x00003C86	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00003C8C	unary-op	; size=2 opcode=0x4440 flow=fallthrough
0x00003C8E	arithmetic-op	; size=6 opcode=0xD179 flow=fallthrough
0x00003C94	arithmetic-op	; size=6 opcode=0xD179 flow=fallthrough
0x00003C9A	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00003C9E	arithmetic-op	; size=6 opcode=0xB279 flow=fallthrough
0x00003CA4	Bcc[7] 0x003CBA	; size=4 opcode=0x6700 flow=conditional
0x00003CA8	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00003CAE	MOVEM	; size=4 opcode=0x48A7 flow=fallthrough
0x00003CB2	BSR 0x0040E2	; size=4 opcode=0x6100 flow=call
0x00003CB6	MOVEM	; size=4 opcode=0x4C9F flow=fallthrough
0x00003CBA	MOVE	; size=2 opcode=0x3002 flow=fallthrough
0x00003CBC	arithmetic-op	; size=2 opcode=0xE94A flow=fallthrough
0x00003CBE	unary-op	; size=2 opcode=0x4A42 flow=fallthrough
0x00003CC0	Bcc[7] 0x003D16	; size=4 opcode=0x6700 flow=conditional
0x00003CC4	Bcc[10] 0x003CDC	; size=4 opcode=0x6A00 flow=conditional
0x00003CC8	arithmetic-op	; size=6 opcode=0xD479 flow=fallthrough
0x00003CCE	arithmetic-op	; size=6 opcode=0xB479 flow=fallthrough
0x00003CD4	Bcc[13] 0x003D16	; size=4 opcode=0x6D00 flow=conditional
0x00003CD8	BRA 0x003CEC	; size=4 opcode=0x6000 flow=branch
0x00003CDC	arithmetic-op	; size=6 opcode=0xD479 flow=fallthrough
0x00003CE2	arithmetic-op	; size=6 opcode=0xB479 flow=fallthrough
0x00003CE8	Bcc[14] 0x003D16	; size=4 opcode=0x6E00 flow=conditional
0x00003CEC	MOVE	; size=6 opcode=0x33C2 flow=fallthrough
0x00003CF2	arithmetic-op	; size=6 opcode=0xD179 flow=fallthrough
0x00003CF8	arithmetic-op	; size=6 opcode=0xD179 flow=fallthrough
0x00003CFE	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x00003D02	arithmetic-op	; size=6 opcode=0xB479 flow=fallthrough
0x00003D08	Bcc[7] 0x003D16	; size=4 opcode=0x6700 flow=conditional
0x00003D0C	MOVE	; size=6 opcode=0x33C2 flow=fallthrough
0x00003D12	BSR 0x004018	; size=4 opcode=0x6100 flow=call
0x00003D16	RTS	; size=2 opcode=0x4E75 flow=return
0x00003D18	BSR 0x0041B6	; size=4 opcode=0x6100 flow=call
0x00003D1C	BSR 0x004000	; size=4 opcode=0x6100 flow=call
0x00003D20	arithmetic-op	; size=2 opcode=0xD845 flow=fallthrough
0x00003D22	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00003D26	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00003D2A	BSR 0x003D4A	; size=4 opcode=0x6100 flow=call
0x00003D2E	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00003D32	MOVE	; size=2 opcode=0x3604 flow=fallthrough
0x00003D34	immediate-op	; size=4 opcode=0x0243 flow=fallthrough
0x00003D38	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x00003D3C	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00003D40	arithmetic-op	; size=2 opcode=0x8843 flow=fallthrough
0x00003D42	ADDQ/SUBQ	; size=2 opcode=0x5041 flow=fallthrough
0x00003D44	DBcc 0x003D26	; size=4 opcode=0x51CA flow=conditional
0x00003D48	RTS	; size=2 opcode=0x4E75 flow=return
0x00003D4A	BSR 0x003FD8	; size=4 opcode=0x6100 flow=call
0x00003D4E	MOVE	; size=2 opcode=0x3E04 flow=fallthrough
0x00003D50	immediate-op	; size=4 opcode=0x0247 flow=fallthrough
0x00003D54	immediate-op	; size=4 opcode=0x0447 flow=fallthrough
0x00003D58	unary-op	; size=2 opcode=0x4447 flow=fallthrough
0x00003D5A	arithmetic-op	; size=2 opcode=0xE24F flow=fallthrough
0x00003D5C	ADDQ/SUBQ	; size=2 opcode=0x5347 flow=fallthrough
0x00003D5E	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00003D62	Bcc[13] 0x003D6E	; size=4 opcode=0x6D00 flow=conditional
0x00003D66	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00003D6A	BRA 0x003D80	; size=4 opcode=0x6000 flow=branch
0x00003D6E	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x00003D70	BSR 0x003D80	; size=4 opcode=0x6100 flow=call
0x00003D74	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x00003D76	immediate-op	; size=4 opcode=0x0447 flow=fallthrough
0x00003D7A	unary-op	; size=2 opcode=0x4447 flow=fallthrough
0x00003D7C	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00003D80		; size=2 opcode=0x40E7 flow=fallthrough
0x00003DDE	BSR 0x003FD8	; size=4 opcode=0x6100 flow=call
0x00003DE2	MOVE	; size=2 opcode=0x3E04 flow=fallthrough
0x00003DE4	immediate-op	; size=4 opcode=0x0247 flow=fallthrough
0x00003DE8	immediate-op	; size=4 opcode=0x0447 flow=fallthrough
0x00003DEC	unary-op	; size=2 opcode=0x4447 flow=fallthrough
0x00003DEE	arithmetic-op	; size=2 opcode=0xEE4F flow=fallthrough
0x00003DF0	ADDQ/SUBQ	; size=2 opcode=0x5347 flow=fallthrough
0x00003DF2	immediate-op	; size=4 opcode=0x0C47 flow=fallthrough
0x00003DF6	Bcc[13] 0x003E02	; size=4 opcode=0x6D00 flow=conditional
0x00003DFA	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00003DFE	BRA 0x003E14	; size=4 opcode=0x6000 flow=branch
0x00003E02	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x00003E04	BSR 0x003E14	; size=4 opcode=0x6100 flow=call
0x00003E08	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x00003E0A	immediate-op	; size=4 opcode=0x0447 flow=fallthrough
0x00003E0E	unary-op	; size=2 opcode=0x4447 flow=fallthrough
0x00003E10	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00003E14		; size=2 opcode=0x40E7 flow=fallthrough
0x00003FD8	MOVE	; size=2 opcode=0x3400 flow=fallthrough
0x00003FDA	MOVE	; size=2 opcode=0x3601 flow=fallthrough
0x00003FDC	arithmetic-op	; size=2 opcode=0xE648 flow=fallthrough
0x00003FDE	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00003FE2	arithmetic-op	; size=2 opcode=0xE649 flow=fallthrough
0x00003FE4	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00003FE8	arithmetic-op	; size=2 opcode=0xEA4B flow=fallthrough
0x00003FEA	MUL/DIV	; size=6 opcode=0xC6F9 flow=fallthrough
0x00003FF0	arithmetic-op	; size=2 opcode=0xEA4A flow=fallthrough
0x00003FF2	arithmetic-op	; size=2 opcode=0xE34A flow=fallthrough
0x00003FF4	arithmetic-op	; size=2 opcode=0xD642 flow=fallthrough
0x00003FF6	arithmetic-op	; size=2 opcode=0xE549 flow=fallthrough
0x00003FF8	arithmetic-op	; size=2 opcode=0xD240 flow=fallthrough
0x00003FFA	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x00003FFC	EXT	; size=2 opcode=0x48C1 flow=fallthrough
0x00003FFE	RTS	; size=2 opcode=0x4E75 flow=return
0x00004000	unary-op	; size=2 opcode=0x4443 flow=fallthrough
0x00004002	immediate-op	; size=4 opcode=0x0243 flow=fallthrough
0x00004006	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x0000400A	arithmetic-op	; size=2 opcode=0xE44B flow=fallthrough
0x0000400C	arithmetic-op	; size=2 opcode=0xE94C flow=fallthrough
0x0000400E	arithmetic-op	; size=2 opcode=0xD843 flow=fallthrough
0x00004010	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00004014	EXT	; size=2 opcode=0x48C4 flow=fallthrough
0x00004016	RTS	; size=2 opcode=0x4E75 flow=return
0x00004018	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0000401E	arithmetic-op	; size=2 opcode=0xE849 flow=fallthrough
0x00004020	unary-op	; size=2 opcode=0x4A40 flow=fallthrough
0x00004022	Bcc[10] 0x00407E	; size=4 opcode=0x6A00 flow=conditional
0x00004026	ADDQ/SUBQ	; size=2 opcode=0x5141 flow=fallthrough
0x00004028	Bcc[11] 0x00407C	; size=4 opcode=0x6B00 flow=conditional
0x0000402C	BSR 0x004216	; size=4 opcode=0x6100 flow=call
0x00004030	MOVE	; size=6 opcode=0x3839 flow=fallthrough
0x00004036	ADDQ/SUBQ	; size=2 opcode=0x5144 flow=fallthrough
0x00004038	BSR 0x004000	; size=2 opcode=0x61C6 flow=call
0x0000403A	MOVEM	; size=4 opcode=0x48A7 flow=fallthrough
0x0000403E	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00004044	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000404A	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x00004050	BSR 0x003D4A	; size=4 opcode=0x6100 flow=call
0x00004054	MOVEM	; size=4 opcode=0x4C9F flow=fallthrough
0x00004058	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x0000405E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00004064	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x0000406A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00004072	BSR 0x003D4A	; size=4 opcode=0x6100 flow=call
0x00004076	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000407C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000407E	ADDQ/SUBQ	; size=2 opcode=0x5041 flow=fallthrough
0x00004080	arithmetic-op	; size=6 opcode=0xB279 flow=fallthrough
0x00004086	Bcc[14] 0x00407C	; size=2 opcode=0x6EF4 flow=conditional
0x00004088	BSR 0x004216	; size=4 opcode=0x6100 flow=call
0x0000408C	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x00004090	MOVE	; size=6 opcode=0x3839 flow=fallthrough
0x00004096	immediate-op	; size=4 opcode=0x0644 flow=fallthrough
0x0000409A	BSR 0x004000	; size=4 opcode=0x6100 flow=call
0x0000409E	MOVEM	; size=4 opcode=0x48A7 flow=fallthrough
0x000040A2	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x000040A8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000040AE	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x000040B4	BSR 0x003D4A	; size=4 opcode=0x6100 flow=call
0x000040B8	MOVEM	; size=4 opcode=0x4C9F flow=fallthrough
0x000040BC	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x000040C2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000040C8	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x000040CE	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000040D6	BSR 0x003D4A	; size=4 opcode=0x6100 flow=call
0x000040DA	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000040E0	BRA 0x00407C	; size=2 opcode=0x609A flow=branch
0x000040E2	unary-op	; size=2 opcode=0x4A40 flow=fallthrough
0x000040E4	Bcc[11] 0x00414A	; size=4 opcode=0x6B00 flow=conditional
0x000040E8	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x000040EE	arithmetic-op	; size=2 opcode=0xE848 flow=fallthrough
0x000040F0	ADDQ/SUBQ	; size=2 opcode=0x5140 flow=fallthrough
0x000040F2	Bcc[11] 0x004148	; size=4 opcode=0x6B00 flow=conditional
0x000040F6	BSR 0x0041F0	; size=4 opcode=0x6100 flow=call
0x000040FA	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x00004100	ADDQ/SUBQ	; size=2 opcode=0x5043 flow=fallthrough
0x00004102	BSR 0x004000	; size=4 opcode=0x6100 flow=call
0x00004106	MOVEM	; size=4 opcode=0x48A7 flow=fallthrough
0x0000410A	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00004110	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00004116	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x0000411C	BSR 0x003DDE	; size=4 opcode=0x6100 flow=call
0x00004120	MOVEM	; size=4 opcode=0x4C9F flow=fallthrough
0x00004124	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x0000412A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00004130	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x00004136	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0000413E	BSR 0x003DDE	; size=4 opcode=0x6100 flow=call
0x00004142	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00004148	RTS	; size=2 opcode=0x4E75 flow=return
0x0000414A	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00004150	arithmetic-op	; size=2 opcode=0xE848 flow=fallthrough
0x00004152	ADDQ/SUBQ	; size=2 opcode=0x5040 flow=fallthrough
0x00004154	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x0000415A	Bcc[14] 0x004148	; size=2 opcode=0x6EEC flow=conditional
0x0000415C	BSR 0x0041F0	; size=4 opcode=0x6100 flow=call
0x00004160	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x00004164	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x0000416A	immediate-op	; size=4 opcode=0x0443 flow=fallthrough
0x0000416E	BSR 0x004000	; size=4 opcode=0x6100 flow=call
0x00004172	MOVEM	; size=4 opcode=0x48A7 flow=fallthrough
0x00004176	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x0000417C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00004182	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x00004188	BSR 0x003DDE	; size=4 opcode=0x6100 flow=call
0x0000418C	MOVEM	; size=4 opcode=0x4C9F flow=fallthrough
0x00004190	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00004196	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000419C	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x000041A2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000041AA	BSR 0x003DDE	; size=4 opcode=0x6100 flow=call
0x000041AE	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000041B4	BRA 0x004148	; size=2 opcode=0x6092 flow=branch
0x000041B6	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x000041BC	MOVE	; size=6 opcode=0x3839 flow=fallthrough
0x000041C2	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x000041C8	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000041CE	arithmetic-op	; size=2 opcode=0xE848 flow=fallthrough
0x000041D0	arithmetic-op	; size=2 opcode=0xE849 flow=fallthrough
0x000041D2	ADDQ/SUBQ	; size=2 opcode=0x5140 flow=fallthrough
0x000041D4	Bcc[11] 0x0041DE	; size=4 opcode=0x6B00 flow=conditional
0x000041D8	ADDQ/SUBQ	; size=2 opcode=0x5043 flow=fallthrough
0x000041DA	BRA 0x0041E0	; size=4 opcode=0x6000 flow=branch
0x000041DE	ADDQ/SUBQ	; size=2 opcode=0x5040 flow=fallthrough
0x000041E0	ADDQ/SUBQ	; size=2 opcode=0x5141 flow=fallthrough
0x000041E2	Bcc[11] 0x0041EC	; size=4 opcode=0x6B00 flow=conditional
0x000041E6	ADDQ/SUBQ	; size=2 opcode=0x5144 flow=fallthrough
0x000041E8	BRA 0x0041EE	; size=4 opcode=0x6000 flow=branch
0x000041EC	ADDQ/SUBQ	; size=2 opcode=0x5041 flow=fallthrough
0x000041EE	RTS	; size=2 opcode=0x4E75 flow=return
0x000041F0	MOVE	; size=6 opcode=0x3839 flow=fallthrough
0x000041F6	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000041FC	arithmetic-op	; size=2 opcode=0xE849 flow=fallthrough
0x000041FE	ADDQ/SUBQ	; size=2 opcode=0x5141 flow=fallthrough
0x00004200	Bcc[11] 0x004212	; size=4 opcode=0x6B00 flow=conditional
0x00004204	ADDQ/SUBQ	; size=2 opcode=0x5144 flow=fallthrough
0x00004206	ADDQ/SUBQ	; size=2 opcode=0x5141 flow=fallthrough
0x00004208	Bcc[11] 0x004212	; size=4 opcode=0x6B00 flow=conditional
0x0000420C	ADDQ/SUBQ	; size=2 opcode=0x5144 flow=fallthrough
0x0000420E	BRA 0x004214	; size=4 opcode=0x6000 flow=branch
0x00004212	ADDQ/SUBQ	; size=2 opcode=0x5041 flow=fallthrough
0x00004214	RTS	; size=2 opcode=0x4E75 flow=return
0x00004216	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x0000421C	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00004222	arithmetic-op	; size=2 opcode=0xE848 flow=fallthrough
0x00004224	ADDQ/SUBQ	; size=2 opcode=0x5140 flow=fallthrough
0x00004226	Bcc[11] 0x004238	; size=4 opcode=0x6B00 flow=conditional
0x0000422A	ADDQ/SUBQ	; size=2 opcode=0x5043 flow=fallthrough
0x0000422C	ADDQ/SUBQ	; size=2 opcode=0x5140 flow=fallthrough
0x0000422E	Bcc[11] 0x004238	; size=4 opcode=0x6B00 flow=conditional
0x00004232	ADDQ/SUBQ	; size=2 opcode=0x5043 flow=fallthrough
0x00004234	BRA 0x00423A	; size=4 opcode=0x6000 flow=branch
0x00004238	ADDQ/SUBQ	; size=2 opcode=0x5040 flow=fallthrough
0x0000423A	RTS	; size=2 opcode=0x4E75 flow=return
0x000042B0	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000042B6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000042BC	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x000042C0	SUBA/CMPA/ADDA	; size=2 opcode=0xD1C0 flow=fallthrough
0x000042C2	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000042C6	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x000042C8	DBcc 0x0042C6	; size=4 opcode=0x51CF flow=conditional
0x000042CC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000042D4	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000042DC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000042E4	RTS	; size=2 opcode=0x4E75 flow=return
0x000042F2	MOVE	; size=6 opcode=0x13C0 flow=fallthrough
0x000042F8	ADDQ/SUBQ	; size=6 opcode=0x5239 flow=fallthrough
0x000042FE	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00004304	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0000430A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000430E	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00004312	MOVE	; size=4 opcode=0x363C flow=fallthrough
0x00004316	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x00004318	BRA 0x004350	; size=4 opcode=0x6000 flow=branch
0x0000431C	MOVE	; size=6 opcode=0x1C39 flow=fallthrough
0x00004322	ADDQ/SUBQ	; size=2 opcode=0x5306 flow=fallthrough
0x00004324	arithmetic-op	; size=2 opcode=0xB006 flow=fallthrough
0x00004326	Bcc[7] 0x004378	; size=4 opcode=0x6700 flow=conditional
0x0000432A	MOVE	; size=6 opcode=0x13C0 flow=fallthrough
0x00004330	ADDQ/SUBQ	; size=6 opcode=0x5239 flow=fallthrough
0x00004336	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000433C	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x00004342	MOVEQ	; size=2 opcode=0x7202 flow=fallthrough
0x00004344	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00004348	MOVE	; size=4 opcode=0x363C flow=fallthrough
0x0000434C	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00004350	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00004356	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0000435A	SUBA/CMPA/ADDA	; size=2 opcode=0xD1C0 flow=fallthrough
0x0000435C	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00004360	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x00004362	DBcc 0x004360	; size=4 opcode=0x51CE flow=conditional
0x00004366	SUBA/CMPA/ADDA	; size=6 opcode=0x93FC flow=fallthrough
0x0000436C	MOVE	; size=4 opcode=0x3341 flow=fallthrough
0x00004370	MOVE	; size=4 opcode=0x3342 flow=fallthrough
0x00004374	MOVE	; size=4 opcode=0x3343 flow=fallthrough
0x00004378	MOVE	; size=2 opcode=0x2F07 flow=fallthrough
0x0000437A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004380	MOVE	; size=2 opcode=0x2E1F flow=fallthrough
0x00004382	MOVE	; size=4 opcode=0x1347 flow=fallthrough
0x00004386	RTS	; size=2 opcode=0x4E75 flow=return
0x00004388	BSR 0x0043B6	; size=4 opcode=0x6100 flow=call
0x0000438C	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00004394	Bcc[6] 0x0043A0	; size=4 opcode=0x6600 flow=conditional
0x00004398	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000043A0	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x000043A8	Bcc[6] 0x0043B4	; size=4 opcode=0x6600 flow=conditional
0x000043AC	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000043B4	RTS	; size=2 opcode=0x4E75 flow=return
0x000043B6	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x000043BE	Bcc[6] 0x0043C8	; size=4 opcode=0x6600 flow=conditional
0x000043C2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000043C8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000043CE	unary-op	; size=6 opcode=0x4AB9 flow=fallthrough
0x000043D4	Bcc[7] 0x0043E8	; size=4 opcode=0x6700 flow=conditional
0x000043D8	MOVE	; size=6 opcode=0x2A79 flow=fallthrough
0x000043DE	BSR 0x004466	; size=4 opcode=0x6100 flow=call
0x000043E2	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x000043E8	unary-op	; size=6 opcode=0x4AB9 flow=fallthrough
0x000043EE	Bcc[7] 0x00445C	; size=4 opcode=0x6700 flow=conditional
0x000043F2	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x000043FC	Bcc[6] 0x00441C	; size=4 opcode=0x6600 flow=conditional
0x00004400	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0000440A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004410	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00004418	BRA 0x004442	; size=4 opcode=0x6000 flow=branch
0x0000441C	immediate-op	; size=10 opcode=0x0CB9 flow=fallthrough
0x00004426	Bcc[6] 0x004442	; size=4 opcode=0x6600 flow=conditional
0x0000442A	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00004434	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000443A	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00004442	MOVE	; size=6 opcode=0x2A79 flow=fallthrough
0x00004448	BSR 0x004466	; size=4 opcode=0x6100 flow=call
0x0000444C	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00004452	MOVE	; size=6 opcode=0x2A79 flow=fallthrough
0x00004458	BRA 0x004466	; size=4 opcode=0x6000 flow=branch
0x0000445C	MOVE	; size=6 opcode=0x2A79 flow=fallthrough
0x00004462	BRA 0x004466	; size=4 opcode=0x6000 flow=branch
0x00004466	unary-op	; size=2 opcode=0x4A95 flow=fallthrough
0x00004468	Bcc[7] 0x0044BA	; size=4 opcode=0x6700 flow=conditional
0x0000446C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004472	Bcc[5] 0x0044BA	; size=4 opcode=0x6500 flow=conditional
0x00004476	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0000447C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004482	Bcc[5] 0x0044BA	; size=4 opcode=0x6500 flow=conditional
0x00004486	MOVE	; size=2 opcode=0x281D flow=fallthrough
0x00004488	MOVE	; size=2 opcode=0x265D flow=fallthrough
0x0000448A	arithmetic-op	; size=2 opcode=0x0884 flow=fallthrough
0x0000448C	immediate-op	; size=4 opcode=0x001F flow=fallthrough
0x00004490	immediate-op	; size=4 opcode=0x0018 flow=fallthrough
0x00004494		; size=2 opcode=0x00FF flow=fallthrough
0x000044BA	RTS	; size=2 opcode=0x4E75 flow=return
0x000044BC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000044C2	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000044C6	MOVE	; size=2 opcode=0x2F09 flow=fallthrough
0x000044C8	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x000044CA	DBcc 0x0044C8	; size=4 opcode=0x51CF flow=conditional
0x000044CE	MOVE	; size=2 opcode=0x225F flow=fallthrough
0x000044D0	MOVE	; size=8 opcode=0x3379 flow=fallthrough
0x000044D8	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x000044DE	MOVE	; size=4 opcode=0x3341 flow=fallthrough
0x000044E2	MOVE	; size=4 opcode=0x334A flow=fallthrough
0x000044E6	MOVE	; size=4 opcode=0x234B flow=fallthrough
0x000044EA	MOVE	; size=2 opcode=0x329B flow=fallthrough
0x000044EC	MOVE	; size=4 opcode=0x3351 flow=fallthrough
0x000044F0	MOVE	; size=2 opcode=0x3E13 flow=fallthrough
0x000044F2	arithmetic-op	; size=6 opcode=0xDE79 flow=fallthrough
0x000044F8	MOVE	; size=4 opcode=0x3347 flow=fallthrough
0x000044FC	MOVE	; size=4 opcode=0x3347 flow=fallthrough
0x00004500	MOVE	; size=2 opcode=0x2044 flow=fallthrough
0x00004502	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x00004506	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x0000450A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000450E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004512	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004516	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000451A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000451E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004522	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x00004526	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x0000452A	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x0000452E	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x00004532	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x00004536	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x0000453A	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x0000453E	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x00004542	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004546	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000454A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000454E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004552	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004556	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000455A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000455E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004562	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004566	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000456A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000456E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004572	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004576	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000457A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000457E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004582	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004586	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000458A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000458E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004592	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00004596	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000459A	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0000459E	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000045A2	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000045A6	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000045AA	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000045AE	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000045B2	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x000045B6	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x000045BA	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000045BE	MOVE	; size=4 opcode=0x1350 flow=fallthrough
0x000045C2	MOVE	; size=4 opcode=0x3340 flow=fallthrough
0x000045C6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000045CC	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000045D0	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000045D8	Bcc[6] 0x0045E0	; size=4 opcode=0x6600 flow=conditional
0x000045DC	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000045E0	MOVE	; size=4 opcode=0x3340 flow=fallthrough
0x000045E4	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000045EA	Bcc[7] 0x0045F4	; size=4 opcode=0x6700 flow=conditional
0x000045EE		; size=2 opcode=0x08E9 flow=fallthrough
0x000045F4	immediate-op	; size=6 opcode=0x0C95 flow=fallthrough
0x000045FA	Bcc[6] 0x004616	; size=4 opcode=0x6600 flow=conditional
0x000045FE	SUBA/CMPA/ADDA	; size=4 opcode=0xDAFC flow=fallthrough
0x00004602	unary-op	; size=2 opcode=0x4A55 flow=fallthrough
0x00004604	Bcc[7] 0x004612	; size=4 opcode=0x6700 flow=conditional
0x00004608	MOVE	; size=2 opcode=0x101D flow=fallthrough
0x0000460A	MOVE	; size=2 opcode=0x121D flow=fallthrough
0x0000460C	MOVE	; size=4 opcode=0x1381 flow=fallthrough
0x00004610	BRA 0x004602	; size=2 opcode=0x60F0 flow=branch
0x00004612	SUBA/CMPA/ADDA	; size=4 opcode=0xDAFC flow=fallthrough
0x00004616	RTS	; size=2 opcode=0x4E75 flow=return
0x00004618	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000461E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00004626	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000462A	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000462E	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00004632	immediate-op	; size=4 opcode=0x0012 flow=fallthrough
0x00004636	MOVE	; size=2 opcode=0x1420 flow=fallthrough
0x00004638	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000463E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004644	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x00004648	DBcc 0x00462A	; size=4 opcode=0x51CF flow=conditional
0x0000464C	BSR 0x004658	; size=4 opcode=0x6100 flow=call
0x00004650	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00004656	RTS	; size=2 opcode=0x4E75 flow=return
0x00004658	BSR 0x005B1E	; size=4 opcode=0x6100 flow=call
0x0000465C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00004664	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0000466A	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0000466E	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00004672	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00004678	MOVE	; size=2 opcode=0x205C flow=fallthrough
0x0000467A	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000467E	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00004682	bit-op	; size=2 opcode=0x0166 flow=fallthrough
0x00004684	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x00004688	Bcc[6] 0x0046B0	; size=4 opcode=0x6600 flow=conditional
0x0000468C	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00004692	Bcc[6] 0x00469E	; size=4 opcode=0x6600 flow=conditional
0x00004696	BSR 0x00557E	; size=4 opcode=0x6100 flow=call
0x0000469A	BRA 0x0047E8	; size=4 opcode=0x6000 flow=branch
0x0000469E	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x000046A4	Bcc[6] 0x0047E8	; size=4 opcode=0x6600 flow=conditional
0x000046A8	BSR 0x0053C4	; size=4 opcode=0x6100 flow=call
0x000046AC	BRA 0x0047E8	; size=4 opcode=0x6000 flow=branch
0x000046B0	immediate-op	; size=8 opcode=0x0CA8 flow=fallthrough
0x000046B8	Bcc[6] 0x0046C6	; size=4 opcode=0x6600 flow=conditional
0x000046BC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000046C2	BRA 0x0046E0	; size=4 opcode=0x6000 flow=branch
0x000046C6	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x000046CC	Bcc[6] 0x0046E0	; size=4 opcode=0x6600 flow=conditional
0x000046D0	immediate-op	; size=8 opcode=0x0CA8 flow=fallthrough
0x000046D8	Bcc[7] 0x0046E0	; size=4 opcode=0x6700 flow=conditional
0x000046DC	BSR 0x0059EA	; size=4 opcode=0x6100 flow=call
0x000046E0	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x000046E4	Bcc[7] 0x0046F8	; size=4 opcode=0x6700 flow=conditional
0x000046E8	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x000046EA	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x000046EC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000046F2	MOVE	; size=4 opcode=0x2271 flow=fallthrough
0x000046F6	JSR	; size=2 opcode=0x4E91 flow=call
0x000046F8	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x000046FC	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x00004700	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00004704	bit-op	; size=2 opcode=0x0760 flow=fallthrough
0x00004706	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000470A	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x0000470E	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00004712	bit-op	; size=2 opcode=0x0726 flow=fallthrough
0x00004714	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x00004718	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x0000471C	immediate-op	; size=4 opcode=0x0012 flow=fallthrough
0x00004720	immediate-op	; size=4 opcode=0x0016 flow=fallthrough
0x00004724		; size=2 opcode=0x07FF flow=fallthrough
0x000047E8	DBcc 0x004678	; size=4 opcode=0x51CF flow=conditional
0x000047EC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000047F2	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x000047F6	Bcc[7] 0x0047FE	; size=4 opcode=0x6700 flow=conditional
0x000047FA	BSR 0x004ADC	; size=4 opcode=0x6100 flow=call
0x000047FE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00004804	Bcc[7] 0x00480C	; size=4 opcode=0x6700 flow=conditional
0x00004808	BSR 0x00AEBA	; size=4 opcode=0x6100 flow=call
0x0000480C	BSR 0x00481E	; size=4 opcode=0x6100 flow=call
0x00004810	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004816	ADDQ/SUBQ	; size=2 opcode=0x514B flow=fallthrough
0x00004818	unary-op	; size=4 opcode=0x422B flow=fallthrough
0x0000481C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000481E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00004824	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00004828	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000482C	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00004830	immediate-op	; size=6 opcode=0x0078 flow=fallthrough
0x00004836	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0000483A	Bcc[7] 0x0048A8	; size=4 opcode=0x6700 flow=conditional
0x0000483E	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00004844	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00004848	Bcc[11] 0x00486C	; size=4 opcode=0x6B00 flow=conditional
0x0000484C	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00004850	Bcc[5] 0x00488E	; size=4 opcode=0x6500 flow=conditional
0x00004854	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0000485A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000485E	Bcc[7] 0x004896	; size=4 opcode=0x6700 flow=conditional
0x00004862	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00004868	BRA 0x004896	; size=4 opcode=0x6000 flow=branch
0x0000486C	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x0000486E	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00004872	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x00004876	Bcc[7] 0x004884	; size=4 opcode=0x6700 flow=conditional
0x0000487A	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00004880	BRA 0x004896	; size=4 opcode=0x6000 flow=branch
0x00004884	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0000488A	BRA 0x004896	; size=4 opcode=0x6000 flow=branch
0x0000488E	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x00004890	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x00004892	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x00004896	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x0000489A	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x0000489E	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000048A2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000048A8	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x000048AC	DBcc 0x004828	; size=4 opcode=0x51CF flow=conditional
0x000048B0	RTS	; size=2 opcode=0x4E75 flow=return
0x00004ADC	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x00004AE0	Bcc[11] 0x004B6C	; size=4 opcode=0x6B00 flow=conditional
0x00004AE4	BSR 0x005A56	; size=4 opcode=0x6100 flow=call
0x00004AE8	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00004AEE	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00004AF2	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00004AF6	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00004AF8	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x00004AFC	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x00004B00	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00004B04	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00004B08	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00004B0C	MOVE	; size=4 opcode=0x3828 flow=fallthrough
0x00004B10	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00004B14	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00004B16	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00004B1C	MOVE	; size=4 opcode=0x2271 flow=fallthrough
0x00004B20	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00004B24	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00004B28	arithmetic-op	; size=2 opcode=0xD259 flow=fallthrough
0x00004B2A	arithmetic-op	; size=2 opcode=0xD459 flow=fallthrough
0x00004B2C	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00004B2E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00004B32	MUL/DIV	; size=4 opcode=0x80FC flow=fallthrough
0x00004B36	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x00004B38	arithmetic-op	; size=2 opcode=0xBE40 flow=fallthrough
0x00004B3A	Bcc[7] 0x004B50	; size=4 opcode=0x6700 flow=conditional
0x00004B3E	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00004B40	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00004B44	Bcc[6] 0x004B4A	; size=4 opcode=0x6600 flow=conditional
0x00004B48	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x00004B4A	arithmetic-op	; size=2 opcode=0xBE40 flow=fallthrough
0x00004B4C	Bcc[6] 0x004B56	; size=4 opcode=0x6600 flow=conditional
0x00004B50	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004B56	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00004B5A	DBcc 0x004B24	; size=4 opcode=0x51CF flow=conditional
0x00004B5E	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00004B64	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004B6A	RTS	; size=2 opcode=0x4E75 flow=return
0x00004B6C	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x00004B70	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00004B74	MOVE	; size=2 opcode=0x2F0B flow=fallthrough
0x00004B76	JSR	; size=6 opcode=0x4EB9 flow=call
0x00004B7C	MOVE	; size=2 opcode=0x265F flow=fallthrough
0x00004B7E	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00004B84	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00004B88	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00004B8A	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x00004B8E	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00004B94	MOVE	; size=2 opcode=0x2248 flow=fallthrough
0x00004B96	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x00004B9A	MOVE	; size=2 opcode=0x3019 flow=fallthrough
0x00004B9C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00004BA0	MOVE	; size=2 opcode=0x3F00 flow=fallthrough
0x00004BA2	arithmetic-op	; size=2 opcode=0xD05A flow=fallthrough
0x00004BA4	MOVE	; size=4 opcode=0x1999 flow=fallthrough
0x00004BA8	MOVE	; size=2 opcode=0x301F flow=fallthrough
0x00004BAA	DBcc 0x004BA0	; size=4 opcode=0x51CF flow=conditional
0x00004BAE	RTS	; size=2 opcode=0x4E75 flow=return
0x000053C4	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000053CA	Bcc[11] 0x0053E0	; size=4 opcode=0x6B00 flow=conditional
0x000053CE	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000053D4	Bcc[11] 0x0053DA	; size=4 opcode=0x6B00 flow=conditional
0x000053D8	RTS	; size=2 opcode=0x4E75 flow=return
0x000053DA	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000053E0	BSR 0x005494	; size=4 opcode=0x6100 flow=call
0x000053E4	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x000053E8	Bcc[7] 0x0053EE	; size=4 opcode=0x6700 flow=conditional
0x000053EC	RTS	; size=2 opcode=0x4E75 flow=return
0x000053EE	MOVE	; size=2 opcode=0x2248 flow=fallthrough
0x000053F0	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000053F4	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x000053F8	immediate-op	; size=6 opcode=0x0C69 flow=fallthrough
0x000053FE	Bcc[12] 0x005434	; size=4 opcode=0x6C00 flow=conditional
0x00005402	immediate-op	; size=6 opcode=0x0C29 flow=fallthrough
0x00005408	Bcc[6] 0x005430	; size=4 opcode=0x6600 flow=conditional
0x0000540C	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x00005410	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00005414	immediate-op	; size=4 opcode=0x001C flow=fallthrough
0x00005418	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x0000541C	Bcc[6] 0x005430	; size=4 opcode=0x6600 flow=conditional
0x00005420	BSR 0x005468	; size=4 opcode=0x6100 flow=call
0x00005424	MOVE	; size=2 opcode=0x2F08 flow=fallthrough
0x00005426	MOVE	; size=2 opcode=0x2049 flow=fallthrough
0x00005428	BSR 0x00AFF2	; size=4 opcode=0x6100 flow=call
0x0000542C	MOVE	; size=2 opcode=0x205F flow=fallthrough
0x0000542E	RTS	; size=2 opcode=0x4E75 flow=return
0x00005430	DBcc 0x0053F4	; size=4 opcode=0x51C8 flow=conditional
0x00005434	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0000543A	Bcc[10] 0x005454	; size=4 opcode=0x6A00 flow=conditional
0x0000543E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00005442	MOVE	; size=2 opcode=0x2F0B flow=fallthrough
0x00005444	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000544A	MOVE	; size=2 opcode=0x265F flow=fallthrough
0x0000544C	MOVE	; size=4 opcode=0x123C flow=fallthrough
0x00005450	BRA 0x005622	; size=4 opcode=0x6000 flow=branch
0x00005454	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0000545C	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x00005462	BSR 0x005468	; size=4 opcode=0x6100 flow=call
0x00005466	RTS	; size=2 opcode=0x4E75 flow=return
0x00005468	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00005470	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x00005476	MOVE	; size=6 opcode=0x3368 flow=fallthrough
0x0000547C	MOVE	; size=6 opcode=0x3368 flow=fallthrough
0x00005482	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x00005488	unary-op	; size=4 opcode=0x4229 flow=fallthrough
0x0000548C	MOVE	; size=6 opcode=0x2368 flow=fallthrough
0x00005492	RTS	; size=2 opcode=0x4E75 flow=return
0x00005494	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00005496	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x0000549A	Bcc[10] 0x0054AC	; size=4 opcode=0x6A00 flow=conditional
0x0000549E	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x000054A4	ADDQ/SUBQ	; size=4 opcode=0x5228 flow=fallthrough
0x000054A8	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000054AC	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x000054B0	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x000054B2	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x000054B6	Bcc[13] 0x005510	; size=4 opcode=0x6D00 flow=conditional
0x000054BA	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000054C0	Bcc[11] 0x00550A	; size=4 opcode=0x6B00 flow=conditional
0x000054C4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000054CA	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x000054D0	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x000054D2	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000054D6	arithmetic-op	; size=2 opcode=0x0800 flow=fallthrough
0x000054D8	immediate-op	; size=4 opcode=0x0001 flow=fallthrough
0x000054DC	immediate-op	; size=4 opcode=0x0004 flow=fallthrough
0x000054E0	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x000054E6	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x000054EA	arithmetic-op	; size=2 opcode=0x0801 flow=fallthrough
0x000054EC	immediate-op	; size=4 opcode=0x0002 flow=fallthrough
0x000054F0	immediate-op	; size=4 opcode=0x0004 flow=fallthrough
0x000054F4	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x000054FA	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x000054FE	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x00005504	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x00005506	BRA 0x005510	; size=4 opcode=0x6000 flow=branch
0x0000550A	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0000550E	RTS	; size=2 opcode=0x4E75 flow=return
0x00005510	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00005512	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00005518	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000551E	arithmetic-op	; size=2 opcode=0xE002 flow=fallthrough
0x00005520	Bcc[7] 0x00552A	; size=4 opcode=0x6700 flow=conditional
0x00005524	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0000552A	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0000552E	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x00005530	Bcc[7] 0x00553A	; size=4 opcode=0x6700 flow=conditional
0x00005534	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000553A	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x0000553E	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00005542	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00005546	arithmetic-op	; size=6 opcode=0xD279 flow=fallthrough
0x0000554C	arithmetic-op	; size=6 opcode=0xD479 flow=fallthrough
0x00005552	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x00005556	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x00005558	ADDQ/SUBQ	; size=2 opcode=0x5940 flow=fallthrough
0x0000555A	Bcc[11] 0x00556E	; size=4 opcode=0x6B00 flow=conditional
0x0000555E	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00005560	arithmetic-op	; size=2 opcode=0xD440 flow=fallthrough
0x00005562	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00005568	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0000556C	arithmetic-op	; size=2 opcode=0xD240 flow=fallthrough
0x0000556E	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00005572	MOVE	; size=4 opcode=0x3828 flow=fallthrough
0x00005576	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000557C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000557E	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00005584	Bcc[11] 0x00559A	; size=4 opcode=0x6B00 flow=conditional
0x00005588	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0000558E	Bcc[11] 0x005594	; size=4 opcode=0x6B00 flow=conditional
0x00005592	RTS	; size=2 opcode=0x4E75 flow=return
0x00005594	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0000559A	BSR 0x005662	; size=4 opcode=0x6100 flow=call
0x0000559E	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x000055A2	Bcc[7] 0x0055A8	; size=4 opcode=0x6700 flow=conditional
0x000055A6	RTS	; size=2 opcode=0x4E75 flow=return
0x000055A8	MOVE	; size=2 opcode=0x2248 flow=fallthrough
0x000055AA	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000055AE	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x000055B2	immediate-op	; size=6 opcode=0x0C69 flow=fallthrough
0x000055B8	Bcc[12] 0x0055EE	; size=4 opcode=0x6C00 flow=conditional
0x000055BC	immediate-op	; size=6 opcode=0x0C29 flow=fallthrough
0x000055C2	Bcc[6] 0x0055EA	; size=4 opcode=0x6600 flow=conditional
0x000055C6	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x000055CA	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x000055CE	immediate-op	; size=4 opcode=0x001C flow=fallthrough
0x000055D2	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x000055D6	Bcc[6] 0x0055EA	; size=4 opcode=0x6600 flow=conditional
0x000055DA	BSR 0x00563E	; size=4 opcode=0x6100 flow=call
0x000055DE	MOVE	; size=2 opcode=0x2F08 flow=fallthrough
0x000055E0	MOVE	; size=2 opcode=0x2049 flow=fallthrough
0x000055E2	BSR 0x00AFF2	; size=4 opcode=0x6100 flow=call
0x000055E6	MOVE	; size=2 opcode=0x205F flow=fallthrough
0x000055E8	RTS	; size=2 opcode=0x4E75 flow=return
0x000055EA	DBcc 0x0055AE	; size=4 opcode=0x51C8 flow=conditional
0x000055EE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000055F4	Bcc[10] 0x00560E	; size=4 opcode=0x6A00 flow=conditional
0x000055F8	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x000055FC	MOVE	; size=2 opcode=0x2F0B flow=fallthrough
0x000055FE	JSR	; size=6 opcode=0x4EB9 flow=call
0x00005604	MOVE	; size=2 opcode=0x265F flow=fallthrough
0x00005606	MOVE	; size=4 opcode=0x123C flow=fallthrough
0x0000560A	BRA 0x005622	; size=4 opcode=0x6000 flow=branch
0x0000560E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00005616	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x0000561C	BSR 0x00563E	; size=4 opcode=0x6100 flow=call
0x00005620	RTS	; size=2 opcode=0x4E75 flow=return
0x00005622	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00005628	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0000562C	unary-op	; size=4 opcode=0x4229 flow=fallthrough
0x00005630	arithmetic-op	; size=4 opcode=0xC329 flow=fallthrough
0x00005634	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x00005638	DBcc 0x00562C	; size=4 opcode=0x51C8 flow=conditional
0x0000563C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000563E	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x00005644	MOVE	; size=6 opcode=0x3368 flow=fallthrough
0x0000564A	MOVE	; size=6 opcode=0x3368 flow=fallthrough
0x00005650	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x00005656	unary-op	; size=4 opcode=0x4229 flow=fallthrough
0x0000565A	MOVE	; size=6 opcode=0x2368 flow=fallthrough
0x00005660	RTS	; size=2 opcode=0x4E75 flow=return
0x00005662	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00005664	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x00005668	Bcc[10] 0x00567A	; size=4 opcode=0x6A00 flow=conditional
0x0000566C	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x00005672	ADDQ/SUBQ	; size=4 opcode=0x5228 flow=fallthrough
0x00005676	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000567A	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x0000567E	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x00005680	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00005684	Bcc[13] 0x00568E	; size=4 opcode=0x6D00 flow=conditional
0x00005688	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0000568C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000568E	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00005690	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00005696	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0000569A	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x0000569C	Bcc[7] 0x0056A6	; size=4 opcode=0x6700 flow=conditional
0x000056A0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000056A6	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x000056AA	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x000056AE	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x000056B2	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x000056B6	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x000056B8	ADDQ/SUBQ	; size=2 opcode=0x5940 flow=fallthrough
0x000056BA	Bcc[11] 0x0056CE	; size=4 opcode=0x6B00 flow=conditional
0x000056BE	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x000056C0	arithmetic-op	; size=2 opcode=0xD440 flow=fallthrough
0x000056C2	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x000056C8	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000056CC	arithmetic-op	; size=2 opcode=0xD240 flow=fallthrough
0x000056CE	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000056D2	MOVE	; size=4 opcode=0x3828 flow=fallthrough
0x000056D6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000056DC	RTS	; size=2 opcode=0x4E75 flow=return
0x00005936	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0000593C	Bcc[7] 0x005988	; size=4 opcode=0x6700 flow=conditional
0x00005940	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00005944	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00005948	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0000594C	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00005952	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x00005956	Bcc[10] 0x005972	; size=4 opcode=0x6A00 flow=conditional
0x0000595A	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x00005960	ADDQ/SUBQ	; size=4 opcode=0x5228 flow=fallthrough
0x00005964	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x0000596A	Bcc[13] 0x005972	; size=4 opcode=0x6D00 flow=conditional
0x0000596E	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x00005972	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x00005976	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x00005978	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0000597C	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0000597E	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x00005982	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00005988	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x0000598C	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00005990	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00005994	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0000599A	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x0000599E	Bcc[10] 0x0059BA	; size=4 opcode=0x6A00 flow=conditional
0x000059A2	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x000059A8	ADDQ/SUBQ	; size=4 opcode=0x5228 flow=fallthrough
0x000059AC	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x000059B2	Bcc[13] 0x0059BA	; size=4 opcode=0x6D00 flow=conditional
0x000059B6	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x000059BA	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x000059BE	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x000059C0	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x000059C4	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x000059C6	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x000059CA	immediate-op	; size=4 opcode=0x0442 flow=fallthrough
0x000059CE	immediate-op	; size=4 opcode=0x0441 flow=fallthrough
0x000059D2	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x000059D6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000059DC	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x000059E0	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x000059E4	JMP	; size=6 opcode=0x4EF9 flow=branch
0x000059EA	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x000059EE	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x000059F2	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000059F6	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x000059FC	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x00005A00	Bcc[10] 0x005A1C	; size=4 opcode=0x6A00 flow=conditional
0x00005A04	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x00005A0A	ADDQ/SUBQ	; size=4 opcode=0x5228 flow=fallthrough
0x00005A0E	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00005A14	Bcc[13] 0x005A1C	; size=4 opcode=0x6D00 flow=conditional
0x00005A18	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x00005A1C	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x00005A20	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x00005A22	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00005A26	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00005A28	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x00005A2C	immediate-op	; size=4 opcode=0x0642 flow=fallthrough
0x00005A30	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00005A56	immediate-op	; size=8 opcode=0x0CA8 flow=fallthrough
0x00005A5E	Bcc[7] 0x005AC6	; size=4 opcode=0x6700 flow=conditional
0x00005A62	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x00005A64	arithmetic-op	; size=6 opcode=0x9079 flow=fallthrough
0x00005A6A	arithmetic-op	; size=2 opcode=0xE840 flow=fallthrough
0x00005A6C	Bcc[4] 0x005A72	; size=4 opcode=0x6400 flow=conditional
0x00005A70	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00005A72	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x00005A76	MOVE	; size=4 opcode=0x3140 flow=fallthrough
0x00005A7A	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00005A7E	arithmetic-op	; size=6 opcode=0x9279 flow=fallthrough
0x00005A84	arithmetic-op	; size=2 opcode=0xE841 flow=fallthrough
0x00005A86	Bcc[4] 0x005A8C	; size=4 opcode=0x6400 flow=conditional
0x00005A8A	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00005A8C	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x00005A90	arithmetic-op	; size=4 opcode=0xD268 flow=fallthrough
0x00005A94	MOVE	; size=4 opcode=0x3141 flow=fallthrough
0x00005A98	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00005A9C	Bcc[13] 0x005ABE	; size=4 opcode=0x6D00 flow=conditional
0x00005AA0	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00005AA4	Bcc[2] 0x005ABE	; size=4 opcode=0x6200 flow=conditional
0x00005AA8	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x00005AAC	Bcc[13] 0x005ABE	; size=4 opcode=0x6D00 flow=conditional
0x00005AB0	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x00005AB4	Bcc[4] 0x005ABE	; size=4 opcode=0x6400 flow=conditional
0x00005AB8	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00005ABC	RTS	; size=2 opcode=0x4E75 flow=return
0x00005ABE	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x00005AC4	RTS	; size=2 opcode=0x4E75 flow=return
0x00005AC6	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x00005AC8	arithmetic-op	; size=6 opcode=0x9079 flow=fallthrough
0x00005ACE	arithmetic-op	; size=2 opcode=0xE840 flow=fallthrough
0x00005AD0	Bcc[4] 0x005AD6	; size=4 opcode=0x6400 flow=conditional
0x00005AD4	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00005AD6	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x00005ADA	Bcc[11] 0x005B10	; size=4 opcode=0x6B00 flow=conditional
0x00005ADE	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00005AE2	Bcc[4] 0x005B10	; size=4 opcode=0x6400 flow=conditional
0x00005AE6	MOVE	; size=4 opcode=0x3140 flow=fallthrough
0x00005AEA	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00005AEE	arithmetic-op	; size=6 opcode=0x9279 flow=fallthrough
0x00005AF4	arithmetic-op	; size=2 opcode=0xE841 flow=fallthrough
0x00005AF6	Bcc[4] 0x005AFC	; size=4 opcode=0x6400 flow=conditional
0x00005AFA	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00005AFC	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x00005B00	arithmetic-op	; size=4 opcode=0xD268 flow=fallthrough
0x00005B04	Bcc[11] 0x005B10	; size=4 opcode=0x6B00 flow=conditional
0x00005B08	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x00005B0C	Bcc[5] 0x005B18	; size=4 opcode=0x6500 flow=conditional
0x00005B10	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x00005B16	RTS	; size=2 opcode=0x4E75 flow=return
0x00005B18	MOVE	; size=4 opcode=0x3141 flow=fallthrough
0x00005B1C	RTS	; size=2 opcode=0x4E75 flow=return
0x00005B1E	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00005B24	unary-op	; size=2 opcode=0x4244 flow=fallthrough
0x00005B26	MOVE	; size=4 opcode=0x363C flow=fallthrough
0x00005B2A	MOVE	; size=2 opcode=0x205C flow=fallthrough
0x00005B2C	MOVE	; size=2 opcode=0x225C flow=fallthrough
0x00005B2E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00005B32	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x00005B36	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00005B3A	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x00005B3E	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00005B42	Bcc[7] 0x005B48	; size=4 opcode=0x6700 flow=conditional
0x00005B46	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x00005B48	MOVE	; size=4 opcode=0x3229 flow=fallthrough
0x00005B4C	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x00005B50	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00005B54	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x00005B58	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00005B5C	Bcc[7] 0x005B62	; size=4 opcode=0x6700 flow=conditional
0x00005B60	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00005B62	arithmetic-op	; size=2 opcode=0xB041 flow=fallthrough
0x00005B64	Bcc[12] 0x005B72	; size=2 opcode=0x6C0C flow=conditional
0x00005B66	ADDQ/SUBQ	; size=2 opcode=0x514C flow=fallthrough
0x00005B68	MOVE	; size=2 opcode=0x28C9 flow=fallthrough
0x00005B6A	MOVE	; size=2 opcode=0x28C8 flow=fallthrough
0x00005B6C	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00005B70	BRA 0x005B74	; size=2 opcode=0x6002 flow=branch
0x00005B72	arithmetic-op	; size=2 opcode=0xC149 flow=fallthrough
0x00005B74	DBcc 0x005B2C	; size=4 opcode=0x51CB flow=conditional
0x00005B78	unary-op	; size=2 opcode=0x4A44 flow=fallthrough
0x00005B7A	Bcc[6] 0x005B1E	; size=2 opcode=0x66A2 flow=conditional
0x00005B7C	RTS	; size=2 opcode=0x4E75 flow=return
0x00005B7E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00005B84	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00005B88		; size=2 opcode=0x08E8 flow=fallthrough
0x00005FDC	MOVE	; size=8 opcode=0x33E8 flow=fallthrough
0x00005FE4	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00005FE8	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00005FEE	arithmetic-op	; size=4 opcode=0xD068 flow=fallthrough
0x00005FF2	arithmetic-op	; size=4 opcode=0xB068 flow=fallthrough
0x00005FF6	Bcc[13] 0x006008	; size=4 opcode=0x6D00 flow=conditional
0x00005FFA	ADDQ/SUBQ	; size=4 opcode=0x5268 flow=fallthrough
0x00005FFE		; size=2 opcode=0x08E8 flow=fallthrough
0x00006008	MOVE	; size=4 opcode=0x3140 flow=fallthrough
0x0000600C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000600E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00006012	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x00006014	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000601A	MOVE	; size=4 opcode=0x2271 flow=fallthrough
0x0000601E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00006022	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00006026	MOVE	; size=4 opcode=0x2471 flow=fallthrough
0x0000602A	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x0000602C	Bcc[10] 0x006060	; size=4 opcode=0x6A00 flow=conditional
0x00006030	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00006034	arithmetic-op	; size=4 opcode=0xB068 flow=fallthrough
0x00006038	Bcc[15] 0x006048	; size=4 opcode=0x6F00 flow=conditional
0x0000603C	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00006040	MOVE	; size=4 opcode=0x315A flow=fallthrough
0x00006044	BRA 0x0060AE	; size=4 opcode=0x6000 flow=branch
0x00006048	immediate-op	; size=6 opcode=0x0068 flow=fallthrough
0x0000604E	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x00006050	MOVE	; size=4 opcode=0x3140 flow=fallthrough
0x00006054	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00006058	MOVE	; size=4 opcode=0x315A flow=fallthrough
0x0000605C	BRA 0x0060AE	; size=4 opcode=0x6000 flow=branch
0x00006060	arithmetic-op	; size=4 opcode=0xB068 flow=fallthrough
0x00006064	Bcc[14] 0x00606C	; size=4 opcode=0x6E00 flow=conditional
0x00006068	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0000606C	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00006070	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x00006074	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0000607A	Bcc[6] 0x006092	; size=4 opcode=0x6600 flow=conditional
0x0000607E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00006082	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00006086	Bcc[6] 0x0060AA	; size=4 opcode=0x6600 flow=conditional
0x0000608A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000608E	BRA 0x0060AA	; size=4 opcode=0x6000 flow=branch
0x00006092	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00006096	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x0000609A	Bcc[4] 0x0060A6	; size=4 opcode=0x6400 flow=conditional
0x0000609E	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x000060A2	BRA 0x0060AA	; size=4 opcode=0x6000 flow=branch
0x000060A6	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x000060AA	MOVE	; size=4 opcode=0x3141 flow=fallthrough
0x000060AE	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x000060B0	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x000060B4	immediate-op	; size=4 opcode=0x0C52 flow=fallthrough
0x000060B8	Bcc[6] 0x0060EE	; size=4 opcode=0x6600 flow=conditional
0x000060BC	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x000060C0	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x000060C4	immediate-op	; size=6 opcode=0x002A flow=fallthrough
0x000060CA	Bcc[4] 0x0060DC	; size=4 opcode=0x6400 flow=conditional
0x000060CE	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x000060D2	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x000060D6	immediate-op	; size=4 opcode=0x0024 flow=fallthrough
0x000060DA	immediate-op	; size=4 opcode=0x0014 flow=fallthrough
0x000060DC	MOVE	; size=8 opcode=0x3179 flow=fallthrough
0x000060DE		; size=2 opcode=0x00FF flow=fallthrough
0x000060E4	MOVE	; size=8 opcode=0x3179 flow=fallthrough
0x000060EC	RTS	; size=2 opcode=0x4E75 flow=return
0x000060EE	MOVE	; size=4 opcode=0x214A flow=fallthrough
0x000060F2	RTS	; size=2 opcode=0x4E75 flow=return
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
0x00006178	Bcc[7] 0x006180	; size=4 opcode=0x6700 flow=conditional
0x0000617C	MOVE	; size=4 opcode=0x2028 flow=fallthrough
0x00006180	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00006184	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x00006188	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000618E	MOVE	; size=2 opcode=0x2009 flow=fallthrough
0x00006190	ADDQ/SUBQ	; size=2 opcode=0x5800 flow=fallthrough
0x00006192	MOVE	; size=2 opcode=0x2240 flow=fallthrough
0x00006194	DBcc 0x00615E	; size=4 opcode=0x51CF flow=conditional
0x00006198	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000619E	RTS	; size=2 opcode=0x4E75 flow=return
0x000061A0	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x000061A6	MOVE	; size=4 opcode=0x2A6A flow=fallthrough
0x000061AA	MOVE	; size=2 opcode=0x228D flow=fallthrough
0x000061AC	MOVE	; size=4 opcode=0x322A flow=fallthrough
0x000061B0	MOVE	; size=4 opcode=0x3341 flow=fallthrough
0x000061B4	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x000061B8	arithmetic-op	; size=2 opcode=0xEB49 flow=fallthrough
0x000061BA	MOVE	; size=4 opcode=0x3341 flow=fallthrough
0x000061BE	ADDQ/SUBQ	; size=6 opcode=0x5039 flow=fallthrough
0x000061C4	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000061CA	RTS	; size=2 opcode=0x4E75 flow=return
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
0x0000A7B4	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0000A7BE	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0000A7C8	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000A7CE	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0000A7D8	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000A7DE	BSR 0x00A7E4	; size=4 opcode=0x6100 flow=call
0x0000A7E2	RTS	; size=2 opcode=0x4E75 flow=return
0x0000A7E4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000A7EA	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000A7F0	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000A7F4	MOVE	; size=2 opcode=0x20C9 flow=fallthrough
0x0000A7F6	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x0000A7FA	DBcc 0x00A7F4	; size=4 opcode=0x51CF flow=conditional
0x0000A7FE	RTS	; size=2 opcode=0x4E75 flow=return
0x0000A800	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000A806	Bcc[7] 0x00A82C	; size=4 opcode=0x6700 flow=conditional
0x0000A80A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0000A812	Bcc[6] 0x00A8AC	; size=4 opcode=0x6600 flow=conditional
0x0000A816	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0000A81C	MOVE	; size=6 opcode=0x23CB flow=fallthrough
0x0000A822	MOVE	; size=6 opcode=0x23D3 flow=fallthrough
0x0000A828	BRA 0x00A8AC	; size=4 opcode=0x6000 flow=branch
0x0000A82C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000A832	Bcc[7] 0x00A84C	; size=4 opcode=0x6700 flow=conditional
0x0000A836	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0000A83C	MOVE	; size=6 opcode=0x23CB flow=fallthrough
0x0000A842	MOVE	; size=6 opcode=0x23D3 flow=fallthrough
0x0000A848	BRA 0x00A8AC	; size=4 opcode=0x6000 flow=branch
0x0000A84C	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0000A854	Bcc[6] 0x00A8AC	; size=4 opcode=0x6600 flow=conditional
0x0000A858	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0000A860	Bcc[6] 0x00A8AC	; size=4 opcode=0x6600 flow=conditional
0x0000A864	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000A86A	Bcc[6] 0x00A880	; size=4 opcode=0x6600 flow=conditional
0x0000A86E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000A872	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000A878	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x0000A87A	MOVE	; size=6 opcode=0x13C0 flow=fallthrough
0x0000A880	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0000A886	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x0000A88C	Bcc[6] 0x00A8AC	; size=4 opcode=0x6600 flow=conditional
0x0000A890	MOVE	; size=6 opcode=0x2839 flow=fallthrough
0x0000A896	Bcc[6] 0x00A8AC	; size=4 opcode=0x6600 flow=conditional
0x0000A89A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000A8A0	MOVE	; size=6 opcode=0x23CB flow=fallthrough
0x0000A8A6	MOVE	; size=6 opcode=0x23D3 flow=fallthrough
0x0000A8AC	RTS	; size=2 opcode=0x4E75 flow=return
0x0000A8EA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000A8F0	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000A8F6	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000A8FA	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x0000A8FC	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000A900	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000A904	bit-op	; size=2 opcode=0x01C2 flow=fallthrough
0x0000A906	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0000A90A	Bcc[11] 0x00A912	; size=4 opcode=0x6B00 flow=conditional
0x0000A90E	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0000A912	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000A916	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000A91A	bit-op	; size=4 opcode=0x013A flow=fallthrough
0x0000A91E	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x0000A922	Bcc[6] 0x00AA74	; size=4 opcode=0x6600 flow=conditional
0x0000A926	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000A92A	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x0000A92E	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x0000A932	immediate-op	; size=4 opcode=0x0001 flow=fallthrough
0x0000A936	MOVE	; size=4 opcode=0x1140 flow=fallthrough
0x0000A93A	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0000A93E	Bcc[11] 0x00A9A6	; size=4 opcode=0x6B00 flow=conditional
0x0000A942	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000A946	Bcc[14] 0x00AA74	; size=4 opcode=0x6E00 flow=conditional
0x0000A94A	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000A94E	Bcc[6] 0x00A978	; size=4 opcode=0x6600 flow=conditional
0x0000A952		; size=2 opcode=0x08E8 flow=fallthrough
0x0000A978	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000A97C	Bcc[6] 0x00AA54	; size=4 opcode=0x6600 flow=conditional
0x0000A980		; size=2 opcode=0x08E8 flow=fallthrough
0x0000A9A6	immediate-op	; size=8 opcode=0x0CA8 flow=fallthrough
0x0000A9AE	Bcc[7] 0x00A9BE	; size=4 opcode=0x6700 flow=conditional
0x0000A9B2	immediate-op	; size=8 opcode=0x0CA8 flow=fallthrough
0x0000A9BA	Bcc[6] 0x00A9CC	; size=4 opcode=0x6600 flow=conditional
0x0000A9BE	BSR 0x00AB02	; size=4 opcode=0x6100 flow=call
0x0000A9C2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000A9C8	BRA 0x00AA74	; size=4 opcode=0x6000 flow=branch
0x0000A9CC	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000A9D0	Bcc[6] 0x00AA02	; size=4 opcode=0x6600 flow=conditional
0x0000A9D4		; size=2 opcode=0x08E8 flow=fallthrough
0x0000AA02	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000AA06	Bcc[6] 0x00AA54	; size=4 opcode=0x6600 flow=conditional
0x0000AA0A		; size=2 opcode=0x08E8 flow=fallthrough
0x0000AA54	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000AA58	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AA5C	immediate-op	; size=4 opcode=0x000A flow=fallthrough
0x0000AA60	immediate-op	; size=4 opcode=0x024E flow=fallthrough
0x0000AA64	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AA68	immediate-op	; size=4 opcode=0x0002 flow=fallthrough
0x0000AA6C	Bcc[7] 0x00AA74	; size=4 opcode=0x6700 flow=conditional
0x0000AA70	BSR 0x00AC84	; size=4 opcode=0x6100 flow=call
0x0000AA74	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000AA78	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AA7C	immediate-op	; size=4 opcode=0x0024 flow=fallthrough
0x0000AA80	MOVE	; size=4 opcode=0x122E flow=fallthrough
0x0000AA84	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x0000AA88	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x0000AA8C	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x0000AA90	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AA94	immediate-op	; size=6 opcode=0x003A flow=fallthrough
0x0000AA9A	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x0000AA9E	immediate-op	; size=6 opcode=0x003A flow=fallthrough
0x0000AAA4	immediate-op	; size=4 opcode=0x0011 flow=fallthrough
0x0000AAA8	immediate-op	; size=6 opcode=0x003A flow=fallthrough
0x0000AAAE	MOVE	; size=2 opcode=0x3F00 flow=fallthrough
0x0000AAB0	JSR	; size=2 opcode=0x4E91 flow=call
0x0000AAB2	BSR 0x00AC5E	; size=4 opcode=0x6100 flow=call
0x0000AAB6	MOVE	; size=2 opcode=0x301F flow=fallthrough
0x0000AAB8	arithmetic-op	; size=4 opcode=0xB068 flow=fallthrough
0x0000AABC	Bcc[7] 0x00AAC6	; size=4 opcode=0x6700 flow=conditional
0x0000AAC0		; size=2 opcode=0x08E8 flow=fallthrough
0x0000AAC6	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x0000AAC8	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x0000AACC	DBcc 0x00A8FA	; size=4 opcode=0x51CF flow=conditional
0x0000AAD0	RTS	; size=2 opcode=0x4E75 flow=return
0x0000AB02	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000AB06	immediate-op	; size=8 opcode=0x00B0 flow=fallthrough
0x0000AB0E	immediate-op	; size=6 opcode=0x0080 flow=fallthrough
0x0000AB14	illegal-line-F	; size=2 opcode=0xF210 flow=fallthrough
0x0000AC5E	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000AC62	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AC66	immediate-op	; size=4 opcode=0x0004 flow=fallthrough
0x0000AC68	RTS	; size=2 opcode=0x4E75 flow=return
0x0000AC6A	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0000AC70	Bcc[6] 0x00AC7C	; size=4 opcode=0x6600 flow=conditional
0x0000AC74	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x0000AC78	immediate-op	; size=8 opcode=0x00B0 flow=fallthrough
0x0000AC7C	Bcc[10] 0x00AC68	; size=2 opcode=0x6AEA flow=conditional
0x0000AC7E	BSR 0x00B7B8	; size=4 opcode=0x6100 flow=call
0x0000AC80	bit-op	; size=4 opcode=0x0B38 flow=fallthrough
0x0000AC82	BRA 0x00AC68	; size=2 opcode=0x60E4 flow=branch
0x0000AC84	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000AC88	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AC8C	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x0000AC90	immediate-op	; size=4 opcode=0x0001 flow=fallthrough
0x0000AC94	BRA 0x00AC9E	; size=4 opcode=0x6000 flow=branch
0x0000AC9E	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000ACA4	Bcc[7] 0x00ACAC	; size=4 opcode=0x6700 flow=conditional
0x0000ACA8	BSR 0x00AE5E	; size=4 opcode=0x6100 flow=call
0x0000ACAC	RTS	; size=2 opcode=0x4E75 flow=return
0x0000AE5E		; size=2 opcode=0x08E8 flow=fallthrough
0x0000AEBA	ADDQ/SUBQ	; size=6 opcode=0x5079 flow=fallthrough
0x0000AEC0	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0000AEC8	Bcc[5] 0x00AED4	; size=4 opcode=0x6500 flow=conditional
0x0000AECC	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000AED2	RTS	; size=2 opcode=0x4E75 flow=return
0x0000AED4	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0000AEDA	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0000AEE0	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0000AEE4	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0000AEEA	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0000AEF0	arithmetic-op	; size=2 opcode=0xE448 flow=fallthrough
0x0000AEF2	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0000AEF6	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0000AEF8	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0000AEFC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000AF02	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000AF06	unary-op	; size=2 opcode=0x4244 flow=fallthrough
0x0000AF08	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0000AF0C	MOVE	; size=4 opcode=0x3031 flow=fallthrough
0x0000AF10		; size=2 opcode=0xC1F9 flow=fallthrough
0x0000AF6C	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0000AF70		; size=2 opcode=0x08E8 flow=fallthrough
0x0000AFF2	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000AFF6	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000AFFA	immediate-op	; size=4 opcode=0x0004 flow=fallthrough
0x0000AFFE	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000B002	Bcc[7] 0x00B330	; size=4 opcode=0x6700 flow=conditional
0x0000B006	MOVE	; size=8 opcode=0x217C flow=fallthrough
0x0000B00E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0000B012	arithmetic-op	; size=2 opcode=0xE758 flow=fallthrough
0x0000B014	Bcc[10] 0x00B01A	; size=4 opcode=0x6A00 flow=conditional
0x0000B018	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x0000B01A	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0000B01E	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x0000B020	MOVE	; size=2 opcode=0x2F09 flow=fallthrough
0x0000B022	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000B028	MOVE	; size=4 opcode=0x3231 flow=fallthrough
0x0000B02C	MOVE	; size=2 opcode=0x225F flow=fallthrough
0x0000B02E	MOVE	; size=4 opcode=0x3141 flow=fallthrough
0x0000B032	BSR 0x00AF6C	; size=4 opcode=0x6100 flow=call
0x0000B036	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0000B03C	RTS	; size=2 opcode=0x4E75 flow=return
0x0000B330		; size=2 opcode=0x08E8 flow=fallthrough
0x0000B7B8	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0000B7BE	Bcc[6] 0x00B7CC	; size=4 opcode=0x6600 flow=conditional
0x0000B7C2		; size=2 opcode=0x08E8 flow=fallthrough
0x0000B7CC	RTS	; size=2 opcode=0x4E75 flow=return
0x0000C0C2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000C0C8	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0000C0CE	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x0000C0D4	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0000C0DA	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0000C0DE		; size=2 opcode=0x40E7 flow=fallthrough
0x0000C134	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x0000C136	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0000C138	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x0000C13A	Bcc[11] 0x00C142	; size=4 opcode=0x6B00 flow=conditional
0x0000C13E	MOVE	; size=2 opcode=0x1AC0 flow=fallthrough
0x0000C140	BRA 0x00C138	; size=2 opcode=0x60F6 flow=branch
0x0000C142	arithmetic-op	; size=2 opcode=0xE308 flow=fallthrough
0x0000C144	Bcc[11] 0x00C14E	; size=4 opcode=0x6B00 flow=conditional
0x0000C148	MOVE	; size=4 opcode=0x103C flow=fallthrough
0x0000C14C	BRA 0x00C13E	; size=2 opcode=0x60F0 flow=branch
0x0000C14E	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x0000C150	Bcc[11] 0x00C15C	; size=4 opcode=0x6B00 flow=conditional
0x0000C154	MOVE	; size=4 opcode=0x1AFC flow=fallthrough
0x0000C158	DBcc 0x00C154	; size=4 opcode=0x51C9 flow=conditional
0x0000C15C	MOVE	; size=2 opcode=0x201F flow=fallthrough
0x0000C15E	RTS	; size=2 opcode=0x4E75 flow=return
0x0000C168	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0000C16C		; size=2 opcode=0x40E7 flow=fallthrough
0x0000C216	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0000C21A		; size=2 opcode=0x40E7 flow=fallthrough
0x0000C376	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000C37A	BSR 0x00C3B2	; size=4 opcode=0x6100 flow=call
0x0000C37E	arithmetic-op	; size=6 opcode=0xD539 flow=fallthrough
0x0000C384	Bcc[12] 0x00C392	; size=4 opcode=0x6C00 flow=conditional
0x0000C388	MOVE	; size=6 opcode=0x13C3 flow=fallthrough
0x0000C38E	BRA 0x00C3A2	; size=4 opcode=0x6000 flow=branch
0x0000C392	arithmetic-op	; size=6 opcode=0xB639 flow=fallthrough
0x0000C398	Bcc[10] 0x00C3A2	; size=4 opcode=0x6A00 flow=conditional
0x0000C39C	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000C3A2	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000C3A6	BSR 0x00C3B2	; size=4 opcode=0x6100 flow=call
0x0000C3AA	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000C3B0	RTS	; size=2 opcode=0x4E75 flow=return
0x0000C3B2	arithmetic-op	; size=2 opcode=0x8244 flow=fallthrough
0x0000C3B4	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0000C3B6	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0000C3BC	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000C3C2	Bcc[11] 0x00C3C8	; size=4 opcode=0x6B00 flow=conditional
0x0000C3C6	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x0000C3C8	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0000C3CC	arithmetic-op	; size=2 opcode=0xD047 flow=fallthrough
0x0000C3CE		; size=2 opcode=0x40E7 flow=fallthrough
0x0000C3E6	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x0000C3EC	MOVEQ	; size=2 opcode=0x7214 flow=fallthrough
0x0000C3EE	MOVEQ	; size=2 opcode=0x7E1B flow=fallthrough
0x0000C3F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C3F6	RTS	; size=2 opcode=0x4E75 flow=return
0x0000C64C	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000C652	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000C658	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0000C660	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0000C668	Bcc[6] 0x00C6B0	; size=4 opcode=0x6600 flow=conditional
0x0000C66C	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000C672	illegal-line-F	; size=2 opcode=0xFC1C flow=fallthrough
0x0000C6B0	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0000C6B8	Bcc[6] 0x00C6D0	; size=4 opcode=0x6600 flow=conditional
0x0000C6BC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000C6C2	illegal-line-F	; size=2 opcode=0xFC1C flow=fallthrough
0x0000C6D0	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0000C6D8	Bcc[6] 0x00C700	; size=4 opcode=0x6600 flow=conditional
0x0000C6DC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000C6E2	illegal-line-F	; size=2 opcode=0xFC1C flow=fallthrough
0x0000C700	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0000C708	Bcc[6] 0x00C716	; size=4 opcode=0x6600 flow=conditional
0x0000C70C	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0000C712	illegal-line-F	; size=2 opcode=0xFC1C flow=fallthrough
0x0000C716	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000C71C	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0000C722	BSR 0x00C802	; size=4 opcode=0x6100 flow=call
0x0000C726	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x0000C72C	bit-op	; size=2 opcode=0x0110 flow=fallthrough
0x0000C72E	arithmetic-op	; size=6 opcode=0x08B9 flow=fallthrough
0x0000C734	bit-op	; size=2 opcode=0x0110 flow=fallthrough
0x0000C736		; size=2 opcode=0x08F9 flow=fallthrough
0x0000C7F8	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000C7FE	Bcc[7] 0x00C64C	; size=4 opcode=0x6700 flow=conditional
0x0000C802	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C808	unary-op	; size=2 opcode=0x4287 flow=fallthrough
0x0000C80A	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0000C80E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000C814	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000C818	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000C81C	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x0000C822	Bcc[7] 0x00C82E	; size=4 opcode=0x6700 flow=conditional
0x0000C826	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000C82A	Bcc[6] 0x00C858	; size=4 opcode=0x6600 flow=conditional
0x0000C82E	MOVE	; size=6 opcode=0x1E39 flow=fallthrough
0x0000C834	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0000C83C	BSR 0x00C8B0	; size=4 opcode=0x6100 flow=call
0x0000C840	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C846	MOVE	; size=2 opcode=0x2008 flow=fallthrough
0x0000C848	arithmetic-op	; size=6 opcode=0xB0B9 flow=fallthrough
0x0000C84E	Bcc[6] 0x00C858	; size=4 opcode=0x6600 flow=conditional
0x0000C852	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C858	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000C85E	arithmetic-op	; size=4 opcode=0x0828 flow=fallthrough
0x0000C862	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0000C866	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x0000C86C	Bcc[7] 0x00C878	; size=4 opcode=0x6700 flow=conditional
0x0000C870	unary-op	; size=4 opcode=0x4A28 flow=fallthrough
0x0000C874	Bcc[6] 0x00C8A2	; size=4 opcode=0x6600 flow=conditional
0x0000C878	MOVE	; size=6 opcode=0x1E39 flow=fallthrough
0x0000C87E	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0000C886	BSR 0x00C8B0	; size=4 opcode=0x6100 flow=call
0x0000C88A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C890	MOVE	; size=2 opcode=0x2008 flow=fallthrough
0x0000C892	arithmetic-op	; size=6 opcode=0xB0B9 flow=fallthrough
0x0000C898	Bcc[6] 0x00C8A2	; size=4 opcode=0x6600 flow=conditional
0x0000C89C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C8A2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C8A8	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000C8AE	RTS	; size=2 opcode=0x4E75 flow=return
0x0000C8B0	ADDQ/SUBQ	; size=2 opcode=0x5347 flow=fallthrough
0x0000C8B2	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0000C8B8	arithmetic-op	; size=4 opcode=0xDD35 flow=fallthrough
0x0000C8BC	Bcc[10] 0x00C8C6	; size=4 opcode=0x6A00 flow=conditional
0x0000C8C0	MOVE	; size=6 opcode=0x1BBC flow=fallthrough
0x0000C8C6	RTS	; size=2 opcode=0x4E75 flow=return
0x0000D81C	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x0000D81E	MOVE	; size=4 opcode=0x1228 flow=fallthrough
0x0000D822	arithmetic-op	; size=4 opcode=0xD228 flow=fallthrough
0x0000D826	arithmetic-op	; size=2 opcode=0xE249 flow=fallthrough
0x0000D828	RTS	; size=2 opcode=0x4E75 flow=return
0x0000E6F4	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E6F8	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E6FE	Bcc[6] 0x00E76E	; size=4 opcode=0x6600 flow=conditional
0x0000E702	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E706	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E70C	Bcc[6] 0x00E71C	; size=4 opcode=0x6600 flow=conditional
0x0000E710	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E716	MOVEQ	; size=2 opcode=0x7A04 flow=fallthrough
0x0000E718	BRA 0x00E724	; size=4 opcode=0x6000 flow=branch
0x0000E71C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E722	unary-op	; size=2 opcode=0x4285 flow=fallthrough
0x0000E724	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E728	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E72E	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x0000E730	MOVE	; size=4 opcode=0x3831 flow=fallthrough
0x0000E734	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E738	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E73E	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0000E742	Bcc[11] 0x00E75E	; size=4 opcode=0x6B00 flow=conditional
0x0000E746	Bcc[7] 0x00E754	; size=4 opcode=0x6700 flow=conditional
0x0000E74A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E750	BRA 0x00E764	; size=4 opcode=0x6000 flow=branch
0x0000E754	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E75A	BRA 0x00E764	; size=4 opcode=0x6000 flow=branch
0x0000E75E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E764	MOVEQ	; size=2 opcode=0x7E01 flow=fallthrough
0x0000E766	MOVEQ	; size=2 opcode=0x7200 flow=fallthrough
0x0000E768	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C5 flow=fallthrough
0x0000E76A	BRA 0x00E7BC	; size=4 opcode=0x6000 flow=branch
0x0000E76E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E772	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E778	Bcc[6] 0x00E788	; size=4 opcode=0x6600 flow=conditional
0x0000E77C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E782	unary-op	; size=2 opcode=0x4285 flow=fallthrough
0x0000E784	BRA 0x00E790	; size=4 opcode=0x6000 flow=branch
0x0000E788	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E78E	MOVEQ	; size=2 opcode=0x7A02 flow=fallthrough
0x0000E790	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E794	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E79A	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x0000E79C	MOVE	; size=4 opcode=0x3831 flow=fallthrough
0x0000E7A0	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0000E7A6	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0000E7AA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E7B0	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0000E7B4	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C0 flow=fallthrough
0x0000E7B6	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C5 flow=fallthrough
0x0000E7B8	MOVEQ	; size=2 opcode=0x7E00 flow=fallthrough
0x0000E7BA	MOVEQ	; size=2 opcode=0x7200 flow=fallthrough
0x0000E7BC	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0000E7C0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0000E7C6	RTS	; size=2 opcode=0x4E75 flow=return
0x0000EE10	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0000EE16	Bcc[6] 0x00EE1C	; size=4 opcode=0x6600 flow=conditional
0x0000EE1A	RTS	; size=2 opcode=0x4E75 flow=return
0x0000EE1C	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0000EE22	MOVEQ	; size=2 opcode=0x7000 flow=fallthrough
0x0000EE24	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000EE2A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000EE2E	MOVE	; size=2 opcode=0x20C0 flow=fallthrough
0x0000EE30	DBcc 0x00EE2E	; size=4 opcode=0x51CF flow=conditional
0x0000EE34	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x0000EE3A	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000EE40	MOVE	; size=2 opcode=0x3019 flow=fallthrough
0x0000EE42	Bcc[10] 0x00EE4C	; size=4 opcode=0x6A00 flow=conditional
0x0000EE46	BSR 0x00EE86	; size=4 opcode=0x6100 flow=call
0x0000EE4A	RTS	; size=2 opcode=0x4E75 flow=return
0x0000EE4C	MOVE	; size=2 opcode=0x3819 flow=fallthrough
0x0000EE4E	MOVE	; size=2 opcode=0x3219 flow=fallthrough
0x0000EE50	MOVE	; size=2 opcode=0x3A19 flow=fallthrough
0x0000EE52	MOVE	; size=2 opcode=0x3C19 flow=fallthrough
0x0000EE54	MOVE	; size=2 opcode=0x3400 flow=fallthrough
0x0000EE56	MOVE	; size=2 opcode=0x3601 flow=fallthrough
0x0000EE58	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x0000EE5C	arithmetic-op	; size=2 opcode=0xD241 flow=fallthrough
0x0000EE5E	arithmetic-op	; size=2 opcode=0xEA48 flow=fallthrough
0x0000EE60	arithmetic-op	; size=2 opcode=0xD041 flow=fallthrough
0x0000EE62	MOVE	; size=2 opcode=0x3E00 flow=fallthrough
0x0000EE64	MOVE	; size=4 opcode=0x1186 flow=fallthrough
0x0000EE68	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x0000EE6A	immediate-op	; size=4 opcode=0x0642 flow=fallthrough
0x0000EE6E	arithmetic-op	; size=2 opcode=0xB842 flow=fallthrough
0x0000EE70	Bcc[4] 0x00EE64	; size=2 opcode=0x64F2 flow=conditional
0x0000EE72	MOVE	; size=4 opcode=0x3429 flow=fallthrough
0x0000EE76	immediate-op	; size=4 opcode=0x0647 flow=fallthrough
0x0000EE7A	MOVE	; size=2 opcode=0x3007 flow=fallthrough
0x0000EE7C	immediate-op	; size=4 opcode=0x0643 flow=fallthrough
0x0000EE80	arithmetic-op	; size=2 opcode=0xBA43 flow=fallthrough
0x0000EE82	Bcc[4] 0x00EE64	; size=2 opcode=0x64E0 flow=conditional
0x0000EE84	BRA 0x00EE40	; size=2 opcode=0x60BA flow=branch
0x0000EE86	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0000EE8E	Bcc[6] 0x00EEF6	; size=4 opcode=0x6600 flow=conditional
0x0000EE92	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000EE98	Bcc[6] 0x00EEB2	; size=4 opcode=0x6600 flow=conditional
0x0000EE9C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000EEA2	Bcc[6] 0x00EEB2	; size=4 opcode=0x6600 flow=conditional
0x0000EEA6	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0000EEAE	Bcc[6] 0x00EEF6	; size=4 opcode=0x6600 flow=conditional
0x0000EEB2	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0000EEB8	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x0000EEBE	Bcc[6] 0x00EEF6	; size=4 opcode=0x6600 flow=conditional
0x0000EEC2	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0000EEC8	Bcc[6] 0x00EEF6	; size=4 opcode=0x6600 flow=conditional
0x0000EECC	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0000EED2	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x0000EED4	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0000EEDA	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0000EEDE	MOVE	; size=2 opcode=0x1218 flow=fallthrough
0x0000EEE0	immediate-op	; size=4 opcode=0x0C01 flow=fallthrough
0x0000EEE4	Bcc[6] 0x00EEF8	; size=4 opcode=0x6600 flow=conditional
0x0000EEE8	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x0000EEEA	Bcc[10] 0x00EEF8	; size=4 opcode=0x6A00 flow=conditional
0x0000EEEE	SUBA/CMPA/ADDA	; size=4 opcode=0x90FC flow=fallthrough
0x0000EEF2	MOVE	; size=4 opcode=0x10BC flow=fallthrough
0x0000EEF6	RTS	; size=2 opcode=0x4E75 flow=return
0x0000EEF8	DBcc 0x00EEDE	; size=4 opcode=0x51CF flow=conditional
0x0000EEFC	BRA 0x00EED4	; size=2 opcode=0x60D6 flow=branch
0x00010918	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00010920	BSR 0x010952	; size=4 opcode=0x6100 flow=call
0x00010924	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001092A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010930		; size=2 opcode=0x08F9 flow=fallthrough
0x00010952	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010958	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001095E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010964	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001096A	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00010970	unary-op	; size=6 opcode=0x42B9 flow=fallthrough
0x00010976	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001097C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010982	RTS	; size=2 opcode=0x4E75 flow=return
0x00010984	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001098C	BSR 0x010918	; size=2 opcode=0x618A flow=call
0x0001098E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010994	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010998	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001099C	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000109A0	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x000109A4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000109AA	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000109B0	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000109B4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000109BA	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000109C0	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000109C4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000109C8	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000109CC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000109D2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000109D8	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000109DC	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000109E0	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000109E4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000109EA	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000109F0	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000109F4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000109F8	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000109FC	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010A02	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010A08	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010A0C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010A12	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010A18	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010A1C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010A20	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010A24	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010A2A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010A30	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010A34	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010A3A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010A40	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010A44	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010A48	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010A4C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010A52	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00010A58	Bcc[7] 0x010A74	; size=4 opcode=0x6700 flow=conditional
0x00010A5C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010A62	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010A66	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010A6A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010A6E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010A74	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00010A7C	RTS	; size=2 opcode=0x4E75 flow=return
0x00010AF0	BSR 0x010918	; size=4 opcode=0x6100 flow=call
0x00010AF4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010AFA	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010AFE	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010B02	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010B06	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00010B0A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010B10	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010B16	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010B1A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010B1E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010B22	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010B28	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010B2E	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010B32	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010B38	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010B3E	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010B42	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010B46	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010B4A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010B50	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00010B58	RTS	; size=2 opcode=0x4E75 flow=return
0x00010B5A	BSR 0x010918	; size=4 opcode=0x6100 flow=call
0x00010B5E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010B64	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010B68	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010B6C	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010B70	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00010B74	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010B7A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010B80	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010B84	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010B88	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010B8C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010B92	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010B98	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010B9C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010BA2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00010BA8	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00010BAC	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00010BB0	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00010BB4	JSR	; size=6 opcode=0x4EB9 flow=call
0x00010BBA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00010BC2	RTS	; size=2 opcode=0x4E75 flow=return
0x00012C5A	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00012C5E	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00012C62	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00012C68	JSR	; size=6 opcode=0x4EB9 flow=call
0x00012C6E	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00012C72	RTS	; size=2 opcode=0x4E75 flow=return
0x000134D2	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000134D4	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x000134DA	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000134E0	MOVE	; size=8 opcode=0x13F1 flow=fallthrough
0x000134E8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000134EE	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x000134F4	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x000134FA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013500	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00013506	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0001350A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001350E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00013512	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00013516	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x00013518	MOVE	; size=2 opcode=0x3A07 flow=fallthrough
0x0001351A	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x0001351C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013522	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013528	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001352E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013534	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001353A	Bcc[6] 0x013542	; size=4 opcode=0x6600 flow=conditional
0x0001353E	BSR 0x00E6F4	; size=4 opcode=0x6100 flow=call
0x00013542	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00013548	Bcc[7] 0x0135D8	; size=4 opcode=0x6700 flow=conditional
0x0001354C	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00013554	BSR 0x013726	; size=4 opcode=0x6100 flow=call
0x00013558	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001355C	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x0001355E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013564	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001356A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013570	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00013576	Bcc[6] 0x01357E	; size=4 opcode=0x6600 flow=conditional
0x0001357A	BSR 0x00E6F4	; size=4 opcode=0x6100 flow=call
0x0001357E	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x00013580	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00013586	Bcc[6] 0x0135D0	; size=4 opcode=0x6600 flow=conditional
0x0001358A	DBcc 0x01355C	; size=4 opcode=0x51CF flow=conditional
0x0001358E	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00013596	BSR 0x013726	; size=4 opcode=0x6100 flow=call
0x0001359A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001359E	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x000135A0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000135A6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000135AC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000135B2	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000135B8	Bcc[6] 0x0135C0	; size=4 opcode=0x6600 flow=conditional
0x000135BC	BSR 0x00E6F4	; size=4 opcode=0x6100 flow=call
0x000135C0	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x000135C2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000135C8	Bcc[6] 0x0135D0	; size=4 opcode=0x6600 flow=conditional
0x000135CC	DBcc 0x01359E	; size=4 opcode=0x51CF flow=conditional
0x000135D0	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x000135D6	RTS	; size=2 opcode=0x4E75 flow=return
0x000135D8	BSR 0x013726	; size=4 opcode=0x6100 flow=call
0x000135DC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000135E2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000135E8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000135EE	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000135F4	Bcc[6] 0x0135FC	; size=4 opcode=0x6600 flow=conditional
0x000135F8	BSR 0x00E6F4	; size=4 opcode=0x6100 flow=call
0x000135FC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00013602	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00013604	Bcc[7] 0x01360C	; size=4 opcode=0x6700 flow=conditional
0x00013608	BRA 0x013618	; size=4 opcode=0x6000 flow=branch
0x0001360C	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00013612	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00013614	Bcc[7] 0x01362E	; size=4 opcode=0x6700 flow=conditional
0x00013618	ADDQ/SUBQ	; size=6 opcode=0x5339 flow=fallthrough
0x0001361E	Bcc[10] 0x01362A	; size=4 opcode=0x6A00 flow=conditional
0x00013622	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0001362A	BSR 0x013726	; size=4 opcode=0x6100 flow=call
0x0001362E	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00013634	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00013636	Bcc[7] 0x01363E	; size=4 opcode=0x6700 flow=conditional
0x0001363A	BRA 0x01364A	; size=4 opcode=0x6000 flow=branch
0x0001363E	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00013644	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00013646	Bcc[7] 0x013666	; size=4 opcode=0x6700 flow=conditional
0x0001364A	ADDQ/SUBQ	; size=6 opcode=0x5239 flow=fallthrough
0x00013650	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x00013658	Bcc[11] 0x013662	; size=4 opcode=0x6B00 flow=conditional
0x0001365C	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00013662	BSR 0x013726	; size=4 opcode=0x6100 flow=call
0x00013666	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0001366C	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0001366E	Bcc[7] 0x013684	; size=4 opcode=0x6700 flow=conditional
0x00013672	JSR	; size=6 opcode=0x4EB9 flow=call
0x00013678	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00013680	BRA 0x0136B6	; size=4 opcode=0x6000 flow=branch
0x00013684	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0001368A	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0001368C	Bcc[7] 0x013692	; size=4 opcode=0x6700 flow=conditional
0x00013690	BRA 0x013672	; size=2 opcode=0x60E0 flow=branch
0x00013692	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00013698	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0001369A	Bcc[7] 0x0136A4	; size=4 opcode=0x6700 flow=conditional
0x0001369E	JSR	; size=6 opcode=0x4EB9 flow=call
0x000136A4	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000136AA	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000136AC	Bcc[7] 0x0135DC	; size=4 opcode=0x6700 flow=conditional
0x000136B0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000136B6	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000136BC	Bcc[6] 0x0136DC	; size=4 opcode=0x6600 flow=conditional
0x000136C0	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x000136C6	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x000136C8	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x000136CE	Bcc[7] 0x0136DC	; size=4 opcode=0x6700 flow=conditional
0x000136D2	BSR 0x0136DE	; size=4 opcode=0x6100 flow=call
0x000136D6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000136DC	RTS	; size=2 opcode=0x4E75 flow=return
0x000136DE	JSR	; size=6 opcode=0x4EB9 flow=call
0x000136E4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000136EA	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000136F2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000136FA	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00013700	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00013706	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001370A	Bcc[6] 0x013724	; size=4 opcode=0x6600 flow=conditional
0x0001370E	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00013714	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001371C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00013724	RTS	; size=2 opcode=0x4E75 flow=return
0x00013726	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00013728	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0001372E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00013734	MOVE	; size=4 opcode=0x1030 flow=fallthrough
0x00013738	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0001373C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00013742	SUBA/CMPA/ADDA	; size=2 opcode=0xD1C0 flow=fallthrough
0x00013744	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0001374A	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x00013750	MOVEQ	; size=2 opcode=0x7C04 flow=fallthrough
0x00013752		; size=2 opcode=0x40E7 flow=fallthrough
0x00013874	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001387A	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00013880	Bcc[7] 0x013890	; size=4 opcode=0x6700 flow=conditional
0x00013884	BSR 0x01438C	; size=4 opcode=0x6100 flow=call
0x00013888	BSR 0x014446	; size=4 opcode=0x6100 flow=call
0x0001388C	BRA 0x0138B6	; size=4 opcode=0x6000 flow=branch
0x00013890	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00013896	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0001389A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001389E	Bcc[6] 0x0138AA	; size=4 opcode=0x6600 flow=conditional
0x000138A2	BSR 0x01438C	; size=4 opcode=0x6100 flow=call
0x000138A6	BRA 0x0138B6	; size=4 opcode=0x6000 flow=branch
0x000138AA	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x000138AE	Bcc[6] 0x0138B6	; size=4 opcode=0x6600 flow=conditional
0x000138B2	BSR 0x014446	; size=4 opcode=0x6100 flow=call
0x000138B6	RTS	; size=2 opcode=0x4E75 flow=return
0x0001438C	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00014392	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00014398	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0001439A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001439E	arithmetic-op	; size=4 opcode=0x082A flow=fallthrough
0x000143A2	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x000143A6	immediate-op	; size=4 opcode=0x0040 flow=fallthrough
0x000143AA	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x000143AE	Bcc[6] 0x0143E6	; size=4 opcode=0x6600 flow=conditional
0x000143B2	arithmetic-op	; size=4 opcode=0x082A flow=fallthrough
0x000143B6	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x000143BA	immediate-op	; size=6 opcode=0x002C flow=fallthrough
0x000143C0	immediate-op	; size=8 opcode=0x00B0 flow=fallthrough
0x000143C8	immediate-op	; size=4 opcode=0x0005 flow=fallthrough
0x000143CC	Bcc[6] 0x0143E6	; size=4 opcode=0x6600 flow=conditional
0x000143D0	arithmetic-op	; size=4 opcode=0x082A flow=fallthrough
0x000143D4	immediate-op	; size=6 opcode=0x009E flow=fallthrough
0x000143DA	unary-op	; size=4 opcode=0x4A6A flow=fallthrough
0x000143DE	Bcc[7] 0x0143E6	; size=4 opcode=0x6700 flow=conditional
0x000143E2	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x000143E4	MOVE	; size=2 opcode=0x28CA flow=fallthrough
0x000143E6	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x000143EA	DBcc 0x01439E	; size=4 opcode=0x51CF flow=conditional
0x000143EE	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x000143F4	RTS	; size=2 opcode=0x4E75 flow=return
0x00014446	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0001444C	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00014452	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x00014454	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00014458	arithmetic-op	; size=4 opcode=0x082A flow=fallthrough
0x0001445C	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00014460	immediate-op	; size=4 opcode=0x0022 flow=fallthrough
0x00014464	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00014468	Bcc[6] 0x014482	; size=4 opcode=0x6600 flow=conditional
0x0001446C	arithmetic-op	; size=4 opcode=0x082A flow=fallthrough
0x00014470	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00014474	immediate-op	; size=4 opcode=0x000E flow=fallthrough
0x00014478	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x0001447C	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x00014480	MOVE	; size=2 opcode=0x28CA flow=fallthrough
0x00014482	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x00014486	DBcc 0x014458	; size=4 opcode=0x51CF flow=conditional
0x0001448A	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x00014490	RTS	; size=2 opcode=0x4E75 flow=return
0x00015A8C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00015A92	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00015A96	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00015A9A	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x00015A9E	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00015AA2	immediate-op	; size=4 opcode=0x0012 flow=fallthrough
0x00015AA6	bit-op	; size=2 opcode=0x0100 flow=fallthrough
0x00015AA8	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00015AAA	DBcc 0x015A9A	; size=4 opcode=0x51CF flow=conditional
0x00015AAE	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x00015AB4	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x00015ABA	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00015AC0	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00015AC4	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x00015AC8	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00015ACC		; size=2 opcode=0x00DA flow=fallthrough
0x00015D3C	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00015D40	arithmetic-op	; size=2 opcode=0x9444 flow=fallthrough
0x00015D42	arithmetic-op	; size=2 opcode=0x9645 flow=fallthrough
0x00015D44	BSR 0x015D4E	; size=4 opcode=0x6100 flow=call
0x00015D48	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00015D4C	RTS	; size=2 opcode=0x4E75 flow=return
0x00015D4E	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x00015D50	unary-op	; size=2 opcode=0x4246 flow=fallthrough
0x00015D52	MOVE	; size=2 opcode=0x3802 flow=fallthrough
0x00015D54	Bcc[10] 0x015D58	; size=2 opcode=0x6A02 flow=conditional
0x00015D56	unary-op	; size=2 opcode=0x4444 flow=fallthrough
0x00015D58	MOVE	; size=2 opcode=0x3A03 flow=fallthrough
0x00015D5A	Bcc[10] 0x015D5E	; size=2 opcode=0x6A02 flow=conditional
0x00015D5C	unary-op	; size=2 opcode=0x4445 flow=fallthrough
0x00015D5E	arithmetic-op	; size=2 opcode=0xBA44 flow=fallthrough
0x00015D60	Bcc[5] 0x015D64	; size=2 opcode=0x6502 flow=conditional
0x00015D62	arithmetic-op	; size=2 opcode=0xC945 flow=fallthrough
0x00015D64		; size=2 opcode=0x40C0 flow=fallthrough
0x00015DCC	MOVE	; size=2 opcode=0x3802 flow=fallthrough
0x00015DCE	MOVE	; size=2 opcode=0x3403 flow=fallthrough
0x00015DD0	Bcc[7] 0x015DF0	; size=2 opcode=0x671E flow=conditional
0x00015DD2	arithmetic-op	; size=2 opcode=0xE04C flow=fallthrough
0x00015DD4	immediate-op	; size=4 opcode=0x0244 flow=fallthrough
0x00015DD8	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x00015DDE		; size=2 opcode=0xC5F5 flow=fallthrough
0x00015DF0	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x00015DF2	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x00015DF4	RTS	; size=2 opcode=0x4E75 flow=return
0x000164A8	MOVE	; size=6 opcode=0x2479 flow=fallthrough
0x000164AE	MOVE	; size=2 opcode=0x2A1A flow=fallthrough
0x000164B0	Bcc[7] 0x0164DA	; size=4 opcode=0x6700 flow=conditional
0x000164B4	BSR 0x0164F2	; size=4 opcode=0x6100 flow=call
0x000164B8	Bcc[5] 0x0164DA	; size=4 opcode=0x6500 flow=conditional
0x000164BC	MOVE	; size=4 opcode=0x30BC flow=fallthrough
0x000164C0	MOVE	; size=4 opcode=0x2145 flow=fallthrough
0x000164C4	MOVE	; size=4 opcode=0x215A flow=fallthrough
0x000164C8	MOVE	; size=8 opcode=0x217C flow=fallthrough
0x000164D0	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x000164D4	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x000164D8	BRA 0x0164AE	; size=2 opcode=0x60D4 flow=branch
0x000164DA	RTS	; size=2 opcode=0x4E75 flow=return
0x000164DC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000164E2	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000164E6	unary-op	; size=2 opcode=0x4250 flow=fallthrough
0x000164E8	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x000164EC	DBcc 0x0164E6	; size=4 opcode=0x51CF flow=conditional
0x000164F0	RTS	; size=2 opcode=0x4E75 flow=return
0x000164F2	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000164F8	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000164FE	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00016502	unary-op	; size=2 opcode=0x4A50 flow=fallthrough
0x00016504	Bcc[7] 0x016518	; size=4 opcode=0x6700 flow=conditional
0x00016508	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x0001650C	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x0001650E	DBcc 0x016502	; size=4 opcode=0x51CF flow=conditional
0x00016512	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x00016518	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x0001651E	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00016524	Bcc[7] 0x016608	; size=4 opcode=0x6700 flow=conditional
0x00016528	unary-op	; size=2 opcode=0x4A40 flow=fallthrough
0x0001652A	Bcc[7] 0x01666E	; size=4 opcode=0x6700 flow=conditional
0x0001652E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00016536	Bcc[6] 0x016578	; size=4 opcode=0x6600 flow=conditional
0x0001653A	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x00016540	unary-op	; size=4 opcode=0x4232 flow=fallthrough
0x00016544	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x00016548	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x0001654C	BSR 0x0166D0	; size=4 opcode=0x6100 flow=call
0x00016550	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00016556	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x00016558	MOVE	; size=6 opcode=0x32B9 flow=fallthrough
0x0001655E	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00016564	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x00016566	MOVE	; size=6 opcode=0x32B9 flow=fallthrough
0x0001656C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00016574	BRA 0x016674	; size=4 opcode=0x6000 flow=branch
0x00016578	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00016580	Bcc[6] 0x016674	; size=4 opcode=0x6600 flow=conditional
0x00016584	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0001658A	unary-op	; size=4 opcode=0x4A32 flow=fallthrough
0x0001658E	Bcc[11] 0x016598	; size=4 opcode=0x6B00 flow=conditional
0x00016592	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00016594	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00016596	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00016598	unary-op	; size=4 opcode=0x4232 flow=fallthrough
0x0001659C	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x000165A0	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x000165A4	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x000165A8	Bcc[6] 0x0165E0	; size=4 opcode=0x6600 flow=conditional
0x000165AC	BSR 0x01669C	; size=4 opcode=0x6100 flow=call
0x000165B0	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000165B6	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000165B8	SUBA/CMPA/ADDA	; size=6 opcode=0x92F9 flow=fallthrough
0x000165BE	MOVE	; size=4 opcode=0x32BC flow=fallthrough
0x000165C2	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x000165C8	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000165CA	MOVE	; size=4 opcode=0x32BC flow=fallthrough
0x000165CE	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000165D0	MOVE	; size=4 opcode=0x32BC flow=fallthrough
0x000165D4	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000165DC	BRA 0x016674	; size=4 opcode=0x6000 flow=branch
0x000165E0	BSR 0x0166D0	; size=4 opcode=0x6100 flow=call
0x000165E4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000165EA	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000165EC	MOVE	; size=4 opcode=0x32BC flow=fallthrough
0x000165F0	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x000165F6	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000165F8	MOVE	; size=4 opcode=0x32BC flow=fallthrough
0x000165FC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00016604	BRA 0x016674	; size=4 opcode=0x6000 flow=branch
0x00016608	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00016610	Bcc[6] 0x01662E	; size=4 opcode=0x6600 flow=conditional
0x00016614	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00016618	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0001661C	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x00016620	Bcc[6] 0x01662E	; size=4 opcode=0x6600 flow=conditional
0x00016624	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0001662E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x00016632	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x00016634	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x00016638	immediate-op	; size=4 opcode=0x0C41 flow=fallthrough
0x0001663C	Bcc[6] 0x01666E	; size=4 opcode=0x6600 flow=conditional
0x00016640	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00016644	BSR 0x0164F2	; size=4 opcode=0x6100 flow=call
0x00016648	MOVE	; size=4 opcode=0x30BC flow=fallthrough
0x0001664C	arithmetic-op	; size=6 opcode=0xD879 flow=fallthrough
0x00016652	MOVE	; size=4 opcode=0x3144 flow=fallthrough
0x00016656	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0001665C	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x00016662	MOVE	; size=8 opcode=0x217C flow=fallthrough
0x0001666A	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x0001666E	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x00016674	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x0001667A	unary-op	; size=2 opcode=0x4600 flow=fallthrough
0x0001667C	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x00016680	arithmetic-op	; size=2 opcode=0x0880 flow=fallthrough
0x00016682	ADDQ/SUBQ	; size=2 opcode=0x5100 flow=fallthrough
0x00016684	bit-op	; size=2 opcode=0x0900 flow=fallthrough
0x00016686	ADDQ/SUBQ	; size=2 opcode=0x5200 flow=fallthrough
0x00016688	bit-op	; size=2 opcode=0x0980 flow=fallthrough
0x0001668A	ADDQ/SUBQ	; size=2 opcode=0x5000 flow=fallthrough
0x0001668C	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x00016690	arithmetic-op	; size=2 opcode=0x0880 flow=fallthrough
0x00016692	ADDQ/SUBQ	; size=2 opcode=0x5B80 flow=fallthrough
0x00016694	bit-op	; size=2 opcode=0x0900 flow=fallthrough
0x00016696	ADDQ/SUBQ	; size=2 opcode=0x5C00 flow=fallthrough
0x00016698	bit-op	; size=2 opcode=0x0980 flow=fallthrough
0x0001669A	RTS	; size=2 opcode=0x4E75 flow=return
0x0001669C	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x0001669E	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x000166A2	arithmetic-op	; size=2 opcode=0xE848 flow=fallthrough
0x000166A4	arithmetic-op	; size=2 opcode=0xE849 flow=fallthrough
0x000166A6	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x000166AC	Bcc[5] 0x0166B8	; size=4 opcode=0x6500 flow=conditional
0x000166B0	immediate-op	; size=4 opcode=0x0440 flow=fallthrough
0x000166B4	BRA 0x0166BC	; size=4 opcode=0x6000 flow=branch
0x000166B8	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x000166BC	arithmetic-op	; size=2 opcode=0xEA48 flow=fallthrough
0x000166BE	arithmetic-op	; size=2 opcode=0xEA49 flow=fallthrough
0x000166C0	ADDQ/SUBQ	; size=2 opcode=0x5341 flow=fallthrough
0x000166C2	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x000166C4	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x000166CA	MUL/DIV	; size=2 opcode=0xC2C2 flow=fallthrough
0x000166CC	arithmetic-op	; size=2 opcode=0xD041 flow=fallthrough
0x000166CE	RTS	; size=2 opcode=0x4E75 flow=return
0x000166D0	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x000166D2	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x000166D6	arithmetic-op	; size=2 opcode=0xE848 flow=fallthrough
0x000166D8	arithmetic-op	; size=2 opcode=0xE849 flow=fallthrough
0x000166DA	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x000166E0	Bcc[5] 0x0166F6	; size=4 opcode=0x6500 flow=conditional
0x000166E4	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x000166EA	Bcc[4] 0x0166F6	; size=4 opcode=0x6400 flow=conditional
0x000166EE	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x000166F2	BRA 0x0166FA	; size=4 opcode=0x6000 flow=branch
0x000166F6	immediate-op	; size=4 opcode=0x0441 flow=fallthrough
0x000166FA	arithmetic-op	; size=2 opcode=0xEA48 flow=fallthrough
0x000166FC	arithmetic-op	; size=2 opcode=0xEA49 flow=fallthrough
0x000166FE	ADDQ/SUBQ	; size=2 opcode=0x5341 flow=fallthrough
0x00016700	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x00016702	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x00016708	MUL/DIV	; size=2 opcode=0xC2C2 flow=fallthrough
0x0001670A	arithmetic-op	; size=2 opcode=0xD041 flow=fallthrough
0x0001670C	RTS	; size=2 opcode=0x4E75 flow=return
0x00016AFA	MOVE	; size=6 opcode=0x3A39 flow=fallthrough
0x00016B00	immediate-op	; size=4 opcode=0x0445 flow=fallthrough
0x00016B04	arithmetic-op	; size=2 opcode=0xDA45 flow=fallthrough
0x00016B06	arithmetic-op	; size=2 opcode=0xDA45 flow=fallthrough
0x00016B08	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x00016B0E	MOVE	; size=4 opcode=0x2033 flow=fallthrough
0x00016B12	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00016B16	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00016B1A	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00016DEE	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00016DF4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00016DF8	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x00016DFC	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x00016E00	immediate-op	; size=4 opcode=0x0012 flow=fallthrough
0x00016E04	illegal-line-F	; size=2 opcode=0xFFF4 flow=fallthrough
0x00016E5C	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00016E62	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x00016E68	MOVE	; size=4 opcode=0x2269 flow=fallthrough
0x00016E6C	MOVE	; size=2 opcode=0x3019 flow=fallthrough
0x00016E6E	Bcc[7] 0x016E9E	; size=4 opcode=0x6700 flow=conditional
0x00016E72	MOVE	; size=2 opcode=0x3219 flow=fallthrough
0x00016E74	MOVE	; size=2 opcode=0x3419 flow=fallthrough
0x00016E76	unary-op	; size=2 opcode=0x4A1C flow=fallthrough
0x00016E78	Bcc[7] 0x016E8A	; size=4 opcode=0x6700 flow=conditional
0x00016E7C	unary-op	; size=2 opcode=0x4A42 flow=fallthrough
0x00016E7E	Bcc[10] 0x016E9C	; size=4 opcode=0x6A00 flow=conditional
0x00016E82	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x00016E86	BRA 0x016E9C	; size=4 opcode=0x6000 flow=branch
0x00016E8A	unary-op	; size=2 opcode=0x4A42 flow=fallthrough
0x00016E8C	Bcc[10] 0x016E94	; size=4 opcode=0x6A00 flow=conditional
0x00016E90	SUBA/CMPA/ADDA	; size=4 opcode=0x98FC flow=fallthrough
0x00016E94	immediate-op	; size=4 opcode=0x0242 flow=fallthrough
0x00016E98	BSR 0x016EA0	; size=4 opcode=0x6100 flow=call
0x00016E9C	BRA 0x016E6C	; size=2 opcode=0x60CE flow=branch
0x00016E9E	RTS	; size=2 opcode=0x4E75 flow=return
0x00016EA0	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x00016EA6	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x00016EA8	ADDQ/SUBQ	; size=2 opcode=0x5341 flow=fallthrough
0x00016EAA	MOVE	; size=4 opcode=0x363C flow=fallthrough
0x00016EAE	MOVE	; size=2 opcode=0x3E00 flow=fallthrough
0x00016EB0	MOVE	; size=2 opcode=0x3A02 flow=fallthrough
0x00016EB2	MOVE	; size=4 opcode=0x3583 flow=fallthrough
0x00016EB6	ADDQ/SUBQ	; size=2 opcode=0x5445 flow=fallthrough
0x00016EB8	DBcc 0x016EB2	; size=4 opcode=0x51CF flow=conditional
0x00016EBC	arithmetic-op	; size=6 opcode=0xD479 flow=fallthrough
0x00016EC2	DBcc 0x016EAE	; size=4 opcode=0x51C9 flow=conditional
0x00016EC6	RTS	; size=2 opcode=0x4E75 flow=return
0x000176BA	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x000176C0	MOVE	; size=6 opcode=0x223C flow=fallthrough
0x000176C6	MOVE	; size=6 opcode=0x243C flow=fallthrough
0x000176CC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000176D2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000176D8	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x000176DE	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x000176E4	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0001792E	MOVE	; size=4 opcode=0x363C flow=fallthrough
0x00017932	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00017938	MOVE	; size=2 opcode=0x2F04 flow=fallthrough
0x0001793A	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00017940	Bcc[7] 0x017962	; size=4 opcode=0x6700 flow=conditional
0x00017944	MOVE	; size=2 opcode=0x2F03 flow=fallthrough
0x00017946	BSR 0x017C3C	; size=4 opcode=0x6100 flow=call
0x0001794A	MOVE	; size=2 opcode=0x3C03 flow=fallthrough
0x0001794C	MOVE	; size=2 opcode=0x261F flow=fallthrough
0x0001794E	unary-op	; size=2 opcode=0x4A46 flow=fallthrough
0x00017950	Bcc[7] 0x017962	; size=4 opcode=0x6700 flow=conditional
0x00017954	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001795A	unary-op	; size=4 opcode=0x4A30 flow=fallthrough
0x0001795E	Bcc[10] 0x017968	; size=4 opcode=0x6A00 flow=conditional
0x00017962	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00017964	BRA 0x017970	; size=4 opcode=0x6000 flow=branch
0x00017968	MOVE	; size=2 opcode=0x3E03 flow=fallthrough
0x0001796A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017970	MUL/DIV	; size=4 opcode=0xCCFC flow=fallthrough
0x00017974	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001797A	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C6 flow=fallthrough
0x0001797C	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00017980	MOVEQ	; size=2 opcode=0x7E00 flow=fallthrough
0x00017982	MOVEQ	; size=2 opcode=0x7203 flow=fallthrough
0x00017984	MOVE	; size=2 opcode=0x281F flow=fallthrough
0x00017986	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001798C	immediate-op	; size=6 opcode=0x0484 flow=fallthrough
0x00017992	DBcc 0x017938	; size=4 opcode=0x51CB flow=conditional
0x00017996	RTS	; size=2 opcode=0x4E75 flow=return
0x00017A5E	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00017A64	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00017A68	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x00017A6E	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00017A70	MOVE	; size=4 opcode=0x1C36 flow=fallthrough
0x00017A74	Bcc[10] 0x017AA6	; size=4 opcode=0x6A00 flow=conditional
0x00017A78	BSR 0x019DAC	; size=4 opcode=0x6100 flow=call
0x00017A7C	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00017A82	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x00017A88	immediate-op	; size=6 opcode=0x0034 flow=fallthrough
0x00017A8E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017A94	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00017A9A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00017A9E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017AA4	RTS	; size=2 opcode=0x4E75 flow=return
0x00017AA6	unary-op	; size=4 opcode=0x4A36 flow=fallthrough
0x00017AAA	Bcc[14] 0x017AB6	; size=4 opcode=0x6E00 flow=conditional
0x00017AAE	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017AB4	BRA 0x017A94	; size=2 opcode=0x60DE flow=branch
0x00017AB6	BSR 0x017C3C	; size=4 opcode=0x6100 flow=call
0x00017ABA	Bcc[14] 0x017AC6	; size=4 opcode=0x6E00 flow=conditional
0x00017ABE	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017AC4	BRA 0x017A94	; size=2 opcode=0x60CE flow=branch
0x00017AC6	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00017ACC	Bcc[7] 0x017AD4	; size=4 opcode=0x6700 flow=conditional
0x00017AD0	BSR 0x017E04	; size=4 opcode=0x6100 flow=call
0x00017AD4	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00017ADA	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x00017AE0	arithmetic-op	; size=2 opcode=0xE34E flow=fallthrough
0x00017AE2	SUBA/CMPA/ADDA	; size=2 opcode=0xDBC6 flow=fallthrough
0x00017AE4	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00017AE6	MOVE	; size=2 opcode=0x1C1D flow=fallthrough
0x00017AE8	Bcc[11] 0x017BBE	; size=4 opcode=0x6B00 flow=conditional
0x00017AEC	MOVE	; size=2 opcode=0x2A06 flow=fallthrough
0x00017AEE	MUL/DIV	; size=4 opcode=0xCCFC flow=fallthrough
0x00017AF2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017AF8	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C6 flow=fallthrough
0x00017AFA	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00017B00	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00017B04	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017B0A	MOVE	; size=4 opcode=0x1A34 flow=fallthrough
0x00017B0E	MUL/DIV	; size=4 opcode=0xCAFC flow=fallthrough
0x00017B12	SUBA/CMPA/ADDA	; size=2 opcode=0xD5C5 flow=fallthrough
0x00017B14	immediate-op	; size=4 opcode=0x0C43 flow=fallthrough
0x00017B18	Bcc[11] 0x017B46	; size=4 opcode=0x6B00 flow=conditional
0x00017B1C	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x00017B1E	MOVE	; size=4 opcode=0x1236 flow=fallthrough
0x00017B22	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00017B28	Bcc[11] 0x017B38	; size=4 opcode=0x6B00 flow=conditional
0x00017B2C	Bcc[7] 0x017B36	; size=4 opcode=0x6700 flow=conditional
0x00017B30	ADDQ/SUBQ	; size=2 opcode=0x5441 flow=fallthrough
0x00017B32	BRA 0x017B38	; size=4 opcode=0x6000 flow=branch
0x00017B36	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00017B38	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00017B3C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017B42	BRA 0x017B52	; size=4 opcode=0x6000 flow=branch
0x00017B46	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017B4C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017B52	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00017B54	MOVE	; size=2 opcode=0x1C1D flow=fallthrough
0x00017B56	Bcc[11] 0x017BBE	; size=4 opcode=0x6B00 flow=conditional
0x00017B5A	MOVE	; size=2 opcode=0x2A06 flow=fallthrough
0x00017B5C	MUL/DIV	; size=4 opcode=0xCCFC flow=fallthrough
0x00017B60	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017B66	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C6 flow=fallthrough
0x00017B68	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00017B6E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00017B72	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017B78	MOVE	; size=4 opcode=0x1A34 flow=fallthrough
0x00017B7C	MUL/DIV	; size=4 opcode=0xCAFC flow=fallthrough
0x00017B80	SUBA/CMPA/ADDA	; size=2 opcode=0xD5C5 flow=fallthrough
0x00017B82	immediate-op	; size=4 opcode=0x0C43 flow=fallthrough
0x00017B86	Bcc[11] 0x017BB2	; size=4 opcode=0x6B00 flow=conditional
0x00017B8A	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x00017B8C	MOVE	; size=4 opcode=0x1236 flow=fallthrough
0x00017B90	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00017B96	Bcc[11] 0x017BA6	; size=4 opcode=0x6B00 flow=conditional
0x00017B9A	Bcc[7] 0x017BA4	; size=4 opcode=0x6700 flow=conditional
0x00017B9E	ADDQ/SUBQ	; size=2 opcode=0x5441 flow=fallthrough
0x00017BA0	BRA 0x017BA6	; size=4 opcode=0x6000 flow=branch
0x00017BA4	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00017BA6	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00017BAA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017BB0	RTS	; size=2 opcode=0x4E75 flow=return
0x00017BB2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017BB8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017BBE	RTS	; size=2 opcode=0x4E75 flow=return
0x00017C3C	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x00017C42	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x00017C48	MOVE	; size=4 opcode=0x1634 flow=fallthrough
0x00017C4C	EXT	; size=2 opcode=0x4883 flow=fallthrough
0x00017C4E	arithmetic-op	; size=2 opcode=0xE84B flow=fallthrough
0x00017C50	RTS	; size=2 opcode=0x4E75 flow=return
0x00017D72	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x00017D76	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00017D7C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00017D80	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00017D84	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00017D88	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017D8E	RTS	; size=2 opcode=0x4E75 flow=return
0x00017D90	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00017D96	SUBA/CMPA/ADDA	; size=6 opcode=0xD1FC flow=fallthrough
0x00017D9C	arithmetic-op	; size=2 opcode=0xD310 flow=fallthrough
0x00017D9E	Bcc[14] 0x017DA8	; size=4 opcode=0x6E00 flow=conditional
0x00017DA2	unary-op	; size=2 opcode=0x4210 flow=fallthrough
0x00017DA4	BRA 0x017DDC	; size=4 opcode=0x6000 flow=branch
0x00017DA8	immediate-op	; size=4 opcode=0x0C10 flow=fallthrough
0x00017DAC	Bcc[15] 0x017DDC	; size=4 opcode=0x6F00 flow=conditional
0x00017DB0	MOVE	; size=4 opcode=0x10BC flow=fallthrough
0x00017DB4	BRA 0x017DDC	; size=4 opcode=0x6000 flow=branch
0x00017DDC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017DE2	unary-op	; size=2 opcode=0x4282 flow=fallthrough
0x00017DE4	MOVE	; size=2 opcode=0x1410 flow=fallthrough
0x00017DE6	arithmetic-op	; size=2 opcode=0xE74A flow=fallthrough
0x00017DE8	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C2 flow=fallthrough
0x00017DEA	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00017DF0	MOVEQ	; size=2 opcode=0x7E00 flow=fallthrough
0x00017DF2	MOVEQ	; size=2 opcode=0x7203 flow=fallthrough
0x00017DF4	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00017DF8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017DFE	BSR 0x01792E	; size=4 opcode=0x6100 flow=call
0x00017E02	RTS	; size=2 opcode=0x4E75 flow=return
0x00017E04	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00017E08	BSR 0x017C3C	; size=4 opcode=0x6100 flow=call
0x00017E0C	Bcc[15] 0x017E72	; size=4 opcode=0x6F00 flow=conditional
0x00017E10	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00017E16	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017E1C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00017E20	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00017E24	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x00017E28	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017E2E	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x00017E34	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00017E3A	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00017E3E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017E44	unary-op	; size=2 opcode=0x4287 flow=fallthrough
0x00017E46	MOVE	; size=4 opcode=0x1E33 flow=fallthrough
0x00017E4A	MUL/DIV	; size=4 opcode=0xCEFC flow=fallthrough
0x00017E4E	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C7 flow=fallthrough
0x00017E50	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00017E56	MOVEQ	; size=2 opcode=0x7E06 flow=fallthrough
0x00017E58	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017E5E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017E64	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x00017E6A	MOVEQ	; size=2 opcode=0x7E01 flow=fallthrough
0x00017E6C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00017E72	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00017E76	RTS	; size=2 opcode=0x4E75 flow=return
0x00017E78	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x00017E7E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00017E84	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00017E8A	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x00017E8E	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x00017E90	MOVE	; size=4 opcode=0x1831 flow=fallthrough
0x00017E94	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00017E9A	Bcc[11] 0x017EAA	; size=4 opcode=0x6B00 flow=conditional
0x00017E9E	Bcc[7] 0x017EA8	; size=4 opcode=0x6700 flow=conditional
0x00017EA2	ADDQ/SUBQ	; size=2 opcode=0x5444 flow=fallthrough
0x00017EA4	BRA 0x017EAA	; size=4 opcode=0x6000 flow=branch
0x00017EA8	ADDQ/SUBQ	; size=2 opcode=0x5244 flow=fallthrough
0x00017EAA	MOVEQ	; size=2 opcode=0x701B flow=fallthrough
0x00017EAC	arithmetic-op	; size=2 opcode=0x9044 flow=fallthrough
0x00017EAE	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00017EB2	RTS	; size=2 opcode=0x4E75 flow=return
0x00018776	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0001877A	MOVE	; size=2 opcode=0x3F06 flow=fallthrough
0x0001877C	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x00018780	MOVE	; size=2 opcode=0x3F05 flow=fallthrough
0x00018782	JSR	; size=6 opcode=0x4EB9 flow=call
0x00018788	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001878E	MOVE	; size=2 opcode=0x3A1F flow=fallthrough
0x00018790	DBcc 0x018780	; size=4 opcode=0x51CD flow=conditional
0x00018794	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001879A		; size=2 opcode=0x08F9 flow=fallthrough
0x0001989A	arithmetic-op	; size=6 opcode=0xD339 flow=fallthrough
0x000198A0	Bcc[7] 0x019904	; size=4 opcode=0x6700 flow=conditional
0x000198A4	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x000198AC	Bcc[7] 0x019904	; size=4 opcode=0x6700 flow=conditional
0x000198B0	Bcc[14] 0x0198C0	; size=4 opcode=0x6E00 flow=conditional
0x000198B4	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000198BC	BRA 0x019904	; size=4 opcode=0x6000 flow=branch
0x000198C0	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000198C8	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000198CE	Bcc[7] 0x019904	; size=4 opcode=0x6700 flow=conditional
0x000198D2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000198D8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000198DE	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000198E2	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x000198E8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000198EE	JSR	; size=6 opcode=0x4EB9 flow=call
0x000198F4	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x000198F8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000198FE	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00019904	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001990A	Bcc[11] 0x019974	; size=4 opcode=0x6B00 flow=conditional
0x0001990E	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x00019910	MOVE	; size=6 opcode=0x1239 flow=fallthrough
0x00019916	ADDQ/SUBQ	; size=2 opcode=0x5201 flow=fallthrough
0x00019918	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x0001991C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00019922	SUBA/CMPA/ADDA	; size=2 opcode=0xD3C1 flow=fallthrough
0x00019924	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0001992A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001992E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00019934	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0001993A	arithmetic-op	; size=6 opcode=0xB039 flow=fallthrough
0x00019940	Bcc[7] 0x019974	; size=4 opcode=0x6700 flow=conditional
0x00019944	MOVE	; size=10 opcode=0x13F9 flow=fallthrough
0x0001994E	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00019952	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019958	Bcc[11] 0x01996E	; size=4 opcode=0x6B00 flow=conditional
0x0001995C	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00019960	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019966	Bcc[7] 0x01996E	; size=4 opcode=0x6700 flow=conditional
0x0001996A	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0001996E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00019974	RTS	; size=2 opcode=0x4E75 flow=return
0x00019B40	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x00019B46	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x00019B4C	MUL/DIV	; size=4 opcode=0xC4FC flow=fallthrough
0x00019B50	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x00019B56	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00019B58	MOVE	; size=4 opcode=0x1036 flow=fallthrough
0x00019B5C	Bcc[15] 0x019B7E	; size=4 opcode=0x6F00 flow=conditional
0x00019B60	MOVE	; size=4 opcode=0x1036 flow=fallthrough
0x00019B64	Bcc[11] 0x019B7E	; size=4 opcode=0x6B00 flow=conditional
0x00019B68	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019B6E	Bcc[11] 0x019B82	; size=4 opcode=0x6B00 flow=conditional
0x00019B72	BSR 0x017C3C	; size=4 opcode=0x6100 flow=call
0x00019B76	Bcc[14] 0x019B82	; size=4 opcode=0x6E00 flow=conditional
0x00019B7A	MOVEQ	; size=2 opcode=0x70FF flow=fallthrough
0x00019B7C	RTS	; size=2 opcode=0x4E75 flow=return
0x00019B7E	MOVEQ	; size=2 opcode=0x7045 flow=fallthrough
0x00019B80	RTS	; size=2 opcode=0x4E75 flow=return
0x00019B82	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019B88	Bcc[15] 0x019BA4	; size=4 opcode=0x6F00 flow=conditional
0x00019B8C	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00019B90	Bcc[11] 0x019BA4	; size=4 opcode=0x6B00 flow=conditional
0x00019B94	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00019B98	Bcc[10] 0x019BA4	; size=4 opcode=0x6A00 flow=conditional
0x00019B9C	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x00019B9E	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x00019BA0	BRA 0x019BA6	; size=4 opcode=0x6000 flow=branch
0x00019BA4	arithmetic-op	; size=2 opcode=0xE348 flow=fallthrough
0x00019BA6	SUBA/CMPA/ADDA	; size=2 opcode=0xDBC0 flow=fallthrough
0x00019BA8	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00019BAA	MOVE	; size=2 opcode=0x1015 flow=fallthrough
0x00019BAC	RTS	; size=2 opcode=0x4E75 flow=return
0x00019BF6	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00019BF8	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00019BFA	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00019BFC	MOVE	; size=4 opcode=0x1031 flow=fallthrough
0x00019C00	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019C06	Bcc[15] 0x019C36	; size=4 opcode=0x6F00 flow=conditional
0x00019C0A	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00019C0E	Bcc[11] 0x019C36	; size=4 opcode=0x6B00 flow=conditional
0x00019C12	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00019C16	Bcc[10] 0x019C36	; size=4 opcode=0x6A00 flow=conditional
0x00019C1A	ADDQ/SUBQ	; size=2 opcode=0x5241 flow=fallthrough
0x00019C1C	MOVE	; size=4 opcode=0x1C31 flow=fallthrough
0x00019C20	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019C26	Bcc[11] 0x019C50	; size=4 opcode=0x6B00 flow=conditional
0x00019C2A	Bcc[7] 0x019C34	; size=4 opcode=0x6700 flow=conditional
0x00019C2E	ADDQ/SUBQ	; size=2 opcode=0x5446 flow=fallthrough
0x00019C30	BRA 0x019C36	; size=4 opcode=0x6000 flow=branch
0x00019C34	ADDQ/SUBQ	; size=2 opcode=0x5246 flow=fallthrough
0x00019C36	MOVE	; size=4 opcode=0x1C31 flow=fallthrough
0x00019C3A	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00019C40	Bcc[11] 0x019C50	; size=4 opcode=0x6B00 flow=conditional
0x00019C44	Bcc[7] 0x019C4E	; size=4 opcode=0x6700 flow=conditional
0x00019C48	ADDQ/SUBQ	; size=2 opcode=0x5446 flow=fallthrough
0x00019C4A	BRA 0x019C50	; size=4 opcode=0x6000 flow=branch
0x00019C4E	ADDQ/SUBQ	; size=2 opcode=0x5246 flow=fallthrough
0x00019C50	MOVE	; size=2 opcode=0x201F flow=fallthrough
0x00019C52	RTS	; size=2 opcode=0x4E75 flow=return
0x00019C5C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00019C62	MOVE	; size=4 opcode=0x1E30 flow=fallthrough
0x00019C66	MOVE	; size=4 opcode=0x1E30 flow=fallthrough
0x00019C6A	RTS	; size=2 opcode=0x4E75 flow=return
0x00019DAC	MOVE	; size=6 opcode=0x3639 flow=fallthrough
0x00019DB2	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00019DB8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00019DBE	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00019DC0	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x00019DC2	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00019DC8	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x00019DCC	MOVE	; size=4 opcode=0x1031 flow=fallthrough
0x00019DD0	arithmetic-op	; size=4 opcode=0x8130 flow=fallthrough
0x00019DD4	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00019DD8	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00019DDE	SUBA/CMPA/ADDA	; size=2 opcode=0xD1C0 flow=fallthrough
0x00019DE0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00019DE6	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A2CC	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0001A2D2	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001A2D6	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001A2DA	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0001A2DE	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x0001A2E0	unary-op	; size=2 opcode=0x4A1B flow=fallthrough
0x0001A2E2	Bcc[7] 0x01A2FA	; size=4 opcode=0x6700 flow=conditional
0x0001A2E6	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x0001A2E8	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x0001A2EC	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x0001A2F0	DBcc 0x01A2E0	; size=4 opcode=0x51CF flow=conditional
0x0001A2F4	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x0001A2FA	SUBA/CMPA/ADDA	; size=4 opcode=0x96FC flow=fallthrough
0x0001A2FE	MOVE	; size=4 opcode=0x16BC flow=fallthrough
0x0001A302	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A304	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0001A30A	unary-op	; size=4 opcode=0x4233 flow=fallthrough
0x0001A30E	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A310	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0001A316	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001A31A	unary-op	; size=2 opcode=0x421B flow=fallthrough
0x0001A31C	DBcc 0x01A31A	; size=4 opcode=0x51CF flow=conditional
0x0001A320	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A322	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001A328	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001A32E	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001A332	arithmetic-op	; size=4 opcode=0x08A9 flow=fallthrough
0x0001A336	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0001A33A	bit-op	; size=2 opcode=0x0100 flow=fallthrough
0x0001A33C	DBcc 0x01A332	; size=4 opcode=0x51CF flow=conditional
0x0001A340	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A342	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001A348	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001A34C	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0001A350	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x0001A354	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0001A358	immediate-op	; size=4 opcode=0x0012 flow=fallthrough
0x0001A35C	bit-op	; size=2 opcode=0x0100 flow=fallthrough
0x0001A35E	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x0001A360	DBcc 0x01A350	; size=4 opcode=0x51CF flow=conditional
0x0001A364	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x0001A36A	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x0001A370	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001A376	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0001A37A	Bcc[6] 0x01A380	; size=4 opcode=0x6600 flow=conditional
0x0001A37E	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A380	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A386	Bcc[6] 0x01A88E	; size=4 opcode=0x6600 flow=conditional
0x0001A38A	BSR 0x01A538	; size=4 opcode=0x6100 flow=call
0x0001A38E	Bcc[6] 0x01A396	; size=4 opcode=0x6600 flow=conditional
0x0001A392	BRA 0x01A818	; size=4 opcode=0x6000 flow=branch
0x0001A396	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001A39C	Bcc[7] 0x01A466	; size=4 opcode=0x6700 flow=conditional
0x0001A3A0	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A3A6	Bcc[6] 0x01A466	; size=4 opcode=0x6600 flow=conditional
0x0001A3AA	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0001A3B0	Bcc[10] 0x01A466	; size=4 opcode=0x6A00 flow=conditional
0x0001A3B4	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A3BA	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0001A3C0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A3C6	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0001A3CC	Bcc[11] 0x01A466	; size=4 opcode=0x6B00 flow=conditional
0x0001A3D0	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001A3D4	Bcc[7] 0x01A442	; size=4 opcode=0x6700 flow=conditional
0x0001A3D8	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001A3DC	Bcc[7] 0x01A3F0	; size=4 opcode=0x6700 flow=conditional
0x0001A3E0	BSR 0x01A54E	; size=4 opcode=0x6100 flow=call
0x0001A3E4	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A3EC	BRA 0x01A466	; size=4 opcode=0x6000 flow=branch
0x0001A3F0	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0001A3F6	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x0001A3F8	Bcc[6] 0x01A400	; size=4 opcode=0x6600 flow=conditional
0x0001A3FC	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0001A400	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0001A404	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x0001A406	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001A40A	MUL/DIV	; size=2 opcode=0x82C0 flow=fallthrough
0x0001A40C	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x0001A412	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0001A41A	Bcc[7] 0x01A436	; size=4 opcode=0x6700 flow=conditional
0x0001A41E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0001A426	Bcc[6] 0x01A466	; size=4 opcode=0x6600 flow=conditional
0x0001A42A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A432	BRA 0x01A466	; size=4 opcode=0x6000 flow=branch
0x0001A436	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A43E	BRA 0x01A466	; size=4 opcode=0x6000 flow=branch
0x0001A442	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A448	MOVE	; size=6 opcode=0x223C flow=fallthrough
0x0001A44E	MUL/DIV	; size=2 opcode=0x82C0 flow=fallthrough
0x0001A450	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x0001A456	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A45E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A466	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A538	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001A53E	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0001A544	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0001A548	unary-op	; size=4 opcode=0x4A30 flow=fallthrough
0x0001A54C	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A54E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A556	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A55E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A566	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A56E	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A818	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A81E	Bcc[7] 0x01A824	; size=4 opcode=0x6700 flow=conditional
0x0001A822	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A824	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0001A82A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A832	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001A838	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A83E	Bcc[6] 0x01A856	; size=4 opcode=0x6600 flow=conditional
0x0001A842	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A84A	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0001A850	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0001A854	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A856	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0001A85C	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x0001A85E	MOVE	; size=4 opcode=0x1428 flow=fallthrough
0x0001A862	arithmetic-op	; size=2 opcode=0xE54A flow=fallthrough
0x0001A864	MOVE	; size=4 opcode=0x2468 flow=fallthrough
0x0001A868	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0001A86C	SUBA/CMPA/ADDA	; size=4 opcode=0xD4FC flow=fallthrough
0x0001A870	MOVE	; size=6 opcode=0x23D2 flow=fallthrough
0x0001A876	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A87E	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0001A882	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0001A888	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0001A88E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001A894	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A89A	Bcc[7] 0x01A8A0	; size=4 opcode=0x6700 flow=conditional
0x0001A89E	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A8A0	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x0001A8A4	Bcc[10] 0x01A8BA	; size=4 opcode=0x6A00 flow=conditional
0x0001A8A8	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0001A8AE	ADDQ/SUBQ	; size=4 opcode=0x5328 flow=fallthrough
0x0001A8B2	Bcc[10] 0x01A8BA	; size=4 opcode=0x6A00 flow=conditional
0x0001A8B6	BRA 0x01A8F4	; size=4 opcode=0x6000 flow=branch
0x0001A8BA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A8C0	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x0001A8C6	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0001A8CC	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x0001A8D2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A8D8	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0001A8DC	MOVE	; size=4 opcode=0x1028 flow=fallthrough
0x0001A8E0	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x0001A8E2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A8E8	MOVE	; size=2 opcode=0x2219 flow=fallthrough
0x0001A8EA	arithmetic-op	; size=2 opcode=0xC081 flow=fallthrough
0x0001A8EC	MOVE	; size=2 opcode=0x2880 flow=fallthrough
0x0001A8EE	DBcc 0x01A8DC	; size=4 opcode=0x51CE flow=conditional
0x0001A8F2	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A8F4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0001A8FA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A902	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0001A906	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A908	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A910	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A916	Bcc[7] 0x01A924	; size=4 opcode=0x6700 flow=conditional
0x0001A91A	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0001A920	BRA 0x01A92A	; size=4 opcode=0x6000 flow=branch
0x0001A924	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001A92A	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0001A930	unary-op	; size=2 opcode=0x4A40 flow=fallthrough
0x0001A932	Bcc[10] 0x01A940	; size=4 opcode=0x6A00 flow=conditional
0x0001A936	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001A93C	BRA 0x01A9AC	; size=4 opcode=0x6000 flow=branch
0x0001A940	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001A944	Bcc[6] 0x01A958	; size=4 opcode=0x6600 flow=conditional
0x0001A948	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0001A94E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A956	RTS	; size=2 opcode=0x4E75 flow=return
0x0001A958	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001A95C	Bcc[7] 0x01A986	; size=4 opcode=0x6700 flow=conditional
0x0001A960	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0001A964	Bcc[6] 0x01A9A0	; size=4 opcode=0x6600 flow=conditional
0x0001A968	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A96E	Bcc[6] 0x01A9A0	; size=4 opcode=0x6600 flow=conditional
0x0001A972	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A97A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A982	BRA 0x01A9A0	; size=4 opcode=0x6000 flow=branch
0x0001A986	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001A98C	Bcc[6] 0x01A9A0	; size=4 opcode=0x6600 flow=conditional
0x0001A990	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A998	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001A9A0	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0001A9A2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001A9A8	MOVE	; size=4 opcode=0x2271 flow=fallthrough
0x0001A9AC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001A9B2	MOVE	; size=4 opcode=0x1159 flow=fallthrough
0x0001A9B6	MOVE	; size=4 opcode=0x1159 flow=fallthrough
0x0001A9BA	MOVE	; size=4 opcode=0x2159 flow=fallthrough
0x0001A9BE	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0001A9C2	MOVE	; size=4 opcode=0x30BC flow=fallthrough
0x0001A9C6	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0001A9CC	RTS	; size=2 opcode=0x4E75 flow=return
0x0001B680	BSR 0x01B7D0	; size=4 opcode=0x6100 flow=call
0x0001B684	MOVE	; size=8 opcode=0x217C flow=fallthrough
0x0001B68C	MOVE	; size=8 opcode=0x217C flow=fallthrough
0x0001B694	RTS	; size=2 opcode=0x4E75 flow=return
0x0001B7D0	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001B7D6	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0001B7DC	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0001B7E2	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0001B7E6	unary-op	; size=4 opcode=0x4228 flow=fallthrough
0x0001B7EA	RTS	; size=2 opcode=0x4E75 flow=return
0x0001D3F4	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0001D3FA	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0001D3FC	MOVE	; size=4 opcode=0x2A75 flow=fallthrough
0x0001D400	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0001D402	MOVE	; size=2 opcode=0x121D flow=fallthrough
0x0001D404	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001D40A	MOVE	; size=4 opcode=0x1235 flow=fallthrough
0x0001D40E	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0001D410	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001D414	arithmetic-op	; size=2 opcode=0xE988 flow=fallthrough
0x0001D416	arithmetic-op	; size=2 opcode=0xE319 flow=fallthrough
0x0001D418	Bcc[4] 0x01D422	; size=4 opcode=0x6400 flow=conditional
0x0001D41C	immediate-op	; size=6 opcode=0x0080 flow=fallthrough
0x0001D422	DBcc 0x01D414	; size=4 opcode=0x51CF flow=conditional
0x0001D426	RTS	; size=2 opcode=0x4E75 flow=return
0x0001DBAA	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0001DBB2	Bcc[6] 0x01DBC2	; size=4 opcode=0x6600 flow=conditional
0x0001DBB6	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001DBBC	Bcc[6] 0x01DBC2	; size=4 opcode=0x6600 flow=conditional
0x0001DBC0	RTS	; size=2 opcode=0x4E75 flow=return
0x0001DBC2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001DBC8	Bcc[6] 0x01DBD4	; size=4 opcode=0x6600 flow=conditional
0x0001DBCC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001DBD4	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001DBDC	unary-op	; size=6 opcode=0x4AB9 flow=fallthrough
0x0001DBE2	Bcc[7] 0x01DBEE	; size=4 opcode=0x6700 flow=conditional
0x0001DBE6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001DBEE	RTS	; size=2 opcode=0x4E75 flow=return
0x0001DF00	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001DF06	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001DF0A	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x0001DF0E	immediate-op	; size=6 opcode=0x009E flow=fallthrough
0x0001DF14	DBcc 0x01DF0A	; size=4 opcode=0x51CF flow=conditional
0x0001DF18	RTS	; size=2 opcode=0x4E75 flow=return
0x0001E04A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0001E052	Bcc[6] 0x01E060	; size=4 opcode=0x6600 flow=conditional
0x0001E056	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E05C	BRA 0x01E098	; size=4 opcode=0x6000 flow=branch
0x0001E060	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0001E068	Bcc[6] 0x01E098	; size=4 opcode=0x6600 flow=conditional
0x0001E06C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001E074	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001E07A	Bcc[6] 0x01E098	; size=4 opcode=0x6600 flow=conditional
0x0001E07E	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0001E086	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0001E08A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E090	BSR 0x01DF00	; size=4 opcode=0x6100 flow=call
0x0001E094	BSR 0x01E0DC	; size=4 opcode=0x6100 flow=call
0x0001E098	RTS	; size=2 opcode=0x4E75 flow=return
0x0001E0DC	MOVE	; size=6 opcode=0x2279 flow=fallthrough
0x0001E0E2	MOVE	; size=4 opcode=0x2A69 flow=fallthrough
0x0001E0E6	unary-op	; size=2 opcode=0x4A95 flow=fallthrough
0x0001E0E8	Bcc[7] 0x01E164	; size=4 opcode=0x6700 flow=conditional
0x0001E0EC	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E0F2	Bcc[5] 0x01E164	; size=4 opcode=0x6500 flow=conditional
0x0001E0F6	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0001E0FC	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E102	Bcc[5] 0x01E164	; size=4 opcode=0x6500 flow=conditional
0x0001E106	MOVE	; size=2 opcode=0x281D flow=fallthrough
0x0001E108	MOVE	; size=2 opcode=0x265D flow=fallthrough
0x0001E10A	arithmetic-op	; size=2 opcode=0x0884 flow=fallthrough
0x0001E10C	immediate-op	; size=4 opcode=0x001F flow=fallthrough
0x0001E110	immediate-op	; size=4 opcode=0x001A flow=fallthrough
0x0001E114		; size=2 opcode=0x00FF flow=fallthrough
0x0001E164	RTS	; size=2 opcode=0x4E75 flow=return
0x0001E2E0	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0001E2E6	Bcc[7] 0x01E2EC	; size=4 opcode=0x6700 flow=conditional
0x0001E2EA	RTS	; size=2 opcode=0x4E75 flow=return
0x0001E2EC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001E2F4	MOVE	; size=10 opcode=0x23F9 flow=fallthrough
0x0001E2FE	RTS	; size=2 opcode=0x4E75 flow=return
0x0001E4AC	MOVE	; size=2 opcode=0x280E flow=fallthrough
0x0001E4AE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E4B4	Bcc[5] 0x01E4FE	; size=4 opcode=0x6500 flow=conditional
0x0001E4B8	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0001E4BE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E4C4	Bcc[5] 0x01E4FE	; size=4 opcode=0x6500 flow=conditional
0x0001E4C8	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0001E4CE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001E4D4		; size=2 opcode=0x08E9 flow=fallthrough
0x0001E4FE	RTS	; size=2 opcode=0x4E75 flow=return
0x0001E514	arithmetic-op	; size=4 opcode=0x08A8 flow=fallthrough
0x0001E518	immediate-op	; size=8 opcode=0x00B0 flow=fallthrough
0x0001E520		; size=2 opcode=0x00FF flow=fallthrough
0x0001E68C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001E694	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001E69A	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F246	MOVE	; size=2 opcode=0x2F08 flow=fallthrough
0x0001F248	MOVE	; size=2 opcode=0x2F09 flow=fallthrough
0x0001F24A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001F252	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F258	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F25E	BSR 0x01F292	; size=4 opcode=0x6100 flow=call
0x0001F262	MOVE	; size=2 opcode=0x225F flow=fallthrough
0x0001F264	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F26A	Bcc[7] 0x01F270	; size=4 opcode=0x6700 flow=conditional
0x0001F26E	JSR	; size=2 opcode=0x4E91 flow=call
0x0001F270	MOVE	; size=2 opcode=0x205F flow=fallthrough
0x0001F272	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001F278	BSR 0x01F3C4	; size=4 opcode=0x6100 flow=call
0x0001F27C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F282	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F288	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001F290	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F292	MOVE	; size=2 opcode=0x2F06 flow=fallthrough
0x0001F294	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F29A	Bcc[7] 0x01F2A2	; size=4 opcode=0x6700 flow=conditional
0x0001F29E	BSR 0x01F32E	; size=4 opcode=0x6100 flow=call
0x0001F2A2	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001F2A8	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F2AE	MOVE	; size=2 opcode=0x205D flow=fallthrough
0x0001F2B0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F2B6	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001F2BC	MOVE	; size=2 opcode=0x265D flow=fallthrough
0x0001F2BE	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x0001F2C4	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F2CA	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F2D0	Bcc[11] 0x01F2F6	; size=4 opcode=0x6B00 flow=conditional
0x0001F2D4	MOVE	; size=2 opcode=0x225D flow=fallthrough
0x0001F2D6	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0001F2DA	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001F2DE	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001F2E2	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F2E6	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x0001F2E8	MOVE	; size=2 opcode=0x3A07 flow=fallthrough
0x0001F2EA	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x0001F2EC	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F2F2	BRA 0x01F32A	; size=4 opcode=0x6000 flow=branch
0x0001F2F6	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F2FC	Bcc[11] 0x01F30A	; size=4 opcode=0x6B00 flow=conditional
0x0001F300	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001F306	BRA 0x01F316	; size=4 opcode=0x6000 flow=branch
0x0001F30A	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001F310	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001F316	BSR 0x01F38A	; size=4 opcode=0x6100 flow=call
0x0001F31A	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x0001F320	BSR 0x01F34A	; size=4 opcode=0x6100 flow=call
0x0001F324	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001F32A	MOVE	; size=2 opcode=0x2C1F flow=fallthrough
0x0001F32C	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F32E	unary-op	; size=2 opcode=0x4283 flow=fallthrough
0x0001F330	MOVE	; size=6 opcode=0x1639 flow=fallthrough
0x0001F336	arithmetic-op	; size=2 opcode=0xE54B flow=fallthrough
0x0001F338	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0001F33E	MOVE	; size=4 opcode=0x2A75 flow=fallthrough
0x0001F342	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001F348	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F34A	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0001F34C	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0001F352	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0001F356	SUBA/CMPA/ADDA	; size=2 opcode=0xD1C0 flow=fallthrough
0x0001F358	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x0001F35C	MUL/DIV	; size=4 opcode=0x84FC flow=fallthrough
0x0001F360	ADDQ/SUBQ	; size=2 opcode=0x5342 flow=fallthrough
0x0001F362	arithmetic-op	; size=2 opcode=0xE54A flow=fallthrough
0x0001F364	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001F36A	MOVE	; size=4 opcode=0x2070 flow=fallthrough
0x0001F36E	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F374	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F378	MOVEQ	; size=2 opcode=0x7A01 flow=fallthrough
0x0001F37A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001F37E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001F382	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F388	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F38A	MOVE	; size=2 opcode=0x2F08 flow=fallthrough
0x0001F38C	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0001F38E	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0001F394	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x0001F398	SUBA/CMPA/ADDA	; size=2 opcode=0xD1C0 flow=fallthrough
0x0001F39A	MOVEQ	; size=2 opcode=0x7220 flow=fallthrough
0x0001F39C	MOVEQ	; size=2 opcode=0x7420 flow=fallthrough
0x0001F39E	MOVE	; size=4 opcode=0x2C28 flow=fallthrough
0x0001F3A2	MOVE	; size=2 opcode=0x2446 flow=fallthrough
0x0001F3A4	MOVEQ	; size=2 opcode=0x7C4F flow=fallthrough
0x0001F3A6	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0001F3A8	MOVE	; size=2 opcode=0x301A flow=fallthrough
0x0001F3AA	arithmetic-op	; size=2 opcode=0xEB80 flow=fallthrough
0x0001F3AC	immediate-op	; size=6 opcode=0x0680 flow=fallthrough
0x0001F3B2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F3B8	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x0001F3BC	DBcc 0x01F3A6	; size=4 opcode=0x51CE flow=conditional
0x0001F3C0	MOVE	; size=2 opcode=0x205F flow=fallthrough
0x0001F3C2	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F3C4	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F3CA	Bcc[7] 0x01F3D8	; size=4 opcode=0x6700 flow=conditional
0x0001F3CE	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001F3D4	BRA 0x01F404	; size=4 opcode=0x6000 flow=branch
0x0001F3D8	BSR 0x01F5A8	; size=4 opcode=0x6100 flow=call
0x0001F3DC	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0001F3E2	MOVE	; size=2 opcode=0x2258 flow=fallthrough
0x0001F3E4	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0001F3E6	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F3EC	MOVE	; size=4 opcode=0x1ABC flow=fallthrough
0x0001F3F0	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0001F3F6	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001F3FA	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F3FE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F404	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0001F40A	MOVE	; size=2 opcode=0x2258 flow=fallthrough
0x0001F40C	MOVE	; size=2 opcode=0x2209 flow=fallthrough
0x0001F40E	Bcc[11] 0x01F592	; size=4 opcode=0x6B00 flow=conditional
0x0001F412	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0001F414	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F41A	MOVE	; size=4 opcode=0x1ABC flow=fallthrough
0x0001F41E	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0001F424	MOVEQ	; size=2 opcode=0x7222 flow=fallthrough
0x0001F426	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F42A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F430	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001F436	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0001F43A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001F43E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001F442	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F446	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F44C	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0001F452	MOVE	; size=2 opcode=0x2258 flow=fallthrough
0x0001F454	MOVE	; size=2 opcode=0x2209 flow=fallthrough
0x0001F456	Bcc[6] 0x01F468	; size=4 opcode=0x6600 flow=conditional
0x0001F45A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0001F462	ADDQ/SUBQ	; size=2 opcode=0x5888 flow=fallthrough
0x0001F464	BRA 0x01F502	; size=4 opcode=0x6000 flow=branch
0x0001F468	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0001F46E	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0001F470	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F476	MOVE	; size=4 opcode=0x1ABC flow=fallthrough
0x0001F47A	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0001F480	MOVEQ	; size=2 opcode=0x7222 flow=fallthrough
0x0001F482	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F486	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F48C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001F492	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0001F496	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001F49A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001F49E	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F4A2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F4A8	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0001F4AE	MOVE	; size=2 opcode=0x2258 flow=fallthrough
0x0001F4B0	MOVE	; size=2 opcode=0x2209 flow=fallthrough
0x0001F4B2	Bcc[6] 0x01F4C2	; size=4 opcode=0x6600 flow=conditional
0x0001F4B6	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0001F4BE	BRA 0x01F502	; size=4 opcode=0x6000 flow=branch
0x0001F4C2	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0001F4C8	unary-op	; size=2 opcode=0x4241 flow=fallthrough
0x0001F4CA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F4D0	MOVE	; size=4 opcode=0x1ABC flow=fallthrough
0x0001F4D4	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0001F4DA	MOVEQ	; size=2 opcode=0x7222 flow=fallthrough
0x0001F4DC	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F4E0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F4E6	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0001F4EC	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0001F4F0	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0001F4F4	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0001F4F8	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0001F4FC	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F502	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0001F506	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F50C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F512	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F518	MOVE	; size=2 opcode=0x2F08 flow=fallthrough
0x0001F51A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F520	MOVE	; size=2 opcode=0x205F flow=fallthrough
0x0001F522	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0001F528	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0001F52A	Bcc[7] 0x01F540	; size=4 opcode=0x6700 flow=conditional
0x0001F52E	MOVE	; size=2 opcode=0x2050 flow=fallthrough
0x0001F530	MOVE	; size=2 opcode=0x2008 flow=fallthrough
0x0001F532	immediate-op	; size=6 opcode=0x0C80 flow=fallthrough
0x0001F538	Bcc[11] 0x01F572	; size=4 opcode=0x6B00 flow=conditional
0x0001F53C	BRA 0x01F3C4	; size=4 opcode=0x6000 flow=branch
0x0001F540	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0001F546	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0001F548	Bcc[7] 0x01F55A	; size=4 opcode=0x6700 flow=conditional
0x0001F54C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F552	Bcc[11] 0x01F50C	; size=2 opcode=0x6BB8 flow=conditional
0x0001F554	MOVE	; size=4 opcode=0x2068 flow=fallthrough
0x0001F558	BRA 0x01F530	; size=2 opcode=0x60D6 flow=branch
0x0001F55A	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0001F560	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0001F562	Bcc[7] 0x01F50C	; size=2 opcode=0x67A8 flow=conditional
0x0001F564	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0001F56A	Bcc[11] 0x01F50C	; size=2 opcode=0x6BA0 flow=conditional
0x0001F56C	MOVE	; size=4 opcode=0x2068 flow=fallthrough
0x0001F570	BRA 0x01F530	; size=2 opcode=0x60BE flow=branch
0x0001F572	immediate-op	; size=6 opcode=0x0C80 flow=fallthrough
0x0001F578	Bcc[10] 0x01F590	; size=4 opcode=0x6A00 flow=conditional
0x0001F57C	MOVE	; size=2 opcode=0x2008 flow=fallthrough
0x0001F57E	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0001F580	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0001F586	MOVE	; size=4 opcode=0x2070 flow=fallthrough
0x0001F58A	JSR	; size=2 opcode=0x4E90 flow=call
0x0001F58C	BRA 0x01F3C4	; size=4 opcode=0x6000 flow=branch
0x0001F590	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F592	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0001F596	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F59C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F5A2	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0001F5A6	RTS	; size=2 opcode=0x4E75 flow=return
0x0001F5A8	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x0001F5AE	MOVEQ	; size=2 opcode=0x720E flow=fallthrough
0x0001F5B0	MOVEQ	; size=2 opcode=0x7E1A flow=fallthrough
0x0001F5B2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F5B8	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x0001F5BE	MOVEQ	; size=2 opcode=0x7208 flow=fallthrough
0x0001F5C0	MOVEQ	; size=2 opcode=0x7E24 flow=fallthrough
0x0001F5C2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0001F5C8	RTS	; size=2 opcode=0x4E75 flow=return
0x00020714	MOVE	; size=6 opcode=0x2239 flow=fallthrough
0x0002071A	arithmetic-op	; size=2 opcode=0x0801 flow=fallthrough
0x0002071C	immediate-op	; size=4 opcode=0x0000 flow=fallthrough
0x00020720	immediate-op	; size=4 opcode=0x000C flow=fallthrough
0x00020724	immediate-op	; size=4 opcode=0x000F flow=fallthrough
0x00020728	BRA 0x020758	; size=4 opcode=0x6000 flow=branch
0x00020758	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x0002075E	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00020766	BRA 0x01F246	; size=4 opcode=0x6000 flow=branch
0x00026938	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0002693E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00026944	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00026948	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0002694C	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x0002694E	DBcc 0x02694C	; size=4 opcode=0x51CF flow=conditional
0x00026952	DBcc 0x026948	; size=4 opcode=0x51CE flow=conditional
0x00026956	RTS	; size=2 opcode=0x4E75 flow=return
0x00026A00	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00026A08	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00026A0C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00026A12	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x00026A18	MOVE	; size=6 opcode=0x223C flow=fallthrough
0x00026A1E	MOVE	; size=6 opcode=0x243C flow=fallthrough
0x00026A24	JSR	; size=6 opcode=0x4EB9 flow=call
0x00026A2A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00026A30	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00026A36	JSR	; size=6 opcode=0x4EB9 flow=call
0x00026A3C		; size=2 opcode=0x08F9 flow=fallthrough
0x00052DC2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00052DCA	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00052DD0	MOVE	; size=2 opcode=0x1100 flow=fallthrough
0x00052DD2	Bcc[6] 0x052DCA	; size=2 opcode=0x66F6 flow=conditional
0x00052DD4	RTS	; size=2 opcode=0x4E75 flow=return
0x00052DD6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00052DDE	RTS	; size=2 opcode=0x4E75 flow=return
0x00052DE0	MOVE	; size=2 opcode=0x2F09 flow=fallthrough
0x00052DE2		; size=2 opcode=0x40E7 flow=fallthrough
0x00052E26		; size=2 opcode=0x40E7 flow=fallthrough
0x00052E4E	MOVE	; size=2 opcode=0x205F flow=fallthrough
0x00052E50	LINK	; size=4 opcode=0x4E56 flow=fallthrough
0x00052E54	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00052E58		; size=2 opcode=0x40E7 flow=fallthrough
0x00052E84	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00052E8C		; size=2 opcode=0x46DF flow=fallthrough
0x00052E96	MOVE	; size=6 opcode=0x13BC flow=fallthrough
0x00052E9C	ADDQ/SUBQ	; size=2 opcode=0x5201 flow=fallthrough
0x00052E9E	immediate-op	; size=4 opcode=0x0201 flow=fallthrough
0x00052EA2	MOVE	; size=4 opcode=0x1380 flow=fallthrough
0x00052EA6	ADDQ/SUBQ	; size=2 opcode=0x5201 flow=fallthrough
0x00052EA8	immediate-op	; size=4 opcode=0x0201 flow=fallthrough
0x00052EAC	MOVE	; size=2 opcode=0x1081 flow=fallthrough
0x00052EAE	RTS	; size=2 opcode=0x4E75 flow=return
0x00052EB0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052EB6	MOVE	; size=4 opcode=0x202E flow=fallthrough
0x00052EBA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052EC0	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00052EC6	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052ECC	MOVE	; size=4 opcode=0x202E flow=fallthrough
0x00052ED0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052ED6	arithmetic-op	; size=2 opcode=0xE080 flow=fallthrough
0x00052ED8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052EDE	arithmetic-op	; size=2 opcode=0xE080 flow=fallthrough
0x00052EE0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052EE6	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00052EEC	LINK	; size=4 opcode=0x4E56 flow=fallthrough
0x00052EF0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052EF6	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052EFC	MOVEQ	; size=2 opcode=0x70FF flow=fallthrough
0x00052EFE	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00052F00	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F06	MOVEQ	; size=2 opcode=0x700B flow=fallthrough
0x00052F08	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00052F0A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F10	MOVE	; size=4 opcode=0x2F2E flow=fallthrough
0x00052F14	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F1A	MOVE	; size=4 opcode=0x2F2E flow=fallthrough
0x00052F1E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F24	MOVE	; size=4 opcode=0x2F2E flow=fallthrough
0x00052F28	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F2E	MOVE	; size=4 opcode=0x2F2E flow=fallthrough
0x00052F32	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F38	UNLK	; size=2 opcode=0x4E5E flow=fallthrough
0x00052F3A	RTS	; size=2 opcode=0x4E75 flow=return
0x00052F3C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F42	MOVEQ	; size=2 opcode=0x7010 flow=fallthrough
0x00052F44	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F4A	MOVE	; size=4 opcode=0x202E flow=fallthrough
0x00052F4E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F54	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00052F5A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F60	MOVEQ	; size=2 opcode=0x7012 flow=fallthrough
0x00052F62	BRA 0x052F44	; size=2 opcode=0x60E0 flow=branch
0x00052F6E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052F74	MOVEQ	; size=2 opcode=0x7020 flow=fallthrough
0x00052F76	BRA 0x052F44	; size=2 opcode=0x60CC flow=branch
0x00052FA0	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052FA6	MOVEQ	; size=2 opcode=0x7016 flow=fallthrough
0x00052FA8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00052FAE	JMP	; size=6 opcode=0x4EF9 flow=branch
0x000530D2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000530D8	Bcc[6] 0x053112	; size=4 opcode=0x6600 flow=conditional
0x000530DC	RTS	; size=2 opcode=0x4E75 flow=return
0x000530DE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000530E4	Bcc[6] 0x0530FE	; size=4 opcode=0x6600 flow=conditional
0x000530E8	RTS	; size=2 opcode=0x4E75 flow=return
0x000530EA	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x000530F0	Bcc[7] 0x0530FC	; size=4 opcode=0x6700 flow=conditional
0x000530F4	BSR 0x053112	; size=4 opcode=0x6100 flow=call
0x000530F8	BRA 0x0530FE	; size=4 opcode=0x6000 flow=branch
0x000530FC	RTS	; size=2 opcode=0x4E75 flow=return
0x000530FE	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00053102	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00053104	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005310A	ADDQ/SUBQ	; size=2 opcode=0x584F flow=fallthrough
0x0005310C	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00053110	RTS	; size=2 opcode=0x4E75 flow=return
0x00053112	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00053116	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00053118	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005311E	ADDQ/SUBQ	; size=2 opcode=0x584F flow=fallthrough
0x00053120	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00053124	RTS	; size=2 opcode=0x4E75 flow=return
0x00053126	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0005312A	MOVEQ	; size=2 opcode=0x7000 flow=fallthrough
0x0005312C	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00053130	immediate-op	; size=4 opcode=0x0600 flow=fallthrough
0x00053134	BSR 0x05315E	; size=4 opcode=0x6100 flow=call
0x00053138	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0005313C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00053142	DBcc 0x053130	; size=4 opcode=0x51CE flow=conditional
0x00053146	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005314C	BSR 0x053156	; size=4 opcode=0x6100 flow=call
0x00053150	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00053154	RTS	; size=2 opcode=0x4E75 flow=return
0x00053156	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0005315A	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0005315C	BRA 0x053162	; size=2 opcode=0x6004 flow=branch
0x0005315E	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00053162	MOVE	; size=2 opcode=0x2F00 flow=fallthrough
0x00053164	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005316A	ADDQ/SUBQ	; size=2 opcode=0x584F flow=fallthrough
0x0005316C	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00053170	RTS	; size=2 opcode=0x4E75 flow=return
0x000531AA	MOVE	; size=6 opcode=0x2F3C flow=fallthrough
0x000531B0	MOVE	; size=6 opcode=0x2F3C flow=fallthrough
0x000531B6	MOVE	; size=6 opcode=0x2F3C flow=fallthrough
0x000531BC	MOVE	; size=6 opcode=0x2F3C flow=fallthrough
0x000531C2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000531C8	SUBA/CMPA/ADDA	; size=4 opcode=0xDEFC flow=fallthrough
0x000531CC	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00053216	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00053218	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0005321E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00053224	arithmetic-op	; size=2 opcode=0xE748 flow=fallthrough
0x00053226	MOVE	; size=4 opcode=0x2070 flow=fallthrough
0x0005322A	RTS	; size=2 opcode=0x4E75 flow=return
0x00053290	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x00053296	LEA	; size=6 opcode=0x49F9 flow=fallthrough
0x0005329C	MOVEQ	; size=2 opcode=0x7E22 flow=fallthrough
0x0005329E	unary-op	; size=2 opcode=0x429B flow=fallthrough
0x000532A0	unary-op	; size=2 opcode=0x429C flow=fallthrough
0x000532A2	DBcc 0x05329E	; size=4 opcode=0x51CF flow=conditional
0x000532A6	RTS	; size=2 opcode=0x4E75 flow=return
0x00055F44	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x00055F48	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00055F4E	MOVE	; size=2 opcode=0x2F01 flow=fallthrough
0x00055F50	JSR	; size=6 opcode=0x4EB9 flow=call
0x00055F56	MOVE	; size=2 opcode=0x2C01 flow=fallthrough
0x00055F58	MOVE	; size=2 opcode=0x221F flow=fallthrough
0x00055F5A	arithmetic-op	; size=4 opcode=0xDC28 flow=fallthrough
0x00055F5E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00055F64	arithmetic-op	; size=4 opcode=0xDC28 flow=fallthrough
0x00055F68	arithmetic-op	; size=4 opcode=0xDC28 flow=fallthrough
0x00055F6C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00055F72	Bcc[14] 0x055FA8	; size=4 opcode=0x6E00 flow=conditional
0x00055F76	Bcc[11] 0x055FC0	; size=4 opcode=0x6B00 flow=conditional
0x00055F7A	arithmetic-op	; size=2 opcode=0xE24E flow=fallthrough
0x00055F7C	Bcc[4] 0x055F88	; size=4 opcode=0x6400 flow=conditional
0x00055F80	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x00055F82	Bcc[7] 0x055F88	; size=4 opcode=0x6700 flow=conditional
0x00055F86	ADDQ/SUBQ	; size=2 opcode=0x5246 flow=fallthrough
0x00055F88	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x00055F8A	Bcc[11] 0x055F92	; size=4 opcode=0x6B00 flow=conditional
0x00055F8E	BRA 0x055FA2	; size=4 opcode=0x6000 flow=branch
0x00055F92	unary-op	; size=2 opcode=0x4287 flow=fallthrough
0x00055F94	MOVE	; size=6 opcode=0x1E39 flow=fallthrough
0x00055F9A	arithmetic-op	; size=2 opcode=0xBE46 flow=fallthrough
0x00055F9C	Bcc[10] 0x055FA2	; size=4 opcode=0x6A00 flow=conditional
0x00055FA0	MOVE	; size=2 opcode=0x3C07 flow=fallthrough
0x00055FA2	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x00055FA6	RTS	; size=2 opcode=0x4E75 flow=return
0x00055FA8	MOVE	; size=2 opcode=0x2A06 flow=fallthrough
0x00055FAA	MOVE	; size=2 opcode=0x2E06 flow=fallthrough
0x00055FAC	MUL/DIV	; size=4 opcode=0x8EFC flow=fallthrough
0x00055FB0	arithmetic-op	; size=2 opcode=0x9C47 flow=fallthrough
0x00055FB2	arithmetic-op	; size=2 opcode=0x9A46 flow=fallthrough
0x00055FB4	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x00055FB6	Bcc[7] 0x055FBC	; size=4 opcode=0x6700 flow=conditional
0x00055FBA	BRA 0x055F92	; size=2 opcode=0x60D6 flow=branch
0x00055FBC	MOVE	; size=2 opcode=0x2C05 flow=fallthrough
0x00055FBE	BRA 0x055FA2	; size=2 opcode=0x60E2 flow=branch
0x00055FC0	MOVE	; size=2 opcode=0x2A06 flow=fallthrough
0x00055FC2	MOVE	; size=2 opcode=0x2E06 flow=fallthrough
0x00055FC4	MUL/DIV	; size=4 opcode=0x8EFC flow=fallthrough
0x00055FC8	arithmetic-op	; size=2 opcode=0x9C47 flow=fallthrough
0x00055FCA	arithmetic-op	; size=2 opcode=0x9A46 flow=fallthrough
0x00055FCC	unary-op	; size=2 opcode=0x4A41 flow=fallthrough
0x00055FCE	Bcc[11] 0x055FD4	; size=4 opcode=0x6B00 flow=conditional
0x00055FD2	BRA 0x055FA2	; size=2 opcode=0x60CE flow=branch
0x00055FD4	MOVE	; size=2 opcode=0x2C05 flow=fallthrough
0x00055FD6	BRA 0x055F92	; size=2 opcode=0x60BA flow=branch
0x000560AA	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000560B0	Bcc[11] 0x056108	; size=4 opcode=0x6B00 flow=conditional
0x000560B4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000560BA	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000560C0	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x000560C4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000560CA	MOVE	; size=4 opcode=0x182D flow=fallthrough
0x000560CE	BSR 0x0562D4	; size=4 opcode=0x6100 flow=call
0x000560D2	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x000560D6	Bcc[10] 0x0560DE	; size=4 opcode=0x6A00 flow=conditional
0x000560DA	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000560DE	JSR	; size=6 opcode=0x4EB9 flow=call
0x000560E4	ADDQ/SUBQ	; size=6 opcode=0x5339 flow=fallthrough
0x000560EA	Bcc[11] 0x056112	; size=4 opcode=0x6B00 flow=conditional
0x000560EE	MOVEQ	; size=2 opcode=0x700A flow=fallthrough
0x000560F0	arithmetic-op	; size=6 opcode=0x9039 flow=fallthrough
0x000560F6	MOVE	; size=6 opcode=0x13C0 flow=fallthrough
0x000560FC	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00056104	MOVEQ	; size=2 opcode=0x7003 flow=fallthrough
0x00056106	RTS	; size=2 opcode=0x4E75 flow=return
0x00056108	ADDQ/SUBQ	; size=6 opcode=0x5339 flow=fallthrough
0x0005610E	Bcc[6] 0x056112	; size=4 opcode=0x6600 flow=conditional
0x00056112	MOVEQ	; size=2 opcode=0x70FF flow=fallthrough
0x00056114	RTS	; size=2 opcode=0x4E75 flow=return
0x00056116	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0005611C	Bcc[11] 0x05613C	; size=4 opcode=0x6B00 flow=conditional
0x00056120	Bcc[14] 0x0561DC	; size=4 opcode=0x6E00 flow=conditional
0x00056124	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0005612C	Bcc[7] 0x0561E2	; size=4 opcode=0x6700 flow=conditional
0x00056130	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00056138	MOVEQ	; size=2 opcode=0x7002 flow=fallthrough
0x0005613A	RTS	; size=2 opcode=0x4E75 flow=return
0x0005613C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00056142	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056148	MOVE	; size=4 opcode=0x1828 flow=fallthrough
0x0005614C	BSR 0x0562DE	; size=4 opcode=0x6100 flow=call
0x00056150	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x00056154	Bcc[10] 0x05615C	; size=4 opcode=0x6A00 flow=conditional
0x00056158	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0005615C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00056162	MOVE	; size=6 opcode=0x1A39 flow=fallthrough
0x00056168	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005616C	BSR 0x055F44	; size=4 opcode=0x6100 flow=call
0x00056170	MOVE	; size=6 opcode=0x2079 flow=fallthrough
0x00056176	arithmetic-op	; size=4 opcode=0xDC28 flow=fallthrough
0x0005617A	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x0005617C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056182	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x00056186	MOVE	; size=4 opcode=0x1831 flow=fallthrough
0x0005618A	BSR 0x0562DE	; size=4 opcode=0x6100 flow=call
0x0005618E	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x00056192	Bcc[10] 0x05619A	; size=4 opcode=0x6A00 flow=conditional
0x00056196	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0005619A	JSR	; size=6 opcode=0x4EB9 flow=call
0x000561A0	arithmetic-op	; size=6 opcode=0x9A39 flow=fallthrough
0x000561A6	Bcc[15] 0x0561BC	; size=4 opcode=0x6F00 flow=conditional
0x000561AA	MOVEQ	; size=2 opcode=0x7201 flow=fallthrough
0x000561AC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000561B2	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000561B8	Bcc[11] 0x0561E2	; size=4 opcode=0x6B00 flow=conditional
0x000561BC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000561C2	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x000561C8	Bcc[15] 0x0561D6	; size=4 opcode=0x6F00 flow=conditional
0x000561CC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000561D2	BRA 0x0561DC	; size=4 opcode=0x6000 flow=branch
0x000561D6	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x000561DC	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x000561E2	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000561E6	RTS	; size=2 opcode=0x4E75 flow=return
0x000561E8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000561EE	LEA	; size=6 opcode=0x4BF9 flow=fallthrough
0x000561F4	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000561FA	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056200	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x00056204	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x00056206	MOVE	; size=4 opcode=0x1830 flow=fallthrough
0x0005620A	arithmetic-op	; size=4 opcode=0xB831 flow=fallthrough
0x0005620E	Bcc[7] 0x056216	; size=4 opcode=0x6700 flow=conditional
0x00056212	ADDQ/SUBQ	; size=4 opcode=0x5231 flow=fallthrough
0x00056216	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00056218	MOVE	; size=4 opcode=0x1831 flow=fallthrough
0x0005621C	immediate-op	; size=4 opcode=0x0C04 flow=fallthrough
0x00056220	Bcc[11] 0x056116	; size=4 opcode=0x6B00 flow=conditional
0x00056224	immediate-op	; size=4 opcode=0x0C04 flow=fallthrough
0x00056228	Bcc[11] 0x056268	; size=4 opcode=0x6B00 flow=conditional
0x0005622C	immediate-op	; size=4 opcode=0x0C04 flow=fallthrough
0x00056230	Bcc[11] 0x0560AA	; size=4 opcode=0x6B00 flow=conditional
0x00056234	immediate-op	; size=4 opcode=0x0C04 flow=fallthrough
0x00056238	Bcc[10] 0x05624A	; size=4 opcode=0x6A00 flow=conditional
0x0005623C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00056242	Bcc[14] 0x0560AA	; size=4 opcode=0x6E00 flow=conditional
0x00056246	BRA 0x056116	; size=4 opcode=0x6000 flow=branch
0x0005624A	immediate-op	; size=4 opcode=0x0C04 flow=fallthrough
0x0005624E	Bcc[10] 0x056260	; size=4 opcode=0x6A00 flow=conditional
0x00056252	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00056258	Bcc[14] 0x056268	; size=4 opcode=0x6E00 flow=conditional
0x0005625C	BRA 0x056116	; size=4 opcode=0x6000 flow=branch
0x00056260	immediate-op	; size=4 opcode=0x0C04 flow=fallthrough
0x00056264	Bcc[11] 0x056116	; size=4 opcode=0x6B00 flow=conditional
0x00056268	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005626E	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056274	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x00056278	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005627E	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x00056280	MOVE	; size=4 opcode=0x182D flow=fallthrough
0x00056284	BSR 0x0562D4	; size=4 opcode=0x6100 flow=call
0x00056288	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x0005628C	Bcc[10] 0x056294	; size=4 opcode=0x6A00 flow=conditional
0x00056290	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00056294	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005629A	JSR	; size=6 opcode=0x4EB9 flow=call
0x000562A0	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x000562A4	Bcc[6] 0x0562B6	; size=4 opcode=0x6600 flow=conditional
0x000562A8	JSR	; size=6 opcode=0x4EB9 flow=call
0x000562AE	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x000562B0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000562B6	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000562B8	MOVE	; size=10 opcode=0x13F9 flow=fallthrough
0x000562C2	Bcc[14] 0x0562D2	; size=4 opcode=0x6E00 flow=conditional
0x000562C6	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000562CA	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000562D2	RTS	; size=2 opcode=0x4E75 flow=return
0x000562D4	unary-op	; size=2 opcode=0x4287 flow=fallthrough
0x000562D6	MOVE	; size=6 opcode=0x1E39 flow=fallthrough
0x000562DC	arithmetic-op	; size=2 opcode=0xD847 flow=fallthrough
0x000562DE	unary-op	; size=2 opcode=0x4287 flow=fallthrough
0x000562E0	MOVE	; size=6 opcode=0x1E39 flow=fallthrough
0x000562E6	arithmetic-op	; size=2 opcode=0xE34F flow=fallthrough
0x000562E8	arithmetic-op	; size=2 opcode=0xD847 flow=fallthrough
0x000562EA	RTS	; size=2 opcode=0x4E75 flow=return
0x00056506	immediate-op	; size=6 opcode=0x0C30 flow=fallthrough
0x0005650C	Bcc[11] 0x056528	; size=4 opcode=0x6B00 flow=conditional
0x00056510	Bcc[7] 0x0565FA	; size=4 opcode=0x6700 flow=conditional
0x00056514	immediate-op	; size=6 opcode=0x0C30 flow=fallthrough
0x0005651A	Bcc[10] 0x056528	; size=4 opcode=0x6A00 flow=conditional
0x0005651E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00056524	MOVE	; size=2 opcode=0x3C07 flow=fallthrough
0x00056526	RTS	; size=2 opcode=0x4E75 flow=return
0x00056528	unary-op	; size=2 opcode=0x4281 flow=fallthrough
0x0005652A	BSR 0x055F44	; size=4 opcode=0x6100 flow=call
0x0005652E	arithmetic-op	; size=6 opcode=0xDC39 flow=fallthrough
0x00056534	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005653A	arithmetic-op	; size=2 opcode=0xBE46 flow=fallthrough
0x0005653C	Bcc[10] 0x056542	; size=4 opcode=0x6A00 flow=conditional
0x00056540	MOVE	; size=2 opcode=0x3C07 flow=fallthrough
0x00056542	arithmetic-op	; size=2 opcode=0xDC47 flow=fallthrough
0x00056544	Bcc[6] 0x05654C	; size=4 opcode=0x6600 flow=conditional
0x00056548	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005654C	unary-op	; size=2 opcode=0x4284 flow=fallthrough
0x0005654E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00056554	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005655A	MUL/DIV	; size=4 opcode=0xC2FC flow=fallthrough
0x0005655E	MOVE	; size=4 opcode=0x1831 flow=fallthrough
0x00056562	ADDQ/SUBQ	; size=2 opcode=0x5304 flow=fallthrough
0x00056564	arithmetic-op	; size=2 opcode=0x9C04 flow=fallthrough
0x00056566	Bcc[15] 0x0565E8	; size=4 opcode=0x6F00 flow=conditional
0x0005656A	MOVE	; size=4 opcode=0x1831 flow=fallthrough
0x0005656E	BSR 0x0562DE	; size=4 opcode=0x6100 flow=call
0x00056572	BSR 0x056632	; size=4 opcode=0x6100 flow=call
0x00056576	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005657C	immediate-op	; size=6 opcode=0x0C30 flow=fallthrough
0x00056582	Bcc[7] 0x0565A4	; size=4 opcode=0x6700 flow=conditional
0x00056586	MOVE	; size=2 opcode=0x2F06 flow=fallthrough
0x00056588	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005658E	MOVE	; size=2 opcode=0x2A06 flow=fallthrough
0x00056590	MOVE	; size=2 opcode=0x2C1F flow=fallthrough
0x00056592	immediate-op	; size=6 opcode=0x0C30 flow=fallthrough
0x00056598	Bcc[11] 0x0565B2	; size=4 opcode=0x6B00 flow=conditional
0x0005659C	MOVE	; size=4 opcode=0x182A flow=fallthrough
0x000565A0	BRA 0x0565B6	; size=4 opcode=0x6000 flow=branch
0x000565A4	unary-op	; size=2 opcode=0x4285 flow=fallthrough
0x000565A6	MOVE	; size=4 opcode=0x1A31 flow=fallthrough
0x000565AA	MOVE	; size=4 opcode=0x182A flow=fallthrough
0x000565AE	BRA 0x0565B6	; size=4 opcode=0x6000 flow=branch
0x000565B2	MOVE	; size=4 opcode=0x182A flow=fallthrough
0x000565B6	BSR 0x0562DE	; size=4 opcode=0x6100 flow=call
0x000565BA	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x000565BE	Bcc[10] 0x0565C4	; size=4 opcode=0x6A00 flow=conditional
0x000565C2	MOVEQ	; size=2 opcode=0x7802 flow=fallthrough
0x000565C4	MOVEQ	; size=2 opcode=0x7007 flow=fallthrough
0x000565C6	arithmetic-op	; size=2 opcode=0x9044 flow=fallthrough
0x000565C8	Bcc[14] 0x0565D2	; size=4 opcode=0x6E00 flow=conditional
0x000565CC	MOVEQ	; size=2 opcode=0x700A flow=fallthrough
0x000565CE	BRA 0x0565D6	; size=4 opcode=0x6000 flow=branch
0x000565D2	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x000565D6	MUL/DIV	; size=2 opcode=0xCAC0 flow=fallthrough
0x000565D8	MUL/DIV	; size=4 opcode=0x8AFC flow=fallthrough
0x000565DC	arithmetic-op	; size=2 opcode=0x9C45 flow=fallthrough
0x000565DE	Bcc[10] 0x0565EC	; size=4 opcode=0x6A00 flow=conditional
0x000565E2	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000565E6	RTS	; size=2 opcode=0x4E75 flow=return
0x000565E8	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x000565EA	RTS	; size=2 opcode=0x4E75 flow=return
0x000565EC	immediate-op	; size=4 opcode=0x0C46 flow=fallthrough
0x000565F0	Bcc[11] 0x0565F8	; size=4 opcode=0x6B00 flow=conditional
0x000565F4	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000565F8	RTS	; size=2 opcode=0x4E75 flow=return
0x000565FA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00056600	MOVE	; size=2 opcode=0x3C07 flow=fallthrough
0x00056602	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00056608	Bcc[13] 0x056612	; size=4 opcode=0x6D00 flow=conditional
0x0005660C	MOVEQ	; size=2 opcode=0x7804 flow=fallthrough
0x0005660E	BRA 0x056624	; size=4 opcode=0x6000 flow=branch
0x00056612	immediate-op	; size=6 opcode=0x0C28 flow=fallthrough
0x00056618	Bcc[13] 0x056622	; size=4 opcode=0x6D00 flow=conditional
0x0005661C	MOVEQ	; size=2 opcode=0x7805 flow=fallthrough
0x0005661E	BRA 0x056624	; size=4 opcode=0x6000 flow=branch
0x00056622	MOVEQ	; size=2 opcode=0x7806 flow=fallthrough
0x00056624	BSR 0x056632	; size=4 opcode=0x6100 flow=call
0x00056628	unary-op	; size=2 opcode=0x4A46 flow=fallthrough
0x0005662A	Bcc[14] 0x0565EC	; size=2 opcode=0x6EC0 flow=conditional
0x0005662C	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x00056630	BRA 0x0565EC	; size=2 opcode=0x60BA flow=branch
0x00056632	immediate-op	; size=4 opcode=0x0C44 flow=fallthrough
0x00056636	Bcc[10] 0x05663E	; size=4 opcode=0x6A00 flow=conditional
0x0005663A	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0005663E	MOVEQ	; size=2 opcode=0x7007 flow=fallthrough
0x00056640	arithmetic-op	; size=2 opcode=0x9044 flow=fallthrough
0x00056642	Bcc[14] 0x05664C	; size=4 opcode=0x6E00 flow=conditional
0x00056646	MOVEQ	; size=2 opcode=0x700A flow=fallthrough
0x00056648	BRA 0x056650	; size=4 opcode=0x6000 flow=branch
0x0005664C	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00056650	MUL/DIV	; size=2 opcode=0xCCC0 flow=fallthrough
0x00056652	MUL/DIV	; size=4 opcode=0x8CFC flow=fallthrough
0x00056656	RTS	; size=2 opcode=0x4E75 flow=return
0x000566A2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000566A8	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000566AE	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x000566B2	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000566B6	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x000566B8	DBcc 0x0566B6	; size=4 opcode=0x51CF flow=conditional
0x000566BC	DBcc 0x0566B2	; size=4 opcode=0x51CE flow=conditional
0x000566C0	RTS	; size=2 opcode=0x4E75 flow=return
0x000566C2	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000566C8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000566CE	MOVEQ	; size=2 opcode=0x7017 flow=fallthrough
0x000566D0	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x000566D2	DBcc 0x0566D0	; size=4 opcode=0x51C8 flow=conditional
0x000566D6	RTS	; size=2 opcode=0x4E75 flow=return
0x000566D8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000566DE	BRA 0x0566FC	; size=4 opcode=0x6000 flow=branch
0x000566E2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000566E8	BRA 0x0566FC	; size=4 opcode=0x6000 flow=branch
0x000566F6	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000566FC	MOVEQ	; size=2 opcode=0x7007 flow=fallthrough
0x000566FE	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x00056700	DBcc 0x0566FE	; size=4 opcode=0x51C8 flow=conditional
0x00056704		; size=2 opcode=0x08F9 flow=fallthrough
0x0005670E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056714	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005671A	MOVEQ	; size=2 opcode=0x7017 flow=fallthrough
0x0005671C	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x0005671E	DBcc 0x05671C	; size=4 opcode=0x51C8 flow=conditional
0x00056722		; size=2 opcode=0x08F9 flow=fallthrough
0x0005672C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00056732	Bcc[7] 0x056738	; size=4 opcode=0x6700 flow=conditional
0x00056736	RTS	; size=2 opcode=0x4E75 flow=return
0x00056738	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056740	Bcc[7] 0x05696A	; size=4 opcode=0x6700 flow=conditional
0x00056744	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005674C	Bcc[7] 0x05689A	; size=4 opcode=0x6700 flow=conditional
0x00056750	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056758	Bcc[7] 0x05696C	; size=4 opcode=0x6700 flow=conditional
0x0005675C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00056762	Bcc[6] 0x05696A	; size=4 opcode=0x6600 flow=conditional
0x00056766	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005676E	Bcc[7] 0x056908	; size=4 opcode=0x6700 flow=conditional
0x00056772	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005677A	Bcc[7] 0x056908	; size=4 opcode=0x6700 flow=conditional
0x0005677E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056786	Bcc[7] 0x056908	; size=4 opcode=0x6700 flow=conditional
0x0005678A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056792	Bcc[7] 0x056908	; size=4 opcode=0x6700 flow=conditional
0x00056796	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005679E	Bcc[7] 0x0569F8	; size=4 opcode=0x6700 flow=conditional
0x000567A2	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567AA	Bcc[7] 0x056A5C	; size=4 opcode=0x6700 flow=conditional
0x000567AE	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567B6	Bcc[7] 0x0567D6	; size=4 opcode=0x6700 flow=conditional
0x000567BA	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567C2	Bcc[12] 0x05696A	; size=4 opcode=0x6C00 flow=conditional
0x000567C6	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567CE	Bcc[12] 0x05682C	; size=4 opcode=0x6C00 flow=conditional
0x000567D2	BRA 0x05696A	; size=4 opcode=0x6000 flow=branch
0x000567D6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000567DC	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000567E2	Bcc[10] 0x05696A	; size=4 opcode=0x6A00 flow=conditional
0x000567E6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000567EE	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000567F4	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000567FC	Bcc[5] 0x056806	; size=4 opcode=0x6500 flow=conditional
0x00056800	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00056806	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005680C	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x0005680E	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00056812	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00056818	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056820	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056828	BRA 0x05696A	; size=4 opcode=0x6000 flow=branch
0x0005682C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056832	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056838	Bcc[10] 0x05696A	; size=4 opcode=0x6A00 flow=conditional
0x0005683C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056844	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005684A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056852	Bcc[5] 0x05685C	; size=4 opcode=0x6500 flow=conditional
0x00056856	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005685C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056862	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x00056864	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00056868	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005686E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056876	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005687E	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056886	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005688E	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056896	BRA 0x05696A	; size=4 opcode=0x6000 flow=branch
0x0005689A	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000568A0	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x000568A6	Bcc[10] 0x05696A	; size=4 opcode=0x6A00 flow=conditional
0x000568AA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000568B2	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000568B8	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x000568C0	Bcc[5] 0x0568CA	; size=4 opcode=0x6500 flow=conditional
0x000568C4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000568CA	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000568D0	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x000568D2	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000568D6	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x000568DC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000568E4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000568EC	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000568F4	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x000568FC	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056904	BRA 0x05696A	; size=4 opcode=0x6000 flow=branch
0x00056908	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005690C	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056912	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056918	Bcc[10] 0x05696A	; size=4 opcode=0x6A00 flow=conditional
0x0005691C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056924	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005692A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056932	Bcc[5] 0x05693C	; size=4 opcode=0x6500 flow=conditional
0x00056936	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005693C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056942	arithmetic-op	; size=2 opcode=0xE749 flow=fallthrough
0x00056944	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005694A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056952	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005695A	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056962	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x0005696A	RTS	; size=2 opcode=0x4E75 flow=return
0x0005696C	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x00056972	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x00056976	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005697A	Bcc[6] 0x05696A	; size=2 opcode=0x66EE flow=conditional
0x0005697C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00056982	Bcc[10] 0x0569B0	; size=4 opcode=0x6A00 flow=conditional
0x00056986	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005698C	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x00056994	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005699C	Bcc[6] 0x0569D2	; size=4 opcode=0x6600 flow=conditional
0x000569A0	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000569A6	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000569AC	BRA 0x0569D2	; size=4 opcode=0x6000 flow=branch
0x000569B0	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x000569B6	immediate-op	; size=8 opcode=0x0279 flow=fallthrough
0x000569BE	Bcc[6] 0x0569D2	; size=4 opcode=0x6600 flow=conditional
0x000569C2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000569CA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000569D2	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x000569D8	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x000569DA	arithmetic-op	; size=2 opcode=0xE149 flow=fallthrough
0x000569DC	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000569E0	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x000569E6	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x000569EE	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x000569F4	BRA 0x05696A	; size=4 opcode=0x6000 flow=branch
0x000569F8	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000569FC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056A02	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056A08	Bcc[10] 0x05696A	; size=4 opcode=0x6A00 flow=conditional
0x00056A0C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056A14	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00056A1A	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056A22	Bcc[5] 0x056A2C	; size=4 opcode=0x6500 flow=conditional
0x00056A26	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00056A2C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056A32	arithmetic-op	; size=2 opcode=0xE749 flow=fallthrough
0x00056A34	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00056A3A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056A42	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056A4A	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056A52	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056A5A	RTS	; size=2 opcode=0x4E75 flow=return
0x00056A5C	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00056A60	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00056A66	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x00056A6C	Bcc[10] 0x05696A	; size=4 opcode=0x6A00 flow=conditional
0x00056A70	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056A78	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x00056A7E	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x00056A86	Bcc[5] 0x056A90	; size=4 opcode=0x6500 flow=conditional
0x00056A8A	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00056A90	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x00056A96	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x00056A98	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x00056A9E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00056AA6	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056AAE	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056AB6	MOVE	; size=8 opcode=0x33F0 flow=fallthrough
0x00056ABE	RTS	; size=2 opcode=0x4E75 flow=return
0x00059092	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00059098	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005909E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000590A4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590AA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590B0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590B6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590BC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000590C2	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x000590C8	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x000590CC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590D2	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x000590D8	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x000590DA	MOVEQ	; size=2 opcode=0x7601 flow=fallthrough
0x000590DC	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000590E0	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000590E4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590EA	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590F6	JSR	; size=6 opcode=0x4EB9 flow=call
0x000590FC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00059102	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00059104	Bcc[7] 0x05910E	; size=4 opcode=0x6700 flow=conditional
0x00059108	MOVEQ	; size=2 opcode=0x74FF flow=fallthrough
0x0005910A	BRA 0x05911C	; size=4 opcode=0x6000 flow=branch
0x0005910E	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00059114	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00059116	Bcc[7] 0x05912C	; size=4 opcode=0x6700 flow=conditional
0x0005911A	MOVEQ	; size=2 opcode=0x7401 flow=fallthrough
0x0005911C	MOVEQ	; size=2 opcode=0x7601 flow=fallthrough
0x0005911E	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00059122	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x00059126	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005912C	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00059132	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00059134	Bcc[7] 0x059154	; size=4 opcode=0x6700 flow=conditional
0x00059138	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0005913E	Bcc[7] 0x059248	; size=4 opcode=0x6700 flow=conditional
0x00059142	BSR 0x059172	; size=4 opcode=0x6100 flow=call
0x00059146	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0005914C	Bcc[10] 0x059170	; size=4 opcode=0x6A00 flow=conditional
0x00059150	BRA 0x0590B6	; size=4 opcode=0x6000 flow=branch
0x00059154	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0005915A	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0005915C	Bcc[7] 0x059162	; size=4 opcode=0x6700 flow=conditional
0x00059160	BRA 0x059138	; size=2 opcode=0x60D6 flow=branch
0x00059162	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00059168	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0005916A	Bcc[7] 0x0590EA	; size=4 opcode=0x6700 flow=conditional
0x0005916E	BRA 0x059138	; size=2 opcode=0x60C8 flow=branch
0x00059170	RTS	; size=2 opcode=0x4E75 flow=return
0x00059172	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00059178	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0005917E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x00059182	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059188	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0005918E	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x00059190	BRA 0x0591C6	; size=4 opcode=0x6000 flow=branch
0x00059194	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005919A	JSR	; size=6 opcode=0x4EB9 flow=call
0x000591A0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000591A6	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000591AC	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000591AE	Bcc[7] 0x0591B8	; size=4 opcode=0x6700 flow=conditional
0x000591B2	MOVEQ	; size=2 opcode=0x74FF flow=fallthrough
0x000591B4	BRA 0x0591C6	; size=4 opcode=0x6000 flow=branch
0x000591B8	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000591BE	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000591C0	Bcc[7] 0x0591D8	; size=4 opcode=0x6700 flow=conditional
0x000591C4	MOVEQ	; size=2 opcode=0x7401 flow=fallthrough
0x000591C6	MOVEQ	; size=2 opcode=0x7601 flow=fallthrough
0x000591C8	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000591CC	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000591D0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000591D6	BRA 0x059194	; size=2 opcode=0x60BC flow=branch
0x000591D8	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000591DE	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000591E0	Bcc[7] 0x0591E8	; size=4 opcode=0x6700 flow=conditional
0x000591E4	BRA 0x059202	; size=4 opcode=0x6000 flow=branch
0x000591E8	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000591EE	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000591F0	Bcc[7] 0x0591F8	; size=4 opcode=0x6700 flow=conditional
0x000591F4	BRA 0x059202	; size=4 opcode=0x6000 flow=branch
0x000591F8	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000591FE	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00059200	Bcc[7] 0x059194	; size=2 opcode=0x6792 flow=conditional
0x00059202	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059208	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005920E	Bcc[5] 0x059224	; size=4 opcode=0x6500 flow=conditional
0x00059212	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005921A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00059220	BRA 0x059232	; size=4 opcode=0x6000 flow=branch
0x00059224	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0005922C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00059232	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x00059238	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005923C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059242	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00059248	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005924E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00059254	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0005925A	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005925E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059264	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x0005926A	unary-op	; size=2 opcode=0x4242 flow=fallthrough
0x0005926C	BRA 0x0592A2	; size=4 opcode=0x6000 flow=branch
0x00059270	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059276	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005927C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059282	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00059288	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0005928A	Bcc[7] 0x059294	; size=4 opcode=0x6700 flow=conditional
0x0005928E	MOVEQ	; size=2 opcode=0x74FF flow=fallthrough
0x00059290	BRA 0x0592A2	; size=4 opcode=0x6000 flow=branch
0x00059294	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x0005929A	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x0005929C	Bcc[7] 0x0592DC	; size=4 opcode=0x6700 flow=conditional
0x000592A0	MOVEQ	; size=2 opcode=0x7401 flow=fallthrough
0x000592A2	MOVEQ	; size=2 opcode=0x7602 flow=fallthrough
0x000592A4	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000592A8	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x000592AC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000592B2	MOVE	; size=10 opcode=0x13F9 flow=fallthrough
0x000592BC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000592C2	JSR	; size=6 opcode=0x4EB9 flow=call
0x000592C8	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000592CE	MOVE	; size=6 opcode=0x283C flow=fallthrough
0x000592D4	JSR	; size=6 opcode=0x4EB9 flow=call
0x000592DA	BRA 0x059270	; size=2 opcode=0x6094 flow=branch
0x000592DC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000592E2	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000592E4	Bcc[7] 0x0592EC	; size=4 opcode=0x6700 flow=conditional
0x000592E8	BRA 0x059308	; size=4 opcode=0x6000 flow=branch
0x000592EC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x000592F2	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x000592F4	Bcc[7] 0x0592FC	; size=4 opcode=0x6700 flow=conditional
0x000592F8	BRA 0x059308	; size=4 opcode=0x6000 flow=branch
0x000592FC	arithmetic-op	; size=6 opcode=0x0839 flow=fallthrough
0x00059302	arithmetic-op	; size=2 opcode=0xDD22 flow=fallthrough
0x00059304	Bcc[7] 0x059270	; size=4 opcode=0x6700 flow=conditional
0x00059308	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0005930A	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x00059310	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059316	RTS	; size=2 opcode=0x4E75 flow=return
0x00059318	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005931E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00059324	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00059328	MOVE	; size=2 opcode=0x12D8 flow=fallthrough
0x0005932A	DBcc 0x059328	; size=4 opcode=0x51CF flow=conditional
0x0005932E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x00059334	BSR 0x0597EE	; size=4 opcode=0x6100 flow=call
0x00059338	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005933E	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00059342	BSR 0x0597EE	; size=4 opcode=0x6100 flow=call
0x00059346	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x0005934A	DBcc 0x059342	; size=4 opcode=0x51CF flow=conditional
0x0005934E	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00059356	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0005935E	RTS	; size=2 opcode=0x4E75 flow=return
0x00059360	BSR 0x059456	; size=4 opcode=0x6100 flow=call
0x00059364	Bcc[5] 0x059370	; size=4 opcode=0x6500 flow=conditional
0x00059368	BSR 0x059372	; size=4 opcode=0x6100 flow=call
0x0005936C	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x00059370	RTS	; size=2 opcode=0x4E75 flow=return
0x00059372	MOVE	; size=6 opcode=0x2A3C flow=fallthrough
0x00059378	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x0005937E	Bcc[7] 0x059388	; size=4 opcode=0x6700 flow=conditional
0x00059382	MOVE	; size=6 opcode=0x2A3C flow=fallthrough
0x00059388	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005938E	MOVE	; size=2 opcode=0x3005 flow=fallthrough
0x00059390	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00059394	BSR 0x0593FC	; size=4 opcode=0x6100 flow=call
0x00059398	Bcc[4] 0x0593CC	; size=4 opcode=0x6400 flow=conditional
0x0005939C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000593A2	MOVE	; size=2 opcode=0x2005 flow=fallthrough
0x000593A4	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x000593A6	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000593AA	BSR 0x0593FC	; size=4 opcode=0x6100 flow=call
0x000593AE	Bcc[5] 0x0593C6	; size=4 opcode=0x6500 flow=conditional
0x000593B2	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x000593B8	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000593BC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000593C2	BRA 0x0593F6	; size=4 opcode=0x6000 flow=branch
0x000593C6	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x000593CC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000593D2	MOVE	; size=2 opcode=0x2005 flow=fallthrough
0x000593D4	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x000593D6	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000593DA	BSR 0x05961C	; size=4 opcode=0x6100 flow=call
0x000593DE	Bcc[4] 0x0593F6	; size=4 opcode=0x6400 flow=conditional
0x000593E2	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x000593E8	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000593EC	JSR	; size=6 opcode=0x4EB9 flow=call
0x000593F2	BRA 0x0593F6	; size=4 opcode=0x6000 flow=branch
0x000593F6	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x000593FC	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059402	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x00059408	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x0005940A	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x0005940E	Bcc[6] 0x059442	; size=4 opcode=0x6600 flow=conditional
0x00059412	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x00059414	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00059418	Bcc[6] 0x059442	; size=4 opcode=0x6600 flow=conditional
0x0005941C	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x0005941E	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00059422	Bcc[6] 0x059442	; size=4 opcode=0x6600 flow=conditional
0x00059426	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x00059428	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x0005942C	Bcc[6] 0x059442	; size=4 opcode=0x6600 flow=conditional
0x00059430	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x00059436	BSR 0x059786	; size=4 opcode=0x6100 flow=call
0x0005943A	MOVE	; size=2 opcode=0x2011 flow=fallthrough
0x0005943C	arithmetic-op	; size=2 opcode=0xB086 flow=fallthrough
0x0005943E	Bcc[7] 0x059450	; size=4 opcode=0x6700 flow=conditional
0x00059442	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00059444	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00059446	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x00059448	NOP	; size=2 opcode=0x4E71 flow=fallthrough
0x0005944A	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x00059450	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x00059456	MOVE	; size=6 opcode=0x2A3C flow=fallthrough
0x0005945C	unary-op	; size=6 opcode=0x4A39 flow=fallthrough
0x00059462	Bcc[7] 0x05946C	; size=4 opcode=0x6700 flow=conditional
0x00059466	MOVE	; size=6 opcode=0x2A3C flow=fallthrough
0x0005946C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00059472	MOVE	; size=2 opcode=0x3005 flow=fallthrough
0x00059474	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x00059478	BSR 0x05961C	; size=4 opcode=0x6100 flow=call
0x0005947C	Bcc[4] 0x0594B0	; size=4 opcode=0x6400 flow=conditional
0x00059480	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x00059486	MOVE	; size=2 opcode=0x2005 flow=fallthrough
0x00059488	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x0005948A	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005948E	BSR 0x05961C	; size=4 opcode=0x6100 flow=call
0x00059492	Bcc[5] 0x0594AA	; size=4 opcode=0x6500 flow=conditional
0x00059496	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x0005949C	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000594A0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000594A6	BRA 0x0594DA	; size=4 opcode=0x6000 flow=branch
0x000594AA	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x000594B0	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000594B6	MOVE	; size=2 opcode=0x2005 flow=fallthrough
0x000594B8	SWAP	; size=2 opcode=0x4840 flow=fallthrough
0x000594BA	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000594BE	BSR 0x05961C	; size=4 opcode=0x6100 flow=call
0x000594C2	Bcc[4] 0x0594DA	; size=4 opcode=0x6400 flow=conditional
0x000594C6	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x000594CC	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000594D0	JSR	; size=6 opcode=0x4EB9 flow=call
0x000594D6	BRA 0x0594DA	; size=4 opcode=0x6000 flow=branch
0x000594DA	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x000594E0	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x000594E6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000594EC	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x000594F2	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x000594F6	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000594F8	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x000594FC	MOVE	; size=2 opcode=0x10D9 flow=fallthrough
0x000594FE	DBcc 0x0594FC	; size=4 opcode=0x51CF flow=conditional
0x00059502	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00059508	MOVE	; size=2 opcode=0x2F09 flow=fallthrough
0x0005950A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059510	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00059518	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0005951E	Bcc[11] 0x05952E	; size=4 opcode=0x6B00 flow=conditional
0x00059522	unary-op	; size=2 opcode=0x4247 flow=fallthrough
0x00059524	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x00059526	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x00059528	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005952E	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x00059534	Bcc[11] 0x059544	; size=4 opcode=0x6B00 flow=conditional
0x00059538	unary-op	; size=2 opcode=0x4247 flow=fallthrough
0x0005953A	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x0005953C	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x0005953E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059544	MOVE	; size=2 opcode=0x225F flow=fallthrough
0x00059546	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x0005954C	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00059552	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00059558	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x0005955E	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00059564	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x0005956A	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00059570	MOVE	; size=6 opcode=0x33D9 flow=fallthrough
0x00059576	BSR 0x05966E	; size=4 opcode=0x6100 flow=call
0x0005957A	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x00059580	Bcc[11] 0x059590	; size=4 opcode=0x6B00 flow=conditional
0x00059584	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0005958C	BRA 0x059596	; size=4 opcode=0x6000 flow=branch
0x00059590	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x00059596	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0005959C	Bcc[11] 0x0595AC	; size=4 opcode=0x6B00 flow=conditional
0x000595A0	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x000595A8	BRA 0x0595B2	; size=4 opcode=0x6000 flow=branch
0x000595AC	unary-op	; size=6 opcode=0x4239 flow=fallthrough
0x000595B2		; size=2 opcode=0x08F9 flow=fallthrough
0x0005961C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059622	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x00059628	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x0005962A	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x0005962E	Bcc[6] 0x059662	; size=4 opcode=0x6600 flow=conditional
0x00059632	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x00059634	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00059638	Bcc[6] 0x059662	; size=4 opcode=0x6600 flow=conditional
0x0005963C	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x0005963E	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x00059642	Bcc[6] 0x059662	; size=4 opcode=0x6600 flow=conditional
0x00059646	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x00059648	immediate-op	; size=4 opcode=0x0C00 flow=fallthrough
0x0005964C	Bcc[6] 0x059662	; size=4 opcode=0x6600 flow=conditional
0x00059650	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x00059656	BSR 0x059786	; size=4 opcode=0x6100 flow=call
0x0005965A	MOVE	; size=2 opcode=0x2011 flow=fallthrough
0x0005965C	arithmetic-op	; size=2 opcode=0xB086 flow=fallthrough
0x0005965E	Bcc[7] 0x059668	; size=4 opcode=0x6700 flow=conditional
0x00059662	immediate-op	; size=6 opcode=0x003C flow=fallthrough
0x00059668	immediate-op	; size=6 opcode=0x023C flow=fallthrough
0x0005966E	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x00059674	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005967A	BSR 0x0598B0	; size=4 opcode=0x6100 flow=call
0x0005967E	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x00059680	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x00059686	Bcc[11] 0x0596A0	; size=4 opcode=0x6B00 flow=conditional
0x0005968A	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x00059690	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x00059694	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x00059696	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x0005969C	BSR 0x0598B0	; size=4 opcode=0x6100 flow=call
0x000596A0	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000596A2	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x000596A8	Bcc[11] 0x0596C2	; size=4 opcode=0x6B00 flow=conditional
0x000596AC	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x000596B2	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x000596B6	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x000596B8	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x000596BE	BSR 0x0598B0	; size=4 opcode=0x6100 flow=call
0x000596C2	RTS	; size=2 opcode=0x4E75 flow=return
0x00059786	unary-op	; size=2 opcode=0x4286 flow=fallthrough
0x00059788	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005978C	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x0005978E	MOVE	; size=2 opcode=0x1019 flow=fallthrough
0x00059790	arithmetic-op	; size=2 opcode=0xDC80 flow=fallthrough
0x00059792	DBcc 0x05978C	; size=4 opcode=0x51CF flow=conditional
0x00059796	RTS	; size=2 opcode=0x4E75 flow=return
0x00059798	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x0005979E	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x000597A4	BSR 0x0597EE	; size=4 opcode=0x6100 flow=call
0x000597A8	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000597AA	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x000597B0	Bcc[11] 0x0597CA	; size=4 opcode=0x6B00 flow=conditional
0x000597B4	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x000597BA	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x000597BE	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000597C0	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x000597C6	BSR 0x0597EE	; size=4 opcode=0x6100 flow=call
0x000597CA	unary-op	; size=2 opcode=0x4280 flow=fallthrough
0x000597CC	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x000597D2	Bcc[11] 0x0597EC	; size=4 opcode=0x6B00 flow=conditional
0x000597D6	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x000597DC	MUL/DIV	; size=4 opcode=0xC0FC flow=fallthrough
0x000597E0	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x000597E2	MOVE	; size=6 opcode=0x207C flow=fallthrough
0x000597E8	BSR 0x0597EE	; size=4 opcode=0x6100 flow=call
0x000597EC	RTS	; size=2 opcode=0x4E75 flow=return
0x000597EE	MOVE	; size=4 opcode=0x32E8 flow=fallthrough
0x000597F2	MOVE	; size=4 opcode=0x32E8 flow=fallthrough
0x000597F6	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x000597FA	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x000597FE	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059802	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059806	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005980A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005980E	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x00059812	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x00059816	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x0005981A	MOVE	; size=4 opcode=0x32E8 flow=fallthrough
0x0005981E	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x00059822	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x00059826	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x0005982A	MOVE	; size=4 opcode=0x22E8 flow=fallthrough
0x0005982E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059832	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059836	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005983A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005983E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059842	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059846	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005984A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005984E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059852	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059856	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005985A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005985E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059862	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059866	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005986A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005986E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059872	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059876	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005987A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005987E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059882	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059886	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005988A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005988E	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059892	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x00059896	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005989A	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x0005989E	MOVE	; size=4 opcode=0x32E8 flow=fallthrough
0x000598A2	MOVE	; size=4 opcode=0x32E8 flow=fallthrough
0x000598A6	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x000598AA	MOVE	; size=4 opcode=0x12E8 flow=fallthrough
0x000598AE	RTS	; size=2 opcode=0x4E75 flow=return
0x000598B0	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x000598B4	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x000598B8	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598BC	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598C0	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598C4	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598C8	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598CC	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598D0	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598D4	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598D8	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598DC	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x000598E0	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598E4	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598E8	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598EC	MOVE	; size=4 opcode=0x2358 flow=fallthrough
0x000598F0	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598F4	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598F8	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x000598FC	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059900	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059904	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059908	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005990C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059910	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059914	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059918	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005991C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059920	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059924	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059928	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005992C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059930	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059934	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059938	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005993C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059940	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059944	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059948	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005994C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059950	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059954	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059958	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005995C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059960	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x00059964	MOVE	; size=4 opcode=0x3358 flow=fallthrough
0x00059968	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x0005996C	MOVE	; size=4 opcode=0x1358 flow=fallthrough
0x00059970	RTS	; size=2 opcode=0x4E75 flow=return
0x00059C5E	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00059C66	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059C6C	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00059C72	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00059C76	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059C7C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059C82	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00059C88	Bcc[6] 0x059CFE	; size=4 opcode=0x6600 flow=conditional
0x00059C8C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059C92	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00059C96	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059C9C	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00059CA2	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CA8	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00059CAE	Bcc[6] 0x059CFE	; size=4 opcode=0x6600 flow=conditional
0x00059CB2	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CB8	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x00059CBC	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CC2	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00059CC8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CCE	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x00059CD4	Bcc[6] 0x059CFE	; size=4 opcode=0x6600 flow=conditional
0x00059CD8	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00059CDE	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CE4	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x00059CEA	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CF0	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00059CF8	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059CFE	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D04	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D0A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D10	RTS	; size=2 opcode=0x4E75 flow=return
0x00059D12	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x00059D18	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D1E	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D24	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D2A	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D30	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x00059D38	JMP	; size=6 opcode=0x4EF9 flow=branch
0x00059D7C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059D82	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x00059D8C	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x00059D96	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x00059D9C	JSR	; size=6 opcode=0x4EB9 flow=call
0x00059DA2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00059DAA	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x00059DB2		; size=2 opcode=0x08F9 flow=fallthrough
0x0005A9E2	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005A9E8	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0005A9EC	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005A9F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005A9F6	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005A9FC	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0005AA00	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AA04	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AA0A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AA10	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0005AA14	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AA18	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005AA5A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AA60	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0005AA64	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AA68	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AA6E	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AA74	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0005AA78	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AA7C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AA82	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AA88	MOVE	; size=4 opcode=0x347C flow=fallthrough
0x0005AA8C	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AA90	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005AA96	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AA9C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AAA2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AAA8	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AAAE	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0005AAB4	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x0005AABA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AAC0	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AAC6	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0005AACC	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x0005AAD2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AAD8	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AADE	BSR 0x05AA5A	; size=4 opcode=0x6100 flow=call
0x0005AAE2	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AAE8	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005AAEE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AAF4	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0005AAF8	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005AAFC	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AB00	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB06	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x0005AB10	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB16	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB1C	BSR 0x05ABD2	; size=4 opcode=0x6100 flow=call
0x0005AB20	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB26	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AB2C	MOVE	; size=4 opcode=0x383C flow=fallthrough
0x0005AB30	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005AB34	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AB38	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0005AB3C	unary-op	; size=2 opcode=0x4243 flow=fallthrough
0x0005AB3E	MOVE	; size=2 opcode=0x3A07 flow=fallthrough
0x0005AB40	ADDQ/SUBQ	; size=2 opcode=0x5245 flow=fallthrough
0x0005AB42	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB48	MOVE	; size=6 opcode=0x3F39 flow=fallthrough
0x0005AB4E	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB54	MOVE	; size=6 opcode=0x33DF flow=fallthrough
0x0005AB5A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB60	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005AB64	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x0005AB66	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB6C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB72	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB78	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB7E	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AB84	MOVE	; size=2 opcode=0x3E1F flow=fallthrough
0x0005AB86	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0005AB8C	Bcc[6] 0x05AB94	; size=4 opcode=0x6600 flow=conditional
0x0005AB90	DBcc 0x05AB64	; size=4 opcode=0x51CF flow=conditional
0x0005AB94	RTS	; size=2 opcode=0x4E75 flow=return
0x0005AB96	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005AB9C	arithmetic-op	; size=6 opcode=0x9079 flow=fallthrough
0x0005ABA2	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005ABA6	Bcc[5] 0x05ABD0	; size=4 opcode=0x6500 flow=conditional
0x0005ABAA	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0005ABAE	Bcc[6] 0x05ABD0	; size=4 opcode=0x6600 flow=conditional
0x0005ABB2	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x0005ABB4	BSR 0x05B2E2	; size=4 opcode=0x6100 flow=call
0x0005ABB8	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005ABBC	BSR 0x05B2E2	; size=4 opcode=0x6100 flow=call
0x0005ABC0	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005ABC4	BSR 0x05B2E2	; size=4 opcode=0x6100 flow=call
0x0005ABC8	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005ABCC	BSR 0x05B2E2	; size=4 opcode=0x6100 flow=call
0x0005ABD0	RTS	; size=2 opcode=0x4E75 flow=return
0x0005ABD2	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005ABDA	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0005ABE0	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0005ABE4	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005ABEA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005ABF0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005ABF6	ADDQ/SUBQ	; size=2 opcode=0x514B flow=fallthrough
0x0005ABF8	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005AC00	unary-op	; size=4 opcode=0x422B flow=fallthrough
0x0005AC04		; size=2 opcode=0x08F9 flow=fallthrough
0x0005AC0E	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005AC14	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005AC18	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0005AC1C	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005AC20	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AC26	RTS	; size=2 opcode=0x4E75 flow=return
0x0005AC28	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AC2E	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AC34	RTS	; size=2 opcode=0x4E75 flow=return
0x0005AC36	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005AC3C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005AC42	arithmetic-op	; size=2 opcode=0xE049 flow=fallthrough
0x0005AC44	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x0005AC48	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0005AC4E	arithmetic-op	; size=2 opcode=0xE04A flow=fallthrough
0x0005AC50	immediate-op	; size=4 opcode=0x0642 flow=fallthrough
0x0005AC54	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005AC58	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AC5E	immediate-op	; size=8 opcode=0x0679 flow=fallthrough
0x0005AC66	immediate-op	; size=8 opcode=0x0679 flow=fallthrough
0x0005AC6E	RTS	; size=2 opcode=0x4E75 flow=return
0x0005AC70	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AC76	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AC7C	RTS	; size=2 opcode=0x4E75 flow=return
0x0005AC7E	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005AC84	arithmetic-op	; size=6 opcode=0x9079 flow=fallthrough
0x0005AC8A	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005AC8E	Bcc[5] 0x05AD00	; size=4 opcode=0x6500 flow=conditional
0x0005AC92	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005AC98	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x0005AC9E	Bcc[5] 0x05ACE4	; size=4 opcode=0x6500 flow=conditional
0x0005ACA2	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005ACAA	Bcc[4] 0x05ACE4	; size=4 opcode=0x6400 flow=conditional
0x0005ACAE	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005ACB4	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005ACBA	arithmetic-op	; size=2 opcode=0xE748 flow=fallthrough
0x0005ACBC	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x0005ACBE	MOVE	; size=6 opcode=0x23D8 flow=fallthrough
0x0005ACC4	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x0005ACCA	MOVE	; size=6 opcode=0x33D0 flow=fallthrough
0x0005ACD0	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005ACD6	ADDQ/SUBQ	; size=2 opcode=0x5C40 flow=fallthrough
0x0005ACD8	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005ACDE	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005ACE4	MOVE	; size=6 opcode=0x2479 flow=fallthrough
0x0005ACEA	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005ACF0	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0005ACF6	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005ACFA	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AD00	RTS	; size=2 opcode=0x4E75 flow=return
0x0005AD1A	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AD20	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x0005AD26	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AD2C	MOVE	; size=10 opcode=0x33F9 flow=fallthrough
0x0005AD36	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AD3C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005AD42	BSR 0x05B0BC	; size=4 opcode=0x6100 flow=call
0x0005AD46	BSR 0x05A9E2	; size=4 opcode=0x6100 flow=call
0x0005AD4A	MOVE	; size=6 opcode=0x3F39 flow=fallthrough
0x0005AD50	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005AD56	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005AD5C	MOVEQ	; size=2 opcode=0x701F flow=fallthrough
0x0005AD5E	MOVE	; size=2 opcode=0x22D8 flow=fallthrough
0x0005AD60	DBcc 0x05AD5E	; size=4 opcode=0x51C8 flow=conditional
0x0005AD64		; size=2 opcode=0x08F9 flow=fallthrough
0x0005AF62	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AF68	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AF6E	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AF74	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005AF7A		; size=2 opcode=0x08F9 flow=fallthrough
0x0005B002	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005B008	RTS	; size=2 opcode=0x4E75 flow=return
0x0005B0BC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005B0C4	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0005B0CA	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0005B0CE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005B0D4	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005B0DA	ADDQ/SUBQ	; size=2 opcode=0x514B flow=fallthrough
0x0005B0DC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005B0E4	unary-op	; size=4 opcode=0x422B flow=fallthrough
0x0005B0E8		; size=2 opcode=0x08F9 flow=fallthrough
0x0005B100	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B106	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005B10A	Bcc[13] 0x05B17E	; size=4 opcode=0x6D00 flow=conditional
0x0005B10E	immediate-op	; size=4 opcode=0x0640 flow=fallthrough
0x0005B112	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B118	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x0005B11E	Bcc[5] 0x05B160	; size=4 opcode=0x6500 flow=conditional
0x0005B122	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005B128	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B12E	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005B130	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005B132	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x0005B134	MOVE	; size=6 opcode=0x23D8 flow=fallthrough
0x0005B13A	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B140	ADDQ/SUBQ	; size=2 opcode=0x5C40 flow=fallthrough
0x0005B142	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005B148	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B14E	ADDQ/SUBQ	; size=2 opcode=0x5240 flow=fallthrough
0x0005B150	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005B154	Bcc[5] 0x05B15A	; size=4 opcode=0x6500 flow=conditional
0x0005B158	unary-op	; size=2 opcode=0x4240 flow=fallthrough
0x0005B15A	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005B160	MOVE	; size=6 opcode=0x2479 flow=fallthrough
0x0005B166	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005B16C	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x0005B170	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0005B174	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005B178	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005B17E	RTS	; size=2 opcode=0x4E75 flow=return
0x0005B198	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005B19E	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005B1A4	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005B1AC	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005B1B4	RTS	; size=2 opcode=0x4E75 flow=return
0x0005B1B6	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B1BC	arithmetic-op	; size=6 opcode=0x9079 flow=fallthrough
0x0005B1C2	immediate-op	; size=4 opcode=0x0C40 flow=fallthrough
0x0005B1C6	Bcc[5] 0x05B26E	; size=4 opcode=0x6500 flow=conditional
0x0005B1CA	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B1D0	arithmetic-op	; size=6 opcode=0xB079 flow=fallthrough
0x0005B1D6	Bcc[5] 0x05B21C	; size=4 opcode=0x6500 flow=conditional
0x0005B1DA	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005B1E2	Bcc[4] 0x05B21C	; size=4 opcode=0x6400 flow=conditional
0x0005B1E6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005B1EC	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005B1F2	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005B1F4	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x0005B1F6	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005B1F8	arithmetic-op	; size=2 opcode=0xD041 flow=fallthrough
0x0005B1FA	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x0005B1FC	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x0005B202	MOVE	; size=6 opcode=0x33D8 flow=fallthrough
0x0005B208	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x0005B20A	arithmetic-op	; size=6 opcode=0xD079 flow=fallthrough
0x0005B210	MOVE	; size=6 opcode=0x33C0 flow=fallthrough
0x0005B216	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005B21C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005B222	arithmetic-op	; size=6 opcode=0xD279 flow=fallthrough
0x0005B228	MOVE	; size=6 opcode=0x33C1 flow=fallthrough
0x0005B22E	arithmetic-op	; size=2 opcode=0xE049 flow=fallthrough
0x0005B230	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0005B236	arithmetic-op	; size=6 opcode=0xD479 flow=fallthrough
0x0005B23C	MOVE	; size=6 opcode=0x33C2 flow=fallthrough
0x0005B242	arithmetic-op	; size=2 opcode=0xE04A flow=fallthrough
0x0005B244	immediate-op	; size=4 opcode=0x0642 flow=fallthrough
0x0005B248	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0005B24E	immediate-op	; size=4 opcode=0x0C42 flow=fallthrough
0x0005B252	Bcc[5] 0x05B264	; size=4 opcode=0x6500 flow=conditional
0x0005B256	immediate-op	; size=4 opcode=0x0C42 flow=fallthrough
0x0005B25A	Bcc[4] 0x05B264	; size=4 opcode=0x6400 flow=conditional
0x0005B25E	MOVE	; size=6 opcode=0x247C flow=fallthrough
0x0005B264	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005B268	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005B26E	RTS	; size=2 opcode=0x4E75 flow=return
0x0005B29A		; size=2 opcode=0x40E7 flow=fallthrough
0x0005B2E2	arithmetic-op	; size=2 opcode=0xEB48 flow=fallthrough
0x0005B2E4	ADDQ/SUBQ	; size=2 opcode=0x5440 flow=fallthrough
0x0005B2E6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005B2EC	SUBA/CMPA/ADDA	; size=2 opcode=0xD0C0 flow=fallthrough
0x0005B2EE	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0005B2F2	MOVE	; size=2 opcode=0x3010 flow=fallthrough
0x0005B2F4	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x0005B2F6	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x0005B2FA	Bcc[7] 0x05B300	; size=4 opcode=0x6700 flow=conditional
0x0005B2FE	ADDQ/SUBQ	; size=2 opcode=0x5540 flow=fallthrough
0x0005B300	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x0005B302	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x0005B306	Bcc[7] 0x05B30E	; size=4 opcode=0x6700 flow=conditional
0x0005B30A	immediate-op	; size=4 opcode=0x0440 flow=fallthrough
0x0005B30E	MOVE	; size=2 opcode=0x3200 flow=fallthrough
0x0005B310	immediate-op	; size=4 opcode=0x0241 flow=fallthrough
0x0005B314	Bcc[7] 0x05B31C	; size=4 opcode=0x6700 flow=conditional
0x0005B318	immediate-op	; size=4 opcode=0x0440 flow=fallthrough
0x0005B31C	MOVE	; size=2 opcode=0x30C0 flow=fallthrough
0x0005B31E	DBcc 0x05B2F2	; size=4 opcode=0x51CA flow=conditional
0x0005B322		; size=2 opcode=0x08F9 flow=fallthrough
0x0005B32C	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0005B330		; size=2 opcode=0x40E7 flow=fallthrough
0x0005BA4C	BRA 0x05BA50	; size=4 opcode=0x6000 flow=branch
0x0005BA50	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005BA56	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0005BA5C	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005BA60	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005BA66	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005BA6C	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005BA72	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005BA78	MOVE	; size=2 opcode=0x329A flow=fallthrough
0x0005BA7A	MOVE	; size=4 opcode=0x335A flow=fallthrough
0x0005BA7E	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x0005BA80	MOVE	; size=2 opcode=0x329A flow=fallthrough
0x0005BA82	MOVE	; size=4 opcode=0x335A flow=fallthrough
0x0005BA86	SUBA/CMPA/ADDA	; size=2 opcode=0xD2C0 flow=fallthrough
0x0005BA88	MOVE	; size=2 opcode=0x329A flow=fallthrough
0x0005BA8A	MOVE	; size=4 opcode=0x335A flow=fallthrough
0x0005BA8E	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005BA96	RTS	; size=2 opcode=0x4E75 flow=return
0x0005BA98	MOVE	; size=6 opcode=0x3A39 flow=fallthrough
0x0005BA9E	immediate-op	; size=4 opcode=0x0445 flow=fallthrough
0x0005BAA2	arithmetic-op	; size=2 opcode=0xDA45 flow=fallthrough
0x0005BAA4	arithmetic-op	; size=2 opcode=0xDA45 flow=fallthrough
0x0005BAA6	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0005BAAC	MOVE	; size=4 opcode=0x2673 flow=fallthrough
0x0005BAB0	MOVE	; size=6 opcode=0x1039 flow=fallthrough
0x0005BAB6	EXT	; size=2 opcode=0x4880 flow=fallthrough
0x0005BAB8	ADDQ/SUBQ	; size=2 opcode=0x5340 flow=fallthrough
0x0005BABA	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005BABC	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005BABE	MOVE	; size=4 opcode=0x2673 flow=fallthrough
0x0005BAC2	RTS	; size=2 opcode=0x4E75 flow=return
0x0005C8C6	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0005C8CA		; size=2 opcode=0x40E7 flow=fallthrough
0x0005D260	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D266	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005D26A	unary-op	; size=2 opcode=0x4298 flow=fallthrough
0x0005D26C	DBcc 0x05D26A	; size=4 opcode=0x51CF flow=conditional
0x0005D270	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D276	MOVE	; size=4 opcode=0x30BC flow=fallthrough
0x0005D27A	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D280	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D286	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D28C	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D29C	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0005D2A4	Bcc[7] 0x05D2B6	; size=4 opcode=0x6700 flow=conditional
0x0005D2A8	immediate-op	; size=8 opcode=0x0C39 flow=fallthrough
0x0005D2B0	Bcc[7] 0x05D2B6	; size=4 opcode=0x6700 flow=conditional
0x0005D2B4	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D2B6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D2BC	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0005D2C0	Bcc[11] 0x05D2B4	; size=2 opcode=0x6BF2 flow=conditional
0x0005D2C2	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D2C8	Bcc[7] 0x05D330	; size=4 opcode=0x6700 flow=conditional
0x0005D2CC	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0005D2D0	Bcc[6] 0x05D2F0	; size=4 opcode=0x6600 flow=conditional
0x0005D2D4	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D2D8	Bcc[10] 0x05D408	; size=4 opcode=0x6A00 flow=conditional
0x0005D2DC	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D2E2	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D2E8	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D2EE	BRA 0x05D2B4	; size=2 opcode=0x60C4 flow=branch
0x0005D2F0	unary-op	; size=4 opcode=0x4A68 flow=fallthrough
0x0005D2F4	Bcc[11] 0x05D302	; size=4 opcode=0x6B00 flow=conditional
0x0005D2F8	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D2FE	Bcc[4] 0x05D306	; size=4 opcode=0x6400 flow=conditional
0x0005D302	ADDQ/SUBQ	; size=4 opcode=0x5068 flow=fallthrough
0x0005D306	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D30C	BSR 0x05D55C	; size=4 opcode=0x6100 flow=call
0x0005D310	BSR 0x05D5FC	; size=4 opcode=0x6100 flow=call
0x0005D314	BSR 0x05D5A2	; size=4 opcode=0x6100 flow=call
0x0005D318	BSR 0x05D57E	; size=4 opcode=0x6100 flow=call
0x0005D31C	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D320	Bcc[10] 0x05D2B4	; size=2 opcode=0x6A92 flow=conditional
0x0005D322	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D328	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D32E	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D330	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D336	BSR 0x05D55C	; size=4 opcode=0x6100 flow=call
0x0005D33A	BSR 0x05D65A	; size=4 opcode=0x6100 flow=call
0x0005D33E	BSR 0x05D5FC	; size=4 opcode=0x6100 flow=call
0x0005D342	BSR 0x05D5A2	; size=4 opcode=0x6100 flow=call
0x0005D346	BSR 0x05D57E	; size=4 opcode=0x6100 flow=call
0x0005D34A	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D350	Bcc[6] 0x05D2B4	; size=4 opcode=0x6600 flow=conditional
0x0005D354	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x0005D35A	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005D360	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005D364	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x0005D368	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0005D36C	immediate-op	; size=4 opcode=0x0022 flow=fallthrough
0x0005D370	immediate-op	; size=4 opcode=0x0004 flow=fallthrough
0x0005D374	Bcc[6] 0x05D38E	; size=4 opcode=0x6600 flow=conditional
0x0005D378	arithmetic-op	; size=4 opcode=0x0829 flow=fallthrough
0x0005D37C	immediate-op	; size=4 opcode=0x0010 flow=fallthrough
0x0005D380	immediate-op	; size=4 opcode=0x000E flow=fallthrough
0x0005D384	immediate-op	; size=4 opcode=0x004C flow=fallthrough
0x0005D388	immediate-op	; size=4 opcode=0x0006 flow=fallthrough
0x0005D38C	immediate-op	; size=4 opcode=0x005A flow=fallthrough
0x0005D38E	SUBA/CMPA/ADDA	; size=4 opcode=0xD2FC flow=fallthrough
0x0005D390	bit-op	; size=2 opcode=0x0100 flow=fallthrough
0x0005D392	DBcc 0x05D364	; size=4 opcode=0x51CF flow=conditional
0x0005D396	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D39A	Bcc[11] 0x05D3D4	; size=4 opcode=0x6B00 flow=conditional
0x0005D39E	MOVE	; size=6 opcode=0x227C flow=fallthrough
0x0005D3A4	MOVE	; size=6 opcode=0x117C flow=fallthrough
0x0005D3AA	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x0005D3B0	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x0005D3B6	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x0005D3BC	BSR 0x05D3E6	; size=4 opcode=0x6100 flow=call
0x0005D3C0	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x0005D3C6	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x0005D3CC	MOVE	; size=6 opcode=0x137C flow=fallthrough
0x0005D3D2	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D3D4	MOVE	; size=10 opcode=0x23FC flow=fallthrough
0x0005D3DE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D3E4	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D3E6		; size=2 opcode=0x08E9 flow=fallthrough
0x0005D408	MOVEM	; size=4 opcode=0x48E7 flow=fallthrough
0x0005D40C	BSR 0x05D416	; size=4 opcode=0x6100 flow=call
0x0005D410	MOVEM	; size=4 opcode=0x4CDF flow=fallthrough
0x0005D414	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D416	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D41C	Bcc[14] 0x05D53A	; size=4 opcode=0x6E00 flow=conditional
0x0005D420	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D426	Bcc[7] 0x05D4DA	; size=4 opcode=0x6700 flow=conditional
0x0005D42A	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D430	Bcc[7] 0x05D516	; size=4 opcode=0x6700 flow=conditional
0x0005D434	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D43A	Bcc[7] 0x05D506	; size=4 opcode=0x6700 flow=conditional
0x0005D43E	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D444	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D448	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D44E	Bcc[7] 0x05D506	; size=4 opcode=0x6700 flow=conditional
0x0005D452	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D458	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D45C	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D462	Bcc[7] 0x05D506	; size=4 opcode=0x6700 flow=conditional
0x0005D466	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D46C	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D470	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D476	Bcc[7] 0x05D516	; size=4 opcode=0x6700 flow=conditional
0x0005D47A	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D480	Bcc[7] 0x05D516	; size=4 opcode=0x6700 flow=conditional
0x0005D484	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D48A	Bcc[7] 0x05D516	; size=4 opcode=0x6700 flow=conditional
0x0005D48E	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D494	Bcc[7] 0x05D506	; size=4 opcode=0x6700 flow=conditional
0x0005D498	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D49E	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D4A2	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D4A8	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D4AC	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D4B2	Bcc[7] 0x05D506	; size=4 opcode=0x6700 flow=conditional
0x0005D4B6	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D4BC	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D4C0	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D4C6	Bcc[7] 0x05D4EC	; size=4 opcode=0x6700 flow=conditional
0x0005D4CA	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D4D0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D4D6	BRA 0x05D522	; size=4 opcode=0x6000 flow=branch
0x0005D4DA	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005D4DE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D4E4	BSR 0x05D6BC	; size=4 opcode=0x6100 flow=call
0x0005D4E8	BRA 0x05D522	; size=4 opcode=0x6000 flow=branch
0x0005D4EC	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005D4F0	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D4F6	ADDQ/SUBQ	; size=6 opcode=0x5079 flow=fallthrough
0x0005D4FC	ADDQ/SUBQ	; size=6 opcode=0x5079 flow=fallthrough
0x0005D502	BRA 0x05D522	; size=4 opcode=0x6000 flow=branch
0x0005D506	ADDQ/SUBQ	; size=6 opcode=0x5179 flow=fallthrough
0x0005D50C	ADDQ/SUBQ	; size=6 opcode=0x5179 flow=fallthrough
0x0005D512	BRA 0x05D522	; size=4 opcode=0x6000 flow=branch
0x0005D516	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D51C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D522		; size=2 opcode=0x08F9 flow=fallthrough
0x0005D53A	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D55C	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D560	Bcc[10] 0x05D57C	; size=4 opcode=0x6A00 flow=conditional
0x0005D564	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D56A	ADDQ/SUBQ	; size=4 opcode=0x5268 flow=fallthrough
0x0005D56E	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D574	Bcc[13] 0x05D57C	; size=4 opcode=0x6D00 flow=conditional
0x0005D578	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0005D57C	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D57E	MOVE	; size=4 opcode=0x3228 flow=fallthrough
0x0005D582	MOVE	; size=4 opcode=0x3428 flow=fallthrough
0x0005D586	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D58A	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D58C	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D58E	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D594	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0005D598	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D59C	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005D5A2	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D5A6	Bcc[10] 0x05D5C2	; size=4 opcode=0x6A00 flow=conditional
0x0005D5AA	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D5B0	ADDQ/SUBQ	; size=4 opcode=0x5268 flow=fallthrough
0x0005D5B4	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D5BA	Bcc[13] 0x05D5C2	; size=4 opcode=0x6D00 flow=conditional
0x0005D5BE	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0005D5C2	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D5C6	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D5C8	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D5CA	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D5D0	MOVE	; size=4 opcode=0x3232 flow=fallthrough
0x0005D5D4	arithmetic-op	; size=4 opcode=0xD268 flow=fallthrough
0x0005D5D8	MOVE	; size=4 opcode=0x3432 flow=fallthrough
0x0005D5DC	arithmetic-op	; size=4 opcode=0xD468 flow=fallthrough
0x0005D5E0	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D5E4	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D5E6	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D5E8	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D5EE	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0005D5F2	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D5F6	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005D5FC	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D600	Bcc[10] 0x05D61C	; size=4 opcode=0x6A00 flow=conditional
0x0005D604	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D60A	ADDQ/SUBQ	; size=4 opcode=0x5268 flow=fallthrough
0x0005D60E	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D614	Bcc[13] 0x05D61C	; size=4 opcode=0x6D00 flow=conditional
0x0005D618	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0005D61C	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D620	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D622	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D624	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D62A	MOVE	; size=4 opcode=0x3232 flow=fallthrough
0x0005D62E	arithmetic-op	; size=4 opcode=0xD268 flow=fallthrough
0x0005D632	MOVE	; size=4 opcode=0x3432 flow=fallthrough
0x0005D636	arithmetic-op	; size=4 opcode=0xD468 flow=fallthrough
0x0005D63A	immediate-op	; size=4 opcode=0x0442 flow=fallthrough
0x0005D63E	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D642	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D644	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D646	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D64C	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0005D650	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D654	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005D65A	ADDQ/SUBQ	; size=4 opcode=0x5368 flow=fallthrough
0x0005D65E	Bcc[10] 0x05D67A	; size=4 opcode=0x6A00 flow=conditional
0x0005D662	MOVE	; size=6 opcode=0x317C flow=fallthrough
0x0005D668	ADDQ/SUBQ	; size=4 opcode=0x5268 flow=fallthrough
0x0005D66C	immediate-op	; size=6 opcode=0x0C68 flow=fallthrough
0x0005D672	Bcc[13] 0x05D67A	; size=4 opcode=0x6D00 flow=conditional
0x0005D676	unary-op	; size=4 opcode=0x4268 flow=fallthrough
0x0005D67A	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D67E	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D680	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D682	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D688	MOVE	; size=4 opcode=0x3232 flow=fallthrough
0x0005D68C	arithmetic-op	; size=4 opcode=0xD268 flow=fallthrough
0x0005D690	MOVE	; size=4 opcode=0x3432 flow=fallthrough
0x0005D694	arithmetic-op	; size=4 opcode=0xD468 flow=fallthrough
0x0005D698	immediate-op	; size=4 opcode=0x0642 flow=fallthrough
0x0005D69C	immediate-op	; size=4 opcode=0x0641 flow=fallthrough
0x0005D6A0	MOVE	; size=4 opcode=0x3028 flow=fallthrough
0x0005D6A4	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D6A6	arithmetic-op	; size=2 opcode=0xD040 flow=fallthrough
0x0005D6A8	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D6AE	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0005D6B2	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D6B6	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005D6BC	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D6C2	Bcc[5] 0x05D744	; size=4 opcode=0x6500 flow=conditional
0x0005D6C6	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D6CC	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005D6D0	MOVE	; size=2 opcode=0x2F09 flow=fallthrough
0x0005D6D2	MOVE	; size=2 opcode=0x32D8 flow=fallthrough
0x0005D6D4	DBcc 0x05D6D2	; size=4 opcode=0x51CF flow=conditional
0x0005D6D8	MOVE	; size=2 opcode=0x225F flow=fallthrough
0x0005D6DA	MOVE	; size=8 opcode=0x237C flow=fallthrough
0x0005D6E2	MOVE	; size=6 opcode=0x337C flow=fallthrough
0x0005D6E8	MOVE	; size=4 opcode=0x32BC flow=fallthrough
0x0005D6EC	MOVE	; size=6 opcode=0x337C flow=fallthrough
0x0005D6F2	MOVE	; size=4 opcode=0x3340 flow=fallthrough
0x0005D6F6	MOVE	; size=6 opcode=0x337C flow=fallthrough
0x0005D6FC		; size=2 opcode=0x08E9 flow=fallthrough
0x0005D744	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D746	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D74E	BSR 0x05DA50	; size=4 opcode=0x6100 flow=call
0x0005D752	MOVE	; size=8 opcode=0x13FC flow=fallthrough
0x0005D75A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D762	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D76A	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D772	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005D778	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005D77C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D784	MOVE	; size=2 opcode=0x3F07 flow=fallthrough
0x0005D786	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D78C	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0005D792	Bcc[7] 0x05D79A	; size=4 opcode=0x6700 flow=conditional
0x0005D796	BSR 0x05D8BA	; size=4 opcode=0x6100 flow=call
0x0005D79A	BSR 0x05DAC4	; size=4 opcode=0x6100 flow=call
0x0005D79E	BSR 0x05D938	; size=4 opcode=0x6100 flow=call
0x0005D7A2	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D7AA	Bcc[7] 0x05D7C8	; size=4 opcode=0x6700 flow=conditional
0x0005D7AE	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005D7B4	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D7BC	Bcc[6] 0x05D838	; size=4 opcode=0x6600 flow=conditional
0x0005D7C0	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D7C8	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005D7CE	Bcc[10] 0x05D838	; size=4 opcode=0x6A00 flow=conditional
0x0005D7D2	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005D7D8	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005D7DE	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0005D7E2	Bcc[6] 0x05D838	; size=4 opcode=0x6600 flow=conditional
0x0005D7E6	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D7EE	Bcc[14] 0x05D7FC	; size=4 opcode=0x6E00 flow=conditional
0x0005D7F2	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005D7F6	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D7FC	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005D802	Bcc[11] 0x05D838	; size=4 opcode=0x6B00 flow=conditional
0x0005D806	Bcc[6] 0x05D81C	; size=4 opcode=0x6600 flow=conditional
0x0005D80A	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005D80E	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D814	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D81C	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005D822	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D82A	Bcc[13] 0x05D838	; size=4 opcode=0x6D00 flow=conditional
0x0005D82E	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005D832	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D838		; size=2 opcode=0x08F9 flow=fallthrough
0x0005D8BA	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005D8C0	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005D8C6	Bcc[11] 0x05D8E0	; size=4 opcode=0x6B00 flow=conditional
0x0005D8CA	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D8D2	Bcc[6] 0x05D936	; size=4 opcode=0x6600 flow=conditional
0x0005D8D6	MOVE	; size=4 opcode=0x303C flow=fallthrough
0x0005D8DA	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005D8E0	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D8E8	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005D8EE	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D8F6	Bcc[5] 0x05D900	; size=4 opcode=0x6500 flow=conditional
0x0005D8FA	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005D900	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005D906	arithmetic-op	; size=2 opcode=0xE349 flow=fallthrough
0x0005D908	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005D90E	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D912	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D916	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D91A	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D91E	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D922	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D926	MOVE	; size=4 opcode=0x32F0 flow=fallthrough
0x0005D92A	MOVE	; size=4 opcode=0x32B0 flow=fallthrough
0x0005D92E		; size=2 opcode=0x08F9 flow=fallthrough
0x0005D936	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D938	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D940	LEA	; size=6 opcode=0x47F9 flow=fallthrough
0x0005D946	MOVE	; size=4 opcode=0x3A3C flow=fallthrough
0x0005D94A	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D950	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005D956	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0005D95C	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D960	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D966	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D96C	MOVE	; size=6 opcode=0x3239 flow=fallthrough
0x0005D972	MOVE	; size=6 opcode=0x3439 flow=fallthrough
0x0005D978	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D97C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D982	BSR 0x05D996	; size=4 opcode=0x6100 flow=call
0x0005D986	ADDQ/SUBQ	; size=2 opcode=0x514B flow=fallthrough
0x0005D988	unary-op	; size=4 opcode=0x422B flow=fallthrough
0x0005D98C	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D994	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D996	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0005D99C	Bcc[11] 0x05D9A2	; size=4 opcode=0x6B00 flow=conditional
0x0005D9A0	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D9A2	unary-op	; size=6 opcode=0x4A79 flow=fallthrough
0x0005D9A8	Bcc[7] 0x05D9AE	; size=4 opcode=0x6700 flow=conditional
0x0005D9AC	RTS	; size=2 opcode=0x4E75 flow=return
0x0005D9AE	ADDQ/SUBQ	; size=6 opcode=0x5379 flow=fallthrough
0x0005D9B4	Bcc[10] 0x05D9D8	; size=4 opcode=0x6A00 flow=conditional
0x0005D9B8	MOVE	; size=8 opcode=0x33FC flow=fallthrough
0x0005D9C0	ADDQ/SUBQ	; size=6 opcode=0x5279 flow=fallthrough
0x0005D9C6	immediate-op	; size=8 opcode=0x0C79 flow=fallthrough
0x0005D9CE	Bcc[13] 0x05D9D8	; size=4 opcode=0x6D00 flow=conditional
0x0005D9D2	unary-op	; size=6 opcode=0x4279 flow=fallthrough
0x0005D9D8	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005D9DE	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005D9E4	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0005D9E6	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0005D9EA	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005D9EE	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0005D9F2	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005D9F6	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005D9FC	LEA	; size=6 opcode=0x45F9 flow=fallthrough
0x0005DA02	MOVE	; size=6 opcode=0x3039 flow=fallthrough
0x0005DA08	arithmetic-op	; size=2 opcode=0xE548 flow=fallthrough
0x0005DA0A	MOVE	; size=4 opcode=0x2472 flow=fallthrough
0x0005DA0E	MOVE	; size=4 opcode=0x323C flow=fallthrough
0x0005DA12	MOVE	; size=4 opcode=0x343C flow=fallthrough
0x0005DA16	MOVE	; size=4 opcode=0x3C3C flow=fallthrough
0x0005DA1A	JMP	; size=6 opcode=0x4EF9 flow=branch
0x0005DA50	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DA56	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DA5C	LEA	; size=6 opcode=0x4DF9 flow=fallthrough
0x0005DA62	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DA68	BSR 0x05DAE8	; size=4 opcode=0x6100 flow=call
0x0005DA6C	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DA72	LEA	; size=6 opcode=0x43F9 flow=fallthrough
0x0005DA78	MOVE	; size=6 opcode=0x267C flow=fallthrough
0x0005DA7E	MOVE	; size=6 opcode=0x203C flow=fallthrough
0x0005DA84	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DA8A		; size=2 opcode=0x08F9 flow=fallthrough
0x0005DAC4	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005DACA	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005DACE	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DAD4	arithmetic-op	; size=2 opcode=0xEA48 flow=fallthrough
0x0005DAD6	immediate-op	; size=4 opcode=0x0240 flow=fallthrough
0x0005DADA	MOVE	; size=2 opcode=0x3080 flow=fallthrough
0x0005DADC	SUBA/CMPA/ADDA	; size=4 opcode=0xD0FC flow=fallthrough
0x0005DAE0	DBcc 0x05DACE	; size=4 opcode=0x51CF flow=conditional
0x0005DAE4	BRA 0x05DAFC	; size=4 opcode=0x6000 flow=branch
0x0005DAE8	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005DAEE	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005DAF2	unary-op	; size=2 opcode=0x4258 flow=fallthrough
0x0005DAF4	DBcc 0x05DAF2	; size=4 opcode=0x51CF flow=conditional
0x0005DAF8	BRA 0x05DAFC	; size=4 opcode=0x6000 flow=branch
0x0005DAFC	LEA	; size=6 opcode=0x41F9 flow=fallthrough
0x0005DB02	MOVE	; size=4 opcode=0x3E3C flow=fallthrough
0x0005DB06	MOVEQ	; size=2 opcode=0x7000 flow=fallthrough
0x0005DB08	JSR	; size=6 opcode=0x4EB9 flow=call
0x0005DB0E	RTS	; size=2 opcode=0x4E75 flow=return
