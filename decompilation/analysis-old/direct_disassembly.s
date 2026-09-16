0x00000200	STOP	; size=4 opcode=0x4E72
0x00000208	STOP	; size=4 opcode=0x4E72
0x00000210	STOP	; size=4 opcode=0x4E72
0x00000218	STOP	; size=4 opcode=0x4E72
0x00000220	STOP	; size=4 opcode=0x4E72
0x00000228	JMP	; size=6 opcode=0x4EF9
0x0000022E	JMP	; size=6 opcode=0x4EF9
0x00000234	RTE	; size=2 opcode=0x4E73
0x00000236	unary-op	; size=6 opcode=0x4AB9
0x0000023C	Bcc[6] 0x000244	; size=2 opcode=0x6606
0x0000023E	unary-op	; size=6 opcode=0x4A79
0x00000244	Bcc[6] 0x0002C2	; size=2 opcode=0x667C
0x00000246	LEA	; size=4 opcode=0x4BFA
0x0000024A	MOVEM	; size=4 opcode=0x4C9D
0x0000024E	MOVEM	; size=4 opcode=0x4CDD
0x00000252	MOVE	; size=4 opcode=0x1029
0x00000256	immediate-op	; size=4 opcode=0x0200
0x0000025A	Bcc[7] 0x000264	; size=2 opcode=0x6708
0x0000025C	MOVE	; size=8 opcode=0x237C
0x00000264	MOVE	; size=2 opcode=0x3014
0x00000266	MOVEQ	; size=2 opcode=0x7000
0x00000268	MOVE	; size=2 opcode=0x2C40
0x0000026A	MOVE USP	; size=2 opcode=0x4E66
0x0000026C	MOVEQ	; size=2 opcode=0x7217
0x0000026E	MOVE	; size=2 opcode=0x1A1D
0x00000270	MOVE	; size=2 opcode=0x3885
0x00000272	arithmetic-op	; size=2 opcode=0xDA47
0x00000274	DBcc 0x00026E	; size=4 opcode=0x51C9
0x00000278	MOVE	; size=2 opcode=0x289D
0x0000027A	MOVE	; size=2 opcode=0x3680
0x0000027C	MOVE	; size=2 opcode=0x3287
0x0000027E	MOVE	; size=2 opcode=0x3487
0x00000280	bit-op	; size=2 opcode=0x0111
0x00000282	Bcc[6] 0x000280	; size=2 opcode=0x66FC
0x00000284	MOVEQ	; size=2 opcode=0x7425
0x00000286	MOVE	; size=2 opcode=0x10DD
0x00000288	DBcc 0x000286	; size=4 opcode=0x51CA
0x0000028C	MOVE	; size=2 opcode=0x3480
0x0000028E	MOVE	; size=2 opcode=0x3280
0x00000290	MOVE	; size=2 opcode=0x3487
0x00000292	MOVE	; size=2 opcode=0x2D00
0x00000294	DBcc 0x000292	; size=4 opcode=0x51CE
0x00000298	MOVE	; size=2 opcode=0x289D
0x0000029A	MOVE	; size=2 opcode=0x289D
0x0000029C	MOVEQ	; size=2 opcode=0x761F
0x0000029E	MOVE	; size=2 opcode=0x2680
0x000002A0	DBcc 0x00029E	; size=4 opcode=0x51CB
0x000002A4	MOVE	; size=2 opcode=0x289D
0x000002A6	MOVEQ	; size=2 opcode=0x7813
0x000002A8	MOVE	; size=2 opcode=0x2680
0x000002AA	DBcc 0x0002A8	; size=4 opcode=0x51CC
0x000002AE	MOVEQ	; size=2 opcode=0x7A03
0x000002B0	MOVE	; size=4 opcode=0x175D
0x000002B4	DBcc 0x0002B0	; size=4 opcode=0x51CD
0x000002B8	MOVE	; size=2 opcode=0x3480
0x000002BA	MOVEM	; size=4 opcode=0x4CD6
0x000002BE		; size=2 opcode=0x46FC
0x000002C2	BRA 0x000330	; size=2 opcode=0x606C
0x00000330	unary-op	; size=6 opcode=0x4A79
0x00000336	unary-op	; size=2 opcode=0x4280
0x00000338	MOVE	; size=6 opcode=0x1039
0x0000033E	arithmetic-op	; size=2 opcode=0xEC08
0x00000340	immediate-op	; size=4 opcode=0x0200
0x00000344	LEA	; size=4 opcode=0x41FA
0x00000348	MOVE	; size=4 opcode=0x1030
0x0000034C	unary-op	; size=2 opcode=0x4A00
0x0000034E	Bcc[7] 0x000436	; size=4 opcode=0x6700
0x00000352	LEA	; size=6 opcode=0x41F9
0x00000358	MOVE	; size=4 opcode=0x323C
0x0000035C	arithmetic-op	; size=2 opcode=0xB010
0x0000035E	Bcc[7] 0x0006C8	; size=4 opcode=0x6700
0x00000362	ADDQ/SUBQ	; size=2 opcode=0x5288
0x00000364	DBcc 0x00035C	; size=4 opcode=0x51C9
0x00000368	LEA	; size=6 opcode=0x49F9
0x0000036E	LEA	; size=6 opcode=0x4BF9
0x00000374	MOVE	; size=4 opcode=0x3ABC
0x00000378	MOVE	; size=4 opcode=0x3ABC
0x0000037C	MOVE	; size=4 opcode=0x3ABC
0x00000380	MOVE	; size=4 opcode=0x3ABC
0x00000384	MOVE	; size=4 opcode=0x3ABC
0x00000388	MOVE	; size=6 opcode=0x2ABC
0x0000038E	MOVE	; size=4 opcode=0x38BC
0x00000392	MOVE	; size=6 opcode=0x2ABC
0x00000398	LEA	; size=4 opcode=0x41FA
0x0000039C	MOVE	; size=4 opcode=0x303C
0x000003A0	MOVE	; size=6 opcode=0x243C
0x000003A6	MOVE	; size=4 opcode=0x3C3C
0x000003AA	MOVE	; size=2 opcode=0x1218
0x000003AC	MOVE	; size=6 opcode=0x283C
0x000003B2	MOVE	; size=4 opcode=0x3A3C
0x000003B6	arithmetic-op	; size=2 opcode=0xE99A
0x000003B8	arithmetic-op	; size=2 opcode=0xE219
0x000003BA	Bcc[4] 0x0003BE	; size=2 opcode=0x6402
0x000003BC	arithmetic-op	; size=2 opcode=0x8882
0x000003BE	DBcc 0x0003B6	; size=4 opcode=0x51CD
0x000003C2	MOVE	; size=2 opcode=0x2884
0x000003C4	DBcc 0x0003AA	; size=4 opcode=0x51CE
0x000003C8	DBcc 0x0003A6	; size=4 opcode=0x51C8
0x000003CC	MOVE	; size=4 opcode=0x123C
0x000003D0	LEA	; size=4 opcode=0x41FA
0x000003D4	MOVE	; size=2 opcode=0x1018
0x000003D6	BSR 0x000438	; size=4 opcode=0x6100
0x000003DA	LEA	; size=6 opcode=0x43F9
0x000003E0	immediate-op	; size=4 opcode=0x0C11
0x000003E4	Bcc[7] 0x000428	; size=2 opcode=0x6742
0x000003E6	LEA	; size=4 opcode=0x45FA
0x000003EA	MOVE	; size=2 opcode=0x381A
0x000003EC	unary-op	; size=2 opcode=0x4A04
0x000003EE	Bcc[7] 0x000424	; size=2 opcode=0x6734
0x000003F0	arithmetic-op	; size=2 opcode=0xB811
0x000003F2	Bcc[6] 0x000420	; size=2 opcode=0x662C
0x000003F4	immediate-op	; size=6 opcode=0x0C29
0x000003FA	Bcc[6] 0x000410	; size=2 opcode=0x6614
0x000003FC		; size=2 opcode=0xB3FC
0x00000410	LEA	; size=4 opcode=0x41FA
0x00000414		; size=2 opcode=0xD1DA
0x00000420	ADDQ/SUBQ	; size=2 opcode=0x588A
0x00000422	BRA 0x0003EA	; size=2 opcode=0x60C6
0x00000424	ADDQ/SUBQ	; size=2 opcode=0x5289
0x00000426	BRA 0x0003E0	; size=2 opcode=0x60B8
0x00000428	LEA	; size=4 opcode=0x41FA
0x0000042C	MOVE	; size=2 opcode=0x1018
0x0000042E	ADDQ/SUBQ	; size=2 opcode=0x5241
0x00000430	BSR 0x000438	; size=4 opcode=0x6100
0x00000434	BRA 0x000434	; size=2 opcode=0x60FE
0x00000436	MOVE	; size=2 opcode=0x1401
0x00000438	immediate-op	; size=6 opcode=0x0282
0x0000043E	SWAP	; size=2 opcode=0x4842
0x00000440	arithmetic-op	; size=2 opcode=0xEF8A
0x00000442	MOVE	; size=2 opcode=0x1600
0x00000444	immediate-op	; size=6 opcode=0x0283
0x0000044A	SWAP	; size=2 opcode=0x4843
0x0000044C	arithmetic-op	; size=2 opcode=0xE383
0x0000044E	arithmetic-op	; size=2 opcode=0xD483
0x00000450	immediate-op	; size=6 opcode=0x0682
0x00000456	MOVE	; size=2 opcode=0x2A82
0x00000458	unary-op	; size=2 opcode=0x4A10
0x0000045A	Bcc[7] 0x00046A	; size=2 opcode=0x670E
0x0000045C	MOVE	; size=2 opcode=0x1418
0x0000045E	immediate-op	; size=4 opcode=0x0402
0x00000462	immediate-op	; size=4 opcode=0x0242
0x00000466	MOVE	; size=2 opcode=0x3882
0x00000468	BRA 0x000458	; size=2 opcode=0x60EE
0x0000046A	RTS	; size=2 opcode=0x4E75
0x000006C8	illegal-line-F	; size=2 opcode=0xFBFC
0x00000E36		; size=2 opcode=0x46FC
0x00000EC2		; size=2 opcode=0x00FF
0x00000EE8	immediate-op	; size=4 opcode=0x0050
0x00000EEC	arithmetic-op	; size=2 opcode=0xDD02
0x00000EEE	Bcc[5] 0x000EFA	; size=4 opcode=0x6500
0x00000EF2	unary-op	; size=6 opcode=0x4239
0x00000EF8	MOVE	; size=6 opcode=0x3439
0x00000EFA		; size=2 opcode=0x00FF
0x00000EFE	ADDQ/SUBQ	; size=2 opcode=0x5242
0x00000F00	Bcc[7] 0x000F16	; size=2 opcode=0x6714
0x00000F02	arithmetic-op	; size=2 opcode=0xE74A
0x00000F04	MOVE	; size=6 opcode=0x203C
0x00000F0A	MOVE	; size=6 opcode=0x3239
0x00000F10	BSR 0x000F9A	; size=4 opcode=0x6100
0x00000F14	RTS	; size=2 opcode=0x4E75
0x00000F16	BSR 0x000E36	; size=4 opcode=0x6100
0x00000F1A	RTS	; size=2 opcode=0x4E75
0x00000F1E		; size=2 opcode=0x00FF
0x00000F3E	unary-op	; size=2 opcode=0x4000
0x00000F40	immediate-op	; size=4 opcode=0x0010
0x00000F44	immediate-op	; size=4 opcode=0x0004
0x00000F48		; size=2 opcode=0x00FF
0x00000F5C	unary-op	; size=2 opcode=0x4284
0x00000F5E	MOVE	; size=2 opcode=0x3802
0x00000F60	arithmetic-op	; size=2 opcode=0xD880
0x00000F62	arithmetic-op	; size=2 opcode=0xB184
0x00000F64	arithmetic-op	; size=2 opcode=0x0804
0x00000F66	immediate-op	; size=4 opcode=0x0010
0x00000F6A	immediate-op	; size=6 opcode=0x002E
0x00000F70	immediate-op	; size=6 opcode=0x0282
0x00000F76	MOVEM	; size=4 opcode=0x48E7
0x00000F7A	MOVE	; size=2 opcode=0x3400
0x00000F7C	unary-op	; size=2 opcode=0x4442
0x00000F7E	MOVE	; size=2 opcode=0x2802
0x00000F80	BSR 0x000F9A	; size=4 opcode=0x6100
0x00000F84	MOVEM	; size=4 opcode=0x4CDF
0x00000F88	arithmetic-op	; size=2 opcode=0xD084
0x00000F8A	arithmetic-op	; size=2 opcode=0xD284
0x00000F8C	arithmetic-op	; size=2 opcode=0x9444
0x00000F8E	BSR 0x000F9A	; size=4 opcode=0x6100
0x00000F92	MOVEM	; size=4 opcode=0x4CDF
0x00000F96	RTS	; size=2 opcode=0x4E75
0x00000F9A		; size=2 opcode=0x46FC
0x00001070	MOVEM	; size=4 opcode=0x48E7
0x00001074	JSR	; size=6 opcode=0x4EB9
0x0000107A	MOVEM	; size=4 opcode=0x4CDF
0x0000107E	RTE	; size=2 opcode=0x4E73
0x00001080	JSR	; size=6 opcode=0x4EB9
0x00001086	RTE	; size=2 opcode=0x4E73
0x00001872	ADDQ/SUBQ	; size=6 opcode=0x52B9
0x00001878	unary-op	; size=6 opcode=0x4279
0x0000187E	arithmetic-op	; size=6 opcode=0x08B9
0x00001884	arithmetic-op	; size=2 opcode=0xEF14
0x00001886	Bcc[7] 0x001894	; size=2 opcode=0x670C
0x00001888	BSR 0x000EE8	; size=4 opcode=0x6100
0x0000188C	MOVE	; size=8 opcode=0x33FC
0x00001894	arithmetic-op	; size=6 opcode=0x08B9
0x0000189A	arithmetic-op	; size=2 opcode=0xEF14
0x0000189C	Bcc[7] 0x0018A2	; size=2 opcode=0x6704
0x0000189E	BSR 0x000EC2	; size=4 opcode=0x6100
0x000018A2	unary-op	; size=6 opcode=0x4A79
0x000018A8	Bcc[6] 0x0018B4	; size=4 opcode=0x6600
0x000018AC	JSR	; size=6 opcode=0x4EB9
0x000018B2	arithmetic-op	; size=6 opcode=0x08B9
0x000018B4	immediate-op	; size=4 opcode=0x0002
0x000018B8	arithmetic-op	; size=2 opcode=0xEF14
0x000018BA	Bcc[7] 0x0018C0	; size=2 opcode=0x6704
0x000018BC	BSR 0x000F1E	; size=4 opcode=0x6100
0x000018C0	arithmetic-op	; size=6 opcode=0x08B9
0x000018C6	arithmetic-op	; size=2 opcode=0xEF14
0x000018C8	Bcc[7] 0x0018CE	; size=2 opcode=0x6704
0x000018CA	BSR 0x000F3E	; size=4 opcode=0x6100
0x000018CE	unary-op	; size=6 opcode=0x4A79
0x000018D4	Bcc[7] 0x0018E0	; size=4 opcode=0x6700
0x000018D8	JSR	; size=6 opcode=0x4EB9
0x000018DE	unary-op	; size=6 opcode=0x4A79
0x000018E0		; size=2 opcode=0x00FF
0x000018E4	Bcc[7] 0x0018F0	; size=4 opcode=0x6700
0x000018E8	JSR	; size=6 opcode=0x4EB9
0x000018EE	unary-op	; size=6 opcode=0x4A79
0x000018F0		; size=2 opcode=0x00FF
0x000018F4	Bcc[7] 0x001900	; size=4 opcode=0x6700
0x000018F8	JSR	; size=6 opcode=0x4EB9
0x000018FE	unary-op	; size=6 opcode=0x4A79
0x00001900		; size=2 opcode=0x00FF
0x00001904	Bcc[7] 0x001910	; size=4 opcode=0x6700
0x00001908	JSR	; size=6 opcode=0x4EB9
0x0000190E	RTS	; size=2 opcode=0x4E75
0x00001910	MOVEM	; size=4 opcode=0x48E7
0x00001914	MOVE	; size=10 opcode=0x23FC
0x0000191E	ADDQ/SUBQ	; size=6 opcode=0x5279
0x00001924	arithmetic-op	; size=6 opcode=0x0839
0x0000192A	illegal-line-F	; size=2 opcode=0xF9F1
0x00002C92	unary-op	; size=6 opcode=0x4279
0x00002C98	MOVE	; size=6 opcode=0x3A39
0x00002C9E	MOVE	; size=6 opcode=0x2079
0x00002CA4	LEA	; size=6 opcode=0x43F9
0x00002CAA	JSR	; size=6 opcode=0x4EB9
0x00002CB0	RTS	; size=2 opcode=0x4E75
0x00002CB2	unary-op	; size=6 opcode=0x4279
0x00002CB8	MOVE	; size=6 opcode=0x3A39
0x00002CBE	MOVE	; size=6 opcode=0x2079
0x00002CC4	LEA	; size=6 opcode=0x43F9
0x00002CCA	MOVE	; size=8 opcode=0x33FC
0x00002CD2	JSR	; size=6 opcode=0x4EB9
0x00002CD8	unary-op	; size=6 opcode=0x4279
0x00002CDE	RTS	; size=2 opcode=0x4E75
0x00003D18	BSR 0x0041B8	; size=4 opcode=0x6100
0x00003D1C	BSR 0x004002	; size=4 opcode=0x6100
0x00003D20	arithmetic-op	; size=2 opcode=0xD845
0x00003D22	MOVE	; size=4 opcode=0x343C
0x00003D26	MOVEM	; size=4 opcode=0x48E7
0x00003D2A	BSR 0x003D4C	; size=4 opcode=0x6100
0x00003D2E	MOVEM	; size=4 opcode=0x4CDF
0x00003D32	MOVE	; size=2 opcode=0x3604
0x00003D34	immediate-op	; size=4 opcode=0x0243
0x00003D38	immediate-op	; size=4 opcode=0x0644
0x00003D3C	immediate-op	; size=4 opcode=0x0244
0x00003D40	arithmetic-op	; size=2 opcode=0x8843
0x00003D42	ADDQ/SUBQ	; size=2 opcode=0x5041
0x00003D44	DBcc 0x003D26	; size=4 opcode=0x51CA
0x00003D48	RTS	; size=2 opcode=0x4E75
0x00003D4C	immediate-op	; size=6 opcode=0x028C
0x00003D52		; size=2 opcode=0x007F
0x00004002	immediate-op	; size=4 opcode=0x0243
0x00004006	immediate-op	; size=4 opcode=0x0244
0x0000400A	arithmetic-op	; size=2 opcode=0xE44B
0x0000400C	arithmetic-op	; size=2 opcode=0xE94C
0x0000400E	arithmetic-op	; size=2 opcode=0xD843
0x00004010	immediate-op	; size=4 opcode=0x0244
0x00004014	EXT	; size=2 opcode=0x48C4
0x00004016	RTS	; size=2 opcode=0x4E75
0x000041B8		; size=2 opcode=0x00FF
0x00006148	MOVE	; size=6 opcode=0x3E39
0x0000614E	MOVE	; size=2 opcode=0x3007
0x00006150	ADDQ/SUBQ	; size=2 opcode=0x5347
0x00006152	MOVE	; size=6 opcode=0x2239
0x00006158	arithmetic-op	; size=2 opcode=0xE508
0x0000615A	arithmetic-op	; size=2 opcode=0x9200
0x0000615C	MOVE	; size=2 opcode=0x2241
0x0000615E	MOVE	; size=2 opcode=0x2051
0x00006160	MOVE	; size=4 opcode=0x2028
0x00006164	arithmetic-op	; size=4 opcode=0x0828
0x00006168	immediate-op	; size=4 opcode=0x0011
0x0000616C	immediate-op	; size=4 opcode=0x0006
0x00006170	immediate-op	; size=4 opcode=0x0044
0x00006174	immediate-op	; size=4 opcode=0x0005
0x00006178	Bcc[7] 0x006182	; size=4 opcode=0x6700
0x0000617C	MOVE	; size=4 opcode=0x2028
0x00006180	MOVE	; size=4 opcode=0x3228
0x00006182	immediate-op	; size=4 opcode=0x0026
0x00006184	MOVE	; size=4 opcode=0x3428
0x00006186	immediate-op	; size=6 opcode=0x0028
0x00006188	JSR	; size=6 opcode=0x4EB9
0x0000618C	bit-op	; size=2 opcode=0x0F5C
0x0000618E	MOVE	; size=2 opcode=0x2009
0x00006190	ADDQ/SUBQ	; size=2 opcode=0x5800
0x00006192	MOVE	; size=2 opcode=0x2240
0x00006194	DBcc 0x00615E	; size=4 opcode=0x51CF
0x00006198	unary-op	; size=6 opcode=0x4279
0x0000619E	RTS	; size=2 opcode=0x4E75
0x000061F4	MOVE	; size=6 opcode=0x3E39
0x000061FA	MOVE	; size=2 opcode=0x3007
0x000061FC	ADDQ/SUBQ	; size=2 opcode=0x5347
0x000061FE	MOVE	; size=6 opcode=0x2239
0x00006204	arithmetic-op	; size=2 opcode=0xE708
0x00006206	arithmetic-op	; size=2 opcode=0x9200
0x00006208	MOVE	; size=2 opcode=0x2241
0x0000620A	MOVE	; size=2 opcode=0x2011
0x0000620C	MOVE	; size=4 opcode=0x3429
0x00006210	MOVE	; size=4 opcode=0x3229
0x00006214	JSR	; size=6 opcode=0x4EB9
0x0000621A	MOVE	; size=2 opcode=0x2009
0x0000621C	ADDQ/SUBQ	; size=2 opcode=0x5000
0x0000621E	MOVE	; size=2 opcode=0x2240
0x00006220	DBcc 0x00620A	; size=4 opcode=0x51CF
0x00006224	unary-op	; size=6 opcode=0x4279
0x0000622A	RTS	; size=2 opcode=0x4E75
0x0005672C	unary-op	; size=6 opcode=0x4A39
0x00056732	Bcc[7] 0x05673A	; size=4 opcode=0x6700
0x00056736	RTS	; size=2 opcode=0x4E75
0x0005673A	immediate-op	; size=4 opcode=0x0014
0x0005673E		; size=2 opcode=0xDDC4
