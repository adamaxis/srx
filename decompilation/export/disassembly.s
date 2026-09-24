00000200: stop #0x2702
00000204: stop #0x2702
00000208: stop #0x2703
0000020C: stop #0x2703
00000210: stop #0x2704
00000214: stop #0x2704
00000218: stop #0x2705
0000021C: stop #0x2705
00000220: stop #0x2700
00000224: stop #0x2700
00000228: jmp 0x00001080.l
0000022E: jmp 0x00001070.l
00000234: rte
00000236: tst.l (0x00a10008).l
0000023C: bne.b 0x00000244
0000023E: tst.w (0x00a1000c).l
00000244: bne.b 0x000002c2
00000246: lea (0x7c,PC),A5 ; data 000002c4
0000024A: movem.w (A5)+,{  D5 D6 D7}
0000024E: movem.l (A5)+,{  A0 A1 A2 A3 A4}
00000252: move.b (-0x10ff,A1),D0b
00000256: andi.b #0xf,D0b
0000025A: beq.b 0x00000264
0000025C: move.l #0x53454741,(0x2f00,A1)
00000264: move.w (A4),D0w
00000266: moveq #0x0,D0
00000268: movea.l D0,A6
0000026A: move A6,USP
0000026C: moveq #0x17,D1
0000026E: move.b (A5)+,D5b
00000270: move.w D5w,(A4)
00000272: add.w D7w,D5w
00000274: dbf D1w,0x0000026e
00000278: move.l (A5)+,(A4)
0000027A: move.w D0w,(A3)
0000027C: move.w D7w,(A1)
0000027E: move.w D7w,(A2)
00000280: btst.b D0,(A1)
00000282: bne.b 0x00000280
00000284: moveq #0x25,D2
00000286: move.b (A5)+,(A0)+
00000288: dbf D2w,0x00000286
0000028C: move.w D0w,(A2)
0000028E: move.w D0w,(A1)
00000290: move.w D7w,(A2)
00000292: move.l D0,-(A6)
00000294: dbf D6w,0x00000292
00000298: move.l (A5)+,(A4)
0000029A: move.l (A5)+,(A4)
0000029C: moveq #0x1f,D3
0000029E: move.l D0,(A3)
000002A0: dbf D3w,0x0000029e
000002A4: move.l (A5)+,(A4)
000002A6: moveq #0x13,D4
000002A8: move.l D0,(A3)
000002AA: dbf D4w,0x000002a8
000002AE: moveq #0x3,D5
000002B0: move.b (A5)+,(0x11,A3)
000002B4: dbf D5w,0x000002b0
000002B8: move.w D0w,(A2)
000002BA: movem.l (A6),{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
000002BE: move #0x2700,SR
000002C2: bra.b 0x00000330
00000330: tst.w (0x00c00004).l
00000336: clr.l D0
00000338: move.b (0x00a10001).l,D0b
0000033E: lsr.b #0x6,D0b
00000340: andi.b #0x3,D0b
00000344: lea (0x126,PC),A0 ; data 0000046c
00000348: move.b (0x0,A0,D0w*0x1),D0b ; data 0000046c
0000034C: tst.b D0b
0000034E: beq.w 0x00000434
00000352: lea (0x1f0).l,A0
00000358: move.w #0xf,D1w
0000035C: cmp.b (A0),D0b
0000035E: beq.w 0x000006c6
00000362: addq.l #0x1,A0
00000364: dbf D1w,0x0000035c
00000368: lea (0xc00000).l,A4 ; data 00c00000
0000036E: lea (0xc00004).l,A5 ; data 00c00004
00000374: move.w #-0x7e9c,(A5)
00000378: move.w #-0x7dd0,(A5)
0000037C: move.w #-0x737f,(A5)
00000380: move.w #-0x70fe,(A5)
00000384: move.w #-0x6fff,(A5)
00000388: move.l #-0x3ffe0000,(A5)
0000038E: move.w #0xeee,(A4)
00000392: move.l #0x40000000,(A5)
00000398: lea (0x154,PC),A0 ; data 000004ee
0000039C: move.w #0x3a,D0w
000003A0: move.l #0x10000000,D2
000003A6: move.w #0x7,D6w
000003AA: move.b (A0)+,D1b
000003AC: move.l #0x0,D4
000003B2: move.w #0x7,D5w
000003B6: rol.l #0x4,D2
000003B8: ror.b #0x1,D1b
000003BA: bcc.b 0x000003be
000003BC: or.l D2,D4
000003BE: dbf D5w,0x000003b6
000003C2: move.l D4,(A4)
000003C4: dbf D6w,0x000003aa
000003C8: dbf D0w,0x000003a6
000003CC: move.b #0x8,D1b
000003D0: lea (0xb2,PC),A0 ; data 00000484
000003D4: move.b (A0)+,D0b
000003D6: bsr.w 0x00000436 ; call 00000436 write_startup_text_to_vdp
000003DA: lea (0x1f0).l,A1
000003E0: cmpi.b #0x20,(A1)
000003E4: beq.b 0x00000428
000003E6: lea (0x88,PC),A2 ; data 00000470
000003EA: move.w (A2)+,D4w
000003EC: tst.b D4b
000003EE: beq.b 0x00000424
000003F0: cmp.b (A1),D4b
000003F2: bne.b 0x00000420
000003F4: cmpi.b #0x20,(0x1,A1)
000003FA: bne.b 0x00000410
000003FC: cmpa.l #0x1f0,A1
00000402: beq.b 0x00000410
00000404: lea (0x9b,PC),A0 ; data 000004a1
00000408: move.b (A0)+,D0b
0000040A: addq.w #0x1,D1w
0000040C: bsr.w 0x00000436 ; call 00000436 write_startup_text_to_vdp
00000410: lea (0x5a,PC),A0 ; data 0000046c
00000414: adda.l (A2)+,A0
00000416: move.b (A0)+,D0b
00000418: addq.w #0x1,D1w
0000041A: bsr.w 0x00000436 ; call 00000436 write_startup_text_to_vdp
0000041E: bra.b 0x00000424
00000420: addq.l #0x4,A2
00000422: bra.b 0x000003ea
00000424: addq.l #0x1,A1
00000426: bra.b 0x000003e0
00000428: lea (0x7a,PC),A0 ; data 000004a4
0000042C: move.b (A0)+,D0b
0000042E: addq.w #0x1,D1w
00000430: bsr.w 0x00000436 ; call 00000436 write_startup_text_to_vdp
00000434: bra.b 0x00000434
00000436: move.b D1b,D2b
00000438: andi.l #0xff,D2
0000043E: swap D2
00000440: lsl.l #0x7,D2
00000442: move.b D0b,D3b
00000444: andi.l #0xff,D3
0000044A: swap D3
0000044C: asl.l #0x1,D3
0000044E: add.l D3,D2
00000450: addi.l #0x40000003,D2
00000456: move.l D2,(A5)
00000458: tst.b (A0)
0000045A: beq.b 0x0000046a
0000045C: move.b (A0)+,D2b
0000045E: subi.b #0x20,D2b
00000462: andi.w #0xff,D2w
00000466: move.w D2w,(A4)
00000468: bra.b 0x00000458
0000046A: rts
00000672: bra.b 0x000006d4
000006C6: lea (-0x404,PC),A5 ; data 000002c4
000006CA: movem.w (A5)+,{  D5 D6 D7}
000006CE: movem.l (A5)+,{  A0 A1 A2 A3 A4}
000006D2: move.w (A4),D0w
000006D4: moveq #0x0,D0
000006D6: movea.l D0,A6
000006D8: move A6,USP
000006DA: moveq #0x17,D1
000006DC: move.b (A5)+,D5b
000006DE: move.w D5w,(A4)
000006E0: add.w D7w,D5w
000006E2: dbf D1w,0x000006dc
000006E6: move.l (A5)+,(A4)
000006E8: move.w D0w,(A3)
000006EA: move.l #0x3ffe,D6
000006F0: movea.l #0x0,A6
000006F6: clr.l D0
000006F8: subq.l #0x4,A6
000006FA: dbf D6w,0x000006f8
000006FE: move.w D7w,(A1)
00000700: move.w D7w,(A2)
00000702: movea.l #0x320,A5
00000708: move.l (A5)+,(A4)
0000070A: move.l (A5)+,(A4)
0000070C: moveq #0x1f,D3
0000070E: move.l D0,(A3)
00000710: dbf D3w,0x0000070e
00000714: move.l (A5)+,(A4)
00000716: moveq #0x13,D4
00000718: move.l D0,(A3)
0000071A: dbf D4w,0x00000718
0000071E: moveq #0x3,D5
00000720: move.b (A5)+,(0x11,A3)
00000724: dbf D5w,0x00000720
00000728: move #0x2700,SR
0000072C: moveq #0xa,D6
0000072E: jsr 0x0000073a.l ; call 0000073A wait_for_vdp_ready
00000734: jmp 0x0000075e.l ; call 0000075E run_game_loop
0000073A: move.w #0xfff,D7w
0000073E: nop
00000740: dbf D7w,0x0000073e
00000744: dbf D6w,0x0000073a
00000748: btst.b #0x3,(0x00c00005).l
00000750: beq.b 0x00000748
00000752: move.w #0xff,D7w
00000756: nop
00000758: dbf D7w,0x00000756
0000075C: rts
0000075E: jsr 0x00000c2c.l ; call 00000C2C clear_runtime_memory_and_vdp
00000764: move #0x2300,SR
00000768: jsr 0x0005d746.l ; call 0005D746 run_pre_title_sequence
0000076E: cmpi.l #0x12345678,(0x00fff9e8).l
00000778: beq.w 0x00000786
0000077C: move.l #0x12345678,(0x00fff9e8).l
00000786: bra.w 0x00000796
0000078A: jsr 0x0000131c.l ; call 0000131C step_palette_transition
00000790: jmp 0x000002c2.l
00000796: move.l (0x00fff9ec).l,D0
0000079C: add.w D0w,(0x00fff9f8).l
000007A2: move.w #0x1,(0x00fffc08).l
000007AA: move.w #0x1,(0x00fffc0a).l
000007B2: move.w #-0x1,(0x00ffdfe8).l
000007BA: jsr 0x00001a9e.l ; call 00001A9E run_title_loop
000007C0: jsr 0x00053126.l ; call 00053126 FUN_00053126
000007C6: tst.w (0x00ffddc0).l
000007CC: beq.b 0x0000078a
000007CE: jsr 0x00059318.l ; call 00059318 FUN_00059318
000007D4: jsr 0x00059092.l ; call 00059092 FUN_00059092
000007DA: jsr 0x0000131c.l ; call 0000131C step_palette_transition
000007E0: tst.w (0x00ffe108).l
000007E6: bne.w 0x000007f0
000007EA: jsr 0x00059c5e.l ; call 00059C5E FUN_00059c5e
000007F0: jsr 0x00053126.l ; call 00053126 FUN_00053126
000007F6: jsr 0x00053290.l ; call 00053290 FUN_00053290
000007FC: move.l #0x14,(0x00fffbfe).l
00000806: clr.l (0x00fffc12).l
0000080C: clr.l (0x00fffc16).l
00000812: clr.l (0x00fffc1a).l
00000818: clr.l (0x00fffc1e).l
0000081E: clr.l (0x00fffc22).l
00000824: clr.b (0x00fffc26).l
0000082A: clr.b (0x00fffc27).l
00000830: clr.w (0x00fffc28).l
00000836: clr.w (0x00fffc2a).l
0000083C: clr.l (0x00fffc2c).l
00000842: clr.l (0x00fffc34).l
00000848: clr.l (0x00fffc38).l
0000084E: move.l #0x1,(0x00fffc30).l
00000858: move.w #0x2,(0x00ffddc4).l
00000860: move.b #0x2,(0x00fffbf8).l
00000868: jsr 0x00003084.l ; call 00003084 run_field_loop
0000086E: move.w #-0x1,(0x00ffdfe8).l
00000876: move.b #-0x1,(0x00fff0a0).l
0000087E: jsr 0x0000a7b4.l ; call 0000A7B4 FUN_0000a7b4
00000884: jsr 0x0001e68c.l ; call 0001E68C FUN_0001e68c
0000088A: jsr 0x000566a2.l ; call 000566A2 FUN_000566a2
00000890: jsr 0x00026938.l ; call 00026938 FUN_00026938
00000896: jsr 0x0000281e.l ; call 0000281E FUN_0000281e
0000089C: tst.w (0x00ffe108).l
000008A2: beq.w 0x000008c0
000008A6: clr.w D0w
000008A8: jsr 0x000042b0.l ; call 000042B0 FUN_000042b0
000008AE: jsr 0x000594e0.l ; call 000594E0 FUN_000594e0
000008B4: jsr 0x000028c2.l ; call 000028C2 FUN_000028c2
000008BA: clr.w (0x00ffe108).l
000008C0: move.w #0x1e,D7w
000008C4: clr.w (0x00ffdfe8).l
000008CA: move.b #0x1,(0x00ffe18a).l
000008D2: clr.w (0x00ffdd22).l
000008D8: clr.w (0x00ffdd26).l
000008DE: move.w #-0x1,(0x00ffdd04).l
000008E6: move.w #-0x1,(0x00ffdd06).l
000008EE: move.w D7w,-(SP) ; data Stack[-0x2]
000008F0: jsr 0x0000a8ea.l ; call 0000A8EA FUN_0000a8ea
000008F6: jsr 0x00003a1c.l ; call 00003A1C FUN_00003a1c
000008FC: jsr 0x00004618.l ; call 00004618 FUN_00004618
00000902: bset.b #0x1,(0x00ffef14).l
0000090A: bset.b #0x2,(0x00ffef14).l
00000912: bset.b #0x3,(0x00ffef14).l
0000091A: move.w (SP)+,D7w
0000091C: dbf D7w,0x000008ee
00000920: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00000926: move.w #-0x1,(0x00ffdfe8).l
0000092E: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
00000934: clr.b (0x00ffe18a).l
0000093A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00000940: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00000946: clr.w (0x00ffdfe6).l
0000094C: clr.w (0x00ffdfe8).l
00000952: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00000958: nop
0000095A: nop
0000095C: nop
0000095E: tst.w (0x00ffdf3e).l
00000964: beq.w 0x00000a68
00000968: move.b #-0x1,(0x00ffe172).l
00000970: clr.w (0x00ffdf3e).l
00000976: jsr 0x00053126.l ; call 00053126 FUN_00053126
0000097C: tst.b (0x00ffe186).l
00000982: beq.w 0x00000992
00000986: move.b #0x2,(0x00ffe186).l
0000098E: bra.w 0x00000a32
00000992: tst.w (0x00ffdf40).l
00000998: bne.w 0x00000a72
0000099C: jsr 0x00003168.l ; call 00003168 FUN_00003168
000009A2: bcs.w 0x00000a54
000009A6: jsr 0x0001f5ca.l ; call 0001F5CA FUN_0001f5ca
000009AC: clr.b (0x00ffe18b).l
000009B2: tst.w D6w
000009B4: beq.w 0x000009fa
000009B8: move.w #-0x8000,(0x00ff012e).l
000009C0: move.w #-0x8000,(0x00ff0134).l
000009C8: move.w #-0x8000,(0x00ff022e).l
000009D0: move.w #-0x8000,(0x00ff0234).l
000009D8: move.w #-0x8000,(0x00ff032e).l
000009E0: move.w #-0x8000,(0x00ff0334).l
000009E8: move.w #-0x1,(0x00ffdfe8).l
000009F0: jsr 0x00002fa4.l ; call 00002FA4 FUN_00002fa4
000009F6: bra.w 0x00000b9c
000009FA: cmpi.w #0x4,(0x00ffddc4).l
00000A02: bne.w 0x00000a12
00000A06: cmpi.b #0x6,(0x00fffbf9).l
00000A0E: beq.w 0x00000a54
00000A12: tst.w (0x00fffc0e).l
00000A18: beq.w 0x00000a32
00000A1C: cmpi.b #0x8,(0x00fffbf9).l
00000A24: bcs.w 0x00000a32
00000A28: jsr 0x00002ce0.l ; call 00002CE0 FUN_00002ce0
00000A2E: bra.w 0x00000a54
00000A32: jsr 0x000167ae.l ; call 000167AE FUN_000167ae
00000A38: cmpi.w #0x37,(0x00ffddc4).l
00000A40: bne.w 0x00000a54
00000A44: move.l #0x20938,(0x00ffdd2c).l
00000A4E: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
00000A54: move.w #-0x1,(0x00ffdfe8).l
00000A5C: move.w #-0x1,(0x00ffdf8e).l
00000A64: bra.w 0x00000b9c
00000A68: tst.w (0x00ffdf42).l
00000A6E: beq.w 0x00000aa8
00000A72: tst.b (0x00ffe18f).l
00000A78: beq.w 0x00000a8a
00000A7C: clr.b (0x00ffe18f).l
00000A82: bset.b #0x7,(0x00fffc1c).l
00000A8A: move.w #-0x1,(0x00ffdfe8).l
00000A92: jsr 0x0001e43c.l ; call 0001E43C FUN_0001e43c
00000A98: jsr 0x00002f32.l ; call 00002F32 FUN_00002f32
00000A9E: clr.w (0x00ffdf42).l
00000AA4: bra.w 0x00000ba2
00000AA8: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00000AAE: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00000AB4: jsr 0x0000a8ea.l ; call 0000A8EA FUN_0000a8ea
00000ABA: jsr 0x00003a1c.l ; call 00003A1C FUN_00003a1c
00000AC0: jsr 0x00004618.l ; call 00004618 FUN_00004618
00000AC6: jsr 0x00016396.l ; call 00016396 FUN_00016396
00000ACC: jsr 0x0001dd94.l ; call 0001DD94 FUN_0001dd94
00000AD2: bset.b #0x1,(0x00ffef14).l
00000ADA: bset.b #0x2,(0x00ffef14).l
00000AE2: bset.b #0x3,(0x00ffef14).l
00000AEA: tst.b (0x00ffe18b).l
00000AF0: bne.w 0x00000b30
00000AF4: tst.w (0x00ffe166).l
00000AFA: bne.w 0x00000b30
00000AFE: tst.w (0x00ffdfe8).l
00000B04: bne.w 0x00000b30
00000B08: btst.b #0xd,(0x00ffdd22).l
00000B10: beq.w 0x00000b1a
00000B14: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
00000B1A: btst.b #0xf,(0x00ffdd22).l
00000B22: bne.w 0x00000b62
00000B26: tst.l (0x00ffe104).l
00000B2C: bne.w 0x00000b34
00000B30: bra.w 0x00000952
00000B34: move.w #-0x1,(0x00ffdfe8).l
00000B3C: movea.l (0x00ffe104).l,A1
00000B42: jsr (A1)
00000B44: cmpi.l #0x5bac4,(0x00ffe104).l
00000B4E: beq.w 0x00000ba6
00000B52: clr.l (0x00ffe104).l
00000B58: movea.l (0x00fffc04).l,A0
00000B5E: bra.w 0x00000b9c
00000B62: move.w #0x8,D0w
00000B66: jsr 0x0005315e.l ; call 0005315E FUN_0005315e
00000B6C: move.w #-0x1,(0x00ffdfe8).l
00000B74: jsr 0x00010d30.l ; call 00010D30 FUN_00010d30
00000B7A: jsr 0x00053156.l ; call 00053156 FUN_00053156
00000B80: jsr 0x0001dc22.l ; call 0001DC22 FUN_0001dc22
00000B86: tst.w (0x00ffe108).l
00000B8C: beq.w 0x00000b9c
00000B90: clr.w D0w
00000B92: jsr 0x000042b0.l ; call 000042B0 FUN_000042b0
00000B98: bra.w 0x000007f0
00000B9C: jsr 0x00002bd2.l ; call 00002BD2 FUN_00002bd2
00000BA2: bra.w 0x000008c0
00000BA6: clr.l (0x00ffe104).l
00000BAC: bra.w 0x0000078a
00000BB0: bsr.w 0x00000bb6 ; call 00000BB6 FUN_00000bb6
00000BB4: rts
00000BB6: move.l (0x00fff9ec).l,D7
00000BBC: andi.l #-0x2,D7
00000BC2: cmp.l (0x00fff9f4).l,D7
00000BC8: beq.b 0x00000bb6
00000BCA: move.l D7,(0x00fff9f4).l
00000BD0: rts
00000BD2: addq.w #0x1,(0x00ffdf88).l
00000BD8: move.w (0x00fff9f6).l,D7w
00000BDE: cmp.w (0x00fff9ee).l,D7w
00000BE4: beq.b 0x00000bde
00000BE6: move.w (0x00fff9ee).l,(0x00fff9f6).l
00000BF0: rts
00000BF2: move.w (0x00ffdd22).l,D0w
00000BF8: or.w (0x00ffdd26).l,D0w
00000BFE: andi.w #-0x100,D0w
00000C02: beq.w 0x00000c22
00000C06: move.w #-0x1,(0x00ffddc0).l
00000C0E: btst.b #0xf,(0x00ffdd22).l
00000C16: beq.w 0x00000c22
00000C1A: move.w #-0x1,(0x00ffddc2).l
00000C22: rts
00000C24: bsr.b 0x00000bd2 ; call 00000BD2 wait_for_vblank
00000C26: dbf D5w,0x00000c24
00000C2A: rts
00000C2C: cmpi.l #0x12345678,(0x00fff9e8).l
00000C36: beq.w 0x00000c58
00000C3A: clr.l D0
00000C3C: move.w #0x3fdf,D7w
00000C40: movea.l #0xff0000,A0
00000C46: add.l (A0),D1
00000C48: move.l D0,(A0)+
00000C4A: dbf D7w,0x00000c46
00000C4E: move.w D1w,(0x00fff9f8).l
00000C54: bra.w 0x00000c6a
00000C58: clr.w D0w
00000C5A: move.w #-0xe19,D7w
00000C5E: movea.l #0xff0000,A0
00000C64: move.b D0b,(A0)+
00000C66: dbf D7w,0x00000c64
00000C6A: jsr 0x00000ec0.l ; call 00000EC0 FUN_00000ec0
00000C70: move.w #0x21,D2w
00000C74: clr.l D0
00000C76: move.w #-0x1,D7w
00000C7A: jsr 0x00000dfc.l ; call 00000DFC FUN_00000dfc
00000C80: move.w #0x1,(0x00ffdd02).l
00000C88: bsr.w 0x00000e34 ; call 00000E34 FUN_00000e34
00000C8C: lea (0xd14).l,A0
00000C92: bsr.w 0x00000cec ; call 00000CEC FUN_00000cec
00000C96: move.w #-0x20,D0w
00000C9A: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
00000C9E: moveq #0x0,D0
00000CA0: move.w #0x7,D1w
00000CA4: move.l D0,(0x00c00000).l
00000CAA: dbf D1w,0x00000ca4
00000CAE: move.l #0x40000010,(0x00c00004).l
00000CB8: move.l #0x0,(0x00c00000).l
00000CC2: cmpi.l #0x12345678,(0x00fff9e8).l
00000CCC: jsr 0x000531aa.l ; call 000531AA FUN_000531aa
00000CD2: jsr 0x000531aa.l ; call 000531AA FUN_000531aa
00000CD8: jsr 0x000531aa.l ; call 000531AA FUN_000531aa
00000CDE: jsr 0x000531aa.l ; call 000531AA FUN_000531aa
00000CE4: jsr 0x000531aa.l ; call 000531AA FUN_000531aa
00000CEA: rts
00000CEC: move.w (A0)+,(0x00ffdd16).l
00000CF2: move.w (A0)+,(0x00ffdd14).l
00000CF8: move.w (A0)+,(0x00ffdd10).l
00000CFE: move.w (A0)+,(0x00ffdd12).l
00000D04: move.w #0xe,D7w
00000D08: move.w (A0)+,(0x00c00004).l
00000D0E: dbf D7w,0x00000d08
00000D12: rts
00000D32: sbcd D2b,D7b
00000D34: sub.b D1b,D0b
00000D36: subx.b D0b,D0b
00000D38: sub.b D0b,D1b
00000D3A: bsr.b 0x00000d44 ; call 00000D44 advance_source_prng
00000D3C: mulu.w D1w,D0
00000D3E: swap D0
00000D40: ext.l D0
00000D42: rts
00000D44: clr.l D0
00000D46: move.w (0x00fff9f8).l,D0w
00000D4C: mulu.w #0x20e5,D0
00000D50: addq.w #0x1,D0w
00000D52: move.w D0w,(0x00fff9f8).l
00000D58: rts
00000D5A: move.b (0x4c,A0),D0b
00000D5E: cmpi.b #0x8,D0b
00000D62: bpl.w 0x00000d76
00000D66: cmpi.b #0x5,D0b
00000D6A: bmi.w 0x00000d74
00000D6E: addq.w #0x1,D4w
00000D70: bra.w 0x00000d76
00000D74: addq.w #0x2,D4w
00000D76: move.b (0x9c,A0),D0b
00000D7A: cmpi.b #0x8,D0b
00000D7E: bmi.w 0x00000d84
00000D82: rts
00000D84: cmpi.b #0x5,D0b
00000D88: bmi.w 0x00000d90
00000D8C: addq.w #0x1,D4w
00000D8E: rts
00000D90: addq.w #0x2,D4w
00000D92: rts
00000D94: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00000D9A: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00000DA0: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00000DA6: move.w (0x00ffdd22).l,D0w
00000DAC: andi.w #-0x100,D0w
00000DB0: beq.b 0x00000d94
00000DB2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00000DB8: rts
00000DBA: movem.l {  D4 D3 D2 D1 D0},-(SP)
00000DBE: subq.w #0x1,D6w
00000DC0: bpl.w 0x00000dc6
00000DC4: clr.w D6w
00000DC6: clr.b (0x00fff0dd).l
00000DCC: moveq #0x6,D1
00000DCE: clr.w D2w
00000DD0: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00000DD6: addq.w #0x1,D0w
00000DD8: cmpi.w #0x6,D0w
00000DDC: bne.w 0x00000de4
00000DE0: add.w D0w,D2w
00000DE2: bra.b 0x00000dd0
00000DE4: add.w D2w,D0w
00000DE6: cmp.w D4w,D0w
00000DE8: bmi.w 0x00000df2
00000DEC: addq.b #0x1,(0x00fff0dd).l
00000DF2: dbf D6w,0x00000dce
00000DF6: movem.l (SP)+,{ D0 D1 D2 D3 D4}
00000DFA: rts
00000DFC: move SR,-(SP) ; data Stack[-0x2]
00000DFE: move #0x2700,SR
00000E02: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00000E08: lea (0xc00000).l,A2 ; data 00c00000
00000E0E: move.w D2w,(A2)
00000E10: dbf D7w,0x00000e0e
00000E14: move (SP)+,SR
00000E16: rts
00000E18: move SR,-(SP) ; data Stack[-0x2]
00000E1A: move #0x2700,SR
00000E1E: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00000E24: lea (0xc00000).l,A2 ; data 00c00000
00000E2A: move.w (A0)+,(A2)
00000E2C: dbf D7w,0x00000e2a
00000E30: move (SP)+,SR
00000E32: rts
00000E34: move SR,-(SP) ; data Stack[-0x2]
00000E36: move #0x2700,SR
00000E3A: move.w D0w,-(SP) ; data Stack[-0x4]
00000E3C: move.w (0x00ffdd10).l,D0w
00000E42: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
00000E46: move.l #0x0,(0x00c00000).l
00000E50: move.l #0x0,(0x00c00000).l
00000E5A: move.l #0xffd986,(0x00ffdd18).l
00000E64: move.w #0x1,(0x00ffdd1c).l
00000E6C: move.l #0x0,(0x00ffd97e).l
00000E76: move.w (SP)+,D0w
00000E78: move (SP)+,SR
00000E7A: rts
00000E80: andi.w #0x3fff,D1w
00000E84: move.w D1w,(0x00c00004).l
00000E8A: move.w D0w,D1w
00000E8C: rol.w #0x2,D1w
00000E8E: andi.w #0x3,D1w
00000E92: move.w D1w,(0x00c00004).l
00000E98: move.l (SP)+,D1
00000E9A: rts
00000E9C: move.l D1,-(SP) ; data Stack[-0x4]
00000E9E: move.w D0w,D1w
00000EA0: andi.w #0x3fff,D1w
00000EA4: ori.w #0x4000,D1w
00000EA8: move.w D1w,(0x00c00004).l
00000EAE: move.w D0w,D1w
00000EB0: rol.w #0x2,D1w
00000EB2: andi.w #0x3,D1w
00000EB6: move.w D1w,(0x00c00004).l
00000EBC: move.l (SP)+,D1
00000EBE: rts
00000EC0: lea (0xffd87e).l,A0 ; data 00ffd87e
00000EC6: move.l #0xc000,D0
00000ECC: move.w D0w,(0x00c00004).l
00000ED2: move.w #0x0,(0x00c00004).l
00000EDA: move.w (A0)+,(0x00c00000).l
00000EE0: addq.b #0x2,D0b
00000EE2: bpl.b 0x00000ecc
00000EE4: rts
00000EE6: cmpi.w #0x50,(0x00ffdd02).l
00000EEE: bcs.w 0x00000ef8
00000EF2: clr.b (0x00ffdbf9).l
00000EF8: move.w (0x00ffdd02).l,D2w
00000EFE: addq.w #0x1,D2w
00000F00: beq.b 0x00000f16
00000F02: lsl.w #0x3,D2w
00000F04: move.l #0xffd97e,D0
00000F0A: move.w (0x00ffdd10).l,D1w
00000F10: bsr.w 0x00000f98 ; call 00000F98 FUN_00000f98
00000F14: rts
00000F16: bsr.w 0x00000e34 ; call 00000E34 FUN_00000e34
00000F1A: rts
00000F1C: move.w (0x00ffdd12).l,D0w
00000F22: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
00000F26: move.w (0x00ffdd08).l,(0x00c00000).l
00000F30: move.w (0x00ffdd0c).l,(0x00c00000).l
00000F3A: rts
00000F3C: move.l #0x40000010,(0x00c00004).l
00000F46: move.w (0x00ffdd0a).l,(0x00c00000).l
00000F50: move.w (0x00ffdd0e).l,(0x00c00000).l
00000F5A: rts
00000F5C: clr.l D4
00000F5E: move.w D2w,D4w
00000F60: add.l D0,D4
00000F62: eor.l D0,D4
00000F64: btst.l #0x10,D4
00000F68: beq.w 0x00000f98
00000F6C: movem.l {  D2 D1 D0},-(SP)
00000F70: andi.l #0xffff,D2
00000F76: movem.l {  D2 D1 D0},-(SP)
00000F7A: move.w D0w,D2w
00000F7C: neg.w D2w
00000F7E: move.l D2,D4
00000F80: bsr.w 0x00000f98 ; call 00000F98 FUN_00000f98
00000F84: movem.l (SP)+,{ D0 D1 D2}
00000F88: add.l D4,D0
00000F8A: add.l D4,D1
00000F8C: sub.w D4w,D2w
00000F8E: bsr.w 0x00000f98 ; call 00000F98 FUN_00000f98
00000F92: movem.l (SP)+,{ D0 D1 D2}
00000F96: rts
00000F98: move SR,-(SP) ; data Stack[-0x2]
00000F9A: move #0x2700,SR
00000F9E: movem.l {  A1 A0 D0},-(SP)
00000FA2: lea (0xc00000).l,A1 ; data 00c00000
00000FA8: movea.l D0,A0
00000FAA: move.l D1,D0
00000FAC: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
00000FB0: move.w D2w,D0w
00000FB2: lsr.w #0x6,D0w
00000FB4: subq.w #0x1,D0w
00000FB6: bmi.b 0x00000fdc
00000FB8: move.l (A0)+,(A1)
00000FBA: move.l (A0)+,(A1)
00000FBC: move.l (A0)+,(A1)
00000FBE: move.l (A0)+,(A1)
00000FC0: move.l (A0)+,(A1)
00000FC2: move.l (A0)+,(A1)
00000FC4: move.l (A0)+,(A1)
00000FC6: move.l (A0)+,(A1)
00000FC8: move.l (A0)+,(A1)
00000FCA: move.l (A0)+,(A1)
00000FCC: move.l (A0)+,(A1)
00000FCE: move.l (A0)+,(A1)
00000FD0: move.l (A0)+,(A1)
00000FD2: move.l (A0)+,(A1)
00000FD4: move.l (A0)+,(A1)
00000FD6: move.l (A0)+,(A1)
00000FD8: dbf D0w,0x00000fb8
00000FDC: move.w D2w,D0w
00000FDE: lsr.w #0x2,D0w
00000FE0: andi.w #0xf,D0w
00000FE4: subq.w #0x1,D0w
00000FE6: bmi.b 0x00000fee
00000FE8: move.l (A0)+,(A1)
00000FEA: dbf D0w,0x00000fe8
00000FEE: btst.l #0x1,D2
00000FF2: beq.b 0x00000ff6
00000FF4: move.w (A0)+,(A1)
00000FF6: movem.l (SP)+,{ D0 A0 A1}
00000FFA: move (SP)+,SR
00000FFC: rts
00000FFE: move SR,-(SP) ; data Stack[-0x2]
00001000: move #0x2700,SR
00001004: bsr.w 0x00001056 ; call 00001056 FUN_00001056
00001008: move.w (0x00ffdd16).l,D0w
0000100E: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
00001012: move.l #0x7ff07ff,D0
00001018: move.w #0x3ff,D2w
0000101C: move.l D0,(0x00c00000).l
00001022: dbf D2w,0x0000101c
00001026: move (SP)+,SR
00001028: rts
0000102A: move SR,-(SP) ; data Stack[-0x2]
0000102C: move #0x2700,SR
00001030: bsr.w 0x00001056 ; call 00001056 FUN_00001056
00001034: move.w (0x00ffdd14).l,D0w
0000103A: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000103E: move.l #0x7ff07ff,D0
00001044: move.w #0x3ff,D1w
00001048: move.l D0,(0x00c00000).l
0000104E: dbf D1w,0x00001048
00001052: move (SP)+,SR
00001054: rts
00001056: move.w #-0x20,D0w
0000105A: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000105E: clr.w D0w
00001060: move.w #0xf,D1w
00001064: move.w D0w,(0x00c00000).l
0000106A: dbf D1w,0x00001064
0000106E: rts
00001070: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00001074: jsr 0x00001872.l ; call 00001872 FUN_00001872
0000107A: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0000107E: rte
00001080: jsr 0x00001910.l ; call 00001910 FUN_00001910
00001086: rte
00001088: lea (0xffd87e).l,A0 ; data 00ffd87e
0000108E: move.w #0x3f,D7w
00001092: move.w (A0),D0w
00001094: move.w D0w,D1w
00001096: andi.w #-0x1120,D1w
0000109A: andi.w #0xe,D0w
0000109E: beq.w 0x000010a4
000010A2: subq.w #0x2,D0w
000010A4: or.w D1w,D0w
000010A6: move.w D0w,D1w
000010A8: andi.w #-0x11f2,D1w
000010AC: andi.w #0xe0,D0w
000010B0: beq.w 0x000010b8
000010B4: subi.w #0x20,D0w
000010B8: or.w D1w,D0w
000010BA: move.w D0w,D1w
000010BC: andi.w #-0x1f12,D1w
000010C0: andi.w #0xe00,D0w
000010C4: beq.w 0x000010cc
000010C8: subi.w #0x200,D0w
000010CC: or.w D1w,D0w
000010CE: move.w D0w,(A0)+
000010D0: dbf D7w,0x00001092
000010D4: rts
000010D6: lea (0xffd87e).l,A0 ; data 00ffd87e
000010DC: lea (0xffd8fe).l,A1 ; data 00ffd8fe
000010E2: move.w #0x3f,D7w
000010E6: move.w (A0),D0w
000010E8: move.w D0w,D1w
000010EA: andi.w #-0x1120,D1w
000010EE: andi.w #0xe,D0w
000010F2: move.w (A1),D2w
000010F4: andi.w #0xe,D2w
000010F8: cmp.w D2w,D0w
000010FA: beq.w 0x00001100
000010FE: addq.w #0x2,D0w
00001100: or.w D1w,D0w
00001102: move.w D0w,D1w
00001104: andi.w #-0x11f2,D1w
00001108: andi.w #0xe0,D0w
0000110C: move.w (A1),D2w
0000110E: andi.w #0xe0,D2w
00001112: cmp.w D2w,D0w
00001114: beq.w 0x0000111c
00001118: addi.w #0x20,D0w
0000111C: or.w D1w,D0w
0000111E: move.w D0w,D1w
00001120: andi.w #-0x1f12,D1w
00001124: andi.w #0xe00,D0w
00001128: move.w (A1)+,D2w
0000112A: andi.w #0xe00,D2w
0000112E: cmp.w D2w,D0w
00001130: beq.w 0x00001138
00001134: addi.w #0x200,D0w
00001138: or.w D1w,D0w
0000113A: move.w D0w,(A0)+
0000113C: dbf D7w,0x000010e6
00001140: rts
00001182: rts
00001184: movem.l {  D3 D2 D1 D0},-(SP)
00001188: move SR,-(SP) ; data Stack[-0x12]
0000118A: move #0x2700,SR
0000118E: tst.w D1w
00001190: beq.w 0x0000119c
00001194: move.w #-0x7080,(0x00c00004).l
0000119C: lea (0xc00000).l,A0 ; data 00c00000
000011A2: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
000011A6: move.w (A1)+,D0w
000011A8: or.w D2w,D0w
000011AA: move.w D0w,(A0)
000011AC: dbf D3w,0x000011a6
000011B0: move.w #-0x70fe,(0x00c00004).l
000011B8: move (SP)+,SR
000011BA: movem.l (SP)+,{ D0 D1 D2 D3}
000011BE: rts
000011C0: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
000011C4: move SR,-(SP) ; data Stack[-0x3e]
000011C6: move #0x2700,SR
000011CA: lea (0xc00000).l,A2 ; data 00c00000
000011D0: move.l D4,D0
000011D2: move.w D7w,D3w
000011D4: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
000011D8: move.w (A1),D0w
000011DA: or.w D2w,D0w
000011DC: move.w D0w,(A2)
000011DE: dbf D7w,0x000011d8
000011E2: addi.w #0x80,D4w
000011E6: move.l D4,D0
000011E8: move.w D3w,D7w
000011EA: dbf D1w,0x000011d4
000011EE: move (SP)+,SR
000011F0: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
000011F4: rts
000011F6: move SR,-(SP) ; data Stack[-0x2]
000011F8: move #0x2700,SR
0000120E: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
00001212: move.w (A1)+,D0w
00001214: or.w D2w,D0w
00001216: move.w D0w,(A0)
00001218: dbf D7w,0x00001212
0000121C: addq.w #0x2,D4w
0000121E: move.l D4,D0
00001220: move.w D3w,D7w
00001222: dbf D1w,0x0000120e
00001226: move.w #-0x70fe,(0x00c00004).l
0000122E: move (SP)+,SR
00001230: rts
00001232: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00001236: move SR,-(SP) ; data Stack[-0x3e]
00001238: move #0x2700,SR
0000123C: lea (0xc00000).l,A2 ; data 00c00000
00001242: move.l D4,D0
00001244: move.w D7w,D3w
00001246: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000124A: move.w (A1)+,D0w
0000124C: or.w D2w,D0w
0000124E: move.w D0w,(A2)
00001250: dbf D7w,0x0000124a
00001254: addi.w #0x80,D4w
00001258: move.l D4,D0
0000125A: move.w D3w,D7w
0000125C: dbf D1w,0x00001246
00001260: move (SP)+,SR
00001262: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00001266: rts
000012F2: move (SP)+,SR
000012F4: rts
000012F6: move.w #0x10,D6w
000012FA: move.w D6w,-(SP) ; data Stack[-0x2]
000012FC: move.w #0x1,D5w
00001300: jsr 0x00001342.l ; call 00001342 wait_for_palette_frame
00001306: jsr 0x000010d6.l ; call 000010D6 step_palette_toward_target
0000130C: bset.b #0x0,(0x00ffef14).l
00001314: move.w (SP)+,D6w
00001316: dbf D6w,0x000012fa
0000131A: rts
0000131C: move.w #0x10,D6w
00001320: move.w D6w,-(SP) ; data Stack[-0x2]
00001322: move.w #0x1,D5w
00001326: jsr 0x00001342.l ; call 00001342 wait_for_palette_frame
0000132C: jsr 0x00001088.l ; call 00001088 step_palette_toward_black
00001332: bset.b #0x0,(0x00ffef14).l
0000133A: move.w (SP)+,D6w
0000133C: dbf D6w,0x00001320
00001340: rts
00001342: move.w D5w,-(SP) ; data Stack[-0x2]
00001344: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000134A: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00001350: move.w (SP)+,D5w
00001352: dbf D5w,0x00001342
00001356: rts
00001358: lea (0xffd87e).l,A1 ; data 00ffd87e
0000135E: moveq #0x1f,D0
00001360: move.l (A0)+,(A1)+
00001362: dbf D0w,0x00001360
00001366: rts
00001368: lea (0xffd8fe).l,A1 ; data 00ffd8fe
0000136E: moveq #0x1f,D0
00001370: move.l (A0)+,(A1)+
00001372: dbf D0w,0x00001370
00001376: rts
00001378: move.w (A2)+,D0w
0000137A: cmpi.w #-0x8000,D0w
0000137E: bne.w 0x00001384
00001382: rts
00001384: add.w D1w,D0w
00001386: move.w D0w,(0x6,A3)
0000138A: move.w (A2)+,D0w
0000138C: add.w D2w,D0w
0000138E: move.w D0w,(A3)
00001390: move.w (A2)+,D0w
00001392: move.b D0b,(0x2,A3)
00001396: move.b D5b,(0x3,A3)
0000139A: move.w (A2)+,D0w
0000139C: or.w D6w,D0w
0000139E: move.w D0w,(0x4,A3)
000013A2: addq.w #0x1,D5w
000013A4: addq.w #0x1,(0x00ffdd02).l
000013AA: addq.l #0x8,A3
000013AC: move.l A3,(0x00ffdd18).l
000013B2: move.w (0x00ffdd02).l,(0x00ffdd1c).l
000013BC: bra.b 0x00001378
000013BE: move.w (A2)+,D0w
000013C0: cmpi.w #-0x8000,D0w
000013C4: bne.w 0x000013ca
000013C8: rts
000013CA: add.w D1w,D0w
000013CC: move.w D0w,(0x6,A3)
000013D0: move.w (A2)+,D0w
000013D2: add.w D2w,D0w
000013D4: move.w D0w,(A3)
000013D6: move.w (A2)+,D0w
000013D8: move.b D0b,(0x2,A3)
000013DC: move.b D5b,(0x3,A3)
000013E0: move.w (A2)+,D0w
000013E2: add.w D4w,D0w
000013E4: or.w D6w,D0w
000013E6: move.w D0w,(0x4,A3)
000013EA: addq.w #0x1,D5w
000013EC: addq.w #0x1,(0x00ffdd02).l
000013F2: addq.l #0x8,A3
000013F4: move.l A3,(0x00ffdd18).l
000013FA: move.w (0x00ffdd02).l,(0x00ffdd1c).l
00001404: bra.b 0x000013be
00001406: bsr.w 0x0000131c ; call 0000131C step_palette_transition
0000140A: movea.l (A6)+,A0
0000140C: jsr 0x00000cec.l ; call 00000CEC FUN_00000cec
00001412: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
00001418: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0000141E: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00001424: movea.l (A6)+,A0
00001426: jsr 0x00001368.l ; call 00001368 copy_target_palette
0000142C: move.w (A6)+,(0x00ffdd08).l
00001432: move.w (A6)+,(0x00ffdd0c).l
00001438: move.w (A6)+,(0x00ffdd0a).l
0000143E: move.w (A6)+,(0x00ffdd0e).l
00001444: move.l (A6)+,D0
00001446: beq.w 0x00001476
0000144A: btst.l #0x0,D0
0000144E: beq.w 0x0000146a
00001452: bclr.l #0x0,D0
00001456: movea.l D0,A3
00001458: lea (0xff9700).l,A1 ; data 00ff9700
0000145E: move.l (A6)+,D1
00001460: move.l (A6)+,D2
00001462: move.w D1w,D0w
00001464: bsr.w 0x00001530 ; call 00001530 FUN_00001530
00001468: bra.b 0x00001444
0000146A: move.l (A6)+,D1
0000146C: move.l (A6)+,D2
0000146E: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00001474: bra.b 0x00001444
00001476: move.l (A6)+,D0
00001478: beq.w 0x000014a4
0000147C: movea.l D0,A1
0000147E: move.l (A6)+,D4
00001480: move.w (A6)+,D7w
00001482: move.w (A6)+,D1w
00001484: move.w (A6)+,D2w
00001486: bclr.l #0x0,D0
0000148A: beq.w 0x0000149c
0000148E: movea.l D0,A1
00001490: clr.w D3w
00001492: move.w D7w,D5w
00001494: addq.w #0x1,D5w
00001496: bsr.w 0x00001728 ; call 00001728 FUN_00001728
0000149A: bra.b 0x00001476
0000149C: jsr 0x00001232.l ; call 00001232 FUN_00001232
000014A2: bra.b 0x00001476
000014A4: nop
000014A6: nop
000014A8: nop
000014AA: bset.b #0x0,(0x00ffef14).l
000014B2: bset.b #0x3,(0x00ffef14).l
000014BA: bset.b #0x2,(0x00ffef14).l
000014C2: nop
000014C4: nop
000014C6: nop
000014C8: jmp 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000014CE: movea.l #0x200000,A0
000014D4: add.w D0w,D0w
000014D6: adda.w D0w,A0
000014D8: adda.w #0x1,A0
000014DC: move.b (A0),(A1)+ ; data 00200001
000014DE: adda.w #0x2,A0
000014E2: dbf D7w,0x000014dc
000014E6: rts
00001502: movea.l #0x200000,A1
00001508: add.w D0w,D0w
0000150A: adda.w D0w,A1
0000150C: adda.w #0x1,A1
00001510: movea.l #0x200000,A0
00001516: swap D0
00001518: add.w D0w,D0w
0000151A: adda.w D0w,A0
0000151C: adda.w #0x1,A0
00001520: move.b (A0),(A1) ; data 00200001
00001522: adda.w #0x2,A0
00001526: adda.w #0x2,A1
0000152A: dbf D7w,0x00001520
0000152E: rts
00001530: movem.l {  A3 D7 D6 D5 D4 D3 D2 D1},-(SP)
00001534: clr.w D6w
00001536: move.w (A3)+,D2w
00001538: move.w D2w,-(SP) ; data Stack[-0x22]
0000153A: move SR,-(SP) ; data Stack[-0x24]
0000153C: move #0x2700,SR
00001540: move.w #-0x70fe,(0x00c00004).l
00001548: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000154C: moveq #0x1,D1
0000154E: subq.w #0x1,D1w
00001550: bne.b 0x00001556
00001552: move.b (A3)+,D0b
00001554: moveq #0x8,D1
00001556: add.b D0b,D0b
00001558: bcs.b 0x0000157e
0000155A: move.b (A3)+,D7b
0000155C: move.b D7b,(0x0,A1,D6w*0x1)
00001560: addq.w #0x1,D6w
00001562: andi.w #0x7ff,D6w
00001566: btst.l #0x0,D2
0000156A: bne.b 0x00001574
0000156C: move.w D7w,(0x00c00000).l
00001572: bra.b 0x00001576
00001574: asl.w #0x8,D7w
00001576: dbf D2w,0x0000154e
0000157A: bra.w 0x00001604
0000157E: move.b (A3)+,D3b
00001580: bmi.b 0x000015b4
00001582: andi.w #0x7f,D3w
00001586: addq.w #0x2,D3w
00001588: move.b (A3)+,D4b
0000158A: move.b D4b,D7b
0000158C: move.b D7b,(0x0,A1,D6w*0x1)
00001590: addq.w #0x1,D6w
00001592: andi.w #0x7ff,D6w
00001596: btst.l #0x0,D2
0000159A: bne.b 0x000015a4
0000159C: move.w D7w,(0x00c00000).l
000015A2: bra.b 0x000015a6
000015A4: asl.w #0x8,D7w
000015A6: subq.w #0x1,D2w
000015A8: dbf D3w,0x0000158a
000015AC: addq.w #0x1,D2w
000015AE: dbf D2w,0x0000154e
000015B2: bra.b 0x00001604
000015B4: move.b (A3)+,D4b
000015B6: lsl.w #0x3,D4w
000015B8: move.b D3b,D5b
000015BA: lsr.b #0x4,D5b
000015BC: andi.w #0x7,D5w
000015C0: or.b D5b,D4b
000015C2: andi.w #0x7ff,D4w
000015C6: andi.w #0xf,D3w
000015CA: addq.w #0x2,D3w
000015CC: neg.w D4w
000015CE: add.w D6w,D4w
000015D0: andi.w #0x7ff,D4w
000015D4: move.b (0x0,A1,D4w*0x1),D7b
000015D8: addq.w #0x1,D4w
000015DA: andi.w #0x7ff,D4w
000015DE: move.b D7b,(0x0,A1,D6w*0x1)
000015E2: addq.w #0x1,D6w
000015E4: andi.w #0x7ff,D6w
000015E8: btst.l #0x0,D2
000015EC: bne.b 0x000015f6
000015EE: move.w D7w,(0x00c00000).l
000015F4: bra.b 0x000015f8
000015F6: asl.w #0x8,D7w
000015F8: subq.w #0x1,D2w
000015FA: dbf D3w,0x000015d4
000015FE: addq.w #0x1,D2w
00001600: dbf D2w,0x0000154e
00001604: move (SP)+,SR
00001606: move.w (SP)+,D0w
00001608: addq.w #0x1,D0w
0000160A: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A3}
0000160E: rts
00001610: movem.l {  A3 A2 A1 D5 D4 D3 D2 D1},-(SP)
00001614: move.w (A3)+,D2w
00001616: move.w D2w,-(SP) ; data Stack[-0x22]
00001618: moveq #0x1,D1
0000161A: subq.w #0x1,D1w
0000161C: bne.b 0x00001622
0000161E: move.b (A3)+,D0b
00001620: moveq #0x8,D1
00001622: add.b D0b,D0b
00001624: bcs.b 0x0000162e
00001626: move.b (A3)+,(A1)+
00001628: dbf D2w,0x0000161a
0000162C: bra.b 0x00001670
0000162E: move.b (A3)+,D3b
00001630: bmi.b 0x00001648
00001632: andi.w #0x7f,D3w
00001636: addq.w #0x2,D3w
00001638: sub.w D3w,D2w
0000163A: move.b (A3)+,D4b
0000163C: move.b D4b,(A1)+
0000163E: dbf D3w,0x0000163c
00001642: dbf D2w,0x0000161a
00001646: bra.b 0x00001670
00001648: move.b (A3)+,D4b
0000164A: lsl.w #0x3,D4w
0000164C: move.b D3b,D5b
0000164E: lsr.b #0x4,D5b
00001650: andi.w #0x7,D5w
00001654: or.b D5b,D4b
00001656: andi.w #0x7ff,D4w
0000165A: andi.w #0xf,D3w
0000165E: addq.w #0x2,D3w
00001660: sub.w D3w,D2w
00001662: movea.l A1,A2
00001664: suba.w D4w,A2
00001666: move.b (A2)+,(A1)+
00001668: dbf D3w,0x00001666
0000166C: dbf D2w,0x0000161a
00001670: movem.w (SP)+,{ D0}
00001674: addq.w #0x1,D0w
00001676: movem.l (SP)+,{  D1 D2 D3 D4 D5 A1 A2 A3}
0000167A: rts
0000167C: movem.l {  A1 A0 D3 D2 D1 D0},-(SP)
00001680: move.w (A0)+,D1w
00001682: bra.w 0x0000168e
0000168E: move.b (A0)+,D0b
00001690: beq.w 0x00001722
00001694: cmpi.b #-0x40,D0b
00001698: bhi.w 0x000016bc
0000169C: move.w #0x1,D3w
000016A0: cmpi.b #-0x80,D0b
000016A4: bhi.w 0x000016ec
000016A8: move.w #0x0,D3w
000016AC: cmpi.b #0x40,D0b
000016B0: bhi.w 0x000016ec
000016B4: move.w #-0x1,D3w
000016B8: bra.w 0x000016ec
000016BC: btst.l #0x5,D0
000016C0: bne.w 0x000016d8
000016C4: andi.b #0x1f,D0b
000016C8: andi.w #0x0,D2w
000016CC: move.b (A0)+,D2b
000016CE: add.w D1w,D2w
000016D0: move.w D2w,(A1)+
000016D2: subq.b #0x1,D0b
000016D4: bgt.b 0x000016c8
000016D6: bra.b 0x0000168e
000016D8: andi.b #0x1f,D0b
000016DC: move.b (A0)+,D2b
000016DE: asl.w #0x8,D2w
000016E0: move.b (A0)+,D2b
000016E2: add.w D1w,D2w
000016E4: move.w D2w,(A1)+
000016E6: subq.b #0x1,D0b
000016E8: bgt.b 0x000016dc
000016EA: bra.b 0x0000168e
000016EC: btst.l #0x5,D0
000016F0: bne.w 0x0000170a
000016F4: andi.b #0x1f,D0b
000016F8: andi.w #0x0,D2w
000016FC: move.b (A0)+,D2b
000016FE: add.w D1w,D2w
00001700: move.w D2w,(A1)+
00001702: add.w D3w,D2w
00001704: subq.b #0x1,D0b
00001706: bge.b 0x00001700
00001708: bra.b 0x0000168e
0000170A: andi.b #0x1f,D0b
0000170E: move.b (A0)+,D2b
00001710: asl.w #0x8,D2w
00001712: move.b (A0)+,D2b
00001714: add.w D1w,D2w
00001716: move.w D2w,(A1)+
00001718: add.w D3w,D2w
0000171A: subq.b #0x1,D0b
0000171C: bge.b 0x00001716
0000171E: bra.w 0x0000168e
00001722: movem.l (SP)+,{ D0 D1 D2 D3 A0 A1}
00001726: rts
00001728: move SR,-(SP) ; data Stack[-0x2]
0000172A: move #0x2700,SR
0000172E: move.w (A1)+,D6w
00001730: bra.w 0x0000173c
0000173C: lea (0xc00000).l,A2 ; data 00c00000
00001742: move.l D4,-(SP) ; data Stack[-0x6]
00001744: move.w D7w,-(SP) ; data Stack[-0x8]
00001746: move.l D4,D0
00001748: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000174C: move.w D7w,D0w
0000174E: sub.w D5w,D7w
00001750: movea.w #0x0,A4
00001754: movea.w D3w,A3
00001756: move.b (A1)+,D4b
00001758: beq.w 0x0000186a
0000175C: cmpi.b #-0x40,D4b
00001760: bhi.w 0x00001784
00001764: move.w #0x1,D3w
00001768: cmpi.b #-0x80,D4b
0000176C: bhi.w 0x00001816
00001770: move.w #0x0,D3w
00001774: cmpi.b #0x40,D4b
00001778: bhi.w 0x00001816
0000177C: move.w #-0x1,D3w
00001780: bra.w 0x00001816
00001784: btst.l #0x5,D4
00001788: bne.w 0x000017d0
0000178C: andi.b #0x1f,D4b
00001790: andi.w #0x0,D5w
00001794: move.b (A1)+,D5b
00001796: add.w D6w,D5w
00001798: adda.w #0x1,A4
0000179C: cmpa.w A4w,A3
0000179E: bpl.w 0x000017ca
000017A2: cmp.w D7w,D0w
000017A4: ble.w 0x000017ac
000017A8: or.w D2w,D5w
000017AA: move.w D5w,(A2)
000017AC: dbf D0w,0x000017ca
000017B0: subq.w #0x1,D1w
000017B2: bmi.w 0x0000186a
000017B6: move.l (0x2,SP),D0
000017BA: addi.l #0x80,D0
000017C0: move.l D0,(0x2,SP)
000017C4: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
000017C8: move.w (SP),D0w ; data Stack[-0x8]
000017CA: subq.b #0x1,D4b
000017CC: bgt.b 0x00001790
000017CE: bra.b 0x00001756
000017D0: andi.b #0x1f,D4b
000017D4: move.b (A1)+,D5b
000017D6: asl.w #0x8,D5w
000017D8: move.b (A1)+,D5b
000017DA: add.w D6w,D5w
000017DC: adda.w #0x1,A4
000017E0: cmpa.w A4w,A3
000017E2: bpl.w 0x0000180e
000017E6: cmp.w D7w,D0w
000017E8: ble.w 0x000017f0
000017EC: or.w D2w,D5w
000017EE: move.w D5w,(A2)
000017F0: dbf D0w,0x0000180e
000017F4: subq.w #0x1,D1w
000017F6: bmi.w 0x0000186a
000017FA: move.l (0x2,SP),D0
000017FE: addi.l #0x80,D0
00001804: move.l D0,(0x2,SP)
00001808: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000180C: move.w (SP),D0w ; data Stack[-0x8]
0000180E: subq.b #0x1,D4b
00001810: bgt.b 0x000017d4
00001812: bra.w 0x00001756
00001816: andi.w #0x0,D5w
0000181A: btst.l #0x5,D4
0000181E: beq.w 0x00001826
00001822: move.b (A1)+,D5b
00001824: asl.w #0x8,D5w
00001826: andi.b #0x1f,D4b
0000182A: move.b (A1)+,D5b
0000182C: add.w D6w,D5w
0000182E: adda.w #0x1,A4
00001832: cmpa.w A4w,A3
00001834: bpl.w 0x00001860
00001838: cmp.w D7w,D0w
0000183A: ble.w 0x00001842
0000183E: or.w D2w,D5w
00001840: move.w D5w,(A2)
00001842: dbf D0w,0x00001860
00001846: subq.w #0x1,D1w
00001848: bmi.w 0x0000186a
0000184C: move.l (0x2,SP),D0
00001850: addi.l #0x80,D0
00001856: move.l D0,(0x2,SP)
0000185A: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000185E: move.w (SP),D0w ; data Stack[-0x8]
00001860: add.w D3w,D5w
00001862: subq.b #0x1,D4b
00001864: bge.b 0x0000182e
00001866: bra.w 0x00001756
0000186A: move.w (SP)+,D7w
0000186C: move.l (SP)+,D4
0000186E: move (SP)+,SR
00001870: rts
00001872: addq.l #0x1,(0x00fff9ec).l
00001878: clr.w (0x00fff9f0).l
0000187E: bclr.b #0x1,(0x00ffef14).l
00001886: beq.b 0x00001894
00001888: bsr.w 0x00000ee6 ; call 00000EE6 FUN_00000ee6
0000188C: move.w #0x1,(0x00ffdd02).l
00001894: bclr.b #0x0,(0x00ffef14).l
0000189C: beq.b 0x000018a2
0000189E: bsr.w 0x00000ec0 ; call 00000EC0 FUN_00000ec0
000018A2: tst.w (0x00ffdfe8).l
000018A8: bne.w 0x000018b2
000018AC: jsr 0x0005672c.l ; call 0005672C FUN_0005672c
000018B2: bclr.b #0x2,(0x00ffef14).l
000018BA: beq.b 0x000018c0
000018BC: bsr.w 0x00000f1c ; call 00000F1C FUN_00000f1c
000018C0: bclr.b #0x3,(0x00ffef14).l
000018C8: beq.b 0x000018ce
000018CA: bsr.w 0x00000f3c ; call 00000F3C FUN_00000f3c
000018CE: tst.w (0x00ffe100).l
000018D4: beq.w 0x000018de
000018D8: jsr 0x00002c92.l ; call 00002C92 FUN_00002c92
000018DE: tst.w (0x00ffe102).l
000018E4: beq.w 0x000018ee
000018E8: jsr 0x00002cb2.l ; call 00002CB2 FUN_00002cb2
000018EE: tst.w (0x00ffdf2a).l
000018F4: beq.w 0x000018fe
000018F8: jsr 0x00006148.l ; call 00006148 FUN_00006148
000018FE: tst.w (0x00ffdf30).l
00001904: beq.w 0x0000190e
00001908: jsr 0x000061f4.l ; call 000061F4 FUN_000061f4
0000190E: rts
00001910: movem.l {  A0 D0},-(SP)
00001914: move.l #-0x3fa20000,(0x00c00004).l
0000191E: addq.w #0x1,(0x00fff9f0).l
00001924: btst.b #0x0,(0x00fff9f1).l
0000192C: bne.w 0x0000193a
00001930: lea (0xffd8dc).l,A0 ; data 00ffd8dc
00001936: bra.w 0x00001940
0000193A: lea (0xffd8bc).l,A0 ; data 00ffd8bc
00001940: btst.b #0x2,(0x00c00005).l
00001948: beq.b 0x00001940
0000194A: move.w (A0)+,(0x00c00000).l
00001950: move.w (A0)+,(0x00c00000).l
00001956: movem.l (SP)+,{ D0 A0}
0000195A: rts
0000195C: bsr.w 0x000019f2 ; call 000019F2 read_controller_port
00001960: andi.w #0xff,D0w
00001964: move.w (0x00ffdd20).l,D1w
0000196A: move.w D0w,(0x00ffdd20).l
00001970: eor.w D0w,D1w
00001972: and.w D1w,D0w
00001974: lsl.w #0x8,D0w
00001976: or.w (0x00ffdd20).l,D0w
0000197C: move.w D0w,(0x00ffdd22).l
00001982: andi.w #0xf,D0w
00001986: lsl.w #0x1,D0w
00001988: lea (0x19d2).l,A0 ; data 000019d2
0000198E: move.w (0x0,A0,D0w*0x1),(0x00ffdd04).l
00001996: bra.w 0x000019d0
000019D0: rts
000019F2: jsr 0x00052dc2.l ; call 00052DC2 FUN_00052dc2
000019F8: move.b #0x40,(0x00a10009).l
00001A00: move.b #0x0,(0x00a10003).l
00001A08: move.w #0xa,D1w
00001A0C: dbf D1w,0x00001a0c
00001A10: move.b (0x00a10003).l,D0b
00001A16: asl.b #0x2,D0b
00001A18: andi.b #-0x40,D0b
00001A1C: move.b #0x40,(0x00a10003).l
00001A24: move.w #0xa,D1w
00001A28: dbf D1w,0x00001a28
00001A2C: move.b (0x00a10003).l,D1b
00001A32: andi.b #0x3f,D1b
00001A36: or.b D1b,D0b
00001A38: not.b D0b
00001A3A: move.b D0b,(0x00ffef0e).l
00001A40: jsr 0x00052dd6.l ; call 00052DD6 FUN_00052dd6
00001A46: rts
00001A9E: clr.b (0x00fff101).l
00001AA4: bsr.w 0x00001d7a ; call 00001D7A setup_title_state
00001AA8: move.w #0x0,(0x00ffe00a).l
00001AB0: move.w (0x00ffe00a).l,(0x00ffdd0e).l
00001ABA: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00001AC0: clr.w (0x00ffddc2).l
00001AC6: move.w #0x5a,(0x00ffdd2c).l
00001ACE: clr.w (0x00ffdd2e).l
00001AD4: clr.l (0x00ffdd3c).l
00001ADA: clr.l (0x00ffdd40).l
00001AE0: clr.w (0x00ffdd5a).l
00001AE6: move.w #0x5a,(0x00ffdd5c).l
00001AEE: move.w #0x98,(0x00ffdd42).l
00001AF6: clr.w (0x00ffdd46).l
00001AFC: move.w #0xe10,D7w
00001B00: move.w D7w,-(SP) ; data Stack[-0x2]
00001B02: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00001B08: bsr.w 0x00001c1e ; call 00001C1E advance_title_text_state
00001B0C: bsr.w 0x00002634 ; call 00002634 cycle_title_circuit_lights
00001B10: bsr.w 0x000026d0 ; call 000026D0 advance_title_animation_timer
00001B14: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00001B1A: bsr.w 0x00001bea ; call 00001BEA record_title_input_edge
00001B1E: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00001B24: subq.w #0x1,(0x00ffdd2c).l
00001B2A: bpl.w 0x00001b62
00001B2E: cmpi.w #0x25,(0x00fff1e6).l
00001B36: bgt.w 0x00001b62
00001B3A: subq.w #0x1,(0x00ffdd40).l
00001B40: bpl.w 0x00001b52
00001B44: move.w #0x1,(0x00ffdd40).l
00001B4C: jsr 0x00001e4e.l ; call 00001E4E reveal_title_logo_column
00001B52: bsr.w 0x00001c6a ; call 00001C6A advance_title_sprite_state
00001B56: bset.b #0x1,(0x00ffef14).l
00001B5E: bra.w 0x00001b68
00001B62: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00001B68: move.w (SP)+,D7w
00001B6A: cmpi.w #0xc6c,D7w
00001B6E: bcc.w 0x00001b96
00001B72: move.w D7w,D0w
00001B74: andi.w #0x3f,D0w
00001B78: cmpi.w #0xa,D0w
00001B7C: bcs.w 0x00001b88
00001B80: bsr.w 0x00001dca ; call 00001DCA draw_title_prompt_variant_a
00001B84: bra.w 0x00001b8c
00001B88: bsr.w 0x00001de0 ; call 00001DE0 draw_title_prompt_variant_b
00001B8C: tst.w (0x00ffddc2).l
00001B92: bne.w 0x00001b9a
00001B96: dbf D7w,0x00001b00
00001B9A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00001BA0: jsr 0x0000131c.l ; call 0000131C step_palette_transition
00001BA6: bsr.w 0x00001bac ; call 00001BAC finalize_title_input_sequence
00001BAA: rts
00001BAC: move.b (0x00fff101).l,D0b
00001BB2: ext.w D0w
00001BB4: cmpi.w #0x7,D0w
00001BB8: bne.w 0x00001be2
00001BBC: cmpi.l #0x40101040,(0x00ffcb40).l
00001BC6: bne.w 0x00001be2
00001BCA: cmpi.l #0x20401000,(0x00ffcb44).l
00001BD4: bne.w 0x00001be2
00001BD8: move.b #-0x1,(0x00fff101).l
00001BE0: rts
00001BE2: clr.b (0x00fff101).l
00001BE8: rts
00001BEA: move.w (0x00ffdd22).l,D0w
00001BF0: bclr.l #0xf,D0
00001BF4: lsr.w #0x8,D0w
00001BF6: tst.b D0b
00001BF8: beq.w 0x00001c1c
00001BFC: move.b (0x00fff101).l,D1b
00001C02: ext.w D1w
00001C04: cmpi.w #0x7,D1w
00001C08: bgt.w 0x00001c1c
00001C0C: lea (0xffcb40).l,A0 ; data 00ffcb40
00001C12: adda.w D1w,A0
00001C14: move.b D0b,(A0) ; data 00ffcb40
00001C16: addq.b #0x1,(0x00fff101).l
00001C1C: rts
00001C1E: cmpi.w #0x1,(0x00ffdd5a).l
00001C26: bne.w 0x00001c68
00001C2A: subq.w #0x1,(0x00ffdd5c).l
00001C30: bmi.w 0x00001c52
00001C34: move.w (0x00ffdd5c).l,D0w
00001C3A: andi.w #0x1f,D0w
00001C3E: bne.w 0x00001c68
00001C42: jsr 0x000010d6.l ; call 000010D6 step_palette_toward_target
00001C48: bset.b #0x0,(0x00ffef14).l
00001C50: rts
00001C52: move.w #0x2,(0x00ffdd5a).l
00001C5A: move.w #0x0,D0w
00001C5E: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
00001C64: bsr.w 0x00001df6 ; call 00001DF6 draw_title_copyright
00001C68: rts
00001C6A: move.w #0x1,(0x00ffdd02).l
00001C72: lea (0xffd986).l,A3 ; data 00ffd986
00001C78: move.w #0x2,D5w
00001C7C: subq.w #0x1,(0x00ffdd3c).l
00001C82: bpl.w 0x00001ca6
00001C86: move.w #0x3,(0x00ffdd3c).l
00001C8E: addq.w #0x1,(0x00ffdd3e).l
00001C94: cmpi.w #0x6,(0x00ffdd3e).l
00001C9C: blt.w 0x00001ca6
00001CA0: clr.w (0x00ffdd3e).l
00001CA6: move.w (0x00ffdd3e).l,D0w
00001CAC: add.w D0w,D0w
00001CAE: add.w D0w,D0w
00001CB0: lea (0x1ce4).l,A2 ; data 00001ce4
00001CB6: movea.l (0x0,A2,D0w*0x1),A2
00001CBA: addq.w #0x4,(0x00ffdd42).l
00001CC0: move.w (0x00ffdd42).l,D1w
00001CC6: move.w #0x110,D2w
00001CCA: move.w #0x6000,D6w
00001CCE: jsr 0x00001378.l ; call 00001378 FUN_00001378
00001CD4: subq.w #0x8,A3
00001CD6: clr.b (0x3,A3)
00001CDA: move.w #0x1,(0x00ffd980).l
00001CE2: rts
00001CFC: lea (0xffbf40).l,A1 ; data 00ffbf40
00001D02: movea.l #0x1f310e,A3
00001D08: moveq #0x20,D0
00001D0A: jsr 0x00001530.l ; call 00001530 FUN_00001530
00001D10: move.l #0x2090,(0x00fff1e2).l
00001D1A: move.w #0x0,(0x00fff1e6).l
00001D22: lea (0xffbf40).l,A1 ; data 00ffbf40
00001D28: movea.l #0x67cb0,A3
00001D2E: move.l #0x4b20,D0
00001D34: jsr 0x00001530.l ; call 00001530 FUN_00001530
00001D3A: lea (0xffbf40).l,A1 ; data 00ffbf40
00001D40: movea.l #0x6875c,A3
00001D46: move.l #0x5020,D0
00001D4C: jsr 0x00001530.l ; call 00001530 FUN_00001530
00001D52: bsr.w 0x00001d58 ; call 00001D58 FUN_00001d58
00001D56: rts
00001D58: lea (0x67df6).l,A1 ; data 00067df6
00001D5E: move.w #-0x2000,D4w
00001D62: move.w #0x27,D7w
00001D66: move.w #0x1f,D1w
00001D6A: move.w #0x4000,D2w
00001D6E: clr.w D3w
00001D70: move.w D7w,D5w
00001D72: addq.w #0x1,D5w
00001D74: jmp 0x00001728.l ; call 00001728 FUN_00001728
00001D7A: clr.w (0x00ffdd08).l
00001D80: clr.w (0x00ffdd0c).l
00001D86: clr.w (0x00ffdd0a).l
00001D8C: clr.w (0x00ffdd0e).l
00001D92: lea (0x1fd8).l,A6 ; data 00001fd8
00001D98: jsr 0x00001406.l ; call 00001406 FUN_00001406
00001D9E: jsr 0x00001cfc.l ; call 00001CFC FUN_00001cfc
00001DA4: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
00001DAA: bset.b #0x2,(0x00ffef14).l
00001DB2: bset.b #0x3,(0x00ffef14).l
00001DBA: bset.b #0x0,(0x00ffef14).l
00001DC2: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
00001DC8: rts
00001DCA: movea.l #0xca84,A2
00001DD0: lea (0x1f81).l,A1 ; data 00001f81
00001DD6: move.w #0x2000,D1w
00001DDA: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00001DE0: movea.l #0xca84,A2
00001DE6: lea (0x1f99).l,A1 ; data 00001f99
00001DEC: move.w #0x2000,D1w
00001DF0: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00001DF6: movea.l #0xcb04,A2
00001DFC: lea (0x1f00).l,A1 ; data 00001f00
00001E02: move.w #0x2000,D1w
00001E06: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00001E0C: movea.l #0xcb84,A2
00001E12: lea (0x1f24).l,A1 ; data 00001f24
00001E18: move.w #0x2000,D1w
00001E1C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00001E22: movea.l #0xcc04,A2
00001E28: lea (0x1f48).l,A1 ; data 00001f48
00001E2E: move.w #0x2000,D1w
00001E32: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00001E38: movea.l #0xcc84,A2
00001E3E: lea (0x1f68).l,A1 ; data 00001f68
00001E44: move.w #0x2000,D1w
00001E48: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00001E4E: move.w (0x00fff1e6).l,D0w
00001E54: andi.w #0x1,D0w
00001E58: bne.w 0x00001e66
00001E5C: move.w #0x5d,D0w
00001E60: jsr 0x000530de.l ; call 000530DE FUN_000530de
00001E66: movea.l (0x00fff1e2).l,A1
00001E6C: move.w (0x00fff1e6).l,D4w
00001E72: cmpi.w #0x26,D4w
00001E76: bcc.w 0x00001e9e
00001E7A: lsl.w #0x1,D4w
00001E7C: addi.w #-0x3efe,D4w
00001E80: move.w #0x25,D7w
00001E84: move.w #0x12,D1w
00001E88: move.w #0x0,D2w
00001E8C: jsr 0x0005c8c6.l ; call 0005C8C6 FUN_0005c8c6
00001E92: addq.w #0x1,(0x00fff1e6).l
00001E98: addq.l #0x2,(0x00fff1e2).l
00001E9E: cmpi.w #0x25,(0x00fff1e6).l
00001EA6: bne.w 0x00001ec8
00001EAA: move.w #0x1,(0x00ffdd5a).l
00001EB2: lea (0x2070).l,A0 ; data 00002070
00001EB8: lea (0xffd93e).l,A1 ; data 00ffd93e
00001EBE: move.w #0xf,D7w
00001EC2: move.w (A0)+,(A1)+
00001EC4: dbf D7w,0x00001ec2
00001EC8: rts
00002634: cmpi.w #0x2,(0x00ffdd5a).l
0000263C: beq.w 0x00002642
00002640: rts
00002642: move.w #-0x3fb0,D0w
00002646: lea (0x26f2).l,A0 ; data 000026f2
0000264C: subq.w #0x1,(0x00ffdf90).l
00002652: bpl.w 0x000026ce
00002656: move.w #0x4,(0x00ffdf90).l
0000265E: addq.w #0x1,(0x00ffdf92).l
00002664: cmpi.w #0x7,(0x00ffdf92).l
0000266C: bcs.w 0x00002676
00002670: clr.w (0x00ffdf92).l
00002676: move.w (0x00ffdf92).l,D1w
0000267C: lsl.w #0x1,D1w
0000267E: move.w D0w,(0x00c00004).l
00002684: move.w #0x0,(0x00c00004).l
0000268C: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
00002694: move.w (0x2,A0,D1w*0x1),(0x00c00000).l
0000269C: move.w (0x4,A0,D1w*0x1),(0x00c00000).l
000026A4: move.w (0x6,A0,D1w*0x1),(0x00c00000).l
000026AC: move.w (0x8,A0,D1w*0x1),(0x00c00000).l
000026B4: move.w (0xa,A0,D1w*0x1),(0x00c00000).l
000026BC: move.w (0xc,A0,D1w*0x1),(0x00c00000).l
000026C4: move.w (0xe,A0,D1w*0x1),(0x00c00000).l
000026CC: rts
000026CE: rts
000026D0: subq.w #0x1,(0x00ffdd2e).l
000026D6: bpl.w 0x000026f0
000026DA: move.w #0x4,(0x00ffdd2e).l
000026E2: addq.w #0x1,(0x00ffdd0e).l
000026E8: bset.b #0x3,(0x00ffef14).l
000026F0: rts
0000281E: bsr.w 0x000028aa ; call 000028AA FUN_000028aa
00002822: clr.w (0x00ffdf8e).l
00002828: move.w #-0x1,(0x00ffdf8a).l
00002830: move.w #-0x1,(0x00ffe172).l
00002838: bsr.w 0x00003074 ; call 00003074 FUN_00003074
0000283C: bra.w 0x00002852
00002852: bsr.w 0x00002900 ; call 00002900 FUN_00002900
00002856: bsr.w 0x00002b34 ; call 00002B34 FUN_00002b34
0000285A: movea.l (0x00ffddfa).l,A6
00002860: bsr.w 0x00002c6c ; call 00002C6C FUN_00002c6c
00002864: bsr.w 0x0000301e ; call 0000301E FUN_0000301e
00002868: bsr.w 0x0000286e ; call 0000286E FUN_0000286e
0000286C: rts
0000286E: move.l (0x00fffc3c).l,D4
00002874: bne.w 0x0000287a
00002878: rts
0000287A: lea (0xff0400).l,A1 ; data 00ff0400
00002880: move.w #0xc,D7w
00002884: btst.b #0x0,(0x10,A1)
0000288A: beq.w 0x0000289a
0000288E: btst.b #0x7,(0xb0,A1)
00002894: beq.w 0x0000289a
00002898: rts
0000289A: adda.w #0x100,A1
0000289E: dbf D7w,0x00002884
000028A2: movea.l D4,A6
000028A4: jmp 0x0001e4ac.l ; call 0001E4AC FUN_0001e4ac
000028AA: jsr 0x00003404.l ; call 00003404 FUN_00003404
000028B0: jsr 0x0000334c.l ; call 0000334C FUN_0000334c
000028B6: jsr 0x000039f2.l ; call 000039F2 FUN_000039f2
000028BC: jmp 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
000028C2: move.w #-0x1,(0x00ffdf8a).l
000028CA: bsr.w 0x00002900 ; call 00002900 FUN_00002900
000028CE: bsr.b 0x0000286e ; call 0000286E FUN_0000286e
000028D0: bsr.b 0x000028aa ; call 000028AA FUN_000028aa
000028D2: andi.w #-0x80,(0x00ffddc6).l
000028DA: andi.w #-0x80,(0x00ffddc8).l
000028E2: move.w #-0x1,(0x00ffdf36).l
000028EA: move.w #-0x1,(0x00ffdf38).l
000028F2: movea.l (0x00ffddfa).l,A6
000028F8: bsr.w 0x00002c6c ; call 00002C6C FUN_00002c6c
000028FC: bra.w 0x0000301e ; call 0000301E FUN_0000301e
00002900: tst.b (0x00ffe172).l
00002906: beq.w 0x0000291c
0000290A: clr.b (0x00ffe172).l
00002910: move.w (0x00ffde06).l,D0w
00002916: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0000291C: tst.w (0x00ffdf8a).l
00002922: beq.w 0x000029bc
00002926: lea (0xffdc9e).l,A0 ; data 00ffdc9e
0000292C: tst.w (0x2,A0)
00002930: beq.w 0x0000293a
00002934: jsr 0x00004b6c.l ; call 00004B6C FUN_00004b6c
0000293A: clr.b (0x00ffe16f).l
00002940: tst.b (0x00ffe191).l
00002946: beq.w 0x00002956
0000294A: clr.b (0x00ffe191).l
00002950: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
00002956: jsr 0x0001a310.l ; call 0001A310 FUN_0001a310
0000295C: jsr 0x0001a322.l ; call 0001A322 FUN_0001a322
00002962: jsr 0x00016dee.l ; call 00016DEE FUN_00016dee
00002968: jsr 0x00004388.l ; call 00004388 FUN_00004388
0000296E: cmpi.b #0x3,(0x00ffe186).l
00002976: bne.w 0x00002986
0000297A: cmpi.b #0x2,(0x00ffe186).l
00002982: bne.w 0x0000298c
00002986: jsr 0x0005d260.l ; call 0005D260 FUN_0005d260
0000298C: tst.b (0x00ffe171).l
00002992: beq.w 0x000029a6
00002996: clr.b (0x00ffe171).l
0000299C: clr.w (0x00ffdf8a).l
000029A2: bra.w 0x000029aa
000029A6: bsr.w 0x0000313a ; call 0000313A FUN_0000313a
000029AA: cmpi.w #0x2,(0x00ffdf40).l
000029B2: bne.w 0x000029bc
000029B6: jsr 0x00016e5c.l ; call 00016E5C FUN_00016e5c
000029BC: jsr 0x0000ee10.l ; call 0000EE10 rebuild_terrain_collision_map
000029C2: cmpi.b #0x2,(0x00ffe186).l
000029CA: blt.w 0x000029d4
000029CE: jsr 0x0005ba50.l ; call 0005BA50 FUN_0005ba50
000029D4: cmpi.w #0x2,(0x00ffdf40).l
000029DC: bne.w 0x000029ea
000029E0: tst.w (0x00ffe118).l
000029E6: beq.w 0x000029ea
000029EA: jsr 0x000164dc.l ; call 000164DC FUN_000164dc
000029F0: jsr 0x000164a8.l ; call 000164A8 FUN_000164a8
000029F6: jsr 0x00013874.l ; call 00013874 FUN_00013874
000029FC: move.l (0x00ffde22).l,D0
00002A02: bclr.l #0x0,D0
00002A06: beq.w 0x00002a2c
00002A0A: movea.l D0,A3
00002A0C: lea (0xff6700).l,A1 ; data 00ff6700
00002A12: jsr 0x00001610.l ; call 00001610 decode_source_byte_stream
00002A18: move.l #0xff6700,(0x00ffddde).l
00002A22: move.l #0xff6700,(0x00ffdde6).l
00002A2C: jmp 0x00005b7e.l ; call 00005B7E mark_entity_slots_for_render
00002A32: move.l #0x2aea,(0x00ffe114).l
00002A3C: move.w #0x0,D0w
00002A40: bra.w 0x00002a52
00002A44: move.l #0x2b24,(0x00ffe114).l
00002A4E: move.w #0x2,D0w
00002A52: jsr 0x00053126.l ; call 00053126 FUN_00053126
00002A58: clr.l (0x00ffe12e).l
00002A5E: clr.b (0x00ffe18f).l
00002A64: clr.b (0x00ffe180).l
00002A6A: clr.b (0x00ffe182).l
00002A70: clr.b (0x00ffe184).l
00002A76: clr.b (0x00ffe187).l
00002A7C: clr.b (0x00ffe18f).l
00002A82: clr.b (0x00ffe183).l
00002A88: clr.w (0x00ffdf84).l
00002A8E: jmp 0x000136de.l ; call 000136DE FUN_000136de
00002A94: jsr 0x00053126.l ; call 00053126 FUN_00053126
00002B34: tst.l (0x00ffe114).l
00002B3A: bne.w 0x00002b78
00002B3E: lea (0xff0100).l,A0 ; data 00ff0100
00002B44: move.w (0x00ffde08).l,(A0)
00002B4A: move.w (0x00ffde0a).l,(0x4,A0)
00002B52: adda.w #0x100,A0
00002B56: move.w (0x00ffde0c).l,(A0)
00002B5C: move.w (0x00ffde0e).l,(0x4,A0)
00002B64: adda.w #0x100,A0
00002B68: move.w (0x00ffde10).l,(A0)
00002B6E: move.w (0x00ffde12).l,(0x4,A0)
00002B76: rts
00002B78: movea.l (0x00ffe114).l,A1
00002B7E: clr.l (0x00ffe114).l
00002B84: move.w (A1)+,(0x00ffddc6).l
00002B8A: move.w (A1)+,(0x00ffddc8).l
00002B90: andi.w #-0x80,(0x00ffddc6).l
00002B98: andi.w #-0x80,(0x00ffddc8).l
00002BA0: move.w #-0x1,(0x00ffdf36).l
00002BA8: move.w #-0x1,(0x00ffdf38).l
00002BB0: lea (0xff0100).l,A0 ; data 00ff0100
00002BB6: move.w (A1)+,(A0)
00002BB8: move.w (A1)+,(0x4,A0)
00002BBC: adda.w #0x100,A0
00002BC0: move.w (A1)+,(A0)
00002BC2: move.w (A1)+,(0x4,A0)
00002BC6: adda.w #0x100,A0
00002BCA: move.w (A1)+,(A0)
00002BCC: move.w (A1)+,(0x4,A0)
00002BD0: rts
00002BD2: jsr 0x0000131c.l ; call 0000131C step_palette_transition
00002BD8: tst.w (0x00ffdf8e).l
00002BDE: bne.w 0x00002822
00002BE2: andi.w #-0x80,(0x00ffddc6).l
00002BEA: andi.w #-0x80,(0x00ffddc8).l
00002BF2: move.w #-0x1,(0x00ffdf36).l
00002BFA: move.w #-0x1,(0x00ffdf38).l
00002C02: bsr.w 0x00002900 ; call 00002900 FUN_00002900
00002C06: bsr.w 0x0000286e ; call 0000286E FUN_0000286e
00002C0A: move.w (0x00ffe11a).l,D0w
00002C10: beq.w 0x00002c38
00002C14: clr.w (0x00ffe11a).l
00002C1A: tst.b (0x00ffe16e).l
00002C20: bpl.w 0x00002c38
00002C24: lea (0xff1700).l,A0 ; data 00ff1700
00002C2A: move.b (0x0,A0,D0w*0x1),D1b ; data 00ff1700
00002C2E: andi.b #-0x20,D1b
00002C32: addq.b #0x1,D1b
00002C34: move.b D1b,(0x0,A0,D0w*0x1) ; data 00ff1700
00002C38: movea.l (0x00ffddfa).l,A6
00002C3E: bsr.w 0x00002c6c ; call 00002C6C FUN_00002c6c
00002C42: bra.w 0x0000301e ; call 0000301E FUN_0000301e
00002C6C: jsr 0x00001406.l ; call 00001406 FUN_00001406
00002C72: jsr 0x000566c2.l ; call 000566C2 FUN_000566c2
00002C78: cmpi.w #0x2,(0x00ffdf40).l
00002C80: bne.w 0x00002c8a
00002C84: jsr 0x00016afa.l ; call 00016AFA FUN_00016afa
00002C8A: bsr.w 0x00002c92 ; call 00002C92 FUN_00002c92
00002C8E: bra.w 0x00002cb2 ; call 00002CB2 FUN_00002cb2
00002C92: clr.w (0x00ffe100).l
00002C98: move.w (0x00ffdd14).l,D5w
00002C9E: movea.l (0x00ffddde).l,A0
00002CA4: lea (0xff2700).l,A1 ; data 00ff2700
00002CAA: jsr 0x00003d18.l ; call 00003D18 FUN_00003d18
00002CB0: rts
00002CB2: clr.w (0x00ffe102).l
00002CB8: move.w (0x00ffdd16).l,D5w
00002CBE: movea.l (0x00ffdde6).l,A0
00002CC4: lea (0xff4700).l,A1 ; data 00ff4700
00002CCA: move.w #-0x8000,(0x00ffdf58).l
00002CD2: jsr 0x00003d18.l ; call 00003D18 FUN_00003d18
00002CD8: clr.w (0x00ffdf58).l
00002CDE: rts
00002CE0: lea (0x2e72).l,A1 ; data 00002e72
00002CE6: move.b (0x00fffbf9).l,D0b
00002CEC: subq.b #0x8,D0b
00002CEE: ext.w D0w
00002CF0: move.w (0x00fffc10).l,D1w
00002CF6: lsl.w #0x2,D1w
00002CF8: adda.w D1w,A1
00002CFA: move.b (0x0,A1,D0w*0x1),D2b
00002CFE: ext.w D2w
00002D00: move.w D2w,(0x00fffc10).l
00002D06: cmpi.w #0x13,D2w
00002D0A: bne.w 0x00002d3c
00002D0E: move.l (0x00fffc1e).l,D6
00002D14: btst.l #0x8,D6
00002D18: bne.w 0x00002d3c
00002D1C: move.l #0x53ee2,(0x00ffdd2c).l
00002D26: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
00002D2C: move.l #0x1e552,(0x00ffe120).l
00002D36: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00002D3C: add.w D2w,D2w
00002D3E: lea (0x2e42).l,A1 ; data 00002e42
00002D44: move.w (0x0,A1,D2w*0x1),(0x00ffddc4).l
00002D4C: lea (0x2ed2).l,A1 ; data 00002ed2
00002D52: adda.w D1w,A1
00002D54: move.b (0x0,A1,D0w*0x1),D0b
00002D58: lsl.w #0x2,D0w
00002D5A: move.w (0x00ffddc4).l,D2w
00002D60: cmpi.w #0x2c,D2w
00002D64: bge.w 0x00002d70
00002D68: subi.w #0x6,D2w
00002D6C: bra.w 0x00002d76
00002D70: subi.w #0x2c,D2w
00002D74: addq.w #0x4,D2w
00002D76: lsl.w #0x4,D2w
00002D78: lea (0x2d92).l,A1 ; data 00002d92
00002D7E: adda.w D2w,A1
00002D80: move.w (0x0,A1,D0w*0x1),(0x00ffe10a).l
00002D88: move.w (0x2,A1,D0w*0x1),(0x00ffe10c).l
00002D90: rts
00002DEE: ori.b #0x0,D0b
00002DF2: addi.b #0x0,D0b
00002DF6: ori.b #0x0,D0b
00002DFA: ori.b #0x0,D0b
00002DFE: ori.b #0x0,D0b
00002E02: bclr.l D2,D0
00002E04: move.l D0,D4
00002E06: addq.l #0x7,D0
00002E08: move.b D0b,-(A1)
00002E0A: ori.b #0x0,D0b
00002E0E: ori.b #0x0,D0b
00002E12: move.b D0b,D7b
00002E14: moves.b D0b,D0
00002E18: move.l D0,D4
00002E1A: ori.b #0x0,D0b
00002E1E: ori.b #0x0,D0b
00002E22: addq.l #0x7,D0
00002E24: move.l D0,-(A1) ; data fffffffb
00002E26: ori.b #0x0,D0b
00002E2A: ori.b #0x0,D0b
00002E2E: ori.b #0x0,D0b
00002E32: bra.w 0x000053b4
00002F32: cmpi.w #0x2,(0x00ffdf40).l
00002F3A: bne.w 0x00002f48
00002F3E: move.l (0x00fffc22).l,D0
00002F44: bclr.l #0xa,D0
00002F48: jsr 0x0000c7f8.l ; call 0000C7F8 FUN_0000c7f8
00002F4E: clr.w (0x00ffdf40).l
00002F54: move.w #-0x1,(0x00ffdf8a).l
00002F5C: lea (0xffddc4).l,A0 ; data 00ffddc4
00002F62: lea (0xffde26).l,A1 ; data 00ffde26
00002F68: move.w #0x30,D7w
00002F6C: move.w (A1)+,(A0)+
00002F6E: dbf D7w,0x00002f6c
00002F72: move.w (0x00ffddc4).l,D0w
00002F78: move.b D0b,(0x00fffbf8).l
00002F7E: bsr.w 0x00003032 ; call 00003032 FUN_00003032
00002F82: move.w (0x00ffdf50).l,(0x00ffddc6).l
00002F8C: move.w (0x00ffdf52).l,(0x00ffddc8).l
00002F96: movea.l (0x00ffddfa).l,A6
00002F9C: bsr.w 0x00002bd2 ; call 00002BD2 FUN_00002bd2
00002FA0: bsr.w 0x0000301e ; call 0000301E FUN_0000301e
00002FA4: lea (0xff0100).l,A3 ; data 00ff0100
00002FAA: lea (0xffdf44).l,A4 ; data 00ffdf44
00002FB0: move.w #0x2,D7w
00002FB4: move.w (A4)+,(A3)
00002FB6: move.w (A4)+,(0x4,A3)
00002FBA: adda.w #0x100,A3
00002FBE: dbf D7w,0x00002fb4
00002FC2: move.l (0x00fffc3c).l,D0
00002FC8: beq.w 0x00002fe4
00002FCC: bsr.w 0x00002ff0 ; call 00002FF0 FUN_00002ff0
00002FD0: lea (0xff0100).l,A0 ; data 00ff0100
00002FD6: move.w (A0),D0w
00002FD8: addi.w #0x80,D0w
00002FDC: move.w D0w,(A1)
00002FDE: move.w (0x4,A0),(0x4,A1)
00002FE4: tst.l (0x00ffe114).l
00002FEA: bne.w 0x00002b78
00002FEE: rts
00002FF0: lea (0xff0400).l,A1 ; data 00ff0400
00002FF6: move.w #0xc,D7w
00002FFA: btst.b #0x0,(0x10,A1)
00003000: beq.w 0x00003010
00003004: btst.b #0x7,(0xb0,A1)
0000300A: beq.w 0x00003010
0000300E: rts
00003010: adda.w #0x100,A1
00003014: dbf D7w,0x00002ffa
00003018: stop #0x2700
0000301C: rts
0000301E: move.l #0x7371a,D0
00003024: move.w #-0x200,D1w
00003028: move.w #0x140,D2w
0000302C: jmp 0x00000f98.l ; call 00000F98 FUN_00000f98
00003032: cmpi.w #0x6,D0w
00003036: beq.w 0x0000306a
0000303A: cmpi.w #0x7,D0w
0000303E: beq.w 0x0000306a
00003042: cmpi.w #0x8,D0w
00003046: beq.w 0x0000306a
0000304A: cmpi.w #0x9,D0w
0000304E: beq.w 0x0000306a
00003052: cmpi.w #0x2c,D0w
00003056: blt.w 0x00003062
0000305A: cmpi.w #0x32,D0w
0000305E: blt.w 0x0000306a
00003062: clr.w (0x00fffc0e).l
00003068: rts
0000306A: move.w #0x1,(0x00fffc0e).l
00003072: rts
00003074: tst.w (0x00ffe108).l
0000307A: bne.w 0x00003084
0000307E: jsr 0x0000c7f8.l ; call 0000C7F8 FUN_0000c7f8
00003084: move.w (0x00ffddc4).l,D0w
0000308A: move.b D0b,(0x00fffbf8).l
00003090: bsr.b 0x00003032 ; call 00003032 FUN_00003032
00003092: lea (0x3232).l,A0 ; data 00003232
00003098: lsl.w #0x2,D0w
0000309A: movea.l (0x0,A0,D0w*0x1),A0 ; data 00003232
0000309E: lea (0xffddc6).l,A1 ; data 00ffddc6
000030A4: move.w #0x2f,D7w
000030A8: move.w (A0)+,(A1)+
000030AA: dbf D7w,0x000030a8
000030AE: move.l (0x00ffddde).l,(0x00ffde22).l
000030B8: tst.w (0x00fffc0e).l
000030BE: beq.w 0x00003134
000030C2: tst.w (0x00ffe10e).l
000030C8: bne.w 0x00003134
000030CC: move.w (0x00ffe10a).l,(0x00ffde08).l
000030D6: move.w (0x00ffe10a).l,(0x00ffde0c).l
000030E0: move.w (0x00ffe10a).l,(0x00ffde10).l
000030EA: move.w (0x00ffe10c).l,(0x00ffde0a).l
000030F4: move.w (0x00ffe10c).l,(0x00ffde0e).l
000030FE: move.w (0x00ffe10c).l,(0x00ffde12).l
00003108: move.w (0x00ffe10a).l,D1w
0000310E: subi.w #0xa00,D1w
00003112: bcc.w 0x00003118
00003116: clr.w D1w
00003118: move.w D1w,(0x00ffddc6).l
0000311E: move.w (0x00ffe10c).l,D1w
00003124: subi.w #0x700,D1w
00003128: bcc.w 0x0000312e
0000312C: clr.w D1w
0000312E: move.w D1w,(0x00ffddc8).l
00003134: clr.w (0x00ffe10e).l
0000313A: movea.l (0x00ffddda).l,A0
00003140: lea (0xff2700).l,A1 ; data 00ff2700
00003146: move.w #0xe0f,D7w
0000314A: move.w (A0)+,(A1)+
0000314C: dbf D7w,0x0000314a
00003150: movea.l (0x00ffdde2).l,A0
00003156: lea (0xff4700).l,A1 ; data 00ff4700
0000315C: move.w #0xe0f,D7w
00003160: move.w (A0)+,(A1)+
00003162: dbf D7w,0x00003160
00003166: rts
00003168: cmpi.b #0x10,(0x00fffbf9).l
00003170: bne.w 0x000031ec
00003174: cmpi.w #0x0,(0x00ffddc4).l
0000317C: bne.w 0x000031b8
00003180: cmpi.w #0x1400,(0x00ffddc6).l
00003188: bcs.w 0x000031a4
0000318C: move.w #0x5,(0x00ffddc4).l
00003194: move.l #0x3212,(0x00ffe114).l
0000319E: ori #0x1,CCR
000031A2: rts
000031A4: move.w #0x3,(0x00ffddc4).l
000031AC: move.l #0x3222,(0x00ffe114).l
000031B6: bra.b 0x0000319e
000031B8: cmpi.w #0x5,(0x00ffddc4).l
000031C0: beq.w 0x000031d8
000031C4: move.w #0x0,(0x00ffddc4).l
000031CC: move.l #0x3202,(0x00ffe114).l
000031D6: bra.b 0x0000319e
000031D8: move.w #0x0,(0x00ffddc4).l
000031E0: move.l #0x31f2,(0x00ffe114).l
000031EA: bra.b 0x000031d6
000031EC: andi #0x1e,CCR
000031F0: rts
00003346: movep.w (0xf0f,SP),D7w
0000334A: btst.l D7,D0
0000334C: move.l #0x6a5be,D0
00003352: move.l #0xdac0,D1
00003358: move.l #0x4c0,D2
0000335E: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00003364: move.w #-0xfbc,D4w
00003368: lea (0x6ace2).l,A1 ; data 0006ace2
0000336E: move.w #0x5,D7w
00003372: move.w #0x1,D1w
00003376: move.w #0x6000,D2w
0000337A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00003380: move.w #-0x2bc,D4w
00003384: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000338A: move.w #-0xebc,D4w
0000338E: lea (0x6aa7e).l,A1 ; data 0006aa7e
00003394: bsr.w 0x000033aa ; call 000033AA FUN_000033aa
00003398: move.w #-0xabc,D4w
0000339C: bsr.w 0x000033aa ; call 000033AA FUN_000033aa
000033A0: move.w #-0x6bc,D4w
000033A4: bsr.w 0x000033aa ; call 000033AA FUN_000033aa
000033A8: rts
000033AA: move.w #0x5,D7w
000033AE: move.w #0x7,D1w
000033B2: move.w #0x6000,D2w
000033B6: jsr 0x00001232.l ; call 00001232 FUN_00001232
000033BC: rts
000033BE: movem.l {  A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
000033C2: movea.l (0x00fffc04).l,A5
000033C8: move.l #0x380,D1
000033CE: moveq #0x20,D2
000033D0: moveq #0x3,D7
000033D2: move.l (0x50,A5),D6
000033D6: add.w (0x54,A5),D6w
000033DA: movea.l D6,A2
000033DC: moveq #0x3,D6
000033DE: clr.l D0
000033E0: move.w (A2)+,D0w
000033E2: asl.l #0x5,D0
000033E4: addi.l #0x6acfa,D0
000033EA: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
000033F0: addi.w #0x20,D1w
000033F4: dbf D6w,0x000033de
000033F8: addq.w #0x8,A2
000033FA: dbf D7w,0x000033dc
000033FE: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1}
00003402: rts
00003404: movem.l {  A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00003408: lea (0xff0100).l,A5 ; data 00ff0100
0000340E: moveq #0x2,D5
00003410: move.l #0xd280,D1
00003416: moveq #0x20,D2
00003418: moveq #0x3,D7
0000341A: btst.b #0x0,(0x10,A5)
00003420: bne.w 0x0000342e
00003424: lea (0x34f8).l,A2 ; data 000034f8
0000342A: bra.w 0x00003438
0000342E: move.l (0x50,A5),D6
00003432: add.w (0x54,A5),D6w
00003436: movea.l D6,A2
00003438: moveq #0x3,D6
0000343A: clr.l D0
0000343C: move.w (A2)+,D0w
0000343E: asl.l #0x5,D0
00003440: addi.l #0x6acfa,D0
00003446: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0000344C: addi.w #0x20,D1w
00003450: dbf D6w,0x0000343a
00003454: addq.w #0x8,A2
00003456: dbf D7w,0x00003438
0000345A: btst.b #0x0,(0x10,A5)
00003460: bne.w 0x0000346e
00003464: lea (0x34f8).l,A2 ; data 000034f8
0000346A: bra.w 0x000034c8
0000346E: lea (0xd4540).l,A4 ; data 000d4540
00003474: tst.b (0x56,A5)
00003478: bpl.w 0x00003482
0000347C: lea (0xd4ea0).l,A4 ; data 000d4ea0
00003482: clr.l D6
00003484: move.b (0x57,A5),D6b
00003488: bne.w 0x000034be
0000348C: move.w (0x90,A5),D0w
00003490: btst.l #0x3,D0
00003494: beq.w 0x000034a2
00003498: lea (0xd1878).l,A2 ; data 000d1878
0000349E: bra.w 0x000034c8
000034A2: btst.l #0x2,D0
000034A6: beq.w 0x000034b4
000034AA: lea (0xd186c).l,A2 ; data 000d186c
000034B0: bra.w 0x000034c8
000034B4: lea (0xd1860).l,A2 ; data 000d1860
000034BA: bra.w 0x000034c8
000034BE: subq.w #0x1,D6w
000034C0: mulu.w #0x30,D6
000034C4: movea.l (0x1a,A4,D6w*0x1),A2 ; data 000d4eba
000034C8: moveq #0x5,D6
000034CA: clr.l D0
000034CC: move.w (A2)+,D0w
000034CE: asl.l #0x5,D0
000034D0: addi.l #0xcf97c,D0
000034D6: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
000034DC: addi.w #0x20,D1w
000034E0: dbf D6w,0x000034ca
000034E4: adda.l #0x100,A5
000034EA: dbf D5w,0x00003418
000034EE: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1}
000034F2: bsr.w 0x00003548 ; call 00003548 FUN_00003548
000034F6: rts
00003548: movem.l {  A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0000354C: moveq #0x2,D0
0000354E: move.l D0,-(SP) ; data Stack[-0x2c]
00003550: bsr.w 0x00003560 ; call 00003560 FUN_00003560
00003554: move.l (SP)+,D0
00003556: dbmi D0w,0x0000354e
0000355A: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1}
0000355E: rts
00003560: bne.w 0x00003586
00003564: lea (0xff0100).l,A0 ; data 00ff0100
0000356A: move.w #-0x795c,D5w
0000356E: move.l #0xf448,D4
00003574: movem.l {  D5 D4},-(SP) ; data 0000f448
00003578: move.w #-0x796c,D5w
0000357C: move.l #0xf1c6,D4
00003582: bra.w 0x000035d4
00003586: lea (0xff0100).l,A0 ; data 00ff0100
0000358C: cmpi.w #0x1,D0w
00003590: bne.w 0x000035b6
00003594: adda.l #0x100,A0
0000359A: move.w #-0x7946,D5w
0000359E: move.l #0xf848,D4
000035A4: movem.l {  D5 D4},-(SP) ; data 0000f848
000035A8: move.w #-0x7956,D5w
000035AC: move.l #0xf5c6,D4
000035B2: bra.w 0x000035d4
000035B6: adda.l #0x200,A0
000035BC: move.w #-0x7930,D5w
000035C0: move.l #0xfc48,D4
000035C6: movem.l {  D5 D4},-(SP)
000035CA: move.w #-0x7940,D5w
000035CE: move.l #0xf9c6,D4
000035D4: move.w #0x3,D7w
000035D8: move.w #0x3,D1w
000035DC: move.w (0x2c,A0),D2w
000035E0: bne.w 0x000035e8
000035E4: move.w #0x2000,D2w
000035E8: bsr.w 0x00003648 ; call 00003648 FUN_00003648
000035EC: movem.l (SP)+,{  D4 D5}
000035F0: move.w #0x2,D7w
000035F4: move.w #0x1,D1w
000035F8: bsr.w 0x00003602 ; call 00003602 FUN_00003602
000035FC: bsr.w 0x00003648 ; call 00003648 FUN_00003648
00003600: rts
00003602: tst.b (0x57,A0)
00003606: beq.w 0x00003634
0000360A: cmpi.b #0x2,(0x9b,A0)
00003610: bpl.w 0x0000362a
00003614: tst.b (0x56,A0)
00003618: bpl.w 0x0000362a
0000361C: cmpi.b #0x6,(0x57,A0)
00003622: bpl.w 0x0000363e
00003626: bra.w 0x00003634
0000362A: cmpi.b #0xf,(0x57,A0)
00003630: bmi.w 0x0000363e
00003634: move.l #0x2000,D2
0000363A: bra.w 0x00003644
0000363E: move.l #0x6000,D2
00003644: move.l D2,D6
00003646: rts
00003648: move SR,-(SP) ; data Stack[-0x2]
0000364A: move #0x2700,SR
0000364E: lea (0xc00000).l,A2 ; data 00c00000
00003654: move.w D7w,D3w
00003656: or.w D2w,D5w
00003658: move.l D4,D0
0000365A: bsr.w 0x00000e9c ; call 00000E9C FUN_00000e9c
0000365E: move.w D5w,(A2)
00003660: addq.w #0x1,D5w
00003662: dbf D7w,0x0000365e
00003666: addi.w #0x80,D4w
0000366A: move.w D3w,D7w
0000366C: dbf D1w,0x00003658
00003670: move (SP)+,SR
00003672: rts
00003674: movem.l {  A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00003678: tst.w (0x3e,A0)
0000367C: bne.w 0x0000368a
00003680: move.l #0xf1c4,D4
00003686: bra.w 0x000036a4
0000368A: cmpi.w #0x1,(0x3e,A0)
00003690: bne.w 0x0000369e
00003694: move.l #0xf5c4,D4
0000369A: bra.w 0x000036a4
0000369E: move.l #0xf9c4,D4
000036A4: btst.b #0x0,(0x10,A0)
000036AA: beq.w 0x000036b6
000036AE: add.b D1b,(0x4c,A0)
000036B2: bgt.w 0x000036c4
000036B6: clr.b (0x4c,A0)
000036BA: lea (0x6ab3e).l,A1 ; data 0006ab3e
000036C0: bra.w 0x000036e4
000036C4: cmpi.b #0xa,(0x4c,A0)
000036CA: ble.w 0x000036d4
000036CE: move.b #0xa,(0x4c,A0)
000036D4: lea (0x6ab3e).l,A1 ; data 0006ab3e
000036DA: clr.l D2
000036DC: move.b (0x4c,A0),D2b
000036E0: lsl.w #0x3,D2w
000036E2: adda.l D2,A1
000036E4: move.w #0x0,D7w
000036E8: move.w #0x3,D1w
000036EC: move.w #0x6000,D2w
000036F0: jsr 0x00001232.l ; call 00001232 FUN_00001232
000036F6: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1}
000036FA: rts
000036FC: movem.l {  A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00003700: clr.l D2
00003702: clr.l D5
00003704: tst.w (0x3e,A0)
00003708: bne.w 0x00003716
0000370C: move.l #0xf4c4,D4
00003712: bra.w 0x00003730
00003716: cmpi.w #0x1,(0x3e,A0)
0000371C: bne.w 0x0000372a
00003720: move.l #0xf8c4,D4
00003726: bra.w 0x00003730
0000372A: move.l #0xfcc4,D4
00003730: btst.b #0x0,(0x10,A0)
00003736: beq.w 0x0000379a
0000373A: tst.b (0x56,A0)
0000373E: bmi.w 0x0000379a
00003742: cmpi.b #0x7f,(0x56,A0)
00003748: beq.w 0x0000379a
0000374C: cmpi.b #0xd,(0x57,A0)
00003752: bmi.w 0x0000378e
00003756: cmpi.b #0x10,(0x57,A0)
0000375C: bpl.w 0x0000379a
00003760: move.b (0x56,A0),D5b
00003764: add.b D1b,(0x6e,A0,D5w*0x1)
00003768: bgt.w 0x00003786
0000376C: clr.b (0x6e,A0,D5w*0x1)
00003770: clr.b (0x66,A0,D5w*0x1)
00003774: move.b #0x7f,(0x56,A0)
0000377A: clr.b (0x57,A0)
0000377E: bsr.w 0x00003404 ; call 00003404 FUN_00003404
00003782: bra.w 0x0000379a
00003786: move.b (0x6e,A0,D5w*0x1),D2b
0000378A: bra.w 0x000037ca
0000378E: add.b D1b,(0x4e,A0)
00003792: bgt.w 0x000037a4
00003796: clr.b (0x4e,A0)
0000379A: lea (0x6ac46).l,A1 ; data 0006ac46
000037A0: bra.w 0x000037d4
000037A4: move.b (0x57,A0),D5b
000037A8: subq.w #0x1,D5w
000037AA: mulu.w #0x30,D5
000037AE: lea (0xd4540).l,A4 ; data 000d4540
000037B4: adda.l D5,A4
000037B6: move.b (0x24,A4),D2b ; data 000d4564
000037BA: cmp.b (0x4e,A0),D2b
000037BE: bpl.w 0x000037c6
000037C2: move.b D2b,(0x4e,A0)
000037C6: move.b (0x4e,A0),D2b
000037CA: lea (0x6ac46).l,A1 ; data 0006ac46
000037D0: lsl.w #0x2,D2w
000037D2: adda.l D2,A1
000037D4: move.w #0x1,D7w
000037D8: move.w #0x0,D1w
000037DC: move.w #0x6000,D2w
000037E0: jsr 0x00001232.l ; call 00001232 FUN_00001232
000037E6: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1}
000037EA: rts
000037EC: movem.l {  A1 A0 D7 D6 D5 D4 D3 D2 D0},-(SP)
000037F0: tst.w (0x3e,A0)
000037F4: bne.w 0x00003802
000037F8: move.l #0xf3c6,D4
000037FE: bra.w 0x0000381c
00003802: cmpi.w #0x1,(0x3e,A0)
00003808: bne.w 0x00003816
0000380C: move.l #0xf7c6,D4
00003812: bra.w 0x0000381c
00003816: move.l #0xfbc6,D4
0000381C: btst.b #0x0,(0x10,A0)
00003822: beq.w 0x00003836
00003826: add.b D1b,(0x9c,A0)
0000382A: bgt.w 0x0000385a
0000382E: move.b (0x9c,A0),D1b
00003832: bsr.w 0x00003674 ; call 00003674 FUN_00003674
00003836: clr.b (0x9c,A0)
0000383A: move.l A0,D7
0000383C: cmp.l (0x00fffc04).l,D7
00003842: bne.w 0x00003850
00003846: lea (0x6abee).l,A1 ; data 0006abee
0000384C: bra.w 0x00003890
00003850: lea (0x6ab96).l,A1 ; data 0006ab96
00003856: bra.w 0x00003890
0000385A: cmpi.b #0xa,(0x9c,A0)
00003860: ble.w 0x0000386a
00003864: move.b #0xa,(0x9c,A0)
0000386A: move.l A0,D7
0000386C: cmp.l (0x00fffc04).l,D7
00003872: bne.w 0x00003880
00003876: lea (0x6abee).l,A1 ; data 0006abee
0000387C: bra.w 0x00003886
00003880: lea (0x6ab96).l,A1 ; data 0006ab96
00003886: clr.l D2
00003888: move.b (0x9c,A0),D2b
0000388C: lsl.w #0x3,D2w
0000388E: adda.l D2,A1
00003890: move.w #0x3,D7w
00003894: move.l D1,-(SP) ; data Stack[-0x28]
00003896: move.w #0x0,D1w
0000389A: move.w #0x6000,D2w
0000389E: jsr 0x00001232.l ; call 00001232 FUN_00001232
000038A4: move.l (SP)+,D1
000038A6: movem.l (SP)+,{ D0 D2 D3 D4 D5 D6 D7 A0 A1}
000038AA: rts
000038AC: clr.l D1
000038AE: tst.b (0x4e,A0)
000038B2: beq.w 0x000038f8
000038B6: move.b (0x57,A0),D1b
000038BA: beq.w 0x000038f8
000038BE: move.w (0x3e,A0),D6w
000038C2: cmpi.w #0x2,D6w
000038C6: bgt.w 0x000038f0
000038CA: cmpi.b #0x8,D1b
000038CE: bmi.w 0x000038e2
000038D2: cmpi.b #0xb,D1b
000038D6: bpl.w 0x000038e2
000038DA: move.w #-0x3,D1w
000038DE: bra.w 0x000038e6
000038E2: move.w #-0x1,D1w
000038E6: bsr.w 0x000036fc ; call 000036FC FUN_000036fc
000038EA: move.w #-0x1,D1w
000038EE: rts
000038F0: subq.b #0x1,(0x4e,A0)
000038F4: move.w #-0x1,D1w
000038F8: rts
000038FA: lea (0xff0100).l,A0 ; data 00ff0100
00003900: moveq #0x2,D7
00003902: clr.w D1w
00003904: bsr.w 0x00003674 ; call 00003674 FUN_00003674
00003908: bsr.w 0x000037ec ; call 000037EC FUN_000037ec
0000390C: bsr.w 0x000036fc ; call 000036FC FUN_000036fc
00003910: adda.w #0x100,A0
00003914: dbf D7w,0x00003902
00003918: rts
0000391A: move.b #0x1,(0x00fff0cc).l
00003922: move.w #0x25,D0w
00003926: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000392C: movea.l #0x6aa7e,A1
00003932: movea.l (0x00fffc04).l,A0
00003938: move.l A0,D5
0000393A: move.w (0x3e,A0),D4w
0000393E: mulu.w #0x400,D4
00003942: addi.w #0x100,D4w
00003946: addi.w #-0xfbc,D4w
0000394A: bsr.w 0x000033aa ; call 000033AA FUN_000033aa
0000394E: movea.l (0x00fffc04).l,A0
00003954: move.l (0xa2,A0),D0
00003958: beq.w 0x00003964
0000395C: movea.l D0,A2
0000395E: bclr.b #0x4,(0x11,A2)
00003964: cmpi.w #0x2,(0x3e,A0)
0000396A: bne.w 0x00003992
0000396E: subq.b #0x1,(0x00fff0cc).l
00003974: bclr.b #0x1,(0x10,A0)
0000397A: move.l #0xff0100,(0x00fffc04).l
00003984: tst.b (0x00fff0cc).l
0000398A: bmi.w 0x000039d4
0000398E: bra.w 0x000039a2
00003992: bclr.b #0x1,(0x10,A0)
00003998: addi.l #0x100,(0x00fffc04).l
000039A2: movea.l (0x00fffc04).l,A0
000039A8: bset.b #0x1,(0x10,A0)
000039AE: btst.b #0x0,(0x10,A0)
000039B4: beq.b 0x00003964
000039B6: tst.b (0x00fff09d).l
000039BC: bne.w 0x000039d4
000039C0: btst.b #0x6,(0x10,A0)
000039C6: bne.b 0x00003964
000039C8: btst.b #0x4,(0x10,A0)
000039CE: beq.w 0x000039d4
000039D2: bra.b 0x00003964
000039D4: clr.b (0x00fff0cc).l
000039DA: cmpa.l D5,A0
000039DC: beq.w 0x000039e6
000039E0: jsr 0x0001e540.l ; call 0001E540 FUN_0001e540
000039E6: movea.l (0x00fffc04).l,A0
000039EC: jsr 0x0001dc4c.l ; call 0001DC4C FUN_0001dc4c
000039F2: movea.l #0x6aade,A1
000039F8: movea.l (0x00fffc04).l,A0
000039FE: move.w (0x3e,A0),D4w
00003A02: mulu.w #0x400,D4
00003A06: addi.w #0x100,D4w
00003A0A: addi.w #-0xfbc,D4w
00003A0E: bsr.w 0x000033aa ; call 000033AA FUN_000033aa
00003A12: bsr.w 0x00003548 ; call 00003548 FUN_00003548
00003A16: bsr.w 0x000038fa ; call 000038FA FUN_000038fa
00003A1A: rts
00003A1C: cmpi.b #0x3,(0x00ffe186).l
00003A24: bne.w 0x00003a3c
00003A28: move.w #0x0,(0x00ffdf5e).l
00003A30: move.w #0x600,(0x00ffdf5c).l
00003A38: bra.w 0x00003bac ; call 00003BAC FUN_00003bac
00003A3C: movea.l (0x00fffc04).l,A0
00003A42: move.w (0x2e,A0),D2w
00003A46: move.w #0x3c0,D3w
00003A4A: jsr 0x00015dcc.l ; call 00015DCC FUN_00015dcc
00003A50: add.w (A0),D3w
00003A52: bpl.w 0x00003a58
00003A56: clr.w D3w
00003A58: add.w (0x4,A0),D2w
00003A5C: bpl.w 0x00003a62
00003A60: clr.w D2w
00003A62: move.w D2w,D4w
00003A64: move.w (0x00ffddc6).l,D6w
00003A6A: move.w (0x00ffddc8).l,D7w
00003A70: addi.w #0x880,D6w
00003A74: addi.w #0x700,D7w
00003A78: bsr.w 0x00003a90 ; call 00003A90 FUN_00003a90
00003A7C: move.b D1b,(0x00ffe173).l
00003A82: bsr.w 0x00003b1e ; call 00003B1E FUN_00003b1e
00003A86: move.b D2b,(0x00ffe174).l
00003A8C: bra.w 0x00003c36
00003A90: clr.w D1w
00003A92: sub.w D3w,D6w
00003A94: bcs.w 0x00003ada
00003A98: cmpi.w #0x300,D6w
00003A9C: bcs.w 0x00003aa6
00003AA0: move.w #-0x2,D1w
00003AA4: rts
00003AA6: cmpi.w #0x180,D6w
00003AAA: bcs.w 0x00003ac6
00003AAE: move.w #-0x2,D1w
00003AB2: cmpi.b #-0x2,(0x00ffe173).l
00003ABA: bne.w 0x00003ac0
00003ABE: rts
00003AC0: move.w #-0x1,D1w
00003AC4: rts
00003AC6: cmpi.b #-0x1,(0x00ffe173).l
00003ACE: beq.w 0x00003ad4
00003AD2: rts
00003AD4: move.w #-0x1,D1w
00003AD8: rts
00003ADA: neg.w D6w
00003ADC: cmpi.w #0x300,D6w
00003AE0: bcs.w 0x00003aea
00003AE4: move.w #0x2,D1w
00003AE8: rts
00003AEA: cmpi.w #0x180,D6w
00003AEE: bcs.w 0x00003b0a
00003AF2: move.w #0x2,D1w
00003AF6: cmpi.b #0x2,(0x00ffe173).l
00003AFE: bne.w 0x00003b04
00003B02: rts
00003B04: move.w #0x1,D1w
00003B08: rts
00003B0A: cmpi.b #0x1,(0x00ffe173).l
00003B12: beq.w 0x00003b18
00003B16: rts
00003B18: move.w #0x1,D1w
00003B1C: rts
00003B1E: clr.w D2w
00003B20: sub.w D4w,D7w
00003B22: bcs.w 0x00003b68
00003B26: cmpi.w #0x300,D7w
00003B2A: bcs.w 0x00003b34
00003B2E: move.w #-0x2,D2w
00003B32: rts
00003B34: cmpi.w #0x180,D7w
00003B38: bcs.w 0x00003b54
00003B3C: move.w #-0x2,D2w
00003B40: cmpi.b #-0x2,(0x00ffe174).l
00003B48: bne.w 0x00003b4e
00003B4C: rts
00003B4E: move.w #-0x1,D2w
00003B52: rts
00003B54: cmpi.b #-0x1,(0x00ffe174).l
00003B5C: beq.w 0x00003b62
00003B60: rts
00003B62: move.w #-0x1,D2w
00003B66: rts
00003B68: neg.w D7w
00003B6A: cmpi.w #0x300,D7w
00003B6E: bcs.w 0x00003b78
00003B72: move.w #0x2,D2w
00003B76: rts
00003B78: cmpi.w #0x180,D7w
00003B7C: bcs.w 0x00003b98
00003B80: move.w #0x2,D2w
00003B84: cmpi.b #0x2,(0x00ffe174).l
00003B8C: bne.w 0x00003b92
00003B90: rts
00003B92: move.w #0x1,D2w
00003B96: rts
00003B98: cmpi.b #0x1,(0x00ffe174).l
00003BA0: beq.w 0x00003ba6
00003BA4: rts
00003BA6: move.w #0x1,D2w
00003BAA: rts
00003BAC: move.w (0x00ffddc6).l,D3w
00003BB2: move.w (0x00ffddc8).l,D2w
00003BB8: move.w (0x00ffdf5c).l,D5w
00003BBE: move.w (0x00ffdf5e).l,D4w
00003BC4: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00003BCA: move.w D3w,D4w
00003BCC: cmpi.w #0x20,D4w
00003BD0: bcc.w 0x00003bdc
00003BD4: clr.w (0x00ffdf5a).l
00003BDA: rts
00003BDC: rol.w #0x3,D2w
00003BDE: andi.w #0x7,D2w
00003BE2: move.w D2w,D0w
00003BE4: lea (0x4290).l,A1 ; data 00004290
00003BEA: lsl.w #0x2,D0w
00003BEC: move.w (0x0,A1,D0w*0x1),D1w ; data 00004290
00003BF0: addq.w #0x2,D0w
00003BF2: move.w (0x0,A1,D0w*0x1),D2w ; data 00004290
00003BF6: cmpi.w #0x28,D4w
00003BFA: bcs.w 0x00003c36
00003BFE: move.w (0x00ffddc6).l,D0w
00003C04: cmp.w (0x00ffddce).l,D0w
00003C0A: beq.w 0x00003c36
00003C0E: cmp.w (0x00ffddca).l,D0w
00003C14: beq.w 0x00003c36
00003C18: move.w (0x00ffddc8).l,D0w
00003C1E: cmp.w (0x00ffddd0).l,D0w
00003C24: beq.w 0x00003c36
00003C28: cmp.w (0x00ffddcc).l,D0w
00003C2E: beq.w 0x00003c36
00003C32: asl.w #0x1,D1w
00003C34: asl.w #0x1,D2w
00003C36: tst.w (0x00ffe100).l
00003C3C: bne.w 0x00003d16
00003C40: tst.w (0x00ffe102).l
00003C46: bne.w 0x00003d16
00003C4A: tst.w (0x00ffdfe8).l
00003C50: bne.w 0x00003d16
00003C54: move.w D1w,D0w
00003C56: lsl.w #0x4,D1w
00003C58: tst.w D1w
00003C5A: beq.w 0x00003cba
00003C5E: bpl.w 0x00003c76
00003C62: add.w (0x00ffddc6).l,D1w
00003C68: cmp.w (0x00ffddca).l,D1w
00003C6E: blt.w 0x00003cba
00003C72: bra.w 0x00003c86
00003C76: add.w (0x00ffddc6).l,D1w
00003C7C: cmp.w (0x00ffddce).l,D1w
00003C82: bgt.w 0x00003cba
00003C86: move.w D1w,(0x00ffddc6).l
00003C8C: neg.w D0w
00003C8E: add.w D0w,(0x00ffdd08).l
00003C94: add.w D0w,(0x00ffdd0c).l
00003C9A: andi.w #-0x80,D1w
00003C9E: cmp.w (0x00ffdf38).l,D1w
00003CA4: beq.w 0x00003cba
00003CA8: move.w D1w,(0x00ffdf38).l
00003CAE: movem.w {  D2w D1w D0w},-(SP)
00003CB2: bsr.w 0x000040e2 ; call 000040E2 FUN_000040e2
00003CB6: movem.w (SP)+,{ D0 D1 D2}
00003CBA: move.w D2w,D0w
00003CBC: lsl.w #0x4,D2w
00003CBE: tst.w D2w
00003CC0: beq.w 0x00003d16
00003CC4: bpl.w 0x00003cdc
00003CC8: add.w (0x00ffddc8).l,D2w
00003CCE: cmp.w (0x00ffddcc).l,D2w
00003CD4: blt.w 0x00003d16
00003CD8: bra.w 0x00003cec
00003CDC: add.w (0x00ffddc8).l,D2w
00003CE2: cmp.w (0x00ffddd0).l,D2w
00003CE8: bgt.w 0x00003d16
00003CEC: move.w D2w,(0x00ffddc8).l
00003CF2: add.w D0w,(0x00ffdd0a).l
00003CF8: add.w D0w,(0x00ffdd0e).l
00003CFE: andi.w #-0x80,D2w
00003D02: cmp.w (0x00ffdf36).l,D2w
00003D08: beq.w 0x00003d16
00003D0C: move.w D2w,(0x00ffdf36).l
00003D12: bsr.w 0x00004018 ; call 00004018 FUN_00004018
00003D16: rts
00003D18: bsr.w 0x000041b6 ; call 000041B6 fold_field_coordinate
00003D1C: bsr.w 0x00004000 ; call 00004000 FUN_00004000
00003D20: add.w D5w,D4w
00003D22: move.w #0x1e,D2w
00003D26: movem.l {  D4 D3 D2 D1 D0},-(SP)
00003D2A: bsr.w 0x00003d4a ; call 00003D4A FUN_00003d4a
00003D2E: movem.l (SP)+,{ D0 D1 D2 D3 D4}
00003D32: move.w D4w,D3w
00003D34: andi.w #-0x1000,D3w
00003D38: addi.w #0x80,D4w
00003D3C: andi.w #0xfff,D4w
00003D40: or.w D3w,D4w
00003D42: addq.w #0x8,D1w
00003D44: dbf D2w,0x00003d26
00003D48: rts
00003D4A: bsr.w 0x00003fd8 ; call 00003FD8 FUN_00003fd8
00003D4E: move.w D4w,D7w
00003D50: andi.w #0x7f,D7w
00003D54: subi.w #0x80,D7w
00003D58: neg.w D7w
00003D5A: lsr.w #0x1,D7w
00003D5C: subq.w #0x1,D7w
00003D5E: cmpi.w #0x2b,D7w
00003D62: blt.w 0x00003d6e
00003D66: move.w #0x2b,D7w
00003D6A: bra.w 0x00003d80 ; call 00003D80 FUN_00003d80
00003D6E: move.w D7w,-(SP) ; data Stack[-0x2]
00003D70: bsr.w 0x00003d80 ; call 00003D80 FUN_00003d80
00003D74: move.w (SP)+,D7w
00003D76: subi.w #0x2a,D7w
00003D7A: neg.w D7w
00003D7C: andi.w #-0x80,D4w
00003D80: move SR,-(SP) ; data Stack[-0x2]
00003D82: move #0x2700,SR
00003D86: move.w D4w,D0w
00003D88: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00003D8E: lea (0xc00000).l,A2 ; data 00c00000
00003D94: clr.l D0
00003D96: move.w (0x0,A1,D3w*0x1),D0w
00003D9A: lsl.l #0x5,D0
00003D9C: move.l D0,D2
00003D9E: add.l D1,D2
00003DA0: move.w (0x0,A0,D2*0x1),D6w
00003DA4: tst.w (0x00ffdf58).l
00003DAA: bpl.w 0x00003db6
00003DAE: eori.w #-0x8000,D6w
00003DB2: bra.w 0x00003dbc
00003DB6: or.w (0x00ffdf58).l,D6w
00003DBC: move.w D6w,(A2)
00003DBE: addq.w #0x2,D1w
00003DC0: move.w D1w,D6w
00003DC2: andi.w #0x7,D6w
00003DC6: bne.w 0x00003dd6
00003DCA: subq.w #0x8,D1w
00003DCC: clr.l D0
00003DCE: addq.w #0x2,D3w
00003DD0: move.w (0x0,A1,D3w*0x1),D0w
00003DD4: lsl.l #0x5,D0
00003DD6: dbf D7w,0x00003d9c
00003DDA: move (SP)+,SR
00003DDC: rts
00003DDE: bsr.w 0x00003fd8 ; call 00003FD8 FUN_00003fd8
00003DE2: move.w D4w,D7w
00003DE4: andi.w #0xf80,D7w
00003DE8: subi.w #0x1000,D7w
00003DEC: neg.w D7w
00003DEE: lsr.w #0x7,D7w
00003DF0: subq.w #0x1,D7w
00003DF2: cmpi.w #0x1f,D7w
00003DF6: blt.w 0x00003e02
00003DFA: move.w #0x1f,D7w
00003DFE: bra.w 0x00003e14 ; call 00003E14 FUN_00003e14
00003E02: move.w D7w,-(SP) ; data Stack[-0x2]
00003E04: bsr.w 0x00003e14 ; call 00003E14 FUN_00003e14
00003E08: move.w (SP)+,D7w
00003E0A: subi.w #0x1e,D7w
00003E0E: neg.w D7w
00003E10: andi.w #-0xf81,D4w
00003E14: move SR,-(SP) ; data Stack[-0x2]
00003E16: move #0x2700,SR
00003E1A: move.w D4w,D0w
00003E1C: move.w #-0x7080,(0x00c00004).l
00003E24: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00003E2A: lea (0xc00000).l,A2 ; data 00c00000
00003E30: clr.l D0
00003E32: move.w (0x0,A1,D3w*0x1),D0w
00003E36: lsl.l #0x5,D0
00003E38: move.l D0,D2
00003E3A: add.l D1,D2
00003E3C: move.w (0x0,A0,D2*0x1),D6w
00003E40: tst.w (0x00ffdf58).l
00003E46: bpl.w 0x00003e52
00003E4A: eori.w #-0x8000,D6w
00003E4E: bra.w 0x00003e58
00003E52: or.w (0x00ffdf58).l,D6w
00003E58: move.w D6w,(A2)
00003E5A: addq.w #0x8,D1w
00003E5C: cmpi.w #0x1f,D1w
00003E60: blt.w 0x00003e76
00003E64: subi.w #0x20,D1w
00003E68: add.w (0x00ffddea).l,D3w
00003E6E: clr.l D0
00003E70: move.w (0x0,A1,D3w*0x1),D0w
00003E74: lsl.l #0x5,D0
00003E76: dbf D7w,0x00003e38
00003E7A: move.w #-0x70fe,(0x00c00004).l
00003E82: move (SP)+,SR
00003E84: rts
00003E86: movea.l (0x2,A2),A5
00003E8A: movea.l (0x48,A0),A4
00003E8E: move.w (0x6,A2),D2w
00003E92: subq.w #0x1,D2w
00003E94: lea (0x3edc).l,A6 ; data 00003edc
00003E9A: cmpi.w #0x2000,(0x2c,A0)
00003EA0: beq.w 0x00003eba
00003EA4: lea (0x3edc).l,A6 ; data 00003edc
00003EAA: cmpi.w #0x4000,(0x2c,A0)
00003EB0: beq.w 0x00003eba
00003EB4: lea (0x3edc).l,A6 ; data 00003edc
00003EBA: move.b (A5)+,D0b
00003EBC: move.b D0b,D1b
00003EBE: andi.b #0xf,D0b
00003EC2: ext.w D0w
00003EC4: move.b (0x0,A6,D0w*0x1),D0b ; data 00003edc
00003EC8: lsr.b #0x4,D1b
00003ECA: ext.w D1w
00003ECC: move.b (0x0,A6,D1w*0x1),D1b ; data 00003edc
00003ED0: lsl.b #0x4,D1b
00003ED2: or.b D1b,D0b
00003ED4: move.b D0b,(A4)+
00003ED6: dbf D2w,0x00003eba
00003EDA: rts
00003EFE: movem.l {  D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00003F02: move.w (0x8,A2),D0w
00003F06: add.w (0xa,A0),D0w
00003F0A: subi.w #0x80,D0w
00003F0E: move.w (0x00ffddc6).l,D2w
00003F14: lsr.w #0x4,D2w
00003F16: add.w D2w,D0w
00003F18: move.w (0xa,A2),D1w
00003F1C: add.w (0xc,A0),D1w
00003F20: subi.w #0x80,D1w
00003F24: move.w (0x00ffddc8).l,D2w
00003F2A: lsr.w #0x4,D2w
00003F2C: add.w D2w,D1w
00003F2E: move.w (0xc,A2),D2w
00003F32: move.w D2w,D3w
00003F34: lsr.w #0x2,D2w
00003F36: andi.w #0x3,D3w
00003F3A: move.w D3w,D5w
00003F3C: move.w D1w,D6w
00003F3E: movea.l (0x2,A2),A5
00003F42: movea.l (0x44,A0),A4
00003F46: movem.l {  D1 D0},-(SP)
00003F4A: bsr.w 0x00003f9e ; call 00003F9E FUN_00003f9e
00003F4E: move.w #0x1f,D7w
00003F52: move.b (A5)+,D0b
00003F54: move.b D0b,D1b
00003F56: beq.w 0x00003f7a
00003F5A: andi.b #-0x10,D1b
00003F5E: bne.w 0x00003f6a
00003F62: move.b #0xf,D1b
00003F66: bra.w 0x00003f7a
00003F6A: move.b #-0x10,D1b
00003F6E: andi.b #0xf,D0b
00003F72: beq.w 0x00003f7a
00003F76: move.b #-0x1,D1b
00003F7A: move.b (A1)+,D4b
00003F7C: and.b D4b,D1b
00003F7E: move.b D1b,(A4)+
00003F80: dbf D7w,0x00003f52
00003F84: movem.l (SP)+,{ D0 D1}
00003F88: addq.w #0x8,D1w
00003F8A: dbf D3w,0x00003f46
00003F8E: move.w D6w,D1w
00003F90: addq.w #0x8,D0w
00003F92: move.w D5w,D3w
00003F94: dbf D2w,0x00003f46
00003F98: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7}
00003F9C: rts
00003F9E: movem.w {  D3w D2w},-(SP)
00003FA2: bsr.w 0x00003fd8 ; call 00003FD8 FUN_00003fd8
00003FA6: lea (0xff2700).l,A1 ; data 00ff2700
00003FAC: move.w (0x0,A1,D3w*0x1),D0w ; data 00ff2700
00003FB0: ext.l D0
00003FB2: movea.l (0x00ffddde).l,A1
00003FB8: lsl.l #0x5,D0
00003FBA: add.l D1,D0
00003FBC: move.w (0x0,A1,D0w*0x1),D0w
00003FC0: andi.w #0x7ff,D0w
00003FC4: subi.w #0x201,D0w
00003FC8: lsl.w #0x5,D0w
00003FCA: movea.l (0x00ffddfe).l,A1
00003FD0: adda.w D0w,A1
00003FD2: movem.w (SP)+,{  D2 D3}
00003FD6: rts
00003FD8: move.w D0w,D2w
00003FDA: move.w D1w,D3w
00003FDC: lsr.w #0x3,D0w
00003FDE: andi.w #0x3,D0w
00003FE2: lsr.w #0x3,D1w
00003FE4: andi.w #0x3,D1w
00003FE8: lsr.w #0x5,D3w
00003FEA: mulu.w (0x00ffddea).l,D3
00003FF0: lsr.w #0x5,D2w
00003FF2: lsl.w #0x1,D2w
00003FF4: add.w D2w,D3w
00003FF6: lsl.w #0x2,D1w
00003FF8: add.w D0w,D1w
00003FFA: lsl.w #0x1,D1w
00003FFC: ext.l D1
00003FFE: rts
00004000: neg.w D3w
00004002: andi.w #0x1ff,D3w
00004006: andi.w #0xf8,D4w
0000400A: lsr.w #0x2,D3w
0000400C: lsl.w #0x4,D4w
0000400E: add.w D3w,D4w
00004010: andi.w #-0x2,D4w
00004014: ext.l D4
00004016: rts
00004018: move.w (0x00ffddc8).l,D1w
0000401E: lsr.w #0x4,D1w
00004020: tst.w D0w
00004022: bpl.w 0x0000407e
00004026: subq.w #0x8,D1w
00004028: bmi.w 0x0000407c
0000402C: bsr.w 0x00004216 ; call 00004216 offset_field_coordinate
00004030: move.w (0x00ffdd0a).l,D4w
00004036: subq.w #0x8,D4w
00004038: bsr.b 0x00004000 ; call 00004000 FUN_00004000
0000403A: movem.w {  D4w D3w D2w D1w D0w},-(SP)
0000403E: movea.l (0x00ffddde).l,A0
00004044: lea (0xff2700).l,A1 ; data 00ff2700
0000404A: add.w (0x00ffdd14).l,D4w
00004050: bsr.w 0x00003d4a ; call 00003D4A FUN_00003d4a
00004054: movem.w (SP)+,{ D0 D1 D2 D3 D4}
00004058: movea.l (0x00ffdde6).l,A0
0000405E: lea (0xff4700).l,A1 ; data 00ff4700
00004064: add.w (0x00ffdd16).l,D4w
0000406A: move.w #-0x8000,(0x00ffdf58).l
00004072: bsr.w 0x00003d4a ; call 00003D4A FUN_00003d4a
00004076: clr.w (0x00ffdf58).l
0000407C: rts
0000407E: addq.w #0x8,D1w
00004080: cmp.w (0x00ffddd0).l,D1w
00004086: bgt.b 0x0000407c
00004088: bsr.w 0x00004216 ; call 00004216 offset_field_coordinate
0000408C: addi.w #0xe0,D1w
00004090: move.w (0x00ffdd0a).l,D4w
00004096: addi.w #0xe8,D4w
0000409A: bsr.w 0x00004000 ; call 00004000 FUN_00004000
0000409E: movem.w {  D4w D3w D2w D1w D0w},-(SP)
000040A2: movea.l (0x00ffddde).l,A0
000040A8: lea (0xff2700).l,A1 ; data 00ff2700
000040AE: add.w (0x00ffdd14).l,D4w
000040B4: bsr.w 0x00003d4a ; call 00003D4A FUN_00003d4a
000040B8: movem.w (SP)+,{ D0 D1 D2 D3 D4}
000040BC: movea.l (0x00ffdde6).l,A0
000040C2: lea (0xff4700).l,A1 ; data 00ff4700
000040C8: add.w (0x00ffdd16).l,D4w
000040CE: move.w #-0x8000,(0x00ffdf58).l
000040D6: bsr.w 0x00003d4a ; call 00003D4A FUN_00003d4a
000040DA: clr.w (0x00ffdf58).l
000040E0: bra.b 0x0000407c
000040E2: tst.w D0w
000040E4: bmi.w 0x0000414a
000040E8: move.w (0x00ffddc6).l,D0w
000040EE: lsr.w #0x4,D0w
000040F0: subq.w #0x8,D0w
000040F2: bmi.w 0x00004148
000040F6: bsr.w 0x000041f0 ; call 000041F0 FUN_000041f0
000040FA: move.w (0x00ffdd08).l,D3w
00004100: addq.w #0x8,D3w
00004102: bsr.w 0x00004000 ; call 00004000 FUN_00004000
00004106: movem.w {  D4w D3w D2w D1w D0w},-(SP)
0000410A: movea.l (0x00ffddde).l,A0
00004110: lea (0xff2700).l,A1 ; data 00ff2700
00004116: add.w (0x00ffdd14).l,D4w
0000411C: bsr.w 0x00003dde ; call 00003DDE FUN_00003dde
00004120: movem.w (SP)+,{ D0 D1 D2 D3 D4}
00004124: movea.l (0x00ffdde6).l,A0
0000412A: lea (0xff4700).l,A1 ; data 00ff4700
00004130: add.w (0x00ffdd16).l,D4w
00004136: move.w #-0x8000,(0x00ffdf58).l
0000413E: bsr.w 0x00003dde ; call 00003DDE FUN_00003dde
00004142: clr.w (0x00ffdf58).l
00004148: rts
0000414A: move.w (0x00ffddc6).l,D0w
00004150: lsr.w #0x4,D0w
00004152: addq.w #0x8,D0w
00004154: cmp.w (0x00ffddce).l,D0w
0000415A: bgt.b 0x00004148
0000415C: bsr.w 0x000041f0 ; call 000041F0 FUN_000041f0
00004160: addi.w #0x140,D0w
00004164: move.w (0x00ffdd08).l,D3w
0000416A: subi.w #0x148,D3w
0000416E: bsr.w 0x00004000 ; call 00004000 FUN_00004000
00004172: movem.w {  D4w D3w D2w D1w D0w},-(SP)
00004176: movea.l (0x00ffddde).l,A0
0000417C: lea (0xff2700).l,A1 ; data 00ff2700
00004182: add.w (0x00ffdd14).l,D4w
00004188: bsr.w 0x00003dde ; call 00003DDE FUN_00003dde
0000418C: movem.w (SP)+,{ D0 D1 D2 D3 D4}
00004190: movea.l (0x00ffdde6).l,A0
00004196: lea (0xff4700).l,A1 ; data 00ff4700
0000419C: add.w (0x00ffdd16).l,D4w
000041A2: move.w #-0x8000,(0x00ffdf58).l
000041AA: bsr.w 0x00003dde ; call 00003DDE FUN_00003dde
000041AE: clr.w (0x00ffdf58).l
000041B4: bra.b 0x00004148
000041B6: move.w (0x00ffdd08).l,D3w
000041BC: move.w (0x00ffdd0a).l,D4w
000041C2: move.w (0x00ffddc6).l,D0w
000041C8: move.w (0x00ffddc8).l,D1w
000041CE: lsr.w #0x4,D0w
000041D0: lsr.w #0x4,D1w
000041D2: subq.w #0x8,D0w
000041D4: bmi.w 0x000041de
000041D8: addq.w #0x8,D3w
000041DA: bra.w 0x000041e0
000041DE: addq.w #0x8,D0w
000041E0: subq.w #0x8,D1w
000041E2: bmi.w 0x000041ec
000041E6: subq.w #0x8,D4w
000041E8: bra.w 0x000041ee
000041EC: addq.w #0x8,D1w
000041EE: rts
000041F0: move.w (0x00ffdd0a).l,D4w
000041F6: move.w (0x00ffddc8).l,D1w
000041FC: lsr.w #0x4,D1w
000041FE: subq.w #0x8,D1w
00004200: bmi.w 0x00004212
00004204: subq.w #0x8,D4w
00004206: subq.w #0x8,D1w
00004208: bmi.w 0x00004212
0000420C: subq.w #0x8,D4w
0000420E: bra.w 0x00004214
00004212: addq.w #0x8,D1w
00004214: rts
00004216: move.w (0x00ffdd08).l,D3w
0000421C: move.w (0x00ffddc6).l,D0w
00004222: lsr.w #0x4,D0w
00004224: subq.w #0x8,D0w
00004226: bmi.w 0x00004238
0000422A: addq.w #0x8,D3w
0000422C: subq.w #0x8,D0w
0000422E: bmi.w 0x00004238
00004232: addq.w #0x8,D3w
00004234: bra.w 0x0000423a
00004238: addq.w #0x8,D0w
0000423A: rts
0000423C: nop
0000423E: nop
00004240: nop
00004242: sub.w (0x00ffddc6).l,D0w
00004248: sub.w (0x00ffddc8).l,D1w
0000424E: move.w (0x00ffdd08).l,D3w
00004254: move.w (0x00ffdd0a).l,D4w
0000425A: neg.w D3w
0000425C: andi.w #0x1ff,D3w
00004260: asr.w #0x4,D0w
00004262: asr.w #0x4,D1w
00004264: andi.w #0xf8,D4w
00004268: lsr.w #0x2,D3w
0000426A: lsl.w #0x4,D4w
0000426C: add.w D3w,D4w
0000426E: andi.w #-0x2,D4w
00004272: asr.w #0x3,D0w
00004274: lsl.w #0x1,D0w
00004276: asr.w #0x3,D1w
00004278: lsl.w #0x7,D1w
0000427A: move.w D4w,D3w
0000427C: andi.w #-0x80,D3w
00004280: add.w D0w,D4w
00004282: andi.w #0x7e,D4w
00004286: or.w D3w,D4w
00004288: add.w D1w,D4w
0000428A: andi.w #0xfff,D4w
0000428E: rts
000042B0: lea (0xff0100).l,A1 ; data 00ff0100
000042B6: lea (0x7291a).l,A0 ; data 0007291a
000042BC: mulu.w #0x100,D0
000042C0: adda.l D0,A0
000042C2: move.w #0x3f,D7w
000042C6: move.l (A0)+,(A1)+ ; data 0007291a
000042C8: dbf D7w,0x000042c6
000042CC: move.w #0x0,(0x00ff013e).l
000042D4: move.w #0x1,(0x00ff023e).l
000042DC: move.w #0x2,(0x00ff033e).l
000042E4: rts
000042F2: move.b D0b,(0x00fffc0c).l
000042F8: addq.b #0x1,(0x00fffc0c).l
000042FE: lea (0xff0200).l,A1 ; data 00ff0200
00004304: lea (0xff0200).l,A5 ; data 00ff0200
0000430A: move.w #0x1,D1w
0000430E: move.w #0x19,D2w
00004312: move.w #0x320,D3w
00004316: clr.l D4
00004318: bra.w 0x00004350
0000431C: move.b (0x00fffc0c).l,D6b
00004322: subq.b #0x1,D6b
00004324: cmp.b D6b,D0b
00004326: beq.w 0x00004378
0000432A: move.b D0b,(0x00fffc0d).l
00004330: addq.b #0x1,(0x00fffc0d).l
00004336: lea (0xff0300).l,A1 ; data 00ff0300
0000433C: lea (0xff0300).l,A5 ; data 00ff0300
00004342: moveq #0x2,D1
00004344: move.w #0x31,D2w
00004348: move.w #0x620,D3w
0000434C: move.w #0x1,D4w
00004350: lea (0x72c1a).l,A0 ; data 00072c1a
00004356: mulu.w #0x100,D0
0000435A: adda.l D0,A0
0000435C: move.w #0x3f,D6w
00004360: move.l (A0)+,(A1)+ ; data 00072c1a
00004362: dbf D6w,0x00004360
00004366: suba.l #0x100,A1
0000436C: move.w D1w,(0x3e,A1)
00004370: move.w D2w,(0x20,A1)
00004374: move.w D3w,(0x26,A1)
00004378: move.l D7,-(SP) ; data Stack[-0x4]
0000437A: jsr 0x0005966e.l ; call 0005966E FUN_0005966e
00004380: move.l (SP)+,D7
00004382: move.b D7b,(0x8f,A1)
00004386: rts
00004388: bsr.w 0x000043b6 ; call 000043B6 FUN_000043b6
0000438C: cmpi.b #0x2,(0x00ffe186).l
00004394: bne.w 0x000043a0
00004398: move.b #0x6,(0x00ff049d).l
000043A0: cmpi.b #0x1,(0x00ffe186).l
000043A8: bne.w 0x000043b4
000043AC: move.b #0x6,(0x00ffe177).l
000043B4: rts
000043B6: cmpi.b #0x2,(0x00ffe186).l
000043BE: bne.w 0x000043c8
000043C2: jsr 0x0005ba4c.l ; call 0005BA4C thunk_FUN_0005ba50
000043C8: jsr 0x0000a800.l ; call 0000A800 FUN_0000a800
000043CE: tst.l (0x00ffe124).l
000043D4: beq.w 0x000043e8
000043D8: movea.l (0x00ffe124).l,A5
000043DE: bsr.w 0x00004466 ; call 00004466 FUN_00004466
000043E2: clr.l (0x00ffe124).l
000043E8: tst.l (0x00ffe120).l
000043EE: beq.w 0x0000445c
000043F2: cmpi.l #0x1e628,(0x00ffe120).l
000043FC: bne.w 0x0000441c
00004400: move.l #0x53ffc,(0x00ffe12e).l
0000440A: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00004410: move.b #0x4,(0x00ffe177).l
00004418: bra.w 0x00004442
0000441C: cmpi.l #0x1e5c4,(0x00ffe120).l
00004426: bne.w 0x00004442
0000442A: move.l #0x54286,(0x00ffe12e).l
00004434: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
0000443A: move.b #0x4,(0x00ffe177).l
00004442: movea.l (0x00ffe120).l,A5
00004448: bsr.w 0x00004466 ; call 00004466 FUN_00004466
0000444C: clr.l (0x00ffe120).l
00004452: movea.l (0x00ffddf0).l,A5
00004458: bra.w 0x00004466 ; call 00004466 FUN_00004466
0000445C: movea.l (0x00ffddf0).l,A5
00004462: bra.w 0x00004466 ; call 00004466 FUN_00004466
00004466: tst.l (A5)
00004468: beq.w 0x000044ba
0000446C: jsr 0x0001a342.l ; call 0001A342 FUN_0001a342
00004472: bcs.w 0x000044ba
00004476: move.w D0w,(0x00ffdd40).l
0000447C: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
00004482: bcs.w 0x000044ba
00004486: move.l (A5)+,D4
00004488: movea.l (A5)+,A3
0000448A: bclr.l #0x1f,D4
0000448E: beq.w 0x000044a8
00004492: clr.w (0x00ffdd42).l
00004498: movea.l A1,A6
0000449A: bsr.w 0x000044bc ; call 000044BC FUN_000044bc
0000449E: bset.b #0x5,(0x10,A1)
000044A4: bra.w 0x000044b8
000044A8: addi.w #0xc0,(0x00ffdd42).l
000044B0: bsr.w 0x000044bc ; call 000044BC FUN_000044bc
000044B4: move.l A6,(0xa8,A1)
000044B8: bra.b 0x00004466
000044BA: rts
000044BC: lea (0x7361a).l,A0 ; data 0007361a
000044C2: move.w #0x7f,D7w
000044C6: move.l A1,-(SP) ; data Stack[-0x4]
000044C8: move.w (A0)+,(A1)+
000044CA: dbf D7w,0x000044c8
000044CE: movea.l (SP)+,A1
000044D0: move.w (0x00ffdd40).l,(0x3e,A1)
000044D8: move.b #0x5,(0x10,A1)
000044DE: move.w D1w,(0x20,A1)
000044E2: move.w A2w,(0x26,A1)
000044E6: move.l A3,(0xe0,A1)
000044EA: move.w (A3)+,(A1)
000044EC: move.w (A1),(0x96,A1)
000044F0: move.w (A3),D7w
000044F2: add.w (0x00ffdd42).l,D7w
000044F8: move.w D7w,(0x4,A1)
000044FC: move.w D7w,(0x98,A1)
00004500: movea.l D4,A0
00004502: move.w (A0)+,(0xe,A1)
00004506: move.w (A0)+,(0x2c,A1)
0000450A: move.b (A0)+,(0x4c,A1)
0000450E: move.b (A0)+,(0x4d,A1)
00004512: move.b (A0)+,(0x4e,A1)
00004516: move.b (A0)+,(0x4f,A1)
0000451A: move.b (A0)+,(0x56,A1)
0000451E: move.b (A0)+,(0x57,A1)
00004522: move.l (A0)+,(0x58,A1)
00004526: move.l (A0)+,(0x5c,A1)
0000452A: move.l (A0)+,(0x60,A1)
0000452E: move.w (A0)+,(0x64,A1)
00004532: move.l (A0)+,(0x66,A1)
00004536: move.l (A0)+,(0x6a,A1)
0000453A: move.l (A0)+,(0x6e,A1)
0000453E: move.l (A0)+,(0x72,A1)
00004542: move.b (A0)+,(0x76,A1)
00004546: move.b (A0)+,(0x77,A1)
0000454A: move.b (A0)+,(0x78,A1)
0000454E: move.b (A0)+,(0x79,A1)
00004552: move.b (A0)+,(0x7a,A1)
00004556: move.b (A0)+,(0x7b,A1)
0000455A: move.b (A0)+,(0x7c,A1)
0000455E: move.b (A0)+,(0x7d,A1)
00004562: move.b (A0)+,(0x7e,A1)
00004566: move.b (A0)+,(0x7f,A1)
0000456A: move.b (A0)+,(0x80,A1)
0000456E: move.b (A0)+,(0x81,A1)
00004572: move.b (A0)+,(0x82,A1)
00004576: move.b (A0)+,(0x83,A1)
0000457A: move.b (A0)+,(0x84,A1)
0000457E: move.b (A0)+,(0x85,A1)
00004582: move.b (A0)+,(0x86,A1)
00004586: move.b (A0)+,(0x87,A1)
0000458A: move.b (A0)+,(0x88,A1)
0000458E: move.b (A0)+,(0x89,A1)
00004592: move.b (A0)+,(0x8a,A1)
00004596: move.b (A0)+,(0x8b,A1)
0000459A: move.b (A0)+,(0x8c,A1)
0000459E: move.b (A0)+,(0x8d,A1)
000045A2: move.b (A0)+,(0x8e,A1)
000045A6: move.b (A0)+,(0x9a,A1)
000045AA: move.b (A0)+,(0x9b,A1)
000045AE: move.b (A0)+,(0x9c,A1)
000045B2: move.w (A0)+,(0x90,A1)
000045B6: move.w (A0)+,(0xa0,A1)
000045BA: move.b (A0)+,(0x8f,A1)
000045BE: move.b (A0),(0x9d,A1)
000045C2: move.w D0w,(0x30,A1)
000045C6: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
000045CC: andi.w #0xf,D0w
000045D0: cmpi.w #0x37,(0x00ffddc4).l
000045D8: bne.w 0x000045e0
000045DC: move.w #0x28,D0w
000045E0: move.w D0w,(0xb6,A1)
000045E4: tst.b (0x00ffe162).l
000045EA: beq.w 0x000045f4
000045EE: bset.b #0x6,(0x9e,A1)
000045F4: cmpi.l #0x2,(A5)
000045FA: bne.w 0x00004616
000045FE: adda.w #0x4,A5
00004602: tst.w (A5)
00004604: beq.w 0x00004612
00004608: move.b (A5)+,D0b
0000460A: move.b (A5)+,D1b
0000460C: move.b D1b,(0x0,A1,D0w*0x1)
00004610: bra.b 0x00004602
00004612: adda.w #0x2,A5
00004616: rts
00004618: lea (0xff0100).l,A0 ; data 00ff0100
0000461E: move.w #0x1,(0x00ffde88).l
00004626: move.w #0xf,D7w
0000462A: btst.b #0x0,(0x10,A0)
00004630: beq.w 0x00004644
00004634: bsr.w 0x00005a56 ; call 00005A56 project_entity_screen_position
00004638: jsr 0x00005fdc.l ; call 00005FDC advance_entity_animation_accumulator
0000463E: jsr 0x0000600e.l ; call 0000600E FUN_0000600e
00004644: adda.w #0x100,A0
00004648: dbf D7w,0x0000462a
0000464C: bsr.w 0x00004658 ; call 00004658 FUN_00004658
00004650: clr.w (0x00ffde88).l
00004656: rts
00004658: bsr.w 0x00005b1e ; call 00005B1E sort_entity_slots_by_depth
0000465C: move.w #0x1,(0x00ffdd02).l
00004664: lea (0xffd986).l,A3 ; data 00ffd986
0000466A: move.w #0x2,D5w
0000466E: move.w #0xf,D7w
00004672: lea (0xffde8a).l,A4 ; data 00ffde8a
00004678: movea.l (A4)+,A0
0000467A: btst.b #0x0,(0x10,A0)
00004680: beq.w 0x000047e8
00004684: tst.w (0xc,A0)
00004688: bne.w 0x000046b0
0000468C: cmpi.b #0x6,(0xb2,A0)
00004692: bne.w 0x0000469e
00004696: bsr.w 0x0000557e ; call 0000557E FUN_0000557e
0000469A: bra.w 0x000047e8
0000469E: cmpi.b #0xd,(0xb2,A0)
000046A4: bne.w 0x000047e8
000046A8: bsr.w 0x000053c4 ; call 000053C4 FUN_000053c4
000046AC: bra.w 0x000047e8
000046B0: cmpi.l #0xafe2,(0x3a,A0)
000046B8: bne.w 0x000046c6
000046BC: jsr 0x00005936.l ; call 00005936 FUN_00005936
000046C2: bra.w 0x000046e0
000046C6: cmpi.w #0xf,(0xe,A0)
000046CC: bne.w 0x000046e0
000046D0: cmpi.l #0xafe0,(0x3a,A0)
000046D8: beq.w 0x000046e0
000046DC: bsr.w 0x000059ea ; call 000059EA FUN_000059ea
000046E0: move.b (0xb2,A0),D0b
000046E4: beq.w 0x000046f8
000046E8: ext.w D0w
000046EA: lsl.w #0x2,D0w
000046EC: lea (0x490a).l,A1 ; data 0000490a
000046F2: movea.l (0x0,A1,D0w*0x1),A1 ; data 0000490a
000046F6: jsr (A1)
000046F8: btst.b #0x5,(0x11,A0)
000046FE: beq.w 0x00004706
00004702: bsr.w 0x00004e64 ; call 00004E64 advance_entity_secondary_timer
00004706: btst.b #0x7,(0x11,A0)
0000470C: beq.w 0x00004714
00004710: bsr.w 0x00004e38 ; call 00004E38 advance_entity_primary_timer
00004714: btst.b #0x4,(0x11,A0)
0000471A: beq.w 0x0000472e
0000471E: move.w (0x16,A0),D0w
00004722: andi.w #0x7ff,D0w
00004726: cmpi.w #0x3c,D0w
0000472A: bsr.w 0x00005c7e ; call 00005C7E FUN_00005c7e
0000472E: btst.b #0x6,(0x11,A0)
00004734: beq.w 0x0000473c
00004738: bsr.w 0x00004e86 ; call 00004E86 FUN_00004e86
0000473C: btst.b #0x7,(0x10,A0)
00004742: beq.w 0x0000474a
00004746: bsr.w 0x00005b98 ; call 00005B98 FUN_00005b98
0000474A: cmpi.l #0x15390,(0x3a,A0)
00004752: bne.w 0x0000475e
00004756: tst.w (0x46,A0)
0000475A: bpl.w 0x000047e0
0000475E: btst.b #0x1,(0x10,A0)
00004764: beq.w 0x0000479c
00004768: tst.w (0x00ffdfda).l
0000476E: beq.w 0x0000479c
00004772: move.w (0x00ffdf88).l,D0w
00004778: andi.w #0xf,D0w
0000477C: cmpi.w #0x7,D0w
00004780: bne.w 0x00004798
00004784: subq.w #0x1,(0x00ffdfdc).l
0000478A: bpl.w 0x00004798
0000478E: clr.w (0x00ffdfda).l
00004794: bra.w 0x0000479c
00004798: bcc.w 0x000047a2
0000479C: jsr 0x0000622c.l ; call 0000622C expand_entity_sprite_descriptors
000047A2: cmpi.l #0x15390,(0x3a,A0)
000047AA: bne.w 0x000047e0
000047AE: tst.w (0x34,A0)
000047B2: beq.w 0x000047e0
000047B6: move.w (0xc,A0),-(SP) ; data Stack[-0x2]
000047BA: move.l (0x12,A0),-(SP) ; data Stack[-0x6]
000047BE: move.w (0xc,A0),D0w
000047C2: sub.w (0x8,A0),D0w
000047C6: move.w D0w,(0xc,A0)
000047CA: move.l #0x959a,(0x12,A0) ; data 0000959a
000047D2: jsr 0x0000622c.l ; call 0000622C expand_entity_sprite_descriptors
000047D8: move.l (SP)+,(0x12,A0)
000047DC: move.w (SP)+,(0xc,A0)
000047E0: move.w #0x1,(0x00ffd980).l
000047E8: dbf D7w,0x00004678
000047EC: lea (0xffdc9e).l,A0 ; data 00ffdc9e
000047F2: tst.w (0x2,A0)
000047F6: beq.w 0x000047fe
000047FA: bsr.w 0x00004adc ; call 00004ADC FUN_00004adc
000047FE: tst.w (0x00ffdfea).l
00004804: beq.w 0x0000480c
00004808: bsr.w 0x0000aeba ; call 0000AEBA FUN_0000aeba
0000480C: bsr.w 0x0000481e ; call 0000481E FUN_0000481e
00004810: jsr 0x0005d29c.l ; call 0005D29C FUN_0005d29c
00004816: subq.w #0x8,A3
00004818: clr.b (0x3,A3)
0000481C: rts
0000481E: lea (0xff164c).l,A0 ; data 00ff164c
00004824: move.w #0x9,D7w
00004828: btst.b #0x0,(0x10,A0)
0000482E: beq.w 0x000048a8
00004832: bsr.w 0x00005a62 ; call 00005A62 project_entity_screen_position_standard
00004836: tst.w (0xc,A0)
0000483A: beq.w 0x000048a8
0000483E: movea.l #0x48b2,A2
00004844: move.w (0x2,A0),D0w
00004848: bmi.w 0x0000486c
0000484C: cmpi.w #0x63,D0w
00004850: bcs.w 0x0000488e
00004854: movea.l #0x5faa,A2
0000485A: cmpi.w #0x64,D0w
0000485E: beq.w 0x00004896
00004862: movea.l #0x1d47c,A2
00004868: bra.w 0x00004896
0000486C: move.w D0w,D1w
0000486E: andi.w #-0x100,D1w
00004872: cmpi.w #-0x4000,D1w
00004876: beq.w 0x00004884
0000487A: movea.l #0x5fa0,A2
00004880: bra.w 0x00004896
00004884: movea.l #0x5f82,A2
0000488A: bra.w 0x00004896
0000488E: add.w D0w,D0w
00004890: add.w D0w,D0w
00004892: movea.l (0x0,A2,D0w*0x1),A2 ; data 000048b2
00004896: move.w (0xa,A0),D1w
0000489A: move.w (0xc,A0),D2w
0000489E: move.w #0x4000,D6w
000048A2: jsr 0x00001378.l ; call 00001378 FUN_00001378
000048A8: adda.w #0x12,A0
000048AC: dbf D7w,0x00004828
000048B0: rts
000049A0: rts
000049A2: lea (0xffdc9e).l,A1 ; data 00ffdc9e
000049A8: tst.w (0x2,A1)
000049AC: beq.w 0x000049be
000049B0: movem.l {  A1 A0},-(SP) ; data 00ffdc9e
000049B4: movea.l A1,A0
000049B6: bsr.w 0x00004b6c ; call 00004B6C FUN_00004b6c
000049BA: movem.l (SP)+,{  A0 A1}
000049BE: jsr 0x0001582a.l ; call 0001582A allocate_first_free_effect_slot
000049C4: bcs.w 0x00004aaa
000049C8: move.w D0w,(0x32,A1)
000049CC: move.w D1w,(0x2a,A1)
000049D0: move.w #0x1,(0x2,A1)
000049D6: move.b (0x9d,A0),D6b
000049DA: ext.w D6w
000049DC: clr.l D7
000049DE: move.b (0x57,A0),D7b
000049E2: subq.w #0x1,D7w
000049E4: move.l D7,D5
000049E6: addi.w #0x19,D5w
000049EA: clr.l D0
000049EC: move.b (0x00fff0dd).l,D0b
000049F2: mulu.w #0x190,D0
000049F6: move.w D0w,(0xe,A1)
000049FA: move.w (0x2e,A0),D0w
000049FE: rol.w #0x3,D0w
00004A00: andi.w #0x7,D0w
00004A04: move.w D0w,(0x6,A1)
00004A08: lea (0x4aac).l,A2 ; data 00004aac
00004A0E: lsl.w #0x2,D0w
00004A10: move.w (0x2,A2,D0w*0x1),D1w ; data 00004aae
00004A14: add.w (0x4,A0),D1w
00004A18: andi.w #-0x200,D1w
00004A1C: addi.w #0x100,D1w
00004A20: move.w D1w,(0x4,A1)
00004A24: move.w (0x0,A2,D0w*0x1),D0w ; data 00004aac
00004A28: add.w (A0),D0w
00004A2A: andi.w #-0x200,D0w
00004A2E: addi.w #0x100,D0w
00004A32: move.w D0w,(A1)
00004A34: lsr.w #0x4,D0w
00004A36: lsr.w #0x4,D1w
00004A38: lsl.w #0x1,D1w
00004A3A: andi.w #-0x40,D1w
00004A3E: lsr.w #0x5,D0w
00004A40: add.w D1w,D0w
00004A42: move.w D0w,D3w
00004A44: lea (0x4bb0).l,A2 ; data 00004bb0
00004A4A: move.w (0x6,A1),D0w
00004A4E: lsl.w #0x2,D0w
00004A50: movea.l (0x0,A2,D0w*0x1),A2 ; data 00004bb0
00004A54: move.w #0x4,D7w
00004A58: lea (0xff1700).l,A4 ; data 00ff1700
00004A5E: movea.l A1,A3
00004A60: adda.w #0x34,A3
00004A64: move.w D3w,(A3)+
00004A66: move.w D3w,-(SP) ; data Stack[-0x2]
00004A68: add.w (A2)+,D3w
00004A6A: move.b (0x0,A4,D3w*0x1),(A3)+ ; data 00ff1700
00004A6E: move.b #0x7e,(0x0,A4,D3w*0x1) ; data 00ff1700
00004A74: cmpi.b #0x9,(0x57,A0)
00004A7A: bgt.w 0x00004a84
00004A7E: move.b #0x7f,(0x0,A4,D3w*0x1) ; data 00ff1700
00004A84: move.w (SP)+,D3w
00004A86: dbf D7w,0x00004a66
00004A8A: movem.l {  A1 A0},-(SP)
00004A8E: movea.l A1,A0
00004A90: lea (0x5802).l,A2 ; data 00005802
00004A96: jsr 0x000061a0.l ; call 000061A0 append_entity_sprite_record
00004A9C: movem.l (SP)+,{  A0 A1}
00004AA0: move.w #0x3b,D0w
00004AA4: jsr 0x000530de.l ; call 000530DE FUN_000530de
00004AAA: rts
00004ADC: subq.w #0x1,(0xe,A0)
00004AE0: bmi.w 0x00004b6c
00004AE4: bsr.w 0x00005a56 ; call 00005A56 project_entity_screen_position
00004AE8: lea (0x4acc).l,A2 ; data 00004acc
00004AEE: move.w (0xe,A0),D0w
00004AF2: andi.w #0x3,D0w
00004AF6: lsl.w #0x2,D0w
00004AF8: movea.l (0x0,A2,D0w*0x1),A2 ; data 00004acc
00004AFC: adda.w #0x8,A2
00004B00: move.w (0xa,A0),D1w
00004B04: move.w (0xc,A0),D2w
00004B08: move.w #0x2000,D6w
00004B0C: move.w (0x2a,A0),D4w
00004B10: move.w (0x6,A0),D0w
00004B14: lsl.w #0x2,D0w
00004B16: lea (0x4c0c).l,A1 ; data 00004c0c
00004B1C: movea.l (0x0,A1,D0w*0x1),A1 ; data 00004c0c
00004B20: move.w #0x4,D7w
00004B24: movem.l {  A2 D7 D6 D4 D2 D1},-(SP)
00004B28: add.w (A1)+,D1w
00004B2A: add.w (A1)+,D2w
00004B2C: clr.l D0
00004B2E: move.w (0xe,A0),D0w
00004B32: divu.w #0x5,D0
00004B36: swap D0
00004B38: cmp.w D0w,D7w
00004B3A: beq.w 0x00004b50
00004B3E: addq.w #0x1,D0w
00004B40: cmpi.w #0x5,D0w
00004B44: bne.w 0x00004b4a
00004B48: clr.w D0w
00004B4A: cmp.w D0w,D7w
00004B4C: bne.w 0x00004b56
00004B50: jsr 0x000013be.l ; call 000013BE FUN_000013be
00004B56: movem.l (SP)+,{  D1 D2 D4 D6 D7 A2}
00004B5A: dbf D7w,0x00004b24
00004B5E: lea (0x5802).l,A2 ; data 00005802
00004B64: jsr 0x000061a0.l ; call 000061A0 append_entity_sprite_record
00004B6A: rts
00004B6C: clr.w (0x2,A0)
00004B70: move.w (0x32,A0),D0w
00004B74: move.l A3,-(SP) ; data Stack[-0x4]
00004B76: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
00004B7C: movea.l (SP)+,A3
00004B7E: lea (0x4bb0).l,A2 ; data 00004bb0
00004B84: move.w (0x6,A0),D0w
00004B88: lsl.w #0x2,D0w
00004B8A: movea.l (0x0,A2,D0w*0x1),A2 ; data 00004bb0
00004B8E: lea (0xff1700).l,A4 ; data 00ff1700
00004B94: movea.l A0,A1
00004B96: adda.w #0x34,A1
00004B9A: move.w (A1)+,D0w
00004B9C: move.w #0x4,D7w
00004BA0: move.w D0w,-(SP) ; data Stack[-0x2]
00004BA2: add.w (A2)+,D0w
00004BA4: move.b (A1)+,(0x0,A4,D0w*0x1) ; data 00ff1700
00004BA8: move.w (SP)+,D0w
00004BAA: dbf D7w,0x00004ba0
00004BAE: rts
00004C7C: ori.b #0x0,D0b
00004C80: ori.b #0x0,-(A0)
00004C84: ori.w #0x0,D0w
00004C88: ori.b #-0x20,D0b
00004C8C: ori.b #-0x40,D0b
00004C90: ori.b #0x0,D0b
00004C94: ori.b #0x0,-(A0)
00004C98: ori.w #0x0,D0w
00004C9C: ori.b #0x20,D0b
00004CA0: ori.b #0x40,D0b
00004CA4: lea (0xff0400).l,A1 ; data 00ff0400
00004CAA: move.w #0x6,D7w
00004CAE: btst.b #0x0,(0x10,A1)
00004CB4: beq.w 0x00004cca
00004CB8: btst.b #0x6,(0x10,A1)
00004CBE: bne.w 0x00004cca
00004CC2: tst.w (0xc,A1)
00004CC6: bne.w 0x00004cd8
00004CCA: adda.w #0x100,A1
00004CCE: dbf D7w,0x00004cae
00004CD2: ori #0x1,CCR
00004CD6: rts
00004CD8: andi #0x1e,CCR
00004CDC: rts
00004DCA: bset.b #0x7,(0x11,A0)
00004DD0: exg A0,A1
00004DD2: move.b (0x9d,A0),D6b
00004DD6: ext.w D6w
00004DD8: clr.l D7
00004DDA: move.b (0x57,A0),D7b
00004DDE: subq.w #0x1,D7w
00004DE0: move.l D7,D5
00004DE2: addi.w #0x19,D5w
00004DE6: exg A0,A1
00004DE8: clr.l D0
00004DEA: move.b (0x00fff0dd).l,D0b
00004DF0: mulu.w #0x8,D0
00004DF4: move.b D0b,(0x40,A0)
00004DF8: clr.b (0xb2,A0)
00004DFC: rts
00004DFE: bset.b #0x5,(0x11,A0)
00004E04: exg A0,A1
00004E06: move.b (0x9d,A0),D6b
00004E0A: ext.w D6w
00004E0C: clr.l D7
00004E0E: move.b (0x57,A0),D7b
00004E12: subq.w #0x1,D7w
00004E14: move.l D7,D5
00004E16: addi.w #0x19,D5w
00004E1A: jsr 0x000104f6.l ; call 000104F6 FUN_000104f6
00004E20: exg A0,A1
00004E22: clr.l D0
00004E24: move.b (0x00fff0dd).l,D0b
00004E2A: mulu.w #0x8,D0
00004E2E: move.b D0b,(0x41,A0)
00004E32: clr.b (0xb2,A0)
00004E36: rts
00004E38: btst.b #0x6,(0x9e,A0)
00004E3E: bne.w 0x00004e62
00004E42: move.w (0x00ffdf88).l,D0w
00004E48: andi.w #0x3f,D0w
00004E4C: beq.w 0x00004e52
00004E50: rts
00004E52: subq.b #0x1,(0x40,A0)
00004E56: bmi.w 0x00004e5c
00004E5A: rts
00004E5C: bclr.b #0x7,(0x11,A0)
00004E62: rts
00004E64: move.w (0x00ffdf88).l,D0w
00004E6A: andi.w #0x3f,D0w
00004E6E: beq.w 0x00004e74
00004E72: rts
00004E74: subq.b #0x1,(0x41,A0)
00004E78: bmi.w 0x00004e7e
00004E7C: rts
00004E7E: bclr.b #0x5,(0x11,A0)
00004E84: rts
00004E86: nop
00004E88: nop
00004E8A: nop
00004E8C: nop
00004E8E: subq.b #0x1,(0x2,A0)
00004E92: bpl.w 0x00004ec0
00004E96: move.b #0x6,(0x2,A0)
00004E9C: addq.b #0x1,(0x3,A0)
00004EA0: cmpi.b #0x4,(0x3,A0)
00004EA6: blt.w 0x00004ec0
00004EAA: move.w (0x32,A0),D0w
00004EAE: move.l A3,-(SP) ; data Stack[-0x4]
00004EB0: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
00004EB6: movea.l (SP)+,A3
00004EB8: bclr.b #0x6,(0x11,A0)
00004EBE: rts
00004EC0: lea (0x4f42).l,A2 ; data 00004f42
00004EC6: move.w (0x2e,A0),D0w
00004ECA: rol.w #0x3,D0w
00004ECC: bpl.w 0x00004ed2
00004ED0: addq.w #0x1,D0w
00004ED2: andi.w #0x7,D0w
00004ED6: add.w D0w,D0w
00004ED8: add.w D0w,D0w
00004EDA: move.w D0w,D4w
00004EDC: movea.l (0x0,A2,D0w*0x1),A2 ; data 00004f42
00004EE0: move.b (0x3,A0),D0b
00004EE4: ext.w D0w
00004EE6: add.w D0w,D0w
00004EE8: add.w D0w,D0w
00004EEA: movea.l (0x0,A2,D0w*0x1),A2
00004EEE: nop
00004EF0: nop
00004EF2: nop
00004EF4: nop
00004EF6: jsr 0x000061a0.l ; call 000061A0 append_entity_sprite_record
00004EFC: adda.w #0x8,A2
00004F00: move.w (0xa,A0),D1w
00004F04: move.w (0xc,A0),D2w
00004F08: lea (0x4f22).l,A1 ; data 00004f22
00004F0E: adda.w D4w,A1
00004F10: add.w (A1)+,D1w ; data 00004f22
00004F12: add.w (A1),D2w ; data 00004f24
00004F14: move.w #0x2000,D6w
00004F18: move.w (0x2a,A0),D4w
00004F1C: jmp 0x000013be.l ; call 000013BE FUN_000013be
000053A0: bmi.w 0x000053b4
000053A4: lsl.w #0x2,D0w
000053A6: add.w D0w,D2w
000053A8: move.w (0x00fff9f8).l,D0w
000053AE: andi.w #0x1,D0w
000053B2: add.w D0w,D1w
000053B4: move.w #0x6000,D6w
000053B8: move.w (0x2a,A0),D4w
000053BC: jsr 0x000013be.l ; call 000013BE FUN_000013be
000053C2: rts
000053C4: tst.w (0x00ffdff6).l
000053CA: bmi.w 0x000053e0
000053CE: subq.w #0x1,(0x00ffdff6).l
000053D4: bmi.w 0x000053da
000053D8: rts
000053DA: subq.w #0x1,(0x00ffdff8).l
000053E0: bsr.w 0x00005494 ; call 00005494 FUN_00005494
000053E4: tst.b (0xb2,A0)
000053E8: beq.w 0x000053ee
000053EC: rts
000053EE: movea.l A0,A1
000053F0: move.w #0x8,D0w
000053F4: adda.w #0x100,A1
000053F8: cmpi.w #0x9,(0x3e,A1)
000053FE: bge.w 0x00005434
00005402: cmpi.b #0xd,(0xde,A1)
00005408: bne.w 0x00005430
0000540C: btst.b #0x0,(0x10,A1)
00005412: beq.w 0x00005430
00005416: btst.b #0x6,(0x10,A1)
0000541C: bne.w 0x00005430
00005420: bsr.w 0x00005468 ; call 00005468 FUN_00005468
00005424: move.l A0,-(SP) ; data Stack[-0x4]
00005426: movea.l A1,A0
00005428: bsr.w 0x0000aff2 ; call 0000AFF2 prepare_entity_auxiliary_behavior
0000542C: movea.l (SP)+,A0
0000542E: rts
00005430: dbf D0w,0x000053f4
00005434: tst.w (0x00ffdff8).l
0000543A: bpl.w 0x00005454
0000543E: move.w (0x32,A0),D0w
00005442: move.l A3,-(SP) ; data Stack[-0x4]
00005444: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
0000544A: movea.l (SP)+,A3
0000544C: move.b #-0x3,D1b
00005450: bra.w 0x00005622 ; call 00005622 FUN_00005622
00005454: move.w #0x1e,(0x00ffdff6).l
0000545C: movea.l (0x00ffdffa).l,A1
00005462: bsr.w 0x00005468 ; call 00005468 FUN_00005468
00005466: rts
00005468: move.w #0x4,(0x00ffe004).l
00005470: move.b #0xd,(0xb2,A1)
00005476: move.w (0x32,A0),(0x32,A1)
0000547C: move.w (0x2a,A0),(0x2a,A1)
00005482: move.b #0x5,(0x2,A1)
00005488: clr.b (0x3,A1)
0000548C: move.l (0xd8,A0),(0xd8,A1)
00005492: rts
00005494: clr.w D1w
00005496: subq.b #0x1,(0x2,A0)
0000549A: bpl.w 0x000054ac
0000549E: move.b #0x6,(0x2,A0)
000054A4: addq.b #0x1,(0x3,A0)
000054A8: move.w #0x1,D1w
000054AC: move.b (0x3,A0),D0b
000054B0: ext.w D0w
000054B2: cmpi.w #0x3,D0w
000054B6: blt.w 0x00005510
000054BA: subq.w #0x1,(0x00ffe004).l
000054C0: bmi.w 0x0000550a
000054C4: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
000054CA: move.w D0w,(0x00ffe002).l
000054D0: move.w D0w,D1w
000054D2: andi.w #0x7,D0w
000054D6: btst.l #0x1,D0
000054DA: beq.w 0x000054e0
000054DE: neg.w D0w
000054E0: move.w D0w,(0x00ffdffe).l
000054E6: andi.w #0x7,D1w
000054EA: btst.l #0x2,D1
000054EE: beq.w 0x000054f4
000054F2: neg.w D1w
000054F4: move.w D1w,(0x00ffe000).l
000054FA: clr.b (0x3,A0)
000054FE: move.b #0x5,(0x2,A0)
00005504: clr.w D0w
00005506: bra.w 0x00005510
0000550A: clr.b (0xb2,A0)
0000550E: rts
00005510: lsl.w #0x2,D0w
00005512: lea (0x585e).l,A2 ; data 0000585e
00005518: btst.b #0x4,(0x00ffe002).l
00005520: beq.w 0x0000552a
00005524: lea (0x586a).l,A2 ; data 0000586a
0000552A: movea.l (0x0,A2,D0w*0x1),A2
0000552E: tst.w D1w
00005530: beq.w 0x0000553a
00005534: jsr 0x000061a0.l ; call 000061A0 append_entity_sprite_record
0000553A: adda.w #0x8,A2
0000553E: move.w (0xa,A0),D1w
00005542: move.w (0xc,A0),D2w
00005546: add.w (0x00ffdffe).l,D1w
0000554C: add.w (0x00ffe000).l,D2w
00005552: move.b (0x3,A0),D0b
00005556: ext.w D0w
00005558: subq.w #0x4,D0w
0000555A: bmi.w 0x0000556e
0000555E: lsl.w #0x2,D0w
00005560: add.w D0w,D2w
00005562: move.w (0x00fff9f8).l,D0w
00005568: andi.w #0x1,D0w
0000556C: add.w D0w,D1w
0000556E: move.w #0x6000,D6w
00005572: move.w (0x2a,A0),D4w
00005576: jsr 0x000013be.l ; call 000013BE FUN_000013be
0000557C: rts
0000557E: tst.w (0x00ffdff2).l
00005584: bmi.w 0x0000559a
00005588: subq.w #0x1,(0x00ffdff2).l
0000558E: bmi.w 0x00005594
00005592: rts
00005594: subq.w #0x1,(0x00ffdff4).l
0000559A: bsr.w 0x00005662 ; call 00005662 FUN_00005662
0000559E: tst.b (0xb2,A0)
000055A2: beq.w 0x000055a8
000055A6: rts
000055A8: movea.l A0,A1
000055AA: move.w #0x8,D0w
000055AE: adda.w #0x100,A1
000055B2: cmpi.w #0x9,(0x3e,A1)
000055B8: bge.w 0x000055ee
000055BC: cmpi.b #0x6,(0xde,A1)
000055C2: bne.w 0x000055ea
000055C6: btst.b #0x0,(0x10,A1)
000055CC: beq.w 0x000055ea
000055D0: btst.b #0x6,(0x10,A1)
000055D6: bne.w 0x000055ea
000055DA: bsr.w 0x0000563e ; call 0000563E FUN_0000563e
000055DE: move.l A0,-(SP) ; data Stack[-0x4]
000055E0: movea.l A1,A0
000055E2: bsr.w 0x0000aff2 ; call 0000AFF2 prepare_entity_auxiliary_behavior
000055E6: movea.l (SP)+,A0
000055E8: rts
000055EA: dbf D0w,0x000055ae
000055EE: tst.w (0x00ffdff4).l
000055F4: bpl.w 0x0000560e
000055F8: move.w (0x32,A0),D0w
000055FC: move.l A3,-(SP) ; data Stack[-0x4]
000055FE: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
00005604: movea.l (SP)+,A3
00005606: move.b #-0x2,D1b
0000560A: bra.w 0x00005622 ; call 00005622 FUN_00005622
0000560E: move.w #0x1e,(0x00ffdff2).l
00005616: movea.l (0x00ffdffa).l,A1
0000561C: bsr.w 0x0000563e ; call 0000563E FUN_0000563e
00005620: rts
00005622: lea (0xff0100).l,A1 ; data 00ff0100
00005628: move.w #0x9,D0w
0000562C: clr.b (0xde,A1)
00005630: and.b D1b,(0xdf,A1)
00005634: adda.w #0x100,A1
00005638: dbf D0w,0x0000562c
0000563C: rts
0000563E: move.b #0x6,(0xb2,A1)
00005644: move.w (0x32,A0),(0x32,A1)
0000564A: move.w (0x2a,A0),(0x2a,A1)
00005650: move.b #0x5,(0x2,A1)
00005656: clr.b (0x3,A1)
0000565A: move.l (0xd8,A0),(0xd8,A1)
00005660: rts
00005662: clr.w D1w
00005664: subq.b #0x1,(0x2,A0)
00005668: bpl.w 0x0000567a
0000566C: move.b #0x6,(0x2,A0)
00005672: addq.b #0x1,(0x3,A0)
00005676: move.w #0x1,D1w
0000567A: move.b (0x3,A0),D0b
0000567E: ext.w D0w
00005680: cmpi.w #0x5,D0w
00005684: blt.w 0x0000568e
00005688: clr.b (0xb2,A0)
0000568C: rts
0000568E: lsl.w #0x2,D0w
00005690: lea (0x584a).l,A2 ; data 0000584a
00005696: movea.l (0x0,A2,D0w*0x1),A2 ; data 0000584a
0000569A: tst.w D1w
0000569C: beq.w 0x000056a6
000056A0: jsr 0x000061a0.l ; call 000061A0 append_entity_sprite_record
000056A6: adda.w #0x8,A2
000056AA: move.w (0xa,A0),D1w
000056AE: move.w (0xc,A0),D2w
000056B2: move.b (0x3,A0),D0b
000056B6: ext.w D0w
000056B8: subq.w #0x4,D0w
000056BA: bmi.w 0x000056ce
000056BE: lsl.w #0x2,D0w
000056C0: add.w D0w,D2w
000056C2: move.w (0x00fff9f8).l,D0w
000056C8: andi.w #0x1,D0w
000056CC: add.w D0w,D1w
000056CE: move.w #0x6000,D6w
000056D2: move.w (0x2a,A0),D4w
000056D6: jsr 0x000013be.l ; call 000013BE FUN_000013be
000056DC: rts
00005750: ori.b #-0x42,D0b
00005754: rts
00005936: cmpi.w #0xf,(0xe,A0)
0000593C: beq.w 0x00005988
00005940: move.w (0xa,A0),D1w
00005944: move.w (0xc,A0),D2w
00005948: move.w #0x4000,D6w
0000594C: lea (0x5a46).l,A2 ; data 00005a46
00005952: subq.b #0x1,(0x6,A0)
00005956: bpl.w 0x00005972
0000595A: move.b #0x3,(0x6,A0)
00005960: addq.b #0x1,(0x7,A0)
00005964: cmpi.b #0x4,(0x7,A0)
0000596A: blt.w 0x00005972
0000596E: clr.b (0x7,A0)
00005972: move.b (0x7,A0),D0b
00005976: ext.w D0w
00005978: andi.w #0x3,D0w
0000597C: lsl.w #0x2,D0w
0000597E: movea.l (0x0,A2,D0w*0x1),A2 ; data 00005a46
00005982: jmp 0x00001378.l ; call 00001378 FUN_00001378
00005988: move.w (0xa,A0),D1w
0000598C: move.w (0xc,A0),D2w
00005990: move.w #0x4000,D6w
00005994: lea (0x5a46).l,A2 ; data 00005a46
0000599A: subq.b #0x1,(0x6,A0)
0000599E: bpl.w 0x000059ba
000059A2: move.b #0x3,(0x6,A0)
000059A8: addq.b #0x1,(0x7,A0)
000059AC: cmpi.b #0x4,(0x7,A0)
000059B2: blt.w 0x000059ba
000059B6: clr.b (0x7,A0)
000059BA: move.b (0x7,A0),D0b
000059BE: ext.w D0w
000059C0: andi.w #0x3,D0w
000059C4: lsl.w #0x2,D0w
000059C6: movea.l (0x0,A2,D0w*0x1),A2 ; data 00005a46
000059CA: subi.w #0x20,D2w
000059CE: subi.w #0x2e,D1w
000059D2: movem.l {  A2 D2 D1},-(SP)
000059D6: jsr 0x00001378.l ; call 00001378 FUN_00001378
000059DC: movem.l (SP)+,{  D1 D2 A2}
000059E0: addi.w #0x57,D1w
000059E4: jmp 0x00001378.l ; call 00001378 FUN_00001378
000059EA: move.w (0xa,A0),D1w
000059EE: move.w (0xc,A0),D2w
000059F2: move.w #0x2000,D6w
000059F6: lea (0x5a36).l,A2 ; data 00005a36
000059FC: subq.b #0x1,(0x6,A0)
00005A00: bpl.w 0x00005a1c
00005A04: move.b #0x3,(0x6,A0)
00005A0A: addq.b #0x1,(0x7,A0)
00005A0E: cmpi.b #0x4,(0x7,A0)
00005A14: blt.w 0x00005a1c
00005A18: clr.b (0x7,A0)
00005A1C: move.b (0x7,A0),D0b
00005A20: ext.w D0w
00005A22: andi.w #0x3,D0w
00005A26: lsl.w #0x2,D0w
00005A28: movea.l (0x0,A2,D0w*0x1),A2 ; data 00005a36
00005A2C: addi.w #0xc,D2w
00005A30: jmp 0x00001378.l ; call 00001378 FUN_00001378
00005A56: cmpi.l #0x15390,(0x3a,A0)
00005A5E: beq.w 0x00005ac6
00005A62: move.w (A0),D0w
00005A64: sub.w (0x00ffddc6).l,D0w
00005A6A: asr.w #0x4,D0w
00005A6C: bcc.w 0x00005a72
00005A70: addq.w #0x1,D0w
00005A72: addi.w #0x80,D0w
00005A76: move.w D0w,(0xa,A0)
00005A7A: move.w (0x4,A0),D1w
00005A7E: sub.w (0x00ffddc8).l,D1w
00005A84: asr.w #0x4,D1w
00005A86: bcc.w 0x00005a8c
00005A8A: addq.w #0x1,D1w
00005A8C: addi.w #0x80,D1w
00005A90: add.w (0x8,A0),D1w
00005A94: move.w D1w,(0xc,A0)
00005A98: cmpi.w #0x70,D0w
00005A9C: blt.w 0x00005abe
00005AA0: cmpi.w #0x1ad,D0w
00005AA4: bhi.w 0x00005abe
00005AA8: cmpi.w #0x63,D1w
00005AAC: blt.w 0x00005abe
00005AB0: cmpi.w #0x17a,D1w
00005AB4: bcc.w 0x00005abe
00005AB8: move.w #0x1,D0w
00005ABC: rts
00005ABE: move.w #0x0,(0xc,A0)
00005AC4: rts
00005AC6: move.w (A0),D0w
00005AC8: sub.w (0x00ffddc6).l,D0w
00005ACE: asr.w #0x4,D0w
00005AD0: bcc.w 0x00005ad6
00005AD4: addq.w #0x1,D0w
00005AD6: addi.w #0x80,D0w
00005ADA: bmi.w 0x00005b10
00005ADE: cmpi.w #0x200,D0w
00005AE2: bcc.w 0x00005b10
00005AE6: move.w D0w,(0xa,A0)
00005AEA: move.w (0x4,A0),D1w
00005AEE: sub.w (0x00ffddc8).l,D1w
00005AF4: asr.w #0x4,D1w
00005AF6: bcc.w 0x00005afc
00005AFA: addq.w #0x1,D1w
00005AFC: addi.w #0x80,D1w
00005B00: add.w (0x8,A0),D1w
00005B04: bmi.w 0x00005b10
00005B08: cmpi.w #0x1a0,D1w
00005B0C: bcs.w 0x00005b18
00005B10: move.w #0x0,(0xc,A0)
00005B16: rts
00005B18: move.w D1w,(0xc,A0)
00005B1C: rts
00005B1E: lea (0xffde8a).l,A4 ; data 00ffde8a
00005B24: clr.w D4w
00005B26: move.w #0xe,D3w
00005B2A: movea.l (A4)+,A0
00005B2C: movea.l (A4)+,A1
00005B2E: move.w (0x4,A0),D0w
00005B32: btst.b #0x4,(0x10,A0)
00005B38: bne.w 0x00005b46
00005B3C: btst.b #0x6,(0x10,A0)
00005B42: beq.w 0x00005b48
00005B46: clr.w D0w
00005B48: move.w (0x4,A1),D1w
00005B4C: btst.b #0x4,(0x10,A1)
00005B52: bne.w 0x00005b60
00005B56: btst.b #0x6,(0x10,A1)
00005B5C: beq.w 0x00005b62
00005B60: clr.w D1w
00005B62: cmp.w D1w,D0w
00005B64: bge.b 0x00005b72
00005B66: subq.w #0x8,A4
00005B68: move.l A1,(A4)+
00005B6A: move.l A0,(A4)+
00005B6C: move.w #0x1,D4w
00005B70: bra.b 0x00005b74
00005B72: exg A0,A1
00005B74: dbf D3w,0x00005b2c
00005B78: tst.w D4w
00005B7A: bne.b 0x00005b1e
00005B7C: rts
00005B7E: lea (0xff0100).l,A0 ; data 00ff0100
00005B84: move.w #0xf,D7w
00005B88: bset.b #0x0,(0x11,A0)
00005B8E: adda.w #0x100,A0
00005B92: dbf D7w,0x00005b88
00005B96: rts
00005B98: cmpi.w #0x2a,(0x38,A0)
00005B9E: beq.w 0x00005bb0
00005BA2: move.w (0x2c,A0),D6w
00005BA6: move.w (0x44,A0),D4w
00005BAA: subq.w #0x8,D4w
00005BAC: bpl.w 0x00005bb2
00005BB0: rts
00005BB2: lea (0x5bee).l,A1 ; data 00005bee
00005BB8: lea (0x5c2e).l,A6 ; data 00005c2e
00005BBE: move.w D4w,D1w
00005BC0: andi.w #0x3,D1w
00005BC4: lsl.w #0x2,D1w
00005BC6: adda.w D1w,A6
00005BC8: move.w #0xf,D3w
00005BCC: movea.l (A6)+,A2 ; data 00005c2e
00005BCE: move.w (0xa,A0),D0w
00005BD2: move.w (A1)+,D1w
00005BD4: mulu.w D4w,D1
00005BD6: add.w D0w,D1w
00005BD8: move.w (0xc,A0),D0w
00005BDC: move.w (A1)+,D2w
00005BDE: mulu.w D4w,D2
00005BE0: add.w D0w,D2w
00005BE2: jsr 0x00001378.l ; call 00001378 FUN_00001378
00005BE8: dbf D3w,0x00005bcc
00005BEC: rts
00005C7E: move.b (0x4c,A0),D0b
00005C82: movea.l (0x00fffc04).l,A1
00005C88: tst.b (0x56,A1)
00005C8C: bpl.w 0x00005c9e
00005C90: cmpi.b #0x5,(0x57,A1)
00005C96: beq.w 0x00005cba
00005C9A: bra.w 0x00005cbe
00005C9E: tst.b (0x57,A1)
00005CA2: bne.w 0x00005cbe
00005CA6: btst.b #0x3,(0x91,A1)
00005CAC: bne.w 0x00005cbe
00005CB0: btst.b #0x2,(0x91,A1)
00005CB6: bne.w 0x00005cbe
00005CBA: move.b (0x9c,A0),D0b
00005CBE: lea (0x5d92).l,A5 ; data 00005d92
00005CC4: cmpi.b #0x3,D0b
00005CC8: blt.w 0x00005cee
00005CCC: lea (0x5dbe).l,A5 ; data 00005dbe
00005CD2: cmpi.b #0x6,D0b
00005CD6: blt.w 0x00005cee
00005CDA: lea (0x5da8).l,A5 ; data 00005da8
00005CE0: cmpi.b #0x9,D0b
00005CE4: blt.w 0x00005cee
00005CE8: lea (0x5dd4).l,A5 ; data 00005dd4
00005CEE: move.w (A5)+,D6w
00005CF0: move.w (0xa,A0),D1w
00005CF4: move.w (0xc,A0),D2w
00005CF8: subi.w #0xc,D2w
00005CFC: cmpi.w #0xf,(0xe,A0)
00005D02: bne.w 0x00005d0a
00005D06: subi.w #0x20,D2w
00005D0A: move.w (0xa6,A0),D0w
00005D0E: bpl.w 0x00005d76
00005D12: subq.w #0x1,D0w
00005D14: bmi.w 0x00005d20
00005D18: clr.w (0xa6,A0)
00005D1C: bra.w 0x00005d76
00005D20: move.w D0w,(0xa6,A0)
00005D24: andi.w #0x7f,D0w
00005D28: mulu.w #0x8,D0
00005D2C: movem.w {  D2w D1w D0w},-(SP)
00005D30: sub.w D0w,D1w
00005D32: sub.w D0w,D2w
00005D34: movea.l (A5)+,A2
00005D36: jsr 0x00001378.l ; call 00001378 FUN_00001378
00005D3C: movem.w (SP)+,{ D0 D1 D2}
00005D40: movem.w {  D2w D1w D0w},-(SP)
00005D44: add.w D0w,D1w
00005D46: sub.w D0w,D2w
00005D48: movea.l (A5)+,A2
00005D4A: jsr 0x00001378.l ; call 00001378 FUN_00001378
00005D50: movem.w (SP)+,{ D0 D1 D2}
00005D54: movem.w {  D2w D1w D0w},-(SP)
00005D58: sub.w D0w,D1w
00005D5A: add.w D0w,D2w
00005D5C: movea.l (A5)+,A2
00005D5E: jsr 0x00001378.l ; call 00001378 FUN_00001378
00005D64: movem.w (SP)+,{ D0 D1 D2}
00005D68: add.w D0w,D1w
00005D6A: add.w D0w,D2w
00005D6C: movea.l (A5)+,A2
00005D6E: jsr 0x00001378.l ; call 00001378 FUN_00001378
00005D74: rts
00005D76: adda.w #0x10,A5
00005D7A: movea.l (A5),A2
00005D7C: jmp 0x00001378.l ; call 00001378 FUN_00001378
00005FDC: move.w (0x1e,A0),(0x00ffdd3e).l
00005FE4: move.w (0x1a,A0),D0w
00005FE8: move.w D0w,(0x00ffdd3c).l
00005FEE: add.w (0x18,A0),D0w
00005FF2: cmp.w (0x1c,A0),D0w
00005FF6: blt.w 0x00006008
00005FFA: addq.w #0x1,(0x1e,A0)
00005FFE: bset.b #0x0,(0x11,A0)
00006004: sub.w (0x1c,A0),D0w
00006008: move.w D0w,(0x1a,A0)
0000600C: rts
0000600E: move.w (0xe,A0),D0w
00006012: lsl.w #0x2,D0w
00006014: lea (0x62ac).l,A1 ; data 000062ac
0000601A: movea.l (0x0,A1,D0w*0x1),A1 ; data 000062ac
0000601E: move.w (0x16,A0),D0w
00006022: andi.w #0x7fff,D0w
00006026: movea.l (0x0,A1,D0w*0x1),A2
0000602A: move.w (A2)+,D0w
0000602C: bpl.w 0x00006060
00006030: andi.w #0x7fff,D0w
00006034: cmp.w (0x1e,A0),D0w
00006038: ble.w 0x00006048
0000603C: move.w (0x1e,A0),D0w
00006040: move.w (A2)+,(0x18,A0)
00006044: bra.w 0x000060ae
00006048: ori.w #-0x8000,(0x16,A0)
0000604E: subq.w #0x1,D0w
00006050: move.w D0w,(0x1e,A0)
00006054: move.w (0x1e,A0),D0w
00006058: move.w (A2)+,(0x18,A0)
0000605C: bra.w 0x000060ae
00006060: cmp.w (0x1e,A0),D0w
00006064: bgt.w 0x0000606c
00006068: clr.w (0x1e,A0)
0000606C: move.w (0x1e,A0),D0w
00006070: adda.w #0x2,A2
00006074: cmpi.w #0xd,(0xe,A0)
0000607A: bne.w 0x00006092
0000607E: move.w #0x180,D1w
00006082: cmpi.w #0x2,D0w
00006086: bne.w 0x000060aa
0000608A: move.w #0x90,D1w
0000608E: bra.w 0x000060aa
00006092: move.w (0xca,A0),D1w
00006096: cmpi.w #0x16,D1w
0000609A: bcc.w 0x000060a6
0000609E: mulu.w #0x1c,D1
000060A2: bra.w 0x000060aa
000060A6: mulu.w #0x10,D1
000060AA: move.w D1w,(0x18,A0)
000060AE: lsl.w #0x2,D0w
000060B0: movea.l (0x0,A2,D0w*0x1),A2
000060B4: cmpi.w #-0x7ffe,(A2)
000060B8: bne.w 0x000060ee
000060BC: btst.b #0x0,(0x11,A0)
000060C2: beq.w 0x000060ee
000060C6: bsr.w 0x000060f4 ; call 000060F4 FUN_000060f4
000060CA: bcc.w 0x000060dc
000060CE: bclr.b #0x0,(0x11,A0)
000060D4: bsr.w 0x000060fa ; call 000060FA FUN_000060fa
000060D8: bra.w 0x000060ee
000060DC: move.w (0x00ffdd3e).l,(0x1e,A0)
000060E4: move.w (0x00ffdd3c).l,(0x1a,A0)
000060EC: rts
000060EE: move.l A2,(0x12,A0)
000060F2: rts
000060F4: ori #0x1,CCR
000060F8: rts
000060FA: btst.b #0x7,(0x11,A0)
00006100: beq.w 0x00006112
00006104: tst.w (0xc,A0)
00006108: beq.w 0x00006146
0000610C: jsr 0x00003efe.l ; call 00003EFE FUN_00003efe
00006112: btst.b #0x5,(0x11,A0)
00006118: beq.w 0x00006122
0000611C: jsr 0x00003e86.l ; call 00003E86 expand_packed_nibbles
00006122: movea.l (0x2,A2),A5
00006126: move.l A5,(0x22,A0)
0000612A: move.w (0x6,A2),D1w
0000612E: move.w D1w,(0x28,A0)
00006132: movea.l (0x00ffdf2c).l,A1
00006138: move.l A0,(A1)
0000613A: addq.b #0x4,(0x00ffdf2f).l
00006140: addq.w #0x1,(0x00ffdf2a).l
00006146: rts
00006148: move.w (0x00ffdf2a).l,D7w
0000614E: move.w D7w,D0w
00006150: subq.w #0x1,D7w
00006152: move.l (0x00ffdf2c).l,D1
00006158: lsl.b #0x2,D0b
0000615A: sub.b D0b,D1b
0000615C: movea.l D1,A1
0000615E: movea.l (A1),A0
00006160: move.l (0x22,A0),D0
00006164: btst.b #0x7,(0x11,A0)
0000616A: beq.w 0x00006172
0000616E: move.l (0x44,A0),D0
00006172: btst.b #0x5,(0x11,A0)
00006178: beq.w 0x00006180
0000617C: move.l (0x48,A0),D0
00006180: move.w (0x26,A0),D1w
00006184: move.w (0x28,A0),D2w
00006188: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0000618E: move.l A1,D0
00006190: addq.b #0x4,D0b
00006192: movea.l D0,A1
00006194: dbf D7w,0x0000615e
00006198: clr.w (0x00ffdf2a).l
0000619E: rts
000061A0: movea.l (0x00ffdf32).l,A1
000061A6: movea.l (0x2,A2),A5
000061AA: move.l A5,(A1)
000061AC: move.w (0x6,A2),D1w
000061B0: move.w D1w,(0x4,A1)
000061B4: move.w (0x2a,A0),D1w
000061B8: lsl.w #0x5,D1w
000061BA: move.w D1w,(0x6,A1)
000061BE: addq.b #0x8,(0x00ffdf35).l
000061C4: addq.w #0x1,(0x00ffdf30).l
000061CA: rts
000061CC: movea.l (0x00ffdf32).l,A1
000061D2: movea.l (0x2,A2),A5
000061D6: move.l A5,(A1)
000061D8: move.w (0x6,A2),D1w
000061DC: move.w D1w,(0x4,A1)
000061E0: lsl.w #0x5,D0w
000061E2: move.w D0w,(0x6,A1)
000061E6: addq.b #0x8,(0x00ffdf35).l
000061EC: addq.w #0x1,(0x00ffdf30).l
000061F2: rts
000061F4: move.w (0x00ffdf30).l,D7w
000061FA: move.w D7w,D0w
000061FC: subq.w #0x1,D7w
000061FE: move.l (0x00ffdf32).l,D1
00006204: lsl.b #0x3,D0b
00006206: sub.b D0b,D1b
00006208: movea.l D1,A1
0000620A: move.l (A1),D0
0000620C: move.w (0x4,A1),D2w
00006210: move.w (0x6,A1),D1w
00006214: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0000621A: move.l A1,D0
0000621C: addq.b #0x8,D0b
0000621E: movea.l D0,A1
00006220: dbf D7w,0x0000620a
00006224: clr.w (0x00ffdf30).l
0000622A: rts
0000622C: clr.w D6w
0000622E: movea.l (0x12,A0),A2
00006232: move.w (A2),D0w
00006234: cmpi.w #-0x7ffe,D0w
00006238: beq.w 0x00006242
0000623C: bsr.w 0x0000624c ; call 0000624C expand_entity_sprite_descriptor_list
00006240: rts
00006242: moveq #0x1,D6
00006244: addq.l #0x8,A2
00006246: bsr.w 0x0000624c ; call 0000624C expand_entity_sprite_descriptor_list
0000624A: rts
0000624C: move.w (A2)+,D0w
0000624E: cmpi.w #-0x8000,D0w
00006252: bne.w 0x00006258
00006256: rts
00006258: add.w (0xa,A0),D0w
0000625C: move.w (A2)+,D1w
0000625E: add.w (0xc,A0),D1w
00006262: move.w D0w,(0x6,A3)
00006266: move.w D1w,(A3)
00006268: move.w (A2)+,D0w
0000626A: move.b D0b,(0x2,A3)
0000626E: move.b D5b,(0x3,A3)
00006272: move.w (A2)+,D0w
00006274: btst.b #0x7,(0x11,A0)
0000627A: bne.w 0x00006282
0000627E: or.w (0x2c,A0),D0w
00006282: tst.w D6w
00006284: beq.w 0x0000628c
00006288: add.w (0x20,A0),D0w
0000628C: move.w D0w,(0x4,A3)
00006290: addq.w #0x1,D5w
00006292: addq.w #0x1,(0x00ffdd02).l
00006298: addq.l #0x8,A3
0000629A: move.l A3,(0x00ffdd18).l
000062A0: move.w (0x00ffdd02).l,(0x00ffdd1c).l
000062AA: bra.b 0x0000624c
0000A7B4: move.l #0xff0100,(0x00fffc04).l
0000A7BE: move.l #0xff0000,(0x00ffdf2c).l
0000A7C8: clr.w (0x00ffdf2a).l
0000A7CE: move.l #0xff1480,(0x00ffdf32).l
0000A7D8: clr.w (0x00ffdf30).l
0000A7DE: bsr.w 0x0000a7e4 ; call 0000A7E4 initialize_entity_slot_order
0000A7E2: rts
0000A7E4: lea (0xff0100).l,A1 ; data 00ff0100
0000A7EA: lea (0xffde8a).l,A0 ; data 00ffde8a
0000A7F0: move.w #0xf,D7w
0000A7F4: move.l A1,(A0)+
0000A7F6: adda.w #0x100,A1
0000A7FA: dbf D7w,0x0000a7f4
0000A7FE: rts
0000A800: tst.b (0x00ffe180).l
0000A806: beq.w 0x0000a82c
0000A80A: cmpi.w #0x4,(0x00fff0c8).l
0000A812: bne.w 0x0000a8ac
0000A816: movea.l #0xa8ae,A3
0000A81C: move.l A3,(0x00ffe124).l
0000A822: move.l (A3),(0x00ffe110).l
0000A828: bra.w 0x0000a8ac
0000A82C: tst.b (0x00ffe182).l
0000A832: beq.w 0x0000a84c
0000A836: movea.l #0xa8cc,A3
0000A83C: move.l A3,(0x00ffe124).l
0000A842: move.l (A3),(0x00ffe110).l
0000A848: bra.w 0x0000a8ac
0000A84C: cmpi.b #0x5,(0x00fffbed).l
0000A854: bne.w 0x0000a8ac
0000A858: cmpi.w #0x2,(0x00ffdf40).l
0000A860: bne.w 0x0000a8ac
0000A864: tst.b (0x00ffe183).l
0000A86A: bne.w 0x0000a880
0000A86E: move.w #0x2,D1w
0000A872: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000A878: addq.w #0x1,D0w
0000A87A: move.b D0b,(0x00ffe183).l
0000A880: move.w (0x00ffe12c).l,D0w
0000A886: cmp.w (0x00fff0c8).l,D0w
0000A88C: bne.w 0x0000a8ac
0000A890: move.l (0x00fffc3c).l,D4
0000A896: bne.w 0x0000a8ac
0000A89A: jsr 0x0005ba98.l ; call 0005BA98 FUN_0005ba98
0000A8A0: move.l A3,(0x00ffe124).l
0000A8A6: move.l (A3),(0x00ffe110).l
0000A8AC: rts
0000A8EA: jsr 0x00013874.l ; call 00013874 FUN_00013874
0000A8F0: lea (0xff0100).l,A0 ; data 00ff0100
0000A8F6: move.w #0xf,D7w
0000A8FA: move.w D7w,-(SP) ; data Stack[-0x2]
0000A8FC: btst.b #0x0,(0x10,A0)
0000A902: beq.w 0x0000aac6
0000A906: tst.w (0xdc,A0)
0000A90A: bmi.w 0x0000a912
0000A90E: subq.w #0x1,(0xdc,A0)
0000A912: btst.b #0x2,(0x10,A0)
0000A918: beq.w 0x0000aa54
0000A91C: btst.b #0x6,(0x10,A0)
0000A922: bne.w 0x0000aa74
0000A926: btst.b #0x4,(0x11,A0)
0000A92C: beq.w 0x0000a93a
0000A930: jsr 0x00015bb4.l ; call 00015BB4 FUN_00015bb4
0000A936: move.b D0b,(0xb3,A0)
0000A93A: move.w (0x16,A0),D0w
0000A93E: bmi.w 0x0000a9a6
0000A942: cmpi.w #0x3c,D0w
0000A946: bgt.w 0x0000aa74
0000A94A: tst.b (0x9c,A0)
0000A94E: bne.w 0x0000a978
0000A952: bset.b #0x4,(0x10,A0)
0000A958: beq.w 0x0000aa4c
0000A95C: bsr.w 0x0000ab02 ; call 0000AB02 FUN_0000ab02
0000A960: btst.b #0x1,(0x10,A0)
0000A966: beq.w 0x0000a974
0000A96A: move.l A0,-(SP) ; data Stack[-0x6]
0000A96C: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000A972: movea.l (SP)+,A0
0000A974: bra.w 0x0000aa74
0000A978: tst.b (0x4c,A0)
0000A97C: bne.w 0x0000aa54
0000A980: bset.b #0x4,(0x10,A0)
0000A986: beq.w 0x0000aa4c
0000A98A: bsr.w 0x0000ab02 ; call 0000AB02 FUN_0000ab02
0000A98E: btst.b #0x1,(0x10,A0)
0000A994: beq.w 0x0000a9a2
0000A998: move.l A0,-(SP) ; data Stack[-0x6]
0000A99A: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000A9A0: movea.l (SP)+,A0
0000A9A2: bra.w 0x0000aa74
0000A9A6: cmpi.l #0xb0d4,(0x3a,A0)
0000A9AE: beq.w 0x0000a9be
0000A9B2: cmpi.l #0xb10c,(0x3a,A0)
0000A9BA: bne.w 0x0000a9cc
0000A9BE: bsr.w 0x0000ab02 ; call 0000AB02 FUN_0000ab02
0000A9C2: jsr 0x0001e514.l ; call 0001E514 FUN_0001e514
0000A9C8: bra.w 0x0000aa74
0000A9CC: tst.b (0x9c,A0)
0000A9D0: bne.w 0x0000aa02
0000A9D4: bset.b #0x4,(0x10,A0)
0000A9DA: beq.w 0x0000aa4c
0000A9DE: cmpi.w #0xe,(0xe,A0)
0000A9E4: beq.b 0x0000a9be
0000A9E6: bsr.w 0x0000ab02 ; call 0000AB02 FUN_0000ab02
0000A9EA: btst.b #0x1,(0x10,A0)
0000A9F0: beq.w 0x0000a9fe
0000A9F4: move.l A0,-(SP) ; data Stack[-0x6]
0000A9F6: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000A9FC: movea.l (SP)+,A0
0000A9FE: bra.w 0x0000aa74
0000AA02: tst.b (0x4c,A0)
0000AA06: bne.w 0x0000aa54
0000AA0A: bset.b #0x4,(0x10,A0)
0000AA10: beq.w 0x0000aa4c
0000AA14: cmpi.w #0xe,(0xe,A0)
0000AA1A: beq.b 0x0000a9be
0000AA1C: cmpi.w #0xc,(0xe,A0)
0000AA22: bne.w 0x0000aa30
0000AA26: cmpi.w #0x80,(0x16,A0)
0000AA2C: beq.w 0x0000aa74
0000AA30: bsr.w 0x0000ab02 ; call 0000AB02 FUN_0000ab02
0000AA34: btst.b #0x1,(0x10,A0)
0000AA3A: beq.w 0x0000aa48
0000AA3E: move.l A0,-(SP) ; data Stack[-0x6]
0000AA40: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000AA46: movea.l (SP)+,A0
0000AA48: bra.w 0x0000aa74
0000AA4C: bsr.w 0x0000b330 ; call 0000B330 enter_entity_behavior_transition
0000AA50: bra.w 0x0000aa74
0000AA54: btst.b #0x1,(0x10,A0)
0000AA5A: beq.w 0x0000aa66
0000AA5E: bsr.w 0x0000acae ; call 0000ACAE FUN_0000acae
0000AA62: bra.w 0x0000aa74
0000AA66: btst.b #0x2,(0x10,A0)
0000AA6C: beq.w 0x0000aa74
0000AA70: bsr.w 0x0000ac84 ; call 0000AC84 FUN_0000ac84
0000AA74: btst.b #0x2,(0x10,A0)
0000AA7A: beq.w 0x0000aaa0
0000AA7E: bsr.w 0x0000bcae ; call 0000BCAE apply_mode_five_boundary_motion
0000AA82: jsr 0x0000eb92.l ; call 0000EB92 FUN_0000eb92
0000AA88: bclr.b #0x1,(0x11,A0)
0000AA8E: beq.w 0x0000aaa0
0000AA92: move.l (0x3a,A0),(0xd4,A0)
0000AA98: move.l #0xb49c,(0x3a,A0) ; data 0000b49c
0000AAA0: bclr.b #0x2,(0x11,A0)
0000AAA6: movea.l (0x3a,A0),A1
0000AAAA: move.w (0x16,A0),D0w
0000AAAE: move.w D0w,-(SP) ; data Stack[-0x4]
0000AAB0: jsr (A1) ; call 0000B49C resolve_entity_fallback_movement
0000AAB2: bsr.w 0x0000ac5e ; call 0000AC5E FUN_0000ac5e
0000AAB6: move.w (SP)+,D0w
0000AAB8: cmp.w (0x16,A0),D0w
0000AABC: beq.w 0x0000aac6
0000AAC0: bset.b #0x0,(0x11,A0)
0000AAC6: move.w (SP)+,D7w
0000AAC8: adda.w #0x100,A0
0000AACC: dbf D7w,0x0000a8fa
0000AAD0: rts
0000AAD2: movea.l (0x00fffc04).l,A0
0000AAD8: tst.b (0x4e,A0)
0000AADC: bne.w 0x0000aafc
0000AAE0: tst.b (0x4f,A0)
0000AAE4: beq.w 0x0000aafc
0000AAE8: tst.w (0xdc,A0)
0000AAEC: bpl.w 0x0000aafc
0000AAF0: jsr 0x00014226.l ; call 00014226 FUN_00014226
0000AAF6: ori #0x1,CCR
0000AAFA: rts
0000AAFC: andi #0x1e,CCR
0000AB00: rts
0000AB02: btst.b #0x7,(0xb0,A0)
0000AB08: beq.w 0x0000ab24
0000AB0C: movem.l {  A0},-(SP)
0000AB10: move.l #0x1f210,(0x00ffdd2c).l
0000AB1A: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
0000AB20: movem.l (SP)+,{  A0}
0000AB24: btst.b #0x3,(0x10,A0)
0000AB2A: bne.w 0x0000ab6e
0000AB2E: jsr 0x00016b98.l ; call 00016B98 FUN_00016b98
0000AB34: btst.b #0x6,(0x9e,A0)
0000AB3A: beq.w 0x0000ab6e
0000AB3E: subq.b #0x1,(0x00ffe177).l
0000AB44: bne.w 0x0000ab6e
0000AB48: jsr 0x0001dbf0.l ; call 0001DBF0 FUN_0001dbf0
0000AB4E: tst.l (0x00ffe12e).l
0000AB54: beq.w 0x0000ab6e
0000AB58: move.l (0x00ffe12e).l,(0x00ffdd2c).l
0000AB62: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
0000AB68: clr.l (0x00ffe12e).l
0000AB6E: bset.b #0x6,(0x10,A0)
0000AB74: btst.b #0x5,(0x10,A0)
0000AB7A: beq.w 0x0000abaa
0000AB7E: move.l A0,D0
0000AB80: lea (0xff0400).l,A1 ; data 00ff0400
0000AB86: move.w #0x6,D7w
0000AB8A: btst.b #0x0,(0x10,A1)
0000AB90: beq.w 0x0000aba2
0000AB94: cmp.l (0xa8,A1),D0
0000AB98: bne.w 0x0000aba2
0000AB9C: bset.b #0x5,(0x10,A1)
0000ABA2: adda.w #0x100,A1
0000ABA6: dbf D7w,0x0000ab8a
0000ABAA: btst.b #0x3,(0x10,A0)
0000ABB0: bne.w 0x0000abb6
0000ABB4: rts
0000ABB6: btst.b #0x6,(0x00ff0110).l
0000ABBE: beq.w 0x0000abfa
0000ABC2: btst.b #0x0,(0x00ff0210).l
0000ABCA: beq.w 0x0000abda
0000ABCE: btst.b #0x6,(0x00ff0210).l
0000ABD6: beq.w 0x0000abfa
0000ABDA: btst.b #0x0,(0x00ff0310).l
0000ABE2: beq.w 0x0000abf2
0000ABE6: btst.b #0x6,(0x00ff0310).l
0000ABEE: beq.w 0x0000abfa
0000ABF2: move.w #0x3c,(0x00ffe166).l
0000ABFA: rts
0000ABFC: movem.w {  D2w D1w},-(SP)
0000AC00: addq.w #0x1,D1w
0000AC02: jsr 0x0000d7ea.l ; call 0000D7EA compute_entity_delay_factor
0000AC08: mulu.w #0x4,D1
0000AC0C: move.w #0x6a,D2w
0000AC10: sub.w D1w,D2w
0000AC12: tst.w D0w
0000AC14: beq.w 0x0000ac28
0000AC18: cmpi.w #0x2,D0w
0000AC1C: bne.w 0x0000ac26
0000AC20: asl.w #0x1,D2w
0000AC22: bra.w 0x0000ac28
0000AC26: asr.w #0x1,D2w
0000AC28: btst.b #0x1,(0x10,A0)
0000AC2E: beq.w 0x0000ac32
0000AC32: cmpi.w #0xf,D2w
0000AC36: bge.w 0x0000ac3e
0000AC3A: move.w #0xf,D2w
0000AC3E: nop
0000AC40: nop
0000AC42: nop
0000AC44: move.w D2w,(0xdc,A0)
0000AC48: movem.w (SP)+,{  D1 D2}
0000AC4C: rts
0000AC4E: tst.w (0x16,A0)
0000AC52: bpl.w 0x0000ac5c
0000AC56: bclr.b #0x0,(0x10,A0)
0000AC5C: rts
0000AC5E: btst.b #0x3,(0x10,A0)
0000AC64: bne.w 0x0000ac6a
0000AC68: rts
0000AC6A: tst.w (0x00ffdf64).l
0000AC70: bne.w 0x0000ac7c
0000AC74: bclr.b #0x5,(0xb0,A0)
0000AC7A: beq.b 0x0000ac68
0000AC7C: bpl.b 0x0000ac68
0000AC7E: bsr.w 0x0000b7b8 ; call 0000B7B8 FUN_0000b7b8
0000AC82: bra.b 0x0000ac68
0000AC84: btst.b #0x3,(0x10,A0)
0000AC8A: beq.w 0x0000ac98
0000AC8E: jsr 0x000138b8.l ; call 000138B8 FUN_000138b8
0000AC94: bra.w 0x0000ac9e
0000AC98: jsr 0x00013904.l ; call 00013904 FUN_00013904
0000AC9E: tst.b (0x00ffe18a).l
0000ACA4: beq.w 0x0000acac
0000ACA8: bsr.w 0x0000ae5e ; call 0000AE5E schedule_entity_collision_recheck
0000ACAC: rts
0000ACAE: tst.b (0x00ffe18a).l
0000ACB4: bne.w 0x0000ae40
0000ACB8: tst.w (0x00ffdf3a).l
0000ACBE: beq.w 0x0000ace4
0000ACC2: move.w (0xcc,A0),D0w
0000ACC6: addi.w #-0x8000,D0w
0000ACCA: move.w D0w,(0x2e,A0)
0000ACCE: move.w D0w,(0x34,A0)
0000ACD2: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
0000ACDA: move.l #0xb574,(0x3a,A0) ; data 0000b574
0000ACE2: rts
0000ACE4: tst.b (0x00ffe18b).l
0000ACEA: beq.w 0x0000ad08
0000ACEE: tst.b (0x00ffe18c).l
0000ACF4: beq.w 0x0000ad02
0000ACF8: subq.b #0x1,(0x00ffe18c).l
0000ACFE: bra.w 0x0000ae40
0000AD02: clr.w D0w
0000AD04: bra.w 0x0000ae50
0000AD08: btst.b #0xc,(0x00ffdd22).l
0000AD10: beq.w 0x0000ad1a
0000AD14: jsr 0x0001dc4c.l ; call 0001DC4C FUN_0001dc4c
0000AD1A: tst.w (0xdc,A0)
0000AD1E: bmi.w 0x0000ad3a
0000AD22: btst.b #0xe,(0x00ffdd22).l
0000AD2A: beq.w 0x0000ae26
0000AD2E: move.w #-0x1,(0x00ffdfe6).l
0000AD36: bra.w 0x0000ae26
0000AD3A: tst.w (0x00ffdfe6).l
0000AD40: beq.w 0x0000ad4e
0000AD44: clr.w (0x00ffdfe6).l
0000AD4A: bra.w 0x0000ad82
0000AD4E: move.w (0x00ffdd22).l,D0w
0000AD54: btst.l #0x6,D0
0000AD58: bne.w 0x0000ad82
0000AD5C: move.w (0x00ffdd04).l,D0w
0000AD62: cmpi.w #-0x1,D0w
0000AD66: beq.w 0x0000ad72
0000AD6A: clr.b (0x43,A0)
0000AD6E: bra.w 0x0000ae26
0000AD72: move.w #0x1,D0w
0000AD76: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000AD7A: addq.b #0x1,(0x43,A0)
0000AD7E: bra.w 0x0000ae26
0000AD82: clr.b (0x43,A0)
0000AD86: tst.b (0x56,A0)
0000AD8A: bpl.w 0x0000adaa
0000AD8E: jsr 0x0000ae6e.l ; call 0000AE6E FUN_0000ae6e
0000AD94: tst.b (0x00fff0dd).l
0000AD9A: beq.w 0x0000ada4
0000AD9E: jmp 0x0000afae.l ; call 0000AFAE prepare_entity_delayed_animation_alternate
0000ADA4: jmp 0x0000af7c.l ; call 0000AF7C prepare_entity_delayed_animation_primary
0000ADAA: cmpi.b #0xd,(0x57,A0)
0000ADB0: blt.w 0x0000addc
0000ADB4: cmpi.b #0x10,(0x57,A0)
0000ADBA: bpl.w 0x0000addc
0000ADBE: jsr 0x00015646.l ; call 00015646 FUN_00015646
0000ADC4: bcc.w 0x0000add2
0000ADC8: bra.w 0x0000ae4e
0000ADD2: move.w (0x2e,A1),D0w
0000ADD6: jmp 0x0000b3fe.l ; call 0000B3FE set_entity_heading_with_alternate_animation
0000ADDC: tst.b (0x57,A0)
0000ADE0: beq.w 0x0000ae16
0000ADE4: cmpi.b #0xe,(0x57,A0)
0000ADEA: bgt.w 0x0000ae4e
0000ADEE: jsr 0x0001514e.l ; call 0001514E FUN_0001514e
0000ADF4: bcc.w 0x0000ae1c
0000ADF8: bsr.w 0x0000aad2 ; call 0000AAD2 FUN_0000aad2
0000ADFC: bcc.w 0x0000ae40
0000AE00: move.w #0x4b,D0w
0000AE04: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000AE0A: move.w #0x0,D0w
0000AE0E: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000AE12: bra.w 0x0000ae26
0000AE16: jmp 0x0000b1dc.l ; call 0000B1DC configure_entity_target_transition
0000AE1C: move.w (0x2e,A1),D0w
0000AE20: jmp 0x0000b39a.l ; call 0000B39A set_entity_heading_with_field_animation
0000AE26: move.w (0x00ffdd04).l,D0w
0000AE2C: cmpi.w #-0x1,D0w
0000AE30: bne.w 0x0000ae50
0000AE34: cmpi.l #0xb45c,(0x3a,A0)
0000AE3C: beq.w 0x0000ae4e
0000AE40: bset.b #0x0,(0x11,A0)
0000AE46: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
0000AE4E: rts
0000AE50: move.w D0w,(0x34,A0)
0000AE54: move.l #0xb574,(0x3a,A0) ; data 0000b574
0000AE5C: rts
0000AE5E: bset.b #0x0,(0x11,A0)
0000AE64: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
0000AE6C: rts
0000AE6E: cmpi.b #0x3,(0x00ffe186).l
0000AE76: beq.w 0x0000aeac
0000AE7A: move.l (0xa2,A0),D1
0000AE7E: beq.w 0x0000aeb4
0000AE82: movea.l D1,A1
0000AE84: move.w (A0),D5w
0000AE86: move.w (0x4,A0),D4w
0000AE8A: move.w (A1),D3w
0000AE8C: move.w (0x4,A1),D2w
0000AE90: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
0000AE96: move.w D2w,(0x2e,A0)
0000AE9A: move.l A0,D0
0000AE9C: cmp.l D0,D1
0000AE9E: beq.w 0x0000aeb4
0000AEA2: jsr 0x00014d8e.l ; call 00014D8E FUN_00014d8e
0000AEA8: bcc.w 0x0000aeb4
0000AEAC: clr.b (0x00fff0dd).l
0000AEB2: rts
0000AEB4: jmp 0x0001033e.l ; call 0001033E FUN_0001033e
0000AEBA: addq.w #0x8,(0x00ffdfec).l
0000AEC0: cmpi.w #0xa0,(0x00ffdfec).l
0000AEC8: bcs.w 0x0000aed4
0000AECC: clr.w (0x00ffdfea).l
0000AED2: rts
0000AED4: move.w (0x00ffdfee).l,D1w
0000AEDA: move.w (0x00ffdff0).l,D2w
0000AEE0: move.w #0x6000,D6w
0000AEE4: lea (0xaf3e).l,A2 ; data 0000af3e
0000AEEA: move.w (0x00ffdf88).l,D0w
0000AEF0: lsr.w #0x2,D0w
0000AEF2: andi.w #0x3,D0w
0000AEF6: lsl.w #0x2,D0w
0000AEF8: movea.l (0x0,A2,D0w*0x1),A2 ; data 0000af3e
0000AEFC: lea (0x15e16).l,A1 ; data 00015e16
0000AF02: move.w #0x1f,D7w
0000AF06: clr.w D4w
0000AF08: movem.l {  A2 D4 D2 D1},-(SP)
0000AF0C: move.w (0x0,A1,D4w*0x1),D0w
0000AF10: muls.w (0x00ffdfec).l,D0
0000AF16: swap D0
0000AF18: add.w D0w,D1w
0000AF1A: addi.w #0x40,D4w
0000AF1E: move.w (0x0,A1,D4w*0x1),D0w
0000AF22: muls.w (0x00ffdfec).l,D0
0000AF28: swap D0
0000AF2A: add.w D0w,D2w
0000AF2C: jsr 0x00001378.l ; call 00001378 FUN_00001378
0000AF32: movem.l (SP)+,{  D1 D2 D4 A2}
0000AF36: addq.w #0x8,D4w
0000AF38: dbf D7w,0x0000af08
0000AF3C: rts
0000AF6C: clr.w (0x1a,A0)
0000AF70: bset.b #0x0,(0x11,A0)
0000AF76: clr.w (0x1e,A0)
0000AF7A: rts
0000AF7C: clr.w D0w
0000AF7E: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000AF82: move.l #0xafe0,(0x3a,A0) ; data 0000afe0
0000AF8A: move.w (0x2e,A0),D0w
0000AF8E: rol.w #0x3,D0w
0000AF90: bpl.w 0x0000af96
0000AF94: addq.w #0x1,D0w
0000AF96: andi.w #0x7,D0w
0000AF9A: lsl.w #0x1,D0w
0000AF9C: lea (0xc082).l,A1 ; data 0000c082
0000AFA2: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c082
0000AFA6: move.w D1w,(0x16,A0)
0000AFAA: bsr.b 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000AFAC: rts
0000AFAE: clr.w D0w
0000AFB0: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000AFB4: move.l #0xafe2,(0x3a,A0) ; data 0000afe2
0000AFBC: move.w (0x2e,A0),D0w
0000AFC0: rol.w #0x3,D0w
0000AFC2: bpl.w 0x0000afc8
0000AFC6: addq.w #0x1,D0w
0000AFC8: andi.w #0x7,D0w
0000AFCC: lsl.w #0x1,D0w
0000AFCE: lea (0xc082).l,A1 ; data 0000c082
0000AFD4: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c082
0000AFD8: move.w D1w,(0x16,A0)
0000AFDC: bsr.b 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000AFDE: rts
0000AFE2: clr.w (0x38,A0)
0000AFE6: move.w (A0),D5w
0000AFE8: move.w (0x4,A0),D6w
0000AFEC: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000AFF0: rts
0000AFF2: btst.b #0x4,(0x10,A0)
0000AFF8: beq.w 0x0000affe
0000AFFC: rts
0000AFFE: tst.b (0x4c,A0)
0000B002: beq.w 0x0000b330
0000B006: move.l #0xb07e,(0x3a,A0) ; data 0000b07e
0000B00E: move.w (0x2e,A0),D0w
0000B012: rol.w #0x3,D0w
0000B014: bpl.w 0x0000b01a
0000B018: addq.w #0x1,D0w
0000B01A: andi.w #0x7,D0w
0000B01E: lsl.w #0x1,D0w
0000B020: move.l A1,-(SP) ; data Stack[-0x4]
0000B022: lea (0xc072).l,A1 ; data 0000c072
0000B028: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c072
0000B02C: movea.l (SP)+,A1
0000B02E: move.w D1w,(0x16,A0)
0000B032: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B036: move.b #0x8,(0xce,A0)
0000B03C: rts
0000B07E: subq.b #0x1,(0xce,A0)
0000B082: bne.w 0x0000b09c
0000B086: move.w (0xe,A0),D0w
0000B08A: add.w D0w,D0w
0000B08C: lea (0xb03e).l,A2 ; data 0000b03e
0000B092: move.w (0x0,A2,D0w*0x1),D0w ; data 0000b03e
0000B096: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000B09C: clr.w (0x38,A0)
0000B0A0: move.w (A0),D5w
0000B0A2: move.w (0x4,A0),D6w
0000B0A6: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B0AA: rts
0000B0AC: btst.b #0x3,(0x10,A0)
0000B0B2: bne.w 0x0000b330
0000B0B6: cmpi.w #0xc,(0xe,A0)
0000B0BC: beq.w 0x0000b330
0000B0C0: move.l #0xb0d4,(0x3a,A0) ; data 0000b0d4
0000B0C8: move.w #0xe0,(0x16,A0)
0000B0CE: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B0D2: rts
0000B0D4: clr.w (0x38,A0)
0000B0D8: move.w (A0),D5w
0000B0DA: move.w (0x4,A0),D6w
0000B0DE: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B0E2: rts
0000B0E4: btst.b #0x3,(0x10,A0)
0000B0EA: bne.w 0x0000b330
0000B0EE: cmpi.w #0xc,(0xe,A0)
0000B0F4: beq.w 0x0000b330
0000B0F8: move.l #0xb10c,(0x3a,A0) ; data 0000b10c
0000B100: move.w #0xe4,(0x16,A0)
0000B106: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B10A: rts
0000B10C: clr.w (0x38,A0)
0000B110: move.w (A0),D5w
0000B112: move.w (0x4,A0),D6w
0000B116: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B11A: rts
0000B11C: move.b (0x4c,A1),D1b
0000B120: move.b (0x9c,A1),D0b
0000B124: movem.w {  D1w D0w},-(SP)
0000B128: jsr 0x00055b16.l ; call 00055B16 FUN_00055b16
0000B12E: movem.w (SP)+,{ D0 D1}
0000B132: cmp.b (0x4c,A1),D1b
0000B136: bne.w 0x0000b142
0000B13A: cmp.b (0x9c,A1),D0b
0000B13E: beq.w 0x0000b198
0000B142: movem.l {  A1 A0},-(SP)
0000B146: exg A0,A1
0000B148: bsr.w 0x0000aff2 ; call 0000AFF2 prepare_entity_auxiliary_behavior
0000B14C: jsr 0x00015a8c.l ; call 00015A8C find_first_available_entity_slot
0000B152: bcs.w 0x0000b194
0000B156: move.w D0w,(0x3e,A1)
0000B15A: bset.b #0x0,(0x11,A1)
0000B160: move.w #0x4,(0x16,A1)
0000B166: move.w #0x1,(0xe,A1)
0000B16C: move.l #0xac4e,(0x3a,A1) ; data 0000ac4e
0000B174: bset.b #0x0,(0x10,A1)
0000B17A: move.w (A0),(A1)
0000B17C: move.w (0x4,A0),D1w
0000B180: addi.w #0x10,D1w
0000B184: move.w D1w,(0x4,A1)
0000B188: move.w #0x2000,(0x2c,A1) ; data 00002000
0000B18E: bset.b #0x0,(0x11,A1)
0000B194: movem.l (SP)+,{  A0 A1}
0000B198: clr.w D0w
0000B19A: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000B19E: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
0000B1A4: move.b #0x6,(0x2,A0)
0000B1AA: clr.b (0x3,A0)
0000B1AE: move.l #0xb3ee,(0x3a,A0) ; data 0000b3ee
0000B1B6: move.w (0x2e,A0),D0w
0000B1BA: rol.w #0x3,D0w
0000B1BC: bpl.w 0x0000b1c2
0000B1C0: addq.w #0x1,D0w
0000B1C2: andi.w #0x7,D0w
0000B1C6: lsl.w #0x1,D0w
0000B1C8: lea (0xc042).l,A1 ; data 0000c042
0000B1CE: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c042
0000B1D2: move.w D1w,(0x16,A0)
0000B1D6: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B1DA: rts
0000B1DC: move.w #0x1,D0w
0000B1E0: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000B1E4: move.w #0x4c,D0w
0000B1E8: cmpi.w #0xc,(0xe,A0)
0000B1EE: bne.w 0x0000b1f6
0000B1F2: move.w #0x55,D0w
0000B1F6: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000B1FC: move.l (0xa2,A0),D3
0000B200: beq.w 0x0000b21c
0000B204: movea.l D3,A2
0000B206: move.w (A0),D5w
0000B208: move.w (0x4,A0),D4w
0000B20C: move.w (A2),D3w
0000B20E: move.w (0x4,A2),D2w
0000B212: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
0000B218: move.w D2w,(0x34,A0)
0000B21C: move.w (0x34,A0),(0x2e,A0)
0000B222: move.w (0x2e,A0),D0w
0000B226: move.l #0xb316,(0x3a,A0) ; data 0000b316
0000B22E: rol.w #0x3,D0w
0000B230: bpl.w 0x0000b236
0000B234: addq.w #0x1,D0w
0000B236: andi.w #0x7,D0w
0000B23A: lsl.w #0x1,D0w
0000B23C: lea (0xc0a2).l,A1 ; data 0000c0a2
0000B242: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c0a2
0000B246: move.w D1w,(0x16,A0)
0000B24A: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B24E: jsr 0x00015c84.l ; call 00015C84 FUN_00015c84
0000B254: bcc.w 0x0000b2f4
0000B258: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
0000B25E: move.w #0x4d,D0w
0000B262: btst.b #0x7,(0x00fff9f8).l
0000B26A: beq.w 0x0000b272
0000B26E: move.w #0x4e,D0w
0000B272: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000B278: move.b (0x4c,A1),D1b
0000B27C: move.b (0x9c,A1),D0b
0000B280: movem.w {  D1w D0w},-(SP)
0000B284: jsr 0x00055c76.l ; call 00055C76 FUN_00055c76
0000B28A: movem.w (SP)+,{ D0 D1}
0000B28E: cmp.b (0x4c,A1),D1b
0000B292: bne.w 0x0000b29e
0000B296: cmp.b (0x9c,A1),D0b
0000B29A: beq.w 0x0000b2f4
0000B29E: movem.l {  A1 A0},-(SP)
0000B2A2: exg A0,A1
0000B2A4: bsr.w 0x0000aff2 ; call 0000AFF2 prepare_entity_auxiliary_behavior
0000B2A8: jsr 0x00015a8c.l ; call 00015A8C find_first_available_entity_slot
0000B2AE: bcs.w 0x0000b2f0
0000B2B2: move.w D0w,(0x3e,A1)
0000B2B6: bset.b #0x0,(0x11,A1)
0000B2BC: move.w #0x4,(0x16,A1)
0000B2C2: move.w #0x1,(0xe,A1)
0000B2C8: move.l #0xac4e,(0x3a,A1) ; data 0000ac4e
0000B2D0: move.b #0x1,(0x10,A1)
0000B2D6: move.w (A0),(A1)
0000B2D8: move.w (0x4,A0),D1w
0000B2DC: addi.w #0x10,D1w
0000B2E0: move.w D1w,(0x4,A1)
0000B2E4: move.w #0x2000,(0x2c,A1) ; data 00002000
0000B2EA: bset.b #0x0,(0x11,A1)
0000B2F0: movem.l (SP)+,{  A0 A1}
0000B2F4: rts
0000B316: cmpi.w #0xc,(0xe,A0)
0000B31C: beq.w 0x0000babc
0000B320: clr.w (0x38,A0)
0000B324: move.w (A0),D5w
0000B326: move.w (0x4,A0),D6w
0000B32A: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B32E: rts
0000B330: bset.b #0x6,(0x10,A0)
0000B336: bsr.w 0x0000abb6 ; call 0000ABB6 FUN_0000abb6
0000B33A: bclr.b #0x6,(0x10,A0)
0000B340: move.w (0xe,A0),D0w
0000B344: add.w D0w,D0w
0000B346: lea (0xb05e).l,A2 ; data 0000b05e
0000B34C: move.w (0x0,A2,D0w*0x1),D0w ; data 0000b05e
0000B350: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000B356: bset.b #0x4,(0x10,A0)
0000B35C: move.w (0x2e,A0),D0w
0000B360: move.l #0xb38a,(0x3a,A0) ; data 0000b38a
0000B368: rol.w #0x3,D0w
0000B36A: bpl.w 0x0000b370
0000B36E: addq.w #0x1,D0w
0000B370: andi.w #0x7,D0w
0000B374: lsl.w #0x1,D0w
0000B376: lea (0xc0b2).l,A1 ; data 0000c0b2
0000B37C: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c0b2
0000B380: move.w D1w,(0x16,A0)
0000B384: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B388: rts
0000B38A: clr.w (0x38,A0)
0000B38E: move.w (A0),D5w
0000B390: move.w (0x4,A0),D6w
0000B394: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B398: rts
0000B39A: move.w D0w,(0x2e,A0)
0000B39E: move.l #0xb3ee,(0x3a,A0) ; data 0000b3ee
0000B3A6: rol.w #0x3,D0w
0000B3A8: bpl.w 0x0000b3ae
0000B3AC: addq.w #0x1,D0w
0000B3AE: andi.w #0x7,D0w
0000B3B2: lsl.w #0x1,D0w
0000B3B4: lea (0xc042).l,A1 ; data 0000c042
0000B3BA: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c042
0000B3BE: move.w D1w,(0x16,A0)
0000B3C2: move.b (0x57,A0),D0b
0000B3C6: subq.b #0x1,D0b
0000B3C8: ext.w D0w
0000B3CA: cmpi.w #0xa,D0w
0000B3CE: beq.w 0x0000b3e2
0000B3D2: cmpi.w #0xb,D0w
0000B3D6: beq.w 0x0000b3e2
0000B3DA: move.w #0x1,D0w
0000B3DE: bra.w 0x0000b3e4
0000B3E2: clr.w D0w
0000B3E4: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000B3E8: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B3EC: rts
0000B3EE: clr.w (0x38,A0)
0000B3F2: move.w (A0),D5w
0000B3F4: move.w (0x4,A0),D6w
0000B3F8: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B3FC: rts
0000B3FE: move.w D0w,(0x2e,A0)
0000B402: move.l #0xb432,(0x3a,A0) ; data 0000b432
0000B40A: rol.w #0x3,D0w
0000B40C: bpl.w 0x0000b412
0000B410: addq.w #0x1,D0w
0000B412: andi.w #0x7,D0w
0000B416: lsl.w #0x1,D0w
0000B418: lea (0xc092).l,A1 ; data 0000c092
0000B41E: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c092
0000B422: move.w D1w,(0x16,A0)
0000B426: bsr.w 0x0000af6c ; call 0000AF6C reset_entity_animation_state
0000B42A: clr.w D0w
0000B42C: bsr.w 0x0000abfc ; call 0000ABFC update_entity_animation_delay
0000B430: rts
0000B432: clr.w (0x38,A0)
0000B436: move.w (A0),D5w
0000B438: move.w (0x4,A0),D6w
0000B43C: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B440: rts
0000B442: bclr.b #0x1,(0xb0,A0)
0000B448: move.w (0x34,A0),(0x2e,A0)
0000B44E: move.l #0xb45c,(0x3a,A0)
0000B456: bset.b #0x0,(0x11,A0)
0000B45C: clr.w (0x38,A0)
0000B460: move.w (0x2e,A0),D0w
0000B464: rol.w #0x3,D0w
0000B466: bpl.w 0x0000b46c
0000B46A: addq.w #0x1,D0w
0000B46C: andi.w #0x7,D0w
0000B470: lsl.w #0x1,D0w
0000B472: lea (0xc052).l,A1 ; data 0000c052
0000B478: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c052
0000B47C: move.w D1w,(0x16,A0)
0000B480: move.w (A0),D5w
0000B482: move.w (0x4,A0),D6w
0000B486: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B48A: rts
0000B48C: jsr 0x0000e7c8.l ; call 0000E7C8 FUN_0000e7c8
0000B492: move.l (0x00ffef10).l,(0xd0,A0)
0000B49A: rts
0000B49C: tst.w (0x38,A0)
0000B4A0: bne.w 0x0000b4aa
0000B4A4: move.w #0x1,(0x38,A0)
0000B4AA: lea (0xbf82).l,A1 ; data 0000bf82
0000B4B0: move.w (0x00ffdf88).l,D0w
0000B4B6: btst.l #0x0,D0
0000B4BA: beq.w 0x0000b4c6
0000B4BE: move.w (0x34,A0),D0w
0000B4C2: bra.w 0x0000b4ca
0000B4C6: move.w (0x92,A0),D0w
0000B4CA: rol.w #0x3,D0w
0000B4CC: andi.w #0x7,D0w
0000B4D0: lsl.w #0x1,D0w
0000B4D2: bsr.w 0x0000b732 ; call 0000B732 apply_movement_result_policy
0000B4D6: tst.w D0w
0000B4D8: beq.w 0x0000b522
0000B4DC: bmi.w 0x0000b522
0000B4E0: move.w #0x2,(0x38,A0)
0000B4E6: move.w (0x92,A0),D0w
0000B4EA: addi.w #0x4000,D0w
0000B4EE: rol.w #0x3,D0w
0000B4F0: andi.w #0x7,D0w
0000B4F4: lsl.w #0x1,D0w
0000B4F6: bsr.w 0x0000b732 ; call 0000B732 apply_movement_result_policy
0000B4FA: tst.w D0w
0000B4FC: bmi.w 0x0000b522
0000B500: beq.w 0x0000b522
0000B504: move.w (0x92,A0),D0w
0000B508: addi.w #-0x8000,D0w
0000B50C: rol.w #0x3,D0w
0000B50E: andi.w #0x7,D0w
0000B512: lsl.w #0x1,D0w
0000B514: bsr.w 0x0000b732 ; call 0000B732 apply_movement_result_policy
0000B518: tst.w D0w
0000B51A: bne.w 0x0000b528
0000B51E: bpl.w 0x0000b528
0000B522: move.w D5w,(A0)
0000B524: move.w D6w,(0x4,A0)
0000B528: move.l (0xd4,A0),(0x3a,A0)
0000B52E: rts
0000B530: move.w (0x2e,A0),D1w
0000B534: sub.w (0x34,A0),D1w
0000B538: beq.w 0x0000b568
0000B53C: bpl.w 0x0000b55c
0000B540: cmpi.w #-0x800,D1w
0000B544: bcc.w 0x0000b552
0000B548: addi.w #0x800,(0x2e,A0)
0000B54E: bra.w 0x0000b568
0000B552: move.w (0x34,A0),(0x2e,A0)
0000B558: bra.w 0x0000b568
0000B55C: cmpi.w #0x800,D1w
0000B560: bcs.b 0x0000b552
0000B562: subi.w #0x800,(0x2e,A0)
0000B568: rts
0000B574: cmpi.w #0xd,(0xe,A0)
0000B57A: bne.w 0x0000b5b4
0000B57E: move.w (0x2e,A0),D0w
0000B582: rol.w #0x3,D0w
0000B584: andi.w #0x7,D0w
0000B588: lsl.w #0x1,D0w
0000B58A: cmpi.w #0x2,(0x1e,A0)
0000B590: bne.w 0x0000b5a4
0000B594: move.w #-0x14,(0x8,A0)
0000B59A: lea (0xbe42).l,A1 ; data 0000be42
0000B5A0: bra.w 0x0000b678
0000B5A4: clr.w (0x8,A0)
0000B5A8: bsr.b 0x0000b530 ; call 0000B530 approach_entity_fixed_point_target
0000B5AA: lea (0xbe22).l,A1 ; data 0000be22
0000B5B0: bra.w 0x0000b678
0000B5B4: move.w #0x1,(0x38,A0)
0000B5BA: bsr.w 0x0000b530 ; call 0000B530 approach_entity_fixed_point_target
0000B5BE: move.w (0x2e,A0),D0w
0000B5C2: rol.w #0x3,D0w
0000B5C4: andi.w #0x7,D0w
0000B5C8: lsl.w #0x1,D0w
0000B5CA: btst.b #0x0,(0x00ffe168).l
0000B5D2: beq.w 0x0000b5ea
0000B5D6: btst.b #0x1,(0x10,A0)
0000B5DC: beq.w 0x0000b5ea
0000B5E0: lea (0xc022).l,A1 ; data 0000c022
0000B5E6: bra.w 0x0000b666
0000B5EA: lea (0xbec2).l,A1 ; data 0000bec2
0000B5F0: btst.b #0x3,(0x10,A0)
0000B5F6: bne.w 0x0000b60a
0000B5FA: btst.b #0x7,(0xb0,A0)
0000B600: bne.w 0x0000b60a
0000B604: lea (0xbea2).l,A1 ; data 0000bea2
0000B60A: btst.b #0x5,(0x9e,A0)
0000B610: beq.w 0x0000b62a
0000B614: lea (0xbe82).l,A1 ; data 0000be82
0000B61A: tst.w (0x00ffdf84).l
0000B620: bne.w 0x0000b666
0000B624: lea (0xbe62).l,A1 ; data 0000be62
0000B62A: tst.w (0x00ffdf84).l
0000B630: beq.w 0x0000b666
0000B634: btst.b #0x7,(0xb0,A0)
0000B63A: beq.w 0x0000b648
0000B63E: lea (0xbe82).l,A1 ; data 0000be82
0000B644: bra.w 0x0000b666
0000B648: move.b (0x78,A0),D1b
0000B64C: ext.w D1w
0000B64E: cmpi.w #0xa,D1w
0000B652: blt.w 0x0000b65a
0000B656: move.w #0x9,D1w
0000B65A: lsl.w #0x2,D1w
0000B65C: lea (0xbdf2).l,A1 ; data 0000bdf2
0000B662: movea.l (0x0,A1,D1w*0x1),A1
0000B666: btst.b #0x1,(0x10,A0)
0000B66C: bne.w 0x0000b678
0000B670: move.w D0w,-(SP)
0000B672: bsr.w 0x0000b6e4 ; call 0000B6E4 select_entity_animation_table
0000B676: move.w (SP)+,D0w
0000B678: btst.b #0x3,(0x10,A0)
0000B67E: beq.w 0x0000b690
0000B682: tst.w (0xc,A0)
0000B686: bne.w 0x0000b690
0000B68A: lea (0xc022).l,A1 ; data 0000c022
0000B690: move.w (A0),-(SP)
0000B692: move.w (0x4,A0),-(SP)
0000B696: bsr.w 0x0000bade ; call 0000BADE advance_entity_movement_step
0000B69A: move.w (SP)+,D1w
0000B69C: move.w (SP)+,D0w
0000B69E: cmpi.w #0xd,(0xe,A0)
0000B6A4: beq.w 0x0000b6ba
0000B6A8: cmp.w (A0),D0w
0000B6AA: bne.w 0x0000b6ba
0000B6AE: cmp.w (0x4,A0),D1w
0000B6B2: bne.w 0x0000b6ba
0000B6B6: bra.w 0x0000b442 ; call 0000B442 reset_entity_movement_state
0000B6BA: move.w (0x2e,A0),D0w
0000B6BE: btst.b #0x2,(0x11,A0)
0000B6C4: beq.w 0x0000b6cc
0000B6C8: move.w (0x36,A0),D0w
0000B6CC: rol.w #0x3,D0w
0000B6CE: andi.w #0x7,D0w
0000B6D2: lsl.w #0x1,D0w
0000B6D4: lea (0xc062).l,A1 ; data 0000c062
0000B6DA: move.w (0x0,A1,D0w*0x1),D1w ; data 0000c062
0000B6DE: move.w D1w,(0x16,A0)
0000B6E2: rts
0000B6E4: btst.b #0x3,(0x10,A0)
0000B6EA: beq.w 0x0000b6f4
0000B6EE: lea (0xbfe2).l,A1 ; data 0000bfe2
0000B6F4: move.w (0xc8,A0),D0w
0000B6F8: cmpi.w #0x460,D0w
0000B6FC: bcc.w 0x0000b730
0000B700: lea (0xbd72).l,A1 ; data 0000bd72
0000B706: cmpi.w #0x360,D0w
0000B70A: bcc.w 0x0000b730
0000B70E: lea (0xbd92).l,A1 ; data 0000bd92
0000B714: cmpi.w #0x200,D0w
0000B718: bcc.w 0x0000b730
0000B71C: lea (0xbdb2).l,A1 ; data 0000bdb2
0000B722: cmpi.w #0x100,D0w
0000B726: bcc.w 0x0000b730
0000B72A: lea (0xbdd2).l,A1 ; data 0000bdd2
0000B730: rts
0000B732: bsr.w 0x0000b77e ; call 0000B77E probe_entity_directional_collision
0000B736: tst.w D0w
0000B738: bmi.w 0x0000b73e
0000B73C: rts
0000B73E: tst.w (0x00ffdf84).l
0000B744: bne.w 0x0000b778
0000B748: move.w D0w,D1w
0000B74A: andi.w #0x1f,D1w
0000B74E: cmpi.w #0x18,D1w
0000B752: bge.w 0x0000b778
0000B756: cmpi.w #0x14,(0x00ffddc4).l
0000B75E: beq.w 0x0000b76c
0000B762: cmpi.w #0x2,(0x00ffdf40).l
0000B76A: bne.b 0x0000b73c
0000B76C: cmpi.w #0x2,D1w
0000B770: blt.b 0x0000b73c
0000B772: cmpi.w #0x5,D1w
0000B776: bgt.b 0x0000b73c
0000B778: move.w #0x7f,D0w
0000B77C: rts
0000B77E: move.w D0w,(0x00ffdf66).l
0000B784: lsl.w #0x1,D0w
0000B786: move.w (0x0,A1,D0w*0x1),D5w
0000B78A: add.w (A0),D5w
0000B78C: addq.w #0x2,D0w
0000B78E: move.w (0x0,A1,D0w*0x1),D6w
0000B792: add.w (0x4,A0),D6w
0000B796: movem.w {  D6w D5w},-(SP)
0000B79A: bsr.w 0x0000b48c ; call 0000B48C evaluate_entity_collision
0000B79E: movem.w (SP)+,{  D5 D6}
0000B7A2: tst.w D0w
0000B7A4: bpl.w 0x0000b7b6
0000B7A8: btst.b #0x1,(0x10,A0)
0000B7AE: bne.w 0x0000b7b6
0000B7B2: move.w #0x7f,D0w
0000B7B6: rts
0000B7B8: tst.w (0x00ffdfe8).l
0000B7BE: bne.w 0x0000b7cc
0000B7C2: bset.b #0x5,(0xb0,A0)
0000B7C8: beq.w 0x0000b7ce
0000B7CC: rts
0000B7CE: btst.b #0x1,(0x10,A0)
0000B7D4: beq.w 0x0000baae
0000B7D8: cmpi.w #0x2,(0x00ffdf40).l
0000B7E0: beq.w 0x0000b7fa
0000B7E4: cmpi.w #0x14,(0x00ffddc4).l
0000B7EC: beq.w 0x0000b7fa
0000B7F0: tst.w (0x00ffdf84).l
0000B7F6: bne.w 0x0000baae
0000B7FA: clr.w D0w
0000B7FC: move.b (0x00ffef10).l,D1b
0000B802: bpl.w 0x0000b808
0000B806: addq.w #0x1,D0w
0000B808: move.b (0x00ffef11).l,D1b
0000B80E: bpl.w 0x0000b814
0000B812: addq.w #0x1,D0w
0000B814: move.b (0x00ffef12).l,D1b
0000B81A: bpl.w 0x0000b820
0000B81E: addq.w #0x1,D0w
0000B820: move.b (0x00ffef13).l,D1b
0000B826: bpl.w 0x0000b82c
0000B82A: addq.w #0x1,D0w
0000B82C: cmpi.w #0x2,D0w
0000B830: blt.w 0x0000baae
0000B834: bsr.w 0x0000bc36 ; call 0000BC36 build_collision_probe_points
0000B838: ror.w #0x2,D4w
0000B83A: move.w D4w,(0xcc,A0)
0000B83E: move.w D5w,D0w
0000B840: move.w D6w,D1w
0000B842: andi.w #-0x200,D0w
0000B846: andi.w #-0x200,D1w
0000B84A: addi.w #-0x190,D1w
0000B84E: addi.w #0x70,D0w
0000B852: jsr 0x0000423c.l ; call 0000423C quantize_field_delta
0000B858: move.w (0x00ffdf64).l,D0w
0000B85E: move.w D0w,(0xc6,A0)
0000B862: andi.w #0x1f,D0w
0000B866: cmpi.w #0x1f,D0w
0000B86A: bne.w 0x0000b882
0000B86E: move.l #0xdb26,(0x00ffdd2c).l
0000B878: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B87E: bra.w 0x0000baae
0000B882: cmpi.w #0x1e,D0w
0000B886: bne.w 0x0000b89e
0000B88A: move.l #0x20990,(0x00ffdd2c).l
0000B894: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B89A: bra.w 0x0000baae
0000B89E: cmpi.w #0x1d,D0w
0000B8A2: bne.w 0x0000b8c4
0000B8A6: tst.b (0x00ffe16f).l
0000B8AC: bne.w 0x0000baae
0000B8B0: move.l #0x58d94,(0x00ffdd2c).l
0000B8BA: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B8C0: bra.w 0x0000baae
0000B8C4: cmpi.w #0x1c,D0w
0000B8C8: bne.w 0x0000b8ea
0000B8CC: tst.b (0x00ffe16f).l
0000B8D2: bne.w 0x0000baae
0000B8D6: move.l #0x58db2,(0x00ffdd2c).l
0000B8E0: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B8E6: bra.w 0x0000baae
0000B8EA: cmpi.w #0x1b,D0w
0000B8EE: bne.w 0x0000b906
0000B8F2: move.l #0x53d5a,(0x00ffdd2c).l
0000B8FC: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B902: bra.w 0x0000baae
0000B906: cmpi.w #0x1a,D0w
0000B90A: bne.w 0x0000b952
0000B90E: clr.b (0x00ffe178).l
0000B914: move.w (0xc6,A0),D0w
0000B918: cmpi.b #-0x26,D0b
0000B91C: beq.w 0x0000b934
0000B920: btst.b #0x7,(0x00fffc1c).l
0000B928: bne.w 0x0000b934
0000B92C: move.b #0x1,(0x00ffe178).l
0000B934: jsr 0x00016760.l ; call 00016760 FUN_00016760
0000B93A: bcc.w 0x0000baae
0000B93E: move.l #0x20c90,(0x00ffdd2c).l
0000B948: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
0000B94E: bra.w 0x0000baae
0000B952: cmpi.w #0x14,(0x00ffddc4).l
0000B95A: beq.w 0x0000b96a
0000B95E: cmpi.w #0x2,(0x00ffdf40).l
0000B966: bne.w 0x0000b9a8
0000B96A: cmpi.w #0x2,D0w
0000B96E: blt.w 0x0000b9a8
0000B972: cmpi.w #0x5,D0w
0000B976: bgt.w 0x0000b9a8
0000B97A: tst.b (0x00ffe17f).l
0000B980: bne.w 0x0000b9a8
0000B984: move.l #0x20e48,(0x00ffdd2c).l
0000B98E: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B994: move.b D0b,(0x00ffe16e).l
0000B99A: move.w (0x00ffdfe4).l,(0x00ffe11a).l
0000B9A4: bra.w 0x0000baae
0000B9A8: cmpi.w #0x18,D0w
0000B9AC: bne.w 0x0000b9c4
0000B9B0: move.l #0x20ec2,(0x00ffdd2c).l
0000B9BA: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000B9C0: bra.w 0x0000baae
0000B9C4: cmpi.w #0x19,D0w
0000B9C8: bne.w 0x0000ba08
0000B9CC: cmpi.w #0x13,(0x00fffc10).l
0000B9D4: beq.w 0x0000b9f4
0000B9D8: move.w #0xa8,D0w
0000B9DC: move.b D0b,(0x00ffef10).l
0000B9E2: move.w D0w,(0x00ffdf64).l
0000B9E8: move.w D0w,(0xc6,A0)
0000B9EC: andi.w #0x1f,D0w
0000B9F0: bra.w 0x0000ba08
0000B9F4: move.l #0x53efa,(0x00ffdd2c).l
0000B9FE: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000BA04: bra.w 0x0000baae
0000BA08: btst.b #0x6,(0x00ff0110).l
0000BA10: bne.w 0x0000ba20
0000BA14: btst.b #0x4,(0x00ff0110).l
0000BA1C: beq.w 0x0000ba3a
0000BA20: jsr 0x00053126.l ; call 00053126 FUN_00053126
0000BA26: move.l #0xc64c,(0x00ffdd2c).l
0000BA30: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
0000BA36: bra.w 0x0000baae
0000BA3A: move.b D0b,(0x00fffbf9).l
0000BA40: tst.w (0x00ffdf40).l
0000BA46: beq.w 0x0000ba7c
0000BA4A: cmpi.w #0x14,(0x00ffddc4).l
0000BA52: beq.w 0x0000ba7c
0000BA56: cmpi.w #0x2,(0x00ffdf40).l
0000BA5E: bne.w 0x0000ba7c
0000BA62: tst.w D0w
0000BA64: bne.w 0x0000ba7c
0000BA68: tst.w (0x00ffdf84).l
0000BA6E: bne.w 0x0000baae
0000BA72: tst.b (0x00ffe16f).l
0000BA78: bne.w 0x0000baae
0000BA7C: jsr 0x0001651e.l ; call 0001651E FUN_0001651e
0000BA82: bcc.w 0x0000baa2
0000BA86: move.w #-0x1,(0x00ffdf3a).l
0000BA8E: move.w #0x10,(0x00ffdf3c).l
0000BA96: move.w #-0x1,(0x00ffdfe8).l
0000BA9E: bra.w 0x0000b7cc
0000BAA2: move.w (0x00ffdf64).l,(0xc6,A0)
0000BAAA: bra.w 0x0000b7cc
0000BAAE: bclr.b #0x5,(0xb0,A0)
0000BAB4: bset.b #0x0,(0xb0,A0)
0000BABA: rts
0000BABC: lea (0xc002).l,A1 ; data 0000c002
0000BAC2: move.w #-0xc,(0x8,A0)
0000BAC8: move.w (0x2e,A0),D0w
0000BACC: rol.w #0x3,D0w
0000BACE: bpl.w 0x0000bad4
0000BAD2: addq.w #0x1,D0w
0000BAD4: andi.w #0x7,D0w
0000BAD8: lsl.w #0x1,D0w
0000BADA: bra.w 0x0000bade ; call 0000BADE advance_entity_movement_step
0000BADE: move.w (0x4,A1),(0xca,A0)
0000BAE4: bsr.w 0x0000b77e ; call 0000B77E probe_entity_directional_collision
0000BAE8: tst.w D0w
0000BAEA: bne.w 0x0000baf8
0000BAEE: move.w #0x1000,(0xb8,A0)
0000BAF4: bra.w 0x0000bbe4 ; call 0000BBE4 commit_entity_position
0000BAF8: tst.w D0w
0000BAFA: bpl.w 0x0000bb0a
0000BAFE: bsr.w 0x0000b7b8 ; call 0000B7B8 FUN_0000b7b8
0000BB02: btst.b #0x0,(0xb0,A0)
0000BB08: beq.b 0x0000baee
0000BB0A: move.w (0x2e,A0),D0w
0000BB0E: andi.w #0x3fff,D0w
0000BB12: beq.w 0x0000bb5e
0000BB16: move.w (0x2e,A0),D0w
0000BB1A: rol.w #0x3,D0w
0000BB1C: andi.w #0x6,D0w
0000BB20: move.w D0w,(0x00ffdd3c).l
0000BB26: move.w D0w,(0xb8,A0)
0000BB2A: lsl.w #0x1,D0w
0000BB2C: bsr.w 0x0000b732 ; call 0000B732 apply_movement_result_policy
0000BB30: tst.w D0w
0000BB32: beq.w 0x0000bb70
0000BB36: bmi.w 0x0000bb70
0000BB3A: move.w (0x00ffdd3c).l,D0w
0000BB40: move.w D0w,(0xb8,A0)
0000BB44: addq.w #0x2,D0w
0000BB46: andi.w #0x7,D0w
0000BB4A: lsl.w #0x1,D0w
0000BB4C: bsr.w 0x0000b732 ; call 0000B732 apply_movement_result_policy
0000BB50: tst.w D0w
0000BB52: beq.w 0x0000bb70
0000BB56: bmi.w 0x0000bb70
0000BB5A: bra.w 0x0000bb66
0000BB5E: bsr.w 0x0000bb86 ; call 0000BB86 evaluate_entity_movement_route
0000BB62: bcc.w 0x0000bbe4
0000BB66: move.w #-0x1,(0xb8,A0)
0000BB6C: bra.w 0x0000b442 ; call 0000B442 reset_entity_movement_state
0000BB70: move.w (0x00ffdf66).l,D1w
0000BB76: ror.w #0x4,D1w
0000BB78: move.w D1w,(0x36,A0)
0000BB7C: bset.b #0x2,(0x11,A0)
0000BB82: bra.w 0x0000bbe4 ; call 0000BBE4 commit_entity_position
0000BB86: move.w (0x2e,A0),D0w
0000BB8A: rol.w #0x3,D0w
0000BB8C: andi.w #0x7,D0w
0000BB90: bclr.l #0x0,D0
0000BB94: lsl.w #0x1,D0w
0000BB96: lea (0xbbf6).l,A2 ; data 0000bbf6
0000BB9C: movea.l (0x0,A2,D0w*0x1),A2 ; data 0000bbf6
0000BBA0: movea.l (A2)+,A3
0000BBA2: move.b (A3),D0b
0000BBA4: beq.w 0x0000bbac
0000BBA8: bpl.w 0x0000bbb6
0000BBAC: adda.w #0x4,A2
0000BBB0: move.w (A2),D0w
0000BBB2: bra.w 0x0000bbc8
0000BBB6: movea.l (A2)+,A3
0000BBB8: move.b (A3),D0b
0000BBBA: beq.w 0x0000bbc2
0000BBBE: bpl.w 0x0000bbde
0000BBC2: adda.w #0x2,A2
0000BBC6: move.w (A2),D0w
0000BBC8: lsl.w #0x1,D0w
0000BBCA: bsr.w 0x0000b732 ; call 0000B732 apply_movement_result_policy
0000BBCE: tst.w D0w
0000BBD0: beq.w 0x0000bbd8
0000BBD4: bpl.w 0x0000bbde
0000BBD8: andi #0x1e,CCR
0000BBDC: rts
0000BBDE: ori #0x1,CCR
0000BBE2: rts
0000BBE4: bclr.b #0x0,(0xb0,A0)
0000BBEA: move.w D5w,(A0)
0000BBEC: move.w D6w,(0x4,A0)
0000BBF0: andi #0x1e,CCR
0000BBF4: rts
0000BC36: clr.w D1w
0000BC38: move.b (0x00ffef10).l,D1b
0000BC3E: bmi.w 0x0000bc52
0000BC42: move.b (0x00ffef11).l,D1b
0000BC48: bmi.w 0x0000bc52
0000BC4C: move.b (0x00ffef12).l,D1b
0000BC52: rol.b #0x3,D1b
0000BC54: andi.b #0x3,D1b
0000BC58: move.w D1w,D4w
0000BC5A: tst.w (0x00ffdf40).l
0000BC60: beq.w 0x0000bc66
0000BC64: rts
0000BC66: lea (0xffdf44).l,A4 ; data 00ffdf44
0000BC6C: lea (0xbd34).l,A3 ; data 0000bd34
0000BC72: mulu.w #0xc,D1
0000BC76: move.w (A0),D2w
0000BC78: move.w (0x4,A0),D3w
0000BC7C: move.w #0x2,D7w
0000BC80: move.w (0x0,A3,D1w*0x1),D0w ; data 0000bd34
0000BC84: addq.w #0x2,D1w
0000BC86: add.w D2w,D0w
0000BC88: move.w D0w,(A4)+
0000BC8A: move.w (0x0,A3,D1w*0x1),D0w ; data 0000bd34
0000BC8E: addq.w #0x2,D1w
0000BC90: add.w D3w,D0w
0000BC92: move.w D0w,(A4)+
0000BC94: dbf D7w,0x0000bc80
0000BC98: move.w (0x00ffddc6).l,(0x00ffdf50).l
0000BCA2: move.w (0x00ffddc8).l,(0x00ffdf52).l
0000BCAC: rts
0000BCAE: cmpi.w #0x5,(0x00ffddc4).l
0000BCB6: bne.w 0x0000bd02
0000BCBA: cmpi.w #0x1200,(0x4,A0)
0000BCC0: bcs.w 0x0000bd02
0000BCC4: cmpi.w #0x1f80,(0x4,A0)
0000BCCA: bcc.w 0x0000bd02
0000BCCE: cmpi.w #0x2600,(A0)
0000BCD2: bcs.w 0x0000bd02
0000BCD6: cmpi.w #0x2800,(A0)
0000BCDA: bcs.w 0x0000bcf2
0000BCDE: cmpi.w #0x3c00,(A0)
0000BCE2: bcs.w 0x0000bd02
0000BCE6: cmpi.w #0x3e00,(A0)
0000BCEA: bcs.w 0x0000bcfa
0000BCEE: bra.w 0x0000bd02
0000BCF2: subi.w #0x8,(0x4,A0)
0000BCF8: rts
0000BCFA: addi.w #0x8,(0x4,A0)
0000BD00: rts
0000BD02: rts
0000C0C2: lea (0xffbf40).l,A1 ; data 00ffbf40
0000C0C8: movea.l #0xc1064,A3
0000C0CE: move.l #0xb560,D0
0000C0D4: jmp 0x00001530.l ; call 00001530 FUN_00001530
0000C0DA: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0000C0DE: move SR,-(SP) ; data Stack[-0x3e]
0000C0E0: move #0x2700,SR
0000C0E4: lea (0xc1320).l,A0 ; data 000c1320
0000C0EA: move.l A2,D0
0000C0EC: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0000C0F2: lea (0xc00000).l,A3 ; data 00c00000
0000C0F8: clr.w D0w
0000C0FA: move.b (A1)+,D0b
0000C0FC: bmi.w 0x0000c11e
0000C100: subi.w #0x20,D0w
0000C104: cmpi.b #0x3d,D0b
0000C108: bmi.w 0x0000c10e
0000C10C: subq.w #0x4,D0w
0000C10E: lsl.w #0x1,D0w
0000C110: move.w (0x0,A0,D0w*0x1),D0w
0000C114: or.w D1w,D0w
0000C116: ori.w #-0x8000,D0w
0000C11A: move.w D0w,(A3)
0000C11C: bra.b 0x0000c0f8
0000C11E: lsl.b #0x1,D0b
0000C120: bmi.w 0x0000c12c
0000C124: adda.l #0x80,A2
0000C12A: bra.b 0x0000c0ea
0000C12C: move (SP)+,SR
0000C12E: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0000C132: rts
0000C134: move.l D0,-(SP) ; data Stack[-0x4]
0000C136: clr.l D0
0000C138: move.b (A1)+,D0b
0000C13A: bmi.w 0x0000c142
0000C13E: move.b D0b,(A5)+
0000C140: bra.b 0x0000c138
0000C142: lsl.b #0x1,D0b
0000C144: bmi.w 0x0000c14e
0000C148: move.b #-0x80,D0b
0000C14C: bra.b 0x0000c13e
0000C14E: tst.w D1w
0000C150: bmi.w 0x0000c15c
0000C154: move.b #0x20,(A5)+
0000C158: dbf D1w,0x0000c154
0000C15C: move.l (SP)+,D0
0000C15E: rts
0000C160: move.b #0x1,(0x00fff0b2).l
0000C168: movem.l {  A5 A4 A3 A2 A1 A0 D6 D5 D4 D3 D2 D1 D0},-(SP)
0000C16C: move SR,-(SP) ; data Stack[-0x36]
0000C16E: move #0x2700,SR
0000C172: lea (0xc1320).l,A0 ; data 000c1320
0000C178: tst.b (0x00fff0b2).l
0000C17E: beq.w 0x0000c18c
0000C182: lea (0xffef96).l,A1 ; data 00ffef96
0000C188: bra.w 0x0000c192
0000C18C: lea (0xffb740).l,A1 ; data 00ffb740
0000C192: move.l A2,D0
0000C194: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0000C19A: lea (0xc00000).l,A3 ; data 00c00000
0000C1A0: movea.l A1,A4
0000C1A2: clr.l D0
0000C1A4: clr.l D4
0000C1A6: move.w D4w,D7w
0000C1A8: move.b (A1)+,D0b
0000C1AA: addq.w #0x1,D4w
0000C1AC: tst.b D0b
0000C1AE: bmi.w 0x0000c1bc
0000C1B2: cmpi.b #0x20,D0b
0000C1B6: bne.b 0x0000c1a8
0000C1B8: cmp.w D4w,D1w
0000C1BA: bgt.b 0x0000c1a6
0000C1BC: cmp.w D4w,D1w
0000C1BE: blt.w 0x0000c1c4
0000C1C2: move.w D4w,D7w
0000C1C4: move.w D7w,D4w
0000C1C6: movea.l A4,A1
0000C1C8: subq.w #0x1,D4w
0000C1CA: clr.w D0w
0000C1CC: move.b (A1)+,D0b
0000C1CE: bmi.w 0x0000c1fa
0000C1D2: subi.w #0x20,D0w
0000C1D6: cmpi.b #0x3d,D0b
0000C1DA: bmi.w 0x0000c1e0
0000C1DE: subq.w #0x4,D0w
0000C1E0: lsl.w #0x1,D0w
0000C1E2: move.w (0x0,A0,D0w*0x1),D0w
0000C1E6: or.w D2w,D0w
0000C1E8: ori.w #-0x8000,D0w
0000C1EC: move.w D0w,(A3)
0000C1EE: dbf D4w,0x0000c1ca
0000C1F2: adda.l #0x80,A2
0000C1F8: bra.b 0x0000c192
0000C1FA: lsl.b #0x1,D0b
0000C1FC: bmi.w 0x0000c208
0000C200: adda.l #0x80,A2
0000C206: bra.b 0x0000c192
0000C208: move (SP)+,SR
0000C20A: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 A0 A1 A2 A3 A4 A5}
0000C20E: clr.b (0x00fff0b2).l
0000C214: rts
0000C216: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0000C21A: move SR,-(SP) ; data Stack[-0x3e]
0000C21C: move #0x2700,SR
0000C220: lea (0xc1320).l,A0 ; data 000c1320
0000C226: moveq #0x10,D5
0000C228: moveq #0x4,D4
0000C22A: move.l A2,D0
0000C22C: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0000C232: lea (0xc00000).l,A3 ; data 00c00000
0000C238: clr.l D0
0000C23A: clr.l D3
0000C23C: subq.w #0x1,D4w
0000C23E: bne.w 0x0000c24c
0000C242: move.w #0x18,D0w
0000C246: moveq #0x4,D4
0000C248: bra.w 0x0000c288
0000C24C: move.l D1,D0
0000C24E: beq.w 0x0000c27a
0000C252: divu.w #0xa,D0
0000C256: bvc.w 0x0000c27a
0000C25A: divu.w #-0x6,D0
0000C25E: move.l D0,(0x00fff096).l
0000C264: clr.l D0
0000C266: move.w (0x00fff096).l,D0w
0000C26C: move.l #0x1999,D3
0000C272: mulu.w (0x00fff098).l,D3
0000C278: bra.b 0x0000c252
0000C27A: clr.l D1
0000C27C: move.w D0w,D1w
0000C27E: add.l D3,D1
0000C280: lsr.l D5,D0
0000C282: lsl.w #0x1,D0w
0000C284: addi.w #0x20,D0w
0000C288: move.w (0x0,A0,D0w*0x1),D0w
0000C28C: or.w D2w,D0w
0000C28E: ori.w #-0x8000,D0w
0000C292: move.w D0w,(A3)
0000C294: subq.l #0x2,A2
0000C296: tst.w D1w
0000C298: beq.w 0x0000c29e
0000C29C: bra.b 0x0000c22a
0000C29E: move (SP)+,SR
0000C2A0: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0000C2A4: rts
0000C2A6: bsr.w 0x00010a7e ; call 00010A7E FUN_00010a7e
0000C2AA: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000C2AE: lea (0xc1418).l,A1 ; data 000c1418
0000C2B4: movea.l #0xc216,A2
0000C2BA: move.w #0x6000,D1w
0000C2BE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000C2C4: bsr.w 0x0000c3f8 ; call 0000C3F8 FUN_0000c3f8
0000C2C8: clr.b (0x00fff0dc).l
0000C2CE: clr.w D2w
0000C2D0: bra.w 0x0000c30a
0000C2D4: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000C2DA: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000C2E0: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000C2E6: bsr.w 0x0000e660 ; call 0000E660 FUN_0000e660
0000C2EA: btst.b #0x8,(0x00ffdd22).l
0000C2F2: beq.w 0x0000c2fc
0000C2F6: moveq #-0x1,D2
0000C2F8: bra.w 0x0000c30a
0000C2FC: btst.b #0x9,(0x00ffdd22).l
0000C304: beq.w 0x0000c32e
0000C308: moveq #0x1,D2
0000C30A: tst.b (0x00fff101).l
0000C310: bne.w 0x0000c31c
0000C314: move.w #0x4,D3w
0000C318: bra.w 0x0000c320
0000C31C: move.w #0x5,D3w
0000C320: move.w #-0x3c6e,D7w
0000C324: move.w #0x6000,D4w
0000C328: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000C32C: bra.b 0x0000c2d4
0000C32E: btst.b #0xe,(0x00ffdd22).l
0000C336: beq.w 0x0000c352
0000C33A: lea (0xc1400).l,A0 ; data 000c1400
0000C340: bsr.w 0x0000c436 ; call 0000C436 FUN_0000c436
0000C344: tst.b (0x00fff0dc).l
0000C34A: bmi.w 0x0000c374
0000C34E: bra.w 0x0000c2aa
0000C352: btst.b #0xd,(0x00ffdd22).l
0000C35A: beq.w 0x0000c368
0000C35E: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000C364: bra.w 0x0000c2d4
0000C368: btst.b #0xf,(0x00ffdd22).l
0000C370: beq.w 0x0000c2d4
0000C374: rts
0000C376: move.w #-0x7a55,D1w
0000C37A: bsr.w 0x0000c3b2 ; call 0000C3B2 FUN_0000c3b2
0000C37E: add.b D2b,(0x00fff0dc).l
0000C384: bge.w 0x0000c392
0000C388: move.b D3b,(0x00fff0dc).l
0000C38E: bra.w 0x0000c3a2
0000C392: cmp.b (0x00fff0dc).l,D3b
0000C398: bpl.w 0x0000c3a2
0000C39C: clr.b (0x00fff0dc).l
0000C3A2: move.w #-0x7a01,D1w
0000C3A6: bsr.w 0x0000c3b2 ; call 0000C3B2 FUN_0000c3b2
0000C3AA: clr.b (0x00fff0fc).l
0000C3B0: rts
0000C3B2: or.w D4w,D1w
0000C3B4: clr.l D0
0000C3B6: move.b (0x00fff0dc).l,D0b
0000C3BC: tst.b (0x00fff0fc).l
0000C3C2: bmi.w 0x0000c3c8
0000C3C6: lsl.w #0x1,D0w
0000C3C8: mulu.w #0x80,D0
0000C3CC: add.w D7w,D0w
0000C3CE: move SR,-(SP) ; data Stack[-0x2]
0000C3D0: move #0x2700,SR
0000C3D4: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0000C3DA: lea (0xc00000).l,A0 ; data 00c00000
0000C3E0: move.w D1w,(A0)
0000C3E2: move (SP)+,SR
0000C3E4: rts
0000C3E6: move.l #0xc186,D4
0000C3EC: moveq #0x14,D1
0000C3EE: moveq #0x1b,D7
0000C3F0: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0000C3F6: rts
0000C3F8: move.w #-0x3672,D4w
0000C3FC: movea.l #0xca2c,A2
0000C402: bra.w 0x0000c410
0000C406: move.w #-0x33f2,D4w
0000C40A: movea.l #0xccac,A2
0000C410: lea (0xcd996).l,A1 ; data 000cd996
0000C416: moveq #0x13,D7
0000C418: moveq #0x2,D1
0000C41A: move.w #0x0,D2w
0000C41E: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000C424: move.l (0x00fffbfe).l,D1
0000C42A: move.w #0x6000,D2w
0000C42E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000C434: rts
0000C436: clr.l D0
0000C438: move.b (0x00fff0dc).l,D0b
0000C43E: lsl.w #0x2,D0w
0000C440: movea.l (0x0,A0,D0w*0x1),A0
0000C444: jsr (A0)
0000C446: rts
0000C64C: clr.b (0x00ffe186).l
0000C652: clr.b (0x00ffe182).l
0000C658: move.b #0x0,(0x00fff10c).l
0000C660: cmpi.b #0x5,(0x00fffbed).l
0000C668: bne.w 0x0000c6b0
0000C66C: btst.b #0x0,(0x00fffc1c).l
0000C674: beq.w 0x0000c716
0000C678: btst.b #0x7,(0x00fffc1c).l
0000C680: bne.w 0x0000c716
0000C684: clr.b (0x00ffe183).l
0000C68A: lea (0xf3d5c).l,A0 ; data 000f3d5c
0000C690: move.l A0,-(SP) ; data 000f3d5c
0000C692: jsr 0x0001ded8.l ; call 0001DED8 FUN_0001ded8
0000C698: movea.l (SP)+,A0
0000C69A: jsr 0x0001e68c.l ; call 0001E68C FUN_0001e68c
0000C6A0: lea (0x1220b4).l,A5 ; data 001220b4
0000C6A6: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0000C6AC: bra.w 0x0000c716
0000C6B0: cmpi.b #0x1,(0x00fffbed).l
0000C6B8: bne.w 0x0000c6d0
0000C6BC: btst.b #0x0,(0x00fffc1c).l
0000C6C4: beq.w 0x0000c716
0000C6C8: lea (0xf359e).l,A0 ; data 000f359e
0000C6CE: bra.b 0x0000c690
0000C6D0: cmpi.b #0x2,(0x00fffbed).l
0000C6D8: bne.w 0x0000c700
0000C6DC: btst.b #0x0,(0x00fffc1c).l
0000C6E4: beq.w 0x0000c716
0000C6E8: move.l (0x00fffc22).l,D0
0000C6EE: bclr.l #0x9,D0
0000C6F2: move.l D0,(0x00fffc22).l
0000C6F8: lea (0xf2510).l,A0 ; data 000f2510
0000C6FE: bra.b 0x0000c69a
0000C700: cmpi.b #0x4,(0x00fffbed).l
0000C708: bne.w 0x0000c716
0000C70C: btst.b #0x0,(0x00fffc1c).l
0000C714: bne.b 0x0000c6e8
0000C716: clr.b (0x00ff024c).l
0000C71C: clr.b (0x00ff034c).l
0000C722: bsr.w 0x0000c802 ; call 0000C802 FUN_0000c802
0000C726: bclr.b #0x6,(0x00ff0110).l
0000C72E: bclr.b #0x4,(0x00ff0110).l
0000C736: bset.b #0x0,(0x00ff0110).l
0000C73E: bclr.b #0x6,(0x00ff0111).l
0000C746: move.l #0xb45c,(0x00ff013a).l
0000C750: cmpi.b #0x6,(0x00ff014c).l
0000C758: bpl.w 0x0000c764
0000C75C: move.b #0x6,(0x00ff014c).l
0000C764: cmpi.b #0x8,(0x00ff019c).l
0000C76C: bpl.w 0x0000c778
0000C770: move.b #0x8,(0x00ff019c).l
0000C778: move.l (0x00fffc1e).l,D0
0000C77E: btst.l #0x0,D0
0000C782: bne.w 0x0000c796
0000C786: lea (0xc25bc).l,A0 ; data 000c25bc
0000C78C: lea (0x12200c).l,A5 ; data 0012200c
0000C792: bra.w 0x0000c7a2
0000C796: lea (0xc25a0).l,A0 ; data 000c25a0
0000C79C: lea (0x1220b4).l,A5 ; data 001220b4
0000C7A2: move.l (0x00fffbfe).l,D1
0000C7A8: lsr.l #0x3,D1
0000C7AA: cmpi.l #0x1388,D1
0000C7B0: bmi.w 0x0000c7ba
0000C7B4: move.l #0x1388,D1
0000C7BA: sub.l D1,(0x00fffbfe).l
0000C7C0: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0000C7C6: jsr 0x000038fa.l ; call 000038FA FUN_000038fa
0000C7CC: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000C7D2: move.l (0x00fffc1e).l,D0
0000C7D8: btst.l #0x0,D0
0000C7DC: beq.w 0x0000c7ec
0000C7E0: jsr 0x00002a32.l ; call 00002A32 FUN_00002a32
0000C7E6: move.w #-0x1,D6w
0000C7EA: rts
0000C7EC: jsr 0x00002a44.l ; call 00002A44 FUN_00002a44
0000C7F2: move.w #-0x1,D6w
0000C7F6: rts
0000C7F8: tst.b (0x00ff014c).l
0000C7FE: beq.w 0x0000c64c
0000C802: jsr 0x00059798.l ; call 00059798 FUN_00059798
0000C808: clr.l D7
0000C80A: move.w #-0x2,D6w
0000C80E: lea (0xff0200).l,A0 ; data 00ff0200
0000C814: btst.b #0x0,(0x10,A0)
0000C81A: beq.w 0x0000c858
0000C81E: tst.b (0x4c,A0)
0000C822: beq.w 0x0000c82e
0000C826: tst.b (0x9c,A0)
0000C82A: bne.w 0x0000c858
0000C82E: move.b (0x00fffc0c).l,D7b
0000C834: move.b #-0x1,(0x00fffc0c).l
0000C83C: bsr.w 0x0000c8b0 ; call 0000C8B0 apply_byte_counter_delta_with_negative_floor
0000C840: jsr 0x0001e514.l ; call 0001E514 FUN_0001e514
0000C846: move.l A0,D0
0000C848: cmp.l (0x00fffc04).l,D0
0000C84E: bne.w 0x0000c858
0000C852: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000C858: lea (0xff0300).l,A0 ; data 00ff0300
0000C85E: btst.b #0x0,(0x10,A0)
0000C864: beq.w 0x0000c8a2
0000C868: tst.b (0x4c,A0)
0000C86C: beq.w 0x0000c878
0000C870: tst.b (0x9c,A0)
0000C874: bne.w 0x0000c8a2
0000C878: move.b (0x00fffc0d).l,D7b
0000C87E: move.b #-0x1,(0x00fffc0d).l
0000C886: bsr.w 0x0000c8b0 ; call 0000C8B0 apply_byte_counter_delta_with_negative_floor
0000C88A: jsr 0x0001e514.l ; call 0001E514 FUN_0001e514
0000C890: move.l A0,D0
0000C892: cmp.l (0x00fffc04).l,D0
0000C898: bne.w 0x0000c8a2
0000C89C: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000C8A2: jsr 0x00003404.l ; call 00003404 FUN_00003404
0000C8A8: jsr 0x000038fa.l ; call 000038FA FUN_000038fa
0000C8AE: rts
0000C8B0: subq.w #0x1,D7w
0000C8B2: lea (0xfffc12).l,A5 ; data 00fffc12
0000C8B8: add.b D6b,(0x0,A5,D7w*0x1) ; data 00fffc12
0000C8BC: bpl.w 0x0000c8c6
0000C8C0: move.b #-0x1,(0x0,A5,D7w*0x1) ; data 00fffc12
0000C8C6: rts
0000C900: move.w #0xe,D7w
0000C904: btst.l D7,D0
0000C906: beq.w 0x0000c91e
0000C90A: move.w D7w,D2w
0000C90C: lsl.w #0x2,D2w
0000C90E: movea.l (0x0,A0,D2w*0x1),A1
0000C912: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000C918: adda.l #0x100,A2
0000C91E: dbf D7w,0x0000c904
0000C922: jmp 0x0000f312.l ; call 0000F312 wait_for_controller_event
0000C928: jsr 0x00010952.l ; call 00010952 FUN_00010952
0000C92E: move.b #0x1,(0x00fff0b7).l
0000C936: lea (0xd29d6).l,A0 ; data 000d29d6
0000C93C: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
0000C942: jsr 0x0005670e.l ; call 0005670E FUN_0005670e
0000C948: bsr.w 0x00010a82 ; call 00010A82 FUN_00010a82
0000C94C: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
0000C952: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000C956: lea (0xc150c).l,A1 ; data 000c150c
0000C95C: movea.l #0xc310,A2
0000C962: move.w #0x6000,D1w
0000C966: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000C96C: clr.b (0x00fff0dc).l
0000C972: clr.w D2w
0000C974: moveq #0x2,D3
0000C976: move.w #-0x3b70,D7w
0000C97A: move.w #0x6000,D4w
0000C97E: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000C982: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000C988: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000C98E: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000C994: bsr.w 0x0000e660 ; call 0000E660 FUN_0000e660
0000C998: btst.b #0x8,(0x00ffdd22).l
0000C9A0: beq.w 0x0000c9aa
0000C9A4: moveq #-0x1,D2
0000C9A6: bra.w 0x0000c9b8
0000C9AA: btst.b #0x9,(0x00ffdd22).l
0000C9B2: beq.w 0x0000c9c6
0000C9B6: moveq #0x1,D2
0000C9B8: moveq #0x2,D3
0000C9BA: move.w #-0x3b70,D7w
0000C9BE: move.w #0x6000,D4w
0000C9C2: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000C9C6: btst.b #0xe,(0x00ffdd22).l
0000C9CE: beq.w 0x0000c9ea
0000C9D2: lea (0xc1558).l,A0 ; data 000c1558
0000C9D8: bsr.w 0x0000c436 ; call 0000C436 FUN_0000c436
0000C9DC: tst.b (0x00fff0dc).l
0000C9E2: bmi.w 0x0000ca0a
0000C9E6: bra.w 0x0000c936
0000C9EA: btst.b #0xd,(0x00ffdd22).l
0000C9F2: beq.w 0x0000c9fe
0000C9F6: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000C9FC: bra.b 0x0000c982
0000C9FE: btst.b #0xf,(0x00ffdd22).l
0000CA06: beq.w 0x0000c982
0000CA0A: clr.b (0x00fff0dc).l
0000CA10: move.w #-0x1,D6w
0000CA14: rts
0000CA16: move.b #0x1,(0x00fff0b7).l
0000CA1E: jsr 0x00056ac0.l ; call 00056AC0 FUN_00056ac0
0000CA24: move.b #-0x1,(0x00fff0dc).l
0000CA2C: rts
0000CCCC: ori.b #0x12,D0b
0000CCD0: move.w #0x6000,D1w
0000CCD4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000CCDA: lea (0xcd996).l,A1 ; data 000cd996
0000CCE0: move.w #-0x3672,D4w
0000CCE4: moveq #0x13,D7
0000CCE6: moveq #0x2,D1
0000CCE8: move.w #0x0,D2w
0000CCEC: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000CCF2: movea.l #0xca2c,A2
0000CCF8: move.l (0x00fffbfe).l,D1
0000CCFE: move.w #0x6000,D2w
0000CD02: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000CD08: clr.b (0x00fff0dc).l
0000CD0E: clr.w D2w
0000CD10: bra.w 0x0000cd4a
0000CD14: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000CD1A: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000CD20: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000CD26: bsr.w 0x0000e660 ; call 0000E660 FUN_0000e660
0000CD2A: btst.b #0x8,(0x00ffdd22).l
0000CD32: beq.w 0x0000cd3c
0000CD36: moveq #-0x1,D2
0000CD38: bra.w 0x0000cd4a
0000CD3C: btst.b #0x9,(0x00ffdd22).l
0000CD44: beq.w 0x0000cd62
0000CD48: moveq #0x1,D2
0000CD4A: clr.w D3w
0000CD4C: move.b (0x00fff0b9).l,D3b
0000CD52: move.l #0xc48c,D7
0000CD58: move.w #0x6000,D4w
0000CD5C: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000CD60: bra.b 0x0000cd14
0000CD62: btst.b #0xe,(0x00ffdd22).l
0000CD6A: beq.w 0x0000cde8
0000CD6E: lea (0xfff0be).l,A2 ; data 00fff0be
0000CD74: clr.l D0
0000CD76: move.b (0x00fff0dc).l,D0b
0000CD7C: lsl.w #0x1,D0w
0000CD7E: move.w (0x0,A2,D0w*0x1),D0w
0000CD82: cmp.l (0x00fffbfe).l,D0
0000CD88: bmi.w 0x0000cdac
0000CD8C: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000CD90: lea (0xc15fb).l,A1 ; data 000c15fb
0000CD96: movea.l #0xc50e,A2
0000CD9C: move.w #0x6000,D1w
0000CDA0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000CDA6: jmp 0x0000f312.l ; call 0000F312 wait_for_controller_event
0000CDAC: sub.l D0,(0x00fffbfe).l
0000CDB2: movea.l (0x00fffc04).l,A0
0000CDB8: move.b #0xa,(0x4c,A0)
0000CDBE: clr.l D0
0000CDC0: move.b (0x00fff0dc).l,D0b
0000CDC6: subq.w #0x1,D0w
0000CDC8: bmi.w 0x0000cdda
0000CDCC: lea (0x166ad4).l,A1 ; data 00166ad4
0000CDD2: move.b (0x0,A1,D0w*0x1),D0b
0000CDD6: add.b D0b,(0x4c,A0)
0000CDDA: move.b #0xa,(0x9c,A0)
0000CDE0: jsr 0x000038fa.l ; call 000038FA FUN_000038fa
0000CDE6: rts
0000CDE8: btst.b #0xf,(0x00ffdd22).l
0000CDF0: beq.w 0x0000cd14
0000CDF4: rts
0000CF38: clr.b (0x00fff10d).l
0000CF3E: jsr 0x00010952.l ; call 00010952 FUN_00010952
0000CF44: bsr.w 0x0000cffe ; call 0000CFFE FUN_0000cffe
0000CF48: jsr 0x0000ef08.l ; call 0000EF08 FUN_0000ef08
0000CF4E: bsr.w 0x0000cffe ; call 0000CFFE FUN_0000cffe
0000CF52: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000CF56: lea (0xc14d0).l,A1 ; data 000c14d0
0000CF5C: movea.l #0xc312,A2
0000CF62: move.w #0x6000,D1w
0000CF66: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000CF6C: clr.b (0x00fff0dc).l
0000CF72: clr.w D2w
0000CF74: moveq #0x1,D3
0000CF76: move.w #-0x3b6e,D7w
0000CF7A: move.w #0x6000,D4w
0000CF7E: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000CF82: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000CF88: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000CF8E: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000CF94: bsr.w 0x0000e660 ; call 0000E660 FUN_0000e660
0000CF98: btst.b #0x8,(0x00ffdd22).l
0000CFA0: beq.w 0x0000cfaa
0000CFA4: moveq #-0x1,D2
0000CFA6: bra.w 0x0000cfb8
0000CFAA: btst.b #0x9,(0x00ffdd22).l
0000CFB2: beq.w 0x0000cfc6
0000CFB6: moveq #0x1,D2
0000CFB8: moveq #0x1,D3
0000CFBA: move.w #-0x3b6e,D7w
0000CFBE: move.w #0x6000,D4w
0000CFC2: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000CFC6: btst.b #0xe,(0x00ffdd22).l
0000CFCE: beq.w 0x0000cfea
0000CFD2: lea (0xc14c8).l,A0 ; data 000c14c8
0000CFD8: bsr.w 0x0000c436 ; call 0000C436 FUN_0000c436
0000CFDC: tst.b (0x00fff0dc).l
0000CFE2: bmi.w 0x0000cffc
0000CFE6: bra.w 0x0000cf4e
0000CFEA: btst.b #0xd,(0x00ffdd22).l
0000CFF2: beq.b 0x0000cf82
0000CFF4: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000CFFA: bra.b 0x0000cf82
0000CFFC: rts
0000CFFE: lea (0xd29d6).l,A0 ; data 000d29d6
0000D004: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
0000D00A: jsr 0x0005670e.l ; call 0005670E FUN_0005670e
0000D010: bsr.w 0x00010a82 ; call 00010A82 FUN_00010a82
0000D014: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
0000D01A: rts
0000D01C: move.w #-0x1,(0x00fff116).l
0000D024: move.w #-0x6f00,(0x00c00004).l
0000D02C: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0000D032: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0000D038: bsr.w 0x00010c96 ; call 00010C96 FUN_00010c96
0000D03C: lea (0xffd8d4).l,A1 ; data 00ffd8d4
0000D042: move.w #0x2c2,(A1)
0000D046: lea (0xffd8f4).l,A1 ; data 00ffd8f4
0000D04C: move.w #0x60,(A1)
0000D050: bset.b #0x0,(0x00ffef14).l
0000D058: lea (0xc1a2a).l,A0 ; data 000c1a2a
0000D05E: movea.l #0xc386,A2
0000D064: move.w #0x6000,D1w
0000D068: moveq #0x3,D7
0000D06A: movea.l (A0)+,A1
0000D06C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D072: adda.l #0x80,A2
0000D078: dbf D7w,0x0000d06a
0000D07C: movea.l #0xc3ac,A2
0000D082: moveq #0x3,D7
0000D084: movea.l (A0)+,A1
0000D086: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D08C: adda.l #0x80,A2
0000D092: dbf D7w,0x0000d084
0000D096: lea (0xc1a4a).l,A0 ; data 000c1a4a
0000D09C: movea.l #0xc686,A2
0000D0A2: moveq #0x5,D7
0000D0A4: movea.l (A0)+,A1
0000D0A6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D0AC: adda.l #0x80,A2
0000D0B2: dbf D7w,0x0000d0a4
0000D0B6: movea.l #0xc6ac,A2
0000D0BC: moveq #0x5,D7
0000D0BE: movea.l (A0)+,A1
0000D0C0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D0C6: adda.l #0x80,A2
0000D0CC: dbf D7w,0x0000d0be
0000D0D0: clr.w (0x00fff0d0).l
0000D0D6: lea (0xc1320).l,A1 ; data 000c1320
0000D0DC: move.l #0xc6a0,D4
0000D0E2: moveq #0x1,D7
0000D0E4: moveq #0x5,D1
0000D0E6: move.w #0x4000,D2w
0000D0EA: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0000D0F0: move.l #0xc6c4,D4
0000D0F6: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0000D0FC: clr.l D2
0000D0FE: movea.l (0x00fffc04).l,A0
0000D104: move.w (0x2c,A0),D2w
0000D108: divu.w #0x2000,D2
0000D10C: subq.w #0x1,D2w
0000D10E: lsl.w #0x2,D2w
0000D110: lea (0x18acb6).l,A0 ; data 0018acb6
0000D116: movea.l (0x0,A0,D2w*0x1),A0 ; data 0018acb6
0000D11A: jsr 0x000566e2.l ; call 000566E2 FUN_000566e2
0000D120: movea.l (0x00fffc04).l,A0
0000D126: move.l #0xe132,D4
0000D12C: move.w #0x2000,D2w
0000D130: bsr.w 0x000110ec ; call 000110EC FUN_000110ec
0000D134: movea.l #0xc194,A2
0000D13A: lea (0x58,A0),A1
0000D13E: move.w #0x4000,D1w
0000D142: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D148: lea (0xd19c7).l,A1 ; data 000d19c7
0000D14E: movea.l (0x00fffc04).l,A0
0000D154: adda.l #0x9a,A0
0000D15A: clr.l D0
0000D15C: move.b (A0),D0b
0000D15E: mulu.w #0x6,D0
0000D162: adda.l D0,A1
0000D164: movea.l #0xc214,A2
0000D16A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D170: lea (0xd19e5).l,A1 ; data 000d19e5
0000D176: movea.l (0x00fffc04).l,A0
0000D17C: adda.l #0x9b,A0
0000D182: move.b (A0),D0b
0000D184: mulu.w #0x8,D0
0000D188: adda.l D0,A1
0000D18A: movea.l #0xc220,A2
0000D190: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D196: clr.l D1
0000D198: movea.l (0x00fffc04).l,A0
0000D19E: adda.l #0x77,A0
0000D1A4: movea.l #0xc3a2,A2
0000D1AA: move.w #0x6000,D2w
0000D1AE: moveq #0x3,D7
0000D1B0: move.b (A0)+,D1b
0000D1B2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D1B8: adda.l #0x80,A2
0000D1BE: dbf D7w,0x0000d1b0
0000D1C2: movea.l #0xc3c6,A2
0000D1C8: moveq #0x1,D7
0000D1CA: move.b (A0)+,D1b
0000D1CC: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D1D2: adda.l #0x80,A2
0000D1D8: dbf D7w,0x0000d1ca
0000D1DC: move.b (A0)+,D1b
0000D1DE: subq.l #0x2,A2
0000D1E0: move.b (A0)+,D1b
0000D1E2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D1E8: adda.l #0x82,A2
0000D1EE: move.b (A0)+,D1b
0000D1F0: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D1F6: suba.l #0x80,A2
0000D1FC: addq.l #0x2,A2
0000D1FE: movea.l (0x00fffc04).l,A0
0000D204: adda.l #0x8e,A0
0000D20A: move.b (A0)+,D1b
0000D20C: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D212: movea.l (0x00fffc04).l,A0
0000D218: adda.l #0x80,A0
0000D21E: movea.l #0xc6a2,A2
0000D224: moveq #0x5,D7
0000D226: move.b (A0)+,D1b
0000D228: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D22E: adda.l #0x80,A2
0000D234: dbf D7w,0x0000d226
0000D238: movea.l (0x00fffc04).l,A0
0000D23E: adda.l #0x87,A0
0000D244: movea.l #0xc6c6,A2
0000D24A: moveq #0x5,D7
0000D24C: move.b (A0)+,D1b
0000D24E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D254: adda.l #0x80,A2
0000D25A: dbf D7w,0x0000d24c
0000D25E: move.w #0x4000,D1w
0000D262: bsr.w 0x0000d5dc ; call 0000D5DC FUN_0000d5dc
0000D266: bsr.w 0x0000d82a ; call 0000D82A FUN_0000d82a
0000D26A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000D270: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000D276: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000D27C: btst.b #0xa,(0x00ffdd22).l
0000D284: beq.w 0x0000d2a0
0000D288: move.w #0x6000,D1w
0000D28C: bsr.w 0x0000d5dc ; call 0000D5DC FUN_0000d5dc
0000D290: tst.b (0x00fff0d0).l
0000D296: beq.b 0x0000d25e
0000D298: clr.b (0x00fff0d0).l
0000D29E: bra.b 0x0000d25e
0000D2A0: btst.b #0xb,(0x00ffdd22).l
0000D2A8: beq.w 0x0000d2c6
0000D2AC: move.w #0x6000,D1w
0000D2B0: bsr.w 0x0000d5dc ; call 0000D5DC FUN_0000d5dc
0000D2B4: tst.b (0x00fff0d0).l
0000D2BA: bne.b 0x0000d25e
0000D2BC: move.b #0x1,(0x00fff0d0).l
0000D2C4: bra.b 0x0000d25e
0000D2C6: btst.b #0x8,(0x00ffdd22).l
0000D2CE: beq.w 0x0000d2ee
0000D2D2: move.w #0x6000,D1w
0000D2D6: bsr.w 0x0000d5dc ; call 0000D5DC FUN_0000d5dc
0000D2DA: subq.b #0x1,(0x00fff0d1).l
0000D2E0: bpl.w 0x0000d25e
0000D2E4: clr.b (0x00fff0d1).l
0000D2EA: bra.w 0x0000d25e
0000D2EE: btst.b #0x9,(0x00ffdd22).l
0000D2F6: beq.w 0x0000d31e
0000D2FA: move.w #0x6000,D1w
0000D2FE: bsr.w 0x0000d5dc ; call 0000D5DC FUN_0000d5dc
0000D302: addq.b #0x1,(0x00fff0d1).l
0000D308: cmpi.b #0x9,(0x00fff0d1).l
0000D310: ble.w 0x0000d25e
0000D314: subq.b #0x1,(0x00fff0d1).l
0000D31A: bra.w 0x0000d25e
0000D31E: btst.b #0xe,(0x00ffdd22).l
0000D326: beq.w 0x0000d32e
0000D32A: bra.w 0x0000d33a
0000D32E: btst.b #0xc,(0x00ffdd22).l
0000D336: beq.w 0x0000d478
0000D33A: tst.w (0x00fff116).l
0000D340: bmi.w 0x0000d4a0
0000D344: tst.b (0x00fff0b3).l
0000D34A: bmi.w 0x0000d26a
0000D34E: move.l #0xca86,D4
0000D354: moveq #0x3,D1
0000D356: moveq #0x21,D7
0000D358: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0000D35E: tst.b (0x00fff0d0).l
0000D364: beq.w 0x0000d39a
0000D368: cmpi.b #0x2,(0x00fff0d1).l
0000D370: beq.w 0x0000d380
0000D374: cmpi.b #0x3,(0x00fff0d1).l
0000D37C: bne.w 0x0000d39a
0000D380: lea (0xc2186).l,A1 ; data 000c2186
0000D386: movea.l #0xcb12,A2
0000D38C: move.w #0x4000,D1w
0000D390: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D396: bra.w 0x0000d26a
0000D39A: cmpi.b #0x4,(0x00fff0d1).l
0000D3A2: bmi.w 0x0000d3ea
0000D3A6: tst.b (0x00fff0d0).l
0000D3AC: bne.w 0x0000d3ea
0000D3B0: cmpi.b #0x4,(0x00fff0d1).l
0000D3B8: bne.w 0x0000d3ea
0000D3BC: movea.l (0x00fffc04).l,A0
0000D3C2: adda.l #0x9b,A0
0000D3C8: cmpi.b #0x2,(A0)
0000D3CC: bmi.w 0x0000d3ea
0000D3D0: movea.l #0xcb12,A2
0000D3D6: lea (0xc2122).l,A1 ; data 000c2122
0000D3DC: move.w #0x4000,D1w
0000D3E0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D3E6: bra.w 0x0000d26a
0000D3EA: clr.l D0
0000D3EC: clr.l D1
0000D3EE: movea.l (0x00fffc04).l,A0
0000D3F4: adda.l #0x8d,A0
0000D3FA: move.b (A0),D0b
0000D3FC: move.b (0x00fff0b8).l,D1b
0000D402: sub.w D1w,D0w
0000D404: bpl.w 0x0000d422
0000D408: movea.l #0xcb12,A2
0000D40E: lea (0xc214e).l,A1 ; data 000c214e
0000D414: move.w #0x4000,D1w
0000D418: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D41E: bra.w 0x0000d26a
0000D422: movem.l {  A0 D0},-(SP)
0000D426: bsr.w 0x0000d53c ; call 0000D53C FUN_0000d53c
0000D42A: movem.l (SP)+,{ D0 A0}
0000D42E: tst.b (0x00fff0dc).l
0000D434: bne.w 0x0000d0d6
0000D438: move.b D0b,(A0)
0000D43A: move.b (0x00fff0b7).l,D0b
0000D440: movea.l (0x00fffc04).l,A0
0000D446: cmpi.b #0x4,(0x00fff0d1).l
0000D44E: bpl.w 0x0000d45c
0000D452: adda.l #0x77,A0
0000D458: bra.w 0x0000d46c
0000D45C: adda.l #0x80,A0
0000D462: cmpi.w #0x6,D0w
0000D466: bmi.w 0x0000d46c
0000D46A: addq.l #0x1,A0
0000D46C: move.b (0x00fff0b6).l,(0x0,A0,D0w*0x1)
0000D474: bra.w 0x0000d0d6
0000D478: btst.b #0xd,(0x00ffdd22).l
0000D480: beq.w 0x0000d494
0000D484: move.w #0x6000,D1w
0000D488: bsr.w 0x0000d5dc ; call 0000D5DC FUN_0000d5dc
0000D48C: bsr.w 0x0000d4c8 ; call 0000D4C8 FUN_0000d4c8
0000D490: bra.w 0x0000d0d6
0000D494: btst.b #0xf,(0x00ffdd22).l
0000D49C: beq.w 0x0000d26a
0000D4A0: move.w #-0x6e6f,(0x00c00004).l
0000D4A8: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0000D4AE: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0000D4B4: clr.w (0x00fff116).l
0000D4BA: jsr 0x0000334c.l ; call 0000334C FUN_0000334c
0000D4C0: jsr 0x000039f2.l ; call 000039F2 FUN_000039f2
0000D4C6: rts
0000D4C8: move.l #0xc3a4,D4
0000D4CE: move.w #0x2,D1w
0000D4D2: move.w #0x3,D7w
0000D4D6: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0000D4DC: movea.l (0x00fffc04).l,A0
0000D4E2: cmpi.w #0x2,(0x3e,A0)
0000D4E8: bne.w 0x0000d50c
0000D4EC: bclr.b #0x1,(0x10,A0)
0000D4F2: move.l #0xff0100,(0x00fffc04).l
0000D4FC: movea.l (0x00fffc04).l,A0
0000D502: bset.b #0x1,(0x10,A0)
0000D508: bra.w 0x0000d53a
0000D50C: bclr.b #0x1,(0x10,A0)
0000D512: adda.l #0x100,A0
0000D518: cmpi.w #0x3,(0x3e,A0)
0000D51E: bpl.b 0x0000d4f2
0000D520: tst.w (0x3e,A0)
0000D524: beq.b 0x0000d4f2
0000D526: btst.b #0x0,(0x10,A0)
0000D52C: beq.b 0x0000d50c
0000D52E: move.l A0,(0x00fffc04).l
0000D534: bset.b #0x1,(0x10,A0)
0000D53A: rts
0000D53C: lea (0xc217d).l,A1 ; data 000c217d
0000D542: move.w #0x4000,D1w
0000D546: movea.l #0xcb18,A2
0000D54C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D552: clr.b (0x00fff0dc).l
0000D558: lea (0xe387e).l,A1 ; data 000e387e
0000D55E: movea.l #0xcab2,A2
0000D564: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D56A: clr.l D2
0000D56C: bra.w 0x0000d5a2
0000D570: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000D576: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000D57C: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000D582: btst.b #0x8,(0x00ffdd22).l
0000D58A: beq.w 0x0000d594
0000D58E: moveq #-0x1,D2
0000D590: bra.w 0x0000d5a2
0000D594: btst.b #0x9,(0x00ffdd22).l
0000D59C: beq.w 0x0000d5b4
0000D5A0: moveq #0x1,D2
0000D5A2: moveq #0x1,D3
0000D5A4: move.l #0xcaae,D7
0000D5AA: move.w #0x4000,D4w
0000D5AE: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
0000D5B2: bra.b 0x0000d570
0000D5B4: btst.b #0xe,(0x00ffdd22).l
0000D5BC: beq.w 0x0000d5c2
0000D5C0: rts
0000D5C2: btst.b #0xc,(0x00ffdd22).l
0000D5CA: beq.w 0x0000d5d0
0000D5CE: rts
0000D5D0: btst.b #0xd,(0x00ffdd22).l
0000D5D8: beq.b 0x0000d570
0000D5DA: rts
0000D5DC: move.l D1,D2
0000D5DE: clr.l D0
0000D5E0: clr.l D3
0000D5E2: move.b (0x00fff0d0).l,D0b
0000D5E8: move.b (0x00fff0d1).l,D3b
0000D5EE: cmpi.b #0x4,D3b
0000D5F2: bpl.w 0x0000d664
0000D5F6: lea (0xc1a2a).l,A1 ; data 000c1a2a
0000D5FC: mulu.w #0x4,D0
0000D600: add.w D3w,D0w
0000D602: lsl.w #0x2,D0w
0000D604: movea.l (0x0,A1,D0w*0x1),A1 ; data 000c1a2a
0000D608: movea.l #0xc386,A2
0000D60E: move.b (0x00fff0d0).l,D0b
0000D614: mulu.w #0x26,D0
0000D618: mulu.w #0x80,D3
0000D61C: adda.l D0,A2
0000D61E: adda.l D3,A2
0000D620: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D626: clr.l D1
0000D628: clr.l D3
0000D62A: movea.l #0xc3a2,A2
0000D630: movea.l (0x00fffc04).l,A0
0000D636: adda.l #0x77,A0
0000D63C: move.b (0x00fff0d0).l,D0b
0000D642: move.b (0x00fff0d1).l,D3b
0000D648: mulu.w #0x4,D0
0000D64C: add.w D3w,D0w
0000D64E: cmpi.w #0x6,D0w
0000D652: beq.w 0x0000d6ea
0000D656: cmpi.w #0x7,D0w
0000D65A: bne.w 0x0000d6ca
0000D65E: addq.l #0x1,A0
0000D660: bra.w 0x0000d6ca
0000D664: subq.w #0x4,D3w
0000D666: lea (0xc1a4a).l,A1 ; data 000c1a4a
0000D66C: mulu.w #0x6,D0
0000D670: add.w D3w,D0w
0000D672: lsl.w #0x2,D0w
0000D674: movea.l (0x0,A1,D0w*0x1),A1 ; data 000c1a4a
0000D678: movea.l #0xc686,A2
0000D67E: move.b (0x00fff0d0).l,D0b
0000D684: mulu.w #0x26,D0
0000D688: mulu.w #0x80,D3
0000D68C: adda.l D0,A2
0000D68E: adda.l D3,A2
0000D690: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D696: clr.l D1
0000D698: clr.l D3
0000D69A: movea.l #0xc6a2,A2
0000D6A0: movea.l (0x00fffc04).l,A0
0000D6A6: adda.l #0x80,A0
0000D6AC: move.b (0x00fff0d0).l,D0b
0000D6B2: move.b (0x00fff0d1).l,D3b
0000D6B8: subq.w #0x4,D3w
0000D6BA: mulu.w #0x6,D0
0000D6BE: add.w D3w,D0w
0000D6C0: cmpi.w #0x6,D0w
0000D6C4: bmi.w 0x0000d6ca
0000D6C8: addq.l #0x1,A0
0000D6CA: move.b (0x0,A0,D0w*0x1),D1b
0000D6CE: move.b (0x00fff0d0).l,D0b
0000D6D4: mulu.w #0x24,D0
0000D6D8: mulu.w #0x80,D3
0000D6DC: adda.l D0,A2
0000D6DE: adda.l D3,A2
0000D6E0: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D6E6: bra.w 0x0000d71e
0000D6EA: movea.l (0x00fffc04).l,A0
0000D6F0: adda.l #0x7e,A0
0000D6F6: move.b (A0)+,D1b
0000D6F8: movea.l #0xc4c4,A2
0000D6FE: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D704: movea.l (0x00fffc04).l,A0
0000D70A: adda.l #0x8e,A0
0000D710: move.b (A0)+,D1b
0000D712: movea.l #0xc4c8,A2
0000D718: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D71E: clr.l D1
0000D720: movea.l (0x00fffc04).l,A0
0000D726: adda.l #0x90,A0
0000D72C: move.w (A0),D5w
0000D72E: moveq #0x3,D7
0000D730: moveq #0x5,D6
0000D732: btst.l D6,D5
0000D734: beq.w 0x0000d796
0000D738: lea (0xc259d).l,A1 ; data 000c259d
0000D73E: move.w #0x6000,D1w
0000D742: movea.l #0xc424,A2
0000D748: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D74E: movea.l #0xc4a4,A2
0000D754: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D75A: movea.l (0x00fffc04).l,A0
0000D760: adda.l #0x77,A0
0000D766: clr.l D1
0000D768: move.b (0x1,A0),D1b
0000D76C: subq.w #0x4,D6w
0000D76E: add.w D6w,D1w
0000D770: movea.l #0xc428,A2
0000D776: move.w #0x6000,D2w
0000D77A: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D780: move.b (0x2,A0),D1b
0000D784: add.w D6w,D1w
0000D786: movea.l #0xc4a8,A2
0000D78C: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D792: bra.w 0x0000d79c
0000D796: addq.w #0x1,D6w
0000D798: dbf D7w,0x0000d732
0000D79C: moveq #0x2,D7
0000D79E: moveq #0x9,D6
0000D7A0: btst.l D6,D5
0000D7A2: beq.w 0x0000d7e2
0000D7A6: lea (0xc259d).l,A1 ; data 000c259d
0000D7AC: move.w #0x6000,D1w
0000D7B0: movea.l #0xc3a4,A2
0000D7B6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D7BC: movea.l (0x00fffc04).l,A0
0000D7C2: adda.l #0x77,A0
0000D7C8: clr.l D1
0000D7CA: move.b (A0),D1b
0000D7CC: subq.w #0x8,D6w
0000D7CE: add.w D6w,D1w
0000D7D0: movea.l #0xc3a8,A2
0000D7D6: move.w #0x6000,D2w
0000D7DA: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D7E0: rts
0000D7E2: addq.w #0x1,D6w
0000D7E4: dbf D7w,0x0000d7a0
0000D7E8: rts
0000D7EA: move.w (0x90,A0),D5w
0000D7EE: moveq #0x2,D7
0000D7F0: moveq #0xc,D2
0000D7F2: btst.l D2,D5
0000D7F4: beq.w 0x0000d804
0000D7F8: subi.w #0xb,D2w
0000D7FC: mulu.w #0x4,D2
0000D800: bra.w 0x0000d80c
0000D804: addq.w #0x1,D2w
0000D806: dbf D7w,0x0000d7f2
0000D80A: clr.l D2
0000D80C: bsr.w 0x0000d81c ; call 0000D81C average_entity_delay_attributes
0000D810: add.w D2w,D1w
0000D812: subq.w #0x1,D1w
0000D814: bpl.w 0x0000d81a
0000D818: clr.l D1
0000D81A: rts
0000D81C: clr.l D1
0000D81E: move.b (0x78,A0),D1b
0000D822: add.b (0x7b,A0),D1b
0000D826: lsr.w #0x1,D1w
0000D828: rts
0000D82A: clr.b (0x00fff0b3).l
0000D830: move.l #0xca86,D4
0000D836: moveq #0x4,D1
0000D838: moveq #0x21,D7
0000D83A: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0000D840: clr.l D1
0000D842: clr.l D0
0000D844: clr.l D3
0000D846: move.b (0x00fff0d0).l,D0b
0000D84C: move.b (0x00fff0d1).l,D3b
0000D852: cmpi.b #0x4,D3b
0000D856: bpl.w 0x0000d8c0
0000D85A: lea (0xc1b70).l,A1 ; data 000c1b70
0000D860: mulu.w #0x4,D0
0000D864: add.w D3w,D0w
0000D866: move.l D0,D5
0000D868: lsl.w #0x2,D5w
0000D86A: adda.l D5,A1
0000D86C: tst.w (0x00fff116).l
0000D872: bmi.w 0x0000da00
0000D876: cmpi.w #0x6,D0w
0000D87A: bpl.w 0x0000da00
0000D87E: movea.l (0x00fffc04).l,A0
0000D884: adda.l #0x77,A0
0000D88A: move.b (0x0,A0,D0w*0x1),(0x00fff0b6).l
0000D892: move.b (0x0,A0,D0w*0x1),D1b
0000D896: addq.b #0x1,D1b
0000D898: bsr.w 0x0000da4a ; call 0000DA4A FUN_0000da4a
0000D89C: clr.l D2
0000D89E: movea.l (0x00fffc04).l,A0
0000D8A4: move.b (0x9a,A0),D2b
0000D8A8: mulu.w #0x6,D2
0000D8AC: lea (0xc22a6).l,A0 ; data 000c22a6
0000D8B2: adda.l D2,A0
0000D8B4: cmp.b (0x0,A0,D0w*0x1),D1b
0000D8B8: bgt.w 0x0000d9de
0000D8BC: bra.w 0x0000d9a4
0000D8C0: lea (0xc1b90).l,A1 ; data 000c1b90
0000D8C6: subq.w #0x4,D3w
0000D8C8: mulu.w #0x6,D0
0000D8CC: add.w D3w,D0w
0000D8CE: move.l D0,D5
0000D8D0: lsl.w #0x2,D5w
0000D8D2: adda.l D5,A1
0000D8D4: tst.w (0x00fff116).l
0000D8DA: bmi.w 0x0000da00
0000D8DE: tst.w D0w
0000D8E0: bne.w 0x0000d8f8
0000D8E4: movea.l (0x00fffc04).l,A0
0000D8EA: adda.l #0x9b,A0
0000D8F0: cmpi.b #0x2,(A0)
0000D8F4: bpl.w 0x0000da00
0000D8F8: bsr.w 0x0000da4a ; call 0000DA4A FUN_0000da4a
0000D8FC: movea.l (0x00fffc04).l,A0
0000D902: adda.l #0x80,A0
0000D908: cmpi.w #0x6,D0w
0000D90C: bmi.w 0x0000d912
0000D910: addq.w #0x1,A0
0000D912: move.b (0x0,A0,D0w*0x1),(0x00fff0b6).l
0000D91A: move.b (0x0,A0,D0w*0x1),D1b
0000D91E: addq.b #0x1,D1b
0000D920: cmpi.b #0xd,D1b
0000D924: bpl.w 0x0000d9de
0000D928: tst.b (0x00fff0d0).l
0000D92E: bne.w 0x0000d9a0
0000D932: cmpi.w #0x2,D3w
0000D936: bmi.w 0x0000d9a0
0000D93A: cmpi.w #0x5,D3w
0000D93E: beq.w 0x0000d9a0
0000D942: cmpi.w #0x6,D3w
0000D946: bgt.w 0x0000d9a0
0000D94A: beq.w 0x0000d968
0000D94E: tst.b (0x00fff0b6).l
0000D954: bne.w 0x0000d988
0000D958: movea.l (0x00fffc04).l,A0
0000D95E: adda.l #0x81,A0
0000D964: bra.w 0x0000d97e
0000D968: tst.b (0x00fff0b6).l
0000D96E: bne.w 0x0000d988
0000D972: movea.l (0x00fffc04).l,A0
0000D978: adda.l #0x85,A0
0000D97E: move.b (A0),D1b
0000D980: move.b D1b,(0x00fff0b6).l
0000D986: addq.b #0x1,D1b
0000D988: mulu.w #0xf,D1
0000D98C: divu.w #0xa,D1
0000D990: move.l D1,D2
0000D992: swap D2
0000D994: tst.w D2w
0000D996: beq.w 0x0000d9a4
0000D99A: addq.w #0x1,D1w
0000D99C: bra.w 0x0000d9a4
0000D9A0: mulu.w #0x2,D1
0000D9A4: addq.b #0x1,(0x00fff0b6).l
0000D9AA: move.b D0b,(0x00fff0b7).l
0000D9B0: move.b D1b,(0x00fff0b8).l
0000D9B6: ext.l D1
0000D9B8: move.w #0x4000,D2w
0000D9BC: movea.l #0xccbe,A2
0000D9C2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D9C8: move.b (0x00fff0b6).l,D1b
0000D9CE: movea.l #0xcca8,A2
0000D9D4: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000D9DA: bra.w 0x0000da00
0000D9DE: move.b #-0x1,(0x00fff0b3).l
0000D9E6: move.l A1,-(SP) ; data Stack[-0x4]
0000D9E8: lea (0xc1b51).l,A1 ; data 000c1b51
0000D9EE: movea.l #0xcc8c,A2
0000D9F4: move.w #0x4000,D1w
0000D9F8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000D9FE: movea.l (SP)+,A1
0000DA00: movea.l (A1),A1 ; data 000c1b70
0000DA02: movea.l #0xca86,A2
0000DA08: move.w #0x4000,D1w
0000DA0C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000DA12: lea (0xc1320).l,A1 ; data 000c1320
0000DA18: move.l #0xca1a,D4
0000DA1E: moveq #0x2,D7
0000DA20: moveq #0x0,D1
0000DA22: move.w #0x4000,D2w
0000DA26: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0000DA2C: movea.l (0x00fffc04).l,A0
0000DA32: adda.l #0x8d,A0
0000DA38: clr.l D1
0000DA3A: move.b (A0),D1b
0000DA3C: movea.l #0xca1e,A2
0000DA42: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000DA48: rts
0000DA4A: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0000DA4E: lea (0xc1b3b).l,A1 ; data 000c1b3b
0000DA54: movea.l #0xcc90,A2
0000DA5A: move.w #0x4000,D1w
0000DA5E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000DA64: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0000DA68: rts
0000DFF0: bsr.w 0x0000c436 ; call 0000C436 FUN_0000c436
0000DFF4: rts
0000E00A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000E010: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000E016: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000E01C: bsr.w 0x0000e6f4 ; call 0000E6F4 FUN_0000e6f4
0000E020: move.w (0x00ffdd22).l,D0w
0000E026: andi.w #-0x100,D0w
0000E02A: beq.b 0x0000e00a
0000E02C: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000E032: rts
0000E050: lea (0xc176e).l,A1 ; data 000c176e
0000E056: movea.l #0xc58c,A2
0000E05C: move.w #0x6000,D1w
0000E060: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000E066: bra.b 0x0000e00a ; call 0000E00A wait_for_controller_event
0000E3E0: cmpi.w #0x7,D7w
0000E3E4: bmi.w 0x0000e3f8
0000E3E8: move.w #0x6,D7w
0000E3EC: bra.w 0x0000e3f8
0000E3F8: movea.l (A3)+,A1
0000E3FA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000E400: adda.l #0x100,A2
0000E406: dbf D7w,0x0000e3f8
0000E40A: rts
0000E40C: btst.b #0x0,(0x00ff018f).l
0000E414: bne.w 0x0000e426
0000E418: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000E41C: lea (0xc21b2).l,A1 ; data 000c21b2
0000E422: bra.w 0x0000e45a
0000E426: tst.b (0x00fffbbe).l
0000E42C: bne.w 0x0000e43e
0000E430: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000E434: lea (0xc2202).l,A1 ; data 000c2202
0000E43A: bra.w 0x0000e45a
0000E43E: movea.l (0x00fffc04).l,A0
0000E444: move.w (0x90,A0),D0w
0000E448: btst.l #0x0,D0
0000E44C: bne.w 0x0000e47a
0000E450: bsr.w 0x0000c3e6 ; call 0000C3E6 FUN_0000c3e6
0000E454: lea (0xc2252).l,A1 ; data 000c2252
0000E45A: movea.l #0xc410,A2
0000E460: move.w #0x6000,D1w
0000E464: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000E46A: move.w #0x1b,D0w
0000E46E: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000E474: bsr.w 0x0000e00a ; call 0000E00A wait_for_controller_event
0000E478: rts
0000E47A: move.w #0x1a,D0w
0000E47E: jsr 0x000530de.l ; call 000530DE FUN_000530de
0000E484: move.w #0x14,D5w
0000E488: jsr 0x00000c24.l ; call 00000C24 FUN_00000c24
0000E48E: jsr 0x000181f8.l ; call 000181F8 FUN_000181f8
0000E494: tst.b (0x00fff0b1).l
0000E49A: bne.w 0x0000e4c0
0000E49E: jsr 0x000173f2.l ; call 000173F2 FUN_000173f2
0000E4A4: move.b #-0x1,(0x00fff0dc).l
0000E4AC: movea.l (0x00fffc04).l,A0
0000E4B2: tst.b (0x4c,A0)
0000E4B6: bne.w 0x0000e4c0
0000E4BA: jsr 0x0000b330.l ; call 0000B330 enter_entity_behavior_transition
0000E4C0: clr.b (0x00fff0b1).l
0000E4C6: rts
0000E600: beq.w 0x0000e60c
0000E604: bset.l D0,D1
0000E606: move.l D1,(0x00fffc2c).l
0000E60C: movea.l (0x2,A1),A0
0000E610: bra.w 0x0000e618
0000E618: movea.l (0xa,A1),A5
0000E61C: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0000E622: bra.w 0x0000e65a
0000E65A: move.l (SP)+,D0
0000E65C: move.l (SP)+,D0
0000E65E: rts
0000E660: move.w #0x2,D1w
0000E664: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E66A: bne.w 0x0000e6b8
0000E66E: move.w #0x8,D1w
0000E672: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E678: lea (0xc13e8).l,A1 ; data 000c13e8
0000E67E: lsl.w #0x1,D0w
0000E680: move.w (0x0,A1,D0w*0x1),D4w ; data 000c13e8
0000E684: move.w #0x3,D1w
0000E688: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E68E: cmpi.w #0x1,D0w
0000E692: bmi.w 0x0000e6ae
0000E696: beq.w 0x0000e6a4
0000E69A: lea (0xce126).l,A1 ; data 000ce126
0000E6A0: bra.w 0x0000e6e4
0000E6A4: lea (0xce12e).l,A1 ; data 000ce12e
0000E6AA: bra.w 0x0000e6e4
0000E6AE: lea (0xce136).l,A1 ; data 000ce136
0000E6B4: bra.w 0x0000e6e4
0000E6B8: move.w #0x4,D1w
0000E6BC: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E6C2: lea (0xc13f8).l,A1 ; data 000c13f8
0000E6C8: lsl.w #0x1,D0w
0000E6CA: move.w (0x0,A1,D0w*0x1),D4w ; data 000c13f8
0000E6CE: lea (0xce13e).l,A1 ; data 000ce13e
0000E6D4: move.w #0x4,D1w
0000E6D8: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E6DE: mulu.w #0x4,D0
0000E6E2: adda.l D0,A1
0000E6E4: move.w #0x0,D2w
0000E6E8: moveq #0x1,D7
0000E6EA: moveq #0x0,D1
0000E6EC: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000E6F2: rts
0000E6F4: move.w #0x2,D1w
0000E6F8: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E6FE: bne.w 0x0000e76e
0000E702: move.w #0x2,D1w
0000E706: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E70C: bne.w 0x0000e71c
0000E710: lea (0xc13e2).l,A1 ; data 000c13e2
0000E716: moveq #0x4,D5
0000E718: bra.w 0x0000e724
0000E71C: lea (0xc13dc).l,A1 ; data 000c13dc
0000E722: clr.l D5
0000E724: move.w #0x3,D1w
0000E728: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E72E: lsl.w #0x1,D0w
0000E730: move.w (0x0,A1,D0w*0x1),D4w
0000E734: move.w #0x3,D1w
0000E738: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E73E: cmpi.w #0x1,D0w
0000E742: bmi.w 0x0000e75e
0000E746: beq.w 0x0000e754
0000E74A: lea (0xce126).l,A1 ; data 000ce126
0000E750: bra.w 0x0000e764
0000E754: lea (0xce12e).l,A1 ; data 000ce12e
0000E75A: bra.w 0x0000e764
0000E75E: lea (0xce136).l,A1 ; data 000ce136
0000E764: moveq #0x1,D7
0000E766: moveq #0x0,D1
0000E768: adda.l D5,A1
0000E76A: bra.w 0x0000e7bc
0000E76E: move.w #0x2,D1w
0000E772: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E778: bne.w 0x0000e788
0000E77C: lea (0xc13d0).l,A1 ; data 000c13d0
0000E782: clr.l D5
0000E784: bra.w 0x0000e790
0000E788: lea (0xc13d6).l,A1 ; data 000c13d6
0000E78E: moveq #0x2,D5
0000E790: move.w #0x3,D1w
0000E794: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E79A: lsl.w #0x1,D0w
0000E79C: move.w (0x0,A1,D0w*0x1),D4w
0000E7A0: lea (0xce13e).l,A1 ; data 000ce13e
0000E7A6: move.w #0x4,D1w
0000E7AA: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0000E7B0: mulu.w #0x4,D0
0000E7B4: adda.l D0,A1
0000E7B6: adda.l D5,A1
0000E7B8: moveq #0x0,D7
0000E7BA: moveq #0x0,D1
0000E7BC: move.w #0x0,D2w
0000E7C0: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000E7C6: rts
0000E7C8: clr.w (0x00ffdf68).l
0000E7CE: clr.w (0x00ffdfe2).l
0000E7D4: lsr.w #0x4,D5w
0000E7D6: lsr.w #0x4,D6w
0000E7D8: clr.w D0w
0000E7DA: cmp.w (0x00ffddd2).l,D5w
0000E7E0: bcs.w 0x0000e7ec
0000E7E4: move.w #0x7f,D0w
0000E7E8: bra.w 0x0000e81e
0000E7EC: cmp.w (0x00ffddd6).l,D5w
0000E7F2: bcc.w 0x0000e7fe
0000E7F6: move.w #0x7f,D0w
0000E7FA: bra.w 0x0000e81e
0000E7FE: cmp.w (0x00ffddd4).l,D6w
0000E804: bcs.w 0x0000e810
0000E808: move.w #0x7f,D0w
0000E80C: bra.w 0x0000e81e
0000E810: cmp.w (0x00ffddd8).l,D6w
0000E816: bcc.w 0x0000e838
0000E81A: move.w #0x7f,D0w
0000E81E: move.w #-0x1,(0x00ffdf68).l
0000E826: move.w #0x7f,(0x00ffef10).l
0000E82E: move.w #0x7f,(0x00ffef12).l
0000E836: rts
0000E838: bsr.w 0x0000e8a0 ; call 0000E8A0 sample_terrain_collision_tiles
0000E83C: ext.w D0w
0000E83E: move.w D0w,(0x00ffdf64).l
0000E844: beq.b 0x0000e836
0000E846: cmpi.w #0x7f,D0w
0000E84A: beq.b 0x0000e836
0000E84C: tst.w D0w
0000E84E: bmi.b 0x0000e836
0000E850: btst.l #0x6,D0
0000E854: beq.w 0x0000e874
0000E858: move.w #-0x1,(0x00ffdfe2).l
0000E860: cmpi.w #0x7e,D0w
0000E864: beq.w 0x0000e88c
0000E868: btst.l #0x5,D0
0000E86C: beq.w 0x0000e88c
0000E870: bra.w 0x0000e87c
0000E874: btst.l #0x5,D0
0000E878: beq.w 0x0000e892
0000E87C: bsr.w 0x0000ea36 ; call 0000EA36 FUN_0000ea36
0000E880: bcc.w 0x0000e89c
0000E884: nop
0000E886: nop
0000E888: nop
0000E88A: nop
0000E88C: move.w #0x7f,D0w
0000E890: rts
0000E892: bsr.w 0x0000e9ee ; call 0000E9EE contains_terrain_object_bounds
0000E896: bcc.w 0x0000e89c
0000E89A: bra.b 0x0000e88c
0000E89C: clr.w D0w
0000E89E: rts
0000E8A0: clr.w (0x00ffdd28).l
0000E8A6: lea (0xff1700).l,A2 ; data 00ff1700
0000E8AC: btst.b #0x2,(0x10,A0)
0000E8B2: bne.w 0x0000e8d6
0000E8B6: move.w D5w,D1w
0000E8B8: move.w D6w,D2w
0000E8BA: lsl.w #0x1,D2w
0000E8BC: andi.w #-0x40,D2w
0000E8C0: lsr.w #0x5,D1w
0000E8C2: add.w D1w,D2w
0000E8C4: move.b (0x0,A2,D2w*0x1),D0b ; data 00ff1700
0000E8C8: move.w D2w,(0x00ffdfe4).l
0000E8CE: move.b D0b,(0x00ffef10).l
0000E8D4: rts
0000E8D6: move.w D5w,D1w
0000E8D8: move.w D6w,D2w
0000E8DA: subq.w #0x7,D2w
0000E8DC: subi.w #0xc,D1w
0000E8E0: lsl.w #0x1,D2w
0000E8E2: andi.w #-0x40,D2w
0000E8E6: lsr.w #0x5,D1w
0000E8E8: add.w D1w,D2w
0000E8EA: move.b (0x0,A2,D2w*0x1),D7b ; data 00ff1700
0000E8EE: move.w D2w,(0x00ffdfe4).l
0000E8F4: move.b D7b,(0x00ffef10).l
0000E8FA: cmpi.b #0x7f,D7b
0000E8FE: bne.w 0x0000e908
0000E902: move.b D7b,(0x00ffdd28).l
0000E908: move.w D5w,D1w
0000E90A: move.w D6w,D2w
0000E90C: subq.w #0x7,D2w
0000E90E: addi.w #0xc,D1w
0000E912: lsl.w #0x1,D2w
0000E914: andi.w #-0x40,D2w
0000E918: lsr.w #0x5,D1w
0000E91A: add.w D1w,D2w
0000E91C: move.b (0x0,A2,D2w*0x1),D0b ; data 00ff1700
0000E920: move.b D0b,(0x00ffef11).l
0000E926: cmpi.b #0x7f,D0b
0000E92A: bne.w 0x0000e934
0000E92E: move.b D0b,(0x00ffdd28).l
0000E934: cmp.b D7b,D0b
0000E936: bcs.w 0x0000e942
0000E93A: move.b D0b,D7b
0000E93C: move.w D2w,(0x00ffdfe4).l
0000E942: move.w D5w,D1w
0000E944: move.w D6w,D2w
0000E946: addq.w #0x7,D2w
0000E948: subi.w #0xc,D1w
0000E94C: lsl.w #0x1,D2w
0000E94E: andi.w #-0x40,D2w
0000E952: lsr.w #0x5,D1w
0000E954: add.w D1w,D2w
0000E956: move.b (0x0,A2,D2w*0x1),D0b ; data 00ff1700
0000E95A: move.b D0b,(0x00ffef12).l
0000E960: cmpi.b #0x7f,D0b
0000E964: bne.w 0x0000e96e
0000E968: move.b D0b,(0x00ffdd28).l
0000E96E: cmp.b D7b,D0b
0000E970: bcs.w 0x0000e97c
0000E974: move.b D0b,D7b
0000E976: move.w D2w,(0x00ffdfe4).l
0000E97C: move.w D5w,D1w
0000E97E: move.w D6w,D2w
0000E980: addq.w #0x7,D2w
0000E982: addi.w #0xc,D1w
0000E986: lsl.w #0x1,D2w
0000E988: andi.w #-0x40,D2w
0000E98C: lsr.w #0x5,D1w
0000E98E: add.w D1w,D2w
0000E990: move.b (0x0,A2,D2w*0x1),D0b ; data 00ff1700
0000E994: move.b D0b,(0x00ffef13).l
0000E99A: cmpi.b #0x7f,D0b
0000E99E: bne.w 0x0000e9a8
0000E9A2: move.b D0b,(0x00ffdd28).l
0000E9A8: cmp.b D7b,D0b
0000E9AA: bcs.w 0x0000e9b8
0000E9AE: move.w D2w,(0x00ffdfe4).l
0000E9B4: bra.w 0x0000e9ba
0000E9B8: move.b D7b,D0b
0000E9BA: tst.b (0x00ffdd28).l
0000E9C0: beq.w 0x0000e9ca
0000E9C4: move.b (0x00ffdd28).l,D0b
0000E9CA: rts
0000E9EE: movea.l (0x00ffddec).l,A2
0000E9F4: mulu.w #0xa,D0
0000E9F8: adda.w D0w,A2
0000E9FA: move.l (A2)+,D2
0000E9FC: move.w D2w,D3w
0000E9FE: swap D2
0000EA00: subi.w #0xc,D2w
0000EA04: addi.w #0xc,D3w
0000EA08: cmp.w D2w,D5w
0000EA0A: blt.w 0x0000ea30
0000EA0E: cmp.w D3w,D5w
0000EA10: bgt.w 0x0000ea30
0000EA14: move.l (A2)+,D2
0000EA16: move.w D2w,D3w
0000EA18: swap D2
0000EA1A: subq.w #0x7,D2w
0000EA1C: addq.w #0x7,D3w
0000EA1E: cmp.w D2w,D6w
0000EA20: blt.w 0x0000ea30
0000EA24: cmp.w D3w,D6w
0000EA26: bgt.w 0x0000ea30
0000EA2A: ori #0x1,CCR
0000EA2E: rts
0000EA30: andi #0x1e,CCR
0000EA34: rts
0000EA36: move.b (0x00ffef10).l,D0b
0000EA3C: btst.l #0x5,D0
0000EA40: beq.w 0x0000ea5a
0000EA44: movem.w {  D6w D5w},-(SP)
0000EA48: subq.w #0x7,D6w
0000EA4A: subi.w #0xc,D5w
0000EA4E: bsr.w 0x0000ead2 ; call 0000EAD2 evaluate_terrain_direction_boundary
0000EA52: movem.w (SP)+,{  D5 D6}
0000EA56: bcs.w 0x0000eacc
0000EA5A: move.b (0x00ffef11).l,D0b
0000EA60: btst.l #0x5,D0
0000EA64: beq.w 0x0000ea7e
0000EA68: movem.w {  D6w D5w},-(SP)
0000EA6C: subq.w #0x7,D6w
0000EA6E: addi.w #0xc,D5w
0000EA72: bsr.w 0x0000ead2 ; call 0000EAD2 evaluate_terrain_direction_boundary
0000EA76: movem.w (SP)+,{  D5 D6}
0000EA7A: bcs.w 0x0000eacc
0000EA7E: move.b (0x00ffef12).l,D0b
0000EA84: btst.l #0x5,D0
0000EA88: beq.w 0x0000eaa2
0000EA8C: movem.w {  D6w D5w},-(SP)
0000EA90: addq.w #0x7,D6w
0000EA92: subi.w #0xc,D5w
0000EA96: bsr.w 0x0000ead2 ; call 0000EAD2 evaluate_terrain_direction_boundary
0000EA9A: movem.w (SP)+,{  D5 D6}
0000EA9E: bcs.w 0x0000eacc
0000EAA2: move.b (0x00ffef13).l,D0b
0000EAA8: btst.l #0x5,D0
0000EAAC: beq.w 0x0000eac6
0000EAB0: movem.w {  D6w D5w},-(SP)
0000EAB4: addq.w #0x7,D6w
0000EAB6: addi.w #0xc,D5w
0000EABA: bsr.w 0x0000ead2 ; call 0000EAD2 evaluate_terrain_direction_boundary
0000EABE: movem.w (SP)+,{  D5 D6}
0000EAC2: bcs.w 0x0000eacc
0000EAC6: andi #0x1e,CCR
0000EACA: rts
0000EACC: ori #0x1,CCR
0000EAD0: rts
0000EAD2: ext.w D0w
0000EAD4: move.w (0x00ffdfe4).l,D2w
0000EADA: bsr.w 0x0000edfa ; call 0000EDFA decode_terrain_tile_window
0000EADE: andi.w #0x38,D0w
0000EAE2: cmpi.w #0x20,D0w
0000EAE6: beq.w 0x0000eb22
0000EAEA: cmpi.w #0x28,D0w
0000EAEE: beq.w 0x0000eb0e
0000EAF2: cmpi.w #0x38,D0w
0000EAF6: beq.w 0x0000eb32
0000EAFA: exg D1,D2
0000EAFC: sub.w D5w,D2w
0000EAFE: neg.w D2w
0000EB00: sub.w D6w,D1w
0000EB02: neg.w D1w
0000EB04: cmp.w D1w,D2w
0000EB06: bcs.w 0x0000eb42
0000EB0A: bra.w 0x0000eb48
0000EB0E: exg D1,D2
0000EB10: sub.w D5w,D2w
0000EB12: neg.w D2w
0000EB14: sub.w D6w,D1w
0000EB16: neg.w D1w
0000EB18: cmp.w D1w,D2w
0000EB1A: bcc.w 0x0000eb42
0000EB1E: bra.w 0x0000eb48
0000EB22: sub.w D5w,D1w
0000EB24: neg.w D1w
0000EB26: sub.w D6w,D3w
0000EB28: cmp.w D3w,D1w
0000EB2A: bcs.w 0x0000eb42
0000EB2E: bra.w 0x0000eb48
0000EB32: sub.w D5w,D1w
0000EB34: neg.w D1w
0000EB36: sub.w D6w,D3w
0000EB38: cmp.w D3w,D1w
0000EB3A: bcc.w 0x0000eb42
0000EB3E: bra.w 0x0000eb48
0000EB42: ori #0x1,CCR
0000EB46: rts
0000EB48: andi #0x1e,CCR
0000EB4C: rts
0000EB92: btst.b #0x6,(0x10,A0)
0000EB98: beq.w 0x0000eb9e
0000EB9C: rts
0000EB9E: cmpa.l #0xff0a00,A0
0000EBA4: bne.w 0x0000ebaa
0000EBA8: rts
0000EBAA: move.w (A0),D5w
0000EBAC: move.w (0x4,A0),D6w
0000EBB0: movea.l A0,A1
0000EBB2: adda.w #0x100,A1
0000EBB6: btst.b #0x0,(0x10,A1)
0000EBBC: beq.w 0x0000ecbc
0000EBC0: btst.b #0x6,(0x10,A1)
0000EBC6: bne.w 0x0000ecbc
0000EBCA: cmpa.l (0xa2,A1),A0
0000EBCE: beq.w 0x0000ebea
0000EBD2: cmpa.l (0xa2,A0),A1
0000EBD6: beq.w 0x0000ebea
0000EBDA: cmpa.l (0xac,A1),A0
0000EBDE: beq.w 0x0000ebea
0000EBE2: cmpa.l (0xac,A0),A1
0000EBE6: bne.w 0x0000ec1c
0000EBEA: move.w (A1),D0w
0000EBEC: subi.w #0x100,D0w
0000EBF0: cmp.w D0w,D5w
0000EBF2: bcs.w 0x0000ecbc
0000EBF6: addi.w #0x200,D0w
0000EBFA: cmp.w D0w,D5w
0000EBFC: bcc.w 0x0000ecbc
0000EC00: move.w (0x4,A1),D0w
0000EC04: subi.w #0x100,D0w
0000EC08: cmp.w D0w,D6w
0000EC0A: bcs.w 0x0000ecbc
0000EC0E: addi.w #0x200,D0w
0000EC12: cmp.w D0w,D6w
0000EC14: bcc.w 0x0000ecbc
0000EC18: bra.w 0x0000ec4a
0000EC1C: move.w (A1),D0w
0000EC1E: subi.w #0x40,D0w
0000EC22: cmp.w D0w,D5w
0000EC24: bcs.w 0x0000ecbc
0000EC28: addi.w #0x80,D0w
0000EC2C: cmp.w D0w,D5w
0000EC2E: bcc.w 0x0000ecbc
0000EC32: move.w (0x4,A1),D0w
0000EC36: subi.w #0x40,D0w
0000EC3A: cmp.w D0w,D6w
0000EC3C: bcs.w 0x0000ecbc
0000EC40: addi.w #0x80,D0w
0000EC44: cmp.w D0w,D6w
0000EC46: bcc.w 0x0000ecbc
0000EC4A: bset.b #0x1,(0x11,A0)
0000EC50: bset.b #0x1,(0x11,A1)
0000EC56: movem.w {  D7w D6w D5w},-(SP)
0000EC5A: move.w (A0),D5w
0000EC5C: move.w (0x4,A0),D4w
0000EC60: move.w (A1),D3w
0000EC62: move.w (0x4,A1),D2w
0000EC66: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
0000EC6C: movem.w (SP)+,{  D5 D6 D7}
0000EC70: addi.w #0x4000,D2w
0000EC74: move.w D2w,(0x92,A1)
0000EC78: addi.w #-0x8000,D2w
0000EC7C: move.w D2w,(0x92,A0)
0000EC80: move.w (0x38,A0),(0x94,A1)
0000EC86: move.w (0x38,A1),(0x94,A0)
0000EC8C: bsr.w 0x0000eccc ; call 0000ECCC FUN_0000eccc
0000EC90: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0000EC96: move.w D0w,D1w
0000EC98: andi.w #0x7000,D1w
0000EC9C: bne.w 0x0000ecbc
0000ECA0: addi.w #0x4000,(0x92,A1)
0000ECA6: addi.w #-0x4000,(0x92,A0)
0000ECAC: move.w (0x94,A1),D0w
0000ECB0: add.w D0w,(0x94,A1)
0000ECB4: move.w (0x94,A0),D0w
0000ECB8: add.w D0w,(0x94,A0)
0000ECBC: adda.w #0x100,A1
0000ECC0: cmpa.l #0xff0b00,A1
0000ECC6: bcs.w 0x0000ebb6
0000ECCA: rts
0000ECCC: btst.b #0x4,(0x9e,A1)
0000ECD2: beq.w 0x0000eddc
0000ECD6: btst.b #0x1,(0x10,A0)
0000ECDC: beq.w 0x0000ed3c
0000ECE0: move.l A1,(0x00ffe128).l
0000ECE6: tst.b (0x00ffe176).l
0000ECEC: bne.w 0x0000ed3c
0000ECF0: move.l #0x21174,(0x00ffdd2c).l
0000ECFA: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0000ED00: move.w (0x00ffddc4).l,D0w
0000ED06: subi.w #0xa,D0w
0000ED0A: bne.w 0x0000ed2e
0000ED0E: move.b #0x4,(0x00fff0ae).l
0000ED16: cmpi.b #0x6,(0x00fffbf9).l
0000ED1E: bne.w 0x0000ed3c
0000ED22: move.b #0x2,(0x00fff0ae).l
0000ED2A: bra.w 0x0000ed3c
0000ED2E: lea (0xeefe).l,A2 ; data 0000eefe
0000ED34: move.b (0x0,A2,D0w*0x1),(0x00fff0ae).l
0000ED3C: btst.b #0x3,(0x9e,A1)
0000ED42: beq.w 0x0000edb4
0000ED46: btst.b #0x7,(0xb0,A1)
0000ED4C: bne.w 0x0000edda
0000ED50: btst.b #0x1,(0x10,A0)
0000ED56: beq.w 0x0000edda
0000ED5A: bset.b #0x7,(0xb0,A1)
0000ED60: move.l (0x00ffe110).l,(0x00fffc3c).l
0000ED6A: move.l #0x1f146,(0x00ffdd2c).l
0000ED74: tst.b (0x00ffe180).l
0000ED7A: beq.w 0x0000ed92
0000ED7E: clr.b (0x00ffe180).l
0000ED84: move.l #0x53c90,(0x00ffdd2c).l
0000ED8E: bra.w 0x0000edac
0000ED92: tst.b (0x00ffe182).l
0000ED98: beq.w 0x0000edac
0000ED9C: clr.b (0x00ffe182).l
0000EDA2: move.l #0x54402,(0x00ffdd2c).l
0000EDAC: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
0000EDB2: rts
0000EDB4: btst.b #0x3,(0x9e,A0)
0000EDBA: beq.w 0x0000edda
0000EDBE: btst.b #0x7,(0xb0,A0)
0000EDC4: bne.w 0x0000edda
0000EDC8: btst.b #0x1,(0x10,A1)
0000EDCE: beq.w 0x0000edda
0000EDD2: bset.b #0x7,(0xb0,A0)
0000EDD8: bra.b 0x0000ed60
0000EDDA: rts
0000EDDC: btst.b #0x4,(0x9e,A0)
0000EDE2: beq.w 0x0000ed3c
0000EDE6: btst.b #0x1,(0x10,A1)
0000EDEC: beq.w 0x0000ed3c
0000EDF0: move.l A0,(0x00ffe128).l
0000EDF6: bra.w 0x0000ece6
0000EDFA: move.w D2w,D1w
0000EDFC: andi.w #-0x40,D2w
0000EE00: lsr.w #0x1,D2w
0000EE02: andi.w #0x3f,D1w
0000EE06: lsl.w #0x5,D1w
0000EE08: move.w D2w,D3w
0000EE0A: addi.w #0x1f,D3w
0000EE0E: rts
0000EE10: tst.w (0x00ffdf8a).l
0000EE16: bne.w 0x0000ee1c
0000EE1A: rts
0000EE1C: clr.w (0x00ffdf8a).l
0000EE22: moveq #0x0,D0
0000EE24: lea (0xff1700).l,A0 ; data 00ff1700
0000EE2A: move.w #0x3ff,D7w
0000EE2E: move.l D0,(A0)+
0000EE30: dbf D7w,0x0000ee2e
0000EE34: movea.l (0x00ffddec).l,A1
0000EE3A: lea (0xff1700).l,A0 ; data 00ff1700
0000EE40: move.w (A1)+,D0w
0000EE42: bpl.w 0x0000ee4c
0000EE46: bsr.w 0x0000ee86 ; call 0000EE86 replace_nth_terrain_marker
0000EE4A: rts
0000EE4C: move.w (A1)+,D4w
0000EE4E: move.w (A1)+,D1w
0000EE50: move.w (A1)+,D5w
0000EE52: move.w (A1)+,D6w
0000EE54: move.w D0w,D2w
0000EE56: move.w D1w,D3w
0000EE58: andi.w #-0x20,D1w
0000EE5C: add.w D1w,D1w
0000EE5E: lsr.w #0x5,D0w
0000EE60: add.w D1w,D0w
0000EE62: move.w D0w,D7w
0000EE64: move.b D6b,(0x0,A0,D0w*0x1) ; data 00ff1700
0000EE68: addq.w #0x1,D0w
0000EE6A: addi.w #0x20,D2w
0000EE6E: cmp.w D2w,D4w
0000EE70: bcc.b 0x0000ee64
0000EE72: move.w (-0xa,A1),D2w
0000EE76: addi.w #0x40,D7w
0000EE7A: move.w D7w,D0w
0000EE7C: addi.w #0x20,D3w
0000EE80: cmp.w D3w,D5w
0000EE82: bcc.b 0x0000ee64
0000EE84: bra.b 0x0000ee40
0000EE86: cmpi.w #0x2,(0x00ffdf40).l
0000EE8E: bne.w 0x0000eef6
0000EE92: tst.b (0x00ffe184).l
0000EE98: bne.w 0x0000eeb2
0000EE9C: tst.b (0x00ffe187).l
0000EEA2: bne.w 0x0000eeb2
0000EEA6: cmpi.b #0x4,(0x00fffbed).l
0000EEAE: bne.w 0x0000eef6
0000EEB2: move.w (0x00ffe12c).l,D0w
0000EEB8: cmp.w (0x00fff0c8).l,D0w
0000EEBE: bne.w 0x0000eef6
0000EEC2: tst.b (0x00ffe180).l
0000EEC8: bne.w 0x0000eef6
0000EECC: move.b (0x00ffe185).l,D0b
0000EED2: ext.w D0w
0000EED4: lea (0xff1700).l,A0 ; data 00ff1700
0000EEDA: move.w #0xfff,D7w
0000EEDE: move.b (A0)+,D1b
0000EEE0: cmpi.b #-0x26,D1b
0000EEE4: bne.w 0x0000eef8
0000EEE8: subq.w #0x1,D0w
0000EEEA: bpl.w 0x0000eef8
0000EEEE: suba.w #0x1,A0
0000EEF2: move.b #-0x66,(A0)
0000EEF6: rts
0000EEF8: dbf D7w,0x0000eede
0000EEFC: bra.b 0x0000eed4
0000EF08: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
0000EF0E: movea.l #0xc288,A2
0000EF14: lea (0xff0100).l,A0 ; data 00ff0100
0000EF1A: moveq #0x2,D5
0000EF1C: btst.b #0x0,(0x10,A0)
0000EF22: beq.w 0x0000ef94
0000EF26: clr.l D1
0000EF28: move.b (0x4c,A0),D1b
0000EF2C: cmpi.w #0x5,D1w
0000EF30: bmi.w 0x0000ef3a
0000EF34: moveq #0x2,D1
0000EF36: bra.w 0x0000ef80
0000EF3A: cmpi.w #0x3,D1w
0000EF3E: bmi.w 0x0000ef48
0000EF42: moveq #0x1,D1
0000EF44: bra.w 0x0000ef80
0000EF48: moveq #0xa,D1
0000EF4A: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
0000EF50: lea (0x58,A0),A1
0000EF54: adda.l #0xe,A2
0000EF5A: move.w #0x6000,D1w
0000EF5E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000EF64: suba.l #0xe,A2
0000EF6A: lea (0xc2643).l,A1 ; data 000c2643
0000EF70: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000EF76: adda.l #0x180,A2
0000EF7C: bra.w 0x0000ef94
0000EF80: jsr 0x00003674.l ; call 00003674 FUN_00003674
0000EF86: moveq #0xa,D1
0000EF88: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
0000EF8E: bclr.b #0x4,(0x10,A0)
0000EF94: adda.l #0x100,A0
0000EF9A: dbf D5w,0x0000ef1c
0000EF9E: lea (0xc265e).l,A1 ; data 000c265e
0000EFA4: move.w #0x6000,D1w
0000EFA8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000EFAE: jmp 0x0000f312.l ; call 0000F312 wait_for_controller_event
0000EFE0: jmp 0x0000f312.l ; call 0000F312 wait_for_controller_event
0000F300: sne -(SP)
0000F302: movea.l #0xc30c,A2
0000F308: move.w #0x6000,D1w
0000F30C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F312: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000F318: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000F31E: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000F324: jsr 0x0000e660.l ; call 0000E660 FUN_0000e660
0000F32A: move.w (0x00ffdd22).l,D0w
0000F330: andi.w #-0x100,D0w
0000F334: beq.b 0x0000f312
0000F336: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000F33C: rts
0000F400: move.l (A4)+,(A3)+
0000F402: rts
0000F404: move.l #0xc306,D4
0000F40A: moveq #0x10,D1
0000F40C: moveq #0x1b,D7
0000F40E: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0000F414: rts
0000F416: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
0000F41C: lea (0xc56ae).l,A1 ; data 000c56ae
0000F422: movea.l #0xc216,A2
0000F428: move.w #0x6000,D1w
0000F42C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F432: lea (0xc56d6).l,A1 ; data 000c56d6
0000F438: movea.l #0xc292,A2
0000F43E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F448: roxl.b #0x7,D6b
0000F44A: movea.l #0xc394,A2
0000F450: move.b #-0x1,(0x00fff0b9).l
0000F458: btst.b #0x0,(0x00fffc27).l
0000F460: beq.w 0x0000f482
0000F464: lea (0xc5728).l,A1 ; data 000c5728
0000F46A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F470: adda.l #0x100,A2
0000F476: addq.b #0x1,(0x00fff0b9).l
0000F47C: move.l #0xc5774,(A3)+ ; data 000c5774
0000F482: btst.b #0x1,(0x00fffc27).l
0000F48A: beq.w 0x0000f4ac
0000F48E: lea (0xc5755).l,A1 ; data 000c5755
0000F494: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F49A: adda.l #0x100,A2
0000F4A0: addq.b #0x1,(0x00fff0b9).l
0000F4A6: move.l #0xc58d5,(A3)+ ; data 000c58d5
0000F4AC: btst.b #0x2,(0x00fffc27).l
0000F4B4: beq.w 0x0000f4d6
0000F4B8: lea (0xc5766).l,A1 ; data 000c5766
0000F4BE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F4C4: adda.l #0x100,A2
0000F4CA: addq.b #0x1,(0x00fff0b9).l
0000F4D0: move.l #0xc593f,(A3)+ ; data 000c593f
0000F4D6: btst.b #0x3,(0x00fffc27).l
0000F4DE: beq.w 0x0000f500
0000F4E2: lea (0xc5736).l,A1 ; data 000c5736
0000F4E8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F4EE: adda.l #0x100,A2
0000F4F4: addq.b #0x1,(0x00fff0b9).l
0000F4FA: move.l #0xc57d2,(A3)+ ; data 000c57d2
0000F500: btst.b #0x4,(0x00fffc27).l
0000F508: beq.w 0x0000f52a
0000F50C: lea (0xc5744).l,A1 ; data 000c5744
0000F512: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F518: adda.l #0x100,A2
0000F51E: addq.b #0x1,(0x00fff0b9).l
0000F524: move.l #0xc584e,(A3)+ ; data 000c584e
0000F52A: clr.b (0x00fff0dc).l
0000F530: clr.w D2w
0000F532: bra.w 0x0000f56e
0000F536: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000F53C: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000F542: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000F548: jsr 0x0000e660.l ; call 0000E660 FUN_0000e660
0000F54E: btst.b #0x8,(0x00ffdd22).l
0000F556: beq.w 0x0000f560
0000F55A: moveq #-0x1,D2
0000F55C: bra.w 0x0000f56e
0000F560: btst.b #0x9,(0x00ffdd22).l
0000F568: beq.w 0x0000f586
0000F56C: moveq #0x1,D2
0000F56E: clr.w D3w
0000F570: move.b (0x00fff0b9).l,D3b
0000F576: move.w #-0x3c70,D7w
0000F57A: move.w #0x6000,D4w
0000F57E: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
0000F584: bra.b 0x0000f536
0000F586: btst.b #0xe,(0x00ffdd22).l
0000F58E: beq.w 0x0000f5c2
0000F592: bsr.w 0x0000f404 ; call 0000F404 FUN_0000f404
0000F596: clr.l D2
0000F598: move.b (0x00fff0dc).l,D2b
0000F59E: lsl.w #0x2,D2w
0000F5A0: lea (0xffef16).l,A1 ; data 00ffef16
0000F5A6: movea.l (0x0,A1,D2w*0x1),A1
0000F5AA: movea.l #0xc38c,A2
0000F5B0: move.w #0x6000,D1w
0000F5B4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F5BA: bsr.w 0x0000f312 ; call 0000F312 wait_for_controller_event
0000F5BE: bra.w 0x0000f416
0000F5C2: btst.b #0xc,(0x00ffdd22).l
0000F5CA: beq.w 0x0000f5d0
0000F5CE: bra.b 0x0000f592
0000F5D0: btst.b #0xd,(0x00ffdd22).l
0000F5D8: beq.w 0x0000f5e6
0000F5DC: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000F5E2: bra.w 0x0000f536
0000F5E6: btst.b #0xf,(0x00ffdd22).l
0000F5EE: beq.w 0x0000f536
0000F5F2: rts
0000F5F4: tst.b (0x00fff112).l
0000F5FA: bne.w 0x0000f692
0000F5FE: lea (0xffef16).l,A3 ; data 00ffef16
0000F604: lea (0xffef56).l,A4 ; data 00ffef56
0000F60A: clr.b (0x00fff0cf).l
0000F610: move.b #-0x1,(0x00fff0b9).l
0000F618: clr.l D1
0000F61A: btst.l D1,D0
0000F61C: beq.w 0x0000f658
0000F620: cmpi.b #0x7,(0x00fff0cf).l
0000F628: bne.w 0x0000f644
0000F62C: move.l #0xc661d,(A3)+
0000F632: move.l #0x0,(A4)+
0000F638: addq.b #0x1,(0x00fff0b9).l
0000F63E: addq.b #0x1,(0x00fff0cf).l
0000F644: move.l (A1)+,(A3)+
0000F646: move.l (A2)+,(A4)+
0000F648: addq.b #0x1,(0x00fff0b9).l
0000F64E: addq.b #0x1,(0x00fff0cf).l
0000F654: bra.w 0x0000f65c
0000F658: move.l (A1)+,D2
0000F65A: move.l (A2)+,D2
0000F65C: addq.w #0x1,D1w
0000F65E: dbf D7w,0x0000f61a
0000F662: cmpi.b #0x9,(0x00fff0cf).l
0000F66A: bmi.w 0x0000f692
0000F66E: beq.w 0x0000f682
0000F672: move.l #0xc662b,(A3)+
0000F678: move.l #0x0,(A4)+
0000F67E: bra.w 0x0000f692
0000F682: movea.l A3,A1
0000F684: movea.l A4,A2
0000F686: subq.l #0x4,A1
0000F688: subq.l #0x4,A2
0000F68A: subq.l #0x8,A3
0000F68C: subq.l #0x8,A4
0000F68E: move.l (A1),(A3)
0000F690: move.l (A2),(A4)
0000F692: clr.l D7
0000F694: move.w #0x6000,D1w
0000F698: movea.l A6,A2
0000F69A: move.b (0x00fff0b9).l,D7b
0000F6A0: tst.b (0x00fff112).l
0000F6A6: bne.w 0x0000f6c0
0000F6AA: lea (0xffef16).l,A3 ; data 00ffef16
0000F6B0: cmpi.w #0x8,D7w
0000F6B4: bmi.w 0x0000f6c8
0000F6B8: move.w #0x7,D7w
0000F6BC: bra.w 0x0000f6c8
0000F6C0: lea (0xffef36).l,A3 ; data 00ffef36
0000F6C6: subq.w #0x7,D7w
0000F6C8: movea.l (A3)+,A1
0000F6CA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F6D0: adda.l #0x100,A2
0000F6D6: dbf D7w,0x0000f6c8
0000F6DA: rts
0000F848: ori.b #-0x1a,D0b
0000F84C: lea (0xc56a5).l,A1 ; data 000c56a5
0000F852: movea.l #0xc21a,A2
0000F858: move.w #0x6000,D1w
0000F85C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F862: lea (0xc56d6).l,A1 ; data 000c56d6
0000F868: movea.l #0xc292,A2
0000F86E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F874: move.l (0x00fffc2c).l,D0
0000F87A: lea (0xc66ac).l,A1 ; data 000c66ac
0000F880: lea (0xc66e0).l,A2 ; data 000c66e0
0000F886: move.w #0xc,D7w
0000F88A: movea.l #0xc394,A6
0000F890: bsr.w 0x0000f5f4 ; call 0000F5F4 FUN_0000f5f4
0000F894: clr.b (0x00fff0dc).l
0000F89A: clr.w D2w
0000F89C: bra.w 0x0000f8d8
0000F8A0: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000F8A6: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000F8AC: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000F8B2: jsr 0x0000e660.l ; call 0000E660 FUN_0000e660
0000F8B8: btst.b #0x8,(0x00ffdd22).l
0000F8C0: beq.w 0x0000f8ca
0000F8C4: moveq #-0x1,D2
0000F8C6: bra.w 0x0000f8d8
0000F8CA: btst.b #0x9,(0x00ffdd22).l
0000F8D2: beq.w 0x0000f90c
0000F8D6: moveq #0x1,D2
0000F8D8: clr.w D3w
0000F8DA: move.b (0x00fff0b9).l,D3b
0000F8E0: tst.b (0x00fff112).l
0000F8E6: bne.w 0x0000f8fa
0000F8EA: cmpi.w #0x8,D3w
0000F8EE: bmi.w 0x0000f8fc
0000F8F2: move.w #0x7,D3w
0000F8F6: bra.w 0x0000f8fc
0000F8FA: subq.w #0x7,D3w
0000F8FC: move.w #-0x3c70,D7w
0000F900: move.w #0x6000,D4w
0000F904: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
0000F90A: bra.b 0x0000f8a0
0000F90C: btst.b #0xe,(0x00ffdd22).l
0000F914: beq.w 0x0000f972
0000F918: bsr.w 0x0000f404 ; call 0000F404 FUN_0000f404
0000F91C: clr.l D2
0000F91E: move.b (0x00fff0dc).l,D2b
0000F924: lsl.w #0x2,D2w
0000F926: tst.b (0x00fff112).l
0000F92C: bne.w 0x0000f93a
0000F930: lea (0xffef56).l,A1 ; data 00ffef56
0000F936: bra.w 0x0000f940
0000F93A: lea (0xffef76).l,A1 ; data 00ffef76
0000F940: tst.l (0x0,A1,D2w*0x1)
0000F944: bne.w 0x0000f956
0000F948: move.b #-0x1,D0b
0000F94C: eor.b D0b,(0x00fff112).l
0000F952: bra.w 0x0000f846
0000F956: movea.l (0x0,A1,D2w*0x1),A1
0000F95A: movea.l #0xc38a,A2
0000F960: move.w #0x6000,D1w
0000F964: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F96A: bsr.w 0x0000f312 ; call 0000F312 wait_for_controller_event
0000F96E: bra.w 0x0000f846
0000F972: btst.b #0xc,(0x00ffdd22).l
0000F97A: beq.w 0x0000f980
0000F97E: bra.b 0x0000f918
0000F980: btst.b #0xd,(0x00ffdd22).l
0000F988: beq.w 0x0000f996
0000F98C: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000F992: bra.w 0x0000f8a0
0000F996: btst.b #0xf,(0x00ffdd22).l
0000F99E: beq.w 0x0000f8a0
0000F9A2: rts
0000F9A4: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
0000F9AA: lea (0xc56c9).l,A1 ; data 000c56c9
0000F9B0: movea.l #0xc216,A2
0000F9B6: move.w #0x6000,D1w
0000F9BA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F9C0: lea (0xc56d6).l,A1 ; data 000c56d6
0000F9C6: movea.l #0xc292,A2
0000F9CC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000F9D2: lea (0xc2716).l,A1 ; data 000c2716
0000F9D8: lea (0xffef16).l,A2 ; data 00ffef16
0000F9DE: move.b #-0x1,(0x00fff0b9).l
0000F9E6: clr.l D1
0000F9E8: move.l (0x00fffc30).l,D0
0000F9EE: move.w #0x1f,D7w
0000F9F2: btst.l D1,D0
0000F9F4: bne.w 0x0000f9fe
0000F9F8: move.l (A1)+,D2
0000F9FA: bra.w 0x0000fa06
0000F9FE: move.l (A1)+,(A2)+
0000FA00: addq.b #0x1,(0x00fff0b9).l
0000FA06: addq.w #0x1,D1w
0000FA08: dbf D7w,0x0000f9f2
0000FA0C: lea (0xc2796).l,A1 ; data 000c2796
0000FA12: clr.l D1
0000FA14: move.l (0x00fffc34).l,D0
0000FA1A: move.w #0x1f,D7w
0000FA1E: btst.l D1,D0
0000FA20: bne.w 0x0000fa2a
0000FA24: move.l (A1)+,D2
0000FA26: bra.w 0x0000fa32
0000FA2A: move.l (A1)+,(A2)+
0000FA2C: addq.b #0x1,(0x00fff0b9).l
0000FA32: addq.w #0x1,D1w
0000FA34: dbf D7w,0x0000fa1e
0000FA38: lea (0xc2816).l,A1 ; data 000c2816
0000FA3E: clr.l D1
0000FA40: move.l (0x00fffc38).l,D0
0000FA46: move.w #0x1f,D7w
0000FA4A: btst.l D1,D0
0000FA4C: bne.w 0x0000fa56
0000FA50: move.l (A1)+,D2
0000FA52: bra.w 0x0000fa5e
0000FA56: move.l (A1)+,(A2)+
0000FA58: addq.b #0x1,(0x00fff0b9).l
0000FA5E: addq.w #0x1,D1w
0000FA60: dbf D7w,0x0000fa4a
0000FA64: lea (0xc663a).l,A1 ; data 000c663a
0000FA6A: movea.l #0xcba2,A2
0000FA70: move.w #0x6000,D1w
0000FA74: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000FA7A: clr.b (0x00fff0dc).l
0000FA80: move.w #0x1,D1w
0000FA84: movea.l #0xcbae,A2
0000FA8A: move.w #0x6000,D2w
0000FA8E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000FA94: move.b (0x00fff0b9).l,D1b
0000FA9A: addq.w #0x1,D1w
0000FA9C: movea.l #0xcbba,A2
0000FAA2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000FAA8: clr.l D2
0000FAAA: bra.w 0x0000fb12
0000FAAE: clr.l D1
0000FAB0: lea (0xc6645).l,A1 ; data 000c6645
0000FAB6: movea.l #0xcbac,A2
0000FABC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000FAC2: move.b (0x00fff0dc).l,D1b
0000FAC8: addq.l #0x1,D1
0000FACA: movea.l #0xcbae,A2
0000FAD0: move.w #0x6000,D2w
0000FAD4: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0000FADA: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000FAE0: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000FAE6: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000FAEC: jsr 0x0000e660.l ; call 0000E660 FUN_0000e660
0000FAF2: btst.b #0xa,(0x00ffdd22).l
0000FAFA: beq.w 0x0000fb04
0000FAFE: moveq #-0x1,D2
0000FB00: bra.w 0x0000fb12
0000FB04: btst.b #0xb,(0x00ffdd22).l
0000FB0C: beq.w 0x0000fb66
0000FB10: moveq #0x1,D2
0000FB12: add.b D2b,(0x00fff0dc).l
0000FB18: bpl.w 0x0000fb24
0000FB1C: clr.b (0x00fff0dc).l
0000FB22: bra.b 0x0000fada
0000FB24: clr.w D3w
0000FB26: move.b (0x00fff0dc).l,D3b
0000FB2C: cmp.b (0x00fff0b9).l,D3b
0000FB32: ble.w 0x0000fb42
0000FB36: move.b (0x00fff0b9).l,(0x00fff0dc).l
0000FB40: bra.b 0x0000fada
0000FB42: bsr.w 0x0000f404 ; call 0000F404 FUN_0000f404
0000FB46: lsl.w #0x2,D3w
0000FB48: lea (0xffef16).l,A1 ; data 00ffef16
0000FB4E: movea.l (0x0,A1,D3w*0x1),A1 ; data 00ffef16
0000FB52: movea.l #0xc38a,A2
0000FB58: move.w #0x6000,D1w
0000FB5C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000FB62: bra.w 0x0000faae
0000FB66: btst.b #0xe,(0x00ffdd22).l
0000FB6E: beq.w 0x0000fb74
0000FB72: rts
0000FB74: btst.b #0xc,(0x00ffdd22).l
0000FB7C: beq.w 0x0000fb82
0000FB80: bra.b 0x0000fb72
0000FB82: btst.b #0xd,(0x00ffdd22).l
0000FB8A: beq.w 0x0000fb98
0000FB8E: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
0000FB94: bra.w 0x0000fada
0000FB98: btst.b #0xf,(0x00ffdd22).l
0000FBA0: beq.w 0x0000fada
0000FBA4: rts
0000FBA6: clr.b (0x00fff0db).l
0000FBAC: movea.l (0x00fffc04).l,A0
0000FBB2: cmpi.b #0x2,(0x9b,A0)
0000FBB8: bmi.w 0x0000fbf4
0000FBBC: jsr 0x00011900.l ; call 00011900 FUN_00011900
0000FBC2: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
0000FBC6: lea (0xc89c0).l,A1 ; data 000c89c0
0000FBCC: movea.l #0xc31c,A2
0000FBD2: move.w #0x6000,D1w
0000FBD6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0000FBDC: clr.w D2w
0000FBDE: bsr.w 0x000106da ; call 000106DA FUN_000106da
0000FBE2: moveq #-0x1,D2
0000FBE4: bsr.w 0x000106da ; call 000106DA FUN_000106da
0000FBE8: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
0000FBEE: jmp 0x00011928.l ; call 00011928 FUN_00011928
0000FBF4: tst.b (0x7f,A0)
0000FBF8: beq.b 0x0000fbbc
0000FBFA: move.w #-0x7ecc,(0x00c00004).l
0000FC02: move.w (0x00fff0d0).l,-(SP)
0000FC08: clr.w (0x00fff0d0).l
0000FC0E: lea (0xc8a60).l,A0 ; data 000c8a60
0000FC14: jsr 0x00001358.l ; call 00001358 copy_source_word_block
0000FC1A: bset.b #0x0,(0x00ffef14).l
0000FC22: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0000FC28: lea (0xffbf40).l,A1 ; data 00ffbf40
0000FC2E: movea.l #0xc6714,A3
0000FC34: move.l #0x16a0,D0
0000FC3A: jsr 0x00001530.l ; call 00001530 FUN_00001530
0000FC40: lea (0xc8108).l,A1 ; data 000c8108
0000FC46: move.w #-0x2000,D4w
0000FC4A: move.w #0x21,D7w
0000FC4E: move.w #0x1b,D1w
0000FC52: move.w #0x0,D2w
0000FC56: clr.w D3w
0000FC58: move.w D7w,D5w
0000FC5A: addq.w #0x1,D5w
0000FC5C: jsr 0x00001728.l ; call 00001728 FUN_00001728
0000FC62: movea.l (0x00fffc04).l,A0
0000FC68: adda.l #0xa0,A0
0000FC6E: move.w (A0),D1w
0000FC70: mulu.w #0x22,D1
0000FC74: lea (0xfff9fa).l,A0 ; data 00fff9fa
0000FC7A: adda.l D1,A0
0000FC7C: move.l A0,(0x00fff10e).l
0000FC82: movea.l (0x00fff10e).l,A0
0000FC88: move.w (A0)+,D3w ; data 00fff9fa
0000FC8A: move.w #-0x1578,D4w
0000FC8E: clr.l D0
0000FC90: clr.l D1
0000FC92: move.b (A0),D1b ; data 00fff9fc
0000FC94: cmpi.b #0x6,D1b
0000FC98: bpl.w 0x0000fca4
0000FC9C: move.w #0x2000,D6w
0000FCA0: bra.w 0x0000fca8
0000FCA4: move.w #0x6000,D6w
0000FCA8: mulu.w #0x30,D1
0000FCAC: lea (0xd4ea0).l,A4 ; data 000d4ea0
0000FCB2: adda.l D1,A4
0000FCB4: movea.l (0x1a,A4),A1 ; data 000d4eba
0000FCB8: bsr.w 0x00011190 ; call 00011190 FUN_00011190
0000FCBC: addq.l #0x2,A0
0000FCBE: adda.l #0x30,A4
0000FCC4: addq.l #0x8,D4
0000FCC6: addq.w #0x1,D0w
0000FCC8: cmpi.w #0x7,D0w
0000FCCC: bne.w 0x0000fcda
0000FCD0: addi.w #0x180,D4w
0000FCD4: subi.l #0x38,D4
0000FCDA: dbf D3w,0x0000fc90
0000FCDE: bsr.w 0x00010276 ; call 00010276 FUN_00010276
0000FCE2: move.w #-0x7e8c,(0x00c00004).l
0000FCEA: jsr 0x0000fdfe.l ; call 0000FDFE FUN_0000fdfe
0000FCF0: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0000FCF6: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0000FCFC: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0000FD02: btst.b #0xa,(0x00ffdd22).l
0000FD0A: beq.w 0x0000fd14
0000FD0E: moveq #-0x1,D0
0000FD10: bra.w 0x0000fd22
0000FD14: btst.b #0xb,(0x00ffdd22).l
0000FD1C: beq.w 0x0000fd32
0000FD20: moveq #0x1,D0
0000FD22: bsr.w 0x0000febe ; call 0000FEBE move_selection_column
0000FD26: bsr.w 0x00010276 ; call 00010276 FUN_00010276
0000FD2A: clr.b (0x00fff0da).l
0000FD30: bra.b 0x0000fcea
0000FD32: btst.b #0x8,(0x00ffdd22).l
0000FD3A: beq.w 0x0000fd44
0000FD3E: moveq #-0x1,D0
0000FD40: bra.w 0x0000fd52
0000FD44: btst.b #0x9,(0x00ffdd22).l
0000FD4C: beq.w 0x0000fd62
0000FD50: moveq #0x1,D0
0000FD52: bsr.w 0x0000ff06 ; call 0000FF06 move_selection_row
0000FD56: bsr.w 0x00010276 ; call 00010276 FUN_00010276
0000FD5A: clr.b (0x00fff0da).l
0000FD60: bra.b 0x0000fcea
0000FD62: btst.b #0xd,(0x00ffdd22).l
0000FD6A: beq.w 0x0000fd72
0000FD6E: bra.w 0x0000fd8e
0000FD72: btst.b #0xc,(0x00ffdd22).l
0000FD7A: beq.w 0x0000fd82
0000FD7E: bra.w 0x0000fd8e
0000FD82: btst.b #0xe,(0x00ffdd22).l
0000FD8A: beq.w 0x0000fd9a
0000FD8E: bsr.w 0x0000ff5a ; call 0000FF5A FUN_0000ff5a
0000FD92: bsr.w 0x00010276 ; call 00010276 FUN_00010276
0000FD96: bra.w 0x0000fcea
0000FD9A: btst.b #0xf,(0x00ffdd22).l
0000FDA2: beq.w 0x0000fcea
0000FDA6: move.w (SP)+,(0x00fff0d0).l
0000FDAC: rts
0000FDAE: lea (0xc8aee).l,A2 ; data 000c8aee
0000FDB4: clr.l D1
0000FDB6: move.b (0x00fff0db).l,D1b
0000FDBC: lsl.w #0x2,D1w
0000FDBE: adda.l D1,A2
0000FDC0: movea.l (A2),A2 ; data 000c8aee
0000FDC2: moveq #0x4,D7
0000FDC4: moveq #0x0,D1
0000FDC6: move.w #0x0,D2w
0000FDCA: moveq #0x1,D6
0000FDCC: move.l #0xc112,D4
0000FDD2: movea.l (A2)+,A1
0000FDD4: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000FDDA: addi.l #0x180,D4
0000FDE0: dbf D6w,0x0000fdd2
0000FDE4: moveq #0x0,D7
0000FDE6: moveq #0x1,D1
0000FDE8: moveq #0x1,D6
0000FDEA: move.w #-0x3e6e,D4w
0000FDEE: movea.l (A2)+,A1
0000FDF0: jsr 0x00001232.l ; call 00001232 FUN_00001232
0000FDF6: addq.l #0x8,D4
0000FDF8: dbf D6w,0x0000fdee
0000FDFC: rts
0000FDFE: subq.b #0x1,(0x00fff0da).l
0000FE04: bpl.w 0x0000feb8
0000FE08: move.b #0x6,(0x00fff0da).l
0000FE10: addq.b #0x1,(0x00fff0db).l
0000FE16: cmpi.b #0x4,(0x00fff0db).l
0000FE1E: bmi.w 0x0000fe28
0000FE22: clr.b (0x00fff0db).l
0000FE28: lea (0xc8b3e).l,A2 ; data 000c8b3e
0000FE2E: clr.l D1
0000FE30: move.b (0x00fff0db).l,D1b
0000FE36: lsl.w #0x2,D1w
0000FE38: adda.l D1,A2
0000FE3A: movea.l (A2),A2 ; data 000c8b3e
0000FE3C: lea (0xffd986).l,A3 ; data 00ffd986
0000FE42: move.w #0x1,(0x00ffdd02).l
0000FE4A: move.w #0x2,D5w
0000FE4E: clr.l D1
0000FE50: clr.l D2
0000FE52: move.w #0x4000,D6w
0000FE56: jsr 0x00001378.l ; call 00001378 FUN_00001378
0000FE5C: clr.b (0x3,A3)
0000FE60: lea (0xffd986).l,A4 ; data 00ffd986
0000FE66: clr.l D0
0000FE68: move.b (0x00fff0d0).l,D0b
0000FE6E: mulu.w #0x20,D0
0000FE72: add.w D0w,(0x6,A4)
0000FE76: tst.b (0x3,A4)
0000FE7A: beq.w 0x0000fe82
0000FE7E: addq.l #0x8,A4
0000FE80: bra.b 0x0000fe72
0000FE82: lea (0xffd986).l,A4 ; data 00ffd986
0000FE88: clr.l D0
0000FE8A: move.b (0x00fff0d1).l,D0b
0000FE90: mulu.w #0x18,D0
0000FE94: add.w D0w,(A4)
0000FE96: tst.b (0x3,A4)
0000FE9A: beq.w 0x0000fea2
0000FE9E: addq.l #0x8,A4
0000FEA0: bra.b 0x0000fe94
0000FEA2: subq.w #0x8,A3
0000FEA4: clr.b (0x3,A3)
0000FEA8: move.w #0x1,(0x00ffd980).l
0000FEB0: bset.b #0x1,(0x00ffef14).l
0000FEB8: bsr.w 0x0000fdae ; call 0000FDAE FUN_0000fdae
0000FEBC: rts
0000FEBE: movea.l (0x00fff10e).l,A0
0000FEC4: move.w (A0)+,D3w
0000FEC6: tst.b (0x00fff0d1).l
0000FECC: beq.w 0x0000fed6
0000FED0: subq.w #0x7,D3w
0000FED2: bra.w 0x0000fee0
0000FED6: cmpi.w #0x7,D3w
0000FEDA: bmi.w 0x0000fee0
0000FEDE: moveq #0x6,D3
0000FEE0: add.b D0b,(0x00fff0d0).l
0000FEE6: bpl.w 0x0000fef2
0000FEEA: move.b D3b,(0x00fff0d0).l
0000FEF0: rts
0000FEF2: cmp.b (0x00fff0d0).l,D3b
0000FEF8: bmi.w 0x0000fefe
0000FEFC: rts
0000FEFE: clr.b (0x00fff0d0).l
0000FF04: rts
0000FF06: movea.l (0x00fff10e).l,A0
0000FF0C: move.w (A0)+,D3w
0000FF0E: subq.w #0x7,D3w
0000FF10: bpl.w 0x0000ff16
0000FF14: rts
0000FF16: add.b D0b,(0x00fff0d1).l
0000FF1C: bpl.w 0x0000ff3c
0000FF20: move.b #0x1,(0x00fff0d1).l
0000FF28: cmp.b (0x00fff0d0).l,D3b
0000FF2E: bmi.w 0x0000ff34
0000FF32: rts
0000FF34: clr.b (0x00fff0d1).l
0000FF3A: rts
0000FF3C: cmpi.b #0x2,(0x00fff0d1).l
0000FF44: beq.w 0x0000ff52
0000FF48: tst.b (0x00fff0d1).l
0000FF4E: bne.b 0x0000ff28
0000FF50: rts
0000FF52: clr.b (0x00fff0d1).l
0000FF58: rts
0000FF5A: bsr.w 0x000102d4 ; call 000102D4 FUN_000102d4
0000FF5E: movea.l (0x00fffc04).l,A0
0000FF64: tst.b (0x56,A0)
0000FF68: bmi.w 0x0000ff7e
0000FF6C: tst.b (0x57,A0)
0000FF70: beq.w 0x0000ff7e
0000FF74: jsr 0x000125fa.l ; call 000125FA FUN_000125fa
0000FF7A: move.b D1b,(0x4e,A0)
0000FF7E: move.b D7b,(0x57,A0)
0000FF82: addq.b #0x1,(0x57,A0)
0000FF86: bsr.w 0x00010016 ; call 00010016 FUN_00010016
0000FF8A: btst.b #0xd,(0x00ffdd22).l
0000FF92: beq.w 0x0000ff9a
0000FF96: bra.w 0x0000ffb6
0000FF9A: btst.b #0xc,(0x00ffdd22).l
0000FFA2: beq.w 0x0000ffaa
0000FFA6: bra.w 0x0000ffb6
0000FFAA: btst.b #0xe,(0x00ffdd22).l
0000FFB2: beq.w 0x0000ffe8
0000FFB6: movea.l (0x00fffc04).l,A0
0000FFBC: move.b #-0x1,(0x56,A0)
0000FFC2: jsr 0x00003404.l ; call 00003404 FUN_00003404
0000FFC8: jsr 0x00003548.l ; call 00003548 FUN_00003548
0000FFCE: clr.w D1w
0000FFD0: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
0000FFD6: jsr 0x000036fc.l ; call 000036FC FUN_000036fc
0000FFDC: move.w (0x00fff096).l,D6w
0000FFE2: move.b D6b,(0x9d,A0)
0000FFE6: rts
0000FFE8: bsr.w 0x00010722 ; call 00010722 FUN_00010722
0000FFEC: btst.b #0xd,(0x00ffdd22).l
0000FFF4: beq.w 0x0000fffa
0000FFF8: bra.b 0x0000ffb6
0000FFFA: btst.b #0xc,(0x00ffdd22).l
00010002: beq.w 0x00010008
00010006: bra.b 0x0000ffb6
00010008: btst.b #0xe,(0x00ffdd22).l
00010010: beq.w 0x0000ff86
00010014: bra.b 0x0000ffb6
00010016: lea (0xc85a8).l,A2 ; data 000c85a8
0001001C: bsr.w 0x00010172 ; call 00010172 FUN_00010172
00010020: jsr 0x0000fdfe.l ; call 0000FDFE FUN_0000fdfe
00010026: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0001002C: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00010032: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00010038: btst.b #0xb,(0x00ffdd22).l
00010040: beq.w 0x0001004c
00010044: move.w #0x1,D1w
00010048: bra.w 0x0001005c
0001004C: btst.b #0xa,(0x00ffdd22).l
00010054: beq.w 0x00010062
00010058: move.w #-0x1,D1w
0001005C: bsr.w 0x000100a8 ; call 000100A8 FUN_000100a8
00010060: bra.b 0x00010020
00010062: btst.b #0x9,(0x00ffdd22).l
0001006A: beq.w 0x00010072
0001006E: bra.w 0x0001009c
00010072: btst.b #0xe,(0x00ffdd22).l
0001007A: beq.w 0x00010082
0001007E: bra.w 0x0001009c
00010082: btst.b #0xc,(0x00ffdd22).l
0001008A: beq.w 0x00010092
0001008E: bra.w 0x0001009c
00010092: btst.b #0xd,(0x00ffdd22).l
0001009A: beq.b 0x00010020
0001009C: lea (0xc85ac).l,A2 ; data 000c85ac
000100A2: bsr.w 0x00010172 ; call 00010172 FUN_00010172
000100A6: rts
000100A8: move.w (0x00fff098).l,D0w
000100AE: add.w D1w,(0x00fff096).l
000100B4: cmp.w (0x00fff096).l,D0w
000100BA: bge.w 0x000100c4
000100BE: move.w D0w,(0x00fff096).l
000100C4: tst.w (0x00fff096).l
000100CA: bne.w 0x000100d6
000100CE: move.w #0x1,(0x00fff096).l
000100D6: move.w (0x00fff096).l,D1w
000100DC: movea.l (0x00fffc04).l,A0
000100E2: move.b D1b,(0x9d,A0)
000100E6: move.w #0x0,D2w
000100EA: movea.l #0xc6b6,A2
000100F0: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000100F6: lea (0xc85a8).l,A2 ; data 000c85a8
000100FC: bsr.w 0x00010172 ; call 00010172 FUN_00010172
00010100: move.l #0xc588,D4
00010106: lea (0xc1320).l,A1 ; data 000c1320
0001010C: move.w #0x0,D2w
00010110: moveq #0xf,D7
00010112: clr.l D1
00010114: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0001011A: moveq #0x0,D0
0001011C: move.l (0x00fff096).l,-(SP)
00010122: bsr.w 0x0001055e ; call 0001055E FUN_0001055e
00010126: move.l (SP)+,(0x00fff096).l
0001012C: bsr.w 0x00010156 ; call 00010156 FUN_00010156
00010130: clr.l D0
00010132: move.b (0x00fff0d1).l,D0b
00010138: mulu.w #0x6,D0
0001013C: add.b (0x00fff0d0).l,D0b
00010142: movea.l (0x00fff10e).l,A0
00010148: addq.w #0x1,D0w
0001014A: lsl.w #0x1,D0w
0001014C: move.b (0x0,A0,D0w*0x1),D7b
00010150: bsr.w 0x000105e6 ; call 000105E6 FUN_000105e6
00010154: rts
00010156: lea (0xc1320).l,A1 ; data 000c1320
0001015C: move.l #0xc688,D4
00010162: clr.l D1
00010164: moveq #0x9,D7
00010166: move.w #0x0,D2w
0001016A: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00010170: rts
00010172: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00010176: move.l #0xc728,D4
0001017C: move.w #0x0,D2w
00010180: clr.l D1
00010182: clr.l D7
00010184: lea (0xc85a2).l,A1 ; data 000c85a2
0001018A: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00010190: addq.w #0x2,D4w
00010192: move.w (0x00fff096).l,D0w
00010198: subq.w #0x2,D0w
0001019A: bmi.w 0x000101b4
0001019E: move.w D0w,D7w
000101A0: lea (0xc85a6).l,A1 ; data 000c85a6
000101A6: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000101AC: addq.w #0x1,D0w
000101AE: mulu.w #0x2,D0
000101B2: add.w D0w,D4w
000101B4: move.w #0x1,D7w
000101B8: movea.l A2,A1
000101BA: jsr 0x00001232.l ; call 00001232 FUN_00001232
000101C0: addq.w #0x4,D4w
000101C2: move.w (0x00fff098).l,D0w
000101C8: sub.w (0x00fff096).l,D0w
000101CE: beq.w 0x000101ea
000101D2: subq.w #0x1,D0w
000101D4: move.w D0w,D7w
000101D6: lea (0xc85a6).l,A1 ; data 000c85a6
000101DC: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000101E2: addq.w #0x1,D0w
000101E4: mulu.w #0x2,D0
000101E8: add.w D0w,D4w
000101EA: clr.l D7
000101EC: lea (0xc85a4).l,A1 ; data 000c85a4
000101F2: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000101F8: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
000101FC: rts
000101FE: clr.l D7
00010200: clr.l D3
00010202: clr.l D0
00010204: move.b (0x00fff0d1).l,D0b
0001020A: mulu.w #0x7,D0
0001020E: add.b (0x00fff0d0).l,D0b
00010214: movea.l (0x00fff10e).l,A0
0001021A: addq.w #0x1,D0w
0001021C: lsl.w #0x1,D0w
0001021E: move.b (0x0,A0,D0w*0x1),D7b
00010222: addq.w #0x1,D0w
00010224: move.b (0x0,A0,D0w*0x1),D3b
00010228: move.w D3w,(0x00fff098).l
0001022E: movea.l (0x00fffc04).l,A4
00010234: tst.b (0x56,A4)
00010238: bpl.w 0x00010248
0001023C: move.w D7w,D4w
0001023E: addq.w #0x1,D4w
00010240: cmp.b (0x57,A4),D4b
00010244: beq.w 0x00010254
00010248: move.w (0x00fff098).l,(0x00fff096).l
00010252: rts
00010254: move.b (0x9d,A4),D3b
00010258: move.w D3w,(0x00fff096).l
0001025E: rts
00010260: move.w #-0x3d78,D4w
00010264: move.w #0x11,D1w
00010268: move.w #0x1c,D7w
0001026C: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
00010272: bsr.b 0x000101fe ; call 000101FE FUN_000101fe
00010274: rts
00010276: bsr.b 0x00010260 ; call 00010260 FUN_00010260
00010278: lea (0xc85ac).l,A2 ; data 000c85ac
0001027E: bsr.w 0x00010172 ; call 00010172 FUN_00010172
00010282: cmpi.b #0x6,D7b
00010286: bpl.w 0x00010292
0001028A: move.w #0x2000,D6w
0001028E: bra.w 0x00010296
00010292: move.w #0x6000,D6w
00010296: move.w #-0x3e6c,D4w
0001029A: lea (0xd4ea0).l,A4 ; data 000d4ea0
000102A0: mulu.w #0x30,D7
000102A4: movea.l (0x1a,A4,D7w*0x1),A1 ; data 000d4eba
000102A8: bsr.w 0x00011190 ; call 00011190 FUN_00011190
000102AC: lea (0x0,A4,D7w*0x1),A1
000102B0: movea.l #0xc21c,A2
000102B6: move.w #0x0,D1w
000102BA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000102C0: movea.l (0x1e,A4,D7w*0x1),A1 ; data 000d4ebe
000102C4: movea.l #0xc308,A2
000102CA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000102D0: bra.w 0x000102ec
000102D4: bsr.b 0x00010260 ; call 00010260 FUN_00010260
000102D6: lea (0xc8a4a).l,A1 ; data 000c8a4a
000102DC: movea.l #0xc390,A2
000102E2: move.w #0x0,D1w
000102E6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000102EC: lea (0xc89fc).l,A1 ; data 000c89fc
000102F2: movea.l #0xc508,A2
000102F8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000102FE: lea (0xc8a3a).l,A1 ; data 000c8a3a
00010304: movea.l #0xc6a8,A2
0001030A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00010310: move.l D7,-(SP) ; data Stack[-0x4]
00010312: clr.l D0
00010314: lea (0xc85b0).l,A1 ; data 000c85b0
0001031A: bsr.w 0x000107ce ; call 000107CE FUN_000107ce
0001031E: bsr.w 0x0001088c ; call 0001088C FUN_0001088c
00010322: move.l (SP)+,D7
00010324: clr.l D1
00010326: move.w (0x00fff096).l,D1w
0001032C: move.w #0x0,D2w
00010330: movea.l #0xc6b6,A2
00010336: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0001033C: rts
0001033E: cmpi.b #0x1,(0x9c,A0)
00010344: bne.w 0x0001035a
00010348: clr.b (0x00fff0dd).l
0001034E: move.b (0x00fff0dd).l,(0x9f,A0)
00010356: bra.w 0x000103c0
0001035A: jsr 0x00055f02.l ; call 00055F02 FUN_00055f02
00010360: clr.l D6
00010362: clr.l D7
00010364: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
0001036A: cmp.b (0x7f,A0),D6b
0001036E: ble.w 0x00010376
00010372: move.b (0x7f,A0),D6b
00010376: add.b (0x9d,A0),D6b
0001037A: clr.l D7
0001037C: move.b (0x57,A0),D7b
00010380: subq.w #0x1,D7w
00010382: move.l D7,D5
00010384: addi.w #0x19,D5w
00010388: bsr.w 0x000104f6 ; call 000104F6 FUN_000104f6
0001038C: moveq #0x4,D4
0001038E: jsr 0x00055740.l ; call 00055740 FUN_00055740
00010394: jsr 0x00055786.l ; call 00055786 FUN_00055786
0001039A: jsr 0x00000d5a.l ; call 00000D5A FUN_00000d5a
000103A0: cmpi.w #0x2,D4w
000103A4: bpl.w 0x000103ac
000103A8: move.w #0x2,D4w
000103AC: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000103B2: move.b (0x00fff0dd).l,(0x9f,A0)
000103BA: bne.w 0x000103d8
000103BE: rts
000103C0: cmpi.w #0x3,(0x3e,A0)
000103C6: bmi.w 0x000103d6
000103CA: move.b #0x0,(0x56,A0)
000103D0: move.b (0x66,A0),(0x57,A0)
000103D6: rts
000103D8: move.l A5,-(SP) ; data Stack[-0x4]
000103DA: jsr 0x00015830.l ; call 00015830 FUN_00015830
000103E0: movea.l (SP)+,A5
000103E2: clr.l D4
000103E4: move.b (0x9d,A0),D4b
000103E8: clr.l D7
000103EA: move.b (0x57,A0),D7b
000103EE: subq.w #0x1,D7w
000103F0: movea.l (0x00fff10e).l,A6
000103F6: cmpi.w #0x5,D7w
000103FA: bne.w 0x0001040e
000103FE: move.b (0x9f,A0),(0x1e,A6)
00010404: move.b (0x9d,A0),(0x1f,A6)
0001040A: bra.w 0x00010422
0001040E: cmpi.w #0xc,D7w
00010412: bne.w 0x00010422
00010416: move.b (0x9f,A0),(0x20,A6)
0001041C: move.b (0x9d,A0),(0x21,A6)
00010422: bsr.w 0x00010660 ; call 00010660 FUN_00010660
00010426: ble.w 0x0001046e
0001042A: move.w D2w,D1w
0001042C: moveq #0x6,D7
0001042E: cmpi.b #0x13,(0x66,A0,D7w*0x1)
00010434: bne.w 0x00010462
00010438: cmp.b (0x6e,A0,D7w*0x1),D1b
0001043C: ble.w 0x00010450
00010440: sub.b (0x6e,A0,D7w*0x1),D1b
00010444: clr.b (0x66,A0,D7w*0x1)
00010448: clr.b (0x6e,A0,D7w*0x1)
0001044C: bra.w 0x00010462
00010450: sub.b D1b,(0x6e,A0,D7w*0x1)
00010454: bgt.w 0x0001047c
00010458: clr.b (0x66,A0,D7w*0x1)
0001045C: clr.b (0x6e,A0,D7w*0x1)
00010460: rts
00010462: dbf D7w,0x0001042e
00010466: neg.w D1w
00010468: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
0001046E: tst.b (0x9c,A0)
00010472: bne.w 0x0001047c
00010476: move.b #-0x1,D1b
0001047A: bra.b 0x00010466
0001047C: rts
0001047E: movem.l {  A0 D5},-(SP)
00010482: clr.l D6
00010484: lea (0xff0100).l,A0 ; data 00ff0100
0001048A: moveq #0x2,D5
0001048C: btst.b #0x0,(0x10,A0)
00010492: beq.w 0x000104b8
00010496: cmpi.b #0x1,(0x9b,A0)
0001049C: bgt.w 0x000104b8
000104A0: jsr 0x00055f02.l ; call 00055F02 FUN_00055f02
000104A6: divu.w #0x4,D1
000104AA: add.w D1w,D6w
000104AC: swap D1
000104AE: cmpi.w #0x3,D1w
000104B2: bmi.w 0x000104b8
000104B6: addq.w #0x1,D6w
000104B8: adda.l #0x100,A0
000104BE: dbf D5w,0x0001048c
000104C2: movem.l (SP)+,{  D5 A0}
000104C6: rts
000104C8: clr.l D0
000104CA: move.b (0x00fff0d1).l,D0b
000104D0: mulu.w #0x6,D0
000104D4: add.b (0x00fff0d0).l,D0b
000104DA: movea.l (0x00fff10e).l,A0
000104E0: lsl.w #0x1,D0w
000104E2: addq.w #0x2,D0w
000104E4: clr.l D5
000104E6: move.b (0x0,A0,D0w*0x1),D5b
000104EA: move.l D5,D7
000104EC: addi.w #0x19,D5w
000104F0: movea.l (0x00fffc04).l,A0
000104F6: move.l D4,-(SP) ; data Stack[-0x4]
000104F8: moveq #0x6,D4
000104FA: cmp.b (0x66,A0,D4w*0x1),D5b
000104FE: bne.w 0x0001050a
00010502: add.b (0x6e,A0,D4w*0x1),D6b
00010506: bra.w 0x0001050e
0001050A: dbf D4w,0x000104fa
0001050E: cmpi.b #0x29,(0x6d,A0)
00010514: beq.w 0x0001052c
00010518: cmpi.b #0x2a,(0x6d,A0)
0001051E: beq.w 0x0001052c
00010522: cmpi.b #0x2b,(0x6d,A0)
00010528: bne.w 0x0001055a
0001052C: lea (0xc8ae0).l,A5 ; data 000c8ae0
00010532: clr.l D4
00010534: move.b (0x0,A5,D7w*0x1),D4b ; data 000c8ae0
00010538: bmi.w 0x0001055a
0001053C: lsl.w #0x1,D4w
0001053E: move.b (0x6d,A0),D5b
00010542: subq.w #0x1,D5w
00010544: mulu.w #0x30,D5
00010548: add.w D5w,D4w
0001054A: lea (0xd4540).l,A5 ; data 000d4540
00010550: add.w (0x22,A5,D4w*0x1),D6w ; data 000d4562
00010554: bgt.w 0x0001055a
00010558: moveq #0x1,D6
0001055A: move.l (SP)+,D4
0001055C: rts
0001055E: move.l D3,-(SP) ; data Stack[-0x4]
00010560: clr.l D6
00010562: tst.w D0w
00010564: bne.w 0x00010598
00010568: jsr 0x00055f02.l ; call 00055F02 FUN_00055f02
0001056E: clr.l D7
00010570: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
00010576: cmp.b (0x7f,A0),D6b
0001057A: ble.w 0x00010582
0001057E: move.b (0x7f,A0),D6b
00010582: add.w (0x00fff096).l,D6w
00010588: bsr.w 0x000104c8 ; call 000104C8 FUN_000104c8
0001058C: move.l #0xc588,D4
00010592: clr.l D5
00010594: bra.w 0x000105ac
00010598: bsr.w 0x0001047e ; call 0001047E FUN_0001047e
0001059C: move.l D6,D5
0001059E: add.b (0x77,A0),D6b
000105A2: add.b (0x7c,A0),D5b
000105A6: move.l #0xc808,D4
000105AC: lea (0xc8784).l,A1 ; data 000c8784
000105B2: move.w #0x0,D2w
000105B6: subq.w #0x1,D6w
000105B8: bpl.w 0x000105be
000105BC: clr.l D6
000105BE: mulu.w #0x1a,D6
000105C2: adda.l D6,A1
000105C4: move.w #0xc,D7w
000105C8: clr.l D1
000105CA: jsr 0x00001232.l ; call 00001232 FUN_00001232
000105D0: tst.w D5w
000105D2: beq.w 0x000105e2
000105D6: move.l D5,D6
000105D8: move.l #0xc908,D4
000105DE: clr.l D5
000105E0: bra.b 0x000105ac
000105E2: move.l (SP)+,D3
000105E4: rts
000105E6: move.l D3,-(SP) ; data Stack[-0x4]
000105E8: clr.l D7
000105EA: move.b (0x00fff0d1).l,D7b
000105F0: mulu.w #0x7,D7
000105F4: add.b (0x00fff0d0).l,D7b
000105FA: movea.l (0x00fff10e).l,A0
00010600: addq.w #0x1,D7w
00010602: lsl.w #0x1,D7w
00010604: move.b (0x0,A0,D7w*0x1),D7b
00010608: clr.l D4
0001060A: move.w (0x00fff096).l,D4w
00010610: movea.l (0x00fffc04).l,A0
00010616: bsr.w 0x00010660 ; call 00010660 FUN_00010660
0001061A: bgt.w 0x00010638
0001061E: lea (0xc8a2c).l,A1 ; data 000c8a2c
00010624: movea.l #0xc68c,A2
0001062A: move.w #0x0,D1w
0001062E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00010634: bra.w 0x0001065c
00010638: subq.w #0x1,D2w
0001063A: mulu.w #0xe,D2
0001063E: lea (0xc86f8).l,A1 ; data 000c86f8
00010644: adda.l D2,A1
00010646: move.l #0xc688,D4
0001064C: move.w #0x6,D7w
00010650: clr.l D1
00010652: move.w #0x0,D2w
00010656: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001065C: move.l (SP)+,D3
0001065E: rts
00010660: move.l D7,-(SP) ; data Stack[-0x4]
00010662: jsr 0x00055f02.l ; call 00055F02 FUN_00055f02
00010668: clr.l D6
0001066A: moveq #-0x1,D7
0001066C: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
00010672: add.b (0x7c,A0),D6b
00010676: move.l (SP)+,D7
00010678: lsr.w #0x1,D4w
0001067A: bcc.w 0x00010680
0001067E: addq.w #0x1,D4w
00010680: lea (0xd4ea0).l,A4 ; data 000d4ea0
00010686: mulu.w #0x30,D7
0001068A: add.b (0x26,A4,D7w*0x1),D4b ; data 000d4ec6
0001068E: cmpi.w #0x2,D4w
00010692: bpl.w 0x00010698
00010696: moveq #0x2,D4
00010698: clr.l D2
0001069A: move.b (0x24,A4,D7w*0x1),D2b ; data 000d4ec4
0001069E: lea (0x166ad4).l,A4 ; data 00166ad4
000106A4: move.b (0x0,A4,D2w*0x1),D2b ; data 00166ad4
000106A8: neg.b D2b
000106AA: moveq #0x7,D0
000106AC: sub.w D4w,D0w
000106AE: bgt.w 0x000106b8
000106B2: moveq #0xa,D0
000106B4: bra.w 0x000106bc
000106B8: mulu.w #0x14,D0
000106BC: mulu.w D0w,D6
000106BE: divu.w #0x64,D6
000106C2: move.l D6,D0
000106C4: swap D0
000106C6: cmpi.w #0x32,D0w
000106CA: bmi.w 0x000106d0
000106CE: addq.w #0x1,D6w
000106D0: ext.l D6
000106D2: divu.w #0x2,D6
000106D6: sub.w D6w,D2w
000106D8: rts
000106DA: bsr.w 0x0001047e ; call 0001047E FUN_0001047e
000106DE: tst.w D2w
000106E0: bne.w 0x000106f2
000106E4: add.b (0x77,A0),D6b
000106E8: move.l #0xc51e,D4
000106EE: bra.w 0x000106fc
000106F2: add.b (0x7c,A0),D6b
000106F6: move.l #0xc69e,D4
000106FC: lea (0xcf610).l,A1 ; data 000cf610
00010702: move.w #0x2000,D2w
00010706: subq.w #0x1,D6w
00010708: bpl.w 0x0001070e
0001070C: clr.l D6
0001070E: mulu.w #0x1a,D6
00010712: adda.l D6,A1
00010714: move.w #0xc,D7w
00010718: clr.l D1
0001071A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010720: rts
00010722: clr.l D0
00010724: lea (0xc8600).l,A1 ; data 000c8600
0001072A: bsr.w 0x000107ce ; call 000107CE FUN_000107ce
0001072E: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00010734: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001073A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00010740: jsr 0x0000fdfe.l ; call 0000FDFE FUN_0000fdfe
00010746: btst.b #0x8,(0x00ffdd22).l
0001074E: beq.w 0x00010760
00010752: clr.l D0
00010754: lea (0xc85b0).l,A1 ; data 000c85b0
0001075A: bsr.w 0x000107ce ; call 000107CE FUN_000107ce
0001075E: rts
00010760: btst.b #0xb,(0x00ffdd22).l
00010768: beq.w 0x0001077a
0001076C: moveq #0x1,D0
0001076E: lea (0xc8600).l,A1 ; data 000c8600
00010774: bsr.w 0x000107ce ; call 000107CE FUN_000107ce
00010778: bra.b 0x0001072e
0001077A: btst.b #0xa,(0x00ffdd22).l
00010782: beq.w 0x00010794
00010786: moveq #-0x1,D0
00010788: lea (0xc8600).l,A1 ; data 000c8600
0001078E: bsr.w 0x000107ce ; call 000107CE FUN_000107ce
00010792: bra.b 0x0001072e
00010794: btst.b #0xe,(0x00ffdd22).l
0001079C: beq.w 0x000107a4
000107A0: bra.w 0x000107c0
000107A4: btst.b #0xc,(0x00ffdd22).l
000107AC: beq.w 0x000107b4
000107B0: bra.w 0x000107c0
000107B4: btst.b #0xd,(0x00ffdd22).l
000107BC: beq.w 0x0001072e
000107C0: clr.l D0
000107C2: lea (0xc85b0).l,A1 ; data 000c85b0
000107C8: bsr.w 0x000107ce ; call 000107CE FUN_000107ce
000107CC: rts
000107CE: movea.l (0x00fffc04).l,A0
000107D4: add.b D0b,(0x4d,A0)
000107D8: bpl.w 0x000107e4
000107DC: clr.b (0x4d,A0)
000107E0: bra.w 0x000107f4
000107E4: cmpi.b #0x4,(0x4d,A0)
000107EA: ble.w 0x000107f4
000107EE: move.b #0x4,(0x4d,A0)
000107F4: clr.l D0
000107F6: move.b (0x4d,A0),D0b
000107FA: mulu.w #0x10,D0
000107FE: adda.l D0,A1
00010800: move.w #-0x3758,D4w
00010804: move.w #0x7,D7w
00010808: move.w #0x0,D1w
0001080C: move.w #0x0,D2w
00010810: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010816: lea (0xd1a05).l,A1 ; data 000d1a05
0001081C: clr.l D0
0001081E: move.b (0x4d,A0),D0b
00010822: mulu.w #0xd,D0
00010826: adda.l D0,A1
00010828: movea.l #0xc924,A2
0001082E: move.w #0x0,D1w
00010832: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00010838: bsr.w 0x00010854 ; call 00010854 FUN_00010854
0001083C: bsr.w 0x000105e6 ; call 000105E6 FUN_000105e6
00010840: clr.l D0
00010842: bsr.w 0x0001055e ; call 0001055E FUN_0001055e
00010846: movea.l (0x00fffc04).l,A0
0001084C: moveq #-0x1,D0
0001084E: bsr.w 0x0001055e ; call 0001055E FUN_0001055e
00010852: rts
00010854: move.l #0xc588,D4
0001085A: lea (0xc1320).l,A1 ; data 000c1320
00010860: move.w #0x0,D2w
00010864: moveq #0xd,D7
00010866: clr.l D1
00010868: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0001086E: move.l #0xc808,D4
00010874: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0001087A: move.l #0xc908,D4
00010880: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00010886: bsr.w 0x00010156 ; call 00010156 FUN_00010156
0001088A: rts
0001088C: clr.l D7
0001088E: clr.l D0
00010890: move.b (0x00fff0d1).l,D0b
00010896: mulu.w #0x7,D0
0001089A: add.b (0x00fff0d0).l,D0b
000108A0: movea.l (0x00fff10e).l,A0
000108A6: addq.w #0x1,D0w
000108A8: lsl.w #0x1,D0w
000108AA: move.b (0x0,A0,D0w*0x1),D7b
000108AE: lea (0xd4ea0).l,A4 ; data 000d4ea0
000108B4: move.l D7,D6
000108B6: mulu.w #0x30,D6
000108BA: move.b (0x25,A4,D6w*0x1),D0b ; data 000d4ec5
000108BE: bpl.w 0x000108de
000108C2: move.l #0xc528,D4
000108C8: lea (0xc1320).l,A1 ; data 000c1320
000108CE: move.w #0x0,D2w
000108D2: moveq #0x5,D7
000108D4: clr.l D1
000108D6: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000108DC: rts
000108DE: lea (0xc8a32).l,A1 ; data 000c8a32
000108E4: movea.l #0xc528,A2
000108EA: move.w #0x0,D1w
000108EE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000108F4: mulu.w #0xe,D0
000108F8: lea (0xc86c0).l,A1 ; data 000c86c0
000108FE: adda.l D0,A1
00010900: move.l #0xc5a8,D4
00010906: move.w #0x6,D7w
0001090A: clr.l D1
0001090C: move.w #0x0,D2w
00010910: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010916: rts
00010918: move.w #-0x7ecc,(0x00c00004).l
00010920: bsr.w 0x00010952 ; call 00010952 FUN_00010952
00010924: lea (0xd29d6).l,A0 ; data 000d29d6
0001092A: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
00010930: bset.b #0x0,(0x00ffef14).l
00010938: lea (0xffbf40).l,A1 ; data 00ffbf40
0001093E: movea.l #0xcab0c,A3
00010944: move.l #0x1e20,D0
0001094A: jsr 0x00001530.l ; call 00001530 FUN_00001530
00010950: rts
00010952: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
00010958: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0001095E: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00010964: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
0001096A: clr.l (0x00ffdd08).l
00010970: clr.l (0x00ffdd0c).l
00010976: jsr 0x00000f1c.l ; call 00000F1C FUN_00000f1c
0001097C: jsr 0x00000f3c.l ; call 00000F3C FUN_00000f3c
00010982: rts
00010984: move.w #-0x7ecc,(0x00c00004).l
0001098C: bsr.b 0x00010918 ; call 00010918 FUN_00010918
0001098E: lea (0xcecd8).l,A1 ; data 000cecd8
00010994: move.w #-0x2000,D4w
00010998: move.w #0x13,D7w
0001099C: move.w #0x1,D1w
000109A0: move.w #0x0,D2w
000109A4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000109AA: lea (0xced28).l,A1 ; data 000ced28
000109B0: move.w #-0x1fd8,D4w
000109B4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000109BA: lea (0xced78).l,A1 ; data 000ced78
000109C0: move.w #-0x1f00,D4w
000109C4: move.w #0x9,D7w
000109C8: move.w #0xe,D1w
000109CC: jsr 0x00001232.l ; call 00001232 FUN_00001232
000109D2: lea (0xceea4).l,A1 ; data 000ceea4
000109D8: move.w #-0x1eb4,D4w
000109DC: move.w #0x1,D7w
000109E0: move.w #0xe,D1w
000109E4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000109EA: lea (0xceee0).l,A1 ; data 000ceee0
000109F0: move.w #-0x1780,D4w
000109F4: move.w #0x13,D7w
000109F8: move.w #0x1,D1w
000109FC: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010A02: lea (0xcef30).l,A1 ; data 000cef30
00010A08: move.w #-0x1758,D4w
00010A0C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010A12: lea (0xcef80).l,A1 ; data 000cef80
00010A18: move.w #-0x1680,D4w
00010A1C: move.w #0x0,D7w
00010A20: move.w #0x8,D1w
00010A24: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010A2A: lea (0xcef92).l,A1 ; data 000cef92
00010A30: move.w #-0x1632,D4w
00010A34: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010A3A: lea (0xcefa4).l,A1 ; data 000cefa4
00010A40: move.w #-0x127e,D4w
00010A44: move.w #0x25,D7w
00010A48: move.w #0x0,D1w
00010A4C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010A52: tst.b (0x00fff0af).l
00010A58: beq.w 0x00010a74
00010A5C: lea (0xceff0).l,A1 ; data 000ceff0
00010A62: move.w #-0x1f00,D4w
00010A66: move.w #0x9,D7w
00010A6A: move.w #0xe,D1w
00010A6E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010A74: move.w #-0x7e8c,(0x00c00004).l
00010A7C: rts
00010A7E: bsr.w 0x00010918 ; call 00010918 FUN_00010918
00010A82: move.w #-0x7ecc,(0x00c00004).l
00010A8A: lea (0xcda0e).l,A1 ; data 000cda0e
00010A90: move.w #-0x2000,D4w
00010A94: move.w #0x21,D7w
00010A98: move.w #0x2,D1w
00010A9C: move.w #0x0,D2w
00010AA0: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010AA6: lea (0xcdada).l,A1 ; data 000cdada
00010AAC: move.w #-0x1e80,D4w
00010AB0: move.w #0x2,D7w
00010AB4: move.w #0x14,D1w
00010AB8: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010ABE: lea (0xcdb58).l,A1 ; data 000cdb58
00010AC4: move.w #-0x1e42,D4w
00010AC8: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010ACE: lea (0xcdbd6).l,A1 ; data 000cdbd6
00010AD4: move.w #-0x1400,D4w
00010AD8: move.w #0x21,D7w
00010ADC: move.w #0x3,D1w
00010AE0: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010AE6: move.w #-0x7e8c,(0x00c00004).l
00010AEE: rts
00010AF0: bsr.w 0x00010918 ; call 00010918 FUN_00010918
00010AF4: lea (0xcdce6).l,A1 ; data 000cdce6
00010AFA: move.w #-0x2000,D4w
00010AFE: move.w #0x21,D7w
00010B02: move.w #0x3,D1w
00010B06: move.w #0x0,D2w
00010B0A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010B10: lea (0xcddf6).l,A1 ; data 000cddf6
00010B16: move.w #-0x1e00,D4w
00010B1A: move.w #0x4,D7w
00010B1E: move.w #0x10,D1w
00010B22: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010B28: lea (0xcdea0).l,A1 ; data 000cdea0
00010B2E: move.w #-0x1dc6,D4w
00010B32: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010B38: lea (0xcdf4a).l,A1 ; data 000cdf4a
00010B3E: move.w #-0x1580,D4w
00010B42: move.w #0x21,D7w
00010B46: move.w #0x6,D1w
00010B4A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010B50: move.w #-0x7e8c,(0x00c00004).l
00010B58: rts
00010B5A: bsr.w 0x00010918 ; call 00010918 FUN_00010918
00010B5E: lea (0xcdce6).l,A1 ; data 000cdce6
00010B64: move.w #-0x1ffa,D4w
00010B68: move.w #0x21,D7w
00010B6C: move.w #0x3,D1w
00010B70: move.w #0x0,D2w
00010B74: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010B7A: lea (0xcddf6).l,A1 ; data 000cddf6
00010B80: move.w #-0x1dfa,D4w
00010B84: move.w #0x4,D7w
00010B88: move.w #0x10,D1w
00010B8C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010B92: lea (0xcdea0).l,A1 ; data 000cdea0
00010B98: move.w #-0x1dc0,D4w
00010B9C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010BA2: lea (0xcdf4a).l,A1 ; data 000cdf4a
00010BA8: move.w #-0x157a,D4w
00010BAC: move.w #0x21,D7w
00010BB0: move.w #0x6,D1w
00010BB4: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010BBA: move.w #-0x7e8c,(0x00c00004).l
00010BC2: rts
00010BC4: bsr.w 0x00010918 ; call 00010918 FUN_00010918
00010BC8: move.l #0xd263c,D0
00010BCE: move.l #0x9f40,D1
00010BD4: move.l #0x340,D2
00010BDA: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00010BE0: lea (0xce2a6).l,A1 ; data 000ce2a6
00010BE6: move.w #-0x2000,D4w
00010BEA: move.w #0x21,D7w
00010BEE: move.w #0x1,D1w
00010BF2: move.w #0x0,D2w
00010BF6: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010BFC: lea (0xce32e).l,A1 ; data 000ce32e
00010C02: move.w #-0x1f00,D4w
00010C06: move.w #0x1,D7w
00010C0A: move.w #0xc,D1w
00010C0E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C14: lea (0xce362).l,A1 ; data 000ce362
00010C1A: move.w #-0x1ebe,D4w
00010C1E: move.w #0x0,D7w
00010C22: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C28: lea (0xce37c).l,A1 ; data 000ce37c
00010C2E: move.w #-0x1880,D4w
00010C32: move.w #0x21,D7w
00010C36: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C3C: lea (0xce6f0).l,A1 ; data 000ce6f0
00010C42: move.w #-0x3fbc,D4w
00010C46: move.w #0x5,D7w
00010C4A: move.w #0xd,D1w
00010C4E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C54: lea (0xce798).l,A1 ; data 000ce798
00010C5A: move.w #-0x18bc,D4w
00010C5E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C64: lea (0xce840).l,A1 ; data 000ce840
00010C6A: move.w #-0x1d3a,D4w
00010C6E: move.w #0x3,D7w
00010C72: move.w #0x0,D1w
00010C76: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C7C: lea (0xce848).l,A1 ; data 000ce848
00010C82: move.w #-0x1aba,D4w
00010C86: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010C8C: move.w #-0x7e8c,(0x00c00004).l
00010C94: rts
00010C96: bsr.w 0x00010918 ; call 00010918 FUN_00010918
00010C9A: lea (0xce850).l,A1 ; data 000ce850
00010CA0: move.w #-0x2000,D4w
00010CA4: move.w #0x13,D7w
00010CA8: move.w #0x6,D1w
00010CAC: move.w #0x0,D2w
00010CB0: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010CB6: lea (0xce968).l,A1 ; data 000ce968
00010CBC: move.w #-0x1fd8,D4w
00010CC0: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010CC6: lea (0xcea80).l,A1 ; data 000cea80
00010CCC: move.w #-0x1c80,D4w
00010CD0: move.w #0x1,D7w
00010CD4: move.w #0x14,D1w
00010CD8: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010CDE: lea (0xcead4).l,A1 ; data 000cead4
00010CE4: move.w #-0x1c34,D4w
00010CE8: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010CEE: lea (0xceb28).l,A1 ; data 000ceb28
00010CF4: move.w #-0x1a7c,D4w
00010CF8: move.w #0x23,D7w
00010CFC: move.w #0x1,D1w
00010D00: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010D06: lea (0xcebb8).l,A1 ; data 000cebb8
00010D0C: move.w #-0x167c,D4w
00010D10: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010D16: lea (0xcec48).l,A1 ; data 000cec48
00010D1C: move.w #-0x12fc,D4w
00010D20: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010D26: move.w #-0x7e8c,(0x00c00004).l
00010D2E: rts
00010D30: movea.l (0x00fffc04).l,A0
00010D36: btst.b #0x6,(0x10,A0)
00010D3C: bne.w 0x00010d4a
00010D40: btst.b #0x4,(0x10,A0)
00010D46: beq.w 0x00010d50
00010D4A: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
00010D50: move.b #-0x1,(0x00fff09d).l
00010D58: clr.b (0x00fff0b1).l
00010D5E: clr.w (0x00fff112).l
00010D64: move.w #-0x7ecc,(0x00c00004).l
00010D6C: clr.b (0x00fff0e7).l
00010D72: clr.b (0x00fff100).l
00010D78: bsr.w 0x00010f50 ; call 00010F50 FUN_00010f50
00010D7C: jsr 0x0005670e.l ; call 0005670E FUN_0005670e
00010D82: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
00010D88: move.w #-0x7e8c,(0x00c00004).l
00010D90: clr.b (0x00fff0e0).l
00010D96: bsr.w 0x000110bc ; call 000110BC FUN_000110bc
00010D9A: bsr.w 0x0001111a ; call 0001111A FUN_0001111a
00010D9E: bsr.w 0x00011928 ; call 00011928 FUN_00011928
00010DA2: bsr.w 0x00011624 ; call 00011624 FUN_00011624
00010DA6: movea.l #0xcd2c6,A1
00010DAC: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00010DB0: movea.l #0xcd3f2,A1
00010DB6: bsr.w 0x00011454 ; call 00011454 FUN_00011454
00010DBA: move.b #-0x1,(0x00fff112).l
00010DC2: bra.w 0x00010e2e
00010DC6: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00010DCC: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00010DD2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00010DD8: btst.b #0xa,(0x00ffdd22).l
00010DE0: beq.w 0x00010dea
00010DE4: moveq #-0x1,D0
00010DE6: bra.w 0x00010e0a
00010DEA: btst.b #0x8,(0x00ffdd22).l
00010DF2: beq.w 0x00010dfc
00010DF6: moveq #-0x1,D0
00010DF8: bra.w 0x00010e20
00010DFC: btst.b #0xb,(0x00ffdd22).l
00010E04: beq.w 0x00010e12
00010E08: moveq #0x1,D0
00010E0A: bsr.w 0x000114a0 ; call 000114A0 FUN_000114a0
00010E0E: bra.w 0x00010e2e
00010E12: btst.b #0x9,(0x00ffdd22).l
00010E1A: beq.w 0x00010e42
00010E1E: moveq #0x1,D0
00010E20: bsr.w 0x00011506 ; call 00011506 FUN_00011506
00010E24: tst.b (0x00fff0b1).l
00010E2A: bne.w 0x00010f22
00010E2E: move.w #0x25,D0w
00010E32: jsr 0x000530de.l ; call 000530DE FUN_000530de
00010E38: bsr.w 0x0001170c ; call 0001170C FUN_0001170c
00010E3C: bsr.w 0x000111e8 ; call 000111E8 FUN_000111e8
00010E40: bra.b 0x00010dc6
00010E42: btst.b #0xe,(0x00ffdd22).l
00010E4A: beq.w 0x00010ec8
00010E4E: move.w #0x24,D0w
00010E52: jsr 0x000530de.l ; call 000530DE FUN_000530de
00010E58: move.w (0x00fff0d0).l,-(SP)
00010E5E: bsr.w 0x00011740 ; call 00011740 FUN_00011740
00010E62: move.w (SP)+,(0x00fff0d0).l
00010E68: cmpi.b #0x2,(0x00fff0d0).l
00010E70: blt.w 0x00010ec0
00010E74: cmpi.b #0x3,(0x00fff0d0).l
00010E7C: beq.w 0x00010d30
00010E80: cmpi.b #0x6,(0x00fff0d0).l
00010E88: bne.w 0x00010ea0
00010E8C: tst.b (0x00fff0cd).l
00010E92: beq.w 0x00010d30
00010E96: clr.b (0x00fff0cd).l
00010E9C: bra.w 0x00010f22
00010EA0: movea.l (0x00fffc04).l,A0
00010EA6: cmpi.b #0x1,(0x9b,A0)
00010EAC: bgt.w 0x00010d90
00010EB0: cmpi.b #0x5,(0x00fff0d0).l
00010EB8: beq.w 0x00010d30
00010EBC: bra.w 0x00010d90
00010EC0: bsr.w 0x00011592 ; call 00011592 FUN_00011592
00010EC4: bra.w 0x00010dc6
00010EC8: btst.b #0xc,(0x00ffdd22).l
00010ED0: beq.w 0x00010ed8
00010ED4: bra.w 0x00010e4e
00010ED8: btst.b #0xd,(0x00ffdd22).l
00010EE0: beq.w 0x00010f16
00010EE4: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
00010EEA: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00010EF0: move.b #0x6,(0x00fff0d0).l
00010EF8: clr.b (0x00fff0d1).l
00010EFE: clr.l D0
00010F00: bsr.w 0x000114a0 ; call 000114A0 FUN_000114a0
00010F04: clr.l D0
00010F06: move.b #-0x1,(0x00fff115).l
00010F0E: bsr.w 0x00011506 ; call 00011506 FUN_00011506
00010F12: bra.w 0x00010d90
00010F16: btst.b #0xf,(0x00ffdd22).l
00010F1E: beq.w 0x00010dc6
00010F22: movea.l (0x00fffc04).l,A0
00010F28: btst.b #0x6,(0x10,A0)
00010F2E: bne.w 0x00010f3c
00010F32: btst.b #0x4,(0x10,A0)
00010F38: beq.w 0x00010f42
00010F3C: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
00010F42: clr.b (0x00fff09d).l
00010F48: clr.b (0x00fff0b1).l
00010F4E: rts
00010F50: move.b #0x6,(0x00fff0d0).l
00010F58: clr.b (0x00fff0d1).l
00010F5E: clr.w (0x00fff098).l
00010F64: clr.w (0x00fff0dc).l
00010F6A: bsr.w 0x00010918 ; call 00010918 FUN_00010918
00010F6E: lea (0xcd598).l,A1 ; data 000cd598
00010F74: move.w #-0x2000,D4w
00010F78: move.w #0xb,D7w
00010F7C: move.w #0x1b,D1w
00010F80: move.w #0x0,D2w
00010F84: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010F8A: lea (0xcd838).l,A1 ; data 000cd838
00010F90: move.w #-0x1fe8,D4w
00010F94: move.w #0x14,D7w
00010F98: move.w #0x1,D1w
00010F9C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010FA2: lea (0xcd88c).l,A1 ; data 000cd88c
00010FA8: move.w #-0x1fbe,D4w
00010FAC: move.w #0x0,D7w
00010FB0: move.w #0x16,D1w
00010FB4: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010FBA: lea (0xcd8ba).l,A1 ; data 000cd8ba
00010FC0: move.w #-0x1468,D4w
00010FC4: move.w #0x15,D7w
00010FC8: move.w #0x4,D1w
00010FCC: jsr 0x00001232.l ; call 00001232 FUN_00001232
00010FD2: bsr.w 0x00010ff4 ; call 00010FF4 FUN_00010ff4
00010FD6: bsr.w 0x00011048 ; call 00011048 FUN_00011048
00010FDA: move.l #0xcf134,D0
00010FE0: move.l #0x5a80,D1
00010FE6: move.l #0x200,D2
00010FEC: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00010FF2: rts
00010FF4: lea (0xff0100).l,A0 ; data 00ff0100
00010FFA: moveq #0x2,D5
00010FFC: moveq #0x20,D1
00010FFE: moveq #0x20,D2
00011000: btst.b #0x0,(0x10,A0)
00011006: bne.w 0x00011014
0001100A: lea (0x720fa).l,A2 ; data 000720fa
00011010: bra.w 0x0001101a
00011014: move.l (0x50,A0),D6
00011018: movea.l D6,A2
0001101A: moveq #0x4f,D6
0001101C: clr.l D0
0001101E: move.w (A2)+,D0w
00011020: asl.l #0x5,D0
00011022: addi.l #0x6acfa,D0
00011028: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0001102E: addi.w #0x20,D1w
00011032: dbf D6w,0x0001101c
00011036: adda.l #0x100,A0
0001103C: dbf D5w,0x00011000
00011040: clr.b (0x00fff112).l
00011046: rts
00011048: bsr.w 0x0001107e ; call 0001107E FUN_0001107e
0001104C: move.l #0xcf97c,D0
00011052: move.l #0x7d20,D1
00011058: move.l #0x1e00,D2
0001105E: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00011064: move.l #0xd21bc,D0
0001106A: move.l #0x9b20,D1
00011070: move.l #0x420,D2
00011076: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0001107C: rts
0001107E: tst.b (0x00fff100).l
00011084: bne.w 0x000110a2
00011088: lea (0xffbf40).l,A1 ; data 00ffbf40
0001108E: movea.l #0xc8bb6,A3
00011094: move.l #0x5c80,D0
0001109A: jsr 0x00001530.l ; call 00001530 FUN_00001530
000110A0: rts
000110A2: lea (0xffbf40).l,A1 ; data 00ffbf40
000110A8: movea.l #0xc9b7c,A3
000110AE: move.l #0x5c80,D0
000110B4: jsr 0x00001530.l ; call 00001530 FUN_00001530
000110BA: rts
000110BC: movea.l (0x00fffc04).l,A0
000110C2: move.w (0x3e,A0),D5w
000110C6: mulu.w #0x50,D5
000110CA: addi.w #-0x7fff,D5w
000110CE: move.l #0xe104,D4
000110D4: move.w #0x7,D7w
000110D8: move.w #0x9,D1w
000110DC: move.w (0x2c,A0),D2w
000110E0: jsr 0x00003648.l ; call 00003648 FUN_00003648
000110E6: rts
000110E8: move.w (0x2c,A0),D2w
000110EC: move.w (0x3e,A0),D1w
000110F0: subq.w #0x1,D1w
000110F2: bgt.w 0x0001110a
000110F6: beq.w 0x00011102
000110FA: move.w #-0x796c,D5w
000110FE: bra.w 0x0001110e
00011102: move.w #-0x7956,D5w
00011106: bra.w 0x0001110e
0001110A: move.w #-0x7940,D5w
0001110E: moveq #0x3,D7
00011110: moveq #0x3,D1
00011112: jsr 0x00003648.l ; call 00003648 FUN_00003648
00011118: rts
0001111A: movea.l (0x00fffc04).l,A0
00011120: movea.l #0xc0a0,A2
00011126: lea (0x58,A0),A1
0001112A: move.w #0x6000,D1w
0001112E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011134: rts
00011136: move.w #-0x387c,D4w
0001113A: moveq #0x7,D3
0001113C: lea (0xd4540).l,A4 ; data 000d4540
00011142: movea.l (0x00fffc04).l,A0
00011148: clr.l D0
0001114A: move.b (0x66,A0),D0b
0001114E: bne.w 0x0001115c
00011152: lea (0x34f8).l,A1 ; data 000034f8
00011158: bra.w 0x00011166
0001115C: subq.w #0x1,D0w
0001115E: mulu.w #0x30,D0
00011162: movea.l (0x1a,A4,D0w*0x1),A1 ; data 000d455a
00011166: jsr 0x00003602.l ; call 00003602 FUN_00003602
0001116C: bsr.w 0x00011190 ; call 00011190 FUN_00011190
00011170: addi.l #0xa,D4
00011176: addq.l #0x1,A0
00011178: move.l D3,D0
0001117A: lsr.w #0x1,D0w
0001117C: bcs.w 0x0001118a
00011180: addi.w #0x180,D4w
00011184: subi.l #0x14,D4
0001118A: dbf D3w,0x00011148
0001118E: rts
00011190: movem.l {  A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00011194: move SR,-(SP) ; data Stack[-0x2e]
00011196: move #0x2700,SR
0001119A: lea (0xc00000).l,A2 ; data 00c00000
000111A0: move.l D4,D0
000111A2: move.w #0x1,D1w
000111A6: move.w #0x2,D7w
000111AA: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
000111B0: move.w (A1)+,D0w
000111B2: addi.w #0x3e9,D0w
000111B6: or.w D6w,D0w
000111B8: move.w D0w,(A2)
000111BA: dbf D7w,0x000111b0
000111BE: move.l D4,D0
000111C0: addi.w #0x80,D0w
000111C4: dbf D1w,0x000111a6
000111C8: move (SP)+,SR
000111CA: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2}
000111CE: rts
000111D0: move.w #-0x1462,D4w
000111D4: move.w #0xe,D7w
000111D8: move.w #0x1,D1w
000111DC: move.w #0x0,D2w
000111E0: jsr 0x00001232.l ; call 00001232 FUN_00001232
000111E6: rts
000111E8: tst.b (0x00fff112).l
000111EE: beq.w 0x0001134c
000111F2: clr.b (0x00fff112).l
000111F8: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
000111FC: lea (0xd198c).l,A1 ; data 000d198c
00011202: movea.l #0xc2a0,A2
00011208: move.w #0x6000,D1w
0001120C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011212: lea (0xd19c7).l,A1 ; data 000d19c7
00011218: movea.l (0x00fffc04).l,A0
0001121E: adda.l #0x9a,A0
00011224: clr.l D0
00011226: move.b (A0),D0b
00011228: mulu.w #0x6,D0
0001122C: adda.l D0,A1
0001122E: movea.l #0xc1a0,A2
00011234: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001123A: lea (0xd19e5).l,A1 ; data 000d19e5
00011240: movea.l (0x00fffc04).l,A0
00011246: adda.l #0x9b,A0
0001124C: move.b (A0),D0b
0001124E: mulu.w #0x8,D0
00011252: adda.l D0,A1
00011254: movea.l #0xc1ac,A2
0001125A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011260: movea.l (0x00fffc04).l,A0
00011266: clr.l D5
00011268: move.b (0x57,A0),D5b
0001126C: bne.w 0x000112c6
00011270: move.w (0x90,A0),D0w
00011274: btst.l #0x3,D0
00011278: beq.w 0x00011292
0001127C: lea (0xd21a6).l,A1 ; data 000d21a6
00011282: movea.l #0xc422,A2
00011288: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001128E: bra.w 0x00011304
00011292: btst.l #0x2,D0
00011296: beq.w 0x000112b0
0001129A: lea (0xd21af).l,A1 ; data 000d21af
000112A0: movea.l #0xc422,A2
000112A6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000112AC: bra.w 0x00011304
000112B0: lea (0xd1a7e).l,A1 ; data 000d1a7e
000112B6: movea.l #0xc420,A2
000112BC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000112C2: bra.w 0x00011304
000112C6: subq.w #0x1,D5w
000112C8: mulu.w #0x30,D5
000112CC: tst.b (0x56,A0)
000112D0: bpl.w 0x000112ee
000112D4: lea (0xd4ea0).l,A4 ; data 000d4ea0
000112DA: adda.l D5,A4
000112DC: lea (A4),A1
000112DE: movea.l #0xc420,A2
000112E4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000112EA: bra.w 0x0001130e
000112EE: lea (0xd4540).l,A4 ; data 000d4540
000112F4: adda.l D5,A4
000112F6: lea (A4),A1
000112F8: movea.l #0xc420,A2
000112FE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011304: cmpi.b #0x10,(0x57,A0)
0001130A: bpl.w 0x00011312
0001130E: bsr.w 0x00012eae ; call 00012EAE FUN_00012eae
00011312: clr.l D1
00011314: move.b (0x4c,A0),D1b
00011318: movea.l #0xc2b4,A2
0001131E: move.w #0x6000,D2w
00011322: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00011328: clr.l D1
0001132A: move.b (0x9c,A0),D1b
0001132E: movea.l #0xc334,A2
00011334: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0001133A: bsr.w 0x00011352 ; call 00011352 FUN_00011352
0001133E: clr.l D0
00011340: lea (0xcf384).l,A1 ; data 000cf384
00011346: bsr.w 0x00011abe ; call 00011ABE FUN_00011abe
0001134A: rts
0001134C: bsr.w 0x00011928 ; call 00011928 FUN_00011928
00011350: rts
00011352: move.w #0x6000,D1w
00011356: tst.b (0x00fff0b1).l
0001135C: bmi.w 0x0001136e
00011360: clr.l D0
00011362: move.b (0x76,A0),D0b
00011366: beq.w 0x0001142e
0001136A: bra.w 0x0001138a
0001136E: subi.b #0x2b,D0b
00011372: move.b (0x76,A0),(0x00ffef96).l
0001137A: move.b D0b,(0x76,A0)
0001137E: move.b (0x6e,A0,D7w*0x1),(0x00ffef97).l
00011386: move.b D0b,(0x6e,A0,D7w*0x1)
0001138A: subq.w #0x1,D0w
0001138C: lea (0xd4d50).l,A4 ; data 000d4d50
00011392: mulu.w #0x30,D0
00011396: lea (0x0,A4,D0w*0x1),A1
0001139A: movea.l #0xc820,A2
000113A0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000113A6: lea (0xd1f4e).l,A1 ; data 000d1f4e
000113AC: movea.l #0xc922,A2
000113B2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000113B8: move.l D7,D6
000113BA: move.w #0x2000,D2w
000113BE: movea.l A0,A1
000113C0: jsr 0x00055eb4.l ; call 00055EB4 FUN_00055eb4
000113C6: mulu.w #0xc,D0
000113CA: lea (0xcf884).l,A1 ; data 000cf884
000113D0: adda.l D0,A1
000113D2: move.w #0x5,D7w
000113D6: clr.l D1
000113D8: move.l #0xc92c,D4
000113DE: jsr 0x00001232.l ; call 00001232 FUN_00001232
000113E4: movea.l A0,A1
000113E6: jsr 0x00055e92.l ; call 00055E92 FUN_00055e92
000113EC: mulu.w #0xc,D0
000113F0: lea (0xcf884).l,A1 ; data 000cf884
000113F6: adda.l D0,A1
000113F8: move.w #0x5,D7w
000113FC: clr.l D1
000113FE: move.l #0xc9ac,D4
00011404: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001140A: move.l D6,D7
0001140C: tst.b (0x00fff0b1).l
00011412: bpl.w 0x0001142c
00011416: clr.b (0x00fff0b1).l
0001141C: move.b (0x00ffef96).l,(0x76,A0)
00011424: move.b (0x00ffef97).l,(0x6e,A0,D7w*0x1)
0001142C: rts
0001142E: lea (0xd1f45).l,A1 ; data 000d1f45
00011434: movea.l #0xc8a6,A2
0001143A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011440: rts
00011442: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00011448: lea (0xcd2c6).l,A1 ; data 000cd2c6
0001144E: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011452: rts
00011454: clr.l D4
00011456: move.b (0x00fff0d0).l,D4b
0001145C: subq.w #0x2,D4w
0001145E: mulu.w #0x8,D4
00011462: adda.l D4,A1
00011464: move.b (0x00fff0d0).l,D4b
0001146A: subq.w #0x2,D4w
0001146C: mulu.w #0x6,D4
00011470: tst.w (0x00fff116).l
00011476: bmi.w 0x00011484
0001147A: addi.l #0xeba0,D4
00011480: bra.w 0x0001148a
00011484: addi.l #0xeba2,D4
0001148A: clr.w (0x00fff116).l
00011490: moveq #0x1,D7
00011492: moveq #0x1,D1
00011494: move.w #0x0,D2w
00011498: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001149E: rts
000114A0: bsr.b 0x00011442 ; call 00011442 FUN_00011442
000114A2: add.b D0b,(0x00fff0d0).l
000114A8: bpl.w 0x000114bc
000114AC: move.b #-0x1,(0x00fff112).l
000114B4: move.b #0x6,(0x00fff0d0).l
000114BC: cmpi.b #0x7,(0x00fff0d0).l
000114C4: bne.w 0x000114d0
000114C8: move.b #0x0,(0x00fff0d0).l
000114D0: cmpi.b #0x2,(0x00fff0d0).l
000114D8: bne.w 0x000114ea
000114DC: tst.b D0b
000114DE: bmi.w 0x000114ea
000114E2: move.b #-0x1,(0x00fff112).l
000114EA: cmpi.b #0x2,(0x00fff0d0).l
000114F2: bmi.w 0x00011592
000114F6: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
000114FC: movea.l #0xcd3f2,A1
00011502: bra.w 0x00011454 ; call 00011454 FUN_00011454
00011506: cmpi.b #0x2,(0x00fff0d0).l
0001150E: bmi.w 0x00011564
00011512: clr.l D0
00011514: move.w (0x00ffdd22).l,D0w
0001151A: btst.l #0x1,D0
0001151E: bne.w 0x00011542
00011522: btst.l #0x3,D0
00011526: bne.w 0x00011542
0001152A: btst.l #0x2,D0
0001152E: bne.w 0x00011542
00011532: tst.b (0x00fff115).l
00011538: bpl.w 0x00011544
0001153C: clr.b (0x00fff115).l
00011542: rts
00011544: bsr.w 0x00011442 ; call 00011442 FUN_00011442
00011548: move.b #-0x1,(0x00fff115).l
00011550: bsr.w 0x000119ce ; call 000119CE FUN_000119ce
00011554: clr.b (0x00fff115).l
0001155A: movea.l #0xcd3f2,A1
00011560: bra.w 0x00011454 ; call 00011454 FUN_00011454
00011564: bsr.w 0x00011442 ; call 00011442 FUN_00011442
00011568: add.b D0b,(0x00fff0d1).l
0001156E: bpl.w 0x0001157e
00011572: move.b #0x0,(0x00fff0d1).l
0001157A: bra.w 0x00011592 ; call 00011592 FUN_00011592
0001157E: cmpi.b #0x4,(0x00fff0d1).l
00011586: bne.w 0x00011592
0001158A: move.b #0x3,(0x00fff0d1).l
00011592: clr.l D1
00011594: move.b (0x00fff0d0).l,D1b
0001159A: mulu.w #0x28,D1
0001159E: clr.l D2
000115A0: move.b (0x00fff0d1).l,D2b
000115A6: mulu.w #0x18,D2
000115AA: move.w #0x2,D5w
000115AE: move.w #0x1,(0x00ffdd02).l
000115B6: lea (0xffd986).l,A3 ; data 00ffd986
000115BC: lea (0xd29f6).l,A2 ; data 000d29f6
000115C2: move.w #0x2000,D6w
000115C6: jsr 0x00001378.l ; call 00001378 FUN_00001378
000115CC: subq.w #0x8,A3
000115CE: clr.b (0x3,A3)
000115D2: move.w #0x1,(0x00ffd980).l
000115DA: bset.b #0x1,(0x00ffef14).l
000115E2: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
000115E6: bsr.w 0x00012148 ; call 00012148 FUN_00012148
000115EA: beq.w 0x0001160c
000115EE: bsr.w 0x00012a8c ; call 00012A8C FUN_00012a8c
000115F2: cmpi.b #0x10,D0b
000115F6: bpl.w 0x000115fe
000115FA: bsr.w 0x00012e9e ; call 00012E9E FUN_00012e9e
000115FE: clr.l D0
00011600: lea (0xcf384).l,A1 ; data 000cf384
00011606: bsr.w 0x00011abe ; call 00011ABE FUN_00011abe
0001160A: rts
0001160C: movea.l #0xc520,A2
00011612: lea (0xd1ca5).l,A1 ; data 000d1ca5
00011618: move.w #0x6000,D1w
0001161C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011622: rts
00011624: move.w #0x0,D2w
00011628: bsr.w 0x000116c4 ; call 000116C4 FUN_000116c4
0001162C: clr.l D3
0001162E: move.w D0w,-(SP) ; data Stack[-0x2]
00011630: clr.l D7
00011632: movea.l (0x00fffc04).l,A0
00011638: move.b (0x56,A0),D7b
0001163C: bmi.w 0x0001164c
00011640: cmpi.b #0x7f,D7b
00011644: beq.w 0x0001164c
00011648: bsr.w 0x00011678 ; call 00011678 FUN_00011678
0001164C: clr.l D1
0001164E: move.b (0x76,A0),D1b
00011652: beq.w 0x00011670
00011656: addi.w #0x2b,D1w
0001165A: moveq #0x7,D7
0001165C: cmp.b (0x66,A0,D7w*0x1),D1b
00011660: beq.w 0x0001166c
00011664: dbf D7w,0x0001165c
00011668: bra.w 0x00011670
0001166C: bsr.w 0x00011678 ; call 00011678 FUN_00011678
00011670: bsr.w 0x00011136 ; call 00011136 FUN_00011136
00011674: move.w (SP)+,D0w
00011676: rts
00011678: divu.w #0x2,D7
0001167C: move.l D7,D1
0001167E: swap D1
00011680: ext.l D1
00011682: ext.l D7
00011684: beq.w 0x0001169e
00011688: cmpi.w #0x2,D7w
0001168C: beq.w 0x0001169e
00011690: lea (0xcd520).l,A1 ; data 000cd520
00011696: move.w #-0x38fe,D4w
0001169A: bra.w 0x000116a8
0001169E: lea (0xcd548).l,A1 ; data 000cd548
000116A4: move.w #-0x18fe,D4w
000116A8: mulu.w #0xa,D1
000116AC: mulu.w #0x180,D7
000116B0: add.w D1w,D4w
000116B2: add.w D7w,D4w
000116B4: move.w #0x4,D7w
000116B8: move.w #0x3,D1w
000116BC: jsr 0x00001232.l ; call 00001232 FUN_00001232
000116C2: rts
000116C4: lea (0xcd570).l,A1 ; data 000cd570
000116CA: move.w #-0x18fe,D4w
000116CE: move.w #0x4,D7w
000116D2: move.w #0x3,D1w
000116D6: jsr 0x00001232.l ; call 00001232 FUN_00001232
000116DC: addi.w #0xa,D4w
000116E0: jsr 0x00001232.l ; call 00001232 FUN_00001232
000116E6: addi.w #0x300,D4w
000116EA: jsr 0x00001232.l ; call 00001232 FUN_00001232
000116F0: subi.w #0xa,D4w
000116F4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000116FA: move.w #-0x377e,D4w
000116FE: move.w #0x9,D7w
00011702: move.w #0x9,D1w
00011706: bsr.w 0x00012c5a ; call 00012C5A FUN_00012c5a
0001170A: rts
0001170C: lea (0xd1d3a).l,A1 ; data 000d1d3a
00011712: movea.l #0xec9c,A2
00011718: move.w #0x6000,D1w
0001171C: clr.l D4
0001171E: move.b (0x00fff0d0).l,D4b
00011724: subq.w #0x2,D4w
00011726: bpl.w 0x00011732
0001172A: lea (0xd1d28).l,A1 ; data 000d1d28
00011730: clr.l D4
00011732: mulu.w #0x12,D4
00011736: adda.l D4,A1
00011738: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001173E: rts
00011740: cmpi.b #0x2,(0x00fff0d0).l
00011748: bmi.w 0x0001176a
0001174C: clr.b (0x00fff0cd).l
00011752: lea (0xd2a0a).l,A0 ; data 000d2a0a
00011758: clr.l D1
0001175A: move.b (0x00fff0d0).l,D1b
00011760: subq.w #0x2,D1w
00011762: lsl.w #0x2,D1w
00011764: adda.l D1,A0
00011766: movea.l (A0),A0 ; data 000d2a0a
00011768: jmp (A0)
0001176A: clr.b (0x00fff09f).l
00011770: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00011774: bne.w 0x0001177a
00011778: rts
0001177A: move.w (0x00fff0d0).l,-(SP)
00011780: move.b #0x2,(0x00fff0d0).l
00011788: cmpi.b #0x10,D0b
0001178C: bmi.w 0x0001179e
00011790: cmpi.b #0x13,D0b
00011794: bpl.w 0x0001179e
00011798: move.b D0b,(0x00fff09f).l
0001179E: lea (0xcd302).l,A1 ; data 000cd302
000117A4: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
000117A8: movea.l #0xcd41a,A1
000117AE: move.w #-0x1,(0x00fff116).l
000117B6: bsr.w 0x00011454 ; call 00011454 FUN_00011454
000117BA: lea (0xd1d94).l,A1 ; data 000d1d94
000117C0: tst.b (0x00fff09f).l
000117C6: beq.w 0x000117d0
000117CA: lea (0xd1e6c).l,A1 ; data 000d1e6c
000117D0: bsr.w 0x00011712 ; call 00011712 FUN_00011712
000117D4: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000117DA: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000117E0: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000117E6: btst.b #0xa,(0x00ffdd22).l
000117EE: beq.w 0x00011840
000117F2: move.w #-0x1,D0w
000117F6: move.l D0,-(SP) ; data Stack[-0x6]
000117F8: move.w #0x25,D0w
000117FC: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011802: move.l (SP)+,D0
00011804: lea (0xcd302).l,A1 ; data 000cd302
0001180A: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
0001180E: bsr.w 0x000118c6 ; call 000118C6 cycle_inventory_action_selector
00011812: movea.l #0xcd41a,A1
00011818: move.w #-0x1,(0x00fff116).l
00011820: bsr.w 0x00011454 ; call 00011454 FUN_00011454
00011824: lea (0xd1d94).l,A1 ; data 000d1d94
0001182A: tst.b (0x00fff09f).l
00011830: beq.w 0x0001183a
00011834: lea (0xd1e6c).l,A1 ; data 000d1e6c
0001183A: bsr.w 0x00011712 ; call 00011712 FUN_00011712
0001183E: bra.b 0x000117d4
00011840: btst.b #0xb,(0x00ffdd22).l
00011848: beq.w 0x00011852
0001184C: move.w #0x1,D0w
00011850: bra.b 0x000117f6
00011852: btst.b #0xe,(0x00ffdd22).l
0001185A: beq.w 0x0001188a
0001185E: move.w #0x24,D0w
00011862: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011868: clr.l D1
0001186A: move.b (0x00fff0d0).l,D1b
00011870: move.w (SP)+,(0x00fff0d0).l
00011876: subq.w #0x2,D1w
00011878: lsl.w #0x2,D1w
0001187A: lea (0xd2a1e).l,A0 ; data 000d2a1e
00011880: adda.l D1,A0
00011882: movea.l (A0),A0 ; data 000d2a1e
00011884: jsr (A0)
00011886: bra.w 0x000118b6
0001188A: btst.b #0xc,(0x00ffdd22).l
00011892: beq.w 0x00011898
00011896: bra.b 0x0001185e
00011898: btst.b #0xd,(0x00ffdd22).l
000118A0: beq.w 0x000118a4
000118A4: btst.b #0xf,(0x00ffdd22).l
000118AC: beq.w 0x000117d4
000118B0: move.w (SP)+,(0x00fff0d0).l
000118B6: movea.l #0xcd2c6,A1
000118BC: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
000118C0: bsr.w 0x0001170c ; call 0001170C FUN_0001170c
000118C4: rts
000118C6: add.b D0b,(0x00fff0d0).l
000118CC: cmpi.b #0x2,(0x00fff0d0).l
000118D4: bpl.w 0x000118ea
000118D8: move.b #0x5,(0x00fff0d0).l
000118E0: move.b #-0x1,(0x00fff112).l
000118E8: rts
000118EA: cmpi.b #0x6,(0x00fff0d0).l
000118F2: bne.w 0x000118fe
000118F6: move.b #0x2,(0x00fff0d0).l
000118FE: rts
00011900: move.l #0xcb18,D4
00011906: moveq #0x0,D1
00011908: bsr.w 0x00012c56 ; call 00012C56 FUN_00012c56
0001190C: lea (0xc1320).l,A1 ; data 000c1320
00011912: move.l #0xea18,D4
00011918: moveq #0x14,D7
0001191A: moveq #0x2,D1
0001191C: move.w #0x0,D2w
00011920: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00011926: rts
00011928: lea (0xcd48a).l,A1 ; data 000cd48a
0001192E: move.l #0xea18,D4
00011934: moveq #0x14,D7
00011936: moveq #0x2,D1
00011938: move.w #0x0,D2w
0001193C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00011942: clr.l D0
00011944: lea (0xcf384).l,A1 ; data 000cf384
0001194A: bsr.w 0x00011ae4 ; call 00011AE4 FUN_00011ae4
0001194E: rts
00011950: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011954: bsr.b 0x00011900 ; call 00011900 FUN_00011900
00011956: movea.l #0xc1a2,A2
0001195C: lea (0xd2054).l,A1 ; data 000d2054
00011962: move.w #0x6000,D1w
00011966: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001196C: movea.l (0x00fffc04).l,A0
00011972: adda.l #0x90,A0
00011978: move.w (A0),D3w
0001197A: beq.w 0x000119b0
0001197E: lea (0xd2072).l,A5 ; data 000d2072
00011984: movea.l #0xc31c,A2
0001198A: moveq #0xe,D7
0001198C: moveq #0xe,D0
0001198E: sub.w D7w,D0w
00011990: btst.l D0,D3
00011992: beq.w 0x000119a8
00011996: lsl.w #0x2,D0w
00011998: movea.l (0x0,A5,D0w*0x1),A1
0001199C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000119A2: adda.l #0x100,A2
000119A8: dbf D7w,0x0001198c
000119AC: bra.w 0x000119c2
000119B0: movea.l #0xc328,A2
000119B6: lea (0xd206b).l,A1 ; data 000d206b
000119BC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000119C2: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
000119C8: bsr.w 0x00011928 ; call 00011928 FUN_00011928
000119CC: rts
000119CE: lea (0xcd514).l,A1 ; data 000cd514
000119D4: move.w #-0x3562,D4w
000119D8: move.w #0x5,D7w
000119DC: move.w #0x0,D1w
000119E0: move.w #0x0,D2w
000119E4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000119EA: clr.l D0
000119EC: movea.l (0x00fffc04).l,A0
000119F2: lea (0xcf334).l,A1 ; data 000cf334
000119F8: bsr.w 0x00011abe ; call 00011ABE FUN_00011abe
000119FC: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00011A00: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00011A06: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00011A0C: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00011A12: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00011A16: btst.b #0x9,(0x00ffdd22).l
00011A1E: beq.w 0x00011a44
00011A22: clr.l D0
00011A24: move.w (0x00ffdd22).l,D0w
00011A2A: btst.l #0x3,D0
00011A2E: bne.w 0x00011a44
00011A32: btst.l #0x2,D0
00011A36: bne.w 0x00011a5c
00011A3A: tst.b (0x00fff115).l
00011A40: bmi.w 0x00011a9a
00011A44: btst.b #0xb,(0x00ffdd22).l
00011A4C: beq.w 0x00011a5c
00011A50: moveq #0x1,D0
00011A52: lea (0xcf334).l,A1 ; data 000cf334
00011A58: bra.w 0x00011a70
00011A5C: btst.b #0xa,(0x00ffdd22).l
00011A64: beq.w 0x00011a86
00011A68: moveq #-0x1,D0
00011A6A: lea (0xcf334).l,A1 ; data 000cf334
00011A70: move.l D0,-(SP) ; data Stack[-0x4]
00011A72: move.w #0x25,D0w
00011A76: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011A7C: move.l (SP)+,D0
00011A7E: bsr.w 0x00011abe ; call 00011ABE FUN_00011abe
00011A82: bra.w 0x000119fc
00011A86: btst.b #0xf,(0x00ffdd22).l
00011A8E: beq.w 0x000119fc
00011A92: move.b #-0x1,(0x00fff0b1).l
00011A9A: lea (0xcd508).l,A1 ; data 000cd508
00011AA0: move.w #-0x3562,D4w
00011AA4: move.w #0x5,D7w
00011AA8: move.w #0x0,D1w
00011AAC: move.w #0x0,D2w
00011AB0: jsr 0x00001232.l ; call 00001232 FUN_00001232
00011AB6: clr.l D0
00011AB8: lea (0xcf384).l,A1 ; data 000cf384
00011ABE: bsr.w 0x00011ae4 ; call 00011AE4 FUN_00011ae4
00011AC2: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00011AC6: move.l #0xc69c,D4
00011ACC: clr.l D0
00011ACE: bsr.w 0x00012cee ; call 00012CEE FUN_00012cee
00011AD2: move.l #0xc79c,D4
00011AD8: moveq #-0x1,D0
00011ADA: bsr.w 0x00012cee ; call 00012CEE FUN_00012cee
00011ADE: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00011AE2: rts
00011AE4: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00011AE8: movea.l (0x00fffc04).l,A0
00011AEE: add.b D0b,(0x4d,A0)
00011AF2: bpl.w 0x00011afe
00011AF6: clr.b (0x4d,A0)
00011AFA: bra.w 0x00011b0e
00011AFE: cmpi.b #0x4,(0x4d,A0)
00011B04: ble.w 0x00011b0e
00011B08: move.b #0x4,(0x4d,A0)
00011B0E: clr.l D0
00011B10: move.b (0x4d,A0),D0b
00011B14: mulu.w #0x10,D0
00011B18: adda.l D0,A1
00011B1A: move.w #-0x3554,D4w
00011B1E: move.w #0x7,D7w
00011B22: move.w #0x0,D1w
00011B26: move.w #0x2000,D2w
00011B2A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00011B30: lea (0xd1a05).l,A1 ; data 000d1a05
00011B36: clr.l D0
00011B38: move.b (0x4d,A0),D0b
00011B3C: mulu.w #0xd,D0
00011B40: adda.l D0,A1
00011B42: movea.l #0xcb22,A2
00011B48: move.w #0x6000,D1w
00011B4C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011B52: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00011B56: rts
00011B58: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011B5C: bsr.w 0x00011900 ; call 00011900 FUN_00011900
00011B60: movea.l #0xc2a2,A2
00011B66: lea (0xd1ae0).l,A1 ; data 000d1ae0
00011B6C: move.w #0x6000,D1w
00011B70: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011B76: clr.l D1
00011B78: movea.l (0x00fffc04).l,A0
00011B7E: move.b (0x4f,A0),D1b
00011B82: movea.l #0xc4b4,A2
00011B88: move.w #0x6000,D2w
00011B8C: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00011B92: move.w (0x00fff0d0).l,-(SP)
00011B98: move.b #0x2,(0x00fff0d0).l
00011BA0: lea (0xcd33e).l,A1 ; data 000cd33e
00011BA6: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011BAA: movea.l #0xcd43a,A1
00011BB0: move.w #-0x1,(0x00fff116).l
00011BB8: bsr.w 0x00011454 ; call 00011454 FUN_00011454
00011BBC: lea (0xd1ddc).l,A1 ; data 000d1ddc
00011BC2: bsr.w 0x00011712 ; call 00011712 FUN_00011712
00011BC6: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00011BCC: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00011BD2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00011BD8: btst.b #0xa,(0x00ffdd22).l
00011BE0: beq.w 0x00011c20
00011BE4: moveq #-0x1,D0
00011BE6: move.l D0,-(SP) ; data Stack[-0x6]
00011BE8: move.w #0x25,D0w
00011BEC: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011BF2: move.l (SP)+,D0
00011BF4: lea (0xcd33e).l,A1 ; data 000cd33e
00011BFA: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011BFE: bsr.w 0x000118c6 ; call 000118C6 cycle_inventory_action_selector
00011C02: movea.l #0xcd43a,A1
00011C08: move.w #-0x1,(0x00fff116).l
00011C10: bsr.w 0x00011454 ; call 00011454 FUN_00011454
00011C14: lea (0xd1ddc).l,A1 ; data 000d1ddc
00011C1A: bsr.w 0x00011712 ; call 00011712 FUN_00011712
00011C1E: bra.b 0x00011bc6
00011C20: btst.b #0xb,(0x00ffdd22).l
00011C28: beq.w 0x00011c30
00011C2C: moveq #0x1,D0
00011C2E: bra.b 0x00011be6
00011C30: btst.b #0xe,(0x00ffdd22).l
00011C38: beq.w 0x00011c80
00011C3C: move.w #0x24,D0w
00011C40: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011C46: bsr.w 0x00012062 ; call 00012062 FUN_00012062
00011C4A: bgt.w 0x00011c58
00011C4E: move.w (SP)+,(0x00fff0d0).l
00011C54: bra.w 0x00011c9c
00011C58: clr.l D1
00011C5A: move.b (0x00fff0d0).l,D1b
00011C60: move.w (SP)+,(0x00fff0d0).l
00011C66: subq.w #0x2,D1w
00011C68: beq.w 0x00011cca
00011C6C: cmpi.w #0x1,D1w
00011C70: beq.w 0x00011dd2
00011C74: cmpi.w #0x2,D1w
00011C78: beq.w 0x00011fde
00011C7C: bra.w 0x00011c9c
00011C80: btst.b #0xc,(0x00ffdd22).l
00011C88: beq.w 0x00011c8e
00011C8C: bra.b 0x00011c3c
00011C8E: btst.b #0xd,(0x00ffdd22).l
00011C96: beq.w 0x00011bc6
00011C9A: bra.b 0x00011c3c
00011C9C: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011CA0: lea (0xd1ed8).l,A1 ; data 000d1ed8
00011CA6: movea.l #0xc5a0,A2
00011CAC: move.w #0x6000,D1w
00011CB0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011CB6: lea (0xcd2c6).l,A1 ; data 000cd2c6
00011CBC: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011CC0: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
00011CC6: bra.w 0x00011928 ; call 00011928 FUN_00011928
00011CCA: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011CCE: movea.l (0x00fffc04).l,A0
00011CD4: move.b (0x56,A0),D1b
00011CD8: lsl.b #0x1,D1b
00011CDA: bmi.w 0x00011ce8
00011CDE: cmpi.b #0xd,(0x57,A0)
00011CE4: bmi.w 0x00011cf8
00011CE8: movea.l #0xc522,A2
00011CEE: lea (0xd1b40).l,A1 ; data 000d1b40
00011CF4: bra.w 0x00011dc4
00011CF8: tst.b (0x4e,A0)
00011CFC: beq.w 0x00011d86
00011D00: clr.b (0x00fff114).l
00011D06: movea.l #0xcd3b6,A1
00011D0C: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011D10: bsr.w 0x000127b0 ; call 000127B0 FUN_000127b0
00011D14: movea.l #0xc49e,A2
00011D1A: lea (0xd1aff).l,A1 ; data 000d1aff
00011D20: move.w #0x6000,D1w
00011D24: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011D2A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00011D30: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00011D36: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00011D3C: btst.b #0xa,(0x00ffdd22).l
00011D44: beq.w 0x00011d58
00011D48: move.w #0x25,D0w
00011D4C: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011D52: bsr.w 0x000127b0 ; call 000127B0 FUN_000127b0
00011D56: bra.b 0x00011d2a
00011D58: btst.b #0xb,(0x00ffdd22).l
00011D60: beq.w 0x00011d66
00011D64: bra.b 0x00011d48
00011D66: btst.b #0xe,(0x00ffdd22).l
00011D6E: beq.b 0x00011d2a
00011D70: move.w #0x24,D0w
00011D74: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011D7A: tst.b (0x00fff114).l
00011D80: bpl.w 0x00011d86
00011D84: rts
00011D86: movea.l (0x00fffc04).l,A0
00011D8C: subq.b #0x1,(0x4f,A0)
00011D90: bmi.w 0x00012062
00011D94: tst.b (0x56,A0)
00011D98: bpl.w 0x00011dac
00011D9C: move.b #0x7f,(0x56,A0)
00011DA2: clr.b (0x57,A0)
00011DA6: jsr 0x00003404.l ; call 00003404 FUN_00003404
00011DAC: moveq #0x26,D1
00011DAE: jsr 0x000036fc.l ; call 000036FC FUN_000036fc
00011DB4: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011DB8: movea.l #0xc51c,A2
00011DBE: lea (0xd1b27).l,A1 ; data 000d1b27
00011DC4: move.w #0x6000,D1w
00011DC8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011DCE: bra.w 0x00011cb6
00011DD2: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011DD6: moveq #0x2,D7
00011DD8: lea (0xffdd48).l,A1 ; data 00ffdd48
00011DDE: lea (0xff0100).l,A0 ; data 00ff0100
00011DE4: btst.b #0x1,(0x10,A0)
00011DEA: bne.w 0x00011dfa
00011DEE: btst.b #0x0,(0x10,A0)
00011DF4: beq.w 0x00011dfa
00011DF8: move.l A0,(A1)+
00011DFA: adda.l #0x100,A0
00011E00: dbf D7w,0x00011de4
00011E04: move.l #-0x1,(A1)
00011E0A: tst.w (0x00ffdd48).l
00011E10: bpl.w 0x00011e2e
00011E14: movea.l #0xc49e,A2
00011E1A: lea (0xd1b8a).l,A1 ; data 000d1b8a
00011E20: move.w #0x6000,D1w
00011E24: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011E2A: bra.w 0x00011cb6
00011E2E: clr.w (0x00ffdd28).l
00011E34: movea.l #0xc4a4,A2
00011E3A: lea (0xd1b61).l,A1 ; data 000d1b61
00011E40: move.w #0x6000,D1w
00011E44: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011E4A: movea.l (0x00fffc04).l,A0
00011E50: movea.l #0xc4b2,A2
00011E56: clr.l D1
00011E58: move.b (0x4f,A0),D1b
00011E5C: move.l D1,(0x00ffdd2a).l
00011E62: move.w #0x6000,D2w
00011E66: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00011E6C: bsr.w 0x0001209a ; call 0001209A FUN_0001209a
00011E70: move.w (0x00fff0d0).l,-(SP)
00011E76: move.b #0x2,(0x00fff0d0).l
00011E7E: lea (0xcd37a).l,A1 ; data 000cd37a
00011E84: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011E88: movea.l #0xcd45a,A1
00011E8E: move.w #-0x1,(0x00fff116).l
00011E96: bsr.w 0x00011454 ; call 00011454 FUN_00011454
00011E9A: lea (0xd1e24).l,A1 ; data 000d1e24
00011EA0: bsr.w 0x00011712 ; call 00011712 FUN_00011712
00011EA4: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00011EAA: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00011EB0: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00011EB6: btst.b #0xa,(0x00ffdd22).l
00011EBE: beq.w 0x00011efe
00011EC2: moveq #-0x1,D0
00011EC4: move.l D0,-(SP) ; data Stack[-0x6]
00011EC6: move.w #0x25,D0w
00011ECA: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011ED0: move.l (SP)+,D0
00011ED2: lea (0xcd37a).l,A1 ; data 000cd37a
00011ED8: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
00011EDC: bsr.w 0x000118c6 ; call 000118C6 cycle_inventory_action_selector
00011EE0: movea.l #0xcd45a,A1
00011EE6: move.w #-0x1,(0x00fff116).l
00011EEE: bsr.w 0x00011454 ; call 00011454 FUN_00011454
00011EF2: lea (0xd1e24).l,A1 ; data 000d1e24
00011EF8: bsr.w 0x00011712 ; call 00011712 FUN_00011712
00011EFC: bra.b 0x00011ea4
00011EFE: btst.b #0xb,(0x00ffdd22).l
00011F06: beq.w 0x00011f0e
00011F0A: moveq #0x1,D0
00011F0C: bra.b 0x00011ec4
00011F0E: btst.b #0xe,(0x00ffdd22).l
00011F16: beq.w 0x00011f5e
00011F1A: move.w #0x24,D0w
00011F1E: jsr 0x000530de.l ; call 000530DE FUN_000530de
00011F24: clr.l D1
00011F26: move.b (0x00fff0d0).l,D1b
00011F2C: subq.w #0x2,D1w
00011F2E: bne.w 0x00011f3c
00011F32: moveq #0x1,D2
00011F34: bsr.w 0x000120e4 ; call 000120E4 FUN_000120e4
00011F38: bra.w 0x00011ea4
00011F3C: cmpi.w #0x1,D1w
00011F40: bne.w 0x00011f4e
00011F44: moveq #-0x1,D2
00011F46: bsr.w 0x000120e4 ; call 000120E4 FUN_000120e4
00011F4A: bra.w 0x00011ea4
00011F4E: cmpi.w #0x2,D1w
00011F52: beq.w 0x00011f90
00011F56: bsr.w 0x0001209a ; call 0001209A FUN_0001209a
00011F5A: bra.w 0x00011ea4
00011F5E: btst.b #0xc,(0x00ffdd22).l
00011F66: beq.w 0x00011f6c
00011F6A: bra.b 0x00011f1a
00011F6C: btst.b #0xd,(0x00ffdd22).l
00011F74: beq.w 0x00011f7a
00011F78: bra.b 0x00011f1a
00011F7A: btst.b #0xf,(0x00ffdd22).l
00011F82: beq.w 0x00011ea4
00011F86: move.w (SP)+,(0x00fff0d0).l
00011F8C: bra.w 0x00011c9c
00011F90: move.w (SP)+,(0x00fff0d0).l
00011F96: movea.l (0x00fffc04).l,A0
00011F9C: movea.l (0x00ffdd4c).l,A1
00011FA2: tst.w (0x00ffdd28).l
00011FA8: beq.w 0x00011fb2
00011FAC: movea.l (0x00ffdd48).l,A1
00011FB2: move.l (0x00ffdd2a).l,D0
00011FB8: sub.b D0b,(0x4f,A0)
00011FBC: add.b D0b,(0x4f,A1)
00011FC0: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011FC4: movea.l #0xc51c,A2
00011FCA: lea (0xd1b78).l,A1 ; data 000d1b78
00011FD0: move.w #0x6000,D1w
00011FD4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00011FDA: bra.w 0x00011cb6
00011FDE: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00011FE2: clr.l D5
00011FE4: clr.l D6
00011FE6: moveq #0x2,D7
00011FE8: lea (0xff0100).l,A0 ; data 00ff0100
00011FEE: btst.b #0x0,(0x10,A0)
00011FF4: beq.w 0x00011ffe
00011FF8: addq.w #0x1,D6w
00011FFA: add.b (0x4f,A0),D5b
00011FFE: adda.l #0x100,A0
00012004: dbf D7w,0x00011fee
00012008: cmpi.w #0x2,D6w
0001200C: bpl.w 0x00012014
00012010: bra.w 0x00011e14
00012014: tst.w D5w
00012016: beq.w 0x00012044
0001201A: divu.w D6w,D5
0001201C: moveq #0x2,D7
0001201E: lea (0xff0100).l,A0 ; data 00ff0100
00012024: move.b D5b,(0x4f,A0)
00012028: btst.b #0x1,(0x10,A0)
0001202E: beq.w 0x0001203a
00012032: swap D5
00012034: add.b D5b,(0x4f,A0)
00012038: swap D5
0001203A: adda.l #0x100,A0
00012040: dbf D7w,0x00012024
00012044: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00012048: movea.l #0xc51c,A2
0001204E: lea (0xd1b78).l,A1 ; data 000d1b78
00012054: move.w #0x6000,D1w
00012058: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001205E: bra.w 0x00011cb6
00012062: movea.l (0x00fffc04).l,A0
00012068: tst.b (0x4f,A0)
0001206C: bgt.w 0x00012098
00012070: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
00012074: movea.l #0xc51c,A2
0001207A: lea (0x58,A0),A1
0001207E: move.w #0x6000,D1w
00012082: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012088: lea (0xd1ac6).l,A1 ; data 000d1ac6
0001208E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012094: clr.b (0x4f,A0)
00012098: rts
0001209A: lea (0xffdd48).l,A1 ; data 00ffdd48
000120A0: tst.w (0x00ffdd28).l
000120A6: beq.w 0x000120b6
000120AA: clr.w (0x00ffdd28).l
000120B0: addq.l #0x4,A1
000120B2: bra.w 0x000120be
000120B6: move.w #-0x1,(0x00ffdd28).l
000120BE: tst.w (A1)
000120C0: bmi.b 0x0001209a
000120C2: movea.l (A1),A0
000120C4: lea (0x58,A0),A1
000120C8: movea.l #0xc59e,A2
000120CE: move.w #0x6000,D1w
000120D2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000120D8: move.l #0xc928,D4
000120DE: bsr.w 0x000110e8 ; call 000110E8 FUN_000110e8
000120E2: rts
000120E4: move.w #-0x3b50,D4w
000120E8: clr.w D1w
000120EA: clr.w D7w
000120EC: move SR,-(SP) ; data Stack[-0x2]
000120EE: move #0x2700,SR
000120F2: lea (0xc00000).l,A4 ; data 00c00000
000120F8: lea (0xc1320).l,A5 ; data 000c1320
000120FE: move.w D4w,D0w
00012100: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00012106: move.w (A5),D0w
00012108: ori.w #0x6000,D0w
0001210C: move.w D0w,(A4)
0001210E: move (SP)+,SR
00012110: movea.l (0x00fffc04).l,A0
00012116: move.l (0x00ffdd2a).l,D1
0001211C: add.w D2w,D1w
0001211E: bgt.w 0x00012126
00012122: move.b (0x4f,A0),D1b
00012126: cmp.b (0x4f,A0),D1b
0001212A: ble.w 0x00012130
0001212E: moveq #0x1,D1
00012130: move.l D1,(0x00ffdd2a).l
00012136: movea.l #0xc4b2,A2
0001213C: move.w #0x6000,D2w
00012140: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00012146: rts
00012148: movea.l (0x00fffc04).l,A0
0001214E: clr.l D0
00012150: clr.l D7
00012152: move.b (0x00fff0d1).l,D7b
00012158: mulu.w #0x2,D7
0001215C: add.b (0x00fff0d0).l,D7b
00012162: move.b (0x66,A0,D7w*0x1),D0b
00012166: rts
00012168: bsr.b 0x00012148 ; call 00012148 FUN_00012148
0001216A: tst.b (0x00fff09f).l
00012170: bgt.w 0x00012298
00012174: cmpi.b #0x14,D0b
00012178: beq.w 0x000121c0
0001217C: cmpi.b #0x16,D0b
00012180: beq.w 0x000121c0
00012184: cmpi.b #0x13,D0b
00012188: beq.w 0x000121c0
0001218C: cmpi.b #0x18,D0b
00012190: bmi.w 0x000121e0
00012194: cmpi.b #0x2c,D0b
00012198: bmi.w 0x000121c0
0001219C: tst.b (0x76,A0)
000121A0: beq.w 0x000121b0
000121A4: clr.b (0x76,A0)
000121A8: clr.b (0x6e,A0,D7w*0x1)
000121AC: bra.w 0x0001228c
000121B0: subi.b #0x2b,D0b
000121B4: move.b D0b,(0x76,A0)
000121B8: move.b D0b,(0x6e,A0,D7w*0x1)
000121BC: bra.w 0x0001228c
000121C0: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
000121C4: movea.l #0xc620,A2
000121CA: lea (0xd202f).l,A1 ; data 000d202f
000121D0: move.w #0x6000,D1w
000121D4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000121DA: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
000121E0: tst.b (0x56,A0)
000121E4: bmi.w 0x000121fe
000121E8: cmpi.b #0xd,(0x57,A0)
000121EE: bpl.w 0x00012210
000121F2: clr.l D5
000121F4: move.b (0x57,A0),D5b
000121F8: subq.w #0x1,D5w
000121FA: bpl.w 0x0001220c
000121FE: clr.l D1
00012200: move.b (0x4e,A0),D1b
00012204: clr.b (0x4e,A0)
00012208: bra.w 0x00012236
0001220C: bsr.w 0x000125fa ; call 000125FA FUN_000125fa
00012210: cmp.b (0x56,A0),D7b
00012214: bne.w 0x00012236
00012218: cmpi.b #0xd,(0x57,A0)
0001221E: bpl.w 0x00012226
00012222: move.b D1b,(0x4e,A0)
00012226: move.b #0x7f,(0x56,A0)
0001222C: clr.b (0x57,A0)
00012230: clr.w D0w
00012232: bra.w 0x00012286
00012236: clr.b (0x9d,A0)
0001223A: move.b D7b,(0x56,A0)
0001223E: move.b D0b,(0x57,A0)
00012242: cmpi.b #0xd,(0x57,A0)
00012248: bmi.w 0x00012256
0001224C: move.b D1b,(0x4e,A0)
00012250: clr.w D1w
00012252: bra.w 0x00012286
00012256: move.l D0,D2
00012258: subq.w #0x1,D2w
0001225A: mulu.w #0x30,D2
0001225E: lea (0xd4540).l,A4 ; data 000d4540
00012264: adda.l D2,A4
00012266: clr.l D2
00012268: move.b (0x24,A4),D2b ; data 000d4564
0001226C: mulu.w D2w,D1
0001226E: divu.w #0x64,D1
00012272: move.l D1,D3
00012274: swap D3
00012276: andi.l #0xffff,D3
0001227C: cmpi.w #0x32,D3w
00012280: bmi.w 0x00012286
00012284: addq.w #0x1,D1w
00012286: jsr 0x000036fc.l ; call 000036FC FUN_000036fc
0001228C: jsr 0x00003404.l ; call 00003404 FUN_00003404
00012292: bsr.w 0x00011624 ; call 00011624 FUN_00011624
00012296: rts
00012298: bsr.w 0x00011900 ; call 00011900 FUN_00011900
0001229C: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
000122A0: clr.b (0x00fff0b9).l
000122A6: lea (0xfff0ba).l,A4 ; data 00fff0ba
000122AC: movea.l #0xc5a0,A2
000122B2: lea (0xd1eea).l,A1 ; data 000d1eea
000122B8: move.w #0x6000,D1w
000122BC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000122C2: movea.l #0xc6a0,A2
000122C8: lea (0xff0100).l,A6 ; data 00ff0100
000122CE: btst.b #0x0,(0x10,A6)
000122D4: beq.w 0x000122ec
000122D8: addq.b #0x1,(0x00fff0b9).l
000122DE: lea (0x58,A6),A1 ; data 00ff0158
000122E2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000122E8: move.l A6,(A4)
000122EA: addq.l #0x4,A4
000122EC: adda.l #0x100,A6
000122F2: btst.b #0x0,(0x10,A6)
000122F8: beq.w 0x00012316
000122FC: addq.b #0x1,(0x00fff0b9).l
00012302: adda.l #0x80,A2
00012308: lea (0x58,A6),A1 ; data 00ff0258
0001230C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012312: move.l A6,(A4)
00012314: addq.l #0x4,A4
00012316: adda.l #0x100,A6
0001231C: btst.b #0x0,(0x10,A6)
00012322: beq.w 0x0001233e
00012326: addq.b #0x1,(0x00fff0b9).l
0001232C: adda.l #0x80,A2
00012332: lea (0x58,A6),A1 ; data 00ff0358
00012336: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001233C: move.l A6,(A4)
0001233E: adda.l #0x86,A2
00012344: lea (0xd1f03).l,A1 ; data 000d1f03
0001234A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012350: clr.b (0x00fff0dc).l
00012356: clr.w D2w
00012358: bra.w 0x0001238e
0001235C: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00012362: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00012368: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0001236E: btst.b #0x8,(0x00ffdd22).l
00012376: beq.w 0x00012380
0001237A: moveq #-0x1,D2
0001237C: bra.w 0x0001238e
00012380: btst.b #0x9,(0x00ffdd22).l
00012388: beq.w 0x00012402
0001238C: moveq #0x1,D2
0001238E: move.w #0x25,D0w
00012392: jsr 0x000530de.l ; call 000530DE FUN_000530de
00012398: clr.w D3w
0001239A: move.b (0x00fff0b9).l,D3b
000123A0: move.l #0xc69e,D7
000123A6: move.w #0x6000,D4w
000123AA: move.b #-0x1,(0x00fff0fc).l
000123B2: bsr.w 0x0000c376 ; call 0000C376 move_wrapped_menu_cursor
000123B6: clr.l D0
000123B8: move.b (0x00fff0dc).l,D0b
000123BE: cmp.b (0x00fff0b9).l,D0b
000123C4: bne.w 0x000123e6
000123C8: lea (0xc1320).l,A1 ; data 000c1320
000123CE: move.l #0xc928,D4
000123D4: moveq #0x3,D7
000123D6: moveq #0x3,D1
000123D8: move.w #0x0,D2w
000123DC: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000123E2: bra.w 0x0001235c
000123E6: mulu.w #0x4,D0
000123EA: lea (0xfff0ba).l,A0 ; data 00fff0ba
000123F0: adda.l D0,A0
000123F2: movea.l (A0),A0
000123F4: move.l #0xc928,D4
000123FA: bsr.w 0x000110e8 ; call 000110E8 FUN_000110e8
000123FE: bra.w 0x0001235c
00012402: btst.b #0xe,(0x00ffdd22).l
0001240A: beq.w 0x0001250c
0001240E: move.w #0x24,D0w
00012412: jsr 0x000530de.l ; call 000530DE FUN_000530de
00012418: bsr.w 0x00012c1e ; call 00012C1E FUN_00012c1e
0001241C: clr.l D0
0001241E: move.b (0x00fff0dc).l,D0b
00012424: cmp.b (0x00fff0b9).l,D0b
0001242A: beq.w 0x00012538
0001242E: mulu.w #0x4,D0
00012432: lea (0xfff0ba).l,A0 ; data 00fff0ba
00012438: adda.l D0,A0
0001243A: movea.l (A0),A1
0001243C: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012440: subi.w #0x11,D0w
00012444: bmi.w 0x000124d4
00012448: beq.w 0x0001247e
0001244C: cmpi.b #0xa,(0x4c,A1)
00012452: bne.w 0x00012466
00012456: movea.l #0xc69a,A2
0001245C: lea (0xd1f28).l,A1 ; data 000d1f28
00012462: bra.w 0x000124f8
00012466: exg A0,A1
00012468: move.w #0x2,D1w
0001246C: jsr 0x00003674.l ; call 00003674 FUN_00003674
00012472: tst.b (0x9c,A0)
00012476: beq.w 0x000124ac
0001247A: bra.w 0x0001249a
0001247E: cmpi.b #0xa,(0x9c,A1)
00012484: beq.b 0x00012456
00012486: exg A0,A1
00012488: move.w #0x2,D1w
0001248C: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
00012492: tst.b (0x4c,A0)
00012496: beq.w 0x000124ac
0001249A: bclr.b #0x6,(0x10,A0)
000124A0: bclr.b #0x4,(0x10,A0)
000124A6: bset.b #0x0,(0x10,A0)
000124AC: exg A0,A1
000124AE: subq.b #0x1,(0x6e,A0,D7w*0x1) ; data ffffc70c
000124B2: bgt.w 0x000124c4
000124B6: bsr.w 0x0001275c ; call 0001275C FUN_0001275c
000124BA: bsr.w 0x00011624 ; call 00011624 FUN_00011624
000124BE: jsr 0x000038fa.l ; call 000038FA FUN_000038fa
000124C4: movea.l #0xc69c,A2
000124CA: lea (0xd1f0a).l,A1 ; data 000d1f0a
000124D0: bra.w 0x000124f8
000124D4: bsr.w 0x0001253e ; call 0001253E FUN_0001253e
000124D8: bmi.w 0x000124f8
000124DC: subq.b #0x1,(0x6e,A0,D7w*0x1) ; data ffffc70c
000124E0: bgt.w 0x000124ec
000124E4: bsr.w 0x0001275c ; call 0001275C FUN_0001275c
000124E8: bsr.w 0x00011624 ; call 00011624 FUN_00011624
000124EC: movea.l #0xc6a4,A2
000124F2: lea (0xd1f1b).l,A1 ; data 000d1f1b
000124F8: move.w #0x6000,D1w
000124FC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012502: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
00012508: bra.w 0x00012538
0001250C: btst.b #0xc,(0x00ffdd22).l
00012514: beq.w 0x0001251c
00012518: bra.w 0x0001240e
0001251C: btst.b #0xd,(0x00ffdd22).l
00012524: beq.w 0x0001252c
00012528: bra.w 0x0001240e
0001252C: btst.b #0xf,(0x00ffdd22).l
00012534: beq.w 0x0001235c
00012538: bsr.w 0x00011928 ; call 00011928 FUN_00011928
0001253C: rts
0001253E: movem.l {  D7 D6 D5 D4 D3 D2 D1},-(SP)
00012542: clr.l D6
00012544: move.b (0x89,A0),D6b
00012548: cmpi.b #0xa,(0x4c,A1)
0001254E: bne.w 0x00012572
00012552: cmpi.b #0xa,(0x9c,A1)
00012558: bne.w 0x00012572
0001255C: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7}
00012560: movea.l #0xc69a,A2
00012566: lea (0xd1f28).l,A1 ; data 000d1f28
0001256C: move.w #-0x1,D6w
00012570: rts
00012572: cmpi.b #0x7,(0x4c,A1)
00012578: bmi.w 0x00012584
0001257C: move.w #0x4,D4w
00012580: bra.w 0x0001259a
00012584: cmpi.b #0x4,(0x4c,A1)
0001258A: bmi.w 0x00012596
0001258E: move.w #0x6,D4w
00012592: bra.w 0x0001259a
00012596: move.w #0x8,D4w
0001259A: clr.l D3
0001259C: move.b (0x77,A1),D3b
000125A0: divu.w #0x3,D3
000125A4: sub.w D3w,D4w
000125A6: cmpi.w #0x2,D4w
000125AA: bpl.w 0x000125b2
000125AE: move.w #0x2,D4w
000125B2: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000125B8: clr.l D1
000125BA: move.b (0x00fff0dd).l,D1b
000125C0: bgt.w 0x000125c8
000125C4: move.w #0x1,D1w
000125C8: exg A0,A1
000125CA: jsr 0x00003674.l ; call 00003674 FUN_00003674
000125D0: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
000125D6: bclr.b #0x6,(0x10,A0)
000125DC: bclr.b #0x4,(0x10,A0)
000125E2: bset.b #0x0,(0x10,A0)
000125E8: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
000125F0: exg A0,A1
000125F2: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7}
000125F6: clr.l D6
000125F8: rts
000125FA: mulu.w #0x30,D5
000125FE: lea (0xd4540).l,A4 ; data 000d4540
00012604: adda.l D5,A4
00012606: clr.l D2
00012608: move.b (0x24,A4),D2b ; data 000d4564
0001260C: clr.l D1
0001260E: move.b (0x4e,A0),D1b
00012612: clr.b (0x4e,A0)
00012616: mulu.w #0x64,D1
0001261A: tst.w D2w
0001261C: beq.w 0x00012622
00012620: divu.w D2w,D1
00012622: move.l D1,D3
00012624: lsr.l #0x8,D3
00012626: beq.w 0x0001262c
0001262A: addq.w #0x1,D1w
0001262C: andi.l #0xffff,D1
00012632: rts
00012634: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012638: cmpi.w #0x7,D7w
0001263C: bne.w 0x0001267e
00012640: cmpi.b #0x29,(0x6d,A0)
00012646: beq.w 0x0001265e
0001264A: cmpi.b #0x2a,(0x6d,A0)
00012650: beq.w 0x0001265e
00012654: cmpi.b #0x2b,(0x6d,A0)
0001265A: bne.w 0x0001267e
0001265E: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
00012662: movea.l #0xc620,A2
00012668: lea (0xd1ffa).l,A1 ; data 000d1ffa
0001266E: move.w #0x6000,D1w
00012672: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012678: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
0001267E: clr.w (0x00fff098).l
00012684: movea.l #0xcd3b6,A1
0001268A: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
0001268E: clr.b (0x00fff114).l
00012694: bsr.w 0x000127b0 ; call 000127B0 FUN_000127b0
00012698: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
0001269C: bsr.w 0x00012148 ; call 00012148 FUN_00012148
000126A0: movea.l (0x00fffc04).l,A0
000126A6: cmp.b (0x56,A0),D7b
000126AA: bne.w 0x000126d0
000126AE: movea.l #0xc620,A2
000126B4: lea (0xd1cbb).l,A1 ; data 000d1cbb
000126BA: move.w #0x6000,D1w
000126BE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000126C4: move.w #-0x1,(0x00fff098).l
000126CC: bra.w 0x000126d0
000126D0: movea.l #0xc7a0,A2
000126D6: lea (0xd1cd3).l,A1 ; data 000d1cd3
000126DC: move.w #0x6000,D1w
000126E0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000126E6: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000126EC: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000126F2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000126F8: btst.b #0xa,(0x00ffdd22).l
00012700: beq.w 0x00012714
00012704: move.w #0x25,D0w
00012708: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001270E: bsr.w 0x000127b0 ; call 000127B0 FUN_000127b0
00012712: bra.b 0x000126e6
00012714: btst.b #0xb,(0x00ffdd22).l
0001271C: beq.w 0x00012722
00012720: bra.b 0x00012704
00012722: btst.b #0xe,(0x00ffdd22).l
0001272A: beq.w 0x00012750
0001272E: move.w #0x24,D0w
00012732: jsr 0x000530de.l ; call 000530DE FUN_000530de
00012738: tst.b (0x00fff114).l
0001273E: bmi.w 0x0001274e
00012742: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012746: bsr.w 0x0001275c ; call 0001275C FUN_0001275c
0001274A: bsr.w 0x00011624 ; call 00011624 FUN_00011624
0001274E: rts
00012750: btst.b #0xc,(0x00ffdd22).l
00012758: beq.b 0x000126e6
0001275A: bra.b 0x0001272e
0001275C: move.l D0,D1
0001275E: cmpi.b #0x2c,D1b
00012762: bmi.w 0x00012782
00012766: subi.w #0x2b,D1w
0001276A: cmp.b (0x76,A0),D1b
0001276E: bne.w 0x000127a6
00012772: tst.b (0x6e,A0,D7w*0x1)
00012776: beq.w 0x000127a6
0001277A: clr.b (0x76,A0)
0001277E: bra.w 0x000127a6
00012782: cmpi.w #-0x1,(0x00fff098).l
0001278A: bne.w 0x000127a6
0001278E: move.b #0x7f,(0x56,A0)
00012794: clr.b (0x57,A0)
00012798: clr.b (0x4e,A0)
0001279C: move.l D7,-(SP) ; data Stack[-0x4]
0001279E: jsr 0x00003404.l ; call 00003404 FUN_00003404
000127A4: move.l (SP)+,D7
000127A6: clr.b (0x66,A0,D7w*0x1)
000127AA: clr.b (0x6e,A0,D7w*0x1)
000127AE: rts
000127B0: lea (0xcd3b6).l,A1 ; data 000cd3b6
000127B6: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
000127BA: eori.b #-0x1,(0x00fff114).l
000127C2: movea.l #0xcd47a,A1
000127C8: bsr.w 0x000127fc ; call 000127FC FUN_000127fc
000127CC: lea (0xd1eb4).l,A1 ; data 000d1eb4
000127D2: movea.l #0xec9c,A2
000127D8: move.w #0x6000,D1w
000127DC: tst.b (0x00fff114).l
000127E2: bmi.w 0x000127ec
000127E6: moveq #0x0,D4
000127E8: bra.w 0x000127ee
000127EC: moveq #0x1,D4
000127EE: mulu.w #0x12,D4
000127F2: adda.l D4,A1
000127F4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000127FA: rts
000127FC: tst.b (0x00fff114).l
00012802: bmi.w 0x0001280e
00012806: moveq #0x0,D3
00012808: moveq #0x1,D4
0001280A: bra.w 0x00012812
0001280E: moveq #0x1,D3
00012810: moveq #0x2,D4
00012812: mulu.w #0x8,D3
00012816: adda.l D3,A1
00012818: mulu.w #0x6,D4
0001281C: addi.l #0xeba2,D4
00012822: move.w #0x1,D7w
00012826: move.w #0x1,D1w
0001282A: move.w #0x0,D2w
0001282E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012834: rts
00012836: bsr.w 0x00012148 ; call 00012148 FUN_00012148
0001283A: cmpi.b #0x17,D0b
0001283E: bmi.w 0x0001286a
00012842: cmpi.b #0x2c,D0b
00012846: bpl.w 0x0001286a
0001284A: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
0001284E: movea.l #0xc622,A2
00012854: lea (0xd2015).l,A1 ; data 000d2015
0001285A: move.w #0x6000,D1w
0001285E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012864: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
0001286A: bsr.w 0x00011900 ; call 00011900 FUN_00011900
0001286E: movea.l (0x00fffc04).l,A6
00012874: movea.l #0xcd3b6,A1
0001287A: bsr.w 0x000111d0 ; call 000111D0 FUN_000111d0
0001287E: clr.b (0x00fff114).l
00012884: bsr.w 0x000127b0 ; call 000127B0 FUN_000127b0
00012888: clr.w (0x00fff096).l
0001288E: adda.l #0x100,A6
00012894: cmpi.w #0x2,(0x3e,A6)
0001289A: ble.w 0x000128a4
0001289E: lea (0xff0100).l,A6 ; data 00ff0100
000128A4: btst.b #0x0,(0x10,A6)
000128AA: beq.b 0x0001288e
000128AC: move.l A6,D5
000128AE: move.l (0x00fffc04).l,D7
000128B4: cmp.l D7,D5
000128B6: bne.w 0x000128be
000128BA: bra.w 0x000129dc
000128BE: movea.l A6,A0
000128C0: moveq #0x7,D6
000128C2: move.b (0x66,A0),D5b
000128C6: beq.w 0x00012908
000128CA: addq.l #0x1,A0
000128CC: dbf D6w,0x000128c2
000128D0: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
000128D4: movea.l #0xc69e,A2
000128DA: lea (0x58,A6),A1 ; data 00ff0158
000128DE: move.w #0x6000,D1w
000128E2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000128E8: movea.l #0xc71e,A2
000128EE: lea (0xd1ce1).l,A1 ; data 000d1ce1
000128F4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000128FA: moveq #0x8,D6
000128FC: move.w #-0x1,(0x00fff096).l
00012904: bra.w 0x00012966
00012908: bsr.w 0x00012c42 ; call 00012C42 FUN_00012c42
0001290C: movea.l #0xc7a6,A2
00012912: lea (0xd1d19).l,A1 ; data 000d1d19
00012918: move.w #0x6000,D1w
0001291C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012922: movea.l #0xc820,A2
00012928: lea (0x58,A6),A1 ; data 00ff0158
0001292C: move.w #0x6000,D1w
00012930: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012936: exg A0,A6
00012938: move.l #0xc928,D4
0001293E: bsr.w 0x000110e8 ; call 000110E8 FUN_000110e8
00012942: exg A0,A6
00012944: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012948: cmp.b (0x56,A0),D7b
0001294C: bne.w 0x00012966
00012950: movea.l #0xc620,A2
00012956: lea (0xd1cbb).l,A1 ; data 000d1cbb
0001295C: move.w #0x6000,D1w
00012960: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012966: moveq #0x7,D5
00012968: sub.w D6w,D5w
0001296A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00012970: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00012976: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0001297C: btst.b #0xa,(0x00ffdd22).l
00012984: beq.w 0x00012998
00012988: move.w #0x25,D0w
0001298C: jsr 0x000530de.l ; call 000530DE FUN_000530de
00012992: bsr.w 0x000127b0 ; call 000127B0 FUN_000127b0
00012996: bra.b 0x0001296a
00012998: btst.b #0xb,(0x00ffdd22).l
000129A0: beq.w 0x000129a6
000129A4: bra.b 0x00012988
000129A6: btst.b #0xe,(0x00ffdd22).l
000129AE: beq.w 0x000129e2
000129B2: move.w #0x24,D0w
000129B6: jsr 0x000530de.l ; call 000530DE FUN_000530de
000129BC: tst.w (0x00fff096).l
000129C2: bmi.w 0x0001287e
000129C6: tst.b (0x00fff114).l
000129CC: bmi.w 0x0001287e
000129D0: bsr.w 0x000129fe ; call 000129FE FUN_000129fe
000129D4: bsr.w 0x00011624 ; call 00011624 FUN_00011624
000129D8: bsr.w 0x00012c36 ; call 00012C36 FUN_00012c36
000129DC: bsr.w 0x00011928 ; call 00011928 FUN_00011928
000129E0: rts
000129E2: btst.b #0xd,(0x00ffdd22).l
000129EA: beq.w 0x000129f0
000129EE: bra.b 0x000129b2
000129F0: btst.b #0xc,(0x00ffdd22).l
000129F8: beq.w 0x0001296a
000129FC: bra.b 0x000129b2
000129FE: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012A02: move.b D0b,(0x66,A6,D5w*0x1)
00012A06: move.l D0,D1
00012A08: cmpi.b #0x2c,D1b
00012A0C: bmi.w 0x00012a30
00012A10: clr.b (0x6e,A6,D5w*0x1)
00012A14: subi.w #0x2b,D1w
00012A18: cmp.b (0x76,A0),D1b
00012A1C: bne.w 0x00012a4e
00012A20: tst.b (0x6e,A0,D7w*0x1)
00012A24: beq.w 0x00012a4e
00012A28: clr.b (0x76,A0)
00012A2C: bra.w 0x00012a4e
00012A30: move.b (0x6e,A0,D7w*0x1),(0x6e,A6,D5w*0x1)
00012A36: cmp.b (0x56,A0),D7b
00012A3A: bne.w 0x00012a4e
00012A3E: move.b #0x7f,(0x56,A0)
00012A44: clr.b (0x57,A0)
00012A48: jsr 0x00003404.l ; call 00003404 FUN_00003404
00012A4E: clr.b (0x66,A0,D7w*0x1)
00012A52: rts
00012A54: move.l D0,-(SP) ; data Stack[-0x4]
00012A56: cmpi.w #0xd,D0w
00012A5A: bmi.w 0x00012a74
00012A5E: tst.b (0x00fff100).l
00012A64: bne.w 0x00012a88
00012A68: move.b #0x1,(0x00fff100).l
00012A70: bra.w 0x00012a84
00012A74: tst.b (0x00fff100).l
00012A7A: beq.w 0x00012a88
00012A7E: clr.b (0x00fff100).l
00012A84: bsr.w 0x0001107e ; call 0001107E FUN_0001107e
00012A88: move.l (SP)+,D0
00012A8A: rts
00012A8C: move.l D0,-(SP) ; data Stack[-0x4]
00012A8E: bsr.b 0x00012a54 ; call 00012A54 FUN_00012a54
00012A90: lea (0xd4540).l,A4 ; data 000d4540
00012A96: subq.w #0x1,D0w
00012A98: mulu.w #0x30,D0
00012A9C: tst.l (0x1e,A4,D0w*0x1) ; data 000d455e
00012AA0: bmi.w 0x00012abe
00012AA4: movea.l (0x1e,A4,D0w*0x1),A1 ; data 000d455e
00012AA8: move.w #-0x3e5c,D4w
00012AAC: move.w #0x8,D7w
00012AB0: move.w #0x4,D1w
00012AB4: move.w #0x2000,D2w
00012AB8: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012ABE: movea.l #0xc420,A2
00012AC4: lea (0x0,A4,D0w*0x1),A1
00012AC8: move.w #0x6000,D1w
00012ACC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012AD2: clr.l D3
00012AD4: move.b (0x24,A4,D0w*0x1),D3b ; data 000d4564
00012AD8: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012ADC: cmpi.b #0xd,D0b
00012AE0: bpl.w 0x00012b8e
00012AE4: move.w D7w,D6w
00012AE6: move.w #0x3,D7w
00012AEA: move.w #0x1,D1w
00012AEE: move.w #0x2000,D2w
00012AF2: btst.b #0x1,(0x6e,A0,D6w*0x1)
00012AF8: beq.w 0x00012b0c
00012AFC: lea (0xd25ec).l,A1 ; data 000d25ec
00012B02: move.w #-0x3e64,D4w
00012B06: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012B0C: btst.b #0x2,(0x6e,A0,D6w*0x1)
00012B12: beq.w 0x00012b26
00012B16: lea (0xd25fc).l,A1 ; data 000d25fc
00012B1C: move.w #-0x3e64,D4w
00012B20: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012B26: btst.b #0x0,(0x6e,A0,D6w*0x1)
00012B2C: beq.w 0x00012b40
00012B30: lea (0xd25dc).l,A1 ; data 000d25dc
00012B36: move.w #-0x3e4a,D4w
00012B3A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012B40: btst.b #0x3,(0x6e,A0,D6w*0x1)
00012B46: beq.w 0x00012b5a
00012B4A: lea (0xd260c).l,A1 ; data 000d260c
00012B50: move.w #-0x3e4a,D4w
00012B54: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012B5A: btst.b #0x4,(0x6e,A0,D6w*0x1)
00012B60: beq.w 0x00012b74
00012B64: lea (0xd261c).l,A1 ; data 000d261c
00012B6A: move.w #-0x3cca,D4w
00012B6E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012B74: btst.b #0x5,(0x6e,A0,D6w*0x1)
00012B7A: beq.w 0x00012b8e
00012B7E: lea (0xd262c).l,A1 ; data 000d262c
00012B84: move.w #-0x3cca,D4w
00012B88: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012B8E: cmpi.b #0xd,D0b
00012B92: bmi.w 0x00012bca
00012B96: cmpi.b #0x14,D0b
00012B9A: bmi.w 0x00012bd4
00012B9E: beq.w 0x00012bde
00012BA2: cmpi.b #0x18,D0b
00012BA6: bmi.w 0x00012c1a
00012BAA: cmpi.b #0x29,D0b
00012BAE: bmi.w 0x00012bde
00012BB2: cmpi.b #0x2c,D0b
00012BB6: bmi.w 0x00012c1a
00012BBA: move.b #-0x1,(0x00fff0b1).l
00012BC2: bsr.w 0x00011352 ; call 00011352 FUN_00011352
00012BC6: bra.w 0x00012c1a
00012BCA: lea (0xd29c4).l,A1 ; data 000d29c4
00012BD0: bra.w 0x00012bec
00012BD4: lea (0xd29ca).l,A1 ; data 000d29ca
00012BDA: bra.w 0x00012be4
00012BDE: lea (0xd29d0).l,A1 ; data 000d29d0
00012BE4: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012BE8: move.b (0x6e,A0,D7w*0x1),D3b
00012BEC: movea.w #-0x3ce4,A2
00012BF0: move.w #0x4000,D1w
00012BF4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012BFA: move.w #-0x3c62,D4w
00012BFE: lea (0x6ac46).l,A1 ; data 0006ac46
00012C04: lsl.w #0x2,D3w
00012C06: adda.l D3,A1
00012C08: move.w #0x1,D7w
00012C0C: move.w #0x0,D1w
00012C10: move.w #0x2000,D2w
00012C14: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012C1A: move.l (SP)+,D0
00012C1C: rts
00012C1E: move.w #-0x3ae8,D4w
00012C22: move.w #0x7,D1w
00012C26: bra.w 0x00012c56 ; call 00012C56 FUN_00012c56
00012C36: move.w #-0x3ee8,D4w
00012C3A: move.w #0x13,D1w
00012C3E: bra.w 0x00012c56 ; call 00012C56 FUN_00012c56
00012C42: move.w #-0x3ae8,D4w
00012C46: move.w #0xb,D1w
00012C4A: bra.w 0x00012c56 ; call 00012C56 FUN_00012c56
00012C56: move.w #0x14,D7w
00012C5A: movem.l {  A1 D2},-(SP)
00012C5E: move.w #0x6000,D2w
00012C62: lea (0xc1320).l,A1 ; data 000c1320
00012C68: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00012C6E: movem.l (SP)+,{  D2 A1}
00012C72: rts
00012C74: clr.l D0
00012C76: move.b (0x81,A0),D0b
00012C7A: cmpi.b #0x8,(0x57,A0)
00012C80: bpl.w 0x00012c94
00012C84: cmp.b (0x82,A0),D0b
00012C88: bpl.w 0x00012cbe
00012C8C: move.b (0x82,A0),D7b
00012C90: bra.w 0x00012cc2
00012C94: cmpi.b #0xb,(0x57,A0)
00012C9A: bpl.w 0x00012cae
00012C9E: cmp.b (0x83,A0),D0b
00012CA2: bpl.w 0x00012cbe
00012CA6: move.b (0x83,A0),D7b
00012CAA: bra.w 0x00012cc2
00012CAE: cmp.b (0x84,A0),D0b
00012CB2: bpl.w 0x00012cbe
00012CB6: move.b (0x84,A0),D7b
00012CBA: bra.w 0x00012cc2
00012CBE: move.b (0x81,A0),D7b
00012CC2: cmp.w D6w,D7w
00012CC4: bpl.w 0x00012cca
00012CC8: move.w D7w,D6w
00012CCA: add.w D7w,D6w
00012CCC: bne.w 0x00012cd4
00012CD0: move.w #0x1,D6w
00012CD4: rts
00012CD6: movea.l #0xc69c,A2
00012CDC: lea (0xd19a8).l,A1 ; data 000d19a8
00012CE2: move.w #0x6000,D1w
00012CE6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012CEC: rts
00012CEE: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00012CF2: move.w #0x0,D1w
00012CF6: move.w #0x13,D7w
00012CFA: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
00012D00: jsr 0x00055ed6.l ; call 00055ED6 FUN_00055ed6
00012D06: clr.l D6
00012D08: tst.w D0w
00012D0A: bne.w 0x00012e12
00012D0E: cmpi.b #0x2,(0x00fff0d0).l
00012D16: bpl.w 0x00012d74
00012D1A: clr.l D7
00012D1C: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
00012D22: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012D26: cmpi.b #0x17,D0b
00012D2A: bne.w 0x00012d32
00012D2E: bra.w 0x00012dd2
00012D32: cmpi.b #0xd,D0b
00012D36: bmi.w 0x00012d60
00012D3A: cmpi.b #0x10,D0b
00012D3E: bmi.w 0x00012d46
00012D42: bra.w 0x00012e98
00012D46: move.b (0x87,A0),D7b
00012D4A: cmp.w D6w,D7w
00012D4C: bpl.w 0x00012d52
00012D50: move.w D7w,D6w
00012D52: add.w D7w,D6w
00012D54: bne.w 0x00012dfe
00012D58: move.w #0x1,D6w
00012D5C: bra.w 0x00012dfe
00012D60: move.b (0x57,A0),-(SP) ; data Stack[-0x3a]
00012D64: move.b D0b,(0x57,A0)
00012D68: bsr.w 0x00012c74 ; call 00012C74 FUN_00012c74
00012D6C: move.b (SP)+,(0x57,A0)
00012D70: bra.w 0x00012dfe
00012D74: tst.b (0x56,A0)
00012D78: bpl.w 0x00012dc0
00012D7C: bsr.w 0x00012fa4 ; call 00012FA4 FUN_00012fa4
00012D80: jsr 0x00055f02.l ; call 00055F02 FUN_00055f02
00012D86: clr.l D7
00012D88: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
00012D8E: cmp.b (0x7f,A0),D6b
00012D92: ble.w 0x00012d9a
00012D96: move.b (0x7f,A0),D6b
00012D9A: add.b (0x9d,A0),D6b
00012D9E: clr.l D7
00012DA0: move.b (0x57,A0),D7b
00012DA4: subq.w #0x1,D7w
00012DA6: move.l D7,D5
00012DA8: addi.w #0x19,D5w
00012DAC: bsr.w 0x000104f6 ; call 000104F6 FUN_000104f6
00012DB0: lea (0xcf3d4).l,A1 ; data 000cf3d4
00012DB6: move.l #0xc6a4,D4
00012DBC: bra.w 0x00012e7a
00012DC0: clr.l D7
00012DC2: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
00012DC8: cmpi.b #0x7f,(0x56,A0)
00012DCE: bne.w 0x00012dec
00012DD2: move.b (0x85,A0),D7b
00012DD6: cmp.w D6w,D7w
00012DD8: bpl.w 0x00012dde
00012DDC: move.w D7w,D6w
00012DDE: add.w D7w,D6w
00012DE0: bne.w 0x00012dfe
00012DE4: move.w #0x1,D6w
00012DE8: bra.w 0x00012dfe
00012DEC: cmpi.b #0xd,(0x57,A0)
00012DF2: bmi.w 0x00012dfa
00012DF6: bra.w 0x00012d46
00012DFA: bsr.w 0x00012c74 ; call 00012C74 FUN_00012c74
00012DFE: bsr.w 0x00012cd6 ; call 00012CD6 FUN_00012cd6
00012E02: lea (0xcf3d4).l,A1 ; data 000cf3d4
00012E08: move.l #0xc6a4,D4
00012E0E: bra.w 0x00012e7a
00012E12: cmpi.b #0x2,(0x00fff0d0).l
00012E1A: bpl.w 0x00012e36
00012E1E: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012E22: cmpi.b #0x17,D0b
00012E26: beq.w 0x00012e36
00012E2A: cmpi.b #0x10,D0b
00012E2E: bmi.w 0x00012e36
00012E32: bra.w 0x00012e98
00012E36: moveq #-0x1,D7
00012E38: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
00012E3E: add.b (0x77,A0),D6b
00012E42: jsr 0x00055e62.l ; call 00055E62 FUN_00055e62
00012E48: movea.l A0,A1
00012E4A: adda.l #0x90,A1
00012E50: move.w (A1),D0w
00012E52: moveq #0x2,D7
00012E54: moveq #0x9,D5
00012E56: btst.l D5,D0
00012E58: beq.w 0x00012e64
00012E5C: subq.w #0x8,D5w
00012E5E: add.w D5w,D6w
00012E60: bra.w 0x00012e6a
00012E64: addq.w #0x1,D5w
00012E66: dbf D7w,0x00012e56
00012E6A: bsr.w 0x00012cd6 ; call 00012CD6 FUN_00012cd6
00012E6E: lea (0xcf610).l,A1 ; data 000cf610
00012E74: move.l #0xc724,D4
00012E7A: subq.w #0x1,D6w
00012E7C: bpl.w 0x00012e82
00012E80: clr.l D6
00012E82: mulu.w #0x1a,D6
00012E86: adda.l D6,A1
00012E88: move.w #0xc,D7w
00012E8C: clr.l D1
00012E8E: move.w #0x2000,D2w
00012E92: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012E98: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00012E9C: rts
00012E9E: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00012EA2: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00012EA6: move.w D0w,D7w
00012EA8: subq.w #0x1,D7w
00012EAA: bra.w 0x00012f50 ; call 00012F50 FUN_00012f50
00012EAE: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00012EB2: clr.l D7
00012EB4: move.b (0x57,A0),D7b
00012EB8: subq.w #0x1,D7w
00012EBA: bpl.w 0x00012f00
00012EBE: move.w (0x90,A0),D1w
00012EC2: btst.l #0x3,D1
00012EC6: beq.w 0x00012ed2
00012ECA: move.w #0x1,D6w
00012ECE: bra.w 0x00012f60
00012ED2: btst.l #0x2,D1
00012ED6: beq.w 0x00012ee2
00012EDA: move.w #0x0,D6w
00012EDE: bra.w 0x00012f60
00012EE2: lea (0xd1aa5).l,A1 ; data 000d1aa5
00012EE8: movea.l #0xc538,A2
00012EEE: move.w #0x6000,D1w
00012EF2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012EF8: move.w #0x1,D6w
00012EFC: bra.w 0x00012f60
00012F00: tst.b (0x56,A0)
00012F04: bpl.w 0x00012f50
00012F08: cmpi.b #0x2,(0x00fff0d0).l
00012F10: bmi.w 0x00012f50
00012F14: cmpi.b #0x4,D7b
00012F18: bne.w 0x00012f32
00012F1C: lea (0xd1aa5).l,A1 ; data 000d1aa5
00012F22: movea.l #0xc538,A2
00012F28: move.w #0x6000,D1w
00012F2C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012F32: bsr.w 0x00012fa4 ; call 00012FA4 FUN_00012fa4
00012F36: lea (0xd4ea0).l,A4 ; data 000d4ea0
00012F3C: clr.l D6
00012F3E: mulu.w #0x30,D7
00012F42: move.b (0x25,A4,D7w*0x1),D6b ; data 000d4ec5
00012F46: bpl.w 0x00012f60
00012F4A: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00012F4E: rts
00012F50: lea (0xd4540).l,A4 ; data 000d4540
00012F56: clr.l D6
00012F58: mulu.w #0x30,D7
00012F5C: move.b (0x25,A4,D7w*0x1),D6b ; data 000d4565
00012F60: bsr.w 0x00013000 ; call 00013000 FUN_00013000
00012F64: lea (0xcf84c).l,A3 ; data 000cf84c
00012F6A: movea.l #0xc51c,A2
00012F70: lea (0xd19b0).l,A1 ; data 000d19b0
00012F76: move.w #0x6000,D1w
00012F7A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012F80: exg A1,A3
00012F82: mulu.w #0xe,D6
00012F86: adda.l D6,A1
00012F88: move.l #0xc52a,D4
00012F8E: move.w #0x2000,D2w
00012F92: move.w #0x6,D7w
00012F96: clr.l D1
00012F98: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012F9E: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00012FA2: rts
00012FA4: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00012FA8: lea (0xd19b7).l,A1 ; data 000d19b7
00012FAE: movea.l #0xc49c,A2
00012FB4: move.w #0x6000,D1w
00012FB8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00012FBE: clr.l D4
00012FC0: move.b (0x9d,A0),D4b
00012FC4: clr.l D7
00012FC6: move.b (0x57,A0),D7b
00012FCA: subq.w #0x1,D7w
00012FCC: jsr 0x00010660.l ; call 00010660 FUN_00010660
00012FD2: ble.w 0x00012ffa
00012FD6: lea (0xcf8f0).l,A1 ; data 000cf8f0
00012FDC: subq.w #0x1,D2w
00012FDE: mulu.w #0xe,D2
00012FE2: adda.l D2,A1
00012FE4: move.l #0xc4aa,D4
00012FEA: move.w #0x2000,D2w
00012FEE: move.w #0x6,D7w
00012FF2: clr.l D1
00012FF4: jsr 0x00001232.l ; call 00001232 FUN_00001232
00012FFA: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00012FFE: rts
00013000: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00013004: lea (0xd1f58).l,A1 ; data 000d1f58
0001300A: movea.l #0xc59c,A2
00013010: move.w #0x6000,D1w
00013014: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001301A: movea.l #0xc5aa,A2
00013020: cmpi.b #0x2,(0x00fff0d0).l
00013028: bmi.w 0x0001304e
0001302C: clr.l D0
0001302E: movea.l (0x00fffc04).l,A0
00013034: tst.b (0x56,A0)
00013038: bpl.w 0x00013046
0001303C: clr.l D6
0001303E: move.b (0x9d,A0),D6b
00013042: bra.w 0x00013094
00013046: move.b (0x57,A0),D0b
0001304A: bra.w 0x00013052
0001304E: bsr.w 0x00012148 ; call 00012148 FUN_00012148
00013052: clr.l D6
00013054: subq.w #0x1,D0w
00013056: bpl.w 0x00013084
0001305A: move.b (0x79,A0),D6b
0001305E: movea.l A0,A3
00013060: adda.l #0x90,A3
00013066: move.w (A3),D0w
00013068: moveq #0x3,D7
0001306A: moveq #0x5,D4
0001306C: btst.l D4,D0
0001306E: beq.w 0x0001307a
00013072: subq.w #0x4,D4w
00013074: add.w D4w,D6w
00013076: bra.w 0x00013080
0001307A: addq.w #0x1,D4w
0001307C: dbf D7w,0x0001306c
00013080: bra.w 0x00013094
00013084: lea (0xd4540).l,A4 ; data 000d4540
0001308A: clr.l D6
0001308C: mulu.w #0x30,D0
00013090: move.b (0x26,A4,D0w*0x1),D6b ; data 000d4566
00013094: subq.w #0x1,D6w
00013096: bpl.w 0x0001309c
0001309A: clr.l D6
0001309C: cmpi.w #0xd,D6w
000130A0: bmi.w 0x000130a8
000130A4: move.w #0xc,D6w
000130A8: mulu.w #0xc,D6
000130AC: lea (0xd1f5e).l,A1 ; data 000d1f5e
000130B2: adda.l D6,A1
000130B4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000130BA: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
000130BE: rts
000130C0: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
000130C4: clr.l D0
000130C6: move.b (A0),D0b
000130C8: move.l D0,-(SP) ; data Stack[-0x40]
000130CA: addq.w #0x1,D0w
000130CC: bsr.w 0x00012a54 ; call 00012A54 FUN_00012a54
000130D0: move.l (SP),D0 ; data Stack[-0x40]
000130D2: mulu.w #0x30,D0
000130D6: lea (0xd4540).l,A4 ; data 000d4540
000130DC: movea.l (0x1e,A4,D0w*0x1),A1 ; data 000d455e
000130E0: move.w #-0x3de6,D4w
000130E4: move.w #0x8,D7w
000130E8: move.w #0x4,D1w
000130EC: move.w #0x2000,D2w
000130F0: jsr 0x00001232.l ; call 00001232 FUN_00001232
000130F6: clr.l D3
000130F8: move.b (0x24,A4,D0w*0x1),D3b ; data 000d4564
000130FC: move.l (SP)+,D0
000130FE: cmpi.b #0xc,D0b
00013102: bpl.w 0x000131ae
00013106: move.w #0x3,D7w
0001310A: move.w #0x1,D1w
0001310E: move.w #0x2000,D2w
00013112: btst.b #0x1,(0x1,A0)
00013118: beq.w 0x0001312c
0001311C: lea (0xd25ec).l,A1 ; data 000d25ec
00013122: move.w #-0x3dee,D4w
00013126: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001312C: btst.b #0x2,(0x1,A0)
00013132: beq.w 0x00013146
00013136: lea (0xd25fc).l,A1 ; data 000d25fc
0001313C: move.w #-0x3dee,D4w
00013140: jsr 0x00001232.l ; call 00001232 FUN_00001232
00013146: btst.b #0x0,(0x1,A0)
0001314C: beq.w 0x00013160
00013150: lea (0xd25dc).l,A1 ; data 000d25dc
00013156: move.w #-0x3dd4,D4w
0001315A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00013160: btst.b #0x3,(0x1,A0)
00013166: beq.w 0x0001317a
0001316A: lea (0xd260c).l,A1 ; data 000d260c
00013170: move.w #-0x3dd4,D4w
00013174: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001317A: btst.b #0x4,(0x1,A0)
00013180: beq.w 0x00013194
00013184: lea (0xd261c).l,A1 ; data 000d261c
0001318A: move.w #-0x3c54,D4w
0001318E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00013194: btst.b #0x5,(0x1,A0)
0001319A: beq.w 0x000131ae
0001319E: lea (0xd262c).l,A1 ; data 000d262c
000131A4: move.w #-0x3c54,D4w
000131A8: jsr 0x00001232.l ; call 00001232 FUN_00001232
000131AE: cmpi.b #0xc,D0b
000131B2: bmi.w 0x000131d6
000131B6: cmpi.b #0x13,D0b
000131BA: bmi.w 0x000131e0
000131BE: beq.w 0x000131ea
000131C2: cmpi.b #0x17,D0b
000131C6: bmi.w 0x00013222
000131CA: cmpi.b #0x28,D0b
000131CE: bmi.w 0x000131ea
000131D2: bra.w 0x00013222
000131D6: lea (0xd29c4).l,A1 ; data 000d29c4
000131DC: bra.w 0x000131f4
000131E0: lea (0xd29ca).l,A1 ; data 000d29ca
000131E6: bra.w 0x000131f0
000131EA: lea (0xd29d0).l,A1 ; data 000d29d0
000131F0: move.b (0x1,A0),D3b
000131F4: movea.w #-0x3c6e,A2
000131F8: move.w #0x6000,D1w
000131FC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00013202: move.w #-0x3bec,D4w
00013206: lea (0x6ac46).l,A1 ; data 0006ac46
0001320C: lsl.w #0x2,D3w
0001320E: adda.l D3,A1
00013210: move.w #0x1,D7w
00013214: move.w #0x0,D1w
00013218: move.w #0x2000,D2w
0001321C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00013222: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00013226: rts
00013228: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0001322C: clr.l D0
0001322E: clr.l D6
00013230: move.b (0x00fff0d0).l,D6b
00013236: movea.l (0x00fffc04).l,A0
0001323C: move.b (0x66,A0,D6w*0x1),D0b
00013240: movem.l {  D6 D0},-(SP)
00013244: bsr.w 0x00012a54 ; call 00012A54 FUN_00012a54
00013248: subq.w #0x1,D0w
0001324A: mulu.w #0x30,D0
0001324E: lea (0xd4540).l,A4 ; data 000d4540
00013254: movea.l (0x1e,A4,D0w*0x1),A1 ; data 000d455e
00013258: move.w #-0x3de6,D4w
0001325C: move.w #0x8,D7w
00013260: move.w #0x4,D1w
00013264: move.w #0x2000,D2w
00013268: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001326E: clr.l D3
00013270: move.b (0x24,A4,D0w*0x1),D3b ; data 000d4564
00013274: movem.l (SP)+,{ D0 D6}
00013278: cmpi.b #0xd,D0b
0001327C: bpl.w 0x00013328
00013280: move.w #0x3,D7w
00013284: move.w #0x1,D1w
00013288: move.w #0x2000,D2w
0001328C: btst.b #0x1,(0x6e,A0,D6w*0x1)
00013292: beq.w 0x000132a6
00013296: lea (0xd25ec).l,A1 ; data 000d25ec
0001329C: move.w #-0x3dee,D4w
000132A0: jsr 0x00001232.l ; call 00001232 FUN_00001232
000132A6: btst.b #0x2,(0x6e,A0,D6w*0x1)
000132AC: beq.w 0x000132c0
000132B0: lea (0xd25fc).l,A1 ; data 000d25fc
000132B6: move.w #-0x3dee,D4w
000132BA: jsr 0x00001232.l ; call 00001232 FUN_00001232
000132C0: btst.b #0x0,(0x6e,A0,D6w*0x1)
000132C6: beq.w 0x000132da
000132CA: lea (0xd25dc).l,A1 ; data 000d25dc
000132D0: move.w #-0x3dd4,D4w
000132D4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000132DA: btst.b #0x3,(0x6e,A0,D6w*0x1)
000132E0: beq.w 0x000132f4
000132E4: lea (0xd260c).l,A1 ; data 000d260c
000132EA: move.w #-0x3dd4,D4w
000132EE: jsr 0x00001232.l ; call 00001232 FUN_00001232
000132F4: btst.b #0x4,(0x6e,A0,D6w*0x1)
000132FA: beq.w 0x0001330e
000132FE: lea (0xd261c).l,A1 ; data 000d261c
00013304: move.w #-0x3c54,D4w
00013308: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001330E: btst.b #0x5,(0x6e,A0,D6w*0x1)
00013314: beq.w 0x00013328
00013318: lea (0xd262c).l,A1 ; data 000d262c
0001331E: move.w #-0x3c54,D4w
00013322: jsr 0x00001232.l ; call 00001232 FUN_00001232
00013328: cmpi.b #0xd,D0b
0001332C: bmi.w 0x00013350
00013330: cmpi.b #0x14,D0b
00013334: bmi.w 0x0001335a
00013338: beq.w 0x00013364
0001333C: cmpi.b #0x18,D0b
00013340: bmi.w 0x000133a0
00013344: cmpi.b #0x29,D0b
00013348: bmi.w 0x00013364
0001334C: bra.w 0x000133a0
00013350: lea (0xd29c4).l,A1 ; data 000d29c4
00013356: bra.w 0x00013372
0001335A: lea (0xd29ca).l,A1 ; data 000d29ca
00013360: bra.w 0x0001336a
00013364: lea (0xd29d0).l,A1 ; data 000d29d0
0001336A: bsr.w 0x00012148 ; call 00012148 FUN_00012148
0001336E: move.b (0x6e,A0,D6w*0x1),D3b
00013372: movea.w #-0x3c6e,A2
00013376: move.w #0x6000,D1w
0001337A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00013380: move.w #-0x3bec,D4w
00013384: lea (0x6ac46).l,A1 ; data 0006ac46
0001338A: lsl.w #0x2,D3w
0001338C: adda.l D3,A1
0001338E: move.w #0x1,D7w
00013392: move.w #0x0,D1w
00013396: move.w #0x2000,D2w
0001339A: jsr 0x00001232.l ; call 00001232 FUN_00001232
000133A0: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
000133A4: rts
000133A6: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
000133AA: move.w D0w,D1w
000133AC: bpl.w 0x000133e4
000133B0: movea.l (0x00fffc04).l,A0
000133B6: lsl.w #0x1,D1w
000133B8: bpl.w 0x000133d4
000133BC: add.b D0b,(0x4f,A0)
000133C0: cmpi.b #0x15,(0x4f,A0)
000133C6: bmi.w 0x0001345e
000133CA: move.b #0x14,(0x4f,A0)
000133D0: bra.w 0x00013432
000133D4: andi.w #0xfff,D0w
000133D8: ext.l D0
000133DA: add.l D0,(0x00fffbfe).l
000133E0: bra.w 0x0001345e
000133E4: cmpi.w #0x64,D0w
000133E8: bne.w 0x00013400
000133EC: move.l (0x00fffc22).l,D1
000133F2: bset.l #0x4,D1
000133F6: move.l D1,(0x00fffc22).l
000133FC: bra.w 0x0001345e
00013400: cmpi.w #0x65,D0w
00013404: bne.w 0x0001341c
00013408: move.l (0x00fffc22).l,D1
0001340E: bset.l #0x3,D1
00013412: move.l D1,(0x00fffc22).l
00013418: bra.w 0x0001345e
0001341C: movea.l (0x00fffc04).l,A0
00013422: moveq #0x7,D7
00013424: tst.b (0x66,A0)
00013428: beq.w 0x0001343c
0001342C: addq.l #0x1,A0
0001342E: dbf D7w,0x00013424
00013432: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00013436: move.w #-0x1,D7w
0001343A: rts
0001343C: addq.w #0x1,D0w
0001343E: move.b D0b,(0x66,A0)
00013442: clr.b (0x6e,A0)
00013446: cmpi.w #0xd,D0w
0001344A: bmi.w 0x0001345e
0001344E: move.b #0x5,D1b
00013452: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00013458: addq.w #0x2,D0w
0001345A: move.b D0b,(0x6e,A0)
0001345E: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00013462: clr.w D7w
00013464: rts
00013466: clr.l D1
00013468: move.b D0b,(0x57,A0)
0001346C: bne.w 0x00013482
00013470: clr.b (0x57,A0)
00013474: clr.b (0x9d,A0)
00013478: move.b (0x0000007f).w,(0x56,A0)
0001347E: bra.w 0x000134c4
00013482: move.b D7b,(0x56,A0)
00013486: move.b (0x4e,A0),D1b
0001348A: beq.w 0x000134c4
0001348E: clr.b (0x4e,A0)
00013492: clr.b (0x9d,A0)
00013496: subq.w #0x1,D0w
00013498: mulu.w #0x30,D0
0001349C: lea (0xd4540).l,A4 ; data 000d4540
000134A2: adda.l D0,A4
000134A4: clr.l D0
000134A6: move.b (0x24,A4),D0b ; data 000d4564
000134AA: mulu.w D0w,D1
000134AC: divu.w #0x64,D1
000134B0: move.l D1,D3
000134B2: swap D3
000134B4: andi.l #0xffff,D3
000134BA: cmpi.w #0x32,D3w
000134BE: bmi.w 0x000134c4
000134C2: addq.w #0x1,D1w
000134C4: jsr 0x000036fc.l ; call 000036FC FUN_000036fc
000134CA: jsr 0x00003404.l ; call 00003404 FUN_00003404
000134D0: rts
000134D2: clr.l D0
000134D4: move.b (0x00fffbf8).l,D0b
000134DA: lea (0xd4456).l,A1 ; data 000d4456
000134E0: move.b (0x0,A1,D0w*0x1),(0x00fff09c).l
000134E8: lea (0xffbf40).l,A1 ; data 00ffbf40
000134EE: movea.l #0xd2a2e,A3
000134F4: move.l #0x6420,D0
000134FA: jsr 0x00001530.l ; call 00001530 FUN_00001530
00013500: lea (0xd4298).l,A1 ; data 000d4298
00013506: move.w #-0x1df8,D4w
0001350A: move.w #0x19,D7w
0001350E: move.w #0x10,D1w
00013512: move.w #0x0,D2w
00013516: clr.w D3w
00013518: move.w D7w,D5w
0001351A: addq.w #0x1,D5w
0001351C: jsr 0x00001728.l ; call 00001728 FUN_00001728
00013522: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00013528: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001352E: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00013534: tst.b (0x00fff09e).l
0001353A: bne.w 0x00013542
0001353E: bsr.w 0x0000e6f4 ; call 0000E6F4 FUN_0000e6f4
00013542: tst.b (0x00ffe18b).l
00013548: beq.w 0x000135d8
0001354C: move.b #0x4,(0x00fff09c).l
00013554: bsr.w 0x00013726 ; call 00013726 FUN_00013726
00013558: move.w #0x40,D7w
0001355C: move.w D7w,-(SP) ; data Stack[-0x2]
0001355E: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00013564: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001356A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00013570: tst.b (0x00fff09e).l
00013576: bne.w 0x0001357e
0001357A: bsr.w 0x0000e6f4 ; call 0000E6F4 FUN_0000e6f4
0001357E: move.w (SP)+,D7w
00013580: tst.w (0x00ffddc2).l
00013586: bne.w 0x000135d0
0001358A: dbf D7w,0x0001355c
0001358E: move.b #0x2,(0x00fff09c).l
00013596: bsr.w 0x00013726 ; call 00013726 FUN_00013726
0001359A: move.w #0x60,D7w
0001359E: move.w D7w,-(SP) ; data Stack[-0x2]
000135A0: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000135A6: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000135AC: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000135B2: tst.b (0x00fff09e).l
000135B8: bne.w 0x000135c0
000135BC: bsr.w 0x0000e6f4 ; call 0000E6F4 FUN_0000e6f4
000135C0: move.w (SP)+,D7w
000135C2: tst.w (0x00ffddc2).l
000135C8: bne.w 0x000135d0
000135CC: dbf D7w,0x0001359e
000135D0: clr.b (0x00fff09e).l
000135D6: rts
000135D8: bsr.w 0x00013726 ; call 00013726 FUN_00013726
000135DC: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000135E2: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000135E8: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000135EE: tst.b (0x00fff09e).l
000135F4: bne.w 0x000135fc
000135F8: bsr.w 0x0000e6f4 ; call 0000E6F4 FUN_0000e6f4
000135FC: btst.b #0x8,(0x00ffdd22).l
00013604: beq.w 0x0001360c
00013608: bra.w 0x00013618
0001360C: btst.b #0xb,(0x00ffdd22).l
00013614: beq.w 0x0001362e
00013618: subq.b #0x1,(0x00fff09c).l
0001361E: bpl.w 0x0001362a
00013622: move.b #0x6,(0x00fff09c).l
0001362A: bsr.w 0x00013726 ; call 00013726 FUN_00013726
0001362E: btst.b #0xa,(0x00ffdd22).l
00013636: beq.w 0x0001363e
0001363A: bra.w 0x0001364a
0001363E: btst.b #0x9,(0x00ffdd22).l
00013646: beq.w 0x00013666
0001364A: addq.b #0x1,(0x00fff09c).l
00013650: cmpi.b #0x7,(0x00fff09c).l
00013658: bmi.w 0x00013662
0001365C: clr.b (0x00fff09c).l
00013662: bsr.w 0x00013726 ; call 00013726 FUN_00013726
00013666: btst.b #0xe,(0x00ffdd22).l
0001366E: beq.w 0x00013684
00013672: jsr 0x00020714.l ; call 00020714 FUN_00020714
00013678: move.b #-0x1,(0x00fff0dc).l
00013680: bra.w 0x000136b6
00013684: btst.b #0xc,(0x00ffdd22).l
0001368C: beq.w 0x00013692
00013690: bra.b 0x00013672
00013692: btst.b #0xd,(0x00ffdd22).l
0001369A: beq.w 0x000136a4
0001369E: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
000136A4: btst.b #0xf,(0x00ffdd22).l
000136AC: beq.w 0x000135dc
000136B0: jsr 0x00010af0.l ; call 00010AF0 FUN_00010af0
000136B6: tst.b (0x00fff0e0).l
000136BC: bne.w 0x000136dc
000136C0: move.b (0x00fffbf8).l,D0b
000136C6: ext.w D0w
000136C8: cmp.w (0x00ffddc4).l,D0w
000136CE: beq.w 0x000136dc
000136D2: bsr.w 0x000136de ; call 000136DE FUN_000136de
000136D6: jsr 0x00053126.l ; call 00053126 FUN_00053126
000136DC: rts
000136DE: jsr 0x00053126.l ; call 00053126 FUN_00053126
000136E4: clr.w (0x00ffdf40).l
000136EA: move.b #-0x1,(0x00fff0a0).l
000136F2: move.w #-0x1,(0x00ffdf8e).l
000136FA: move.w D0w,(0x00ffddc4).l
00013700: clr.w (0x00fffc0e).l
00013706: cmpi.w #0x6,D0w
0001370A: bne.w 0x00013724
0001370E: clr.w (0x00fffc10).l
00013714: move.w #0x1,(0x00fffc0e).l
0001371C: move.w #0x1,(0x00ffe10e).l
00013724: rts
00013726: clr.l D0
00013728: move.b (0x00fff09c).l,D0b
0001372E: lea (0xd445d).l,A0 ; data 000d445d
00013734: move.b (0x0,A0,D0w*0x1),D0b ; data 000d445d
00013738: mulu.w #0x1e,D0
0001373C: lea (0xd4464).l,A0 ; data 000d4464
00013742: adda.l D0,A0
00013744: lea (0xc00000).l,A2 ; data 00c00000
0001374A: lea (0xd4536).l,A3 ; data 000d4536
00013750: moveq #0x4,D6
00013752: move SR,-(SP) ; data Stack[-0x2]
00013754: move #0x2700,SR
00013758: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
0001375E: lea (0xd4446).l,A1 ; data 000d4446
00013764: move.l #0xc208,D4
0001376A: clr.l D1
0001376C: clr.l D2
0001376E: clr.l D5
00013770: move.w (A0)+,D5w
00013772: move.b (A3)+,D1b
00013774: move.b (A3)+,D2b
00013776: move.l D4,D0
00013778: add.l D5,D0
0001377A: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00013780: move.w (A1)+,D0w
00013782: ori.w #0x0,D0w
00013786: move.w D0w,(A2)
00013788: move.w (A1)+,D0w
0001378A: ori.w #0x0,D0w
0001378E: move.w D1w,D7w
00013790: move.w D0w,(A2)
00013792: dbf D7w,0x00013790
00013796: move.w (A1)+,D0w
00013798: ori.w #0x0,D0w
0001379C: move.w D0w,(A2)
0001379E: move.w #-0x7080,(0x00c00004).l
000137A6: move.l D4,D0
000137A8: add.l D5,D0
000137AA: addi.l #0x80,D0
000137B0: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
000137B6: move.w (A1)+,D0w
000137B8: ori.w #0x0,D0w
000137BC: move.w D2w,D7w
000137BE: move.w D0w,(A2)
000137C0: dbf D7w,0x000137be
000137C4: move.l D4,D0
000137C6: add.l D5,D0
000137C8: addi.l #0x80,D0
000137CE: move.l D1,D3
000137D0: addq.l #0x2,D3
000137D2: mulu.w #0x2,D3
000137D6: add.l D3,D0
000137D8: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
000137DE: move.w (A1)+,D0w
000137E0: ori.w #0x0,D0w
000137E4: move.w D2w,D7w
000137E6: move.w D0w,(A2)
000137E8: dbf D7w,0x000137e6
000137EC: move.w #-0x70fe,(0x00c00004).l
000137F4: move.l D4,D0
000137F6: add.l D5,D0
000137F8: move.l D2,D3
000137FA: addq.l #0x2,D3
000137FC: mulu.w #0x80,D3
00013800: add.l D3,D0
00013802: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00013808: move.w (A1)+,D0w
0001380A: ori.w #0x0,D0w
0001380E: move.w D0w,(A2)
00013810: move.w (A1)+,D0w
00013812: ori.w #0x0,D0w
00013816: move.w D1w,D7w
00013818: move.w D0w,(A2)
0001381A: dbf D7w,0x00013818
0001381E: move.w (A1)+,D0w
00013820: ori.w #0x0,D0w
00013824: move.w D0w,(A2)
00013826: move (SP)+,SR
00013828: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP) ; data 000d4538
0001382C: moveq #0x1,D6
0001382E: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00013834: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001383A: tst.b (0x00fff09e).l
00013840: bne.w 0x00013848
00013844: bsr.w 0x0000e6f4 ; call 0000E6F4 FUN_0000e6f4
00013848: dbf D6w,0x0001382e
0001384C: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00013850: dbf D6w,0x00013752
00013854: movea.l A0,A1
00013856: movea.l #0xea10,A2
0001385C: move.w #0x6000,D1w
00013860: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00013866: adda.l #0x13,A1
0001386C: move.b (A1),(0x00fff0e0).l
00013872: rts
00013874: clr.b (0x00ffe175).l
0001387A: tst.w (0x00ffdfe8).l
00013880: beq.w 0x00013890
00013884: bsr.w 0x0001438c ; call 0001438C FUN_0001438c
00013888: bsr.w 0x00014446 ; call 00014446 FUN_00014446
0001388C: bra.w 0x000138b6
00013890: move.w (0x00ffdf88).l,D0w
00013896: andi.w #0x3f,D0w
0001389A: cmpi.w #0xb,D0w
0001389E: bne.w 0x000138aa
000138A2: bsr.w 0x0001438c ; call 0001438C FUN_0001438c
000138A6: bra.w 0x000138b6
000138AA: cmpi.w #0xf,D0w
000138AE: bne.w 0x000138b6
000138B2: bsr.w 0x00014446 ; call 00014446 FUN_00014446
000138B6: rts
000138B8: subq.w #0x1,(0xb6,A0)
000138BC: bmi.w 0x000138c2
000138C0: rts
000138C2: tst.b (0x00ffe175).l
000138C8: beq.w 0x000138ce
000138CC: rts
000138CE: move.b #-0x1,(0x00ffe175).l
000138D6: move.w #0x14,(0xb6,A0)
000138DC: tst.b (0x4d,A0)
000138E0: beq.w 0x000138ee
000138E4: tst.w (0x00ffdf84).l
000138EA: bne.w 0x000138f4
000138EE: bsr.w 0x00014508 ; call 00014508 FUN_00014508
000138F2: rts
000138F4: tst.w (0xdc,A0)
000138F8: bpl.w 0x00013900
000138FC: bsr.w 0x00013a92 ; call 00013A92 FUN_00013a92
00013900: bra.w 0x000139d8
00013904: subq.w #0x1,(0xb6,A0)
00013908: bmi.w 0x0001390e
0001390C: rts
0001390E: tst.b (0x00ffe175).l
00013914: beq.w 0x0001391a
00013918: rts
0001391A: move.w #0x14,(0xb6,A0)
00013920: cmpi.w #0xf,(0xe,A0)
00013926: beq.w 0x00013a4c
0001392A: btst.b #0x7,(0xb0,A0)
00013930: beq.w 0x0001393c
00013934: bra.w 0x00014508 ; call 00014508 FUN_00014508
0001393C: btst.b #0x3,(0x9e,A0)
00013942: beq.w 0x0001394a
00013946: rts
0001394A: btst.b #0x5,(0x9e,A0)
00013950: beq.w 0x00013962
00013954: tst.w (0x00ffdf84).l
0001395A: beq.w 0x00013962
0001395E: bra.w 0x00013e8c ; call 00013E8C FUN_00013e8c
00013962: tst.w (0xc,A0)
00013966: bne.w 0x0001397a
0001396A: btst.b #0x2,(0x9e,A0)
00013970: beq.w 0x0001397a
00013974: jmp 0x0001e514.l ; call 0001E514 FUN_0001e514
0001397A: btst.b #0x6,(0xb0,A0)
00013980: bne.w 0x00013e8c
00013984: tst.w (0xc,A0)
00013988: bne.w 0x000139be
0001398C: tst.w (0x00ffdf84).l
00013992: beq.w 0x000142ae
00013996: tst.b (0x00ffe186).l
0001399C: beq.w 0x000139b4
000139A0: btst.b #0x3,(0x10,A0)
000139A6: bne.w 0x000139b4
000139AA: cmpi.b #0x28,(0x4e,A0)
000139B0: beq.w 0x00013e40
000139B4: movea.l (0x00fffc04).l,A1
000139BA: bra.w 0x0001450e
000139BE: btst.b #0x6,(0x9e,A0)
000139C4: beq.w 0x000139ce
000139C8: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
000139CE: tst.w (0x00ffdf84).l
000139D4: beq.w 0x00013ef4
000139D8: move.l (0xac,A0),D0
000139DC: bne.w 0x000139f6
000139E0: bsr.w 0x00013ffe ; call 00013FFE FUN_00013ffe
000139E4: cmpi.l #0x12345678,D0
000139EA: beq.w 0x000144fa
000139EE: bra.w 0x00013a1c
000139F6: movea.l D0,A1
000139F8: move.w (A0),D5w
000139FA: move.w (0x4,A0),D4w
000139FE: move.w (A1),D3w
00013A00: move.w (0x4,A1),D2w
00013A04: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00013A0A: move.l A1,-(SP) ; data Stack[-0x4]
00013A0C: bsr.w 0x00013ffe ; call 00013FFE FUN_00013ffe
00013A10: movea.l (SP)+,A1
00013A12: cmpi.l #0x12345678,D0
00013A18: beq.w 0x000144fa
00013A1C: bra.w 0x00013d0c ; call 00013D0C FUN_00013d0c
00013A40: move.w (A1),(0x96,A0)
00013A44: move.w (0x4,A1),(0x98,A0)
00013A4A: rts
00013A4C: move.w (0x16,A0),D0w
00013A50: cmpi.w #0x3c,D0w
00013A54: ble.w 0x00013a5c
00013A58: bra.w 0x00013a8e
00013A5C: subq.b #0x1,(0x00ffe188).l
00013A62: bpl.w 0x00013a8e
00013A66: move.b #0x8,(0x00ffe188).l
00013A6E: bsr.w 0x00014492 ; call 00014492 FUN_00014492
00013A72: jsr 0x0000ae6e.l ; call 0000AE6E FUN_0000ae6e
00013A78: tst.b (0x00fff0dd).l
00013A7E: beq.w 0x00013a88
00013A82: jmp 0x0000afae.l ; call 0000AFAE prepare_entity_delayed_animation_alternate
00013A88: jmp 0x0000af7c.l ; call 0000AF7C prepare_entity_delayed_animation_primary
00013A8E: bra.w 0x00013e40
00013A92: cmpi.b #0x2,(0x9b,A0)
00013A98: bge.w 0x00013ae4
00013A9C: tst.b (0x56,A0)
00013AA0: bpl.w 0x00013aac
00013AA4: bsr.w 0x00013bba ; call 00013BBA FUN_00013bba
00013AA8: bcs.w 0x00013ac0
00013AAC: cmpi.b #0x5,(0x9c,A0)
00013AB2: blt.w 0x00013ae4
00013AB6: bsr.w 0x00013b12 ; call 00013B12 FUN_00013b12
00013ABA: bcs.w 0x00013ae6
00013ABE: rts
00013AC0: bsr.w 0x00013cd4 ; call 00013CD4 FUN_00013cd4
00013AC4: bcc.w 0x00013ae4
00013AC8: tst.b (0x56,A0)
00013ACC: bmi.w 0x00013ae0
00013AD0: move.b (0x57,A0),D0b
00013AD4: beq.w 0x00013ae0
00013AD8: cmpi.b #0xd,D0b
00013ADC: blt.w 0x00013ae4
00013AE0: bsr.w 0x00013ae6 ; call 00013AE6 FUN_00013ae6
00013AE4: rts
00013AE6: bsr.w 0x00013af2 ; call 00013AF2 FUN_00013af2
00013AEA: jsr 0x00013466.l ; call 00013466 FUN_00013466
00013AF0: rts
00013AF2: movea.l A0,A2
00013AF4: adda.w #0x66,A2
00013AF8: move.w #0x7,D1w
00013AFC: clr.w D7w
00013AFE: move.b (A2)+,D0b
00013B00: cmpi.w #0xd,D0w
00013B04: blt.w 0x00013b10
00013B08: addq.w #0x1,D7w
00013B0A: dbf D1w,0x00013afe
00013B0E: clr.w D0w
00013B10: rts
00013B12: move.w (0xa0,A0),D1w
00013B16: mulu.w #0x22,D1
00013B1A: lea (0xfff9fa).l,A2 ; data 00fff9fa
00013B20: adda.l D1,A2
00013B22: bsr.w 0x00013c74 ; call 00013C74 FUN_00013c74
00013B26: tst.w D3w
00013B28: beq.w 0x00013b4e
00013B2C: move.b #0xb,D7b
00013B30: move.b (0x00ffdd66).l,D4b
00013B36: movem.w {  D7w D4w},-(SP)
00013B3A: bsr.w 0x00013cde ; call 00013CDE FUN_00013cde
00013B3E: movem.w (SP)+,{  D4 D7}
00013B42: bcs.w 0x00013b4e
00013B46: bsr.w 0x00013c20 ; call 00013C20 FUN_00013c20
00013B4A: bcs.w 0x00013b7a
00013B4E: lea (0xffdd28).l,A3 ; data 00ffdd28
00013B54: move.w (A3)+,D1w
00013B56: beq.w 0x00013bb8
00013B5A: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00013B60: add.w D0w,D0w
00013B62: move.b (0x0,A3,D0w*0x1),D7b ; data 00ffdd2a
00013B66: move.b (0x1,A3,D0w*0x1),D4b ; data 00ffdd2b
00013B6A: movem.w {  D7w D4w},-(SP)
00013B6E: bsr.w 0x00013cde ; call 00013CDE FUN_00013cde
00013B72: movem.w (SP)+,{  D4 D7}
00013B76: bcs.w 0x00013bb8
00013B7A: move.b (0x57,A0),D0b
00013B7E: beq.w 0x00013b94
00013B82: cmpi.b #0xd,D0b
00013B86: bge.w 0x00013b94
00013B8A: jsr 0x000125fa.l ; call 000125FA FUN_000125fa
00013B90: move.b D1b,(0x4e,A0)
00013B94: move.b #-0x1,(0x56,A0)
00013B9A: addq.b #0x1,D7b
00013B9C: move.b D7b,(0x57,A0)
00013BA0: move.b D4b,(0x9d,A0)
00013BA4: clr.w D0w
00013BA6: jsr 0x0000abfc.l ; call 0000ABFC update_entity_animation_delay
00013BAC: jsr 0x00003404.l ; call 00003404 FUN_00003404
00013BB2: andi #0x1e,CCR
00013BB6: rts
00013BB8: rts
00013BBA: move.b (0x57,A0),D0b
00013BBE: cmpi.b #0x1,D0b
00013BC2: beq.w 0x00013bf2
00013BC6: cmpi.b #0x2,D0b
00013BCA: beq.w 0x00013bf2
00013BCE: cmpi.b #0x8,D0b
00013BD2: beq.w 0x00013bf2
00013BD6: cmpi.b #0x9,D0b
00013BDA: beq.w 0x00013bf2
00013BDE: cmpi.b #0xc,D0b
00013BE2: bne.w 0x00013c1a
00013BE6: bsr.w 0x00013c20 ; call 00013C20 FUN_00013c20
00013BEA: bcc.w 0x00013c1a
00013BEE: bra.w 0x00013c14
00013BF2: move.w (0xa0,A0),D1w
00013BF6: mulu.w #0x22,D1
00013BFA: lea (0xfff9fa).l,A2 ; data 00fff9fa
00013C00: adda.l D1,A2
00013C02: bsr.w 0x00013c74 ; call 00013C74 FUN_00013c74
00013C06: tst.w D3w
00013C08: beq.w 0x00013c14
00013C0C: bsr.w 0x00013c20 ; call 00013C20 FUN_00013c20
00013C10: bcs.w 0x00013c1a
00013C14: ori #0x1,CCR
00013C18: rts
00013C1A: andi #0x1e,CCR
00013C1E: rts
00013C20: clr.w D0w
00013C22: cmpi.b #0x4,(0x00ff014c).l
00013C2A: bcs.w 0x00013c68
00013C2E: move.w #0x1,D0w
00013C32: tst.b (0x00fffc0c).l
00013C38: bmi.w 0x00013c48
00013C3C: cmpi.b #0x4,(0x00ff024c).l
00013C44: bcs.w 0x00013c68
00013C48: tst.b (0x00fffc0d).l
00013C4E: bmi.w 0x00013c62
00013C52: move.w #0x2,D0w
00013C56: cmpi.b #0x4,(0x00ff034c).l
00013C5E: bcs.w 0x00013c68
00013C62: andi #0x1e,CCR
00013C66: rts
00013C68: move.b D0b,(0x00ffe190).l
00013C6E: ori #0x1,CCR
00013C72: rts
00013C74: move.w (A2)+,D1w
00013C76: lea (0xffdd2a).l,A3 ; data 00ffdd2a
00013C7C: clr.w D2w
00013C7E: clr.w D3w
00013C80: move.b (A2)+,D0b
00013C82: cmpi.b #0x0,D0b
00013C86: beq.w 0x00013cb6
00013C8A: cmpi.b #0x1,D0b
00013C8E: beq.w 0x00013cb6
00013C92: cmpi.b #0x7,D0b
00013C96: beq.w 0x00013cb6
00013C9A: cmpi.b #0x8,D0b
00013C9E: beq.w 0x00013cb6
00013CA2: cmpi.b #0xb,D0b
00013CA6: bne.w 0x00013cc0
00013CAA: addq.w #0x1,D3w
00013CAC: move.b (A2)+,(0x00ffdd66).l
00013CB2: bra.w 0x00013cc4
00013CB6: addq.w #0x1,D2w
00013CB8: move.b D0b,(A3)+
00013CBA: move.b (A2)+,(A3)+
00013CBC: bra.w 0x00013cc4
00013CC0: adda.w #0x1,A2
00013CC4: dbf D1w,0x00013c80
00013CC8: move.w D2w,(0x00ffdd28).l
00013CCE: move.w #-0x1,(A3)
00013CD2: rts
00013CD4: move.b (0x9d,A0),D4b
00013CD8: move.b (0x57,A0),D7b
00013CDC: subq.w #0x1,D7w
00013CDE: cmpi.b #0x1,(0x9c,A0)
00013CE4: beq.w 0x00013d06
00013CE8: ext.w D4w
00013CEA: ext.w D7w
00013CEC: jsr 0x00010660.l ; call 00010660 FUN_00010660
00013CF2: tst.w D2w
00013CF4: bmi.w 0x00013d00
00013CF8: cmp.b (0x9c,A0),D2b
00013CFC: bcc.w 0x00013d06
00013D00: andi #0x1e,CCR
00013D04: rts
00013D06: ori #0x1,CCR
00013D0A: rts
00013D0C: tst.b (0x00ffe186).l
00013D12: beq.w 0x00013d2a
00013D16: btst.b #0x3,(0x10,A0)
00013D1C: bne.w 0x00013d2a
00013D20: cmpi.b #0x28,(0x4e,A0)
00013D26: beq.w 0x00013e40
00013D2A: tst.w (0x00ffdfe0).l
00013D30: beq.w 0x00013d56
00013D34: cmpi.w #0x2,(0x00ffdfe0).l
00013D3C: beq.w 0x00013d42
00013D40: rts
00013D42: move.l (0xa2,A0),D0
00013D46: bne.w 0x00013d4c
00013D4A: rts
00013D4C: movea.l D0,A1
00013D4E: bsr.w 0x00013a40 ; call 00013A40 FUN_00013a40
00013D52: bra.w 0x000145a2 ; call 000145A2 FUN_000145a2
00013D56: move.l (0xa2,A0),D0
00013D5A: bne.w 0x00013d60
00013D5E: rts
00013D60: movea.l D0,A1
00013D62: move.w (A0),D5w
00013D64: move.w (0x4,A0),D4w
00013D68: move.w (A1),D3w
00013D6A: move.w (0x4,A1),D2w
00013D6E: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00013D74: bclr.b #0x4,(0xb0,A0)
00013D7A: btst.b #0x3,(0x10,A0)
00013D80: beq.w 0x00013d94
00013D84: lea (0x13e58).l,A4 ; data 00013e58
00013D8A: clr.w D1w
00013D8C: move.b (0x4d,A0),D1b
00013D90: bra.w 0x00013db0
00013D94: tst.b (0xb5,A0)
00013D98: beq.w 0x00013dca
00013D9C: subq.b #0x1,(0xb5,A0)
00013DA0: bsr.w 0x00014d6a ; call 00014D6A FUN_00014d6a
00013DA4: clr.w D1w
00013DA6: move.b (0x27,A2),D1b
00013DAA: lea (0x13e7c).l,A4 ; data 00013e7c
00013DB0: lsl.w #0x2,D1w
00013DB2: move.w (0x0,A4,D1w*0x1),D0w ; data 00013e58
00013DB6: move.w (0x2,A4,D1w*0x1),D1w ; data 00013e5a
00013DBA: cmp.w D0w,D3w
00013DBC: bcs.w 0x00013de2
00013DC0: cmp.w D1w,D3w
00013DC2: bcc.w 0x00013e2a
00013DC6: clr.b (0xb5,A0)
00013DCA: bset.b #0x4,(0xb0,A0)
00013DD0: tst.w (0x00ffdfde).l
00013DD6: bne.w 0x00013dde
00013DDA: bra.w 0x00013e0c
00013DDE: bra.w 0x00013e40
00013DE2: bra.b 0x00013dca
00013E0C: addi.w #0x4000,D2w
00013E10: lsl.w #0x1,D3w
00013E12: jsr 0x00015dcc.l ; call 00015DCC FUN_00015dcc
00013E18: add.w (A0),D3w
00013E1A: add.w (0x4,A0),D2w
00013E1E: move.w D3w,(0x96,A0)
00013E22: move.w D2w,(0x98,A0)
00013E26: bra.w 0x00013e34
00013E2A: move.w (A1),(0x96,A0)
00013E2E: move.w (0x4,A1),(0x98,A0)
00013E34: bsr.w 0x000144e8 ; call 000144E8 FUN_000144e8
00013E38: bcc.w 0x000145a2
00013E3C: clr.b (0xb5,A0)
00013E40: move.w (0x16,A0),D0w
00013E44: cmpi.w #0x3c,D0w
00013E48: ble.w 0x00013e4e
00013E4C: rts
00013E4E: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
00013E56: rts
00013E8C: tst.w (0xc,A0)
00013E90: bne.w 0x00013e96
00013E94: bra.b 0x00013e40
00013E96: bsr.w 0x00013e9e ; call 00013E9E FUN_00013e9e
00013E9A: bra.w 0x000145a2 ; call 000145A2 FUN_000145a2
00013E9E: bset.b #0x6,(0xb0,A0)
00013EA4: beq.w 0x00013eaa
00013EA8: rts
00013EAA: move.w #0x2580,D2w
00013EAE: move.w (0x00ffddce).l,D0w
00013EB4: lsr.w #0x1,D0w
00013EB6: move.w (0x00ffddc6).l,D1w
00013EBC: addi.w #0xa00,D1w
00013EC0: cmp.w D1w,D0w
00013EC2: bcc.w 0x00013ec8
00013EC6: neg.w D2w
00013EC8: add.w D1w,D2w
00013ECA: move.w #0x2580,D3w
00013ECE: move.w (0x00ffddd0).l,D0w
00013ED4: lsr.w #0x1,D0w
00013ED6: move.w (0x00ffddc8).l,D4w
00013EDC: addi.w #0xa00,D4w
00013EE0: cmp.w D4w,D0w
00013EE2: bcc.w 0x00013ee8
00013EE6: neg.w D3w
00013EE8: add.w D4w,D3w
00013EEA: move.w D2w,(0x96,A0)
00013EEE: move.w D3w,(0x98,A0)
00013EF2: rts
00013EF4: bsr.w 0x00013f76 ; call 00013F76 FUN_00013f76
00013EF8: cmpi.b #0x2,(0x4d,A0)
00013EFE: ble.w 0x000142ae
00013F02: cmpi.b #0x4,(0x4d,A0)
00013F08: bne.w 0x00013f36
00013F0C: move.w (A0),D0w
00013F0E: move.w (0x4,A0),D1w
00013F12: movea.l (0x00fffc04).l,A1
00013F18: move.w (A1),D2w
00013F1A: move.w (0x4,A1),D3w
00013F1E: jsr 0x00015ca4.l ; call 00015CA4 compute_diagonal_step_cost
00013F24: cmpi.w #0x500,D0w
00013F28: bcc.w 0x000142ae
00013F2C: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00013F32: bra.w 0x000139d8
00013F36: bgt.w 0x000139d8
00013F3A: move.w (A0),D5w
00013F3C: move.w (0x4,A0),D4w
00013F40: movea.l (0x00fffc04).l,A1
00013F46: move.w (A1),D3w
00013F48: move.w (0x4,A1),D2w
00013F4C: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00013F52: cmpi.w #0x640,D3w
00013F56: bcc.w 0x000142ae
00013F5A: move.w (0x16,A0),D0w
00013F5E: cmpi.w #0x3c,D0w
00013F62: ble.w 0x00013f68
00013F66: rts
00013F68: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
00013F70: move.w D2w,(0x2e,A0)
00013F74: rts
00013F76: tst.w (0x00ffdf84).l
00013F7C: bne.w 0x00013fa4
00013F80: tst.b (0x00ffe16f).l
00013F86: beq.w 0x00013f90
00013F8A: jmp 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00013F90: btst.b #0x0,(0x9e,A0)
00013F96: bne.w 0x00013fa4
00013F9A: btst.b #0x5,(0x10,A0)
00013FA0: bne.w 0x00013fa6
00013FA4: rts
00013FA6: movea.l (0x00fffc04).l,A1
00013FAC: jsr 0x00014d8e.l ; call 00014D8E FUN_00014d8e
00013FB2: bcs.b 0x00013fa4
00013FB4: move.w (0x00ff13ae).l,D0w
00013FBA: move.w (0x2e,A0),D1w
00013FBE: sub.w D0w,D1w
00013FC0: bpl.w 0x00013fc6
00013FC4: neg.w D1w
00013FC6: cmpi.w #0x4000,D1w
00013FCA: bcc.b 0x00013fa4
00013FCC: cmpi.w #0x2,(0x00ffdf40).l
00013FD4: bne.w 0x00013ff6
00013FD8: movea.l (0x00fffc04).l,A1
00013FDE: btst.b #0x7,(0x11,A1)
00013FE4: bne.b 0x00013fa4
00013FE6: move.l #0x20fec,(0x00ffdd2c).l
00013FF0: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
00013FF6: bset.b #0x0,(0x9e,A0)
00013FFC: rts
00013FFE: clr.w (0x00ffdfe0).l
00014004: bsr.w 0x00014492 ; call 00014492 FUN_00014492
00014008: move.l (0xa2,A0),D0
0001400C: bne.w 0x00014020
00014010: btst.b #0x3,(0x10,A0)
00014016: beq.w 0x0001401e
0001401A: bra.w 0x00014508 ; call 00014508 FUN_00014508
0001401E: rts
00014020: movea.l (0xa2,A0),A1
00014024: tst.b (0x57,A0)
00014028: beq.w 0x000141a0
0001402C: tst.w (0xdc,A0)
00014030: bpl.w 0x00014146
00014034: tst.w (0xc,A0)
00014038: beq.w 0x00014070
0001403C: tst.b (0x56,A0)
00014040: bmi.w 0x000140a0
00014044: cmpi.b #0xd,(0x57,A0)
0001404A: blt.w 0x00014058
0001404E: cmpi.b #0x10,(0x57,A0)
00014054: blt.w 0x00014078
00014058: tst.b (0x57,A0)
0001405C: beq.w 0x000141a0
00014060: jsr 0x00014d8e.l ; call 00014D8E FUN_00014d8e
00014066: bcc.w 0x0001411e
0001406A: clr.w (0x00ffdfde).l
00014070: move.l #0x12345678,D0
00014076: rts
00014078: jsr 0x00014d8e.l ; call 00014D8E FUN_00014d8e
0001407E: bcc.w 0x0001408a
00014082: move.l #0x12345678,D0
00014088: rts
0001408A: jsr 0x00015646.l ; call 00015646 FUN_00015646
00014090: bcc.w 0x00014096
00014094: rts
00014096: move.w (0x2e,A1),D0w
0001409A: jmp 0x0000b3fe.l ; call 0000B3FE set_entity_heading_with_alternate_animation
000140A0: cmpi.b #0x4,(0x57,A0)
000140A6: beq.w 0x0001411c
000140AA: cmpi.b #0xb,(0x57,A0)
000140B0: beq.w 0x0001411c
000140B4: jsr 0x00014d8e.l ; call 00014D8E FUN_00014d8e
000140BA: bcc.w 0x000140c6
000140BE: move.l #0x12345678,D0
000140C4: rts
000140C6: cmpi.b #0xc,(0x57,A0)
000140CC: bne.w 0x00014100
000140D0: move.l #0xff0100,D3
000140D6: move.b (0x00ffe190).l,D0b
000140DC: beq.w 0x000140fc
000140E0: move.l #0xff0200,D3
000140E6: cmpi.b #0x1,D0b
000140EA: beq.w 0x000140fc
000140EE: move.l #0xff0300,D3
000140F4: cmpi.b #0x2,D0b
000140F8: bne.w 0x00014100
000140FC: move.l D3,(0xa2,A0) ; data 00ff0300
00014100: jsr 0x0000ae6e.l ; call 0000AE6E FUN_0000ae6e
00014106: tst.b (0x00fff0dd).l
0001410C: beq.w 0x00014116
00014110: jmp 0x0000afae.l ; call 0000AFAE prepare_entity_delayed_animation_alternate
00014116: jmp 0x0000af7c.l ; call 0000AF7C prepare_entity_delayed_animation_primary
0001411C: rts
0001411E: tst.b (0x00ffe186).l
00014124: bne.w 0x00014134
00014128: btst.b #0x4,(0xb0,A0)
0001412E: bne.w 0x00014134
00014132: rts
00014134: move.w #0x1,(0x00ffdfde).l
0001413C: jsr 0x0001514e.l ; call 0001514E FUN_0001514e
00014142: bcc.w 0x000141e8
00014146: tst.b (0x4e,A0)
0001414A: beq.w 0x00014160
0001414E: jsr 0x00014d8e.l ; call 00014D8E FUN_00014d8e
00014154: bcc.w 0x0001415e
00014158: move.l #0x12345678,D0
0001415E: rts
00014160: btst.b #0x3,(0x10,A0)
00014166: beq.w 0x0001417c
0001416A: tst.b (0x4f,A0)
0001416E: beq.w 0x0001417c
00014172: tst.w (0xdc,A0)
00014176: bmi.w 0x0001419a
0001417A: rts
0001417C: move.b #0x0,(0x57,A0)
00014182: move.b #0x7f,(0x56,A0)
00014188: clr.w D0w
0001418A: jsr 0x0000abfc.l ; call 0000ABFC update_entity_animation_delay
00014190: jsr 0x00003404.l ; call 00003404 FUN_00003404
00014196: bra.w 0x000141a0
0001419A: jmp 0x00014226.l ; call 00014226 FUN_00014226
000141A0: bsr.w 0x00014c7c ; call 00014C7C FUN_00014c7c
000141A4: bcc.w 0x000141de
000141A8: move.w (A0),(0x96,A0)
000141AC: move.w (0x4,A0),(0x98,A0)
000141B2: tst.w (0xdc,A0)
000141B6: bmi.w 0x000141c6
000141BA: move.w #0x3,(0x00ffdfe0).l
000141C2: bra.w 0x00013e40
000141C6: move.w #0x1,(0x00ffdfe0).l
000141CE: btst.b #0x6,(0x11,A0)
000141D4: bne.w 0x00014220
000141D8: jmp 0x0000b1dc.l ; call 0000B1DC configure_entity_target_transition
000141DE: move.w #0x2,(0x00ffdfe0).l
000141E6: rts
000141E8: move.w (0x2e,A1),D0w
000141EC: jmp 0x0000b39a.l ; call 0000B39A set_entity_heading_with_field_animation
00014220: jmp 0x0000b11c.l ; call 0000B11C synchronize_entity_behavior_spawn
00014226: move.w #0x4b,D0w
0001422A: jsr 0x000530de.l ; call 000530DE FUN_000530de
00014230: move.b (0x57,A0),D0b
00014234: subq.b #0x1,D0b
00014236: ext.w D0w
00014238: cmpi.w #0x4,D0w
0001423C: beq.w 0x00014250
00014240: cmpi.w #0xa,D0w
00014244: beq.w 0x00014250
00014248: cmpi.w #0xb,D0w
0001424C: bne.w 0x00014278
00014250: move.w #0x2,D0w
00014254: move.b (0x56,A0),D1b
00014258: addi.b #0x6e,D1b
0001425C: btst.b #0x0,(0x0,A0,D1w*0x1)
00014262: beq.w 0x0001429a
00014266: btst.b #0x4,(0x91,A0)
0001426C: beq.w 0x0001429a
00014270: move.w #0x0,D1w
00014274: bra.w 0x0001429a
00014278: clr.w D0w
0001427A: move.b (0x56,A0),D1b
0001427E: addi.b #0x6e,D1b
00014282: btst.b #0x0,(0x0,A0,D1w*0x1)
00014288: beq.w 0x0001429a
0001428C: btst.b #0x4,(0x91,A0)
00014292: beq.w 0x0001429a
00014296: move.w #0x1,D1w
0001429A: jsr 0x0000abfc.l ; call 0000ABFC update_entity_animation_delay
000142A0: subq.b #0x1,(0x4f,A0)
000142A4: move.b #0x26,D1b
000142A8: jmp 0x000036fc.l ; call 000036FC FUN_000036fc
000142AE: btst.b #0x6,(0x9e,A0)
000142B4: beq.w 0x000142c2
000142B8: movea.l (0x00fffc04).l,A1
000142BE: bra.w 0x0001450e
000142C2: movea.l (0xa8,A0),A1
000142C6: bsr.w 0x000144e8 ; call 000144E8 FUN_000144e8
000142CA: bcc.w 0x000145a2
000142CE: btst.b #0x5,(0x10,A0)
000142D4: bne.w 0x000142dc
000142D8: bra.w 0x0001450e
000142DC: movea.l (0xe0,A0),A2
000142E0: move.w (A2),D0w
000142E2: bpl.w 0x000142f0
000142E6: movea.l (0x00fffc04).l,A1
000142EC: bra.w 0x0001450e
000142F0: move.w (0xe4,A0),D0w
000142F4: addq.w #0x4,D0w
000142F6: move.w (0x0,A2,D0w*0x1),D1w
000142FA: bne.w 0x00014300
000142FE: clr.w D0w
00014300: move.w D0w,(0xe4,A0)
00014304: move.w (0x0,A2,D0w*0x1),(0x96,A0)
0001430A: move.w (0x2,A2,D0w*0x1),(0x98,A0)
00014310: rts
0001438C: lea (0xff0400).l,A2 ; data 00ff0400
00014392: lea (0xffdf96).l,A4 ; data 00ffdf96
00014398: clr.w D1w
0001439A: move.w #0x6,D7w
0001439E: btst.b #0x0,(0x10,A2)
000143A4: beq.w 0x000143e6
000143A8: btst.b #0x6,(0x10,A2)
000143AE: bne.w 0x000143e6
000143B2: btst.b #0x4,(0x10,A2)
000143B8: bne.w 0x000143e6
000143BC: btst.b #0x7,(0xb0,A2)
000143C2: bne.w 0x000143e6
000143C6: btst.b #0x5,(0x9e,A2)
000143CC: bne.w 0x000143e6
000143D0: btst.b #0x3,(0x9e,A2)
000143D6: bne.w 0x000143e6
000143DA: tst.w (0xc,A2)
000143DE: beq.w 0x000143e6
000143E2: addq.w #0x1,D1w
000143E4: move.l A2,(A4)+
000143E6: adda.w #0x100,A2
000143EA: dbf D7w,0x0001439e
000143EE: move.w D1w,(0x00ffdf94).l
000143F4: rts
000143F6: lea (0xffdf94).l,A2 ; data 00ffdf94
000143FC: move.w (A2)+,D1w
000143FE: bne.w 0x0001441c
00014402: tst.b (0x00ffe162).l
00014408: bne.w 0x00014412
0001440C: jsr 0x0001dbf0.l ; call 0001DBF0 FUN_0001dbf0
00014412: move.l #0x0,(0xa2,A0)
0001441A: rts
0001441C: move.l (0xa2,A0),D2
00014420: move.w D1w,D7w
00014422: subq.w #0x1,D7w
00014424: cmp.l (A2)+,D2
00014426: beq.w 0x00014444
0001442A: dbf D7w,0x00014424
0001442E: lea (0xffdf96).l,A2 ; data 00ffdf96
00014434: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001443A: lsl.w #0x2,D0w
0001443C: movea.l (0x0,A2,D0w*0x1),A2 ; data 00ffdf96
00014440: move.l A2,(0xa2,A0)
00014444: rts
00014446: lea (0xff0100).l,A2 ; data 00ff0100
0001444C: lea (0xffdfca).l,A4 ; data 00ffdfca
00014452: clr.w D1w
00014454: move.w #0x2,D7w
00014458: btst.b #0x0,(0x10,A2)
0001445E: beq.w 0x00014482
00014462: btst.b #0x6,(0x10,A2)
00014468: bne.w 0x00014482
0001446C: btst.b #0x4,(0x10,A2)
00014472: bne.w 0x00014482
00014476: tst.w (0xc,A2)
0001447A: beq.w 0x00014482
0001447E: addq.w #0x1,D1w
00014480: move.l A2,(A4)+
00014482: adda.w #0x100,A2
00014486: dbf D7w,0x00014458
0001448A: move.w D1w,(0x00ffdfc8).l
00014490: rts
00014492: btst.b #0x3,(0x10,A0)
00014498: bne.w 0x000143f6
0001449C: move.l (0xa2,A0),D2
000144A0: lea (0xffdfc8).l,A2 ; data 00ffdfc8
000144A6: move.w (A2)+,D1w
000144A8: bne.w 0x000144b6
000144AC: move.l #0x0,(0xa2,A0)
000144B4: rts
000144B6: move.w D1w,D7w
000144B8: subq.w #0x1,D7w
000144BA: cmp.l (A2)+,D2
000144BC: beq.w 0x000144e6
000144C0: dbf D7w,0x000144ba
000144C4: lea (0xffdfca).l,A2 ; data 00ffdfca
000144CA: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000144D0: lsl.w #0x2,D0w
000144D2: movea.l (0x0,A2,D0w*0x1),A2 ; data 00ffdfca
000144D6: move.l A2,(0xa2,A0)
000144DA: cmpa.l D2,A2
000144DC: beq.w 0x000144e6
000144E0: move.b #0x6,(0xb5,A0)
000144E6: rts
000144E8: move.w (A0),D0w
000144EA: move.w (0x4,A0),D1w
000144EE: move.w (0x96,A0),D2w
000144F2: move.w (0x98,A0),D3w
000144F6: bra.w 0x0001460e ; call 0001460E FUN_0001460e
000144FA: move.l (0xa2,A0),D3
000144FE: beq.w 0x00014508
00014502: movea.l D3,A1
00014504: bra.w 0x0001450e
00014508: movea.l (0x00fffc04).l,A1
0001450E: move.w (0x2e,A1),D2w
00014512: move.w #0x20,D3w
00014516: move.w #0xe0,D3w
0001451A: move.w (0x3e,A0),D4w
0001451E: beq.w 0x00014538
00014522: cmpi.w #0x1,D4w
00014526: beq.w 0x00014532
0001452A: addi.w #0x6000,D2w
0001452E: bra.w 0x0001453c
00014532: tst.w (0x3e,A1)
00014536: bne.b 0x0001452a
00014538: subi.w #0x6000,D2w
0001453C: bsr.w 0x00015dcc ; call 00015DCC FUN_00015dcc
00014540: exg D2,D3
00014542: move.w (A1),D5w
00014544: move.w (0x4,A1),D6w
00014548: add.w D2w,D5w
0001454A: add.w D3w,D6w
0001454C: movem.w {  D6w D5w D4w D3w D2w},-(SP)
00014550: jsr 0x0000e7c8.l ; call 0000E7C8 FUN_0000e7c8
00014556: movem.w (SP)+,{  D2 D3 D4 D5 D6}
0001455A: tst.w D0w
0001455C: beq.w 0x00014590
00014560: bmi.w 0x00014590
00014564: neg.w D2w
00014566: neg.w D3w
00014568: move.w (A1),D5w
0001456A: move.w (0x4,A1),D6w
0001456E: add.w D2w,D5w
00014570: add.w D3w,D6w
00014572: movem.w {  D6w D5w D4w D3w D2w},-(SP)
00014576: jsr 0x0000e7c8.l ; call 0000E7C8 FUN_0000e7c8
0001457C: movem.w (SP)+,{  D2 D3 D4 D5 D6}
00014580: tst.w D0w
00014582: beq.w 0x00014590
00014586: bmi.w 0x00014590
0001458A: move.w (A1),D5w
0001458C: move.w (0x4,A1),D6w
00014590: move.w D5w,(0x96,A0)
00014594: move.w D6w,(0x98,A0)
00014598: bsr.w 0x000144e8 ; call 000144E8 FUN_000144e8
0001459C: bcc.w 0x000145a2
000145A0: rts
000145A2: move.w (0x16,A0),D0w
000145A6: cmpi.w #0x3c,D0w
000145AA: ble.w 0x000145b0
000145AE: rts
000145B0: btst.b #0x1,(0xb0,A0)
000145B6: bne.w 0x000149c0
000145BA: bclr.b #0x2,(0xb0,A0)
000145C0: beq.w 0x000145cc
000145C4: bsr.w 0x00014c2e ; call 00014C2E derive_entity_axis_relation_flags
000145C8: bra.w 0x000145e8
000145CC: cmpi.w #0x1000,(0xb8,A0)
000145D2: beq.w 0x000145e8
000145D6: bsr.w 0x00014ba6 ; call 00014BA6 FUN_00014ba6
000145DA: btst.b #0x1,(0xb0,A0)
000145E0: bne.w 0x000149c0
000145E4: bsr.w 0x00014c2e ; call 00014C2E derive_entity_axis_relation_flags
000145E8: move.w (A0),D5w
000145EA: move.w (0x4,A0),D4w
000145EE: move.w (0x96,A0),D3w
000145F2: move.w (0x98,A0),D2w
000145F6: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
000145FC: move.w D2w,(0x34,A0)
00014600: move.w D3w,(0xc8,A0)
00014604: move.l #0xb574,(0x3a,A0) ; data 0000b574
0001460C: rts
0001460E: jsr 0x00015ca4.l ; call 00015CA4 compute_diagonal_step_cost
00014614: cmpi.w #0x1e0,D0w
00014618: bcc.w 0x0001463a
0001461C: andi.b #-0x80,(0xb0,A0)
00014622: clr.b (0xb1,A0)
00014626: bset.b #0x2,(0xb0,A0)
0001462C: move.l #0xb45c,(0x3a,A0) ; data 0000b45c
00014634: ori #0x1,CCR
00014638: rts
0001463A: andi #0x1e,CCR
0001463E: rts
00014640: clr.l D1
00014642: move.w D0w,D1w
00014644: lsr.w #0x1,D1w
00014646: move.w (0x00ffddea).l,D2w
0001464C: lsr.w #0x1,D2w
0001464E: divu.w D2w,D1
00014650: move.w D1w,D0w
00014652: swap D1
00014654: lsl.w #0x5,D1w
00014656: lsl.w #0x5,D0w
00014658: lsl.w #0x4,D0w
0001465A: lsl.w #0x4,D1w
0001465C: addi.w #0x100,D0w
00014660: addi.w #0x100,D1w
00014664: rts
00014682: bset.b #0x1,(0xb0,A0)
00014688: btst.b #0x3,(0xb0,A0)
0001468E: bne.w 0x0001470c
00014692: lea (0xff1700).l,A4 ; data 00ff1700
00014698: move.w (A0),D0w
0001469A: move.w (0x4,A0),D1w
0001469E: move.w (0x96,A0),D2w
000146A2: move.w (0x98,A0),D3w
000146A6: bsr.w 0x000148d2 ; call 000148D2 choose_dominant_axis_directions
000146AA: lsr.w #0x4,D2w
000146AC: lsr.w #0x4,D3w
000146AE: lsl.w #0x1,D3w
000146B0: andi.w #-0x40,D3w
000146B4: lsr.w #0x5,D2w
000146B6: add.w D2w,D3w
000146B8: tst.b (0x0,A4,D5w*0x1) ; data 00ff1700
000146BC: bne.w 0x00014756
000146C0: clr.w (0xba,A0)
000146C4: bsr.w 0x000148aa ; call 000148AA find_first_occupied_obstruction_tile
000146C8: cmp.w D5w,D1w
000146CA: beq.w 0x000146e8
000146CE: movea.l A0,A2
000146D0: adda.w #0xbc,A2
000146D4: move.w (0xba,A0),D2w
000146D8: adda.w D2w,A2
000146DA: add.w D5w,D0w
000146DC: move.w D0w,(A2)+
000146DE: move.w #-0x1,(A2)
000146E2: clr.w (0xba,A0)
000146E6: rts
000146E8: move.w D1w,D5w
000146EA: bsr.w 0x00014854 ; call 00014854 FUN_00014854
000146EE: bcs.w 0x0001470e
000146F2: movea.l A0,A2
000146F4: adda.w #0xbc,A2
000146F8: move.w (0xba,A0),D0w
000146FC: adda.w D0w,A2
000146FE: move.w D1w,(A2)+
00014700: move.w D4w,(A2)+
00014702: move.w #-0x1,(A2)
00014706: clr.w (0xba,A0)
0001470A: rts
0001470C: rts
0001470E: bset.b #0x3,(0xb0,A0)
00014714: bsr.w 0x00014af4 ; call 00014AF4 FUN_00014af4
00014718: bra.w 0x000149c0 ; call 000149C0 FUN_000149c0
00014756: bclr.b #0x1,(0xb0,A0)
0001475C: bclr.b #0x3,(0xb0,A0)
00014762: rts
00014764: lea (0x148ca).l,A2 ; data 000148ca
0001476A: move.w (0x0,A2,D6w*0x1),D0w ; data 000148ca
0001476E: move.w (0x0,A2,D7w*0x1),D2w ; data 000148ca
00014772: move.w D5w,D1w
00014774: add.w D2w,D1w
00014776: bmi.w 0x000147ba
0001477A: move.w (0x00ffddea).l,D3w
00014780: move.w D1w,D4w
00014782: andi.w #0x3f,D4w
00014786: cmp.w D4w,D3w
00014788: bcs.w 0x000147ba
0001478C: move.w (0x00ffddd4).l,D3w
00014792: lsr.w #0x5,D3w
00014794: move.w D1w,D4w
00014796: lsr.w #0x6,D4w
00014798: cmp.w D4w,D3w
0001479A: bcs.w 0x000147ba
0001479E: tst.b (0x0,A4,D1w*0x1)
000147A2: bne.w 0x000147ba
000147A6: addq.w #0x2,D6w
000147A8: addq.w #0x2,D7w
000147AA: andi.w #0x6,D6w
000147AE: andi.w #0x6,D7w
000147B2: move.w (0x0,A2,D6w*0x1),D0w ; data 000148ca
000147B6: move.w (0x0,A2,D7w*0x1),D2w ; data 000148ca
000147BA: move.w D5w,D1w
000147BC: add.w D0w,D1w
000147BE: bmi.w 0x000147fc
000147C2: move.w (0x00ffddea).l,D3w
000147C8: move.w D1w,D4w
000147CA: andi.w #0x3f,D4w
000147CE: cmp.w D4w,D3w
000147D0: bcs.w 0x000147fc
000147D4: move.w (0x00ffddd4).l,D3w
000147DA: lsr.w #0x5,D3w
000147DC: move.w D1w,D4w
000147DE: lsr.w #0x6,D4w
000147E0: cmp.w D4w,D3w
000147E2: bcs.w 0x000147fc
000147E6: tst.b (0x0,A4,D1w*0x1)
000147EA: bne.w 0x000147fc
000147EE: move.w D1w,D4w
000147F0: add.w D2w,D4w
000147F2: tst.b (0x0,A4,D4w*0x1)
000147F6: beq.w 0x000147fe
000147FA: bra.b 0x000147bc
000147FC: sub.w D0w,D1w
000147FE: rts
00014800: lea (0x148ca).l,A2 ; data 000148ca
00014806: move.w (0x0,A2,D6w*0x1),D0w ; data 000148ca
0001480A: move.w (0x0,A2,D7w*0x1),D2w ; data 000148ca
0001480E: move.w D5w,D1w
00014810: add.w D0w,D1w
00014812: bmi.w 0x00014850
00014816: move.w (0x00ffddea).l,D3w
0001481C: move.w D1w,D4w
0001481E: andi.w #0x3f,D4w
00014822: cmp.w D4w,D3w
00014824: bcs.w 0x00014850
00014828: move.w (0x00ffddd4).l,D3w
0001482E: lsr.w #0x5,D3w
00014830: move.w D1w,D4w
00014832: lsr.w #0x6,D4w
00014834: cmp.w D4w,D3w
00014836: bcs.w 0x00014850
0001483A: tst.b (0x0,A4,D1w*0x1)
0001483E: bne.w 0x00014850
00014842: move.w D1w,D4w
00014844: add.w D2w,D4w
00014846: tst.b (0x0,A4,D4w*0x1)
0001484A: beq.w 0x00014852
0001484E: bra.b 0x00014810
00014850: sub.w D0w,D1w
00014852: rts
00014854: lea (0x148ca).l,A2 ; data 000148ca
0001485A: move.w (0x0,A2,D6w*0x1),D0w ; data 000148ca
0001485E: move.w (0x0,A2,D7w*0x1),D2w ; data 000148ca
00014862: move.w D5w,D1w
00014864: add.w D0w,D1w
00014866: tst.b (0x0,A4,D1w*0x1)
0001486A: bne.w 0x00014882
0001486E: move.w D1w,D4w
00014870: add.w D2w,D4w
00014872: tst.b (0x0,A4,D4w*0x1)
00014876: beq.w 0x0001487c
0001487A: bra.b 0x00014864
0001487C: andi #0x1e,CCR
00014880: rts
00014882: move.w D6w,D0w
00014884: eori.w #0x4,D0w
00014888: move.w (0x0,A2,D0w*0x1),D0w ; data 000148ca
0001488C: move.w D5w,D1w
0001488E: add.w D0w,D1w
00014890: tst.b (0x0,A4,D1w*0x1)
00014894: bne.w 0x000148a4
00014898: move.w D1w,D4w
0001489A: add.w D2w,D4w
0001489C: tst.b (0x0,A4,D4w*0x1)
000148A0: beq.b 0x0001487c
000148A2: bra.b 0x0001488e
000148A4: ori #0x1,CCR
000148A8: rts
000148AA: lea (0x148ca).l,A2 ; data 000148ca
000148B0: move.w (0x0,A2,D7w*0x1),D0w ; data 000148ca
000148B4: move.w D5w,D1w
000148B6: add.w D0w,D1w
000148B8: tst.b (0x0,A4,D1w*0x1)
000148BC: bne.w 0x000148c2
000148C0: bra.b 0x000148b6
000148C2: neg.w D0w
000148C4: add.w D0w,D1w
000148C6: neg.w D0w
000148C8: rts
000148D2: movem.w {  D1w D0w},-(SP)
000148D6: move.w #0x6,D7w
000148DA: clr.w D6w
000148DC: sub.w D2w,D0w
000148DE: bpl.w 0x000148e8
000148E2: neg.w D0w
000148E4: move.w #0x2,D7w
000148E8: move.w D0w,D5w
000148EA: sub.w D3w,D1w
000148EC: bpl.w 0x000148f6
000148F0: neg.w D1w
000148F2: move.w #0x4,D6w
000148F6: cmp.w D1w,D5w
000148F8: bcc.w 0x000148fe
000148FC: exg D6,D7
000148FE: movem.w (SP)+,{ D0 D1}
00014902: lsr.w #0x4,D0w
00014904: lsr.w #0x4,D1w
00014906: lsl.w #0x1,D1w
00014908: andi.w #-0x40,D1w
0001490C: lsr.w #0x5,D0w
0001490E: add.w D1w,D0w
00014910: move.w D0w,D5w
00014912: rts
00014914: lea (0xff1700).l,A4 ; data 00ff1700
0001491A: move.w (0x96,A0),D0w
0001491E: move.w (0x98,A0),D1w
00014922: bsr.b 0x00014902 ; call 00014902 compute_obstruction_grid_index
00014924: move.w D5w,D4w
00014926: move.w (A0),D0w
00014928: move.w (0x4,A0),D1w
0001492C: bsr.b 0x00014902 ; call 00014902 compute_obstruction_grid_index
0001492E: bsr.w 0x0001494c ; call 0001494C FUN_0001494c
00014932: bcs.w 0x00014946
00014936: exg D4,D5
00014938: bsr.w 0x0001494c ; call 0001494C FUN_0001494c
0001493C: bcs.w 0x00014946
00014940: andi #0x1e,CCR
00014944: rts
00014946: ori #0x1,CCR
0001494A: rts
0001494C: move.w D4w,D0w
0001494E: move.w D5w,D1w
00014950: move.w D4w,D2w
00014952: move.w D5w,D3w
00014954: andi.w #0x3f,D0w
00014958: andi.w #0x3f,D1w
0001495C: cmp.w D0w,D1w
0001495E: beq.w 0x0001497c
00014962: bcc.w 0x0001496e
00014966: subq.w #0x1,D0w
00014968: subq.w #0x1,D2w
0001496A: bra.w 0x00014972
0001496E: addq.w #0x1,D0w
00014970: addq.w #0x1,D2w
00014972: tst.b (0x0,A4,D2w*0x1)
00014976: bne.w 0x000149ba
0001497A: bra.b 0x0001495c
0001497C: move.w D5w,D0w
0001497E: move.w D4w,D1w
00014980: move.w D5w,D2w
00014982: move.w D4w,D3w
00014984: andi.w #-0x40,D0w
00014988: andi.w #-0x40,D1w
0001498C: cmp.w D0w,D1w
0001498E: beq.w 0x000149b4
00014992: bcc.w 0x000149a2
00014996: subi.w #0x40,D0w
0001499A: subi.w #0x40,D2w
0001499E: bra.w 0x000149aa
000149A2: addi.w #0x40,D0w
000149A6: addi.w #0x40,D2w
000149AA: tst.b (0x0,A4,D2w*0x1)
000149AE: bne.w 0x000149ba
000149B2: bra.b 0x0001498c
000149B4: ori #0x1,CCR
000149B8: rts
000149BA: andi #0x1e,CCR
000149BE: rts
000149C0: btst.b #0x3,(0xb0,A0)
000149C6: beq.w 0x000149d2
000149CA: bsr.w 0x00014914 ; call 00014914 FUN_00014914
000149CE: bcs.w 0x00014756
000149D2: move.w (A0),D0w
000149D4: move.w (0x4,A0),D1w
000149D8: lsr.w #0x4,D0w
000149DA: lsr.w #0x4,D1w
000149DC: lsl.w #0x1,D1w
000149DE: andi.w #-0x40,D1w
000149E2: lsr.w #0x5,D0w
000149E4: add.w D1w,D0w
000149E6: move.w D0w,D5w
000149E8: movea.l A0,A2
000149EA: adda.w #0xbc,A2
000149EE: move.w (0xba,A0),D0w
000149F2: adda.w D0w,A2
000149F4: move.w (A2)+,D0w
000149F6: cmp.w D0w,D5w
000149F8: beq.w 0x00014a00
000149FC: bra.w 0x00014a0e
00014A00: cmpi.w #-0x1,(A2)
00014A04: beq.w 0x00014a36
00014A08: move.w (A2),D0w
00014A0A: addq.w #0x2,(0xba,A0)
00014A0E: bsr.w 0x00014b4e ; call 00014B4E decode_obstruction_tile_projection
00014A12: ror.w #0x3,D2w
00014A14: move.w D2w,-(SP) ; data Stack[-0x2]
00014A16: move.w (A0),D0w
00014A18: move.w (0x4,A0),D1w
00014A1C: move.w (0x00ffdd2a).l,D2w
00014A22: move.w (0x00ffdd2c).l,D3w
00014A28: jsr 0x00015ca4.l ; call 00015CA4 compute_diagonal_step_cost
00014A2E: move.w (SP)+,D2w
00014A30: move.w D0w,D3w
00014A32: bra.w 0x000145fc
00014A36: btst.b #0x3,(0xb0,A0)
00014A3C: bne.w 0x00014a54
00014A40: bclr.b #0x1,(0xb0,A0)
00014A46: bclr.b #0x3,(0xb0,A0)
00014A4C: bsr.w 0x00014c2e ; call 00014C2E derive_entity_axis_relation_flags
00014A50: bra.w 0x000145e8
00014A54: lea (0xff1700).l,A4 ; data 00ff1700
00014A5A: move.w (A0),D0w
00014A5C: move.w (0x4,A0),D1w
00014A60: bsr.w 0x00014902 ; call 00014902 compute_obstruction_grid_index
00014A64: move.w (0x34,A0),D6w
00014A68: rol.w #0x3,D6w
00014A6A: andi.w #0x6,D6w
00014A6E: lea (0x148ca).l,A2 ; data 000148ca
00014A74: move.w (0x0,A2,D6w*0x1),D0w ; data 000148ca
00014A78: move.w D5w,D1w
00014A7A: add.w D0w,D1w
00014A7C: tst.b (0x0,A4,D1w*0x1) ; data 00ff1700
00014A80: bne.w 0x00014ab4
00014A84: move.w D6w,D0w
00014A86: addq.w #0x4,D0w
00014A88: andi.w #0x6,D0w
00014A8C: move.w (0x0,A2,D0w*0x1),D0w ; data 000148ca
00014A90: move.w D5w,D1w
00014A92: add.w D0w,D1w
00014A94: move.w D6w,D0w
00014A96: addq.w #0x2,D0w
00014A98: andi.w #0x6,D0w
00014A9C: move.w (0x0,A2,D0w*0x1),D0w ; data 000148ca
00014AA0: add.w D0w,D1w
00014AA2: tst.b (0x0,A4,D1w*0x1) ; data 00ff1700
00014AA6: bne.w 0x00014ab4
00014AAA: move.w D6w,D7w
00014AAC: bsr.w 0x000148aa ; call 000148AA find_first_occupied_obstruction_tile
00014AB0: bra.w 0x00014ae2
00014AB4: move.w D6w,D7w
00014AB6: addq.w #0x2,D7w
00014AB8: andi.w #0x6,D7w
00014ABC: move.w #0x4,D0w
00014AC0: move.w D0w,-(SP) ; data Stack[-0x2]
00014AC2: bsr.w 0x00014764 ; call 00014764 FUN_00014764
00014AC6: move.w (SP)+,D0w
00014AC8: cmp.w D5w,D1w
00014ACA: bne.w 0x00014ae2
00014ACE: subq.w #0x2,D6w
00014AD0: subq.w #0x2,D7w
00014AD2: andi.w #0x6,D6w
00014AD6: andi.w #0x6,D7w
00014ADA: dbf D0w,0x00014ac0
00014ADE: bra.w 0x00014756 ; call 00014756 clear_entity_route_flags
00014AE2: movea.l A0,A2
00014AE4: adda.w #0xbc,A2
00014AE8: clr.w (0xba,A0)
00014AEC: move.w D1w,(A2)+
00014AEE: move.w #-0x1,(A2)
00014AF2: rts
00014AF4: lea (0xff1700).l,A4 ; data 00ff1700
00014AFA: move.w (A0),D0w
00014AFC: move.w (0x4,A0),D1w
00014B00: bsr.w 0x00014902 ; call 00014902 compute_obstruction_grid_index
00014B04: move.w (0x34,A0),D6w
00014B08: rol.w #0x3,D6w
00014B0A: andi.w #0x6,D6w
00014B0E: move.w D6w,D7w
00014B10: addq.w #0x2,D7w
00014B12: andi.w #0x6,D7w
00014B16: move.w #0x4,D0w
00014B1A: move.w D0w,-(SP) ; data Stack[-0x2]
00014B1C: bsr.w 0x00014800 ; call 00014800 probe_obstruction_strip
00014B20: move.w (SP)+,D0w
00014B22: cmp.w D5w,D1w
00014B24: bne.w 0x00014b3c
00014B28: subq.w #0x2,D6w
00014B2A: subq.w #0x2,D7w
00014B2C: andi.w #0x6,D6w
00014B30: andi.w #0x6,D7w
00014B34: dbf D0w,0x00014b1a
00014B38: bra.w 0x00014756 ; call 00014756 clear_entity_route_flags
00014B3C: movea.l A0,A2
00014B3E: adda.w #0xbc,A2
00014B42: clr.w (0xba,A0)
00014B46: move.w D1w,(A2)+
00014B48: move.w #-0x1,(A2)
00014B4C: rts
00014B4E: move.w D0w,D1w
00014B50: lsr.w #0x6,D1w
00014B52: ror.w #0x7,D1w
00014B54: addi.w #0x100,D1w
00014B58: move.w D1w,(0x00ffdd2c).l
00014B5E: move.w D0w,D1w
00014B60: andi.w #0x3f,D1w
00014B64: ror.w #0x7,D1w
00014B66: addi.w #0x100,D1w
00014B6A: move.w D1w,(0x00ffdd2a).l
00014B70: move.w D0w,D1w
00014B72: move.w D5w,D4w
00014B74: andi.w #-0x40,D1w
00014B78: andi.w #-0x40,D4w
00014B7C: cmp.w D1w,D4w
00014B7E: bne.w 0x00014b9a
00014B82: move.w #0x2,D2w
00014B86: andi.w #0x3f,D0w
00014B8A: andi.w #0x3f,D5w
00014B8E: cmp.w D0w,D5w
00014B90: bcs.w 0x00014b98
00014B94: move.w #0x6,D2w
00014B98: rts
00014B9A: move.w #0x4,D2w
00014B9E: cmp.w D1w,D4w
00014BA0: bcs.b 0x00014b98
00014BA2: clr.w D2w
00014BA4: rts
00014BA6: move.w (0x96,A0),D0w
00014BAA: move.w (0x98,A0),D1w
00014BAE: sub.w (A0),D0w
00014BB0: sub.w (0x4,A0),D1w
00014BB4: btst.b #0x0,(0xb1,A0)
00014BBA: bne.w 0x00014bc8
00014BBE: tst.w D0w
00014BC0: bpl.w 0x00014bce
00014BC4: bra.w 0x00014bd4
00014BC8: tst.w D0w
00014BCA: bpl.w 0x00014bd4
00014BCE: bset.b #0x2,(0xb1,A0)
00014BD4: btst.b #0x1,(0xb1,A0)
00014BDA: bne.w 0x00014be8
00014BDE: tst.w D1w
00014BE0: bpl.w 0x00014bee
00014BE4: bra.w 0x00014bf4
00014BE8: tst.w D1w
00014BEA: bpl.w 0x00014bf4
00014BEE: bset.b #0x3,(0xb1,A0)
00014BF4: move.w (0xb8,A0),D0w
00014BF8: bmi.w 0x00014c24
00014BFC: btst.b #0x2,(0xb1,A0)
00014C02: bne.w 0x00014c1a
00014C06: btst.b #0x3,(0xb1,A0)
00014C0C: beq.w 0x00014c18
00014C10: btst.l #0x1,D0
00014C14: bne.w 0x00014c24
00014C18: rts
00014C1A: btst.l #0x1,D0
00014C1E: beq.w 0x00014c24
00014C22: bra.b 0x00014c18
00014C24: bset.b #0x1,(0xb0,A0)
00014C2A: bra.w 0x00014682 ; call 00014682 FUN_00014682
00014C2E: move.w (0x96,A0),D0w
00014C32: move.w (0x98,A0),D1w
00014C36: clr.b (0xb1,A0)
00014C3A: cmp.w (A0),D0w
00014C3C: bcs.w 0x00014c46
00014C40: bset.b #0x0,(0xb1,A0)
00014C46: cmp.w (0x4,A0),D1w
00014C4A: bcs.w 0x00014c54
00014C4E: bset.b #0x1,(0xb1,A0)
00014C54: rts
00014C56: move.w (0x2e,A0),D0w
00014C5A: rol.w #0x3,D0w
00014C5C: andi.w #0x7,D0w
00014C60: lsl.w #0x2,D0w
00014C62: lea (0xb2f6).l,A2 ; data 0000b2f6
00014C68: move.w (0x0,A2,D0w*0x1),D1w ; data 0000b2f6
00014C6C: move.w (0x2,A2,D0w*0x1),D0w ; data 0000b2f8
00014C70: move.w (A0),D5w
00014C72: move.w (0x4,A0),D6w
00014C76: add.w D0w,D6w
00014C78: add.w D1w,D5w
00014C7A: rts
00014C7C: cmpi.w #0xc,(0xe,A0)
00014C82: beq.w 0x00014d04
00014C86: cmpi.w #0xd,(0xe,A0)
00014C8C: beq.w 0x00014cb8
00014C90: cmpi.w #0xe,(0xe,A0)
00014C96: beq.w 0x00014cb8
00014C9A: movea.l (0xa2,A0),A1
00014C9E: jsr 0x00015c58.l ; call 00015C58 FUN_00015c58
00014CA4: bcc.w 0x00014cb2
00014CA8: move.w D2w,(0x34,A0)
00014CAC: ori #0x1,CCR
00014CB0: rts
00014CB2: andi #0x1e,CCR
00014CB6: rts
00014CB8: bsr.w 0x00014d58 ; call 00014D58 FUN_00014d58
00014CBC: move.w (0x2e,A0),(0x00ffdd48).l
00014CC4: bsr.b 0x00014c56 ; call 00014C56 project_heading_position
00014CC6: move.w D2w,(0x2e,A0)
00014CCA: jsr 0x00015c84.l ; call 00015C84 FUN_00015c84
00014CD0: move SR,-(SP) ; data Stack[-0x2]
00014CD2: move.w (0x00ffdd48).l,(0x2e,A0)
00014CDA: move (SP)+,SR
00014CDC: bcc.b 0x00014cb2
00014CDE: cmpi.w #0xe,(0xe,A0)
00014CE4: beq.w 0x00014cfa
00014CE8: cmpi.w #0x2,(0x1e,A0)
00014CEE: bne.w 0x00014cfa
00014CF2: move.w #0x1000,(0x1a,A0)
00014CF8: bra.b 0x00014cb2
00014CFA: move.w D2w,(0x2e,A0)
00014CFE: move.w D2w,(0x34,A0)
00014D02: bra.b 0x00014ca8
00014D04: bsr.w 0x00014d58 ; call 00014D58 FUN_00014d58
00014D08: cmpi.w #0x300,D3w
00014D0C: bcc.w 0x00014d44
00014D10: tst.w (0xdc,A0)
00014D14: bmi.w 0x00014d1a
00014D18: bra.b 0x00014ca8
00014D1A: movem.l {  D2},-(SP)
00014D1E: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
00014D24: movem.l (SP)+,{  D2}
00014D28: bcs.b 0x00014cb2
00014D2A: move.w D0w,(0x32,A0)
00014D2E: move.w D1w,(0x2a,A0)
00014D32: bset.b #0x6,(0x11,A0)
00014D38: move.w D2w,(0x2e,A0)
00014D3C: move.w D2w,(0x34,A0)
00014D40: bra.w 0x00014ca8
00014D44: cmpi.w #0x600,D3w
00014D48: bcc.w 0x00014cb2
00014D4C: move.w D2w,(0x2e,A0)
00014D50: move.w D2w,(0x34,A0)
00014D54: bra.w 0x00014ca8
00014D58: move.w (A0),D5w
00014D5A: move.w (0x4,A0),D4w
00014D5E: move.w (A1),D3w
00014D60: move.w (0x4,A1),D2w
00014D64: jmp 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00014D6A: lea (0xd4540).l,A2 ; data 000d4540
00014D70: tst.b (0x56,A0)
00014D74: bpl.w 0x00014d7e
00014D78: lea (0xd4ea0).l,A2 ; data 000d4ea0
00014D7E: clr.l D6
00014D80: move.b (0x57,A0),D6b
00014D84: subq.w #0x1,D6w
00014D86: mulu.w #0x30,D6
00014D8A: adda.w D6w,A2
00014D8C: rts
00014D8E: move.w #-0x1,(0x00ffdfd8).l
00014D96: move.l A1,D3
00014D98: move.l D3,(0x00ffdd3c).l
00014D9E: lea (0xff1380).l,A1 ; data 00ff1380
00014DA4: move.w #-0x1,D0w
00014DA8: bsr.w 0x00014ff8 ; call 00014FF8 FUN_00014ff8
00014DAC: move.l A0,-(SP) ; data Stack[-0x4]
00014DAE: lea (0xff1380).l,A0 ; data 00ff1380
00014DB4: move.w #0x1a,D7w
00014DB8: move.w D7w,-(SP) ; data Stack[-0x6]
00014DBA: bsr.w 0x00014f26 ; call 00014F26 FUN_00014f26
00014DBE: move.l D5,(A0)
00014DC0: move.l D6,(0x4,A0)
00014DC4: swap D5
00014DC6: swap D6
00014DC8: move.w (SP)+,D7w
00014DCA: tst.w D0w
00014DCC: beq.w 0x00014dda
00014DD0: tst.w (0x00ffdfe2).l
00014DD6: beq.w 0x00014dfc
00014DDA: movem.w {  D7w},-(SP)
00014DDE: bsr.w 0x00015aba ; call 00015ABA FUN_00015aba
00014DE2: movem.w (SP)+,{  D7}
00014DE6: bcs.w 0x00014dee
00014DEA: dbf D7w,0x00014db8
00014DEE: clr.w (0x00ffdfd8).l
00014DF4: movea.l (SP)+,A0
00014DF6: andi #0x1e,CCR
00014DFA: rts
00014DFC: clr.w (0x00ffdfd8).l
00014E02: movea.l (SP)+,A0
00014E04: ori #0x1,CCR
00014E08: rts
00014E0A: btst.b #0x3,(0x11,A0)
00014E10: bne.w 0x00014f1c
00014E14: move.w #0x3,D7w
00014E18: move.w D7w,(0x00ffdd50).l
00014E1E: bsr.w 0x00014f50 ; call 00014F50 FUN_00014f50
00014E22: bcs.w 0x00014e32
00014E26: move.w (0x00ffdd50).l,D7w
00014E2C: dbf D7w,0x00014e18
00014E30: rts
00014E32: tst.w (0x00ffdf68).l
00014E38: beq.w 0x00014e42
00014E3C: bclr.b #0x0,(0x10,A0)
00014E42: bset.b #0x0,(0x11,A0)
00014E48: bset.b #0x3,(0x11,A0)
00014E4E: clr.w (0x16,A0)
00014E52: clr.w (0x4c,A0)
00014E56: clr.w (0x4e,A0)
00014E5A: tst.w D1w
00014E5C: bne.w 0x00014e6e
00014E60: move.w #0x3c,D0w
00014E64: jsr 0x000530de.l ; call 000530DE FUN_000530de
00014E6A: bra.w 0x00014eda
00014E6E: move.w (A1),(A0)
00014E70: move.w (0x4,A1),D1w
00014E74: addi.w #0x10,D1w
00014E78: move.w D1w,(0x4,A0)
00014E7C: move.b (0x4c,A1),D1b
00014E80: move.w D1w,-(SP)
00014E82: jsr 0x00055948.l ; call 00055948 FUN_00055948
00014E88: move.w (SP)+,D1w
00014E8A: sub.b (0x4c,A1),D1b
00014E8E: nop
00014E90: nop
00014E92: nop
00014E94: beq.w 0x00014ec4
00014E98: move.w #0x2000,(0x2c,A0) ; data 00002000
00014E9E: move.w #0x4,(0x16,A0)
00014EA4: cmpi.b #0x3,D1b
00014EA8: blt.w 0x00014ed2
00014EAC: move.w #0xc,(0x16,A0)
00014EB2: cmpi.b #0x6,D1b
00014EB6: blt.w 0x00014ed2
00014EBA: move.w #0x10,(0x16,A0)
00014EC0: bra.w 0x00014ed2
00014EC4: move.w #0x3c,D0w
00014EC8: jsr 0x000530de.l ; call 000530DE FUN_000530de
00014ECE: bra.w 0x00014eda
00014ED2: movea.l A1,A0
00014ED4: jsr 0x0000aff2.l ; call 0000AFF2 prepare_entity_auxiliary_behavior
00014EDA: rts
00014F1C: tst.w (0x16,A0)
00014F20: bmi.w 0x00014f86
00014F24: rts
00014F26: move.w (0x4c,A0),D5w
00014F2A: ext.l D5
00014F2C: asl.l #0x8,D5
00014F2E: add.l (A0),D5
00014F30: move.w (0x4e,A0),D6w
00014F34: ext.l D6
00014F36: asl.l #0x8,D6
00014F38: add.l (0x4,A0),D6
00014F3C: movem.l {  D6 D5},-(SP)
00014F40: swap D5
00014F42: swap D6
00014F44: jsr 0x0000e7c8.l ; call 0000E7C8 FUN_0000e7c8
00014F4A: movem.l (SP)+,{  D5 D6}
00014F4E: rts
00014F50: bsr.b 0x00014f26 ; call 00014F26 FUN_00014f26
00014F52: clr.w D1w
00014F54: tst.w D0w
00014F56: beq.w 0x00014f64
00014F5A: tst.w (0x00ffdfe2).l
00014F60: beq.w 0x00014f80
00014F64: subq.b #0x1,(0x89,A0)
00014F68: bmi.w 0x00014f80
00014F6C: move.l D5,(A0)
00014F6E: move.l D6,(0x4,A0)
00014F72: swap D5
00014F74: swap D6
00014F76: move.w #0x1,D1w
00014F7A: bsr.w 0x00015aba ; call 00015ABA FUN_00015aba
00014F7E: rts
00014F80: ori #0x1,CCR
00014F84: rts
00014F86: subq.b #0x1,(0x41,A0)
00014F8A: bmi.w 0x00014ff0
00014F8E: move.w #0x14,(0x16,A0)
00014F94: move.b #0x20,(0x89,A0)
00014F9A: movea.l (0x60,A0),A1
00014F9E: movem.l {  A1 A0},-(SP)
00014FA2: exg A0,A1
00014FA4: jsr 0x00055640.l ; call 00055640 FUN_00055640
00014FAA: movem.l (SP)+,{  A0 A1}
00014FAE: move.w (0x58,A0),(A0)
00014FB2: move.w (0x5a,A0),(0x4,A0)
00014FB8: tst.b (0x41,A0)
00014FBC: beq.w 0x00014fca
00014FC0: subi.w #0x400,(0x2e,A0)
00014FC6: bra.w 0x00014fd0
00014FCA: addi.w #0x800,(0x2e,A0)
00014FD0: move.w #0x4000,(0x2c,A0) ; data 00004000
00014FD6: clr.w (0x1e,A0)
00014FDA: clr.w (0x18,A0)
00014FDE: bclr.b #0x3,(0x11,A0)
00014FE4: movea.l A0,A1
00014FE6: move.w (0x2e,A0),D1w
00014FEA: bsr.w 0x000150e8 ; call 000150E8 FUN_000150e8
00014FEE: rts
00014FF0: bclr.b #0x0,(0x10,A0)
00014FF6: rts
00014FF8: movem.l {  A1 A0 D3 D0},-(SP)
00014FFC: movem.l {  A1 A0},-(SP)
00015000: bsr.w 0x00015118 ; call 00015118 FUN_00015118
00015004: movem.l (SP)+,{  A0 A1}
00015008: tst.w (0x00ffdfd8).l
0001500E: bne.w 0x00015018
00015012: jsr 0x00055640.l ; call 00055640 FUN_00055640
00015018: movem.l (SP)+,{ D0 D3 A0 A1}
0001501C: move.w #0x4000,(0x2c,A1) ; data 00004000
00015022: move.w (A0),(A1)
00015024: move.w (0x4,A0),(0x4,A1)
0001502A: move.w (0x2e,A0),(0x2e,A1)
00015030: move.b (0x9d,A0),(0x9d,A1)
00015036: move.b (0x57,A0),(0x57,A1)
0001503C: move.b (0x56,A0),(0x56,A1)
00015042: move.w D0w,(0x3e,A1)
00015046: move.w #0x14,(0x16,A1)
0001504C: move.l A0,(0x60,A1)
00015050: btst.b #0x3,(0x10,A0)
00015056: beq.w 0x00015060
0001505A: bset.b #0x3,(0x10,A1)
00015060: tst.l D3
00015062: beq.w 0x0001509a
00015066: movea.l D3,A2
00015068: btst.b #0x0,(0x10,A2)
0001506E: beq.w 0x0001509a
00015072: tst.w (0x00ffdfd8).l
00015078: bne.w 0x00015080
0001507C: move.l A0,(0xac,A2)
00015080: move.w (A0),D5w
00015082: move.w (0x4,A0),D4w
00015086: move.w (A2),D3w
00015088: move.w (0x4,A2),D2w
0001508C: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00015092: move.w D2w,(0x2e,A1)
00015096: bra.w 0x0001509a
0001509A: move.w (0x2e,A1),D1w
0001509E: move.w D1w,D0w
000150A0: rol.w #0x3,D0w
000150A2: bpl.w 0x000150a8
000150A6: addq.w #0x1,D0w
000150A8: andi.w #0x7,D0w
000150AC: lsl.w #0x2,D0w
000150AE: lea (0x15a6c).l,A3 ; data 00015a6c
000150B4: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a6c
000150B8: add.w (A1),D6w
000150BA: move.w D6w,(0x00ffdd60).l
000150C0: addq.w #0x2,D0w
000150C2: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a6c
000150C6: add.w (0x4,A1),D6w
000150CA: move.w D6w,(0x00ffdd62).l
000150D0: subq.w #0x2,D0w
000150D2: lea (0x15a4c).l,A3 ; data 00015a4c
000150D8: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a4c
000150DC: add.w D6w,(A1)
000150DE: addq.w #0x2,D0w
000150E0: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a4c
000150E4: add.w D6w,(0x4,A1)
000150E8: lsr.w #0x8,D1w
000150EA: andi.w #-0x2,D1w
000150EE: lea (0x15e56).l,A3 ; data 00015e56
000150F4: move.w (0x0,A3,D1w*0x1),D6w ; data 00015e56
000150F8: lea (0x15e16).l,A3 ; data 00015e16
000150FE: move.w (0x0,A3,D1w*0x1),D7w ; data 00015e16
00015102: muls.w #0xc,D6
00015106: asr.l #0x4,D6
00015108: move.w D6w,(0x4c,A1)
0001510C: muls.w #0xc,D7
00015110: asr.l #0x4,D7
00015112: move.w D7w,(0x4e,A1)
00015116: rts
00015118: move.l A1,-(SP) ; data Stack[-0x4]
0001511A: move.w #0x7f,D7w
0001511E: clr.w (A1)+
00015120: dbf D7w,0x0001511e
00015124: movea.l (SP)+,A1
00015126: move.w #-0xc,(0x8,A1)
0001512C: move.w #0x1,(0xe,A1)
00015132: move.w #0x1000,(0x1c,A1)
00015138: move.w #0x4000,(0x2c,A1) ; data 00004000
0001513E: bset.b #0x0,(0x10,A1)
00015144: move.l #0x14e0a,(0x3a,A1) ; data 00014e0a
0001514C: rts
0001514E: clr.w (0x00ffdd3a).l
00015154: cmpi.b #0x8,(0x57,A0)
0001515A: beq.w 0x00015172
0001515E: cmpi.b #0x9,(0x57,A0)
00015164: beq.w 0x00015172
00015168: cmpi.b #0xa,(0x57,A0)
0001516E: bne.w 0x0001517a
00015172: move.w #-0x1,(0x00ffdd3a).l
0001517A: cmpi.b #0x3,(0x00ffe186).l
00015182: beq.w 0x000152f4
00015186: move.w (0x16,A0),D0w
0001518A: andi.w #0x7ff,D0w
0001518E: cmpi.w #0x3c,D0w
00015192: ble.w 0x000151ac
00015196: btst.b #0x1,(0x10,A0)
0001519C: beq.w 0x000152f4
000151A0: move.w #-0x1,(0x00ffdfe6).l
000151A8: bra.w 0x000152f4
000151AC: jsr 0x000038ac.l ; call 000038AC FUN_000038ac
000151B2: beq.w 0x000152f4
000151B6: bsr.w 0x00015a8c ; call 00015A8C find_first_available_entity_slot
000151BA: bcs.w 0x000152f4
000151BE: move.l (0xa2,A0),D3
000151C2: beq.w 0x000151c6
000151C6: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
000151CC: bsr.w 0x00014ff8 ; call 00014FF8 FUN_00014ff8
000151D0: move.b #0x7f,(0x89,A1)
000151D6: clr.b (0x8b,A1)
000151DA: tst.w (0x00ffdd3a).l
000151E0: beq.w 0x000151e8
000151E4: bsr.w 0x000152fa ; call 000152FA FUN_000152fa
000151E8: movea.l A1,A2
000151EA: bsr.w 0x00015a8c ; call 00015A8C find_first_available_entity_slot
000151EE: bcs.w 0x000152f4
000151F2: move.w D0w,-(SP) ; data Stack[-0x2]
000151F4: lea (0x15318).l,A4 ; data 00015318
000151FA: clr.b (0x00ffdd6d).l
00015200: move.b (0x56,A0),D0b
00015204: addi.b #0x6e,D0b
00015208: btst.b #0x1,(0x0,A0,D0w*0x1)
0001520E: bne.w 0x0001521c
00015212: btst.b #0x2,(0x0,A0,D0w*0x1)
00015218: beq.w 0x0001522a
0001521C: move.b (0xffffffff).w,(0x00ffdd6d).l
00015224: lea (0x15336).l,A4 ; data 00015336
0001522A: move.b (0x57,A0),D0b
0001522E: subq.b #0x1,D0b
00015230: ext.w D0w
00015232: lsl.w #0x1,D0w
00015234: move.w (0x0,A4,D0w*0x1),D0w ; data 00015336
00015238: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001523E: move.w (SP)+,D0w
00015240: cmpi.w #0x2,(0x00ffdf40).l
00015248: bne.w 0x00015278
0001524C: move.b (0x56,A0),D7b
00015250: ext.w D7w
00015252: movea.l A0,A3
00015254: adda.w D7w,A3
00015256: adda.w #0x6e,A3
0001525A: btst.b #0x1,(A3)
0001525E: bne.w 0x00015278
00015262: btst.b #0x2,(A3)
00015266: bne.w 0x00015278
0001526A: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP) ; data 00015336
0001526E: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
00015274: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00015278: move.w #0x4f,D7w
0001527C: movem.l {  A2 A1},-(SP)
00015280: move.w (A2)+,(A1)+
00015282: dbf D7w,0x00015280
00015286: movem.l (SP)+,{  A1 A2}
0001528A: move.w (0x00ffdd60).l,(A1)
00015290: move.w (0x00ffdd62).l,(0x4,A1)
00015298: move.w D0w,(0x3e,A1)
0001529C: bset.b #0x0,(0x11,A1)
000152A2: move.w #0x0,(0xe,A1)
000152A8: move.l #0xac4e,(0x3a,A1) ; data 0000ac4e
000152B0: move.b (0x00ffdd6d).l,(0x64,A1)
000152B8: clr.b (0x41,A1)
000152BC: lea (0x62ec).l,A2 ; data 000062ec
000152C2: tst.w (0x00ffdd3a).l
000152C8: beq.w 0x000152d2
000152CC: lea (0x62fc).l,A2 ; data 000062fc
000152D2: move.w (0x2e,A1),D0w
000152D6: rol.w #0x3,D0w
000152D8: btst.l #0xf,D0
000152DC: beq.w 0x000152e2
000152E0: addq.w #0x1,D0w
000152E2: andi.w #0x7,D0w
000152E6: lsl.w #0x1,D0w
000152E8: move.w (0x0,A2,D0w*0x1),(0x16,A1) ; data 000062fc
000152EE: andi #0x1e,CCR
000152F2: rts
000152F4: ori #0x1,CCR
000152F8: rts
000152FA: move.b #0x2,(0x41,A1)
00015300: move.w (A1),(0x58,A1)
00015304: move.w (0x4,A1),(0x5a,A1)
0001530A: move.b #0x1,(0x8b,A1)
00015310: move.b #0x20,(0x89,A1)
00015316: rts
00015354: bset.b #0x7,(0x10,A0)
0001535A: move.w #0xc,(0x16,A0)
00015360: cmpi.b #0xe,(0x57,A0)
00015366: bne.w 0x00015370
0001536A: move.w #0x10,(0x16,A0)
00015370: move.b #0x0,(0x41,A0)
00015376: move.w (A0),(0x2,A0)
0001537A: move.w (0x4,A0),(0x6,A0)
00015380: clr.w (0x44,A0)
00015384: move.w #0x2c,D0w
00015388: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001538E: rts
00015390: btst.b #0x7,(0x10,A0)
00015396: beq.w 0x000153f8
0001539A: addq.w #0x1,(0x44,A0)
0001539E: tst.w (0x16,A0)
000153A2: bpl.w 0x000153f6
000153A6: subq.b #0x1,(0x41,A0)
000153AA: bmi.w 0x00015410
000153AE: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
000153B4: move.w D0w,D1w
000153B6: andi.w #0x7f,D1w
000153BA: btst.l #0x0,D1
000153BE: beq.w 0x000153c4
000153C2: neg.w D1w
000153C4: add.w (0x2,A0),D1w
000153C8: move.w D1w,(A0)
000153CA: lsr.w #0x4,D0w
000153CC: andi.w #0x7f,D0w
000153D0: btst.l #0x0,D0
000153D4: beq.w 0x000153da
000153D8: neg.w D0w
000153DA: add.w (0x6,A0),D0w
000153DE: move.w D0w,(0x4,A0)
000153E2: clr.w (0x1e,A0)
000153E6: andi.w #0x7fff,(0x16,A0)
000153EC: clr.w (0x18,A0)
000153F0: bset.b #0x0,(0x11,A0)
000153F6: rts
000153F8: subq.w #0x1,(0x46,A0)
000153FC: bmi.w 0x00015402
00015400: rts
00015402: subq.w #0x1,(0x40,A0)
00015406: bpl.w 0x00015426
0001540A: bsr.w 0x00015354 ; call 00015354 FUN_00015354
0001540E: rts
00015410: move.w (0x30,A0),D0w
00015414: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
0001541A: bclr.b #0x0,(0x10,A0)
00015420: bsr.w 0x00015554 ; call 00015554 FUN_00015554
00015424: rts
00015426: addi.w #0x20,(0x36,A0)
0001542C: move.w (0x34,A0),D0w
00015430: add.w (0x36,A0),D0w
00015434: move.w D0w,(0x34,A0)
00015438: asr.w #0x8,D0w
0001543A: move.w D0w,(0x8,A0)
0001543E: bmi.w 0x00015488
00015442: move.w (0x36,A0),D0w
00015446: cmpi.w #0x40,D0w
0001544A: blt.w 0x00015472
0001544E: lsr.w #0x1,D0w
00015450: neg.w D0w
00015452: move.w D0w,(0x36,A0)
00015456: move.w (0x4c,A0),D0w
0001545A: asr.w #0x1,D0w
0001545C: move.w D0w,(0x4c,A0)
00015460: move.w (0x4e,A0),D0w
00015464: asr.w #0x1,D0w
00015466: move.w D0w,(0x4e,A0)
0001546A: clr.w (0x34,A0)
0001546E: bra.w 0x00015488
00015472: move.w #0x4,(0x16,A0)
00015478: clr.w (0x34,A0)
0001547C: clr.w (0x36,A0)
00015480: clr.w (0x4c,A0)
00015484: clr.w (0x4e,A0)
00015488: move.w (0x4c,A0),D5w
0001548C: ext.l D5
0001548E: asl.l #0x8,D5
00015490: add.l (A0),D5
00015492: move.w (0x4e,A0),D6w
00015496: ext.l D6
00015498: asl.l #0x8,D6
0001549A: add.l (0x4,A0),D6
0001549E: movem.l {  D6 D5},-(SP)
000154A2: move.w (0x8,A0),D0w
000154A6: lsl.w #0x4,D0w
000154A8: add.w D0w,D6w
000154AA: swap D5
000154AC: swap D6
000154AE: jsr 0x0000e7c8.l ; call 0000E7C8 FUN_0000e7c8
000154B4: movem.l (SP)+,{  D5 D6}
000154B8: clr.w D1w
000154BA: tst.w D0w
000154BC: beq.w 0x000154f8
000154C0: tst.w (0x00ffdfe2).l
000154C6: beq.w 0x000154ee
000154CA: tst.w (0x34,A0)
000154CE: bne.w 0x000154f8
000154D2: tst.w (0x00ffdf40).l
000154D8: bne.w 0x000154f8
000154DC: move.w (0x30,A0),D0w
000154E0: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
000154E6: bclr.b #0x0,(0x10,A0)
000154EC: rts
000154EE: tst.w (0x00ffdf68).l
000154F4: beq.w 0x0001550e
000154F8: move.l D5,(A0)
000154FA: move.l D6,(0x4,A0)
000154FE: rts
0001550E: move.w (0x4e,A0),D1w
00015512: asr.w #0x1,D1w
00015514: move.w D1w,(0x4e,A0)
00015518: move.w (0x4c,A0),D1w
0001551C: asr.w #0x1,D1w
0001551E: move.w D1w,(0x4c,A0)
00015522: move.w (0x4,A0),D1w
00015526: move.w (A0),D3w
00015528: lsr.w #0x4,D3w
0001552A: lsr.w #0x4,D1w
0001552C: lsl.w #0x1,D1w
0001552E: andi.w #-0x40,D1w
00015532: lsr.w #0x5,D3w
00015534: add.w D3w,D1w
00015536: andi.w #0x3f,D1w
0001553A: andi.w #0x3f,D2w
0001553E: cmp.w D1w,D2w
00015540: bne.w 0x0001554c
00015544: neg.w (0x4e,A0)
00015548: bra.w 0x00015550
0001554C: neg.w (0x4c,A0)
00015550: rts
00015554: move.w (A0),D5w
00015556: move.w (0x4,A0),D6w
0001555A: lea (0xff0100).l,A1 ; data 00ff0100
00015560: move.w #0x9,D7w
00015564: cmpi.b #0xe,(0x57,A0)
0001556A: beq.w 0x000155ce
0001556E: btst.b #0x0,(0x10,A1)
00015574: beq.w 0x000155c0
00015578: btst.b #0x6,(0x10,A1)
0001557E: bne.w 0x000155c0
00015582: move.w (A1),D0w
00015584: subi.w #0x280,D0w
00015588: bcc.w 0x0001558e
0001558C: clr.w D0w
0001558E: cmp.w D0w,D5w
00015590: bcs.w 0x000155c0
00015594: addi.w #0x500,D0w
00015598: cmp.w D0w,D5w
0001559A: bcc.w 0x000155c0
0001559E: move.w (0x4,A1),D0w
000155A2: subi.w #0x280,D0w
000155A6: bcc.w 0x000155ac
000155AA: clr.w D0w
000155AC: cmp.w D0w,D6w
000155AE: bcs.w 0x000155c0
000155B2: addi.w #0x500,D0w
000155B6: cmp.w D0w,D6w
000155B8: bcc.w 0x000155c0
000155BC: bsr.w 0x0001562e ; call 0001562E FUN_0001562e
000155C0: adda.w #0x100,A1
000155C4: dbf D7w,0x0001556e
000155C8: andi #0x1e,CCR
000155CC: rts
000155CE: btst.b #0x0,(0x10,A1)
000155D4: beq.w 0x00015620
000155D8: btst.b #0x6,(0x10,A1)
000155DE: bne.w 0x00015620
000155E2: move.w (A1),D0w
000155E4: subi.w #0x140,D0w
000155E8: bcc.w 0x000155ee
000155EC: clr.w D0w
000155EE: cmp.w D0w,D5w
000155F0: bcs.w 0x00015620
000155F4: addi.w #0x280,D0w
000155F8: cmp.w D0w,D5w
000155FA: bcc.w 0x00015620
000155FE: move.w (0x4,A1),D0w
00015602: subi.w #0x140,D0w
00015606: bcc.w 0x0001560c
0001560A: clr.w D0w
0001560C: cmp.w D0w,D6w
0001560E: bcs.w 0x00015620
00015612: addi.w #0x280,D0w
00015616: cmp.w D0w,D6w
00015618: bcc.w 0x00015620
0001561C: bsr.w 0x0001562e ; call 0001562E FUN_0001562e
00015620: adda.w #0x100,A1
00015624: dbf D7w,0x000155ce
00015628: andi #0x1e,CCR
0001562C: rts
0001562E: movem.l {  A1 A0 D7 D6 D5},-(SP)
00015632: jsr 0x00055900.l ; call 00055900 FUN_00055900
00015638: movea.l A1,A0
0001563A: jsr 0x0000aff2.l ; call 0000AFF2 prepare_entity_auxiliary_behavior
00015640: movem.l (SP)+,{  D5 D6 D7 A0 A1}
00015644: rts
00015646: move.w (0x16,A0),D0w
0001564A: andi.w #0x7ff,D0w
0001564E: cmpi.w #0x3c,D0w
00015652: bgt.w 0x00015792
00015656: moveq #-0x1,D1
00015658: jsr 0x000036fc.l ; call 000036FC FUN_000036fc
0001565E: bsr.w 0x00015a8c ; call 00015A8C find_first_available_entity_slot
00015662: bcs.w 0x00015792
00015666: tst.w (0x00ffdf84).l
0001566C: bne.w 0x00015670
00015670: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00015676: move.w #0x4f,D7w
0001567A: movem.l {  A1 A0},-(SP)
0001567E: bsr.w 0x00015118 ; call 00015118 FUN_00015118
00015682: movem.l (SP)+,{  A0 A1}
00015686: move.l #0x15390,(0x3a,A1) ; data 00015390
0001568E: move.w #0x2000,(0x2c,A1) ; data 00002000
00015694: move.w (A0),(A1)
00015696: move.w (0x4,A0),(0x4,A1)
0001569C: move.w (0x2e,A0),(0x2e,A1)
000156A2: move.b (0x9d,A0),(0x9d,A1)
000156A8: move.b (0x57,A0),(0x57,A1)
000156AE: move.b (0x56,A0),(0x56,A1)
000156B4: move.w D0w,(0x3e,A1)
000156B8: move.w #0x13,(0x46,A1)
000156BE: move.w #-0x500,(0x36,A1) ; data 0000fb00
000156C4: move.w #-0xc00,(0x34,A1) ; data 0000f400
000156CA: move.w #0x2,(0xe,A1)
000156D0: move.w #0xb4,(0x40,A1)
000156D6: move.w #0x0,(0x16,A1)
000156DC: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
000156E2: bcc.w 0x000156ee
000156E6: clr.b (0x10,A1)
000156EA: bra.w 0x00015792
000156EE: move.w D0w,(0x30,A1)
000156F2: move.w D1w,(0x20,A1)
000156F6: move.w A2w,(0x26,A1)
000156FA: move.w (0x2e,A1),D1w
000156FE: move.w D1w,D0w
00015700: rol.w #0x3,D0w
00015702: bpl.w 0x00015708
00015706: addq.w #0x1,D0w
00015708: andi.w #0x7,D0w
0001570C: lsl.w #0x2,D0w
0001570E: lea (0x15a6c).l,A3 ; data 00015a6c
00015714: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a6c
00015718: add.w (A1),D6w
0001571A: move.w D6w,(0x00ffdd60).l
00015720: addq.w #0x2,D0w
00015722: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a6c
00015726: add.w (0x4,A1),D6w
0001572A: move.w D6w,(0x00ffdd62).l
00015730: subq.w #0x2,D0w
00015732: lea (0x15a4c).l,A3 ; data 00015a4c
00015738: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a4c
0001573C: add.w D6w,(A1)
0001573E: addq.w #0x2,D0w
00015740: move.w (0x0,A3,D0w*0x1),D6w ; data 00015a4c
00015744: add.w D6w,(0x4,A1)
00015748: move.l (0xa2,A0),D0
0001574C: bne.w 0x00015798
00015750: move.b (0x87,A0),D0b
00015754: lsr.b #0x1,D0b
00015756: move.b D0b,(0x9f,A1)
0001575A: lsr.w #0x8,D1w
0001575C: andi.w #-0x2,D1w
00015760: lea (0x15e56).l,A3 ; data 00015e56
00015766: move.w (0x0,A3,D1w*0x1),D6w ; data 00015e56
0001576A: lea (0x15e16).l,A3 ; data 00015e16
00015770: move.w (0x0,A3,D1w*0x1),D7w ; data 00015e16
00015774: muls.w #0x8,D6
00015778: asr.l #0x5,D6
0001577A: move.w D6w,(0x4c,A1)
0001577E: muls.w #0x8,D7
00015782: asr.l #0x5,D7
00015784: move.w D7w,(0x4e,A1)
00015788: bra.w 0x0001578c
0001578C: andi #0x1e,CCR
00015790: rts
00015792: ori #0x1,CCR
00015796: rts
00015798: movea.l D0,A2
0001579A: move.w (A2),D3w
0001579C: move.w (0x4,A2),D2w
000157A0: movem.l {  A6 A5 A4 A3 A2 A1 A0 D4 D3 D2 D1 D0},-(SP) ; data 00015a4c
000157A4: jsr 0x00055848.l ; call 00055848 FUN_00055848
000157AA: movem.l (SP)+,{ D0 D1 D2 D3 D4 A0 A1 A2 A3 A4 A5 A6}
000157AE: move.b (0x00fff0dd).l,(0x9f,A1)
000157B6: lsl.w #0x4,D5w
000157B8: lsl.w #0x4,D6w
000157BA: add.w D5w,D2w
000157BC: add.w D6w,D3w
000157BE: move.w (A0),D5w
000157C0: move.w (0x4,A0),D4w
000157C4: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
000157CA: cmpi.w #0xbe0,D3w
000157CE: bcs.w 0x000157d6
000157D2: move.w #0xbe0,D3w
000157D6: move.w D2w,(0x2e,A1)
000157DA: swap D3
000157DC: clr.w D3w
000157DE: swap D3
000157E0: divu.w #0x80,D3
000157E4: move.w D3w,D0w
000157E6: add.w D3w,D0w
000157E8: move.w D0w,(0x40,A1)
000157EC: lsr.w #0x8,D2w
000157EE: andi.w #-0x2,D2w
000157F2: lea (0x15e56).l,A3 ; data 00015e56
000157F8: move.w (0x0,A3,D2w*0x1),D6w ; data 00015e56
000157FC: lea (0x15e16).l,A3 ; data 00015e16
00015802: move.w (0x0,A3,D2w*0x1),D7w ; data 00015e16
00015806: muls.w #0x8,D6
0001580A: asr.l #0x4,D6
0001580C: move.w D6w,(0x4c,A1)
00015810: muls.w #0x8,D7
00015814: asr.l #0x4,D7
00015816: move.w D7w,(0x4e,A1)
0001581A: mulu.w #0x20,D3
0001581E: neg.w D3w
00015820: move.w D3w,(0x36,A1)
00015824: andi #0x1e,CCR
00015828: rts
0001582A: jmp 0x0001a2cc.l
00015830: move.b (0x57,A0),D0b
00015834: cmpi.b #0x7,D0b
00015838: beq.w 0x00015850
0001583C: cmpi.b #0xe,D0b
00015840: beq.w 0x00015850
00015844: move.l (0xa2,A0),D3
00015848: bne.w 0x00015856
0001584C: bra.w 0x0001592e
00015850: jmp 0x000049a2.l ; call 000049A2 FUN_000049a2
00015856: movea.l D3,A1
00015858: tst.w (0x00ffdf84).l
0001585E: bne.w 0x00015862
00015862: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00015868: bsr.b 0x0001582a ; call 0001582A allocate_first_free_effect_slot
0001586A: bcs.w 0x0001592e
0001586E: move.l A0,(0xd8,A1)
00015872: movem.w {  D1w D0w},-(SP)
00015876: move.w (A0),D5w
00015878: move.w (0x4,A0),D4w
0001587C: move.w (A1),D3w
0001587E: move.w (0x4,A1),D2w
00015882: jsr 0x00015d3c.l ; call 00015D3C measure_relative_heading_and_distance
00015888: move.w D2w,(0x2e,A0)
0001588C: bsr.w 0x0001597e ; call 0001597E FUN_0001597e
00015890: move.b (0x57,A0),D2b
00015894: cmpi.b #0x6,D2b
00015898: beq.w 0x000158c0
0001589C: cmpi.b #0xd,D2b
000158A0: bne.w 0x000158dc
000158A4: clr.w (0x00ffdff6).l
000158AA: move.b (0x9d,A0),D2b
000158AE: ext.w D2w
000158B0: move.w D2w,(0x00ffdff8).l
000158B6: movea.l (0x00ffdffa).l,A1
000158BC: bra.w 0x000158dc
000158C0: clr.w (0x00ffdff2).l
000158C6: move.b (0x9d,A0),D2b
000158CA: ext.w D2w
000158CC: move.w D2w,(0x00ffdff4).l
000158D2: movea.l (0x00ffdffa).l,A1
000158D8: bra.w 0x000158dc
000158DC: movem.w (SP)+,{ D0 D1}
000158E0: move.b (0x57,A0),(0xb2,A1)
000158E6: move.w D0w,(0x32,A1)
000158EA: move.w D1w,(0x2a,A1)
000158EE: move.b #0x5,(0x2,A1)
000158F4: clr.b (0x3,A1)
000158F8: move.b (0x57,A0),D0b
000158FC: ext.w D0w
000158FE: movem.l {  A1 A0},-(SP)
00015902: exg A0,A1
00015904: move.w D0w,D1w
00015906: lsl.w #0x1,D0w
00015908: lea (0x4946).l,A2 ; data 00004946
0001590E: move.w (0x0,A2,D0w*0x1),D0w ; data 00004946
00015912: beq.w 0x0001591c
00015916: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001591C: lsl.w #0x2,D1w
0001591E: lea (0x4964).l,A2 ; data 00004964
00015924: movea.l (0x0,A2,D1w*0x1),A2 ; data 00004964
00015928: jsr (A2)
0001592A: movem.l (SP)+,{  A0 A1}
0001592E: rts
00015930: bsr.w 0x0001582a ; call 0001582A allocate_first_free_effect_slot
00015934: bcs.w 0x0001597c
00015938: move.b #0x2,(0xb2,A1)
0001593E: move.w D0w,(0x32,A1)
00015942: move.w D1w,(0x2a,A1)
00015946: clr.b (0x2,A1)
0001594A: clr.b (0x3,A1)
0001594E: move.b #0x2,D0b
00015952: ext.w D0w
00015954: movea.l A1,A0
00015956: move.w D0w,D1w
00015958: lsl.w #0x1,D0w
0001595A: lea (0x4946).l,A2 ; data 00004946
00015960: move.w (0x0,A2,D0w*0x1),D0w
00015964: beq.w 0x0001596e
00015968: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001596E: lsl.w #0x2,D1w
00015970: lea (0x4964).l,A2 ; data 00004964
00015976: movea.l (0x0,A2,D1w*0x1),A2 ; data 00004964
0001597A: jsr (A2) ; call 000049A0 FUN_000049a0
0001597C: rts
0001597E: move.w #0x1,D2w
00015982: move.b (0x57,A0),D0b
00015986: cmpi.b #0x3,D0b
0001598A: beq.w 0x000159b2
0001598E: cmpi.b #0xa,D0b
00015992: beq.w 0x000159b2
00015996: cmpi.b #0x5,D0b
0001599A: beq.w 0x000159b2
0001599E: clr.w D2w
000159A0: cmpi.b #0x6,D0b
000159A4: beq.w 0x000159b2
000159A8: cmpi.b #0xd,D0b
000159AC: beq.w 0x000159b2
000159B0: rts
000159B2: move.w #-0x1,(0x00ffdfea).l
000159BA: clr.w (0x00ffdfec).l
000159C0: move.w (0xa,A1),(0x00ffdfee).l
000159C8: move.w (0xc,A1),(0x00ffdff0).l
000159D0: move.l A0,D5
000159D2: move.l A0,-(SP) ; data Stack[-0x4]
000159D4: lea (0xff0100).l,A0 ; data 00ff0100
000159DA: move.w #0x9,D4w
000159DE: btst.b #0x0,(0x10,A0)
000159E4: beq.w 0x00015a40
000159E8: btst.b #0x6,(0x10,A0)
000159EE: bne.w 0x00015a40
000159F2: jsr 0x00015cc0.l ; call 00015CC0 compute_diagonal_step_cost
000159F8: cmpi.w #0x500,D3w
000159FC: bcc.w 0x00015a40
00015A00: cmpi.b #0xd,D0b
00015A04: bne.w 0x00015a16
00015A08: bset.b #0x1,(0xdf,A0)
00015A0E: move.l D5,(0xd8,A0)
00015A12: bra.w 0x00015a28
00015A16: cmpi.b #0x6,D0b
00015A1A: bne.w 0x00015a28
00015A1E: bset.b #0x0,(0xdf,A0)
00015A24: move.l D5,(0xd8,A0)
00015A28: move.b D0b,(0xde,A0)
00015A2C: tst.w D2w
00015A2E: bne.w 0x00015a40
00015A32: move.w #0x1,D2w
00015A36: move.b D0b,(0xb2,A0)
00015A3A: move.l A0,(0x00ffdffa).l
00015A40: adda.w #0x100,A0
00015A44: dbf D4w,0x000159de
00015A48: movea.l (SP)+,A0
00015A4A: rts
00015A8C: lea (0xff0b00).l,A1 ; data 00ff0b00
00015A92: move.w #0x5,D7w
00015A96: move.w #0x9,D0w
00015A9A: btst.b #0x0,(0x10,A1)
00015AA0: beq.w 0x00015ab4
00015AA4: adda.w #0x100,A1
00015AA8: addq.w #0x1,D0w
00015AAA: dbf D7w,0x00015a9a
00015AAE: ori #0x1,CCR
00015AB2: rts
00015AB4: andi #0x1e,CCR
00015AB8: rts
00015ABA: lea (0xff0100).l,A1 ; data 00ff0100
00015AC0: move.w #0x9,D7w
00015AC4: btst.b #0x0,(0x10,A1)
00015ACA: beq.w 0x00015ba6
00015ACE: btst.b #0x6,(0x10,A1)
00015AD4: bne.w 0x00015ba6
00015AD8: btst.b #0x4,(0x10,A1)
00015ADE: bne.w 0x00015ba6
00015AE2: tst.b (0x4c,A1)
00015AE6: beq.w 0x00015ba6
00015AEA: move.w (A1),D0w
00015AEC: subi.w #0x90,D0w
00015AF0: cmp.w D0w,D5w
00015AF2: bcs.w 0x00015ba6
00015AF6: addi.w #0x120,D0w
00015AFA: cmp.w D0w,D5w
00015AFC: bcc.w 0x00015ba6
00015B00: move.w (0x4,A1),D0w
00015B04: subi.w #0x90,D0w
00015B08: cmp.w D0w,D6w
00015B0A: bcs.w 0x00015ba6
00015B0E: addi.w #0x120,D0w
00015B12: cmp.w D0w,D6w
00015B14: bcc.w 0x00015ba6
00015B18: tst.w (0x00ffdfd8).l
00015B1E: beq.w 0x00015b32
00015B22: move.l A1,D3
00015B24: cmp.l (0x00ffdd3c).l,D3
00015B2A: bne.w 0x00015ba6
00015B2E: bra.w 0x00015ba0
00015B32: tst.b (0x9f,A0)
00015B36: bne.w 0x00015b78
00015B3A: btst.b #0x3,(0x10,A0)
00015B40: beq.w 0x00015b62
00015B44: btst.b #0x7,(0xb0,A1)
00015B4A: bne.w 0x00015ba6
00015B4E: btst.b #0x3,(0x10,A1)
00015B54: bne.w 0x00015ba6
00015B58: btst.b #0x4,(0x9e,A1)
00015B5E: bne.w 0x00015ba6
00015B62: move.w (A1),(A0)
00015B64: move.w (0x4,A1),D0w
00015B68: addi.w #0x10,D0w
00015B6C: move.w D0w,(0x4,A0)
00015B70: clr.w D1w
00015B72: ori #0x1,CCR
00015B76: rts
00015B78: btst.b #0x3,(0x10,A0)
00015B7E: beq.w 0x00015ba0
00015B82: btst.b #0x7,(0xb0,A1)
00015B88: bne.w 0x00015ba6
00015B8C: btst.b #0x3,(0x10,A1)
00015B92: bne.w 0x00015ba6
00015B96: btst.b #0x4,(0x9e,A1)
00015B9C: bne.w 0x00015ba6
00015BA0: ori #0x1,CCR
00015BA4: rts
00015BA6: adda.w #0x100,A1
00015BAA: dbf D7w,0x00015ac4
00015BAE: andi #0x1e,CCR
00015BB2: rts
00015BB4: rts
00015C58: btst.b #0x0,(0x10,A1)
00015C5E: beq.w 0x00015c7e
00015C62: btst.b #0x6,(0x10,A1)
00015C68: bne.w 0x00015c7e
00015C6C: bsr.w 0x00014d58 ; call 00014D58 FUN_00014d58
00015C70: cmpi.w #0x200,D3w
00015C74: bcc.w 0x00015c7e
00015C78: ori #0x1,CCR
00015C7C: rts
00015C7E: andi #0x1e,CCR
00015C82: rts
00015C84: move.l (0xa2,A0),D0
00015C88: beq.w 0x00015c9e
00015C8C: movea.l D0,A1
00015C8E: bsr.b 0x00015c58 ; call 00015C58 FUN_00015c58
00015C90: bcc.w 0x00015c9e
00015C94: move.w D2w,(0x34,A0)
00015C98: ori #0x1,CCR
00015C9C: rts
00015C9E: andi #0x1e,CCR
00015CA2: rts
00015CA4: sub.w D3w,D1w
00015CA6: bpl.b 0x00015caa
00015CA8: neg.w D1w
00015CAA: sub.w D2w,D0w
00015CAC: bpl.b 0x00015cb0
00015CAE: neg.w D0w
00015CB0: cmp.w D1w,D0w
00015CB2: bcc.b 0x00015cb6
00015CB4: exg D0,D1
00015CB6: asr.w #0x1,D1w
00015CB8: bcc.b 0x00015cbc
00015CBA: addq.w #0x1,D1w
00015CBC: add.w D1w,D0w
00015CBE: rts
00015CC0: move.w (0x4,A0),D2w
00015CC4: sub.w (0x4,A1),D2w
00015CC8: bpl.b 0x00015ccc
00015CCA: neg.w D2w
00015CCC: move.w (A0),D3w
00015CCE: sub.w (A1),D3w
00015CD0: bpl.b 0x00015cd4
00015CD2: neg.w D3w
00015CD4: cmp.w D2w,D3w
00015CD6: bcc.b 0x00015cda
00015CD8: exg D2,D3
00015CDA: asr.w #0x1,D2w
00015CDC: bcc.b 0x00015ce0
00015CDE: addq.w #0x1,D2w
00015CE0: add.w D2w,D3w
00015CE2: rts
00015D3C: movem.l {  D1 D0},-(SP)
00015D40: sub.w D4w,D2w
00015D42: sub.w D5w,D3w
00015D44: bsr.w 0x00015d4e ; call 00015D4E measure_heading_and_distance
00015D48: movem.l (SP)+,{ D0 D1}
00015D4C: rts
00015D4E: clr.w D0w
00015D50: clr.w D6w
00015D52: move.w D2w,D4w
00015D54: bpl.b 0x00015d58
00015D56: neg.w D4w
00015D58: move.w D3w,D5w
00015D5A: bpl.b 0x00015d5e
00015D5C: neg.w D5w
00015D5E: cmp.w D4w,D5w
00015D60: bcs.b 0x00015d64
00015D62: exg D4,D5
00015D64: move SR,D0w
00015D66: swap D0
00015D68: move.w D5w,D0w
00015D6A: lsr.w #0x1,D0w
00015D6C: add.w D4w,D0w
00015D6E: swap D0
00015D70: move.w #0x20,D1w
00015D74: cmp.w D1w,D4w
00015D76: bcs.b 0x00015d7e
00015D78: lsr.w #0x1,D4w
00015D7A: lsr.w #0x1,D5w
00015D7C: bra.b 0x00015d74
00015D7E: btst.l #0x0,D0
00015D82: bne.w 0x00015d88
00015D86: exg D4,D5
00015D88: tst.w D2w
00015D8A: move SR,D1w
00015D8C: andi.w #0xc,D1w
00015D90: move.w D1w,-(SP) ; data Stack[-0x2]
00015D92: tst.w D3w
00015D94: move SR,D1w
00015D96: lsr.w #0x2,D1w
00015D98: andi.w #0x3,D1w
00015D9C: or.w (SP)+,D1w
00015D9E: lsl.w #0x1,D1w
00015DA0: clr.l D2
00015DA2: lea (0x15df6).l,A5 ; data 00015df6
00015DA8: move.w (0x0,A5,D1w*0x1),D2w ; data 00015df6
00015DAC: btst.l #0x6,D2
00015DB0: bne.b 0x00015db4
00015DB2: exg D4,D5
00015DB4: lsl.w #0x5,D4w
00015DB6: add.w D5w,D4w
00015DB8: lea (0x15f96).l,A5 ; data 00015f96
00015DBE: add.b (0x0,A5,D4w*0x1),D2b ; data 00015f96
00015DC2: lsl.w #0x8,D2w
00015DC4: swap D0
00015DC6: clr.l D3
00015DC8: move.w D0w,D3w
00015DCA: rts
00015DCC: move.w D2w,D4w
00015DCE: move.w D3w,D2w
00015DD0: beq.b 0x00015df0
00015DD2: lsr.w #0x8,D4w
00015DD4: andi.w #0xfe,D4w
00015DD8: lea (0x15e16).l,A5 ; data 00015e16
00015DDE: muls.w (0x0,A5,D4w*0x1),D2 ; data 00015e16
00015DE2: add.l D2,D2
00015DE4: swap D2
00015DE6: muls.w (0x40,A5,D4w*0x1),D3 ; data 00015e56
00015DEA: add.l D3,D3
00015DEC: swap D3
00015DEE: rts
00015DF0: clr.w D2w
00015DF2: clr.w D3w
00015DF4: rts
00016396: bsr.w 0x000163da ; call 000163DA FUN_000163da
0001639A: tst.w (0x00ffdf3a).l
000163A0: beq.w 0x000163d8
000163A4: move.w (0x00ffdf3c).l,D0w
000163AA: andi.w #0x1,D0w
000163AE: bne.w 0x000163c0
000163B2: jsr 0x00001088.l ; call 00001088 step_palette_toward_black
000163B8: bset.b #0x0,(0x00ffef14).l
000163C0: subq.w #0x1,(0x00ffdf3c).l
000163C6: bpl.w 0x000163d8
000163CA: clr.w (0x00ffdf3a).l
000163D0: move.w #-0x1,(0x00ffdf3e).l
000163D8: rts
000163DA: cmpi.w #0x2,(0x00ffdf40).l
000163E2: bne.w 0x000163f2
000163E6: tst.b (0x00ffe170).l
000163EC: beq.w 0x000163f2
000163F0: rts
000163F2: lea (0xffd740).l,A0 ; data 00ffd740
000163F8: move.w #0x9,D7w
000163FC: tst.w (A0)
000163FE: beq.w 0x00016410
00016402: movea.l (0x8,A0),A1
00016406: movem.l {  A0 D7 D1},-(SP) ; data 00ffd740
0001640A: jsr (A1)
0001640C: movem.l (SP)+,{  D1 D7 A0}
00016410: adda.w #0x1e,A0
00016414: dbf D7w,0x000163fc
00016418: rts
0001641A: subq.w #0x1,(0x2,A0)
0001641E: bmi.w 0x00016424
00016422: rts
00016424: movea.l (0x10,A0),A3
00016428: move.w (A3)+,D0w
0001642A: move.w (0xe,A0),D1w
0001642E: addq.w #0x1,D1w
00016430: cmp.w D0w,D1w
00016432: blt.w 0x00016438
00016436: clr.w D1w
00016438: move.w D1w,(0xe,A0)
0001643C: move.w (A3)+,(0x2,A0)
00016440: movea.l (0x4,A0),A1
00016444: movea.l A1,A2
00016446: lsl.w #0x1,D1w
00016448: move.w (0x0,A3,D1w*0x1),D2w
0001644C: lsl.w #0x1,D2w
0001644E: adda.w D2w,A2
00016450: lsl.w #0x1,D0w
00016452: adda.w D0w,A3
00016454: move.w (A2),D1w
00016456: movea.l (0x00ffddde).l,A2
0001645C: lsl.w #0x5,D1w
0001645E: adda.w D1w,A2
00016460: move.w (A1),D1w
00016462: movea.l (0x00ffddde).l,A1
00016468: lsl.w #0x5,D1w
0001646A: adda.w D1w,A1
0001646C: move.b (A3)+,D0b
0001646E: bmi.w 0x000164a6
00016472: ext.w D0w
00016474: lsl.w #0x1,D0w
00016476: move.w (0x0,A2,D0w*0x1),D1w
0001647A: andi.w #0x7ff,D1w
0001647E: subi.w #0x201,D1w
00016482: lsl.w #0x5,D1w
00016484: movea.l (0x00ffddfe).l,A4
0001648A: adda.w D1w,A4
0001648C: move.w (0x0,A1,D0w*0x1),D1w
00016490: lsl.w #0x5,D1w
00016492: move.l A4,D0
00016494: move.w #0x20,D2w
00016498: nop
0001649A: nop
0001649C: nop
0001649E: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
000164A4: bra.b 0x0001646c
000164A6: rts
000164A8: movea.l (0x00ffde02).l,A2
000164AE: move.l (A2)+,D5
000164B0: beq.w 0x000164da
000164B4: bsr.w 0x000164f2 ; call 000164F2 FUN_000164f2
000164B8: bcs.w 0x000164da
000164BC: move.w #-0x1,(A0)
000164C0: move.l D5,(0x4,A0)
000164C4: move.l (A2)+,(0x10,A0)
000164C8: move.l #0x1641a,(0x8,A0) ; data 0001641a
000164D0: clr.w (0xe,A0)
000164D4: clr.w (0x2,A0)
000164D8: bra.b 0x000164ae
000164DA: rts
000164DC: lea (0xffd740).l,A0 ; data 00ffd740
000164E2: move.w #0x9,D7w
000164E6: clr.w (A0)
000164E8: adda.w #0x1e,A0
000164EC: dbf D7w,0x000164e6
000164F0: rts
000164F2: move.w (0x00ffdf6e).l,D1w
000164F8: lea (0xffd740).l,A0 ; data 00ffd740
000164FE: move.w #0x9,D7w
00016502: tst.w (A0)
00016504: beq.w 0x00016518
00016508: adda.w #0x1e,A0
0001650C: addq.w #0x1,D1w
0001650E: dbf D7w,0x00016502
00016512: ori #0x1,CCR
00016516: rts
00016518: andi #0x1e,CCR
0001651C: rts
0001651E: tst.w (0x00ffdf40).l
00016524: beq.w 0x00016608
00016528: tst.w D0w
0001652A: beq.w 0x0001666e
0001652E: cmpi.w #0x14,(0x00ffddc4).l
00016536: bne.w 0x00016578
0001653A: move.w (0x00ffdfe4).l,D2w
00016540: clr.b (0x0,A2,D2w*0x1)
00016544: move.w (0xc6,A0),D1w
00016548: andi.w #0xe0,D1w
0001654C: bsr.w 0x000166d0 ; call 000166D0 FUN_000166d0
00016550: lea (0xff2700).l,A1 ; data 00ff2700
00016556: adda.w D0w,A1
00016558: move.w (0x001cd296).l,(A1)
0001655E: move.w (0x00ffddea).l,D0w
00016564: adda.w D0w,A1
00016566: move.w (0x001cd298).l,(A1)
0001656C: move.w #0x1,(0x00ffe100).l
00016574: bra.w 0x00016674
00016578: cmpi.w #0x2,(0x00ffdf40).l
00016580: bne.w 0x00016674
00016584: move.w (0x00ffdfe4).l,D2w
0001658A: tst.b (0x0,A2,D2w*0x1)
0001658E: bmi.w 0x00016598
00016592: nop
00016594: nop
00016596: nop
00016598: clr.b (0x0,A2,D2w*0x1)
0001659C: move.w (0xc6,A0),D1w
000165A0: andi.w #0xe0,D1w
000165A4: cmpi.w #0xa0,D1w
000165A8: bne.w 0x000165e0
000165AC: bsr.w 0x0001669c ; call 0001669C FUN_0001669c
000165B0: lea (0xff4700).l,A1 ; data 00ff4700
000165B6: adda.w D0w,A1
000165B8: suba.w (0x00ffddea).l,A1
000165BE: move.w #0x3,(A1)
000165C2: move.w (0x00ffddea).l,D0w
000165C8: adda.w D0w,A1
000165CA: move.w #0x4,(A1)
000165CE: adda.w D0w,A1
000165D0: move.w #0x5,(A1)
000165D4: move.w #0x1,(0x00ffe102).l
000165DC: bra.w 0x00016674
000165E0: bsr.w 0x000166d0 ; call 000166D0 FUN_000166d0
000165E4: lea (0xff2700).l,A1 ; data 00ff2700
000165EA: adda.w D0w,A1
000165EC: move.w #0x6,(A1) ; data 00ff2700
000165F0: move.w (0x00ffddea).l,D0w
000165F6: adda.w D0w,A1
000165F8: move.w #0x7,(A1)
000165FC: move.w #0x1,(0x00ffe100).l
00016604: bra.w 0x00016674
00016608: cmpi.w #0x4,(0x00ffddc4).l
00016610: bne.w 0x0001662e
00016614: move.w (0xc6,A0),D0w
00016618: andi.w #0x1f,D0w
0001661C: cmpi.w #0x6,D0w
00016620: bne.w 0x0001662e
00016624: move.l #0x1667a,(0x00ffe114).l
0001662E: move.w (0xc6,A0),D0w
00016632: move.w D0w,D1w
00016634: andi.w #0xe0,D1w
00016638: cmpi.w #0xc0,D1w
0001663C: bne.w 0x0001666e
00016640: movem.l {  A0 D6 D5},-(SP)
00016644: bsr.w 0x000164f2 ; call 000164F2 FUN_000164f2
00016648: move.w #-0x1,(A0)
0001664C: add.w (0x00ffdd16).l,D4w
00016652: move.w D4w,(0x6,A0)
00016656: move.w #0x3,(0xc,A0)
0001665C: move.w #0x3,(0xe,A0)
00016662: move.l #0x1669a,(0x8,A0) ; data 0001669a
0001666A: movem.l (SP)+,{  D5 D6 A0}
0001666E: ori #0x1,CCR
00016672: rts
00016674: andi #0x1e,CCR
00016678: rts
0001668A: addq.b #0x8,D0b
0001668C: ori.b #0x0,D0b
00016690: bclr.l #-0x80,D0
00016694: btst.l D4,D0
00016696: addq.b #0x6,D0b
00016698: bclr.l D4,D0
0001669A: rts
0001669C: move.w (A0),D0w
0001669E: move.w (0x4,A0),D1w
000166A2: lsr.w #0x4,D0w
000166A4: lsr.w #0x4,D1w
000166A6: cmpi.w #-0x8000,(0x2e,A0)
000166AC: bcs.w 0x000166b8
000166B0: subi.w #0x10,D0w
000166B4: bra.w 0x000166bc
000166B8: addi.w #0x10,D0w
000166BC: lsr.w #0x5,D0w
000166BE: lsr.w #0x5,D1w
000166C0: subq.w #0x1,D1w
000166C2: lsl.w #0x1,D0w
000166C4: move.w (0x00ffddea).l,D2w
000166CA: mulu.w D2w,D1
000166CC: add.w D1w,D0w
000166CE: rts
000166D0: move.w (A0),D0w
000166D2: move.w (0x4,A0),D1w
000166D6: lsr.w #0x4,D0w
000166D8: lsr.w #0x4,D1w
000166DA: cmpi.w #0x4000,(0x2e,A0)
000166E0: bcs.w 0x000166f6
000166E4: cmpi.w #-0x4000,(0x2e,A0)
000166EA: bcc.w 0x000166f6
000166EE: addi.w #0x10,D1w
000166F2: bra.w 0x000166fa
000166F6: subi.w #0x10,D1w
000166FA: lsr.w #0x5,D0w
000166FC: lsr.w #0x5,D1w
000166FE: subq.w #0x1,D1w
00016700: lsl.w #0x1,D0w
00016702: move.w (0x00ffddea).l,D2w
00016708: mulu.w D2w,D1
0001670A: add.w D1w,D0w
0001670C: rts
00016760: move.w (0xc6,A0),D1w
00016764: andi.w #0xe0,D1w
00016768: bsr.w 0x000166d0 ; call 000166D0 FUN_000166d0
0001676C: lea (0xff2700).l,A1 ; data 00ff2700
00016772: adda.w D0w,A1
00016774: move.w (0x00ffddea).l,D0w
0001677A: adda.w D0w,A1
0001677C: cmpi.w #0xb,(A1)
00016780: bne.w 0x0001678a
00016784: andi #0x1e,CCR
00016788: rts
0001678A: move.l A0,-(SP) ; data Stack[-0x4]
0001678C: lea (0xffe179).l,A0 ; data 00ffe179
00016792: adda.w (0x00fff0c8).l,A0
00016798: addq.b #0x1,(A0) ; data 00ffe179
0001679A: movea.l (SP)+,A0
0001679C: move.w #0xb,(A1)
000167A0: move.w #0x1,(0x00ffe100).l
000167A8: ori #0x1,CCR
000167AC: rts
000167AE: clr.b (0x00ffe179).l
000167B4: clr.b (0x00ffe17a).l
000167BA: clr.b (0x00ffe17b).l
000167C0: clr.b (0x00ffe17c).l
000167C6: clr.b (0x00ffe17d).l
000167CC: clr.b (0x00ffe176).l
000167D2: clr.b (0x00ffe16f).l
000167D8: clr.b (0x00ffe170).l
000167DE: move.b (0x00fffbf9).l,D0b
000167E4: ext.w D0w
000167E6: movea.l (0x00ffde14).l,A1
000167EC: move.b (0x0,A1,D0w*0x1),D0b
000167F0: tst.b (0x00ffe186).l
000167F6: beq.w 0x00016814
000167FA: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00016800: move.w #0x36,D0w
00016804: cmpi.b #0x1,(0x00ffe186).l
0001680C: beq.w 0x00016814
00016810: move.w #0x37,D0w
00016814: tst.l (0x00ffe162).l
0001681A: beq.w 0x00016826
0001681E: bsr.w 0x00016a26 ; call 00016A26 FUN_00016a26
00016822: move.w #0xf,D0w
00016826: move.w D0w,-(SP) ; data Stack[-0x2]
00016828: bsr.w 0x00016e30 ; call 00016E30 FUN_00016e30
0001682C: move.w #-0x1,(0x00ffdf8e).l
00016834: move.w (SP)+,(0x00ffddc4).l
0001683A: jsr 0x00003074.l ; call 00003074 FUN_00003074
00016840: cmpi.w #0x15,(0x00ffddc4).l
00016848: bge.w 0x00016950
0001684C: move.w #0x1,(0x00ffdf40).l
00016854: tst.l (0x00ffe120).l
0001685A: bne.w 0x0001694c
0001685E: move.w (0x00ffddc4).l,D0w
00016864: subi.w #0xa,D0w
00016868: bne.w 0x00016884
0001686C: move.b #0x4,D0b
00016870: cmpi.b #0x6,(0x00fffbf9).l
00016878: bne.w 0x0001688e
0001687C: move.b #0x2,D0b
00016880: bra.w 0x0001688e
00016884: lea (0xeefe).l,A0 ; data 0000eefe
0001688A: move.b (0x0,A0,D0w*0x1),D0b
0001688E: cmpi.w #0x14,(0x00ffddc4).l
00016896: bne.w 0x000168bc
0001689A: move.l (0x00fffc2c).l,D0
000168A0: btst.l #0x1a,D0
000168A4: beq.w 0x0001694c
000168A8: bclr.l #0x1a,D0
000168AC: move.l D0,(0x00fffc2c).l
000168B2: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
000168B8: bra.w 0x0001694c
000168BC: move.l (0x00fffc1e).l,D1
000168C2: cmpi.w #0xd,(0x00ffddc4).l
000168CA: bne.w 0x000168da
000168CE: btst.l #0x13,D1
000168D2: beq.w 0x0001694c
000168D6: bra.w 0x000168ee
000168DA: cmpi.w #0xf,(0x00ffddc4).l
000168E2: bne.w 0x000168ee
000168E6: btst.l #0x1d,D1
000168EA: beq.w 0x0001694c
000168EE: addq.b #0x1,D0b
000168F0: cmp.b (0x00fffc0c).l,D0b
000168F6: beq.w 0x0001694c
000168FA: cmp.b (0x00fffc0d).l,D0b
00016900: beq.w 0x0001694c
00016904: move.w (0x00ffddc4).l,D0w
0001690A: subi.w #0xa,D0w
0001690E: bne.w 0x00016936
00016912: move.l #0x17034,(0x00ffe120).l
0001691C: cmpi.b #0x6,(0x00fffbf9).l
00016924: bne.w 0x00016a24
00016928: move.l #0x17018,(0x00ffe120).l
00016932: bra.w 0x00016a24
00016936: lsl.w #0x2,D0w
00016938: lea (0x16fd4).l,A0 ; data 00016fd4
0001693E: move.l (0x0,A0,D0w*0x1),D0
00016942: beq.w 0x00016a24
00016946: move.l D0,(0x00ffe120).l
0001694C: bra.w 0x00016a24
00016950: cmpi.w #0x2c,(0x00ffddc4).l
00016958: bge.w 0x000169f8
0001695C: move.w #0x2,(0x00ffdf40).l
00016964: clr.w (0x00fff0c8).l
0001696A: movea.l (0x00ffde1e).l,A1
00016970: movea.l (A1),A1
00016972: move.w (A1),D1w
00016974: move.w D1w,(0x00fff0c6).l
0001697A: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00016980: move.w D0w,(0x00ffe12c).l
00016986: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001698C: andi.w #0x7,D0w
00016990: move.b D0b,(0x00ffe185).l
00016996: tst.b (0x00ffe184).l
0001699C: beq.w 0x000169b0
000169A0: move.w #0x5,(0x00ffe12c).l
000169A8: move.b #0x0,(0x00ffe185).l
000169B0: tst.b (0x00ffe187).l
000169B6: beq.w 0x000169ca
000169BA: move.w #0x2,(0x00ffe12c).l
000169C2: move.b #0x3,(0x00ffe185).l
000169CA: tst.b (0x00ffe180).l
000169D0: beq.w 0x000169dc
000169D4: move.w #0x4,(0x00ffe12c).l
000169DC: movea.l (0x00ffde1e).l,A1
000169E2: movea.l (0x4,A1),A1
000169E6: move.l A1,(0x00ffe114).l
000169EC: bsr.w 0x00016b7c ; call 00016B7C FUN_00016b7c
000169F0: bsr.w 0x00016afa ; call 00016AFA FUN_00016afa
000169F4: bra.w 0x00016a14
000169F8: cmpi.w #0x38,(0x00ffddc4).l
00016A00: bge.w 0x00016a10
00016A04: move.w #0x3,(0x00ffdf40).l
00016A0C: bra.w 0x00016a14
00016A10: stop #0x2700
00016A14: move.w #0x1f,D7w
00016A18: lea (0xffb700).l,A1 ; data 00ffb700
00016A1E: clr.w (A1)+
00016A20: dbf D7w,0x00016a1e
00016A24: rts
00016A26: move.b #0x1,(0x00ffe18f).l
00016A2E: movea.l (0x00ffe162).l,A0
00016A34: lea (0xffe132).l,A2 ; data 00ffe132
00016A3A: lea (0x16a6a).l,A1 ; data 00016a6a
00016A40: tst.l (A0)
00016A42: beq.w 0x00016a50
00016A46: move.l (A0)+,(A2)+
00016A48: move.l A1,(A2)+
00016A4A: adda.w #0x6,A1
00016A4E: bra.b 0x00016a40
00016A50: clr.l (A2)
00016A52: move.l #0xffe132,(0x00ffe120).l
00016A5C: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
00016A62: clr.l (0x00ffe162).l
00016A68: rts
00016AFA: move.w (0x00ffddc4).l,D5w
00016B00: subi.w #0x15,D5w
00016B04: add.w D5w,D5w
00016B06: add.w D5w,D5w
00016B08: lea (0x16b20).l,A3 ; data 00016b20
00016B0E: move.l (0x0,A3,D5w*0x1),D0 ; data 00016b20
00016B12: move.w #0x4020,D1w
00016B16: move.w #0x200,D2w
00016B1A: jmp 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00016B7C: movea.l (0x00ffde1e).l,A1
00016B82: move.w (0x14,A1),D0w
00016B86: move.b D0b,(0x00fffbbc).l
00016B8C: move.w (0x16,A1),D0w
00016B90: move.w D0w,(0x00fff118).l
00016B96: rts
00016B98: cmpi.w #0x0,(0x2c,A0)
00016B9E: bne.w 0x00016bac
00016BA2: cmpi.w #0x7,(0xe,A0)
00016BA8: beq.w 0x00016c0a
00016BAC: cmpi.w #0xd,(0xe,A0)
00016BB2: bne.w 0x00016bc6
00016BB6: btst.b #0x0,(0x00fffc40).l
00016BBE: beq.w 0x00016cc6
00016BC2: bra.w 0x00016d86
00016BC6: cmpi.w #0xc,(0xe,A0)
00016BCC: bne.w 0x00016be0
00016BD0: btst.b #0x1,(0x00fffc40).l
00016BD8: beq.w 0x00016cc6
00016BDC: bra.w 0x00016d60
00016BE0: cmpi.w #0x2,(0x00ffdf40).l
00016BE8: beq.w 0x00016bf6
00016BEC: tst.w (0x00ffdf40).l
00016BF2: bne.w 0x00016cc6
00016BF6: cmpi.w #0xe,(0xe,A0)
00016BFC: beq.w 0x00016cc6
00016C00: tst.w (0x00fffc0e).l
00016C06: bne.w 0x00016cc6
00016C0A: bsr.w 0x00016e08 ; call 00016E08 FUN_00016e08
00016C0E: bcs.w 0x00016cc6
00016C12: move.w (A0),D0w
00016C14: addi.w #0x80,D0w
00016C18: move.w D0w,(A1)
00016C1A: move.w (0x4,A0),D0w
00016C1E: addi.w #0x80,D0w
00016C22: move.w D0w,(0x4,A1)
00016C26: bset.b #0x0,(0x10,A1)
00016C2C: cmpi.w #0x2,(0x00ffdf40).l
00016C34: bne.w 0x00016c42
00016C38: lea (0x16cf0).l,A2 ; data 00016cf0
00016C3E: bra.w 0x00016c70
00016C42: lea (0x16d50).l,A2 ; data 00016d50
00016C48: cmpi.w #0x0,(0x2c,A0)
00016C4E: bne.w 0x00016c5c
00016C52: cmpi.w #0x7,(0xe,A0)
00016C58: beq.w 0x00016c70
00016C5C: lea (0x16cc8).l,A2 ; data 00016cc8
00016C62: move.w (0x00ffddc4).l,D0w
00016C68: add.w D0w,D0w
00016C6A: add.w D0w,D0w
00016C6C: movea.l (0x0,A2,D0w*0x1),A2 ; data 00016cc8
00016C70: move.w #0x64,D1w
00016C74: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00016C7A: move.w D0w,D1w
00016C7C: move.w (A2)+,D0w
00016C7E: move.w (A2)+,D2w
00016C80: sub.w D2w,D1w
00016C82: bcc.b 0x00016c7c
00016C84: cmpi.w #0x5a,D0w
00016C88: bne.w 0x00016cb0
00016C8C: tst.b (0x56,A0)
00016C90: bmi.w 0x00016cb8
00016C94: tst.b (0x57,A0)
00016C98: beq.w 0x00016cb8
00016C9C: cmpi.b #0xe,(0x57,A0)
00016CA2: bgt.w 0x00016cb8
00016CA6: move.b (0x57,A0),D0b
00016CAA: ext.w D0w
00016CAC: bra.w 0x00016cc2
00016CB0: cmpi.w #0x5b,D0w
00016CB4: bne.w 0x00016cc2
00016CB8: bclr.b #0x0,(0x10,A1)
00016CBE: bra.w 0x00016cc6
00016CC2: move.w D0w,(0x2,A1)
00016CC6: rts
00016D60: move.w #0x65,D6w
00016D64: bsr.w 0x00016d8a ; call 00016D8A FUN_00016d8a
00016D68: bcs.w 0x00016d84
00016D6C: move.w A0w,(0x6,A1)
00016D70: move.w #0x80,(0x16,A0)
00016D76: bset.b #0x0,(0x11,A0)
00016D7C: move.l #0x49a0,(0x3a,A0) ; data 000049a0
00016D84: rts
00016D86: move.w #0x64,D6w
00016D8A: bsr.w 0x00016e08 ; call 00016E08 FUN_00016e08
00016D8E: bcc.w 0x00016d98
00016D92: ori #0x1,CCR
00016D96: rts
00016D98: move.w (A0),D0w
00016D9A: addi.w #0x80,D0w
00016D9E: move.w D0w,(A1)
00016DA0: move.w (0x4,A0),D0w
00016DA4: addi.w #0x80,D0w
00016DA8: move.w D0w,(0x4,A1)
00016DAC: bset.b #0x0,(0x10,A1)
00016DB2: move.w D6w,(0x2,A1)
00016DB6: andi #0x1e,CCR
00016DBA: rts
00016DEE: lea (0xff164c).l,A0 ; data 00ff164c
00016DF4: move.w #0x9,D7w
00016DF8: bclr.b #0x0,(0x10,A0)
00016DFE: adda.w #0x12,A0
00016E02: dbf D7w,0x00016df8
00016E06: rts
00016E08: lea (0xff164c).l,A1 ; data 00ff164c
00016E0E: move.w #0x9,D7w
00016E12: btst.b #0x0,(0x10,A1)
00016E18: beq.w 0x00016e2a
00016E1C: adda.w #0x12,A1
00016E20: dbf D7w,0x00016e12
00016E24: ori #0x1,CCR
00016E28: rts
00016E2A: andi #0x1e,CCR
00016E2E: rts
00016E30: move.w #-0x1,(0x00ffdf8a).l
00016E38: lea (0xffddc4).l,A0 ; data 00ffddc4
00016E3E: lea (0xffde26).l,A1 ; data 00ffde26
00016E44: move.w #0x30,D7w
00016E48: move.w (A0)+,(A1)+
00016E4A: dbf D7w,0x00016e48
00016E4E: jsr 0x0001a322.l ; call 0001A322 FUN_0001a322
00016E54: jsr 0x0001a310.l ; call 0001A310 FUN_0001a310
00016E5A: rts
00016E5C: lea (0xffb700).l,A4 ; data 00ffb700
00016E62: movea.l (0x00ffde1e).l,A1
00016E68: movea.l (0x8,A1),A1
00016E6C: move.w (A1)+,D0w
00016E6E: beq.w 0x00016e9e
00016E72: move.w (A1)+,D1w
00016E74: move.w (A1)+,D2w
00016E76: tst.b (A4)+
00016E78: beq.w 0x00016e8a
00016E7C: tst.w D2w
00016E7E: bpl.w 0x00016e9c
00016E82: adda.w #0x6,A1
00016E86: bra.w 0x00016e9c
00016E8A: tst.w D2w
00016E8C: bpl.w 0x00016e94
00016E90: suba.w #0x1,A4
00016E94: andi.w #0x7fff,D2w
00016E98: bsr.w 0x00016ea0 ; call 00016EA0 FUN_00016ea0
00016E9C: bra.b 0x00016e6c
00016E9E: rts
00016EA0: lea (0xff4700).l,A2 ; data 00ff4700
00016EA6: subq.w #0x1,D0w
00016EA8: subq.w #0x1,D1w
00016EAA: move.w #0x2,D3w
00016EAE: move.w D0w,D7w
00016EB0: move.w D2w,D5w
00016EB2: move.w D3w,(0x0,A2,D5w*0x1) ; data 00ff4700
00016EB6: addq.w #0x2,D5w
00016EB8: dbf D7w,0x00016eb2
00016EBC: add.w (0x00ffddea).l,D2w
00016EC2: dbf D1w,0x00016eae
00016EC6: rts
00016ECA: movea.l (0x00fffc04).l,A0
00016ED0: move.w (A0),D5w
00016ED2: move.w (0x4,A0),D6w
00016ED6: movea.l (0x00ffde1e).l,A1
00016EDC: movea.l (0x8,A1),A1
00016EE0: lea (0xffb700).l,A2 ; data 00ffb700
00016EE6: move.w (A1)+,D0w
00016EE8: beq.w 0x00016f12
00016EEC: move.w (A1)+,D1w
00016EEE: move.w (A1)+,D2w
00016EF0: tst.b (A2)+
00016EF2: beq.w 0x00016f00
00016EF6: tst.w D2w
00016EF8: bpl.b 0x00016ee6
00016EFA: adda.w #0x6,A1
00016EFE: bra.b 0x00016ee6
00016F00: bsr.w 0x00016f14 ; call 00016F14 FUN_00016f14
00016F04: bcc.b 0x00016ee6
00016F06: bsr.w 0x00016f8e ; call 00016F8E FUN_00016f8e
00016F0A: move.w #0x1,(0x00ffe102).l
00016F12: rts
00016F14: tst.w D2w
00016F16: bpl.w 0x00016f3c
00016F1A: andi.w #0x7fff,D2w
00016F1E: bsr.w 0x00016f3c ; call 00016F3C FUN_00016f3c
00016F22: bcc.w 0x00016f28
00016F26: rts
00016F28: move.w (A1)+,D0w
00016F2A: move.w (A1)+,D1w
00016F2C: move.w (A1)+,D2w
00016F2E: bsr.w 0x00016f3c ; call 00016F3C FUN_00016f3c
00016F32: bcc.w 0x00016f3a
00016F36: suba.w #0x6,A1
00016F3A: rts
00016F3C: subq.w #0x2,D2w
00016F3E: addq.w #0x1,D0w
00016F40: movem.w {  D1w D0w},-(SP)
00016F44: move.w D2w,D0w
00016F46: jsr 0x00014640.l ; call 00014640 FUN_00014640
00016F4C: movem.w (SP)+,{  D2 D3}
00016F50: exg D0,D1
00016F52: addi.w #0x200,D1w
00016F56: cmp.w D0w,D5w
00016F58: bcs.w 0x00016f88
00016F5C: cmp.w D1w,D6w
00016F5E: bcs.w 0x00016f88
00016F62: ror.w #0x7,D2w
00016F64: ror.w #0x7,D3w
00016F66: add.w D0w,D2w
00016F68: add.w D1w,D3w
00016F6A: cmp.w D2w,D5w
00016F6C: bcc.w 0x00016f88
00016F70: cmp.w D3w,D6w
00016F72: bcc.w 0x00016f88
00016F76: suba.w #0x6,A1
00016F7A: suba.w #0x1,A2
00016F7E: move.b #-0x1,(A2)
00016F82: ori #0x1,CCR
00016F86: rts
00016F88: andi #0x1e,CCR
00016F8C: rts
00016F8E: move.w (A1)+,D0w
00016F90: move.w (A1)+,D1w
00016F92: move.w (A1)+,D2w
00016F94: tst.w D2w
00016F96: bpl.w 0x00016fa8
00016F9A: andi.w #0x7fff,D2w
00016F9E: bsr.w 0x00016fa8 ; call 00016FA8 FUN_00016fa8
00016FA2: move.w (A1)+,D0w
00016FA4: move.w (A1)+,D1w
00016FA6: move.w (A1),D2w
00016FA8: lea (0xff4700).l,A2 ; data 00ff4700
00016FAE: subq.w #0x1,D0w
00016FB0: subq.w #0x1,D1w
00016FB2: movea.l (0x00ffdde2).l,A0
00016FB8: move.w D0w,D7w
00016FBA: move.w D2w,D5w
00016FBC: move.w (0x0,A0,D5w*0x1),(0x0,A2,D5w*0x1) ; data 00ff4700
00016FC2: addq.w #0x2,D5w
00016FC4: dbf D7w,0x00016fbc
00016FC8: add.w (0x00ffddea).l,D2w
00016FCE: dbf D1w,0x00016fb8
00016FD2: rts
000173F2: tst.b (0x00ffe16f).l
000173F8: beq.w 0x00017408
000173FC: move.b #0x1,(0x00fff0a0).l
00017404: bra.w 0x00017410
00017408: move.b #-0x1,(0x00fff0a0).l
00017410: move.b #-0x1,(0x00fff0b0).l
00017418: bsr.w 0x0001745e ; call 0001745E FUN_0001745e
0001741C: clr.b (0x00fff0e3).l
00017422: clr.b (0x00fff0b0).l
00017428: move.w #-0x7ecc,(0x00c00004).l
00017430: movea.l (0x00ffddfa).l,A6
00017436: jsr 0x00001406.l ; call 00001406 FUN_00001406
0001743C: cmpi.w #0x2,(0x00ffdf40).l
00017444: bne.w 0x0001745c
00017448: cmpi.b #0x1,(0x00fff0a0).l
00017450: bne.w 0x0001745c
00017454: move.b #0x1,(0x00ffe191).l
0001745C: rts
0001745E: move.w #-0x7ecc,(0x00c00004).l
00017466: move.b (0x00fff0a0).l,(0x00ffe189).l
00017470: lea (0xfffbbe).l,A0 ; data 00fffbbe
00017476: move.b #0xa,(0xc,A0)
0001747C: clr.b (0x00fff0a2).l
00017482: clr.b (0x00fff0b1).l
00017488: move.w #-0x1,(0x00fff11a).l
00017490: move.b (0x00fffbbc).l,D0b
00017496: cmp.b (0x00fffbbd).l,D0b
0001749C: beq.w 0x000174b0
000174A0: jsr 0x00053290.l ; call 00053290 FUN_00053290
000174A6: move.b (0x00fffbbc).l,(0x00fffbbd).l
000174B0: clr.w (0x00ffdd48).l
000174B6: jsr 0x000531ea.l ; call 000531EA FUN_000531ea
000174BC: bsr.w 0x00017eb4 ; call 00017EB4 FUN_00017eb4
000174C0: clr.b (0x00fff0d2).l
000174C6: move.w #-0x7ecc,(0x00c00004).l
000174CE: move.b (0x00fff0a7).l,(0x00fff0e5).l
000174D8: clr.b (0x00fff0b7).l
000174DE: clr.w (0x00fff0e6).l
000174E4: clr.b (0x00ffdf84).l
000174EA: clr.b (0x00fff0a6).l
000174F0: clr.b (0x00fff0a9).l
000174F6: clr.b (0x00fff0ad).l
000174FC: move.b #0x1,(0x00fff0a3).l
00017504: move.b #0xc,(0x00fff0e4).l
0001750C: move.b #0x28,(0x00fff0ac).l
00017514: jsr 0x000533ee.l ; call 000533EE FUN_000533ee
0001751A: tst.w (0x00ffdf40).l
00017520: beq.w 0x0001752c
00017524: move.b #-0x1,(0x00ffe171).l
0001752C: jsr 0x00053126.l ; call 00053126 FUN_00053126
00017532: move.w #-0x1,(0x00ffe172).l
0001753A: move.w #-0x1,(0x00ffdf8a).l
00017542: move.w #-0x1,(0x00fff106).l
0001754A: clr.b (0x00fff0e1).l
00017550: move.b #0x5,(0x00fff0de).l
00017558: clr.b (0x00fff0df).l
0001755E: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
00017564: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0001756A: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00017570: move.w #-0x74fd,(0x00c00004).l
00017578: jsr 0x0001b5a6.l ; call 0001B5A6 FUN_0001b5a6
0001757E: move.w #-0x6f00,(0x00c00004).l
00017586: bsr.w 0x000176ea ; call 000176EA FUN_000176ea
0001758A: bsr.w 0x000177cc ; call 000177CC FUN_000177cc
0001758E: lea (0xffbf40).l,A1 ; data 00ffbf40
00017594: movea.l #0xd646a,A3
0001759A: move.l #0x8160,D0
000175A0: jsr 0x00001530.l ; call 00001530 FUN_00001530
000175A6: lea (0xd806c).l,A1 ; data 000d806c
000175AC: move.l #0xe000,D4
000175B2: move.w #0x27,D7w
000175B6: move.w #0xc,D1w
000175BA: move.w #0x6000,D2w
000175BE: clr.w D3w
000175C0: move.w D7w,D5w
000175C2: addq.w #0x1,D5w
000175C4: jsr 0x00001728.l ; call 00001728 FUN_00001728
000175CA: lea (0xd74f4).l,A1 ; data 000d74f4
000175D0: move.l #0xe680,D4
000175D6: move.w #0x27,D7w
000175DA: move.w #0x8,D1w
000175DE: move.w #0x6000,D2w
000175E2: clr.w D3w
000175E4: move.w D7w,D5w
000175E6: addq.w #0x1,D5w
000175E8: jsr 0x00001728.l ; call 00001728 FUN_00001728
000175EE: bsr.w 0x00017708 ; call 00017708 FUN_00017708
000175F2: bsr.w 0x000177a4 ; call 000177A4 FUN_000177a4
000175F6: bsr.w 0x00019856 ; call 00019856 FUN_00019856
000175FA: jsr 0x000033be.l ; call 000033BE FUN_000033be
00017600: bsr.w 0x000176ba ; call 000176BA FUN_000176ba
00017604: jsr 0x0001c4ee.l ; call 0001C4EE FUN_0001c4ee
0001760A: lea (0xff1580).l,A0 ; data 00ff1580
00017610: move.w #0x120,(0xe,A0)
00017616: move.w #0x12f,(0x10,A0)
0001761C: jsr 0x0001b628.l ; call 0001B628 FUN_0001b628
00017622: move.b #0x1e,(0x2,A0)
00017628: move.b #0x1e,(0x3,A0)
0001762E: bsr.w 0x0001789c ; call 0001789C FUN_0001789c
00017632: move.w #-0x7e8c,(0x00c00004).l
0001763A: move.w #0x5b,D0w
0001763E: jsr 0x000530de.l ; call 000530DE FUN_000530de
00017644: bsr.w 0x000177fc ; call 000177FC FUN_000177fc
00017648: move.w #0x5b,D0w
0001764C: jsr 0x000530d2.l ; call 000530D2 FUN_000530d2
00017652: move.w #0x3,D0w
00017656: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0001765C: lea (0xd74f4).l,A1 ; data 000d74f4
00017662: move.l #0xe680,D4
00017668: move.w #0x27,D7w
0001766C: move.w #0x8,D1w
00017670: move.w #0x6000,D2w
00017674: clr.w D3w
00017676: move.w D7w,D5w
00017678: addq.w #0x1,D5w
0001767A: jsr 0x00001728.l ; call 00001728 FUN_00001728
00017680: jsr 0x0001ade6.l ; call 0001ADE6 FUN_0001ade6
00017686: tst.b (0x00fff0b1).l
0001768C: beq.w 0x000174c6
00017690: move.w #-0x7ecc,(0x00c00004).l
00017698: move.w #-0x6e6f,(0x00c00004).l
000176A0: jsr 0x00003404.l ; call 00003404 FUN_00003404
000176A6: jsr 0x0000334c.l ; call 0000334C FUN_0000334c
000176AC: jsr 0x000039f2.l ; call 000039F2 FUN_000039f2
000176B2: jsr 0x00053126.l ; call 00053126 FUN_00053126
000176B8: rts
000176BA: move.l #0xda038,D0
000176C0: move.l #0x2360,D1
000176C6: move.l #0xa80,D2
000176CC: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
000176D2: lea (0xffbf40).l,A1 ; data 00ffbf40
000176D8: movea.l #0xdaab8,A3
000176DE: move.l #0x2de0,D0
000176E4: jmp 0x00001530.l ; call 00001530 FUN_00001530
000176EA: lea (0xe2e3a).l,A0 ; data 000e2e3a
000176F0: jsr 0x00001358.l ; call 00001358 copy_source_word_block
000176F6: bset.b #0x0,(0x00ffef14).l
000176FE: bsr.w 0x00019df2 ; call 00019DF2 FUN_00019df2
00017702: bsr.w 0x00017f8e ; call 00017F8E FUN_00017f8e
00017706: rts
00017708: move.w #-0x7ecc,(0x00c00004).l
00017710: lea (0xd8316).l,A1 ; data 000d8316
00017716: move.l #0xeb00,D4
0001771C: move.w #0x27,D7w
00017720: move.w #0x5,D1w
00017724: move.w #0x6000,D2w
00017728: clr.w D3w
0001772A: move.w D7w,D5w
0001772C: addq.w #0x1,D5w
0001772E: jsr 0x00001728.l ; call 00001728 FUN_00001728
00017734: lea (0xfffbbe).l,A6 ; data 00fffbbe
0001773A: move.w #0x4,D6w
0001773E: move.l D6,D4
00017740: clr.l D7
00017742: move.b (0x1a,A6,D4w*0x1),D7b ; data 00fffbd8
00017746: bpl.w 0x00017754
0001774A: lea (0xd92aa).l,A1 ; data 000d92aa
00017750: bra.w 0x00017760
00017754: mulu.w #0xc,D7
00017758: lea (0xd92b6).l,A1 ; data 000d92b6
0001775E: adda.l D7,A1
00017760: mulu.w #0xc,D4
00017764: addi.w #-0x146c,D4w
00017768: moveq #0x2,D7
0001776A: moveq #0x1,D1
0001776C: move.w #0x6000,D2w
00017770: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017776: dbf D6w,0x0001773e
0001777A: bsr.w 0x00019856 ; call 00019856 FUN_00019856
0001777E: tst.b (0x00ffdf84).l
00017784: beq.w 0x000177a2
00017788: bsr.w 0x00019bae ; call 00019BAE FUN_00019bae
0001778C: cmpi.w #0x8,D0w
00017790: bne.w 0x000177a2
00017794: bsr.w 0x00017d72 ; call 00017D72 FUN_00017d72
00017798: clr.l D1
0001779A: bsr.w 0x00017d90 ; call 00017D90 FUN_00017d90
0001779E: bsr.w 0x00017f8e ; call 00017F8E FUN_00017f8e
000177A2: rts
000177A4: lea (0xfff0ed).l,A4 ; data 00fff0ed
000177AA: lea (0xfff0e8).l,A5 ; data 00fff0e8
000177B0: lea (0xfffbbe).l,A6 ; data 00fffbbe
000177B6: move.w #0x4,D6w
000177BA: move.b #0x6,(0x0,A4,D6w*0x1)
000177C0: move.b #0x1,(0x0,A5,D6w*0x1)
000177C6: dbf D6w,0x000177ba
000177CA: rts
000177CC: lea (0xffbf40).l,A1 ; data 00ffbf40
000177D2: movea.l #0xd86e0,A3
000177D8: move.l #0x17e0,D0
000177DE: jsr 0x00001530.l ; call 00001530 FUN_00001530
000177E4: lea (0xffbf40).l,A1 ; data 00ffbf40
000177EA: movea.l #0xd78aa,A3
000177F0: move.l #0x580,D0
000177F6: jmp 0x00001530.l ; call 00001530 FUN_00001530
000177FC: move.w #0x3c,D5w
00017800: move.w D5w,-(SP) ; data Stack[-0x2]
00017802: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00017808: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001780E: bsr.w 0x00017cc8 ; call 00017CC8 FUN_00017cc8
00017812: bsr.w 0x00017fda ; call 00017FDA FUN_00017fda
00017816: move.w #0x1,(0x00ffdd02).l
0001781E: lea (0xffd986).l,A3 ; data 00ffd986
00017824: move.w #0x2,D5w
00017828: bsr.w 0x0001c3fa ; call 0001C3FA FUN_0001c3fa
0001782C: tst.b (0x00fff0e1).l
00017832: beq.w 0x0001783c
00017836: jsr 0x000268e6.l ; call 000268E6 FUN_000268e6
0001783C: subq.w #0x8,A3
0001783E: clr.b (0x3,A3)
00017842: move.w #0x1,(0x00ffd980).l
0001784A: bset.b #0x1,(0x00ffef14).l
00017852: move.w (SP)+,D5w
00017854: dbf D5w,0x00017800
00017858: clr.l D0
0001785A: lea (0xe2eda).l,A5 ; data 000e2eda
00017860: lea (0xff1100).l,A1 ; data 00ff1100
00017866: move.w (0x00fff118).l,D0w
0001786C: mulu.w #0x14,D0
00017870: move.b (0x0,A1,D0w*0x1),D0b ; data 00ff1100
00017874: ext.w D0w
00017876: mulu.w #0xc,D0
0001787A: adda.l D0,A5
0001787C: jsr 0x000266e4.l ; call 000266E4 FUN_000266e4
00017882: jsr 0x0002679e.l ; call 0002679E FUN_0002679e
00017888: clr.b (0x00fff0d4).l
0001788E: lea (0xfffbbe).l,A6 ; data 00fffbbe
00017894: clr.w D5w
00017896: bsr.w 0x000194ac ; call 000194AC FUN_000194ac
0001789A: rts
0001789C: move.w #-0x2000,D2w
000178A0: move.l #0xc004,D4
000178A6: bsr.w 0x000178c8 ; call 000178C8 FUN_000178c8
000178AA: move.l #0xc02c,D4
000178B0: bsr.w 0x000178c8 ; call 000178C8 FUN_000178c8
000178B4: bsr.w 0x00017bc0 ; call 00017BC0 FUN_00017bc0
000178B8: clr.l D1
000178BA: bsr.w 0x0001989a ; call 0001989A FUN_0001989a
000178BE: bsr.w 0x00017a5e ; call 00017A5E FUN_00017a5e
000178C2: bsr.w 0x00017998 ; call 00017998 FUN_00017998
000178C6: rts
000178C8: lea (0xd86c8).l,A1 ; data 000d86c8
000178CE: moveq #0x0,D7
000178D0: moveq #0x0,D1
000178D2: jsr 0x00001232.l ; call 00001232 FUN_00001232
000178D8: addi.l #0x1e,D4
000178DE: lea (0xd86ca).l,A1 ; data 000d86ca
000178E4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000178EA: addi.l #0x1e2,D4
000178F0: lea (0xd86cc).l,A1 ; data 000d86cc
000178F6: moveq #0x1,D7
000178F8: moveq #0x1,D1
000178FA: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017900: addi.l #0x1c,D4
00017906: lea (0xd86d4).l,A1 ; data 000d86d4
0001790C: moveq #0x1,D7
0001790E: moveq #0x1,D1
00017910: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017916: addi.l #0x72,D4
0001791C: lea (0xd86dc).l,A1 ; data 000d86dc
00017922: moveq #0x1,D7
00017924: moveq #0x0,D1
00017926: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001792C: rts
0001792E: move.w #0x4,D3w
00017932: move.l #0xebc0,D4
00017938: move.l D4,-(SP) ; data Stack[-0x4]
0001793A: tst.b (0x00fff0b0).l
00017940: beq.w 0x00017962
00017944: move.l D3,-(SP) ; data Stack[-0x8]
00017946: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
0001794A: move.w D3w,D6w
0001794C: move.l (SP)+,D3
0001794E: tst.w D6w
00017950: beq.w 0x00017962
00017954: lea (0xfffbbe).l,A0 ; data 00fffbbe
0001795A: tst.b (0x1a,A0,D3w*0x1) ; data 00fffbd8
0001795E: bpl.w 0x00017968
00017962: clr.l D6
00017964: bra.w 0x00017970
00017968: move.w D3w,D7w
0001796A: jsr 0x00056506.l ; call 00056506 FUN_00056506
00017970: mulu.w #0x8,D6
00017974: lea (0xd866a).l,A1 ; data 000d866a
0001797A: adda.l D6,A1
0001797C: move.w #0x6000,D2w
00017980: moveq #0x0,D7
00017982: moveq #0x3,D1
00017984: move.l (SP)+,D4
00017986: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001798C: subi.l #0xc,D4
00017992: dbf D3w,0x00017938
00017996: rts
00017998: clr.l D2
0001799A: lea (0xe309e).l,A1 ; data 000e309e
000179A0: lea (0xff1100).l,A6 ; data 00ff1100
000179A6: move.w (0x00fff118).l,D0w
000179AC: mulu.w #0x14,D0
000179B0: move.b (0x0,A6,D0w*0x1),D2b ; data 00ff1100
000179B4: lsl.w #0x2,D2w
000179B6: adda.l D2,A1
000179B8: movea.l #0xc0b0,A2
000179BE: move.w #-0x8000,D1w
000179C2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000179C8: lea (0xe30b6).l,A1 ; data 000e30b6
000179CE: movea.l #0xc0b8,A2
000179D4: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
000179D8: beq.w 0x00017a40
000179DC: move.b (0x5,A6,D0w*0x1),D2b ; data 00ff1105
000179E0: lsl.w #0x3,D2w
000179E2: adda.l D2,A1
000179E4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000179EA: cmpi.w #0x3,D3w
000179EE: bmi.w 0x00017a0c
000179F2: movea.l #0xc12e,A2
000179F8: lea (0xe4060).l,A1 ; data 000e4060
000179FE: move.b (0x1,A6,D0w*0x1),D2b ; data 00ff1101
00017A02: lsl.w #0x4,D2w
00017A04: adda.l D2,A1
00017A06: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017A0C: cmpi.w #0x2,D3w
00017A10: bmi.w 0x00017a34
00017A14: clr.l D1
00017A16: move.b (0x6,A6,D0w*0x1),D1b ; data 00ff1106
00017A1A: movea.l #0xc0c6,A2
00017A20: move.w #-0x8000,D2w
00017A24: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00017A2A: cmpi.w #0x3,D3w
00017A2E: bmi.w 0x00017a46
00017A32: rts
00017A34: lea (0xe4402).l,A1 ; data 000e4402
00017A3A: movea.l #0xc0c6,A2
00017A40: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017A46: lea (0xe30b6).l,A1 ; data 000e30b6
00017A4C: movea.l #0xc134,A2
00017A52: move.w #-0x8000,D1w
00017A56: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017A5C: rts
00017A5E: move.w (0x00fff118).l,D0w
00017A64: mulu.w #0x14,D0
00017A68: lea (0xff1100).l,A6 ; data 00ff1100
00017A6E: clr.l D6
00017A70: move.b (0x13,A6,D0w*0x1),D6b ; data 00ff1113
00017A74: bpl.w 0x00017aa6
00017A78: bsr.w 0x00019dac ; call 00019DAC FUN_00019dac
00017A7C: lea (0xfffb30).l,A4 ; data 00fffb30
00017A82: move.w (0x00fff102).l,D3w
00017A88: ori.b #0x40,(0x0,A4,D3w*0x1) ; data 00fffb30
00017A8E: lea (0xe3177).l,A1 ; data 000e3177
00017A94: movea.l #0xc1ae,A2
00017A9A: move.w #-0x8000,D1w
00017A9E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017AA4: rts
00017AA6: tst.b (0xf,A6,D0w*0x1) ; data 00ff110f
00017AAA: bgt.w 0x00017ab6
00017AAE: lea (0xe3197).l,A1 ; data 000e3197
00017AB4: bra.b 0x00017a94
00017AB6: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
00017ABA: bgt.w 0x00017ac6
00017ABE: lea (0xe3187).l,A1 ; data 000e3187
00017AC4: bra.b 0x00017a94
00017AC6: tst.b (0x00ffdf84).l
00017ACC: beq.w 0x00017ad4
00017AD0: bsr.w 0x00017e04 ; call 00017E04 FUN_00017e04
00017AD4: lea (0xe30de).l,A4 ; data 000e30de
00017ADA: lea (0xe31a7).l,A5 ; data 000e31a7
00017AE0: lsl.w #0x1,D6w
00017AE2: adda.l D6,A5
00017AE4: clr.l D6
00017AE6: move.b (A5)+,D6b ; data 000e31a7
00017AE8: bmi.w 0x00017bbe
00017AEC: move.l D6,D5
00017AEE: mulu.w #0x10,D6
00017AF2: lea (0xe30e7).l,A1 ; data 000e30e7
00017AF8: adda.l D6,A1
00017AFA: movea.l #0xc1ae,A2
00017B00: move.w #-0x8000,D1w
00017B04: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017B0A: move.b (0x0,A4,D5w*0x1),D5b ; data 000e30de
00017B0E: mulu.w #0x2,D5
00017B12: adda.l D5,A2
00017B14: cmpi.w #0x2,D3w
00017B18: bmi.w 0x00017b46
00017B1C: clr.l D1
00017B1E: move.b (0x11,A6,D0w*0x1),D1b ; data 00ff1111
00017B22: tst.b (0x00fff0a0).l
00017B28: bmi.w 0x00017b38
00017B2C: beq.w 0x00017b36
00017B30: addq.w #0x2,D1w
00017B32: bra.w 0x00017b38
00017B36: addq.w #0x1,D1w
00017B38: move.w #-0x8000,D2w
00017B3C: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00017B42: bra.w 0x00017b52
00017B46: lea (0xe4402).l,A1 ; data 000e4402
00017B4C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017B52: clr.l D6
00017B54: move.b (A5)+,D6b ; data 000e31a8
00017B56: bmi.w 0x00017bbe
00017B5A: move.l D6,D5
00017B5C: mulu.w #0x10,D6
00017B60: lea (0xe30e7).l,A1 ; data 000e30e7
00017B66: adda.l D6,A1
00017B68: movea.l #0xc22e,A2
00017B6E: move.w #-0x8000,D1w
00017B72: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017B78: move.b (0x0,A4,D5w*0x1),D5b ; data 000e30de
00017B7C: mulu.w #0x2,D5
00017B80: adda.l D5,A2
00017B82: cmpi.w #0x2,D3w
00017B86: bmi.w 0x00017bb2
00017B8A: clr.l D1
00017B8C: move.b (0x12,A6,D0w*0x1),D1b ; data 00ff1112
00017B90: tst.b (0x00fff0a0).l
00017B96: bmi.w 0x00017ba6
00017B9A: beq.w 0x00017ba4
00017B9E: addq.w #0x2,D1w
00017BA0: bra.w 0x00017ba6
00017BA4: addq.w #0x1,D1w
00017BA6: move.w #-0x8000,D2w
00017BAA: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00017BB0: rts
00017BB2: lea (0xe4402).l,A1 ; data 000e4402
00017BB8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017BBE: rts
00017BC0: lea (0xfffbbe).l,A6 ; data 00fffbbe
00017BC6: clr.l D3
00017BC8: move.b (0x00fff0d4).l,D3b
00017BCE: move.b (0x1a,A6,D3w*0x1),D3b ; data 00fffbd8
00017BD2: bmi.w 0x00017c2a
00017BD6: mulu.w #0xc,D3
00017BDA: lea (0xe2f22).l,A1 ; data 000e2f22
00017BE0: adda.l D3,A1
00017BE2: movea.l #0xc086,A2
00017BE8: move.w #-0x8000,D1w
00017BEC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017BF2: clr.l D1
00017BF4: clr.l D3
00017BF6: move.b (0x00fff0d4).l,D3b
00017BFC: move.b (0x1a,A6,D3w*0x1),D1b ; data 00fffbd8
00017C00: lea (0xe2fb2).l,A5 ; data 000e2fb2
00017C06: move.b (0x0,A5,D1w*0x1),D1b ; data 000e2fb2
00017C0A: mulu.w #0x2,D1
00017C0E: adda.l D1,A2
00017C10: clr.l D1
00017C12: move.b (0x1a,A6,D3w*0x1),D3b ; data 00fffbd8
00017C16: move.b (0xe,A6,D3w*0x1),D1b ; data 00fffbcc
00017C1A: move.w #-0x8000,D2w
00017C1E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00017C24: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00017C28: rts
00017C2A: move.l #0xc086,D4
00017C30: moveq #0xd,D7
00017C32: moveq #0x1,D1
00017C34: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
00017C3A: rts
00017C3C: lea (0xfffb30).l,A4 ; data 00fffb30
00017C42: move.w (0x00fff102).l,D3w
00017C48: move.b (0x0,A4,D3w*0x1),D3b ; data 00fffb30
00017C4C: ext.w D3w
00017C4E: lsr.w #0x4,D3w
00017C50: rts
00017C52: clr.l D3
00017C54: clr.l D1
00017C56: lea (0xfff0e8).l,A5 ; data 00fff0e8
00017C5C: move.b (0x00fff0d4).l,D1b
00017C62: move.b (0x0,A5,D1w*0x1),D3b ; data 00fff0e8
00017C66: lea (0xe2fbe).l,A1 ; data 000e2fbe
00017C6C: mulu.w #0xe,D3
00017C70: adda.l D3,A1
00017C72: movea.l #0xc108,A2
00017C78: move.w #-0x8000,D1w
00017C7C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00017C82: rts
00017C84: bsr.w 0x00017c98 ; call 00017C98 FUN_00017c98
00017C88: move.b D1b,(0x0,A5,D4w*0x1)
00017C8C: rts
00017C8E: bsr.w 0x00017c98 ; call 00017C98 FUN_00017c98
00017C92: cmp.b (0x0,A5,D4w*0x1),D1b
00017C96: rts
00017C98: lea (0xfffbbe).l,A6 ; data 00fffbbe
00017C9E: clr.l D4
00017CA0: moveq #0x4,D7
00017CA2: cmp.b (0x1a,A6,D4w*0x1),D0b
00017CA6: beq.w 0x00017cb0
00017CAA: addq.w #0x1,D4w
00017CAC: dbf D7w,0x00017ca2
00017CB0: lea (0xfff0e8).l,A5 ; data 00fff0e8
00017CB6: rts
00017CC8: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00017CCC: subq.b #0x1,(0x00fff0de).l
00017CD2: bpl.w 0x00017d38
00017CD6: move.b #0x1,(0x00fff0de).l
00017CDE: addq.b #0x1,(0x00fff0df).l
00017CE4: cmpi.b #0x3,(0x00fff0df).l
00017CEC: ble.w 0x00017cf6
00017CF0: clr.b (0x00fff0df).l
00017CF6: lea (0x17cb8).l,A1 ; data 00017cb8
00017CFC: move.b (0x00fff0df).l,D4b
00017D02: ext.w D4w
00017D04: add.w D4w,D4w
00017D06: add.w D4w,D4w
00017D08: movea.l (0x0,A1,D4w*0x1),A1
00017D0C: move.l #0xe680,D4
00017D12: move.w #0x27,D7w
00017D16: move.w #0x8,D1w
00017D1A: move.w #0x6000,D2w
00017D1E: clr.w D3w
00017D20: move.w D7w,D5w
00017D22: addq.w #0x1,D5w
00017D24: jsr 0x00001728.l ; call 00001728 FUN_00001728
00017D2A: tst.b (0x00fff0df).l
00017D30: bne.w 0x00017d38
00017D34: bsr.w 0x00017d3e ; call 00017D3E FUN_00017d3e
00017D38: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00017D3C: rts
00017D3E: addq.b #0x1,(0x00ffe18d).l
00017D44: move.b (0x00ffe18d).l,D0b
00017D4A: ext.w D0w
00017D4C: andi.w #0x7,D0w
00017D50: add.w D0w,D0w
00017D52: lea (0x17d62).l,A0 ; data 00017d62
00017D58: move.w (0x0,A0,D0w*0x1),D0w ; data 00017d62
00017D5C: jmp 0x000530de.l ; call 000530DE FUN_000530de
00017D72: move.w #-0x7fe4,D5w
00017D76: move.l #0xeb84,D4
00017D7C: move.w #0x3,D7w
00017D80: move.w #0x3,D1w
00017D84: move.w #0x4000,D2w
00017D88: jsr 0x00003648.l ; call 00003648 FUN_00003648
00017D8E: rts
00017D90: movea.l (0x00fffc04).l,A0
00017D96: adda.l #0x4c,A0
00017D9C: add.b D1b,(A0)
00017D9E: bgt.w 0x00017da8
00017DA2: clr.b (A0)
00017DA4: bra.w 0x00017ddc
00017DA8: cmpi.b #0xa,(A0)
00017DAC: ble.w 0x00017ddc
00017DB0: move.b #0xa,(A0)
00017DB4: bra.w 0x00017ddc
00017DB8: lea (0xfffbbe).l,A0 ; data 00fffbbe
00017DBE: adda.l #0xc,A0
00017DC4: add.b D1b,(A0)
00017DC6: bgt.w 0x00017dd0
00017DCA: clr.b (A0)
00017DCC: bra.w 0x00017ddc
00017DD0: cmpi.b #0xa,(A0)
00017DD4: ble.w 0x00017ddc
00017DD8: move.b #0xa,(A0)
00017DDC: lea (0xd847e).l,A1 ; data 000d847e
00017DE2: clr.l D2
00017DE4: move.b (A0),D2b
00017DE6: lsl.w #0x3,D2w
00017DE8: adda.l D2,A1
00017DEA: move.l #0xeb8c,D4
00017DF0: moveq #0x0,D7
00017DF2: moveq #0x3,D1
00017DF4: move.w #0x6000,D2w
00017DF8: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017DFE: bsr.w 0x0001792e ; call 0001792E FUN_0001792e
00017E02: rts
00017E04: movem.l {  A3 D7},-(SP)
00017E08: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
00017E0C: ble.w 0x00017e72
00017E10: move.l #0xc12e,D4
00017E16: lea (0xd86b2).l,A1 ; data 000d86b2
00017E1C: move.w #0x4,D7w
00017E20: move.w #0x0,D1w
00017E24: move.w #-0x2000,D2w
00017E28: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017E2E: lea (0xff1100).l,A3 ; data 00ff1100
00017E34: move.w (0x00fff118).l,D0w
00017E3A: mulu.w #0x14,D0
00017E3E: lea (0xd85d0).l,A1 ; data 000d85d0
00017E44: clr.l D7
00017E46: move.b (0xf,A3,D0w*0x1),D7b ; data 00ff110f
00017E4A: mulu.w #0xe,D7
00017E4E: adda.l D7,A1
00017E50: move.l #0xc138,D4
00017E56: moveq #0x6,D7
00017E58: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017E5E: lea (0xd8454).l,A1 ; data 000d8454
00017E64: move.l #0xc146,D4
00017E6A: moveq #0x1,D7
00017E6C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017E72: movem.l (SP)+,{  D7 A3}
00017E76: rts
00017E78: lea (0xfffbbe).l,A5 ; data 00fffbbe
00017E7E: lea (0xff1100).l,A1 ; data 00ff1100
00017E84: move.w (0x00fff118).l,D1w
00017E8A: mulu.w #0x14,D1
00017E8E: clr.l D4
00017E90: move.b (0x10,A1,D1w*0x1),D4b ; data 00ff1110
00017E94: tst.b (0x00fff0a0).l
00017E9A: bmi.w 0x00017eaa
00017E9E: beq.w 0x00017ea8
00017EA2: addq.w #0x2,D4w
00017EA4: bra.w 0x00017eaa
00017EA8: addq.w #0x1,D4w
00017EAA: moveq #0x1b,D0
00017EAC: sub.w D4w,D0w
00017EAE: mulu.w #0x14,D0
00017EB2: rts
00017EB4: moveq #0x6,D0
00017EB6: clr.l D1
00017EB8: lea (0xfffbbe).l,A0 ; data 00fffbbe
00017EBE: move.b (0x7,A0),D1b
00017EC2: mulu.w #0x4,D1
00017EC6: add.w D1w,D0w
00017EC8: movea.l (0x00fffc04).l,A0
00017ECE: jsr 0x0000d81c.l ; call 0000D81C average_entity_delay_attributes
00017ED4: add.w D1w,D0w
00017ED6: move.w #0x1d,D1w
00017EDA: sub.w D0w,D1w
00017EDC: move.b D1b,(0x00fff0a7).l
00017EE2: rts
00017EE4: clr.b (0x00fff0e4).l
00017EEA: bra.w 0x00017f3c
00017EEE: clr.b (0x00fff0b8).l
00017EF4: cmpi.b #0xc,(0x00fff0e4).l
00017EFC: bne.w 0x00017f08
00017F00: move.b #-0x1,(0x00fff0b8).l
00017F08: subq.b #0x1,(0x00fff0e5).l
00017F0E: bpl.w 0x00017f28
00017F12: move.b (0x00fff0a7).l,(0x00fff0e5).l
00017F1C: cmpi.b #0xc,(0x00fff0e4).l
00017F24: bne.w 0x00017f2a
00017F28: rts
00017F2A: cmpi.b #0xc,(0x00fff0e4).l
00017F32: beq.w 0x00017f3c
00017F36: addq.b #0x1,(0x00fff0e4).l
00017F3C: move.w #-0x2000,D2w
00017F40: move.l #0xc186,D4
00017F46: lea (0xd86bc).l,A1 ; data 000d86bc
00017F4C: move.w #0x5,D7w
00017F50: move.w #0x0,D1w
00017F54: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017F5A: clr.l D4
00017F5C: move.b (0x00fff0e4).l,D4b
00017F62: mulu.w #0x10,D4
00017F66: lea (0xd8500).l,A1 ; data 000d8500
00017F6C: adda.l D4,A1
00017F6E: move.l #0xc192,D4
00017F74: moveq #0x7,D7
00017F76: moveq #0x0,D1
00017F78: jsr 0x00001232.l ; call 00001232 FUN_00001232
00017F7E: rts
00017F80: lea (0xe2eba).l,A0 ; data 000e2eba
00017F86: jsr 0x000566e2.l ; call 000566E2 FUN_000566e2
00017F8C: rts
00017F8E: clr.l D0
00017F90: movea.l (0x00fffc04).l,A1
00017F96: move.w (0x2c,A1),D0w
00017F9A: divu.w #0x2000,D0
00017F9E: subq.w #0x1,D0w
00017FA0: lsl.w #0x2,D0w
00017FA2: lea (0x18acb6).l,A0 ; data 0018acb6
00017FA8: movea.l (0x0,A0,D0w*0x1),A0 ; data 0018acb6
00017FAC: jsr 0x000566ec.l ; call 000566EC FUN_000566ec
00017FB2: rts
00017FB4: lea (0xd8454).l,A1 ; data 000d8454
00017FBA: move.l #0xc300,D4
00017FC0: add.w (0x00fff09a).l,D4w
00017FC6: move.w #0x7,D1w
00017FCA: move.w #0x2,D7w
00017FCE: move.w #0x6000,D2w
00017FD2: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00017FD8: rts
00017FDA: tst.w (0x00fff106).l
00017FE0: bmi.w 0x00017fee
00017FE4: bgt.w 0x00018038
00017FE8: bsr.b 0x00017fb4 ; call 00017FB4 FUN_00017fb4
00017FEA: bra.w 0x00018148
00017FEE: moveq #0x20,D1
00017FF0: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00017FF6: tst.w D0w
00017FF8: bne.w 0x0001814e
00017FFC: move.w #0x7,(0x00fff106).l
00018004: moveq #0x9,D1
00018006: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001800C: move.w D0w,(0x00fff09a).l
00018012: moveq #0x2,D1
00018014: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001801A: tst.w D0w
0001801C: bne.w 0x00018028
00018020: addi.w #0x1c,(0x00fff09a).l
00018028: move.w (0x00fff09a).l,D0w
0001802E: mulu.w #0x2,D0
00018032: move.w D0w,(0x00fff09a).l
00018038: bsr.w 0x00017fb4 ; call 00017FB4 FUN_00017fb4
0001803C: cmpi.w #0x4,(0x00fff106).l
00018044: bmi.w 0x0001805e
00018048: cmpi.w #0x6,(0x00fff106).l
00018050: bpl.w 0x0001805e
00018054: lea (0xd844c).l,A1 ; data 000d844c
0001805A: bra.w 0x00018064
0001805E: lea (0xd844a).l,A1 ; data 000d844a
00018064: move.l #0xc682,D4
0001806A: add.w (0x00fff09a).l,D4w
00018070: clr.w D1w
00018072: clr.w D7w
00018074: move.w #0x6000,D2w
00018078: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001807E: cmpi.w #0x2,(0x00fff106).l
00018086: beq.w 0x00018148
0001808A: cmpi.w #0x7,(0x00fff106).l
00018092: beq.w 0x00018148
00018096: lea (0xd845a).l,A1 ; data 000d845a
0001809C: clr.l D1
0001809E: moveq #0x4,D1
000180A0: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000180A6: move.w D0w,D1w
000180A8: mulu.w #0x6,D1
000180AC: adda.l D1,A1
000180AE: move.l #0xc502,D4
000180B4: add.w (0x00fff09a).l,D4w
000180BA: move.w #0x2,D7w
000180BE: clr.w D1w
000180C0: move.w #0x6000,D2w
000180C4: jsr 0x000011f6.l ; call 000011F6 FUN_000011f6
000180CA: cmpi.w #0x4,(0x00fff106).l
000180D2: bne.w 0x000180da
000180D6: bra.w 0x000180e6
000180DA: cmpi.w #0x5,(0x00fff106).l
000180E2: bne.w 0x00018148
000180E6: lea (0xd845a).l,A1 ; data 000d845a
000180EC: clr.l D1
000180EE: moveq #0x4,D1
000180F0: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000180F6: move.w D0w,D1w
000180F8: mulu.w #0x6,D1
000180FC: adda.l D1,A1
000180FE: move.l #0xc382,D4
00018104: add.w (0x00fff09a).l,D4w
0001810A: move.w #0x2,D7w
0001810E: clr.w D1w
00018110: move.w #0x6000,D2w
00018114: jsr 0x000011f6.l ; call 000011F6 FUN_000011f6
0001811A: cmpi.w #0x5,(0x00fff106).l
00018122: bne.w 0x00018148
00018126: lea (0xd844e).l,A1 ; data 000d844e
0001812C: move.l #0xc300,D4
00018132: add.w (0x00fff09a).l,D4w
00018138: clr.w D1w
0001813A: move.w #0x2,D7w
0001813E: move.w #0x6000,D2w
00018142: jsr 0x00001232.l ; call 00001232 FUN_00001232
00018148: subq.w #0x1,(0x00fff106).l
0001814E: rts
000181F8: move.w #-0x7ecc,(0x00c00004).l
00018200: move.w #-0x6f00,(0x00c00004).l
00018208: lea (0xe2e3a).l,A0 ; data 000e2e3a
0001820E: jsr 0x00001358.l ; call 00001358 copy_source_word_block
00018214: bset.b #0x0,(0x00ffef14).l
0001821C: bsr.w 0x000177cc ; call 000177CC FUN_000177cc
00018220: lea (0xfffbbe).l,A0 ; data 00fffbbe
00018226: move.b #0xa,(0xc,A0)
0001822C: move.b (0x00fff0b7).l,D0b
00018232: move.l D0,-(SP) ; data Stack[-0x4]
00018234: clr.b (0x00fff0b1).l
0001823A: move.l (0x00ffdd08).l,-(SP)
00018240: clr.l (0x00ffdd08).l
00018246: jsr 0x00000f1c.l ; call 00000F1C FUN_00000f1c
0001824C: jsr 0x00000f3c.l ; call 00000F3C FUN_00000f3c
00018252: move.w #-0x7ecc,(0x00c00004).l
0001825A: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
00018260: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00018266: lea (0xffbf40).l,A1 ; data 00ffbf40
0001826C: movea.l #0xd9376,A3
00018272: move.l #0x8160,D0
00018278: jsr 0x00001530.l ; call 00001530 FUN_00001530
0001827E: bsr.w 0x00017f80 ; call 00017F80 FUN_00017f80
00018282: lea (0xd9c96).l,A1 ; data 000d9c96
00018288: move.l #0xe000,D4
0001828E: move.w #0x27,D7w
00018292: move.w #0x15,D1w
00018296: move.w #0x2000,D2w
0001829A: clr.w D3w
0001829C: move.w D7w,D5w
0001829E: addq.w #0x1,D5w
000182A0: jsr 0x00001728.l ; call 00001728 FUN_00001728
000182A6: bsr.w 0x00017708 ; call 00017708 FUN_00017708
000182AA: lea (0xffd986).l,A3 ; data 00ffd986
000182B0: move.w #0x1,(0x00ffdd02).l
000182B8: move.w #0x2,D5w
000182BC: clr.l D1
000182BE: clr.l D2
000182C0: lea (0xe31e8).l,A2 ; data 000e31e8
000182C6: move.w #0x2000,D6w
000182CA: jsr 0x00001378.l ; call 00001378 FUN_00001378
000182D0: subq.w #0x8,A3
000182D2: clr.b (0x3,A3)
000182D6: move.w #0x1,(0x00ffd980).l
000182DE: bset.b #0x1,(0x00ffef14).l
000182E6: clr.w (0x00fff0d0).l
000182EC: bsr.w 0x00019504 ; call 00019504 FUN_00019504
000182F0: bsr.w 0x000187aa ; call 000187AA FUN_000187aa
000182F4: clr.b (0x00fff10a).l
000182FA: clr.b (0x00fff109).l
00018300: move.b #0x3,(0x00fff0d1).l
00018308: bsr.w 0x000189d8 ; call 000189D8 FUN_000189d8
0001830C: move.w #-0x7e8c,(0x00c00004).l
00018314: clr.b (0x00fff0d1).l
0001831A: bsr.w 0x0001882e ; call 0001882E FUN_0001882e
0001831E: move.b #-0x1,(0x00fff10c).l
00018326: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0001832C: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00018332: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00018338: bsr.w 0x000196f2 ; call 000196F2 FUN_000196f2
0001833C: btst.b #0x8,(0x00ffdd22).l
00018344: beq.w 0x00018394
00018348: move.w #-0x1,D0w
0001834C: bsr.w 0x000189d8 ; call 000189D8 FUN_000189d8
00018350: add.b D0b,(0x00fff0d1).l
00018356: bpl.w 0x00018366
0001835A: move.b #0x3,(0x00fff0d1).l
00018362: bra.w 0x0001837a
00018366: cmpi.b #0x4,(0x00fff0d1).l
0001836E: bmi.w 0x0001837a
00018372: move.b #0x0,(0x00fff0d1).l
0001837A: lea (0xffd986).l,A3 ; data 00ffd986
00018380: move.b (0x00fff0d1).l,D0b
00018386: mulu.w #0x28,D0
0001838A: addi.w #0x94,D0w
0001838E: move.w D0w,(A3)
00018390: bra.w 0x000183e0
00018394: btst.b #0x9,(0x00ffdd22).l
0001839C: beq.w 0x000183a6
000183A0: move.w #0x1,D0w
000183A4: bra.b 0x0001834c
000183A6: btst.b #0xa,(0x00ffdd22).l
000183AE: beq.w 0x00018408
000183B2: move.w #-0x1,D0w
000183B6: add.b D0b,(0x00fff0d0).l
000183BC: bpl.w 0x000183cc
000183C0: move.b #0x3,(0x00fff0d0).l
000183C8: bra.w 0x000183e0
000183CC: cmpi.b #0x4,(0x00fff0d0).l
000183D4: bmi.w 0x000183e0
000183D8: move.b #0x0,(0x00fff0d0).l
000183E0: lea (0xffd986).l,A3 ; data 00ffd986
000183E6: move.b (0x00fff0d0).l,D0b
000183EC: mulu.w #0x20,D0
000183F0: addi.w #0x8c,D0w
000183F4: move.w D0w,(0x6,A3)
000183F8: bset.b #0x1,(0x00ffef14).l
00018400: bsr.w 0x0001882e ; call 0001882E FUN_0001882e
00018404: bra.w 0x00018326
00018408: btst.b #0xb,(0x00ffdd22).l
00018410: beq.w 0x0001841a
00018414: move.b #0x1,D0b
00018418: bra.b 0x000183b6
0001841A: btst.b #0xe,(0x00ffdd22).l
00018422: beq.w 0x00018630
00018426: move.b #-0x1,(0x00fff0b7).l
0001842E: cmpi.b #0x3,(0x00fff0d1).l
00018436: bne.w 0x00018628
0001843A: cmpi.b #0x3,(0x00fff0d0).l
00018442: bne.w 0x000185e6
00018446: tst.b (0x00fff0b0).l
0001844C: beq.w 0x00018326
00018450: bsr.w 0x0001897c ; call 0001897C FUN_0001897c
00018454: move.w #0x0,D1w
00018458: bsr.w 0x00019b40 ; call 00019B40 FUN_00019b40
0001845C: cmpi.w #0x8,D0w
00018460: bne.w 0x00018480
00018464: tst.b (0x00ffdf84).l
0001846A: beq.w 0x00018480
0001846E: lea (0xe43b0).l,A1 ; data 000e43b0
00018474: movea.l #0xc228,A2
0001847A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018480: lea (0xe438a).l,A1 ; data 000e438a
00018486: movea.l #0xc426,A2
0001848C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018492: lea (0xe387e).l,A1 ; data 000e387e
00018498: movea.l #0xc638,A2
0001849E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000184A4: clr.w D2w
000184A6: move.b #0x1,(0x00fff0dc).l
000184AE: bra.w 0x000184d8
000184B2: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000184B8: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000184BE: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000184C4: bsr.w 0x000196f2 ; call 000196F2 FUN_000196f2
000184C8: btst.b #0x8,(0x00ffdd22).l
000184D0: beq.w 0x000184ee
000184D4: move.w #-0x1,D2w
000184D8: move.w #0x1,D3w
000184DC: move.l #0xc632,D7
000184E2: move.w #0x0,D4w
000184E6: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
000184EC: bra.b 0x000184b2
000184EE: btst.b #0x9,(0x00ffdd22).l
000184F6: beq.w 0x000184fe
000184FA: moveq #0x1,D2
000184FC: bra.b 0x000184d8
000184FE: btst.b #0xe,(0x00ffdd22).l
00018506: bne.w 0x00018520
0001850A: btst.b #0xc,(0x00ffdd22).l
00018512: bne.w 0x00018520
00018516: btst.b #0xd,(0x00ffdd22).l
0001851E: beq.b 0x000184b2
00018520: tst.b (0x00fff0dc).l
00018526: bne.w 0x0001831a
0001852A: bsr.w 0x00019b40 ; call 00019B40 FUN_00019b40
0001852E: cmpi.w #0x8,D0w
00018532: bne.w 0x000185ce
00018536: tst.b (0x00ffdf84).l
0001853C: beq.w 0x000185ce
00018540: clr.l D1
00018542: lea (0xff1100).l,A1 ; data 00ff1100
00018548: move.w (0x00fff118).l,D1w
0001854E: mulu.w #0x14,D1
00018552: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
00018558: move.w D6w,D4w
0001855A: clr.l D6
0001855C: movea.l (0x00fffc04).l,A1
00018562: move.b (0x7c,A1),D6b
00018566: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
0001856C: tst.b (0x00fff0dd).l
00018572: bne.w 0x0001859c
00018576: lsr.w #0x1,D4w
00018578: move.w D4w,D1w
0001857A: neg.w D1w
0001857C: bsr.w 0x00017d90 ; call 00017D90 FUN_00017d90
00018580: move.w #0x18,D0w
00018584: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001858A: movea.l (0x00fffc04).l,A1
00018590: tst.b (0x4c,A1)
00018594: beq.w 0x000185ce
00018598: bra.w 0x0001831a
0001859C: move.w #0x5,D4w
000185A0: clr.l D6
000185A2: move.b (0x77,A1),D6b
000185A6: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000185AC: tst.b (0x00fff0dd).l
000185B2: bne.w 0x000185ce
000185B6: subq.b #0x3,(0x9c,A1)
000185BA: bgt.w 0x000185c4
000185BE: move.b #0x1,(0x9c,A1)
000185C4: move.w #0x18,D0w
000185C8: jsr 0x000530de.l ; call 000530DE FUN_000530de
000185CE: move.b #-0x1,(0x00fff0dc).l
000185D6: clr.l D7
000185D8: bsr.w 0x00018776 ; call 00018776 FUN_00018776
000185DC: jsr 0x00026a00.l ; call 00026A00 FUN_00026a00
000185E2: bra.w 0x00018694
000185E6: cmpi.b #0x2,(0x00fff0d0).l
000185EE: bne.w 0x000185fc
000185F2: tst.b (0x00fff0b0).l
000185F8: beq.w 0x00018326
000185FC: clr.l D1
000185FE: move.b (0x00fff0d0).l,D1b
00018604: lsl.w #0x2,D1w
00018606: lea (0xe2dfe).l,A6 ; data 000e2dfe
0001860C: movea.l (0x0,A6,D1w*0x1),A6
00018610: jsr (A6) ; call 00018AA6 FUN_00018aa6
00018612: tst.b (0x00fff0ad).l
00018618: beq.w 0x00018326
0001861C: clr.b (0x00fff0ad).l
00018622: clr.l D7
00018624: bra.w 0x00018670
00018628: bsr.w 0x00019238 ; call 00019238 FUN_00019238
0001862C: bra.w 0x00018326
00018630: btst.b #0xc,(0x00ffdd22).l
00018638: beq.w 0x00018648
0001863C: move.b #0x0,(0x00fff0b7).l
00018644: bra.w 0x0001842e
00018648: btst.b #0xd,(0x00ffdd22).l
00018650: beq.w 0x00018660
00018654: move.b #0x1,(0x00fff0b7).l
0001865C: bra.w 0x0001842e
00018660: btst.b #0xf,(0x00ffdd22).l
00018668: beq.w 0x00018326
0001866C: move.w #-0x1,D7w
00018670: move.w #-0x7ecc,(0x00c00004).l
00018678: move.l D7,-(SP) ; data Stack[-0xc]
0001867A: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00018680: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
00018686: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0001868C: move.l (SP)+,D7
0001868E: clr.b (0x00fff10c).l
00018694: move.l (SP)+,(0x00ffdd08).l
0001869A: jsr 0x00000f1c.l ; call 00000F1C FUN_00000f1c
000186A0: jsr 0x00000f3c.l ; call 00000F3C FUN_00000f3c
000186A6: bsr.w 0x000176ea ; call 000176EA FUN_000176ea
000186AA: lea (0xfff0f2).l,A4 ; data 00fff0f2
000186B0: lea (0xfffbbe).l,A1 ; data 00fffbbe
000186B6: clr.l D0
000186B8: clr.b (0x00fff108).l
000186BE: move.w #0x4,D6w
000186C2: move.b (0x00fff108).l,D0b
000186C8: lsl.w #0x1,D0w
000186CA: tst.w (0x0,A4,D0w*0x1)
000186CE: bpl.w 0x000186e0
000186D2: addq.b #0x1,(0x00fff108).l
000186D8: dbf D6w,0x000186c2
000186DC: bra.w 0x000186e8
000186E0: move.w (0x0,A4,D0w*0x1),(0x00ffdd48).l
000186E8: move.l (SP)+,D0
000186EA: move.b D0b,(0x00fff0b7).l
000186F0: move.w #-0x7e8c,(0x00c00004).l
000186F8: rts
000186FA: bsr.w 0x00017708 ; call 00017708 FUN_00017708
000186FE: clr.b (0x00fff0d4).l
00018704: lea (0xfffbbe).l,A6 ; data 00fffbbe
0001870A: clr.w D5w
0001870C: bsr.w 0x000194ac ; call 000194AC FUN_000194ac
00018710: lea (0xffbf40).l,A1 ; data 00ffbf40
00018716: movea.l #0xd646a,A3
0001871C: move.l #0x8160,D0
00018722: jsr 0x00001530.l ; call 00001530 FUN_00001530
00018728: jsr 0x00026958.l ; call 00026958 FUN_00026958
0001872E: lea (0xd806c).l,A1 ; data 000d806c
00018734: move.l #0xe000,D4
0001873A: move.w #0x27,D7w
0001873E: move.w #0xc,D1w
00018742: move.w #0x6000,D2w
00018746: clr.w D3w
00018748: move.w D7w,D5w
0001874A: addq.w #0x1,D5w
0001874C: jsr 0x00001728.l ; call 00001728 FUN_00001728
00018752: lea (0xd74f4).l,A1 ; data 000d74f4
00018758: move.l #0xe680,D4
0001875E: move.w #0x27,D7w
00018762: move.w #0x8,D1w
00018766: move.w #0x6000,D2w
0001876A: clr.w D3w
0001876C: move.w D7w,D5w
0001876E: addq.w #0x1,D5w
00018770: jmp 0x00001728.l ; call 00001728 FUN_00001728
00018776: move.w #0x4,D6w
0001877A: move.w D6w,-(SP) ; data Stack[-0x2]
0001877C: move.w #0x1,D5w
00018780: move.w D5w,-(SP) ; data Stack[-0x4]
00018782: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00018788: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001878E: move.w (SP)+,D5w
00018790: dbf D5w,0x00018780
00018794: jsr 0x00001088.l ; call 00001088 step_palette_toward_black
0001879A: bset.b #0x0,(0x00ffef14).l
000187A2: move.w (SP)+,D6w
000187A4: dbf D6w,0x0001877a
000187A8: rts
000187AA: lea (0xd92b6).l,A1 ; data 000d92b6
000187B0: move.l #0xe184,D4
000187B6: move.w #0x2,D7w
000187BA: move.w #0x1,D1w
000187BE: move.w #0x6000,D2w
000187C2: lea (0xfffbbe).l,A5 ; data 00fffbbe
000187C8: clr.l D5
000187CA: clr.l D0
000187CC: moveq #0xb,D6
000187CE: tst.b (0xe,A5,D5w*0x1)
000187D2: beq.w 0x000187dc
000187D6: jsr 0x00001232.l ; call 00001232 FUN_00001232
000187DC: addq.w #0x8,D4w
000187DE: adda.l #0xc,A1
000187E4: addq.w #0x1,D0w
000187E6: clr.l D3
000187E8: move.w D0w,D3w
000187EA: divu.w #0x4,D3
000187EE: swap D3
000187F0: tst.w D3w
000187F2: bne.w 0x000187fa
000187F6: addi.w #0x260,D4w
000187FA: addq.w #0x1,D5w
000187FC: dbf D6w,0x000187ce
00018800: moveq #0x3,D6
00018802: jsr 0x00001232.l ; call 00001232 FUN_00001232
00018808: addq.w #0x8,D4w
0001880A: adda.l #0xc,A1
00018810: addq.w #0x1,D0w
00018812: clr.l D3
00018814: move.w D0w,D3w
00018816: divu.w #0x4,D3
0001881A: swap D3
0001881C: tst.w D3w
0001881E: bne.w 0x00018826
00018822: addi.w #0x260,D4w
00018826: addq.w #0x1,D5w
00018828: dbf D6w,0x00018802
0001882C: rts
0001882E: bsr.w 0x0001897c ; call 0001897C FUN_0001897c
00018832: bsr.w 0x00018a92 ; call 00018A92 FUN_00018a92
00018836: cmpi.w #0xb,D7w
0001883A: bgt.w 0x00018864
0001883E: lea (0xfffbbe).l,A1 ; data 00fffbbe
00018844: tst.b (0xe,A1,D7w*0x1) ; data 00fffbcc
00018848: bgt.w 0x00018864
0001884C: lea (0xe39b7).l,A1 ; data 000e39b7
00018852: move.w #0x0,D1w
00018856: movea.l #0xc42c,A2
0001885C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018862: rts
00018864: move.l D7,D6
00018866: mulu.w #0xc,D7
0001886A: lea (0xd92b6).l,A1 ; data 000d92b6
00018870: move.l #0xc12e,D4
00018876: adda.l D7,A1
00018878: moveq #0x2,D7
0001887A: moveq #0x1,D1
0001887C: move.w #0x6000,D2w
00018880: jsr 0x00001232.l ; call 00001232 FUN_00001232
00018886: move.l D6,D7
00018888: mulu.w #0xc,D7
0001888C: lea (0xd9ee8).l,A1 ; data 000d9ee8
00018892: move.l #0xc1b6,D4
00018898: adda.l D7,A1
0001889A: moveq #0x5,D7
0001889C: moveq #0x0,D1
0001889E: move.w #0x2000,D2w
000188A2: jsr 0x00001232.l ; call 00001232 FUN_00001232
000188A8: cmpi.w #0xc,D6w
000188AC: bpl.w 0x000188d0
000188B0: bsr.w 0x0001a16a ; call 0001A16A FUN_0001a16a
000188B4: clr.l D1
000188B6: lea (0xfffbbe).l,A1 ; data 00fffbbe
000188BC: move.b (0xe,A1,D6w*0x1),D1b ; data 00fffbcc
000188C0: movea.l #0xc1c2,A2
000188C6: move.w #0x0,D2w
000188CA: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000188D0: lea (0xe31f2).l,A1 ; data 000e31f2
000188D6: move.l D6,D7
000188D8: mulu.w #0x4,D7
000188DC: adda.l D7,A1
000188DE: movea.l (A1),A1 ; data 000e31f2
000188E0: lea (0xffef96).l,A5 ; data 00ffef96
000188E6: clr.w D1w
000188E8: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
000188EE: move.b #-0x1,(A5)
000188F2: move.w #0x12,D1w
000188F6: move.w #0x0,D2w
000188FA: cmpi.w #0xc,D6w
000188FE: bpl.w 0x00018926
00018902: movea.l #0xc628,A2
00018908: jsr 0x0000c160.l ; call 0000C160 FUN_0000c160
0001890E: lea (0xe38c6).l,A1 ; data 000e38c6
00018914: lea (0xffef96).l,A5 ; data 00ffef96
0001891A: clr.w D1w
0001891C: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
00018922: move.b #-0x1,(A5)
00018926: movea.l #0xc2a6,A2
0001892C: move.w #0x14,D1w
00018930: jsr 0x0000c160.l ; call 0000C160 FUN_0000c160
00018936: cmpi.w #0xc,D6w
0001893A: bpl.w 0x0001897a
0001893E: bsr.w 0x000195a6 ; call 000195A6 FUN_000195a6
00018942: movea.l #0xc3c0,A2
00018948: move.w #0x0,D2w
0001894C: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018952: lea (0xfffbbe).l,A1 ; data 00fffbbe
00018958: bsr.w 0x0001952c ; call 0001952C FUN_0001952c
0001895C: movea.l #0xc2c4,A2
00018962: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018968: bsr.w 0x0001957e ; call 0001957E FUN_0001957e
0001896C: move.l D6,D1
0001896E: movea.l #0xc344,A2
00018974: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0001897A: rts
0001897C: lea (0xd8454).l,A1 ; data 000d8454
00018982: move.l #0xc126,D4
00018988: moveq #0x12,D7
0001898A: moveq #0x11,D1
0001898C: move.w #0x6000,D2w
00018990: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00018996: rts
00018998: lea (0xd8454).l,A1 ; data 000d8454
0001899E: move.l #0xc628,D4
000189A4: moveq #0x12,D7
000189A6: moveq #0x8,D1
000189A8: move.w #0x6000,D2w
000189AC: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000189B2: rts
000189B4: movem.l {  D7 D4},-(SP)
000189B8: lea (0xd8454).l,A1 ; data 000d8454
000189BE: move.l #0xc4a6,D4
000189C4: moveq #0x12,D7
000189C6: moveq #0xa,D1
000189C8: move.w #0x6000,D2w
000189CC: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000189D2: movem.l (SP)+,{  D4 D7}
000189D6: rts
000189D8: clr.l D5
000189DA: move.b (0x00fff0d1).l,D5b
000189E0: move.l D5,-(SP) ; data Stack[-0x4]
000189E2: mulu.w #0x24,D5
000189E6: lea (0xd9fa8).l,A1 ; data 000d9fa8
000189EC: adda.l D5,A1
000189EE: move.l (SP),D5 ; data Stack[-0x4]
000189F0: mulu.w #0x280,D5
000189F4: move.l #0xe08a,D4
000189FA: add.l D5,D4
000189FC: move.w #0x8,D7w
00018A00: move.w #0x0,D1w
00018A04: move.w #0x2000,D2w
00018A08: jsr 0x00001232.l ; call 00001232 FUN_00001232
00018A0E: move.l (SP)+,D5
00018A10: btst.b #0x8,(0x00ffdd22).l
00018A18: beq.w 0x00018a2a
00018A1C: subq.w #0x1,D5w
00018A1E: bpl.w 0x00018a36
00018A22: move.w #0x3,D5w
00018A26: bra.w 0x00018a36
00018A2A: addq.w #0x1,D5w
00018A2C: cmpi.w #0x3,D5w
00018A30: ble.w 0x00018a36
00018A34: clr.l D5
00018A36: move.l D5,-(SP) ; data Stack[-0x4]
00018A38: mulu.w #0x24,D5
00018A3C: lea (0xd9fa8).l,A1 ; data 000d9fa8
00018A42: adda.l D5,A1
00018A44: adda.l #0x12,A1
00018A4A: move.l (SP)+,D5
00018A4C: mulu.w #0x280,D5
00018A50: move.l #0xe08a,D4
00018A56: add.l D5,D4
00018A58: jsr 0x00001232.l ; call 00001232 FUN_00001232
00018A5E: rts
00018A92: clr.l D7
00018A94: move.b (0x00fff0d1).l,D7b
00018A9A: mulu.w #0x4,D7
00018A9E: add.b (0x00fff0d0).l,D7b
00018AA4: rts
00018AA6: clr.b (0x00fff0dc).l
00018AAC: bsr.w 0x0001897c ; call 0001897C FUN_0001897c
00018AB0: lea (0xe3886).l,A1 ; data 000e3886
00018AB6: move.w #0x0,D1w
00018ABA: movea.l #0xc32c,A2
00018AC0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018AC6: clr.l D2
00018AC8: bra.w 0x00018b04
00018ACC: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00018AD2: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00018AD8: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00018ADE: bsr.w 0x000196f2 ; call 000196F2 FUN_000196f2
00018AE2: btst.b #0x8,(0x00ffdd22).l
00018AEA: beq.w 0x00018af4
00018AEE: moveq #-0x1,D2
00018AF0: bra.w 0x00018b04
00018AF4: btst.b #0x9,(0x00ffdd22).l
00018AFC: beq.w 0x00018b18
00018B00: move.w #0x1,D2w
00018B04: move.w #0x1,D3w
00018B08: move.w #-0x3b54,D7w
00018B0C: move.w #0x0,D4w
00018B10: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
00018B16: bra.b 0x00018acc
00018B18: btst.b #0xe,(0x00ffdd22).l
00018B20: bne.w 0x00018b3c
00018B24: btst.b #0xc,(0x00ffdd22).l
00018B2C: bne.w 0x00018b3c
00018B30: btst.b #0xd,(0x00ffdd22).l
00018B38: beq.w 0x00018b4a
00018B3C: tst.b (0x00fff0dc).l
00018B42: beq.w 0x00018b5c
00018B46: bra.w 0x00018c28
00018B4A: btst.b #0xf,(0x00ffdd22).l
00018B52: beq.w 0x00018acc
00018B56: bsr.w 0x0001882e ; call 0001882E FUN_0001882e
00018B5A: rts
00018B5C: bsr.w 0x0001897c ; call 0001897C FUN_0001897c
00018B60: lea (0xe3902).l,A1 ; data 000e3902
00018B66: move.w #0x0,D1w
00018B6A: movea.l #0xc2a8,A2
00018B70: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018B76: lea (0xfffbbe).l,A0 ; data 00fffbbe
00018B7C: clr.l D0
00018B7E: move.b (0xd,A0),D0b
00018B82: bpl.w 0x00018b90
00018B86: lea (0x1ab100).l,A1 ; data 001ab100
00018B8C: bra.w 0x00018b9c
00018B90: lea (0x1ab118).l,A1 ; data 001ab118
00018B96: mulu.w #0x18,D0
00018B9A: adda.l D0,A1
00018B9C: movea.l #0xc1ae,A2
00018BA2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018BA8: movea.l #0xc346,A2
00018BAE: move.w #0x0,D2w
00018BB2: move.b (A0)+,D1b
00018BB4: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018BBA: adda.l #0x80,A2
00018BC0: move.b (A0)+,D1b
00018BC2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018BC8: adda.l #0x100,A2
00018BCE: move.w (A0)+,D1w
00018BD0: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018BD6: adda.l #0x80,A2
00018BDC: move.w (A0)+,D1w
00018BDE: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018BE4: adda.l #0x80,A2
00018BEA: clr.l D1
00018BEC: move.b (A0)+,D1b
00018BEE: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018BF4: adda.l #0x100,A2
00018BFA: move.b (A0)+,D1b
00018BFC: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018C02: adda.l #0x100,A2
00018C08: movea.l #0xc7c6,A2
00018C0E: move.w #0x3,D7w
00018C12: clr.l D1
00018C14: move.b (A0)+,D1b
00018C16: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00018C1C: adda.l #0x80,A2
00018C22: dbf D7w,0x00018c12
00018C26: rts
00018C28: bsr.w 0x0001897c ; call 0001897C FUN_0001897c
00018C2C: lea (0xe4404).l,A1 ; data 000e4404
00018C32: move.w #0x0,D1w
00018C36: movea.l #0xc1ac,A2
00018C3C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018C42: tst.b (0x00fff0e3).l
00018C48: bmi.w 0x00018c78
00018C4C: bgt.w 0x00018c64
00018C50: lea (0xe443f).l,A1 ; data 000e443f
00018C56: movea.l #0xc42c,A2
00018C5C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018C62: rts
00018C64: lea (0xe445d).l,A1 ; data 000e445d
00018C6A: movea.l #0xc42e,A2
00018C70: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018C76: rts
00018C78: lea (0xe441f).l,A1 ; data 000e441f
00018C7E: movea.l #0xc328,A2
00018C84: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018C8A: clr.l D0
00018C8C: tst.b (0x00fffbf2).l
00018C92: bpl.w 0x00018cac
00018C96: lea (0xe4480).l,A1 ; data 000e4480
00018C9C: movea.l #0xc334,A2
00018CA2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018CA8: bra.w 0x00018cce
00018CAC: lea (0xe4476).l,A1 ; data 000e4476
00018CB2: movea.l #0xc334,A2
00018CB8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018CBE: tst.b (0x00fffbf6).l
00018CC4: bmi.w 0x00018cd0
00018CC8: beq.w 0x00018cd2
00018CCC: addq.w #0x1,D0w
00018CCE: addq.w #0x1,D0w
00018CD0: addq.w #0x1,D0w
00018CD2: lsl.w #0x2,D0w
00018CD4: lea (0xe44c8).l,A1 ; data 000e44c8
00018CDA: movea.l (0x0,A1,D0w*0x1),A1 ; data 000e44c8
00018CDE: movea.l #0xc630,A2
00018CE4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018CEA: clr.l D0
00018CEC: move.b (0x00fffbf2).l,D0b
00018CF2: bpl.w 0x00018d00
00018CF6: lea (0xe4484).l,A1 ; data 000e4484
00018CFC: bra.w 0x00018d0c
00018D00: lea (0xe4060).l,A1 ; data 000e4060
00018D06: mulu.w #0x10,D0
00018D0A: adda.l D0,A1
00018D0C: movea.l #0xc4ac,A2
00018D12: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00018D18: rts
00018FF2: movea.w (0x323c,A4),A5 ; data 0000323c
00018FF6: ori.b #0x7c,D0b
00018FFA: ori.b #0x2c,D0b
00018FFE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019004: rts
00019116: jsr 0x000532bc.l ; call 000532BC FUN_000532bc
0001911C: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00019122: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00019128: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0001912E: btst.b #0xf,(0x00ffdd22).l
00019136: beq.b 0x0001911c
00019138: move.w #-0x7ecc,(0x00c00004).l
00019140: lea (0xe2e3a).l,A0 ; data 000e2e3a
00019146: jsr 0x00001358.l ; call 00001358 copy_source_word_block
0001914C: bset.b #0x0,(0x00ffef14).l
00019154: lea (0xffbf40).l,A1 ; data 00ffbf40
0001915A: movea.l #0xd86e0,A3
00019160: move.l #0x17e0,D0
00019166: jsr 0x00001530.l ; call 00001530 FUN_00001530
0001916C: bsr.w 0x00017708 ; call 00017708 FUN_00017708
00019170: move.w #-0x7e8c,(0x00c00004).l
00019178: move.l (SP)+,D0
0001917A: jmp 0x00018252.l
00019180: tst.w (0x00fff11a).l
00019186: bpl.w 0x000191c0
0001918A: lea (0xd8454).l,A1 ; data 000d8454
00019190: move.l #0xc2a6,D4
00019196: moveq #0x12,D7
00019198: moveq #0xe,D1
0001919A: move.w #0x6000,D2w
0001919E: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000191A4: bsr.w 0x0001897c ; call 0001897C FUN_0001897c
000191A8: lea (0xe4011).l,A1 ; data 000e4011
000191AE: move.w #0x0,D1w
000191B2: movea.l #0xc2a6,A2
000191B8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000191BE: rts
000191C0: jsr 0x000532bc.l ; call 000532BC FUN_000532bc
000191C6: lea (0xff1100).l,A1 ; data 00ff1100
000191CC: jsr 0x00053a24.l ; call 00053A24 FUN_00053a24
000191D2: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000191D8: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000191DE: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000191E4: btst.b #0xe,(0x00ffdd22).l
000191EC: bne.w 0x00019208
000191F0: btst.b #0xc,(0x00ffdd22).l
000191F8: bne.w 0x00019208
000191FC: btst.b #0xd,(0x00ffdd22).l
00019204: beq.w 0x00019228
00019208: move.w (0x00fff118).l,D0w
0001920E: move.w (0x00fff11a).l,(0x00fff118).l
00019218: move.w D0w,(0x00fff11a).l
0001921E: move.b #-0x1,(0x00fff0ad).l
00019226: rts
00019228: btst.b #0xf,(0x00ffdd22).l
00019230: beq.b 0x000191d2
00019232: jmp 0x00019138.l
00019238: bsr.w 0x00018a92 ; call 00018A92 FUN_00018a92
0001923C: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019242: tst.b (0xe,A1,D7w*0x1) ; data 00fffbcc
00019246: beq.w 0x00019430
0001924A: clr.w (0x00ffdd50).l
00019250: clr.b (0x00fff0dc).l
00019256: bsr.w 0x00018998 ; call 00018998 FUN_00018998
0001925A: bsr.w 0x00018a92 ; call 00018A92 FUN_00018a92
0001925E: move.l D7,D0
00019260: lea (0xfffbbe).l,A6 ; data 00fffbbe
00019266: moveq #0x4,D7
00019268: cmp.b (0x1a,A6),D0b
0001926C: beq.w 0x0001927a
00019270: addq.l #0x1,A6
00019272: dbf D7w,0x00019268
00019276: bra.w 0x000192d4
0001927A: move.w #0x4,D0w
0001927E: sub.w D7w,D0w
00019280: move.b D0b,(0x00fff0d4).l
00019286: lea (0xe3859).l,A1 ; data 000e3859
0001928C: movea.l #0xc628,A2
00019292: move.w #0x0,D1w
00019296: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001929C: lea (0xe3823).l,A1 ; data 000e3823
000192A2: movea.l #0xc72e,A2
000192A8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000192AE: lea (0xe382e).l,A1 ; data 000e382e
000192B4: movea.l #0xc7aa,A2
000192BA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000192C0: move.w #-0x1,(0x00ffdd50).l
000192C8: move.b #0x1,(0x00fff0dc).l
000192D0: bra.w 0x00019368
000192D4: clr.l D0
000192D6: lea (0xfffbbe).l,A6 ; data 00fffbbe
000192DC: moveq #0x4,D7
000192DE: move.b (0x1a,A6),D0b
000192E2: bmi.w 0x00019316
000192E6: addq.l #0x1,A6
000192E8: dbf D7w,0x000192de
000192EC: lea (0xe383d).l,A1 ; data 000e383d
000192F2: movea.l #0xc628,A2
000192F8: move.w #0x0,D1w
000192FC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019302: lea (0xe384a).l,A1 ; data 000e384a
00019308: movea.l #0xc6a8,A2
0001930E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019314: rts
00019316: move.w #0x4,D0w
0001931A: sub.w D7w,D0w
0001931C: move.b D0b,(0x00fff0d4).l
00019322: bsr.w 0x0001957e ; call 0001957E FUN_0001957e
00019326: bsr.w 0x00018a92 ; call 00018A92 FUN_00018a92
0001932A: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019330: bsr.w 0x0001952c ; call 0001952C FUN_0001952c
00019334: cmp.w D1w,D6w
00019336: bpl.w 0x00019410
0001933A: lea (0xe383d).l,A1 ; data 000e383d
00019340: movea.l #0xc628,A2
00019346: move.w #0x0,D1w
0001934A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019350: lea (0xe3851).l,A1 ; data 000e3851
00019356: movea.l #0xc6a8,A2
0001935C: move.w #0x0,D1w
00019360: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019366: rts
00019368: clr.w D5w
0001936A: bsr.w 0x000194ac ; call 000194AC FUN_000194ac
0001936E: lea (0xe387e).l,A1 ; data 000e387e
00019374: movea.l #0xc8ba,A2
0001937A: move.w #0x0,D1w
0001937E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019384: clr.w D2w
00019386: bra.w 0x000193ae
0001938A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00019390: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00019396: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0001939C: bsr.w 0x000196f2 ; call 000196F2 FUN_000196f2
000193A0: btst.b #0x8,(0x00ffdd22).l
000193A8: beq.w 0x000193c0
000193AC: moveq #-0x1,D2
000193AE: moveq #0x1,D3
000193B0: move.w #-0x374e,D7w
000193B4: move.w #0x0,D4w
000193B8: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
000193BE: bra.b 0x0001938a
000193C0: btst.b #0x9,(0x00ffdd22).l
000193C8: beq.w 0x000193d0
000193CC: moveq #0x1,D2
000193CE: bra.b 0x000193ae
000193D0: btst.b #0xe,(0x00ffdd22).l
000193D8: bne.w 0x000193f4
000193DC: btst.b #0xc,(0x00ffdd22).l
000193E4: bne.w 0x000193f4
000193E8: btst.b #0xd,(0x00ffdd22).l
000193F0: beq.w 0x0001941c
000193F4: tst.b (0x00fff0dc).l
000193FA: bne.w 0x00019428
000193FE: tst.w (0x00ffdd50).l
00019404: beq.w 0x00019410
00019408: bsr.w 0x0001962a ; call 0001962A FUN_0001962a
0001940C: bra.w 0x00019428
00019410: bsr.w 0x00019736 ; call 00019736 FUN_00019736
00019414: bsr.w 0x000195d8 ; call 000195D8 FUN_000195d8
00019418: bra.w 0x00019428
0001941C: btst.b #0xf,(0x00ffdd22).l
00019424: beq.w 0x0001938a
00019428: bsr.w 0x00019504 ; call 00019504 FUN_00019504
0001942C: bsr.w 0x0001882e ; call 0001882E FUN_0001882e
00019430: rts
00019432: btst.b #0xa,(0x00ffdd22).l
0001943A: beq.w 0x00019444
0001943E: moveq #-0x1,D5
00019440: bra.w 0x00019452
00019444: btst.b #0xb,(0x00ffdd22).l
0001944C: beq.w 0x000194aa
00019450: moveq #0x1,D5
00019452: lea (0xfffbbe).l,A6 ; data 00fffbbe
00019458: clr.l D0
0001945A: move.b (0x00fff0d4).l,D0b
00019460: cmpi.b #0x3,(0x1a,A6,D0w*0x1) ; data 00fffbd8
00019466: beq.w 0x000194a2
0001946A: cmpi.b #0x5,(0x1a,A6,D0w*0x1) ; data 00fffbd8
00019470: beq.w 0x000194a2
00019474: cmpi.b #0x6,(0x1a,A6,D0w*0x1) ; data 00fffbd8
0001947A: beq.w 0x000194a2
0001947E: cmpi.b #0x7,(0x1a,A6,D0w*0x1) ; data 00fffbd8
00019484: beq.w 0x000194a2
00019488: lea (0xfff0e8).l,A5 ; data 00fff0e8
0001948E: cmpi.b #0x6,(0x0,A5,D0w*0x1) ; data 00fff0e8
00019494: bmi.w 0x000194a2
00019498: move.b (0x1a,A6,D0w*0x1),D0b ; data 00fffbd8
0001949C: moveq #0x1,D1
0001949E: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
000194A2: bsr.w 0x000194ac ; call 000194AC FUN_000194ac
000194A6: bsr.w 0x00017bc0 ; call 00017BC0 FUN_00017bc0
000194AA: rts
000194AC: bsr.w 0x00019504 ; call 00019504 FUN_00019504
000194B0: add.b D5b,(0x00fff0d4).l
000194B6: bpl.w 0x000194c6
000194BA: move.b #0x4,(0x00fff0d4).l
000194C2: bra.w 0x000194d8
000194C6: cmpi.b #0x5,(0x00fff0d4).l
000194CE: bmi.w 0x000194d8
000194D2: clr.b (0x00fff0d4).l
000194D8: clr.l D4
000194DA: move.b (0x00fff0d4).l,D4b
000194E0: mulu.w #0xc,D4
000194E4: addi.w #-0x136c,D4w
000194E8: lea (0xd8472).l,A1 ; data 000d8472
000194EE: move.w #0x2,D7w
000194F2: clr.w D1w
000194F4: move.w #0x6000,D2w
000194F8: jsr 0x00001232.l ; call 00001232 FUN_00001232
000194FE: bsr.w 0x0001792e ; call 0001792E FUN_0001792e
00019502: rts
00019504: clr.l D4
00019506: move.b (0x00fff0d4).l,D4b
0001950C: mulu.w #0xc,D4
00019510: addi.w #-0x136c,D4w
00019514: lea (0xd8478).l,A1 ; data 000d8478
0001951A: move.w #0x2,D7w
0001951E: clr.w D1w
00019520: move.w #0x6000,D2w
00019524: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001952A: rts
0001952C: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019532: clr.l D1
00019534: move.b (0xe,A1,D7w*0x1),D1b ; data 00fffbcc
00019538: mulu.w D1w,D1
0001953A: clr.l D0
0001953C: lea (0xe2df2).l,A4 ; data 000e2df2
00019542: move.b (0x0,A4,D7w*0x1),D0b ; data 000e2df2
00019546: mulu.w D0w,D1
00019548: rts
0001954A: move.l D7,D0
0001954C: mulu.w #0x3,D0
00019550: clr.l D1
00019552: move.b (0x20,A1,D0w*0x1),D1b
00019556: rts
00019558: lea (0xfffbbe).l,A1 ; data 00fffbbe
0001955E: clr.l D5
00019560: moveq #0x4,D7
00019562: bsr.b 0x0001954a ; call 0001954A FUN_0001954a
00019564: add.w D1w,D5w
00019566: dbf D7w,0x00019562
0001956A: moveq #0xb,D7
0001956C: bsr.b 0x0001952c ; call 0001952C FUN_0001952c
0001956E: add.w D1w,D5w
00019570: dbf D7w,0x0001956c
00019574: clr.l D6
00019576: move.w (0x4,A1),D6w
0001957A: sub.w D5w,D6w
0001957C: rts
0001957E: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019584: clr.l D5
00019586: clr.l D7
00019588: moveq #0x4,D4
0001958A: move.w D4w,D7w
0001958C: move.b (0x1a,A1,D7w*0x1),D7b
00019590: bmi.w 0x00019598
00019594: bsr.b 0x0001952c ; call 0001952C FUN_0001952c
00019596: add.w D1w,D5w
00019598: dbf D4w,0x0001958a
0001959C: clr.l D6
0001959E: move.w (0x2,A1),D6w
000195A2: sub.w D5w,D6w
000195A4: rts
000195A6: bsr.w 0x00018a92 ; call 00018A92 FUN_00018a92
000195AA: lea (0xfffbbe).l,A1 ; data 00fffbbe
000195B0: bsr.w 0x0001952c ; call 0001952C FUN_0001952c
000195B4: move.w D1w,D0w
000195B6: move.b (0x6,A1),D1b
000195BA: divu.w D1w,D0
000195BC: move.l D0,D2
000195BE: swap D2
000195C0: tst.w D2w
000195C2: beq.w 0x000195c8
000195C6: addq.w #0x1,D0w
000195C8: cmpi.w #0x7,D0w
000195CC: bmi.w 0x000195d2
000195D0: moveq #0x6,D0
000195D2: clr.l D1
000195D4: move.w D0w,D1w
000195D6: rts
000195D8: bsr.b 0x000195a6 ; call 000195A6 FUN_000195a6
000195DA: lea (0xfff0f2).l,A4 ; data 00fff0f2
000195E0: lea (0xfff0e8).l,A5 ; data 00fff0e8
000195E6: clr.l D4
000195E8: move.b (0x00fff0d4).l,D4b
000195EE: move.b D7b,(0x1a,A1,D4w*0x1)
000195F2: clr.b (0x0,A5,D4w*0x1) ; data 00fff0e8
000195F6: mulu.w #0xa,D1
000195FA: lsl.w #0x1,D4w
000195FC: move.w D1w,(0x0,A4,D4w*0x1) ; data 00fff0f2
00019600: mulu.w #0xc,D7
00019604: lea (0xd92b6).l,A1 ; data 000d92b6
0001960A: adda.l D7,A1
0001960C: lsr.w #0x1,D4w
0001960E: mulu.w #0xc,D4
00019612: addi.w #-0x146c,D4w
00019616: moveq #0x2,D7
00019618: moveq #0x1,D1
0001961A: move.w #0x6000,D2w
0001961E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00019624: bsr.w 0x0001792e ; call 0001792E FUN_0001792e
00019628: rts
0001962A: clr.w D4w
0001962C: move.b (0x00fff0d4).l,D4b
00019632: lea (0xfff0ed).l,A4 ; data 00fff0ed
00019638: lea (0xfffbbe).l,A1 ; data 00fffbbe
0001963E: cmpi.b #0x3,(0x1a,A1,D4w*0x1) ; data 00fffbd8
00019644: bne.w 0x0001965a
00019648: bclr.b #0x5,(0x00ff1592).l
00019650: clr.b (0x00fff0d9).l
00019656: bra.w 0x0001969c
0001965A: cmpi.b #0x5,(0x1a,A1,D4w*0x1) ; data 00fffbd8
00019660: bne.w 0x0001966e
00019664: clr.b (0x00fff0d2).l
0001966A: bra.w 0x0001969c
0001966E: cmpi.b #0x6,(0x1a,A1,D4w*0x1) ; data 00fffbd8
00019674: bne.w 0x00019684
00019678: bclr.b #0x2,(0x00ff1592).l
00019680: bra.w 0x0001969c
00019684: cmpi.b #0x7,(0x1a,A1,D4w*0x1) ; data 00fffbd8
0001968A: bne.w 0x0001969c
0001968E: bclr.b #0x1,(0x00ff1592).l
00019696: clr.b (0x00ffe169).l
0001969C: move.b #-0x1,(0x1a,A1,D4w*0x1) ; data 00fffbd8
000196A2: move.b #0x6,(0x0,A4,D4w*0x1) ; data 00fff0ed
000196A8: lea (0xd92aa).l,A1 ; data 000d92aa
000196AE: moveq #0x1,D1
000196B0: moveq #0x2,D7
000196B2: move.w #0x6000,D2w
000196B6: mulu.w #0xc,D4
000196BA: addi.w #-0x146c,D4w
000196BE: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
000196C4: lea (0xd84d6).l,A1 ; data 000d84d6
000196CA: moveq #0x6,D4
000196CC: mulu.w #0x6,D4
000196D0: adda.l D4,A1
000196D2: clr.w D4w
000196D4: move.b (0x00fff0d4).l,D4b
000196DA: mulu.w #0xc,D4
000196DE: addi.l #0xed14,D4
000196E4: clr.w D1w
000196E6: jsr 0x00001232.l ; call 00001232 FUN_00001232
000196EC: bsr.w 0x0001792e ; call 0001792E FUN_0001792e
000196F0: rts
000196F2: subq.b #0x1,(0x00fff10a).l
000196F8: bmi.w 0x00019700
000196FC: bra.w 0x00019734
00019700: move.b #0x4,(0x00fff10a).l
00019708: lea (0xffd87e).l,A0 ; data 00ffd87e
0001970E: adda.l #0x30,A0
00019714: lea (0xffd87e).l,A1 ; data 00ffd87e
0001971A: adda.l #0x32,A1
00019720: moveq #0x2,D0
00019722: move.w (A0),D1w
00019724: move.w (A1)+,(A0)+
00019726: dbf D0w,0x00019724
0001972A: move.w D1w,(A0)
0001972C: bset.b #0x0,(0x00ffef14).l
00019734: rts
00019736: lea (0xfff0ed).l,A4 ; data 00fff0ed
0001973C: lea (0xfffbbe).l,A6 ; data 00fffbbe
00019742: clr.w D4w
00019744: move.b (0x00fff0d4).l,D4b
0001974A: tst.b (0x00fff10c).l
00019750: bmi.w 0x00019762
00019754: cmpi.b #0x6,(0x0,A4,D4w*0x1) ; data 00fff0ed
0001975A: beq.w 0x00019762
0001975E: clr.w D4w
00019760: rts
00019762: clr.b (0x0,A4,D4w*0x1)
00019766: lea (0xd84d6).l,A1 ; data 000d84d6
0001976C: mulu.w #0xc,D4
00019770: addi.l #0xed14,D4
00019776: moveq #0x2,D7
00019778: moveq #0x0,D1
0001977A: move.w #0x6000,D2w
0001977E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00019784: moveq #-0x1,D4
00019786: rts
00019788: subq.b #0x1,(0x00fff0ac).l
0001978E: bpl.w 0x0001979e
00019792: move.b #0x28,(0x00fff0ac).l
0001979A: bsr.w 0x0001792e ; call 0001792E FUN_0001792e
0001979E: bsr.w 0x00017eee ; call 00017EEE FUN_00017eee
000197A2: tst.b (0x00fff0b8).l
000197A8: beq.w 0x00019854
000197AC: lea (0xfff0ed).l,A4 ; data 00fff0ed
000197B2: lea (0xfffbbe).l,A0 ; data 00fffbbe
000197B8: subq.w #0x1,(0x00ffdd48).l
000197BE: bpl.w 0x00019854
000197C2: moveq #0x2,D7
000197C4: moveq #0x0,D1
000197C6: move.w #0x6000,D2w
000197CA: clr.b (0x00fff108).l
000197D0: move.w #0x4,D6w
000197D4: cmpi.b #0x6,(A4)
000197D8: beq.w 0x00019846
000197DC: addq.b #0x1,(A4)
000197DE: lea (0xd84d6).l,A1 ; data 000d84d6
000197E4: clr.w D4w
000197E6: move.b (A4),D4b
000197E8: mulu.w #0x6,D4
000197EC: adda.l D4,A1
000197EE: clr.l D4
000197F0: move.b (0x00fff108).l,D4b
000197F6: mulu.w #0xc,D4
000197FA: addi.l #0xed14,D4
00019800: jsr 0x00001232.l ; call 00001232 FUN_00001232
00019806: clr.w D4w
00019808: move.b (0x00fff108).l,D4b
0001980E: lsl.w #0x1,D4w
00019810: lea (0xfff0f2).l,A5 ; data 00fff0f2
00019816: move.w (0x0,A5,D4w*0x1),(0x00ffdd48).l
0001981E: cmpi.b #0x6,(A4)
00019822: bne.w 0x00019854
00019826: lea (0xfffbbe).l,A0 ; data 00fffbbe
0001982C: clr.l D0
0001982E: move.b (0x00fff108).l,D0b
00019834: move.b (0x1a,A0,D0w*0x1),D0b
00019838: moveq #0x1,D1
0001983A: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
0001983E: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019842: bra.w 0x00019854
00019846: addq.l #0x1,A0
00019848: addq.l #0x1,A4
0001984A: addq.b #0x1,(0x00fff108).l
00019850: dbf D6w,0x000197d4
00019854: rts
00019856: clr.w D1w
00019858: bsr.w 0x00017db8 ; call 00017DB8 FUN_00017db8
0001985C: lea (0xfff0ed).l,A4 ; data 00fff0ed
00019862: moveq #0x2,D7
00019864: moveq #0x0,D1
00019866: move.w #0x6000,D2w
0001986A: move.w #0x4,D6w
0001986E: lea (0xd84d6).l,A1 ; data 000d84d6
00019874: clr.l D4
00019876: move.b (A4),D4b
00019878: mulu.w #0x6,D4
0001987C: adda.l D4,A1
0001987E: moveq #0x4,D4
00019880: sub.w D6w,D4w
00019882: mulu.w #0xc,D4
00019886: addi.l #0xed14,D4
0001988C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00019892: addq.l #0x1,A4
00019894: dbf D6w,0x0001986e
00019898: rts
0001989A: add.b D1b,(0x00fff0a0).l
000198A0: beq.w 0x00019904
000198A4: cmpi.b #0x1,(0x00fff0a0).l
000198AC: beq.w 0x00019904
000198B0: bgt.w 0x000198c0
000198B4: move.b #-0x1,(0x00fff0a0).l
000198BC: bra.w 0x00019904
000198C0: move.b #0x1,(0x00fff0a0).l
000198C8: tst.b (0x00fff0a6).l
000198CE: beq.w 0x00019904
000198D2: jsr 0x00018776.l ; call 00018776 FUN_00018776
000198D8: jsr 0x00026a00.l ; call 00026A00 FUN_00026a00
000198DE: move.w #-0x8000,D1w
000198E2: movea.l #0xc498,A2
000198E8: lea (0x166a75).l,A1 ; data 00166a75
000198EE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000198F4: move.w #0x1e,D5w
000198F8: jsr 0x00000c24.l ; call 00000C24 FUN_00000c24
000198FE: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00019904: tst.b (0x00fff0a6).l
0001990A: bmi.w 0x00019974
0001990E: clr.l D1
00019910: move.b (0x00fff0a0).l,D1b
00019916: addq.b #0x1,D1b
00019918: mulu.w #0xe,D1
0001991C: lea (0xe3074).l,A1 ; data 000e3074
00019922: adda.l D1,A1
00019924: movea.l #0xc208,A2
0001992A: move.w #-0x8000,D1w
0001992E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00019934: move.b (0x00ffe189).l,D0b
0001993A: cmp.b (0x00fff0a0).l,D0b
00019940: beq.w 0x00019974
00019944: move.b (0x00fff0a0).l,(0x00ffe189).l
0001994E: move.w #0x54,D0w
00019952: tst.b (0x00fff0a0).l
00019958: bmi.w 0x0001996e
0001995C: move.w #0x53,D0w
00019960: tst.b (0x00fff0a0).l
00019966: beq.w 0x0001996e
0001996A: move.w #0x52,D0w
0001996E: jsr 0x000530de.l ; call 000530DE FUN_000530de
00019974: rts
00019976: lea (0xfff0ed).l,A4 ; data 00fff0ed
0001997C: lea (0xfffbbe).l,A6 ; data 00fffbbe
00019982: clr.w D0w
00019984: clr.w D4w
00019986: move.b (0x00fff0d4).l,D4b
0001998C: move.b (0x1a,A6,D4w*0x1),D0b ; data 00fffbd8
00019990: cmpi.b #-0x1,D0b
00019994: beq.w 0x00019a8a
00019998: cmpi.b #0x6,(0x0,A4,D4w*0x1) ; data 00fff0ed
0001999E: bmi.w 0x00019a8a
000199A2: moveq #0x2,D1
000199A4: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
000199A8: beq.w 0x00019a8a
000199AC: moveq #0x3,D1
000199AE: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
000199B2: beq.w 0x00019a8a
000199B6: moveq #0x4,D1
000199B8: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
000199BC: beq.w 0x00019a8a
000199C0: moveq #0x5,D1
000199C2: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
000199C6: beq.w 0x00019a8a
000199CA: cmpi.b #0xc,(0x00fff0e4).l
000199D2: bne.w 0x00019a8a
000199D6: btst.b #0xe,(0x00ffdd22).l
000199DE: beq.w 0x000199ee
000199E2: move.b #-0x1,(0x00fff0b7).l
000199EA: bra.w 0x00019a0e
000199EE: btst.b #0xc,(0x00ffdd22).l
000199F6: beq.w 0x00019a06
000199FA: move.b #0x0,(0x00fff0b7).l
00019A02: bra.w 0x00019a0e
00019A06: move.b #0x1,(0x00fff0b7).l
00019A0E: move.l D0,-(SP) ; data Stack[-0x4]
00019A10: lsl.w #0x2,D0w
00019A12: lea (0xe2e0a).l,A4 ; data 000e2e0a
00019A18: movea.l (0x0,A4,D0w*0x1),A4 ; data 000e2e0a
00019A1C: jsr (A4)
00019A1E: bsr.w 0x00017ee4 ; call 00017EE4 FUN_00017ee4
00019A22: move.l (SP)+,D0
00019A24: tst.b (0x00fff0b6).l
00019A2A: beq.w 0x00019a38
00019A2E: clr.b (0x00fff0b6).l
00019A34: bra.w 0x00019a92
00019A38: moveq #0x7,D1
00019A3A: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
00019A3E: beq.w 0x00019a92
00019A42: moveq #0xc,D1
00019A44: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
00019A48: beq.w 0x00019a8a
00019A4C: moveq #0xb,D1
00019A4E: bsr.w 0x00017c8e ; call 00017C8E FUN_00017c8e
00019A52: beq.w 0x00019a8a
00019A56: cmpi.w #0x3,D0w
00019A5A: bmi.w 0x00019a7a
00019A5E: cmpi.w #0x8,D0w
00019A62: bmi.w 0x00019a74
00019A66: cmpi.w #0xb,D0w
00019A6A: beq.w 0x00019a80
00019A6E: moveq #0x4,D1
00019A70: bra.w 0x00019a82
00019A74: moveq #0x5,D1
00019A76: bra.w 0x00019a82
00019A7A: moveq #0x3,D1
00019A7C: bra.w 0x00019a82
00019A80: moveq #0x2,D1
00019A82: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019A86: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019A8A: jsr 0x0001792e.l ; call 0001792E FUN_0001792e
00019A90: rts
00019A92: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019A98: move.w (0x00fff118).l,D1w
00019A9E: mulu.w #0x14,D1
00019AA2: lea (0xff1100).l,A6 ; data 00ff1100
00019AA8: clr.l D7
00019AAA: move.b (0x13,A6,D1w*0x1),D7b ; data 00ff1113
00019AAE: cmpi.b #0x11,D7b
00019AB2: bmi.b 0x00019a8a
00019AB4: lsr.w #0x1,D7w
00019AB6: bcc.w 0x00019ac2
00019ABA: move.w #0x4,D1w
00019ABE: bra.w 0x00019aca
00019AC2: move.w #0x5,D1w
00019AC6: clr.b (0xe,A1,D0w*0x1) ; data 00fffbcc
00019ACA: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP) ; data 00ff1100
00019ACE: move.b #0x1,(0x00fff0a0).l
00019AD6: clr.l D1
00019AD8: bsr.w 0x0001989a ; call 0001989A FUN_0001989a
00019ADC: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00019AE0: clr.l D2
00019AE2: move.b (0x00fff0d4).l,D2b
00019AE8: jmp 0x0001b1c4.l ; call 0001B1C4 FUN_0001b1c4
00019AEE: lea (0xe31dd).l,A5 ; data 000e31dd
00019AF4: move.w (0x00fff118).l,D1w
00019AFA: mulu.w #0x14,D1
00019AFE: lea (0xff1100).l,A6 ; data 00ff1100
00019B04: clr.l D0
00019B06: move.b (0x13,A6,D1w*0x1),D0b ; data 00ff1113
00019B0A: subi.w #0x11,D0w
00019B0E: move.b (0x0,A5,D0w*0x1),(0x13,A6,D1w*0x1) ; data 000e31dd
00019B14: lea (0xc1320).l,A1 ; data 000c1320
00019B1A: move.l #0xc22e,D4
00019B20: move.w #-0x8000,D2w
00019B24: move.w #0x0,D1w
00019B28: move.w #0xd,D7w
00019B2C: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00019B32: rts
00019B34: moveq #0x7,D1
00019B36: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019B3A: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019B3E: rts
00019B40: lea (0xe31a7).l,A5 ; data 000e31a7
00019B46: move.w (0x00fff118).l,D2w
00019B4C: mulu.w #0x14,D2
00019B50: lea (0xff1100).l,A6 ; data 00ff1100
00019B56: clr.l D0
00019B58: move.b (0xf,A6,D2w*0x1),D0b ; data 00ff110f
00019B5C: ble.w 0x00019b7e
00019B60: move.b (0x13,A6,D2w*0x1),D0b ; data 00ff1113
00019B64: bmi.w 0x00019b7e
00019B68: tst.b (0x00ffdf84).l
00019B6E: bmi.w 0x00019b82
00019B72: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
00019B76: bgt.w 0x00019b82
00019B7A: moveq #-0x1,D0
00019B7C: rts
00019B7E: moveq #0x45,D0
00019B80: rts
00019B82: tst.b (0x00fff0a0).l
00019B88: ble.w 0x00019ba4
00019B8C: cmpi.b #0x7,D0b
00019B90: bmi.w 0x00019ba4
00019B94: cmpi.b #0x11,D0b
00019B98: bpl.w 0x00019ba4
00019B9C: lsl.w #0x1,D0w
00019B9E: addq.w #0x1,D0w
00019BA0: bra.w 0x00019ba6
00019BA4: lsl.w #0x1,D0w
00019BA6: adda.l D0,A5
00019BA8: clr.l D0
00019BAA: move.b (A5),D0b ; data 000e31a7
00019BAC: rts
00019BAE: lea (0xe31a7).l,A5 ; data 000e31a7
00019BB4: move.w (0x00fff118).l,D1w
00019BBA: mulu.w #0x14,D1
00019BBE: lea (0xff1100).l,A6 ; data 00ff1100
00019BC4: clr.l D0
00019BC6: move.b (0x13,A6,D1w*0x1),D0b ; data 00ff1113
00019BCA: tst.b (0x00fff0a0).l
00019BD0: ble.w 0x00019bec
00019BD4: cmpi.b #0x7,D0b
00019BD8: bmi.w 0x00019bec
00019BDC: cmpi.b #0x11,D0b
00019BE0: bpl.w 0x00019bec
00019BE4: lsl.w #0x1,D0w
00019BE6: addq.w #0x1,D0w
00019BE8: bra.w 0x00019bee
00019BEC: lsl.w #0x1,D0w
00019BEE: adda.l D0,A5
00019BF0: clr.l D0
00019BF2: move.b (A5),D0b ; data 000e31a7
00019BF4: rts
00019BF6: move.l D0,-(SP) ; data Stack[-0x4]
00019BF8: clr.l D0
00019BFA: clr.l D6
00019BFC: move.b (0x13,A1,D1w*0x1),D0b
00019C00: tst.b (0x00fff0a0).l
00019C06: ble.w 0x00019c36
00019C0A: cmpi.b #0x7,D0b
00019C0E: bmi.w 0x00019c36
00019C12: cmpi.b #0x11,D0b
00019C16: bpl.w 0x00019c36
00019C1A: addq.w #0x1,D1w
00019C1C: move.b (0x11,A1,D1w*0x1),D6b
00019C20: tst.b (0x00fff0a0).l
00019C26: bmi.w 0x00019c50
00019C2A: beq.w 0x00019c34
00019C2E: addq.w #0x2,D6w
00019C30: bra.w 0x00019c36
00019C34: addq.w #0x1,D6w
00019C36: move.b (0x11,A1,D1w*0x1),D6b
00019C3A: tst.b (0x00fff0a0).l
00019C40: bmi.w 0x00019c50
00019C44: beq.w 0x00019c4e
00019C48: addq.w #0x2,D6w
00019C4A: bra.w 0x00019c50
00019C4E: addq.w #0x1,D6w
00019C50: move.l (SP)+,D0
00019C52: rts
00019C54: clr.l D7
00019C56: move.b (0x00fff0d4).l,D7b
00019C5C: lea (0xfffbbe).l,A0 ; data 00fffbbe
00019C62: move.b (0x1a,A0,D7w*0x1),D7b ; data 00fffbd8
00019C66: move.b (0xe,A0,D7w*0x1),D7b ; data 00fffbcc
00019C6A: rts
00019CB2: tst.b (0x00fff0d3).l
00019CB8: bgt.w 0x00019cc8
00019CBC: move.w #0x4,D0w
00019CC0: bsr.w 0x00019b34 ; call 00019B34 FUN_00019b34
00019CC4: bra.w 0x00019d20
00019CC8: bsr.w 0x00019bae ; call 00019BAE FUN_00019bae
00019CCC: clr.l D1
00019CCE: move.b (0x00fff0d3).l,D1b
00019CD4: cmpi.b #0x8,D0b
00019CD8: bne.w 0x00019d10
00019CDC: tst.b (0x00ffdf84).l
00019CE2: beq.w 0x00019d10
00019CE6: lea (0xfffbbe).l,A0 ; data 00fffbbe
00019CEC: add.b D1b,(0xc,A0)
00019CF0: bgt.w 0x00019cfc
00019CF4: clr.b (0xc,A0)
00019CF8: bra.w 0x00019d14
00019CFC: cmpi.b #0xa,(0xc,A0)
00019D02: ble.w 0x00019d14
00019D06: move.b #0xa,(0xc,A0)
00019D0C: bra.w 0x00019d14
00019D10: bsr.w 0x00017db8 ; call 00017DB8 FUN_00017db8
00019D14: moveq #0x4,D0
00019D16: moveq #0x6,D1
00019D18: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019D1C: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019D20: moveq #0x4,D7
00019D22: bra.w 0x00019f80
00019D66: moveq #0xb,D0
00019D68: lea (0xfffb30).l,A5 ; data 00fffb30
00019D6E: move.w (0x00fff102).l,D3w
00019D74: tst.b (0x0,A5,D3w*0x1) ; data 00fffb30
00019D78: bgt.w 0x00019d94
00019D7C: clr.b (0x0,A5,D3w*0x1) ; data 00fffb30
00019D80: moveq #0xa,D1
00019D82: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019D86: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019D8A: bsr.w 0x00017e04 ; call 00017E04 FUN_00017e04
00019D8E: jmp 0x0001b706.l ; call 0001B706 FUN_0001b706
00019D94: moveq #0x6,D1
00019D96: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019D9A: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019D9E: bsr.w 0x00019dac ; call 00019DAC FUN_00019dac
00019DA2: bsr.w 0x00017998 ; call 00017998 FUN_00017998
00019DA6: bsr.w 0x00017a5e ; call 00017A5E FUN_00017a5e
00019DAA: rts
00019DAC: move.w (0x00fff102).l,D3w
00019DB2: lea (0xfffb30).l,A0 ; data 00fffb30
00019DB8: lea (0xff1100).l,A1 ; data 00ff1100
00019DBE: clr.l D0
00019DC0: clr.l D1
00019DC2: move.w (0x00fff118).l,D1w
00019DC8: mulu.w #0x14,D1
00019DCC: move.b (0x5,A1,D1w*0x1),D0b ; data 00ff1105
00019DD0: or.b D0b,(0x0,A0,D3w*0x1) ; data 00fffb30
00019DD4: mulu.w #0x20,D0
00019DD8: lea (0xe2d52).l,A0 ; data 000e2d52
00019DDE: adda.l D0,A0
00019DE0: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
00019DE6: rts
00019DE8: bsr.w 0x00019df2 ; call 00019DF2 FUN_00019df2
00019DEC: bsr.w 0x0001789c ; call 0001789C FUN_0001789c
00019DF0: rts
00019DF2: clr.l D0
00019DF4: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
00019DF8: ble.w 0x00019e16
00019DFC: clr.l D1
00019DFE: lea (0xff1100).l,A5 ; data 00ff1100
00019E04: move.w (0x00fff118).l,D1w
00019E0A: mulu.w #0x14,D1
00019E0E: move.b (0x5,A5,D1w*0x1),D0b ; data 00ff1105
00019E12: mulu.w #0x20,D0
00019E16: bsr.b 0x00019dd8 ; call 00019DD8 FUN_00019dd8
00019E18: rts
00019E32: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00019E36: moveq #0x0,D0
00019E38: clr.l D1
00019E3A: tst.b (0x00fff0d5).l
00019E40: bpl.w 0x00019e4a
00019E44: moveq #0x7,D1
00019E46: bra.w 0x00019e56
00019E4A: bgt.w 0x00019e54
00019E4E: moveq #0x8,D1
00019E50: bra.w 0x00019e56
00019E54: moveq #0x9,D1
00019E56: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019E5A: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019E5E: move.b (0x00fff0d5).l,D1b
00019E64: ble.w 0x00019e78
00019E68: jsr 0x0001b706.l ; call 0001B706 FUN_0001b706
00019E6E: jsr 0x00056434.l ; call 00056434 FUN_00056434
00019E74: bsr.w 0x00017e04 ; call 00017E04 FUN_00017e04
00019E78: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00019E7C: jsr 0x0001b706.l ; call 0001B706 FUN_0001b706
00019E82: rts
00019EA6: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00019EAA: moveq #0x1,D0
00019EAC: clr.l D1
00019EAE: tst.b (0x00fff0d6).l
00019EB4: bpl.w 0x00019ebe
00019EB8: moveq #0x7,D1
00019EBA: bra.w 0x00019eca
00019EBE: bgt.w 0x00019ec8
00019EC2: moveq #0x8,D1
00019EC4: bra.w 0x00019eca
00019EC8: moveq #0x9,D1
00019ECA: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019ECE: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019ED2: move.b (0x00fff0d6).l,D1b
00019ED8: ble.w 0x00019ef4
00019EDC: clr.l D7
00019EDE: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019EE4: move.b (0xf,A1),D7b
00019EE8: lsr.w #0x1,D7w
00019EEA: lsl.w #0x1,D1w
00019EEC: add.w D7w,D1w
00019EEE: jsr 0x00056482.l ; call 00056482 FUN_00056482
00019EF4: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00019EF8: jsr 0x0001b706.l ; call 0001B706 FUN_0001b706
00019EFE: rts
00019F18: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00019F1C: moveq #0x2,D0
00019F1E: clr.l D1
00019F20: tst.b (0x00fff0d7).l
00019F26: bpl.w 0x00019f32
00019F2A: move.w #0x7,D1w
00019F2E: bra.w 0x00019f42
00019F32: bgt.w 0x00019f3e
00019F36: move.w #0x8,D1w
00019F3A: bra.w 0x00019f42
00019F3E: move.w #0x9,D1w
00019F42: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
00019F46: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
00019F4A: clr.l D1
00019F4C: move.b (0x00fff0d7).l,D1b
00019F52: ble.w 0x00019f60
00019F56: jsr 0x000564d6.l ; call 000564D6 FUN_000564d6
00019F5C: bsr.w 0x00017998 ; call 00017998 FUN_00017998
00019F60: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00019F64: jsr 0x0001b706.l ; call 0001B706 FUN_0001b706
00019F6A: rts
00019F72: moveq #0x5,D7
00019F74: bra.w 0x00019f80
00019F78: moveq #0x6,D7
00019F7A: bra.w 0x00019f80
00019F7E: moveq #0x7,D7
00019F80: lea (0xfff0ed).l,A4 ; data 00fff0ed
00019F86: lea (0xfffbbe).l,A1 ; data 00fffbbe
00019F8C: move.w #0x4,D6w
00019F90: cmp.b (0x1a,A1,D6w*0x1),D7b
00019F94: beq.w 0x00019f9e
00019F98: dbf D6w,0x00019f90
00019F9C: rts
00019F9E: bsr.w 0x000195aa ; call 000195AA FUN_000195aa
00019FA2: lea (0xfff0f2).l,A4 ; data 00fff0f2
00019FA8: lea (0xfff0e8).l,A5 ; data 00fff0e8
00019FAE: clr.b (0x0,A5,D6w*0x1)
00019FB2: mulu.w #0xa,D1
00019FB6: move.w D6w,D5w
00019FB8: lsl.w #0x1,D5w
00019FBA: move.w D1w,(0x0,A4,D5w*0x1)
00019FBE: bsr.w 0x00017bc0 ; call 00017BC0 FUN_00017bc0
00019FC2: lea (0xfff0ed).l,A4 ; data 00fff0ed
00019FC8: move.w D6w,D4w
00019FCA: bsr.w 0x00019762 ; call 00019762 FUN_00019762
00019FCE: rts
0001A01E: move.w #0xa,D1w
0001A022: move.w #0x8,D0w
0001A026: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
0001A02A: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
0001A02E: rts
0001A030: tst.b (0x00ffdf84).l
0001A036: bne.w 0x0001a056
0001A03A: bsr.w 0x00019bae ; call 00019BAE FUN_00019bae
0001A03E: beq.w 0x0001a078
0001A042: cmpi.b #0x1,D0b
0001A078: jsr 0x00055fd8.l ; call 00055FD8 FUN_00055fd8
0001A09C: tst.b (0x00fff0aa).l
0001A0A2: bgt.w 0x0001a0ae
0001A0A6: move.w #0xa,D1w
0001A0AA: bra.w 0x0001a0b6
0001A0AE: bsr.w 0x0001a154 ; call 0001A154 FUN_0001a154
0001A0B2: move.w #0x6,D1w
0001A0B6: move.w #0x9,D0w
0001A0BA: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
0001A0BE: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
0001A0C2: rts
0001A100: ori.b #0x45,D4b
0001A104: rts
0001A106: tst.b (0x00fff0aa).l
0001A10C: bgt.w 0x0001a11c
0001A110: jsr 0x0001b706.l ; call 0001B706 FUN_0001b706
0001A116: moveq #0xa,D1
0001A118: bra.w 0x0001a122
0001A11C: bsr.w 0x0001a154 ; call 0001A154 FUN_0001a154
0001A120: moveq #0x6,D1
0001A122: moveq #0xa,D0
0001A124: bsr.w 0x00017c84 ; call 00017C84 FUN_00017c84
0001A128: bsr.w 0x00017c52 ; call 00017C52 FUN_00017c52
0001A12C: rts
0001A12E: tst.b (0x00fff0d2).l
0001A134: beq.w 0x0001a152
0001A138: add.b (0x00fff0d2).l,D1b
0001A13E: ext.w D1w
0001A140: subq.b #0x1,(0x00fff0d2).l
0001A146: bgt.w 0x0001a152
0001A14A: move.w D1w,-(SP) ; data Stack[-0x2]
0001A14C: bsr.w 0x00019f72 ; call 00019F72 FUN_00019f72
0001A150: move.w (SP)+,D1w
0001A152: rts
0001A154: lea (0xff1100).l,A0 ; data 00ff1100
0001A15A: move.w (0x00fff118).l,D0w
0001A160: mulu.w #0x14,D0
0001A164: clr.b (0xf,A0,D0w*0x1) ; data 00ff110f
0001A168: rts
0001A16A: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0001A16E: lea (0xd9ed4).l,A1 ; data 000d9ed4
0001A174: move.l #0xc4a6,D4
0001A17A: move.w #0x4,D7w
0001A17E: move.w #0x0,D1w
0001A182: move.w #0x2000,D2w
0001A186: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001A18C: lea (0xd9ede).l,A1 ; data 000d9ede
0001A192: move.l #0xc526,D4
0001A198: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001A19E: tst.b (0x00fff0b0).l
0001A1A4: bmi.w 0x0001a1c2
0001A1A8: lea (0xe4370).l,A1 ; data 000e4370
0001A1AE: move.w #0x0,D1w
0001A1B2: movea.l #0xc4b4,A2
0001A1B8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001A1BE: bra.w 0x0001a2c6
0001A1C2: bsr.w 0x00018a92 ; call 00018A92 FUN_00018a92
0001A1C6: cmpi.w #0x4,D7w
0001A1CA: bmi.w 0x0001a20e
0001A1CE: beq.w 0x0001a1fc
0001A1D2: cmpi.w #0x8,D7w
0001A1D6: bpl.w 0x0001a20e
0001A1DA: lea (0xe4380).l,A1 ; data 000e4380
0001A1E0: move.w #0x0,D1w
0001A1E4: movea.l #0xc4b2,A2
0001A1EA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0001A1F0: move.w #0x0,D1w
0001A1F4: move.w #0x2000,D2w
0001A1F8: bra.w 0x0001a28e
0001A1FC: lea (0xfffbbe).l,A0 ; data 00fffbbe
0001A202: clr.l D6
0001A204: move.b (0xe,A0,D7w*0x1),D7b
0001A208: moveq #-0x1,D5
0001A20A: bra.w 0x0001a248
0001A20E: clr.l D1
0001A210: jsr 0x00055f44.l ; call 00055F44 FUN_00055f44
0001A216: move.b (0xe,A0,D7w*0x1),D7b
0001A21A: cmp.w D6w,D7w
0001A21C: bpl.w 0x0001a222
0001A220: move.w D7w,D6w
0001A222: add.w D7w,D6w
0001A224: move.l D6,D7
0001A226: cmpi.w #0xe,D7w
0001A22A: bmi.w 0x0001a230
0001A22E: moveq #0xd,D7
0001A230: clr.l D5
0001A232: lea (0xff1100).l,A6 ; data 00ff1100
0001A238: move.w (0x00fff118).l,D1w
0001A23E: mulu.w #0x14,D1
0001A242: move.b (0x5,A6,D1w*0x1),D5b ; data 00ff1105
0001A246: subq.b #0x2,D5b
0001A248: lea (0xd9ed0).l,A1 ; data 000d9ed0
0001A24E: move.l #0xc4b0,D4
0001A254: move.w #0x0,D1w
0001A258: move.w #0x2000,D2w
0001A25C: subq.w #0x1,D7w
0001A25E: bpl.w 0x0001a264
0001A262: clr.l D7
0001A264: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0001A26A: tst.b D5b
0001A26C: bmi.w 0x0001a28e
0001A270: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
0001A274: ble.w 0x0001a28e
0001A278: lea (0xd9ed2).l,A1 ; data 000d9ed2
0001A27E: sub.w D5w,D7w
0001A280: mulu.w #0x2,D7
0001A284: add.l D7,D4
0001A286: move.w D5w,D7w
0001A288: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0001A28E: move.w #-0x1,D1w
0001A292: jsr 0x00055f44.l ; call 00055F44 FUN_00055f44
0001A298: add.b (0x00fffbbe).l,D6b
0001A29E: move.w D6w,D7w
0001A2A0: cmpi.w #0xe,D7w
0001A2A4: bmi.w 0x0001a2aa
0001A2A8: moveq #0xd,D7
0001A2AA: lea (0xd9ece).l,A1 ; data 000d9ece
0001A2B0: move.l #0xc530,D4
0001A2B6: clr.l D1
0001A2B8: subq.w #0x1,D7w
0001A2BA: bpl.w 0x0001a2c0
0001A2BE: clr.l D7
0001A2C0: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
0001A2C6: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0001A2CA: rts
0001A2CC: lea (0xffd86c).l,A3 ; data 00ffd86c
0001A2D2: move.w #0x11,D7w
0001A2D6: move.w #0x49,D1w
0001A2DA: movea.w #0x920,A2
0001A2DE: clr.w D0w
0001A2E0: tst.b (A3)+
0001A2E2: beq.w 0x0001a2fa
0001A2E6: addq.w #0x1,D0w
0001A2E8: addi.w #0x18,D1w
0001A2EC: adda.w #0x300,A2
0001A2F0: dbf D7w,0x0001a2e0
0001A2F4: ori #0x1,CCR
0001A2F8: rts
0001A2FA: suba.w #0x1,A3
0001A2FE: move.b #0x1,(A3)
0001A302: rts
0001A304: lea (0xffd86c).l,A3 ; data 00ffd86c
0001A30A: clr.b (0x0,A3,D0w*0x1) ; data 00ffd86c
0001A30E: rts
0001A310: lea (0xffd86c).l,A3 ; data 00ffd86c
0001A316: move.w #0x11,D7w
0001A31A: clr.b (A3)+
0001A31C: dbf D7w,0x0001a31a
0001A320: rts
0001A322: clr.b (0x00ffe177).l
0001A328: lea (0xff0400).l,A1 ; data 00ff0400
0001A32E: move.w #0xc,D7w
0001A332: bclr.b #0x0,(0x10,A1)
0001A338: adda.w #0x100,A1
0001A33C: dbf D7w,0x0001a332
0001A340: rts
0001A342: lea (0xff0400).l,A1 ; data 00ff0400
0001A348: move.w #0x6,D7w
0001A34C: move.w #0x3,D0w
0001A350: btst.b #0x0,(0x10,A1)
0001A356: beq.w 0x0001a36a
0001A35A: adda.w #0x100,A1
0001A35E: addq.w #0x1,D0w
0001A360: dbf D7w,0x0001a350
0001A364: ori #0x1,CCR
0001A368: rts
0001A36A: andi #0x1e,CCR
0001A36E: rts
0001A370: lea (0xff163e).l,A0 ; data 00ff163e
0001A376: tst.w (0xc,A0)
0001A37A: bne.w 0x0001a380
0001A37E: rts
0001A380: tst.w (0x00ffe082).l
0001A386: bne.w 0x0001a88e
0001A38A: bsr.w 0x0001a538 ; call 0001A538 FUN_0001a538
0001A38E: bne.w 0x0001a396
0001A392: bra.w 0x0001a818 ; call 0001A818 FUN_0001a818
0001A396: tst.b (0x00ffdf84).l
0001A39C: beq.w 0x0001a466
0001A3A0: tst.w (0x00ffe076).l
0001A3A6: bne.w 0x0001a466
0001A3AA: subq.w #0x1,(0x00ffe074).l
0001A3B0: bpl.w 0x0001a466
0001A3B4: jsr 0x00017e78.l ; call 00017E78 FUN_00017e78
0001A3BA: move.w D0w,(0x00ffe074).l
0001A3C0: jsr 0x000561e8.l ; call 000561E8 FUN_000561e8
0001A3C6: move.w D0w,(0x00ffe07e).l
0001A3CC: bmi.w 0x0001a466
0001A3D0: cmpi.w #0x2,D0w
0001A3D4: beq.w 0x0001a442
0001A3D8: cmpi.w #0x3,D0w
0001A3DC: beq.w 0x0001a3f0
0001A3E0: bsr.w 0x0001a54e ; call 0001A54E FUN_0001a54e
0001A3E4: move.w #0x2,(0x00ffe04e).l
0001A3EC: bra.w 0x0001a466
0001A3F0: move.b (0x00fff0e7).l,D0b
0001A3F6: ext.w D0w
0001A3F8: bne.w 0x0001a400
0001A3FC: move.w #0x1,D0w
0001A400: mulu.w #0xf0,D0
0001A404: clr.l D1
0001A406: move.w #0x2800,D1w
0001A40A: divu.w D0w,D1
0001A40C: move.w D1w,(0x00ffe0c0).l
0001A412: cmpi.w #0x6,(0x00ffe0bc).l
0001A41A: beq.w 0x0001a436
0001A41E: cmpi.w #0x7,(0x00ffe0bc).l
0001A426: bne.w 0x0001a466
0001A42A: move.w #0x2,(0x00ffe0b6).l
0001A432: bra.w 0x0001a466
0001A436: move.w #0x2,(0x00ffe0b8).l
0001A43E: bra.w 0x0001a466
0001A442: jsr 0x00017e78.l ; call 00017E78 FUN_00017e78
0001A448: move.l #0xa00,D1
0001A44E: divu.w D0w,D1
0001A450: move.w D1w,(0x00ffe0c4).l
0001A456: move.w #0x1,(0x00ffe0c2).l
0001A45E: move.w #0x11f0,(0x00ffe0c6).l
0001A466: rts
0001A468: clr.w (0x00ffe0d0).l
0001A46E: move.w #0x2,(0x00ffe0d2).l
0001A476: move.w #0x1,(0x00ffe0d4).l
0001A47E: move.w #0x18,D0w
0001A482: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0001A488: rts
0001A48A: subq.w #0x1,(0x00ffe0d2).l
0001A490: bpl.w 0x0001a4b6
0001A494: move.w #0x2,(0x00ffe0d2).l
0001A49C: addq.w #0x1,(0x00ffe0d0).l
0001A4A2: cmpi.w #0x3,(0x00ffe0d0).l
0001A4AA: bcs.w 0x0001a4b6
0001A4AE: clr.w (0x00ffe0d4).l
0001A4B4: rts
0001A4B6: move.w (0x00ffe0d0).l,D0w
0001A4BC: lsl.w #0x2,D0w
0001A4BE: lea (0xf1bb2).l,A2 ; data 000f1bb2
0001A4C4: movea.l (0x0,A2,D0w*0x1),A2 ; data 000f1bb2
0001A4C8: move.w #0x7da,D0w
0001A4CC: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001A4D2: move.w #0x120,D1w
0001A4D6: move.w #0xd0,D2w
0001A4DA: move.w #0x2000,D6w
0001A4DE: addq.w #0x8,A2
0001A4E0: move.w #0x7da,D4w
0001A4E4: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001A4EA: lea (0xff1100).l,A1 ; data 00ff1100
0001A4F0: move.w (0x00fff118).l,D1w
0001A4F6: mulu.w #0x14,D1
0001A4FA: move.b (0x10,A1,D1w*0x1),D1b ; data 00ff1110
0001A4FE: ext.w D1w
0001A500: lea (0x1a50e).l,A1 ; data 0001a50e
0001A506: lsl.w #0x1,D1w
0001A508: move.w (0x0,A1,D1w*0x1),D1w ; data 0001a50e
0001A50C: rts
0001A538: lea (0xff1100).l,A0 ; data 00ff1100
0001A53E: move.w (0x00fff118).l,D0w
0001A544: mulu.w #0x14,D0
0001A548: tst.b (0xf,A0,D0w*0x1) ; data 00ff110f
0001A54C: rts
0001A54E: move.w #0x1,(0x00ffe076).l
0001A556: move.w #0x1200,(0x00ffe078).l
0001A55E: move.w #0xd00,(0x00ffe07a).l
0001A566: move.w #0x1e,(0x00ffe07c).l
0001A56E: rts
0001A570: lea (0xf1b1e).l,A2 ; data 000f1b1e
0001A576: move.w #0x12a,D1w
0001A57A: move.w #0x100,D2w
0001A57E: move.w #0x2000,D6w
0001A582: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001A588: lea (0xf1b28).l,A2 ; data 000f1b28
0001A58E: move.w #0x115,D1w
0001A592: move.w #0x100,D2w
0001A596: move.w #0x2000,D6w
0001A59A: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001A5A0: cmpi.w #0x2,(0x00ffe076).l
0001A5A8: beq.w 0x0001a64c
0001A5AC: cmpi.w #0x3,(0x00ffe076).l
0001A5B4: beq.w 0x0001a6ac
0001A5B8: cmpi.w #0x5,(0x00ffe076).l
0001A5C0: beq.w 0x0001a6f8
0001A5C4: cmpi.w #0x4,(0x00ffe076).l
0001A5CC: beq.w 0x0001a6e6
0001A5D0: cmpi.w #0x6,(0x00ffe076).l
0001A5D8: beq.w 0x0001a770
0001A5DC: subq.w #0x1,(0x00ffe07c).l
0001A5E2: bpl.w 0x0001a5fa
0001A5E6: move.w #0x2,(0x00ffe076).l
0001A5EE: move.w #0x1e,(0x00ffe07c).l
0001A5F6: bra.w 0x0001a64c
0001A5FA: lea (0xf1aee).l,A2 ; data 000f1aee
0001A600: move.w (0x00ffdf88).l,D0w
0001A606: lsr.w #0x1,D0w
0001A608: andi.w #0x3,D0w
0001A60C: lsl.w #0x2,D0w
0001A60E: movea.l (0x0,A2,D0w*0x1),A2 ; data 000f1aee
0001A612: move.w #0x6e0,D0w
0001A616: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001A61C: addq.w #0x6,(0x00ffe078).l
0001A622: addi.w #-0x2,(0x00ffe07a).l
0001A62A: move.w (0x00ffe078).l,D1w
0001A630: move.w (0x00ffe07a).l,D2w
0001A636: lsr.w #0x4,D1w
0001A638: lsr.w #0x4,D2w
0001A63A: move.w #0x2000,D6w
0001A63E: adda.w #0x8,A2
0001A642: move.w #0x6e0,D4w
0001A646: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001A64C: subq.w #0x1,(0x00ffe07c).l
0001A652: bpl.w 0x0001a72a
0001A656: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2},-(SP)
0001A65A: jsr 0x000562ec.l ; call 000562EC FUN_000562ec
0001A660: movem.l (SP)+,{  D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0001A664: tst.w (0x00ffe07e).l
0001A66A: bne.w 0x0001a6d6
0001A66E: move.l D1,-(SP) ; data Stack[-0x4]
0001A670: move.w #0x18,D0w
0001A674: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0001A67A: move.l (SP)+,D1
0001A67C: move.w D1w,(0x00ffe0fc).l
0001A682: move.w #0x8,(0x00ffe07c).l
0001A68A: move.w #0x3,(0x00ffe076).l
0001A692: tst.b (0x00fff0d2).l
0001A698: beq.w 0x0001a6ac
0001A69C: move.w #0x8,(0x00ffe0f0).l
0001A6A4: move.w #0x1,(0x00ffe0ee).l
0001A6AC: subq.w #0x1,(0x00ffe07c).l
0001A6B2: bpl.w 0x0001a6ca
0001A6B6: move.w #0x5,(0x00ffe076).l
0001A6BE: move.w #0x10,(0x00ffe07c).l
0001A6C6: bra.w 0x0001a6f8
0001A6CA: tst.w (0x00ffe0fc).l
0001A6D0: bmi.w 0x0001a570
0001A6D4: rts
0001A6D6: move.w #0x10,(0x00ffe07c).l
0001A6DE: move.w #0x4,(0x00ffe076).l
0001A6E6: subq.w #0x1,(0x00ffe07c).l
0001A6EC: bpl.w 0x0001a70a
0001A6F0: bsr.w 0x0001b680 ; call 0001B680 FUN_0001b680
0001A6F4: bra.w 0x0001a716
0001A6F8: subq.w #0x1,(0x00ffe07c).l
0001A6FE: bpl.w 0x0001a71e
0001A702: bsr.w 0x0001b680 ; call 0001B680 FUN_0001b680
0001A706: bra.w 0x0001a716
0001A70A: tst.w (0x00ffe0d6).l
0001A710: beq.w 0x0001b5de
0001A714: rts
0001A716: clr.w (0x00ffe076).l
0001A71C: rts
0001A71E: tst.w (0x00ffe0d6).l
0001A724: beq.w 0x0001b5fc
0001A728: rts
0001A72A: lea (0xf1aee).l,A2 ; data 000f1aee
0001A730: move.w (0x00ffdf88).l,D0w
0001A736: lsr.w #0x1,D0w
0001A738: andi.w #0x3,D0w
0001A73C: addq.w #0x4,D0w
0001A73E: cmpi.w #0xf,(0x00ffe07c).l
0001A746: bcc.w 0x0001a74c
0001A74A: addq.w #0x4,D0w
0001A74C: lsl.w #0x2,D0w
0001A74E: movea.l (0x0,A2,D0w*0x1),A2 ; data 000f1aee
0001A752: move.w #0x6e0,D0w
0001A756: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001A75C: addi.w #-0x6,(0x00ffe078).l
0001A764: addi.w #0x1a,(0x00ffe07a).l
0001A76C: bra.w 0x0001a62a
0001A770: subq.w #0x1,(0x00ffe07c).l
0001A776: bpl.w 0x0001a788
0001A77A: jsr 0x0005668c.l ; call 0005668C FUN_0005668c
0001A780: clr.w (0x00ffe076).l
0001A786: rts
0001A788: lea (0xf1aee).l,A2 ; data 000f1aee
0001A78E: move.w (0x00ffdf88).l,D0w
0001A794: lsr.w #0x1,D0w
0001A796: andi.w #0x3,D0w
0001A79A: lsl.w #0x2,D0w
0001A79C: movea.l (0x0,A2,D0w*0x1),A2 ; data 000f1aee
0001A7A0: move.w #0x6e0,D0w
0001A7A4: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001A7AA: addi.w #-0x2,(0x00ffe078).l
0001A7B2: addi.w #-0xe,(0x00ffe07a).l
0001A7BA: bra.w 0x0001a62a
0001A7BE: lea (0xf1aee).l,A2 ; data 000f1aee
0001A7C4: move.w (0x00ffdf88).l,D0w
0001A7CA: lsr.w #0x1,D0w
0001A7CC: andi.w #0x3,D0w
0001A7D0: addq.w #0x8,D0w
0001A7D2: lsl.w #0x2,D0w
0001A7D4: movea.l (0x0,A2,D0w*0x1),A2 ; data 000f1aee
0001A7D8: move.w #0x6e0,D0w
0001A7DC: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001A7E2: addi.w #0x60,(0x00ffe07a).l
0001A7EA: move.w (0x00ffe078).l,D1w
0001A7F0: move.w (0x00ffe07a).l,D2w
0001A7F6: cmpi.w #0x1280,(0x00ffe07a).l
0001A7FE: bcs.w 0x0001a804
0001A802: rts
0001A804: lsr.w #0x4,D1w
0001A806: lsr.w #0x4,D2w
0001A808: move.w #-0x6000,D6w
0001A80C: addq.w #0x8,A2
0001A80E: move.w #0x6e0,D4w
0001A812: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001A818: tst.w (0x00ffe0d4).l
0001A81E: beq.w 0x0001a824
0001A822: rts
0001A824: clr.w (0x00ffe076).l
0001A82A: move.w #0x1,(0x00ffe082).l
0001A832: lea (0xff163e).l,A0 ; data 00ff163e
0001A838: tst.w (0x00ffe0f4).l
0001A83E: bne.w 0x0001a856
0001A842: move.w #0x1,(0x00ffe0f2).l
0001A84A: move.b #0x6,(0x4,A0)
0001A850: clr.b (0x5,A0)
0001A854: rts
0001A856: clr.w (0x00ffe0f2).l
0001A85C: clr.w D2w
0001A85E: move.b (0x5,A0),D2b
0001A862: lsl.w #0x2,D2w
0001A864: movea.l (0x8,A0),A2
0001A868: movea.l (0x0,A2,D2w*0x1),A2
0001A86C: adda.w #0x2,A2
0001A870: move.l (A2),(0x00ffdf7e).l
0001A876: move.w #-0x2d80,(0x00ffdf76).l
0001A87E: clr.b (0x4,A0)
0001A882: move.b #0x7,(0x6,A0)
0001A888: jmp 0x0001b680.l ; call 0001B680 FUN_0001b680
0001A88E: lea (0xff163e).l,A0 ; data 00ff163e
0001A894: tst.w (0x00ffe0f2).l
0001A89A: beq.w 0x0001a8a0
0001A89E: rts
0001A8A0: subq.b #0x1,(0x4,A0)
0001A8A4: bpl.w 0x0001a8ba
0001A8A8: move.b #0x2,(0x4,A0)
0001A8AE: subq.b #0x1,(0x6,A0)
0001A8B2: bpl.w 0x0001a8ba
0001A8B6: bra.w 0x0001a8f4 ; call 0001A8F4 FUN_0001a8f4
0001A8BA: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0001A8C0: lea (0xc00000).l,A4 ; data 00c00000
0001A8C6: move.w (0x00ffdf76).l,D0w
0001A8CC: movea.l (0x00ffdf7e).l,A1
0001A8D2: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0001A8D8: move.w #0x1ff,D6w
0001A8DC: move.b (0x6,A0),D0b
0001A8E0: ext.w D0w
0001A8E2: jsr 0x0001d3f4.l ; call 0001D3F4 FUN_0001d3f4
0001A8E8: move.l (A1)+,D1
0001A8EA: and.l D1,D0
0001A8EC: move.l D0,(A4)
0001A8EE: dbf D6w,0x0001a8dc
0001A8F2: rts
0001A8F4: clr.w (0x00ffe082).l
0001A8FA: move.w #0x1,(0x00ffe080).l
0001A902: clr.w (0xc,A0)
0001A906: rts
0001A908: move.w #0x1,(0x00ff164a).l
0001A910: tst.w (0x00ffe0de).l
0001A916: beq.w 0x0001a924
0001A91A: move.w (0x00ffe0e0).l,D0w
0001A920: bra.w 0x0001a92a
0001A924: jsr 0x00019b40.l ; call 00019B40 FUN_00019b40
0001A92A: move.w D0w,(0x00ffe0bc).l
0001A930: tst.w D0w
0001A932: bpl.w 0x0001a940
0001A936: lea (0x1aa10).l,A1 ; data 0001aa10
0001A93C: bra.w 0x0001a9ac
0001A940: cmpi.w #0x45,D0w
0001A944: bne.w 0x0001a958
0001A948: clr.w (0x00ff164a).l
0001A94E: move.w #0x1,(0x00ffe080).l
0001A956: rts
0001A958: cmpi.w #0x6,D0w
0001A95C: beq.w 0x0001a986
0001A960: cmpi.w #0x7,D0w
0001A964: bne.w 0x0001a9a0
0001A968: tst.w (0x00ffe0b6).l
0001A96E: bne.w 0x0001a9a0
0001A972: move.w #0x1,(0x00ffe0b6).l
0001A97A: move.w #0x4800,(0x00ffe0be).l
0001A982: bra.w 0x0001a9a0
0001A986: tst.w (0x00ffe0b8).l
0001A98C: bne.w 0x0001a9a0
0001A990: move.w #0x1,(0x00ffe0b8).l
0001A998: move.w #0x47c0,(0x00ffe0ba).l
0001A9A0: lsl.w #0x2,D0w
0001A9A2: lea (0x1a9ce).l,A1 ; data 0001a9ce
0001A9A8: movea.l (0x0,A1,D0w*0x1),A1 ; data 0001a9ce
0001A9AC: lea (0xff163e).l,A0 ; data 00ff163e
0001A9B2: move.b (A1)+,(0x6,A0)
0001A9B6: move.b (A1)+,(0x7,A0)
0001A9BA: move.l (A1)+,(0x8,A0)
0001A9BE: clr.w (0x4,A0)
0001A9C2: move.w #0x120,(A0)
0001A9C6: move.w #0xd0,(0x2,A0)
0001A9CC: rts
0001AA40: lea (0xff163e).l,A0 ; data 00ff163e
0001AA46: tst.w (0xc,A0)
0001AA4A: bne.w 0x0001aa50
0001AA4E: rts
0001AA50: tst.w (0x00ffe0f2).l
0001AA56: bne.w 0x0001ab1a
0001AA5A: tst.w (0x00ffe082).l
0001AA60: bne.w 0x0001aad0
0001AA64: tst.w (0x00ffdf8c).l
0001AA6A: beq.w 0x0001aaa0
0001AA6E: tst.w (0x00ffe0bc).l
0001AA74: bne.w 0x0001aaa0
0001AA78: subq.b #0x1,(0x4,A0)
0001AA7C: bpl.w 0x0001aad0
0001AA80: move.b (0x6,A0),(0x4,A0)
0001AA86: addq.b #0x1,(0x5,A0)
0001AA8A: move.b (0x5,A0),D2b
0001AA8E: cmpi.b #0x4,D2b
0001AA92: bcs.w 0x0001aad0
0001AA96: move.b #0x3,(0x5,A0)
0001AA9C: bra.w 0x0001aad0
0001AAA0: subq.b #0x1,(0x4,A0)
0001AAA4: bpl.w 0x0001aad0
0001AAA8: move.b (0x6,A0),D0b
0001AAAC: bsr.w 0x0001a4ea ; call 0001A4EA FUN_0001a4ea
0001AAB0: add.b D1b,D0b
0001AAB2: bpl.w 0x0001aab8
0001AAB6: clr.b D0b
0001AAB8: move.b D0b,(0x4,A0)
0001AABC: addq.b #0x1,(0x5,A0)
0001AAC0: move.b (0x5,A0),D2b
0001AAC4: cmp.b (0x7,A0),D2b
0001AAC8: bcs.w 0x0001aad0
0001AACC: clr.b (0x5,A0)
0001AAD0: clr.w D2w
0001AAD2: move.b (0x5,A0),D2b
0001AAD6: lsl.w #0x2,D2w
0001AAD8: movea.l (0x8,A0),A2
0001AADC: movea.l (0x0,A2,D2w*0x1),A2
0001AAE0: tst.w (0x00ffe082).l
0001AAE6: bne.w 0x0001aaf4
0001AAEA: move.w #0x694,D0w
0001AAEE: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001AAF4: move.w #0x2000,D6w
0001AAF8: move.w (A0),D1w
0001AAFA: move.w (0x2,A0),D2w
0001AAFE: move.w #0x694,D4w
0001AB02: adda.w #0x8,A2
0001AB06: jsr 0x000013be.l ; call 000013BE FUN_000013be
0001AB0C: bsr.w 0x0001ab6e ; call 0001AB6E FUN_0001ab6e
0001AB10: bsr.w 0x0001ac6a ; call 0001AC6A FUN_0001ac6a
0001AB14: bsr.w 0x0001ad42 ; call 0001AD42 FUN_0001ad42
0001AB18: rts
0001AB1A: subq.b #0x1,(0x4,A0)
0001AB1E: bpl.w 0x0001ab3a
0001AB22: move.b #0x5,(0x4,A0)
0001AB28: addq.b #0x1,(0x5,A0)
0001AB2C: cmpi.b #0x6,(0x5,A0)
0001AB32: bcs.w 0x0001ab3a
0001AB36: bra.w 0x0001a8f4 ; call 0001A8F4 FUN_0001a8f4
0001AB3A: clr.w D0w
0001AB3C: move.b (0x5,A0),D0b
0001AB40: lsl.w #0x2,D0w
0001AB42: lea (0xf1c46).l,A2 ; data 000f1c46
0001AB48: movea.l (0x0,A2,D0w*0x1),A2 ; data 000f1c46
0001AB4C: move.w #0x694,D0w
0001AB50: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001AB56: move.w #0x2000,D6w
0001AB5A: move.w (A0),D1w
0001AB5C: move.w (0x2,A0),D2w
0001AB60: move.w #0x694,D4w
0001AB64: adda.w #0x8,A2
0001AB68: jsr 0x000013be.l ; call 000013BE FUN_000013be
0001AB6E: tst.w (0x00ffe084).l
0001AB74: bne.w 0x0001ab82
0001AB78: tst.w (0x00ffe0b6).l
0001AB7E: bne.w 0x0001ab84
0001AB82: rts
0001AB84: tst.w (0x00ffe0c8).l
0001AB8A: beq.w 0x0001abc0
0001AB8E: move.w (0x00ffe056).l,D0w
0001AB94: lsl.w #0x2,D0w
0001AB96: move.w D0w,(0x00ffe0be).l
0001AB9C: cmpi.w #0x6b00,(0x00ffe0be).l
0001ABA4: bcs.w 0x0001abc0
0001ABA8: clr.w (0x00ffe0c8).l
0001ABAE: clr.w (0x00ffe0b6).l
0001ABB4: move.w #0x1,(0x00ffe0f4).l
0001ABBC: bra.w 0x0001a818 ; call 0001A818 FUN_0001a818
0001ABC0: cmpi.w #0x1,(0x00ffe0b6).l
0001ABC8: beq.w 0x0001abec
0001ABCC: move.w (0x00ffe0c0).l,D0w
0001ABD2: add.w D0w,(0x00ffe0be).l
0001ABD8: move.w (0x00ffe0be).l,D0w
0001ABDE: bsr.w 0x0001ac50 ; call 0001AC50 FUN_0001ac50
0001ABE2: bcs.w 0x0001abec
0001ABE6: jmp 0x000563cc.l ; call 000563CC FUN_000563cc
0001ABEC: subq.w #0x1,(0x00ffe0b4).l
0001ABF2: bpl.w 0x0001ac16
0001ABF6: move.w #0x8,(0x00ffe0b4).l
0001ABFE: addq.w #0x1,(0x00ffe0b2).l
0001AC04: cmpi.w #0x1a,(0x00ffe0b2).l
0001AC0C: bcs.w 0x0001ac16
0001AC10: clr.w (0x00ffe0b2).l
0001AC16: move.w (0x00ffe0b2).l,D0w
0001AC1C: lsl.w #0x2,D0w
0001AC1E: lea (0xf1bde).l,A2 ; data 000f1bde
0001AC24: movea.l (0x0,A2,D0w*0x1),A2
0001AC28: move.w #0x7a8,D0w
0001AC2C: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001AC32: move.w #0xb9,D2w
0001AC36: move.w (0x00ffe0be).l,D1w
0001AC3C: lsr.w #0x6,D1w
0001AC3E: move.w #0x2000,D6w
0001AC42: adda.w #0x8,A2
0001AC46: move.w #0x7a8,D4w
0001AC4A: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001AC50: cmpi.w #0x7140,D0w
0001AC54: bcc.w 0x0001ac5a
0001AC58: rts
0001AC5A: move.w #0x1,(0x00ffe084).l
0001AC62: clr.w (0x00ffe086).l
0001AC68: rts
0001AC6A: tst.w (0x00ffe084).l
0001AC70: bne.w 0x0001ac7e
0001AC74: tst.w (0x00ffe0b8).l
0001AC7A: bne.w 0x0001ac80
0001AC7E: rts
0001AC80: tst.w (0x00ffe0c8).l
0001AC86: beq.w 0x0001acbc
0001AC8A: move.w (0x00ffe056).l,D0w
0001AC90: lsl.w #0x2,D0w
0001AC92: move.w D0w,(0x00ffe0ba).l
0001AC98: cmpi.w #0x6b00,(0x00ffe0ba).l
0001ACA0: bcs.w 0x0001acbc
0001ACA4: clr.w (0x00ffe0b8).l
0001ACAA: clr.w (0x00ffe0c8).l
0001ACB0: move.w #0x1,(0x00ffe0f4).l
0001ACB8: bra.w 0x0001a818 ; call 0001A818 FUN_0001a818
0001ACBC: cmpi.w #0x1,(0x00ffe0b8).l
0001ACC4: beq.w 0x0001acde
0001ACC8: move.w (0x00ffe0c0).l,D0w
0001ACCE: add.w D0w,(0x00ffe0ba).l
0001ACD4: move.w (0x00ffe0ba).l,D0w
0001ACDA: bsr.w 0x0001ac50 ; call 0001AC50 FUN_0001ac50
0001ACDE: subq.w #0x1,(0x00ffe0b4).l
0001ACE4: bpl.w 0x0001ad08
0001ACE8: move.w #0x6,(0x00ffe0b4).l
0001ACF0: addq.w #0x1,(0x00ffe0b2).l
0001ACF6: cmpi.w #0x4,(0x00ffe0b2).l
0001ACFE: bcs.w 0x0001ad08
0001AD02: clr.w (0x00ffe0b2).l
0001AD08: move.w (0x00ffe0b2).l,D0w
0001AD0E: lsl.w #0x2,D0w
0001AD10: lea (0xf1bce).l,A2 ; data 000f1bce
0001AD16: movea.l (0x0,A2,D0w*0x1),A2
0001AD1A: move.w #0x7a8,D0w
0001AD1E: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001AD24: move.w #0xb9,D2w
0001AD28: move.w (0x00ffe0ba).l,D1w
0001AD2E: lsr.w #0x6,D1w
0001AD30: move.w #0x2000,D6w
0001AD34: adda.w #0x8,A2
0001AD38: move.w #0x7a8,D4w
0001AD3C: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001AD42: tst.w (0x00ffe084).l
0001AD48: bne.w 0x0001ad56
0001AD4C: tst.w (0x00ffe0c2).l
0001AD52: bne.w 0x0001ad58
0001AD56: rts
0001AD58: tst.b (0x00fff0a3).l
0001AD5E: bne.w 0x0001ad68
0001AD62: clr.w (0x00ffe0c2).l
0001AD68: move.w (0x00ffe0c4).l,D0w
0001AD6E: add.w D0w,(0x00ffe0c6).l
0001AD74: subq.w #0x1,(0x00ffe0b4).l
0001AD7A: bpl.w 0x0001ad9e
0001AD7E: move.w #0x3,(0x00ffe0b4).l
0001AD86: addq.w #0x1,(0x00ffe0b2).l
0001AD8C: cmpi.w #0x4,(0x00ffe0b2).l
0001AD94: bcs.w 0x0001ad9e
0001AD98: clr.w (0x00ffe0b2).l
0001AD9E: move.w (0x00ffe0b2).l,D0w
0001ADA4: lsl.w #0x2,D0w
0001ADA6: lea (0xf1bbe).l,A2 ; data 000f1bbe
0001ADAC: movea.l (0x0,A2,D0w*0x1),A2
0001ADB0: move.w #0x7a8,D0w
0001ADB4: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001ADBA: move.w #0xd0,D2w
0001ADBE: move.w (0x00ffe0c6).l,D1w
0001ADC4: lsr.w #0x4,D1w
0001ADC6: move.w #0x2000,D6w
0001ADCA: adda.w #0x8,A2
0001ADCE: move.w #0x7a8,D4w
0001ADD2: jsr 0x000013be.l ; call 000013BE FUN_000013be
0001ADD8: tst.w (0x00ffe0b2).l
0001ADDE: beq.w 0x0001ade4
0001ADE2: rts
0001ADE4: rts
0001ADE6: lea (0xff1596).l,A0 ; data 00ff1596
0001ADEC: move.w #0x120,(A0)
0001ADF0: move.w #0xd0,(0x2,A0)
0001ADF6: bsr.w 0x0001b5a6 ; call 0001B5A6 FUN_0001b5a6
0001ADFA: clr.w (0x00ffdf8c).l
0001AE00: clr.w (0x00ffe080).l
0001AE06: clr.w (0x00ffe084).l
0001AE0C: clr.w (0x00ffe0f6).l
0001AE12: clr.w (0x00ffe0aa).l
0001AE18: clr.w (0x00ffe0a8).l
0001AE1E: clr.w (0x00ffe058).l
0001AE24: clr.w (0x00ffe076).l
0001AE2A: clr.w (0x00ffe0d4).l
0001AE30: clr.w (0x00ffe0f4).l
0001AE36: clr.w (0x00ffe082).l
0001AE3C: clr.w (0x00ffe0ac).l
0001AE42: clr.w (0x00ffe0f2).l
0001AE48: clr.w (0x00ffe0b6).l
0001AE4E: clr.w (0x00ffe0b8).l
0001AE54: clr.w (0x00ffe0c2).l
0001AE5A: clr.w (0x00ffe0d6).l
0001AE60: clr.w (0x00ffe0ee).l
0001AE66: clr.w (0x00fff11e).l
0001AE6C: clr.b (0x00ff1592).l
0001AE72: clr.w (0x00ffe05e).l
0001AE78: clr.w (0x00ffe06e).l
0001AE7E: clr.b (0x00ff15a4).l
0001AE84: move.b (0x00fff0a0).l,D0b
0001AE8A: move.b D0b,(0x00ffe189).l
0001AE90: clr.b (0x00ffe16d).l
0001AE96: jsr 0x00026958.l ; call 00026958 FUN_00026958
0001AE9C: jsr 0x0001a908.l ; call 0001A908 FUN_0001a908
0001AEA2: lea (0xff1580).l,A0 ; data 00ff1580
0001AEA8: bsr.w 0x0001b680 ; call 0001B680 FUN_0001b680
0001AEAC: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0001AEB2: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001AEB8: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0001AEBE: jsr 0x00017fda.l ; call 00017FDA FUN_00017fda
0001AEC4: tst.w (0x00ffdf8c).l
0001AECA: bne.w 0x0001aedc
0001AECE: tst.w (0x00ffe080).l
0001AED4: beq.w 0x0001af22
0001AED8: bsr.w 0x0001d280 ; call 0001D280 FUN_0001d280
0001AEDC: bsr.w 0x0001d2e6 ; call 0001D2E6 FUN_0001d2e6
0001AEE0: bcs.w 0x0001aee8
0001AEE4: bra.w 0x0001afc0
0001AEE8: clr.w (0x00ffdf8c).l
0001AEEE: jsr 0x00053126.l ; call 00053126 FUN_00053126
0001AEF4: move.w #0x5,D0w
0001AEF8: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0001AEFE: tst.b (0x00fff0a9).l
0001AF04: beq.w 0x0001af12
0001AF08: jsr 0x00053704.l ; call 00053704 FUN_00053704
0001AF0E: bra.w 0x0001af18
0001AF12: jsr 0x00054662.l ; call 00054662 FUN_00054662
0001AF18: jsr 0x00053126.l ; call 00053126 FUN_00053126
0001AF1E: bra.w 0x0001b1b6
0001AF22: tst.w (0x00ffe084).l
0001AF28: beq.w 0x0001af3e
0001AF2C: bsr.w 0x0001b302 ; call 0001B302 FUN_0001b302
0001AF30: tst.w (0x00ffe084).l
0001AF36: bne.w 0x0001af64
0001AF3A: bra.w 0x0001b1b8
0001AF3E: tst.b (0x00fffbca).l
0001AF44: beq.w 0x0001af56
0001AF48: movea.l (0x00fffc04).l,A2
0001AF4E: tst.b (0x4c,A2)
0001AF52: bne.w 0x0001af64
0001AF56: move.w #0x1,(0x00ffe084).l
0001AF5E: clr.w (0x00ffe086).l
0001AF64: jsr 0x00019788.l ; call 00019788 FUN_00019788
0001AF6A: bsr.w 0x0001c754 ; call 0001C754 FUN_0001c754
0001AF6E: jsr 0x00019432.l ; call 00019432 FUN_00019432
0001AF74: bsr.w 0x0001a370 ; call 0001A370 FUN_0001a370
0001AF78: tst.b (0x00fff0b1).l
0001AF7E: bmi.w 0x0001b1b6
0001AF82: tst.b (0x00ffe16d).l
0001AF88: beq.w 0x0001afae
0001AF8C: subq.b #0x1,(0x00ffe16d).l
0001AF92: bne.w 0x0001afae
0001AF96: move.w #0x1a,D0w
0001AF9A: tst.b (0x00ffe18e).l
0001AFA0: bne.w 0x0001afa8
0001AFA4: move.w #0x1b,D0w
0001AFA8: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001AFAE: tst.w (0x00ffe0aa).l
0001AFB4: beq.w 0x0001afc0
0001AFB8: bsr.w 0x0001c0c2 ; call 0001C0C2 FUN_0001c0c2
0001AFBC: bra.w 0x0001afc4
0001AFC0: bsr.w 0x0001bf7a ; call 0001BF7A FUN_0001bf7a
0001AFC4: move.w #0x1,(0x00ffdd02).l
0001AFCC: lea (0xffd986).l,A3 ; data 00ffd986
0001AFD2: move.w #0x2,D5w
0001AFD6: tst.w (0x00ffdf8c).l
0001AFDC: beq.w 0x0001afe8
0001AFE0: bsr.w 0x0001b364 ; call 0001B364 FUN_0001b364
0001AFE4: bra.w 0x0001b116
0001AFE8: cmpi.w #0x4,(0x00ffe076).l
0001AFF0: bne.w 0x0001affa
0001AFF4: jsr 0x0001a7be.l ; call 0001A7BE FUN_0001a7be
0001AFFA: tst.w (0x00ffe0f6).l
0001B000: beq.w 0x0001b008
0001B004: bsr.w 0x0001b218 ; call 0001B218 FUN_0001b218
0001B008: jsr 0x00026972.l ; call 00026972 FUN_00026972
0001B00E: bsr.w 0x0001c3fa ; call 0001C3FA FUN_0001c3fa
0001B012: btst.b #0x0,(0x00ff1592).l
0001B01A: beq.w 0x0001b022
0001B01E: bsr.w 0x0001c61c ; call 0001C61C FUN_0001c61c
0001B022: btst.b #0x1,(0x00ff1592).l
0001B02A: beq.w 0x0001b032
0001B02E: bsr.w 0x0001c1f6 ; call 0001C1F6 FUN_0001c1f6
0001B032: btst.b #0x2,(0x00ff1592).l
0001B03A: beq.w 0x0001b042
0001B03E: bsr.w 0x0001b45c ; call 0001B45C FUN_0001b45c
0001B042: btst.b #0x4,(0x00ff1592).l
0001B04A: beq.w 0x0001b058
0001B04E: lea (0xff1580).l,A0 ; data 00ff1580
0001B054: bsr.w 0x0001d1f4 ; call 0001D1F4 FUN_0001d1f4
0001B058: btst.b #0x6,(0x00ff1592).l
0001B060: beq.w 0x0001b06e
0001B064: lea (0xff1580).l,A0 ; data 00ff1580
0001B06A: bsr.w 0x0001cd5a ; call 0001CD5A FUN_0001cd5a
0001B06E: btst.b #0x5,(0x00ff1592).l
0001B076: beq.w 0x0001b084
0001B07A: lea (0xff1580).l,A0 ; data 00ff1580
0001B080: bsr.w 0x0001b87a ; call 0001B87A FUN_0001b87a
0001B084: tst.w (0x00ffe0ac).l
0001B08A: beq.w 0x0001b092
0001B08E: bsr.w 0x0001ba6e ; call 0001BA6E FUN_0001ba6e
0001B092: btst.b #0x0,(0x00fff0a1).l
0001B09A: beq.w 0x0001b0a2
0001B09E: bsr.w 0x0001bb6a ; call 0001BB6A FUN_0001bb6a
0001B0A2: btst.b #0x7,(0x00ff1592).l
0001B0AA: beq.w 0x0001b0b8
0001B0AE: lea (0xff1580).l,A0 ; data 00ff1580
0001B0B4: bsr.w 0x0001beba ; call 0001BEBA FUN_0001beba
0001B0B8: tst.w (0x00ffe058).l
0001B0BE: beq.w 0x0001b0c6
0001B0C2: bsr.w 0x0001bce8 ; call 0001BCE8 FUN_0001bce8
0001B0C6: tst.w (0x00ffe05e).l
0001B0CC: beq.w 0x0001b0d4
0001B0D0: bsr.w 0x0001c866 ; call 0001C866 FUN_0001c866
0001B0D4: tst.w (0x00ffe06e).l
0001B0DA: beq.w 0x0001b0e2
0001B0DE: bsr.w 0x0001ca12 ; call 0001CA12 FUN_0001ca12
0001B0E2: tst.w (0x00ffe076).l
0001B0E8: beq.w 0x0001b0f0
0001B0EC: bsr.w 0x0001a5a0 ; call 0001A5A0 FUN_0001a5a0
0001B0F0: tst.w (0x00ffe0d4).l
0001B0F6: beq.w 0x0001b100
0001B0FA: jsr 0x0001a48a.l ; call 0001A48A FUN_0001a48a
0001B100: tst.w (0x00ffe0de).l
0001B106: beq.w 0x0001b10e
0001B10A: bsr.w 0x0001b282 ; call 0001B282 FUN_0001b282
0001B10E: bsr.w 0x0001ccdc ; call 0001CCDC FUN_0001ccdc
0001B112: bsr.w 0x0001cc24 ; call 0001CC24 FUN_0001cc24
0001B116: tst.w (0x00ffe080).l
0001B11C: bne.w 0x0001b126
0001B120: jsr 0x0001aa40.l ; call 0001AA40 FUN_0001aa40
0001B126: jsr 0x000268e6.l ; call 000268E6 FUN_000268e6
0001B12C: subq.w #0x8,A3
0001B12E: clr.b (0x3,A3)
0001B132: move.w #0x1,(0x00ffd980).l
0001B13A: tst.w (0x00ffdf8c).l
0001B140: bne.w 0x0001b1aa
0001B144: btst.b #0xf,(0x00ffdd22).l
0001B14C: beq.w 0x0001b1aa
0001B150: jsr 0x00053126.l ; call 00053126 FUN_00053126
0001B156: move.w #0x4,D0w
0001B15A: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0001B160: bsr.w 0x0001b5a6 ; call 0001B5A6 FUN_0001b5a6
0001B164: jsr 0x0001822c.l ; call 0001822C FUN_0001822c
0001B16A: tst.b (0x00fff0b1).l
0001B170: bne.w 0x0001b1b6
0001B174: move.w D7w,-(SP) ; data Stack[-0x2]
0001B176: jsr 0x000176ba.l ; call 000176BA FUN_000176ba
0001B17C: move.w (SP)+,D7w
0001B17E: tst.w D7w
0001B180: bne.w 0x0001b188
0001B184: bra.w 0x0001b1b6
0001B188: bsr.w 0x000186fa ; call 000186FA FUN_000186fa
0001B18C: jsr 0x0001789c.l ; call 0001789C FUN_0001789c
0001B192: move.w #-0x7e8c,(0x00c00004).l
0001B19A: jsr 0x00053126.l ; call 00053126 FUN_00053126
0001B1A0: move.w #0x3,D0w
0001B1A4: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0001B1AA: bset.b #0x1,(0x00ffef14).l
0001B1B2: bra.w 0x0001aeac
0001B1B6: rts
0001B1B8: jsr 0x00018776.l ; call 00018776 FUN_00018776
0001B1BE: jmp 0x00026a00.l ; call 00026A00 FUN_00026a00
0001B1C4: move.w D2w,(0x00ffe0e4).l
0001B1CA: move.w D1w,(0x00ffe0e0).l
0001B1D0: move.w #0x1,(0x00ffe0de).l
0001B1D8: clr.w (0x00ffe0dc).l
0001B1DE: move.w #0xc,(0x00ffe0da).l
0001B1E6: lea (0x1b278).l,A1 ; data 0001b278
0001B1EC: lsl.w #0x1,D2w
0001B1EE: move.w (0x0,A1,D2w*0x1),(0x00ffe0d8).l
0001B1F6: jmp 0x0001a908.l ; call 0001A908 FUN_0001a908
0001B1FC: clr.w (0x00ffe076).l
0001B202: move.w #0x1,(0x00ffe0f6).l
0001B20A: clr.w (0x00ffe0fa).l
0001B210: clr.w (0x00ffe0f8).l
0001B216: rts
0001B218: subq.w #0x1,(0x00ffe0fa).l
0001B21E: bpl.w 0x0001b242
0001B222: move.w #0x3,(0x00ffe0fa).l
0001B22A: addq.w #0x1,(0x00ffe0f8).l
0001B230: cmpi.w #0x6,(0x00ffe0f8).l
0001B238: bcs.w 0x0001b242
0001B23C: clr.w (0x00ffe0f8).l
0001B242: move.w (0x00ffe0f8).l,D0w
0001B248: lsl.w #0x2,D0w
0001B24A: lea (0x1db92).l,A2 ; data 0001db92
0001B250: movea.l (0x0,A2,D0w*0x1),A2
0001B254: move.w #0x6e0,D0w
0001B258: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001B25E: move.w #0x10d,D1w
0001B262: move.w #0x12e,D2w
0001B266: move.w #0x2000,D6w
0001B26A: addq.w #0x8,A2
0001B26C: move.w #0x6e0,D4w
0001B270: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001B282: subq.w #0x1,(0x00ffe0da).l
0001B288: bpl.w 0x0001b2dc
0001B28C: move.w #0xc,(0x00ffe0da).l
0001B294: addq.w #0x1,(0x00ffe0dc).l
0001B29A: cmpi.w #0x6,(0x00ffe0dc).l
0001B2A2: bne.w 0x0001b2b8
0001B2A6: move.w (0x00ffe0e4).l,D4w
0001B2AC: jsr 0x00019632.l ; call 00019632 FUN_00019632
0001B2B2: jsr 0x00017bc0.l ; call 00017BC0 FUN_00017bc0
0001B2B8: cmpi.w #0xc,(0x00ffe0dc).l
0001B2C0: bcs.w 0x0001b2dc
0001B2C4: jsr 0x00019aee.l ; call 00019AEE FUN_00019aee
0001B2CA: clr.w (0x00ffe0de).l
0001B2D0: jsr 0x0001792e.l ; call 0001792E FUN_0001792e
0001B2D6: jmp 0x0001a908.l ; call 0001A908 FUN_0001a908
0001B2DC: move.w (0x00ffe0dc).l,D0w
0001B2E2: lsl.w #0x2,D0w
0001B2E4: lea (0x1db1c).l,A2 ; data 0001db1c
0001B2EA: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001db1c
0001B2EE: move.w (0x00ffe0d8).l,D1w
0001B2F4: move.w #0x147,D2w
0001B2F8: move.w #0x2000,D6w
0001B2FC: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001B302: addq.w #0x1,(0x00ffe086).l
0001B308: move.w (0x00ffe086).l,D1w
0001B30E: cmpi.w #0x3c,D1w
0001B312: bcs.w 0x0001b322
0001B316: clr.w (0x00ffe084).l
0001B31C: bsr.w 0x0001b5a6 ; call 0001B5A6 FUN_0001b5a6
0001B320: rts
0001B322: lea (0xffcb40).l,A0 ; data 00ffcb40
0001B328: move.w #0x57,D7w
0001B32C: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001B332: move.w D0w,D1w
0001B334: andi.w #0x1f,D0w
0001B338: btst.l #0x7,D1
0001B33C: beq.w 0x0001b342
0001B340: neg.w D0w
0001B342: lsr.w #0x8,D1w
0001B344: move.w D1w,D2w
0001B346: andi.w #0x1f,D1w
0001B34A: btst.l #0x7,D2
0001B34E: beq.w 0x0001b354
0001B352: neg.w D1w
0001B354: add.w D0w,(A0)+
0001B356: add.w D0w,(A0)+
0001B358: add.w D1w,(A0)+
0001B35A: add.w D1w,(A0)+
0001B35C: dbf D7w,0x0001b32c
0001B360: bra.w 0x0001b56e
0001B364: bsr.w 0x0001c3fa ; call 0001C3FA FUN_0001c3fa
0001B368: subq.w #0x1,(0x00ffe0b0).l
0001B36E: bpl.w 0x0001b392
0001B372: move.w #0x0,(0x00ffe0b0).l
0001B37A: addq.w #0x1,(0x00ffe0ae).l
0001B380: cmpi.w #0x5,(0x00ffe0ae).l
0001B388: bcs.w 0x0001b392
0001B38C: clr.w (0x00ffe0ae).l
0001B392: nop
0001B394: nop
0001B396: nop
0001B398: move.w (0x00ffe0ae).l,D0w
0001B39E: lsl.w #0x2,D0w
0001B3A0: lea (0x1daf4).l,A2 ; data 0001daf4
0001B3A6: movea.l (0x0,A2,D0w*0x1),A2
0001B3AA: move.w #0x780,D0w
0001B3AE: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001B3B4: move.w #0xf8,D1w
0001B3B8: move.w #0xe4,D2w
0001B3BC: move.w #0x6000,D6w
0001B3C0: move.w #0x780,D4w
0001B3C4: adda.w #0x8,A2
0001B3C8: jsr 0x000013be.l ; call 000013BE FUN_000013be
0001B3CE: move.w (0x00ffe0ae).l,D0w
0001B3D4: lsl.w #0x2,D0w
0001B3D6: lea (0x1db08).l,A2 ; data 0001db08
0001B3DC: movea.l (0x0,A2,D0w*0x1),A2
0001B3E0: move.w #0x148,D1w
0001B3E4: move.w #0xe4,D2w
0001B3E8: move.w #0x6000,D6w
0001B3EC: move.w #0x780,D4w
0001B3F0: adda.w #0x8,A2
0001B3F4: jsr 0x000013be.l ; call 000013BE FUN_000013be
0001B3FA: move.w (0x00ffe0ae).l,D0w
0001B400: lsl.w #0x2,D0w
0001B402: lea (0xea490).l,A2 ; data 000ea490
0001B408: movea.l (0x0,A2,D0w*0x1),A2
0001B40C: move.w #0x7c0,D0w
0001B410: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001B416: move.w #0xe2,D1w
0001B41A: move.w #0x114,D2w
0001B41E: move.w #0x6000,D6w
0001B422: move.w #0x7c0,D4w
0001B426: adda.w #0x8,A2
0001B42A: jsr 0x000013be.l ; call 000013BE FUN_000013be
0001B430: move.w (0x00ffe0ae).l,D0w
0001B436: lsl.w #0x2,D0w
0001B438: lea (0xea4a4).l,A2 ; data 000ea4a4
0001B43E: movea.l (0x0,A2,D0w*0x1),A2
0001B442: move.w #0x15e,D1w
0001B446: move.w #0x114,D2w
0001B44A: move.w #0x6000,D6w
0001B44E: move.w #0x7c0,D4w
0001B452: adda.w #0x8,A2
0001B456: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001B45C: subq.w #0x1,(0x00ffe0e8).l
0001B462: bpl.w 0x0001b488
0001B466: move.w (0x00ffe0e6).l,(0x00ffe0e8).l
0001B470: subq.b #0x1,(0x00ffe16a).l
0001B476: bne.w 0x0001b488
0001B47A: bclr.b #0x2,(0x12,A0)
0001B480: bsr.w 0x00019f78 ; call 00019F78 FUN_00019f78
0001B484: bra.w 0x0001b5a6 ; call 0001B5A6 FUN_0001b5a6
0001B488: lea (0xffcb42).l,A1 ; data 00ffcb42
0001B48E: move.w #0xaf,D7w
0001B492: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001B498: move.w D0w,D2w
0001B49A: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001B4A0: swap D2
0001B4A2: move.w D0w,D2w
0001B4A4: clr.w D0w
0001B4A6: move.w (A1),D1w
0001B4A8: move.b (0x00ffe16a).l,D4b
0001B4AE: addq.w #0x1,D4w
0001B4B0: ext.w D4w
0001B4B2: lsl.w #0x1,D4w
0001B4B4: lea (0x1b592).l,A2 ; data 0001b592
0001B4BA: move.w (0x0,A2,D4w*0x1),D4w ; data 0001b592
0001B4BE: move.w (A1),D1w
0001B4C0: rol.l #0x1,D2
0001B4C2: bcc.w 0x0001b4cc
0001B4C6: subq.w #0x1,D1w
0001B4C8: bra.w 0x0001b4ce
0001B4CC: addq.w #0x1,D1w
0001B4CE: sub.w D1w,D0w
0001B4D0: bmi.w 0x0001b4e0
0001B4D4: cmp.w D4w,D0w
0001B4D6: blt.w 0x0001b4ea
0001B4DA: addq.w #0x1,D1w
0001B4DC: bra.w 0x0001b4ea
0001B4E0: neg.w D0w
0001B4E2: cmp.w D4w,D0w
0001B4E4: blt.w 0x0001b4ea
0001B4E8: subq.w #0x1,D1w
0001B4EA: move.w D1w,(A1)
0001B4EC: move.w D1w,D0w
0001B4EE: adda.w #0x4,A1
0001B4F2: dbf D7w,0x0001b4be
0001B4F6: lea (0xffcb40).l,A1 ; data 00ffcb40
0001B4FC: move.w #0xc8,D7w
0001B500: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001B506: move.w D0w,D2w
0001B508: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001B50E: swap D2
0001B510: move.w D0w,D2w
0001B512: clr.w D0w
0001B514: move.w (A1),D1w
0001B516: move.b (0x00ffe16a).l,D4b
0001B51C: ext.w D4w
0001B51E: cmpi.w #0x3,D4w
0001B522: blt.w 0x0001b52a
0001B526: move.w #0x2,D4w
0001B52A: lsl.w #0x1,D4w
0001B52C: lea (0x1b592).l,A2 ; data 0001b592
0001B532: move.w (0x0,A2,D4w*0x1),D4w
0001B536: move.w (A1),D1w
0001B538: rol.l #0x1,D2
0001B53A: bcc.w 0x0001b544
0001B53E: subq.w #0x1,D1w
0001B540: bra.w 0x0001b546
0001B544: addq.w #0x1,D1w
0001B546: sub.w D1w,D0w
0001B548: bmi.w 0x0001b558
0001B54C: cmp.w D4w,D0w
0001B54E: blt.w 0x0001b562
0001B552: addq.w #0x1,D1w
0001B554: bra.w 0x0001b562
0001B558: neg.w D0w
0001B55A: cmp.w D4w,D0w
0001B55C: blt.w 0x0001b562
0001B560: subq.w #0x1,D1w
0001B562: move.w D1w,(A1)
0001B564: move.w D1w,D0w
0001B566: adda.w #0x4,A1
0001B56A: dbf D7w,0x0001b536
0001B56E: lea (0xffcb40).l,A0 ; data 00ffcb40
0001B574: move.w #0x1bf,D7w
0001B578: moveq #0x0,D0
0001B57A: jsr 0x00000e18.l ; call 00000E18 FUN_00000e18
0001B580: bset.b #0x3,(0x00ffef14).l
0001B588: bset.b #0x2,(0x00ffef14).l
0001B590: rts
0001B5A6: lea (0xffcb40).l,A0 ; data 00ffcb40
0001B5AC: move.w #0x1bf,D7w
0001B5B0: clr.w (A0)+
0001B5B2: dbf D7w,0x0001b5b0
0001B5B6: bra.b 0x0001b56e
0001B5B8: move.w #0x2b,D0w
0001B5BC: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001B5C2: lea (0xff1580).l,A0 ; data 00ff1580
0001B5C8: bsr.w 0x0001b7d0 ; call 0001B7D0 FUN_0001b7d0
0001B5CC: move.l #0x1c330,(0x8,A0) ; data 0001c330
0001B5D4: move.l #0x1c33a,(0x4,A0) ; data 0001c33a
0001B5DC: rts
0001B5DE: bra.w 0x0001b654 ; call 0001B654 FUN_0001b654
0001B5FC: bsr.w 0x0001b7d0 ; call 0001B7D0 FUN_0001b7d0
0001B600: move.l #0x1c2ae,(0x8,A0) ; data 0001c2ae
0001B608: move.l #0x1c2a8,(0x4,A0) ; data 0001c2a8
0001B610: rts
0001B612: bsr.w 0x0001b7d0 ; call 0001B7D0 FUN_0001b7d0
0001B616: move.l #0x1c294,(0x8,A0) ; data 0001c294
0001B61E: move.l #0x1c29e,(0x4,A0) ; data 0001c29e
0001B626: rts
0001B628: bsr.w 0x0001b7d0 ; call 0001B7D0 FUN_0001b7d0
0001B62C: move.l #0x1c2c8,(0x8,A0) ; data 0001c2c8
0001B634: move.l #0x1c2d2,(0x4,A0) ; data 0001c2d2
0001B63C: rts
0001B654: bsr.w 0x0001b7d0 ; call 0001B7D0 FUN_0001b7d0
0001B658: move.l #0x1c318,(0x8,A0) ; data 0001c318
0001B660: move.l #0x1c31e,(0x4,A0) ; data 0001c31e
0001B668: rts
0001B680: bsr.w 0x0001b7d0 ; call 0001B7D0 FUN_0001b7d0
0001B684: move.l #0x1c344,(0x8,A0) ; data 0001c344
0001B68C: move.l #0x1c34a,(0x4,A0) ; data 0001c34a
0001B694: rts
0001B706: tst.b (0x00ffdf84).l
0001B70C: bne.w 0x0001b732
0001B710: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0001B714: move.b #-0x1,(0x00ffdf84).l
0001B71C: jsr 0x00017e78.l ; call 00017E78 FUN_00017e78
0001B722: move.w D0w,(0x00ffe074).l
0001B728: jsr 0x0001a908.l ; call 0001A908 FUN_0001a908
0001B72E: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0001B732: rts
0001B7D0: lea (0xff1580).l,A0 ; data 00ff1580
0001B7D6: move.b #0x4,(0x2,A0)
0001B7DC: move.b #0x4,(0x3,A0)
0001B7E2: clr.b (0xc,A0)
0001B7E6: clr.b (0xd,A0)
0001B7EA: rts
0001B87A: nop
0001B87C: nop
0001B87E: nop
0001B880: move.w (0x00ffe04e).l,D0w
0001B886: lea (0x1bae4).l,A1 ; data 0001bae4
0001B88C: lsl.w #0x2,D0w
0001B88E: movea.l (0x0,A1,D0w*0x1),A1 ; data 0001bae4
0001B892: jsr (A1)
0001B894: move.w (0x00ffe04a).l,D1w
0001B89A: move.w (0x00ffe04c).l,D2w
0001B8A0: move.w #0x6000,D6w
0001B8A4: move.w (0x00ffe050).l,D0w
0001B8AA: lsl.w #0x2,D0w
0001B8AC: lea (0x1bad4).l,A2 ; data 0001bad4
0001B8B2: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001bad4
0001B8B6: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001B95A: move.w (0x00ffdf88).l,D0w
0001B960: andi.w #0x3,D0w
0001B964: beq.w 0x0001b96a
0001B968: rts
0001B96A: subq.w #0x1,(0x00ffe04a).l
0001B970: cmpi.w #0xb0,(0x00ffe04a).l
0001B978: bcs.w 0x0001b97e
0001B97C: rts
0001B97E: clr.w (0x00ffe04e).l
0001B984: rts
0001B986: addq.w #0x3,(0x00ffe04a).l
0001B98C: cmpi.w #0x120,(0x00ffe04a).l
0001B994: bcc.w 0x0001b99a
0001B998: rts
0001B99A: move.w #0x4,(0x00ffe04e).l
0001B9A2: move.w #0x2a,D0w
0001B9A6: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001B9AC: rts
0001B9AE: subq.w #0x3,(0x00ffe04a).l
0001B9B4: cmpi.w #0xb0,(0x00ffe04a).l
0001B9BC: bcs.w 0x0001b9c2
0001B9C0: rts
0001B9C2: clr.w (0x00ffe04e).l
0001B9C8: rts
0001BA6E: subq.w #0x1,(0x00ffe0b0).l
0001BA74: bpl.w 0x0001ba9a
0001BA78: move.w #0x4,(0x00ffe0b0).l
0001BA80: addq.w #0x1,(0x00ffe0ae).l
0001BA86: cmpi.w #0x4,(0x00ffe0ae).l
0001BA8E: bcs.w 0x0001ba9a
0001BA92: clr.w (0x00ffe0ac).l
0001BA98: rts
0001BA9A: move.w (0x00ffe0ae).l,D0w
0001BAA0: lsl.w #0x2,D0w
0001BAA2: lea (0x1d970).l,A2 ; data 0001d970
0001BAA8: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001d970
0001BAAC: move.w #0x780,D0w
0001BAB0: jsr 0x000061cc.l ; call 000061CC append_sprite_record
0001BAB6: move.w (0x00ffe04a).l,D1w
0001BABC: move.w (0x00ffe04c).l,D2w
0001BAC2: move.w #0x6000,D6w
0001BAC6: move.w #0x780,D4w
0001BACA: adda.w #0x8,A2
0001BACE: jmp 0x000013be.l ; call 000013BE FUN_000013be
0001BB08: move.w (A6)+,(A0)+
0001BB0A: move.b #0x3c,(0x00ffe16d).l
0001BB12: jsr 0x0001a030.l ; call 0001A030 FUN_0001a030
0001BB18: bpl.w 0x0001bb38
0001BB1C: move.w #0x9,D0w
0001BB20: jsr 0x00019b34.l ; call 00019B34 FUN_00019b34
0001BB26: clr.b (0x00ffe18e).l
0001BB2C: bclr.b #0x0,(0x00fff0a1).l
0001BB34: bra.w 0x0001b5b8 ; call 0001B5B8 FUN_0001b5b8
0001BB38: cmpi.w #0x45,D0w
0001BB3C: beq.b 0x0001bb26
0001BB3E: tst.b (0x00fff0aa).l
0001BB44: bgt.w 0x0001bb4e
0001BB48: clr.b (0x00ffe18e).l
0001BB4E: move.w #0x1,(0x00ffe0d6).l
0001BB56: bset.b #0x0,(0x00fff0a1).l
0001BB5E: move.w #0x2d,(0x00ffe0ca).l
0001BB66: bra.w 0x0001b5b8 ; call 0001B5B8 FUN_0001b5b8
0001BB6A: btst.b #0x2,(0x00fff0a1).l
0001BB72: beq.w 0x0001bbf4
0001BB76: subq.w #0x1,(0x00ffe0ca).l
0001BB7C: bpl.w 0x0001bba8
0001BB80: bclr.b #0x2,(0x00fff0a1).l
0001BB88: bclr.b #0x0,(0x00fff0a1).l
0001BB90: bsr.w 0x0001b680 ; call 0001B680 FUN_0001b680
0001BB94: clr.w (0x00ffe0d6).l
0001BB9A: move.w #0x1,(0x00ffe0f4).l
0001BBA2: jmp 0x0001a09c.l ; call 0001A09C FUN_0001a09c
0001BBA8: move.w (0x00ffe0ce).l,D4w
0001BBAE: move.w (0x00ffe0ca).l,D0w
0001BBB4: andi.w #0x7,D0w
0001BBB8: cmpi.w #0x4,D0w
0001BBBC: blt.w 0x0001bc1e
0001BBC0: move.w #0x158,D1w
0001BBC4: move.w #0xfc,D2w
0001BBC8: move.w #0x2000,D6w
0001BBCC: lea (0x1dae0).l,A2 ; data 0001dae0
0001BBD2: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001BBD8: move.w #0xe7,D1w
0001BBDC: move.w #0xfc,D2w
0001BBE0: move.w #0x2000,D6w
0001BBE4: lea (0x1daea).l,A2 ; data 0001daea
0001BBEA: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001BBF0: bra.w 0x0001bc1e
0001BBF4: subq.w #0x1,(0x00ffe0ca).l
0001BBFA: bpl.w 0x0001bc10
0001BBFE: bset.b #0x2,(0x00fff0a1).l
0001BC06: move.w #0x3c,(0x00ffe0ca).l
0001BC0E: rts
0001BC10: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001BC16: move.w D0w,D4w
0001BC18: move.w D4w,(0x00ffe0ce).l
0001BC1E: move.w #0xd8,D1w
0001BC22: move.w #0xec,D2w
0001BC26: move.w #0x6,D7w
0001BC2A: addi.w #0x10,D1w
0001BC2E: move.w #0x2000,D6w
0001BC32: lea (0x1dab8).l,A2 ; data 0001dab8
0001BC38: ror.w #0x1,D4w
0001BC3A: bcc.w 0x0001bc54
0001BC3E: lea (0x1dac2).l,A2 ; data 0001dac2
0001BC44: ror.w #0x1,D4w
0001BC46: bcc.w 0x0001bc60
0001BC4A: lea (0x1dad6).l,A2 ; data 0001dad6
0001BC50: bra.w 0x0001bc60
0001BC54: ror.w #0x1,D4w
0001BC56: bcc.w 0x0001bc60
0001BC5A: lea (0x1dacc).l,A2 ; data 0001dacc
0001BC60: nop
0001BC62: nop
0001BC64: nop
0001BC66: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001BC6C: dbf D7w,0x0001bc2a
0001BC70: rts
0001BCE8: lea (0xff1580).l,A0 ; data 00ff1580
0001BCEE: cmpi.w #0x1,(0x00ffe058).l
0001BCF6: bne.w 0x0001bd8a
0001BCFA: lea (0x1be4e).l,A1 ; data 0001be4e
0001BD00: subq.b #0x1,(0x14,A0)
0001BD04: bpl.w 0x0001bd50
0001BD08: move.b #0x1,(0x14,A0)
0001BD0E: addq.b #0x1,(0x13,A0)
0001BD12: move.b (0x13,A0),D0b
0001BD16: cmpi.b #0xd,D0b
0001BD1A: blt.w 0x0001bd50
0001BD1E: move.w #0x2,(0x00ffe058).l
0001BD26: move.w #0x1340,(0x00ffe056).l
0001BD2E: move.w #0xff0,(0x00ffe054).l
0001BD36: move.w #0xf,(0x00ffe05c).l
0001BD3E: move.w #0x3c,(0x00ffe052).l
0001BD46: clr.w (0x00ffe05a).l
0001BD4C: bra.w 0x0001bd8a
0001BD50: move.b (0x13,A0),D0b
0001BD54: ext.w D0w
0001BD56: lsl.w #0x2,D0w
0001BD58: move.w (0x0,A1,D0w*0x1),D1w ; data 0001be4e
0001BD5C: move.b (0x2,A1,D0w*0x1),D3b ; data 0001be50
0001BD60: ext.w D3w
0001BD62: move.b (0x3,A1,D0w*0x1),D4b ; data 0001be51
0001BD66: ext.w D4w
0001BD68: lea (0x1be3e).l,A2 ; data 0001be3e
0001BD6E: lsl.w #0x2,D1w
0001BD70: movea.l (0x0,A2,D1w*0x1),A2 ; data 0001be3e
0001BD74: move.w (0xe,A0),D1w
0001BD78: move.w (0x10,A0),D2w
0001BD7C: move.w #0x6000,D6w
0001BD80: add.w D3w,D1w
0001BD82: add.w D4w,D2w
0001BD84: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001BD8A: subq.w #0x1,(0x00ffe052).l
0001BD90: bmi.w 0x0001bdb8
0001BD94: subq.w #0x1,(0x00ffe05c).l
0001BD9A: bpl.w 0x0001bddc
0001BD9E: move.w #0xf,(0x00ffe05c).l
0001BDA6: addq.w #0x1,(0x00ffe05a).l
0001BDAC: cmpi.w #0x4,(0x00ffe05a).l
0001BDB4: blt.w 0x0001bddc
0001BDB8: clr.w (0x00ffe058).l
0001BDBE: clr.w (0x00ffe0d6).l
0001BDC4: tst.w (0x00ffe0c8).l
0001BDCA: beq.w 0x0001bdd6
0001BDCE: move.w #0x1,(0x00ffe0f4).l
0001BDD6: jmp 0x0001a106.l ; call 0001A106 FUN_0001a106
0001BDDC: move.w #0x3c,D2w
0001BDE0: move.w (0x00ffe052).l,D1w
0001BDE6: sub.w D1w,D2w
0001BDE8: mulu.w #0x3,D2
0001BDEC: andi.w #0xfe,D2w
0001BDF0: lea (0x15e56).l,A2 ; data 00015e56
0001BDF6: move.w (0x0,A2,D2w*0x1),D2w
0001BDFA: muls.w #0x58,D2
0001BDFE: lsl.l #0x6,D2
0001BE00: swap D2
0001BE02: neg.w D2w
0001BE04: addi.w #0x1340,D2w
0001BE08: move.w D2w,D1w
0001BE0A: move.w D1w,(0x00ffe056).l
0001BE10: subi.w #0xa,(0x00ffe054).l
0001BE18: move.w (0x00ffe054).l,D2w
0001BE1E: lsr.w #0x4,D1w
0001BE20: lsr.w #0x4,D2w
0001BE22: move.w (0x00ffe05a).l,D0w
0001BE28: lsl.w #0x2,D0w
0001BE2A: lea (0x1be3e).l,A2 ; data 0001be3e
0001BE30: movea.l (0x0,A2,D0w*0x1),A2
0001BE34: move.w #0x6000,D6w
0001BE38: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001BEBA: move.w (0x00ffdf88).l,D0w
0001BEC0: andi.w #0x7,D0w
0001BEC4: lsl.w #0x1,D0w
0001BEC6: lea (0x1c10a).l,A0 ; data 0001c10a
0001BECC: adda.w D0w,A0
0001BECE: lea (0xffd880).l,A1 ; data 00ffd880
0001BED4: moveq #0x6,D0
0001BED6: move.w (A0)+,(A1)+ ; data 0001c10a
0001BED8: dbf D0w,0x0001bed6
0001BEDC: bset.b #0x0,(0x00ffef14).l
0001BEE4: lea (0xff1580).l,A0 ; data 00ff1580
0001BEEA: subq.w #0x1,(0x00ffe00c).l
0001BEF0: bmi.w 0x0001bf36
0001BEF4: move.w (0x00ffe00c).l,D0w
0001BEFA: neg.w D0w
0001BEFC: lsr.w #0x1,D0w
0001BEFE: andi.w #0x3,D0w
0001BF02: lsl.w #0x2,D0w
0001BF04: lea (0x1c136).l,A2 ; data 0001c136
0001BF0A: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001c136
0001BF0E: move.w #0xee,D2w
0001BF12: move.w #0x15e,D1w
0001BF16: move.w #0x2000,D6w
0001BF1A: move.l A2,-(SP) ; data Stack[-0x4]
0001BF1C: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001BF22: movea.l (SP)+,A2
0001BF24: move.w #0xee,D2w
0001BF28: move.w #0xe0,D1w
0001BF2C: move.w #0x2000,D6w
0001BF30: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001BF36: bsr.w 0x0001bf5c ; call 0001BF5C FUN_0001bf5c
0001BF3A: lea (0xff1580).l,A0 ; data 00ff1580
0001BF40: bclr.b #0x7,(0x12,A0)
0001BF46: bsr.w 0x0001b612 ; call 0001B612 FUN_0001b612
0001BF4A: clr.w (0x00ffe0d6).l
0001BF50: jsr 0x00019d66.l ; call 00019D66 FUN_00019d66
0001BF56: jmp 0x0001a908.l ; call 0001A908 FUN_0001a908
0001BF5C: lea (0x1c10a).l,A0 ; data 0001c10a
0001BF62: lea (0xffd880).l,A1 ; data 00ffd880
0001BF68: moveq #0x6,D0
0001BF6A: move.w (A0)+,(A1)+
0001BF6C: dbf D0w,0x0001bf6a
0001BF70: bset.b #0x0,(0x00ffef14).l
0001BF78: rts
0001BF7A: subq.w #0x1,(0x00fff11e).l
0001BF80: bpl.w 0x0001bffa
0001BF84: move.w (0x00fff11e).l,D0w
0001BF8A: addq.w #0x8,D0w
0001BF8C: bpl.w 0x0001bfba
0001BF90: lea (0xff1100).l,A6 ; data 00ff1100
0001BF96: move.w (0x00fff118).l,D0w
0001BF9C: mulu.w #0x14,D0
0001BFA0: move.b (0x6,A6,D0w*0x1),D1b ; data 00ff1106
0001BFA4: ext.w D1w
0001BFA6: add.w D1w,D1w
0001BFA8: lea (0x1bffc).l,A0 ; data 0001bffc
0001BFAE: move.w (0x0,A0,D1w*0x1),(0x00fff11e).l
0001BFB6: bra.w 0x0001bffa
0001BFBA: add.w D0w,D0w
0001BFBC: move.w D0w,-(SP) ; data Stack[-0x2]
0001BFBE: clr.l D0
0001BFC0: bsr.w 0x00017c3c ; call 00017C3C FUN_00017c3c
0001BFC4: ble.w 0x0001bfe2
0001BFC8: clr.l D1
0001BFCA: lea (0xff1100).l,A5 ; data 00ff1100
0001BFD0: move.w (0x00fff118).l,D1w
0001BFD6: mulu.w #0x14,D1
0001BFDA: move.b (0x5,A5,D1w*0x1),D0b ; data 00ff1105
0001BFDE: mulu.w #0x20,D0
0001BFE2: lea (0x1c022).l,A0 ; data 0001c022
0001BFE8: adda.l D0,A0
0001BFEA: move.w (SP)+,D0w
0001BFEC: adda.l D0,A0
0001BFEE: bsr.w 0x0001c012 ; call 0001C012 FUN_0001c012
0001BFF2: bset.b #0x0,(0x00ffef14).l
0001BFFA: rts
0001C012: lea (0xffd880).l,A1 ; data 00ffd880
0001C018: moveq #0x6,D0
0001C01A: move.w (A0)+,(A1)+
0001C01C: dbf D0w,0x0001c01a
0001C020: rts
0001C0C2: subq.w #0x1,(0x00ffe0aa).l
0001C0C8: bne.w 0x0001c0d8
0001C0CC: nop
0001C0CE: nop
0001C0D0: nop
0001C0D2: jmp 0x00019de8.l ; call 00019DE8 FUN_00019de8
0001C0D8: lea (0x1c126).l,A0 ; data 0001c126
0001C0DE: move.w (0x00ffe0aa).l,D0w
0001C0E4: btst.l #0x0,D0
0001C0E8: beq.w 0x0001c0f2
0001C0EC: lea (0x1c128).l,A0 ; data 0001c128
0001C0F2: lea (0xffd880).l,A1 ; data 00ffd880
0001C0F8: moveq #0x6,D0
0001C0FA: move.w (A0)+,(A1)+
0001C0FC: dbf D0w,0x0001c0fa
0001C100: bset.b #0x0,(0x00ffef14).l
0001C108: rts
0001C1F6: subq.w #0x1,(0x00ffe0ec).l
0001C1FC: bpl.w 0x0001c228
0001C200: move.w (0x00ffe0ea).l,(0x00ffe0ec).l
0001C20A: subq.b #0x1,(0x00ffe169).l
0001C210: bne.w 0x0001c228
0001C214: move.w #0x28,D0w
0001C218: jsr 0x00053112.l ; call 00053112 FUN_00053112
0001C21E: bclr.b #0x1,(0x12,A0)
0001C224: bsr.w 0x00019f7e ; call 00019F7E FUN_00019f7e
0001C228: rts
0001C3FA: tst.w (0x00ffdf8c).l
0001C400: beq.w 0x0001c414
0001C404: movea.l #0x1d452,A4
0001C40A: movea.l #0x1d428,A2
0001C410: bra.w 0x0001c506 ; call 0001C506 FUN_0001c506
0001C414: lea (0xff1580).l,A0 ; data 00ff1580
0001C41A: movea.l (0x8,A0),A1
0001C41E: move.w (A1)+,D4w
0001C420: movea.l A1,A4
0001C422: cmp.b (0xc,A0),D4b
0001C426: ble.w 0x0001c448
0001C42A: subq.b #0x1,(0x2,A0)
0001C42E: bpl.w 0x0001c448
0001C432: move.b #0x4,(0x2,A0)
0001C438: move.b (0xc,A0),D0b
0001C43C: addq.b #0x1,D0b
0001C43E: cmp.b D0b,D4b
0001C440: blt.w 0x0001c448
0001C444: move.b D0b,(0xc,A0)
0001C448: movea.l (0x4,A0),A1
0001C44C: move.w (A1)+,D4w
0001C44E: cmp.b (0xd,A0),D4b
0001C452: ble.w 0x0001c474
0001C456: subq.b #0x1,(0x3,A0)
0001C45A: bpl.w 0x0001c474
0001C45E: move.b #0x4,(0x3,A0)
0001C464: move.b (0xd,A0),D0b
0001C468: addq.b #0x1,D0b
0001C46A: cmp.b D0b,D4b
0001C46C: blt.w 0x0001c474
0001C470: move.b D0b,(0xd,A0)
0001C474: move.b (0xd,A0),D0b
0001C478: ext.w D0w
0001C47A: lsl.w #0x2,D0w
0001C47C: movea.l (0x0,A1,D0w*0x1),A2
0001C480: movea.l A4,A1
0001C482: move.b (0xc,A0),D0b
0001C486: ext.w D0w
0001C488: lsl.w #0x2,D0w
0001C48A: movea.l (0x0,A1,D0w*0x1),A4
0001C48E: bra.w 0x0001c506 ; call 0001C506 FUN_0001c506
0001C4EE: lea (0xff1580).l,A1 ; data 00ff1580
0001C4F4: lea (0x1c4da).l,A0 ; data 0001c4da
0001C4FA: move.w #0xa,D7w
0001C4FE: move.w (A0)+,(A1)+
0001C500: dbf D7w,0x0001c4fe
0001C504: rts
0001C506: nop
0001C508: nop
0001C50A: nop
0001C50C: nop
0001C50E: lea (0xff1580).l,A1 ; data 00ff1580
0001C514: move.w (0xe,A1),D1w
0001C518: move.w (0x10,A1),D2w
0001C51C: move.w #0x6000,D6w
0001C520: move.l A2,-(SP) ; data Stack[-0x4]
0001C522: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001C528: movea.l (SP)+,A2
0001C52A: btst.b #0x1,(0x12,A1)
0001C530: beq.w 0x0001c580
0001C534: lea (0x1c5ec).l,A6 ; data 0001c5ec
0001C53A: move.b (0x00ffe169).l,D0b
0001C540: ext.w D0w
0001C542: subq.w #0x1,D0w
0001C544: cmpi.w #0x4,D0w
0001C548: blt.w 0x0001c550
0001C54C: move.w #0x3,D0w
0001C550: andi.w #0x3,D0w
0001C554: lsl.w #0x2,D0w
0001C556: movea.l (0x0,A6,D0w*0x1),A6
0001C55A: move.w (0x00ffdf88).l,D0w
0001C560: andi.w #0x3,D0w
0001C564: lsl.w #0x1,D0w
0001C566: move.w (0x0,A6,D0w*0x1),D1w ; data 0001c614
0001C56A: beq.w 0x0001c580
0001C56E: add.w (0xe,A1),D1w
0001C572: move.w (0x10,A1),D2w
0001C576: move.l A2,-(SP) ; data Stack[-0x4]
0001C578: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001C57E: movea.l (SP)+,A2
0001C580: move.w (0xe,A1),D1w
0001C584: move.w (0x10,A1),D2w
0001C588: move.w #0x6000,D6w
0001C58C: movea.l A4,A2
0001C58E: move.l A2,-(SP) ; data Stack[-0x4]
0001C590: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001C596: movea.l (SP)+,A2
0001C598: btst.b #0x1,(0x12,A1)
0001C59E: beq.w 0x0001c5c8
0001C5A2: move.w (0x00ffdf88).l,D0w
0001C5A8: andi.w #0x3,D0w
0001C5AC: lsl.w #0x1,D0w
0001C5AE: move.w (0x0,A6,D0w*0x1),D1w ; data 0001c614
0001C5B2: beq.w 0x0001c5c8
0001C5B6: add.w (0xe,A1),D1w
0001C5BA: move.w (0x10,A1),D2w
0001C5BE: move.l A2,-(SP) ; data Stack[-0x4]
0001C5C0: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001C5C6: movea.l (SP)+,A2
0001C5C8: tst.w (0x00ffe084).l
0001C5CE: beq.w 0x0001c5ea
0001C5D2: move.w (0x00ffdf88).l,D0w
0001C5D8: andi.w #0xf,D0w
0001C5DC: cmpi.w #0x8,D0w
0001C5E0: blt.w 0x0001c5ea
0001C5E4: jsr 0x0001a570.l ; call 0001A570 FUN_0001a570
0001C5EA: rts
0001C61C: lea (0xff1580).l,A0 ; data 00ff1580
0001C622: lea (0x1c6d4).l,A1 ; data 0001c6d4
0001C628: subq.b #0x1,(0x14,A0)
0001C62C: bpl.w 0x0001c66a
0001C630: addq.b #0x1,(0x13,A0)
0001C634: move.b (0x13,A0),D0b
0001C638: move.b D0b,D1b
0001C63A: ext.w D1w
0001C63C: lsl.w #0x2,D1w
0001C63E: move.w (0x2,A1,D1w*0x1),D1w ; data 0001c6d6
0001C642: move.b D1b,(0x14,A0)
0001C646: cmpi.b #0x20,D0b
0001C64A: blt.w 0x0001c66a
0001C64E: bclr.b #0x0,(0x12,A0)
0001C654: bclr.b #0x3,(0x12,A0)
0001C65A: bsr.w 0x0001b612 ; call 0001B612 FUN_0001b612
0001C65E: clr.w (0x00ffe0d6).l
0001C664: jmp 0x00019cb2.l ; call 00019CB2 FUN_00019cb2
0001C66A: move.b (0x13,A0),D0b
0001C66E: ext.w D0w
0001C670: lsl.w #0x2,D0w
0001C672: move.b (0x0,A1,D0w*0x1),D1b ; data 0001c6d4
0001C676: move.b (0x1,A1,D0w*0x1),D3b ; data 0001c6d5
0001C67A: ext.w D3w
0001C67C: lea (0x1c6bc).l,A2 ; data 0001c6bc
0001C682: ext.w D1w
0001C684: lsl.w #0x2,D1w
0001C686: movea.l (0x0,A2,D1w*0x1),A2 ; data 0001c6bc
0001C68A: move.w (0xe,A0),D1w
0001C68E: move.w (0x10,A0),D2w
0001C692: move.w #0x6000,D6w
0001C696: btst.b #0x3,(0x12,A0)
0001C69C: beq.w 0x0001c6a2
0001C6A0: rts
0001C6A2: movem.w {  D3w D2w D1w},-(SP)
0001C6A6: add.w D3w,D1w
0001C6A8: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001C6AE: movem.w (SP)+,{  D1 D2 D3}
0001C6B2: neg.w D3w
0001C6B4: add.w D3w,D1w
0001C6B6: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001C754: lea (0xff1580).l,A0 ; data 00ff1580
0001C75A: tst.w (0x00ffe084).l
0001C760: bne.w 0x0001b5fc
0001C764: btst.b #0x0,(0x12,A0)
0001C76A: beq.w 0x0001c770
0001C76E: rts
0001C770: cmpi.l #0x1c36a,(0x4,A0)
0001C778: beq.w 0x0001c7ac
0001C77C: cmpi.l #0x1c350,(0x8,A0)
0001C784: bne.w 0x0001c7d0
0001C788: cmpi.b #0x3,(0xc,A0)
0001C78E: bne.w 0x0001c7a0
0001C792: cmpi.b #0x3,(0x2,A0)
0001C798: bne.w 0x0001c7a0
0001C79C: bra.w 0x0001cad4 ; call 0001CAD4 FUN_0001cad4
0001C7A0: cmpi.b #0x5,(0xc,A0)
0001C7A6: beq.w 0x0001c7d0
0001C7AA: rts
0001C7AC: cmpi.b #0x3,(0xd,A0)
0001C7B2: bne.w 0x0001c7c4
0001C7B6: cmpi.b #0x3,(0x3,A0)
0001C7BC: bne.w 0x0001c7c4
0001C7C0: bra.w 0x0001cb20 ; call 0001CB20 FUN_0001cb20
0001C7C4: cmpi.b #0x5,(0xd,A0)
0001C7CA: beq.w 0x0001c7d0
0001C7CE: rts
0001C7D0: cmpi.w #0x3,(0x00ffe076).l
0001C7D8: bcc.w 0x0001c810
0001C7DC: btst.b #0xe,(0x00ffdd22).l
0001C7E4: bne.w 0x0001c800
0001C7E8: btst.b #0xc,(0x00ffdd22).l
0001C7F0: bne.w 0x0001c800
0001C7F4: btst.b #0xd,(0x00ffdd22).l
0001C7FC: beq.w 0x0001c810
0001C800: tst.w (0x00ffe082).l
0001C806: bne.w 0x0001c810
0001C80A: jmp 0x00019976.l ; call 00019976 FUN_00019976
0001C810: rts
0001C812: move.l #0x1c282,(0x8,A0)
0001C81A: move.w #0x10c,(0x00ffe060).l
0001C822: clr.b (0xc,A0)
0001C826: move.w #0x16,D0w
0001C82A: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001C830: move.w #0x102,(0x00ffe062).l
0001C838: move.w #-0x1,(0x00ffe05e).l
0001C840: clr.w (0x00ffe064).l
0001C846: move.w #0x3,(0x00ffe066).l
0001C84E: rts
0001C850: move.w #0x134,(0x00ffe060).l
0001C858: move.l #0x1c27c,(0x4,A0)
0001C860: clr.b (0xd,A0)
0001C864: bra.b 0x0001c826
0001C866: lea (0xff1580).l,A0 ; data 00ff1580
0001C86C: subq.w #0x1,(0x00ffe066).l
0001C872: bpl.w 0x0001c8cc
0001C876: move.w #0x3,(0x00ffe066).l
0001C87E: addq.w #0x1,(0x00ffe064).l
0001C884: cmpi.w #0xc,(0x00ffe064).l
0001C88C: blt.w 0x0001c8cc
0001C890: cmpi.w #0x120,(0x00ffe060).l
0001C898: bcc.w 0x0001c8ac
0001C89C: move.l #0x1c288,(0x8,A0)
0001C8A4: clr.b (0xc,A0)
0001C8A8: bra.w 0x0001c8b8
0001C8AC: move.l #0x1c28e,(0x4,A0)
0001C8B4: clr.b (0xd,A0)
0001C8B8: clr.w (0x00ffe0d6).l
0001C8BE: jsr 0x00019e32.l ; call 00019E32 FUN_00019e32
0001C8C4: clr.w (0x00ffe05e).l
0001C8CA: rts
0001C8CC: move.w (0x00ffe064).l,D0w
0001C8D2: lea (0x1c926).l,A2 ; data 0001c926
0001C8D8: tst.w (0x00ffe0e2).l
0001C8DE: bne.w 0x0001c8f8
0001C8E2: cmpi.w #0x120,(0x00ffe060).l
0001C8EA: bcc.w 0x0001c90a
0001C8EE: lea (0x1c956).l,A2 ; data 0001c956
0001C8F4: bra.w 0x0001c90a
0001C8F8: cmpi.w #0x120,(0x00ffe060).l
0001C900: bcs.w 0x0001c90a
0001C904: lea (0x1c956).l,A2 ; data 0001c956
0001C90A: lsl.w #0x2,D0w
0001C90C: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001c956
0001C910: move.w (0x00ffe060).l,D1w
0001C916: move.w (0x00ffe062).l,D2w
0001C91C: move.w #0x2000,D6w
0001C920: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001C9A2: move.w #0x10c0,(0x00ffe06a).l
0001C9AA: move.w #0xa,(0x00ffe06c).l
0001C9B2: move.w #0x15,D0w
0001C9B6: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001C9BC: move.w #0x1020,(0x00ffe068).l
0001C9C4: move.w #-0x1,(0x00ffe06e).l
0001C9CC: move.w #0x1e,(0x00ffe070).l
0001C9D4: clr.w (0x00ffe072).l
0001C9DA: tst.w (0x00ffe0e2).l
0001C9E0: beq.w 0x0001c9fe
0001C9E4: tst.w (0x00ffe06c).l
0001C9EA: bpl.w 0x0001c9f8
0001C9EE: subq.w #0x8,(0x00ffe06c).l
0001C9F4: bra.w 0x0001c9fe
0001C9F8: addq.w #0x8,(0x00ffe06c).l
0001C9FE: rts
0001CA00: move.w #-0xa,(0x00ffe06c).l
0001CA08: move.w #0x1340,(0x00ffe06a).l
0001CA10: bra.b 0x0001c9b2
0001CA12: subq.w #0x1,(0x00ffe070).l
0001CA18: bpl.w 0x0001ca2e
0001CA1C: clr.w (0x00ffe06e).l
0001CA22: clr.w (0x00ffe0d6).l
0001CA28: jmp 0x00019e32.l ; call 00019E32 FUN_00019e32
0001CA2E: addq.w #0x1,(0x00ffe072).l
0001CA34: move.w (0x00ffe072).l,D0w
0001CA3A: lea (0x1ca8c).l,A2 ; data 0001ca8c
0001CA40: move.b (0x0,A2,D0w*0x1),D0b ; data 0001ca8c
0001CA44: ext.w D0w
0001CA46: tst.w (0x00ffe06c).l
0001CA4C: bmi.w 0x0001ca52
0001CA50: addq.w #0x5,D0w
0001CA52: lea (0x1caac).l,A2 ; data 0001caac
0001CA58: lsl.w #0x2,D0w
0001CA5A: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001caac
0001CA5E: move.w (0x00ffe06c).l,D0w
0001CA64: add.w D0w,(0x00ffe06a).l
0001CA6A: addi.w #-0x1a,(0x00ffe068).l
0001CA72: move.w (0x00ffe068).l,D2w
0001CA78: move.w (0x00ffe06a).l,D1w
0001CA7E: lsr.w #0x4,D2w
0001CA80: lsr.w #0x4,D1w
0001CA82: move.w #0x2000,D6w
0001CA86: jmp 0x00001378.l ; call 00001378 FUN_00001378
0001CAD4: btst.b #0x6,(0x00ff1592).l
0001CADC: bne.w 0x0001cd14
0001CAE0: tst.w (0x00ffe010).l
0001CAE6: bne.w 0x0001cafc
0001CAEA: tst.b (0x00fff0b7).l
0001CAF0: bmi.w 0x0001c812
0001CAF4: beq.w 0x0001cafc
0001CAF8: bra.w 0x0001c9a2 ; call 0001C9A2 FUN_0001c9a2
0001CAFC: move.w #-0x20,D5w
0001CB00: move.w #-0x28,D6w
0001CB04: move.w #-0x1,D1w
0001CB08: bsr.w 0x0001cb6a ; call 0001CB6A FUN_0001cb6a
0001CB0C: tst.w (0x00ffe0e2).l
0001CB12: bne.w 0x0001cb18
0001CB16: rts
0001CB18: addi.w #-0x6,(0xe,A1)
0001CB1E: rts
0001CB20: btst.b #0x6,(0x00ff1592).l
0001CB28: bne.w 0x0001cd14
0001CB2C: tst.w (0x00ffe010).l
0001CB32: bne.w 0x0001cb48
0001CB36: tst.b (0x00fff0b7).l
0001CB3C: bmi.w 0x0001c850
0001CB40: beq.w 0x0001cb48
0001CB44: bra.w 0x0001ca00 ; call 0001CA00 FUN_0001ca00
0001CB48: move.w #0x20,D5w
0001CB4C: move.w #-0x28,D6w
0001CB50: clr.w D1w
0001CB52: bsr.w 0x0001cb6a ; call 0001CB6A FUN_0001cb6a
0001CB56: tst.w (0x00ffe0e2).l
0001CB5C: bne.w 0x0001cb62
0001CB60: rts
0001CB62: addi.w #-0x6,(0xe,A1)
0001CB68: rts
0001CB6A: tst.w (0x00ffe010).l
0001CB70: beq.w 0x0001cb7c
0001CB74: bsr.w 0x0001cbf8 ; call 0001CBF8 FUN_0001cbf8
0001CB78: bra.w 0x0001cb82
0001CB7C: lea (0xff159e).l,A1 ; data 00ff159e
0001CB82: tst.b (0x6,A1)
0001CB86: beq.w 0x0001cb8c
0001CB8A: rts
0001CB8C: move.w #0x17,D0w
0001CB90: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001CB96: move.w D1w,(0x12,A1)
0001CB9A: move.b #-0x1,(0x6,A1)
0001CBA0: clr.b (0xa,A1)
0001CBA4: clr.b (0x9,A1)
0001CBA8: clr.b (0x5,A1)
0001CBAC: move.b #0x14,(0x4,A1)
0001CBB2: add.w (0xe,A0),D5w
0001CBB6: add.w (0x10,A0),D6w
0001CBBA: lsl.w #0x4,D5w
0001CBBC: lsl.w #0x4,D6w
0001CBBE: move.w D5w,(0x10,A1)
0001CBC2: move.w D6w,(0x2,A1)
0001CBC6: move.w (0x00ff1596).l,D0w
0001CBCC: lsl.w #0x4,D0w
0001CBCE: move.w (0x00ff1598).l,D1w
0001CBD4: lsl.w #0x4,D1w
0001CBD6: sub.w D5w,D0w
0001CBD8: sub.w D6w,D1w
0001CBDA: move.w D0w,D5w
0001CBDC: ext.l D0
0001CBDE: ext.l D1
0001CBE0: divs.w #0x3c,D0
0001CBE4: move.w D0w,(0xc,A1)
0001CBE8: divs.w #0x3c,D1
0001CBEC: move.w D1w,(0xe,A1)
0001CBF0: move.b #0x3c,(0xb,A1)
0001CBF6: rts
0001CBF8: lea (0xff15b2).l,A1 ; data 00ff15b2
0001CBFE: tst.b (0x6,A1)
0001CC02: beq.w 0x0001cc08
0001CC06: rts
0001CC08: clr.w (0x00ffe010).l
0001CC0E: lea (0xffe012).l,A2 ; data 00ffe012
0001CC14: move.w #0x6,D7w
0001CC18: clr.l (A2)
0001CC1A: adda.w #0x8,A2
0001CC1E: dbf D7w,0x0001cc18
0001CC22: rts
0001CC24: lea (0xff15b2).l,A0 ; data 00ff15b2
0001CC2A: tst.b (0x6,A0)
0001CC2E: bne.w 0x0001cc34
0001CC32: rts
0001CC34: bsr.w 0x0001d048 ; call 0001D048 FUN_0001d048
0001CC38: bsr.w 0x0001d0de ; call 0001D0DE FUN_0001d0de
0001CC3C: move.b (0x8,A0),D0b
0001CC40: move.w (0x00ffdf88).l,D6w
0001CC46: andi.w #0x3,D6w
0001CC4A: bne.w 0x0001cc50
0001CC4E: addq.b #0x1,D0b
0001CC50: cmpi.b #0x6,D0b
0001CC54: bcs.w 0x0001cc5a
0001CC58: clr.b D0b
0001CC5A: lea (0xffe012).l,A1 ; data 00ffe012
0001CC60: move.b D0b,(0x8,A0)
0001CC64: ext.w D0w
0001CC66: lsl.w #0x3,D0w
0001CC68: adda.w D0w,A1
0001CC6A: move.w D1w,(A1)
0001CC6C: move.l A2,(0x2,A1)
0001CC70: move.w D2w,(0x6,A1)
0001CC74: move.w #0x6,D7w
0001CC78: move.w #0x6000,D6w
0001CC7C: adda.w #0x8,A1
0001CC80: nop
0001CC82: nop
0001CC84: nop
0001CC86: nop
0001CC88: nop
0001CC8A: nop
0001CC8C: nop
0001CC8E: nop
0001CC90: nop
0001CC92: move.w (A1),D1w
0001CC94: beq.w 0x0001ccae
0001CC98: movea.l (0x2,A1),A2
0001CC9C: move.w (0x6,A1),D2w
0001CCA0: movem.w {  D7w D6w D0w},-(SP)
0001CCA4: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001CCAA: movem.w (SP)+,{ D0 D6 D7}
0001CCAE: addq.w #0x8,D0w
0001CCB0: cmpi.w #0x30,D0w
0001CCB4: bcs.w 0x0001ccba
0001CCB8: clr.w D0w
0001CCBA: lea (0xffe012).l,A1 ; data 00ffe012
0001CCC0: adda.w D0w,A1
0001CCC2: dbf D7w,0x0001cc8a
0001CCC6: tst.b (0x6,A0)
0001CCCA: bne.w 0x0001ccda
0001CCCE: clr.w (0x00ffe0d6).l
0001CCD4: jsr 0x00019ea6.l ; call 00019EA6 FUN_00019ea6
0001CCDA: rts
0001CCDC: lea (0xff159e).l,A0 ; data 00ff159e
0001CCE2: tst.b (0x6,A0)
0001CCE6: bne.w 0x0001ccec
0001CCEA: rts
0001CCEC: bsr.w 0x0001d048 ; call 0001D048 FUN_0001d048
0001CCF0: bsr.w 0x0001d0de ; call 0001D0DE FUN_0001d0de
0001CCF4: move.w #0x6000,D6w
0001CCF8: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001CCFE: tst.b (0x6,A0)
0001CD02: bne.w 0x0001cd12
0001CD06: clr.w (0x00ffe0d6).l
0001CD0C: jsr 0x00019e32.l ; call 00019E32 FUN_00019e32
0001CD12: rts
0001CD14: lea (0xff15c6).l,A1 ; data 00ff15c6
0001CD1A: tst.b (0x6,A1)
0001CD1E: beq.w 0x0001cd24
0001CD22: rts
0001CD24: move.b #-0x1,(0x6,A1)
0001CD2A: clr.b (0xa,A1)
0001CD2E: clr.b (0x9,A1)
0001CD32: clr.b (0x5,A1)
0001CD36: move.b #0x14,(0x4,A1)
0001CD3C: move.b #0x3c,(0xb,A1)
0001CD42: move.w #0x1400,(A1)
0001CD46: move.w #0x1070,(0x2,A1)
0001CD4C: move.w #-0x8,(0xc,A1)
0001CD52: move.w #-0xe,(0xe,A1)
0001CD58: rts
0001CD5A: lea (0xff15c6).l,A0 ; data 00ff15c6
0001CD60: tst.b (0x6,A0)
0001CD64: bne.w 0x0001cd6a
0001CD68: rts
0001CD6A: subq.b #0x1,(0xb,A0)
0001CD6E: bpl.w 0x0001cd88
0001CD72: clr.b (0x6,A0)
0001CD76: tst.w (0x00ffe0a8).l
0001CD7C: beq.w 0x0001cd88
0001CD80: move.w #0xa,(0x00ffe0aa).l
0001CD88: subq.b #0x1,(0x4,A0)
0001CD8C: bpl.w 0x0001cda8
0001CD90: move.b #0x14,(0x4,A0)
0001CD96: move.b (0x5,A0),D0b
0001CD9A: addq.b #0x1,D0b
0001CD9C: cmpi.b #0x3,D0b
0001CDA0: bge.w 0x0001cda8
0001CDA4: move.b D0b,(0x5,A0)
0001CDA8: subq.b #0x1,(0xa,A0)
0001CDAC: bpl.w 0x0001cdc8
0001CDB0: move.b #0x2,(0xa,A0)
0001CDB6: addq.b #0x1,(0x9,A0)
0001CDBA: cmpi.b #0x3,(0x9,A0)
0001CDC0: blt.w 0x0001cdc8
0001CDC4: clr.b (0x9,A0)
0001CDC8: move.b (0x9,A0),D0b
0001CDCC: ext.w D0w
0001CDCE: lsl.w #0x2,D0w
0001CDD0: lea (0x1d158).l,A2 ; data 0001d158
0001CDD6: cmpi.b #0x0,(0x5,A0)
0001CDDC: beq.w 0x0001cdf6
0001CDE0: lea (0x1d140).l,A2 ; data 0001d140
0001CDE6: cmpi.b #0x1,(0x5,A0)
0001CDEC: beq.w 0x0001cdf6
0001CDF0: lea (0x1d14c).l,A2 ; data 0001d14c
0001CDF6: movea.l (0x0,A2,D0w*0x1),A2
0001CDFA: addi.w #-0x8,(A0)
0001CDFE: addi.w #-0xe,(0x2,A0)
0001CE04: move.w (A0),D1w
0001CE06: move.w (0x2,A0),D2w
0001CE0A: lsr.w #0x4,D1w
0001CE0C: lsr.w #0x4,D2w
0001CE0E: move.w #0x2000,D6w
0001CE12: move.l A2,-(SP) ; data 0001d952
0001CE14: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001CE1A: movea.l (SP)+,A2
0001CE1C: tst.b (0x6,A0)
0001CE20: bne.w 0x0001ce38
0001CE24: bclr.b #0x6,(0x00ff1592).l
0001CE2C: clr.w (0x00ffe0d6).l
0001CE32: jmp 0x00019f18.l ; call 00019F18 FUN_00019f18
0001CE38: tst.b (0x4,A0)
0001CE3C: bne.w 0x0001cede
0001CE40: tst.b (0x5,A0)
0001CE44: bne.w 0x0001ce68
0001CE48: move.w (A0),(0x00ffe088).l
0001CE4E: move.w (A0),(0x00ffe08a).l
0001CE54: move.w (0x2,A0),(0x00ffe08c).l
0001CE5C: move.w (0x2,A0),(0x00ffe08e).l
0001CE64: bra.w 0x0001cede
0001CE68: cmpi.b #0x1,(0x5,A0)
0001CE6E: bne.w 0x0001cede
0001CE72: move.w (A0),(0x00ffe090).l
0001CE78: move.w (0x2,A0),(0x00ffe092).l
0001CE80: move.w (A0),(0x00ffe094).l
0001CE86: move.w (0x2,A0),(0x00ffe096).l
0001CE8E: move.w (0x00ffe088).l,(0x00ffe098).l
0001CE98: move.w (0x00ffe08c).l,(0x00ffe09a).l
0001CEA2: move.w (0x00ffe088).l,(0x00ffe09c).l
0001CEAC: move.w (0x00ffe08c).l,(0x00ffe09e).l
0001CEB6: move.w (0x00ffe08a).l,(0x00ffe0a0).l
0001CEC0: move.w (0x00ffe08e).l,(0x00ffe0a2).l
0001CECA: move.w (0x00ffe08a).l,(0x00ffe0a4).l
0001CED4: move.w (0x00ffe08e).l,(0x00ffe0a6).l
0001CEDE: tst.b (0x5,A0)
0001CEE2: beq.w 0x0001cef8
0001CEE6: cmpi.b #0x1,(0x5,A0)
0001CEEC: beq.w 0x0001cef4
0001CEF0: bsr.w 0x0001cf4a ; call 0001CF4A FUN_0001cf4a
0001CEF4: bsr.w 0x0001cefa ; call 0001CEFA FUN_0001cefa
0001CEF8: rts
0001CEFA: addi.w #0x9,(0x00ffe088).l
0001CF02: subi.w #0x1a,(0x00ffe08a).l
0001CF0A: addi.w #-0xe,(0x00ffe08c).l
0001CF12: addi.w #-0xe,(0x00ffe08e).l
0001CF1A: move.w (0x00ffe088).l,D1w
0001CF20: move.w (0x00ffe08c).l,D2w
0001CF26: bsr.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001CF2A: move.w (0x00ffe08a).l,D1w
0001CF30: move.w (0x00ffe08e).l,D2w
0001CF36: lsr.w #0x4,D1w
0001CF38: lsr.w #0x4,D2w
0001CF3A: move.w #0x2000,D6w
0001CF3E: move.l A2,-(SP) ; data Stack[-0x4]
0001CF40: jsr 0x00001378.l ; call 00001378 FUN_00001378
0001CF46: movea.l (SP)+,A2
0001CF48: rts
0001CF4A: addi.w #0x9,(0x00ffe098).l
0001CF52: addi.w #0x9,(0x00ffe09c).l
0001CF5A: subi.w #0x1a,(0x00ffe0a0).l
0001CF62: subi.w #0x1a,(0x00ffe0a4).l
0001CF6A: addi.w #-0x8,(0x00ffe090).l
0001CF72: addi.w #-0x8,(0x00ffe094).l
0001CF7A: tst.w (0x00ffe0a8).l
0001CF80: beq.w 0x0001cfb8
0001CF84: addi.w #-0x18,(0x00ffe092).l
0001CF8C: addi.w #-0x4,(0x00ffe096).l
0001CF94: addi.w #-0x18,(0x00ffe09a).l
0001CF9C: addi.w #-0x4,(0x00ffe09e).l
0001CFA4: addi.w #-0x18,(0x00ffe0a2).l
0001CFAC: addi.w #-0x4,(0x00ffe0a6).l
0001CFB4: bra.w 0x0001cfe8
0001CFB8: addi.w #-0x36,(0x00ffe092).l
0001CFC0: addi.w #0x1a,(0x00ffe096).l
0001CFC8: addi.w #-0x36,(0x00ffe09a).l
0001CFD0: addi.w #0x1a,(0x00ffe09e).l
0001CFD8: addi.w #-0x36,(0x00ffe0a2).l
0001CFE0: addi.w #0x1a,(0x00ffe0a6).l
0001CFE8: move.w (0x00ffe090).l,D1w
0001CFEE: move.w (0x00ffe092).l,D2w
0001CFF4: bsr.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001CFF8: move.w (0x00ffe094).l,D1w
0001CFFE: move.w (0x00ffe096).l,D2w
0001D004: bsr.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001D008: move.w (0x00ffe098).l,D1w
0001D00E: move.w (0x00ffe09a).l,D2w
0001D014: bsr.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001D018: move.w (0x00ffe09c).l,D1w
0001D01E: move.w (0x00ffe09e).l,D2w
0001D024: bsr.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001D028: move.w (0x00ffe0a0).l,D1w
0001D02E: move.w (0x00ffe0a2).l,D2w
0001D034: bsr.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001D038: move.w (0x00ffe0a4).l,D1w
0001D03E: move.w (0x00ffe0a6).l,D2w
0001D044: bra.w 0x0001cf36 ; call 0001CF36 FUN_0001cf36
0001D048: subq.b #0x1,(0xb,A0)
0001D04C: bpl.w 0x0001d054
0001D050: clr.b (0x6,A0)
0001D054: subq.b #0x1,(0x4,A0)
0001D058: bpl.w 0x0001d074
0001D05C: move.b #0x14,(0x4,A0)
0001D062: move.b (0x5,A0),D0b
0001D066: addq.b #0x1,D0b
0001D068: cmpi.b #0x3,D0b
0001D06C: bge.w 0x0001d074
0001D070: move.b D0b,(0x5,A0)
0001D074: move.w (0x10,A0),D0w
0001D078: add.w (0xc,A0),D0w
0001D07C: move.w D0w,(0x10,A0)
0001D080: move.w #0x3c,D2w
0001D084: clr.w D1w
0001D086: move.b (0xb,A0),D1b
0001D08A: sub.w D1w,D2w
0001D08C: lsl.w #0x2,D2w
0001D08E: andi.w #0xff,D2w
0001D092: lea (0x15e56).l,A2 ; data 00015e56
0001D098: move.w (0x0,A2,D2w*0x1),D2w ; data 00015e56
0001D09C: muls.w D1w,D2
0001D09E: lsl.l #0x6,D2
0001D0A0: swap D2
0001D0A2: tst.w (0x12,A0)
0001D0A6: bne.w 0x0001d0ac
0001D0AA: neg.w D2w
0001D0AC: add.w D2w,D0w
0001D0AE: move.w D0w,(A0)
0001D0B0: move.w (0x2,A0),D0w
0001D0B4: add.w (0xe,A0),D0w
0001D0B8: move.w D0w,(0x2,A0)
0001D0BC: subq.b #0x1,(0xa,A0)
0001D0C0: bpl.w 0x0001d0dc
0001D0C4: move.b #0x2,(0xa,A0)
0001D0CA: addq.b #0x1,(0x9,A0)
0001D0CE: cmpi.b #0x3,(0x9,A0)
0001D0D4: blt.w 0x0001d0dc
0001D0D8: clr.b (0x9,A0)
0001D0DC: rts
0001D0DE: move.b (0x9,A0),D0b
0001D0E2: ext.w D0w
0001D0E4: lsl.w #0x2,D0w
0001D0E6: lea (0x1d134).l,A2 ; data 0001d134
0001D0EC: cmpi.b #0x0,(0x5,A0)
0001D0F2: beq.w 0x0001d10c
0001D0F6: lea (0x1d11c).l,A2 ; data 0001d11c
0001D0FC: cmpi.b #0x1,(0x5,A0)
0001D102: beq.w 0x0001d10c
0001D106: lea (0x1d128).l,A2 ; data 0001d128
0001D10C: movea.l (0x0,A2,D0w*0x1),A2 ; data 0001d128
0001D110: move.w (A0),D1w
0001D112: move.w (0x2,A0),D2w
0001D116: lsr.w #0x4,D1w
0001D118: lsr.w #0x4,D2w
0001D11A: rts
0001D184: bsr.w 0x0001d1b4 ; call 0001D1B4 FUN_0001d1b4
0001D188: bra.w 0x0001d1c4 ; call 0001D1C4 FUN_0001d1c4
0001D18C: bsr.w 0x0001d1b4 ; call 0001D1B4 FUN_0001d1b4
0001D190: bra.w 0x0001d1d0 ; call 0001D1D0 FUN_0001d1d0
0001D194: bsr.w 0x0001d1a4 ; call 0001D1A4 FUN_0001d1a4
0001D198: bra.w 0x0001d1c4 ; call 0001D1C4 FUN_0001d1c4
0001D19C: bsr.w 0x0001d1a4 ; call 0001D1A4 FUN_0001d1a4
0001D1A0: bra.w 0x0001d1d0 ; call 0001D1D0 FUN_0001d1d0
0001D1A4: cmpi.w #0xf4,(0x2,A0)
0001D1AA: bcc.w 0x0001d1b2
0001D1AE: addq.w #0x2,(0x2,A0)
0001D1B2: rts
0001D1B4: cmpi.w #0xb0,(0x2,A0)
0001D1BA: bcs.w 0x0001d1c2
0001D1BE: subq.w #0x2,(0x2,A0)
0001D1C2: rts
0001D1C4: cmpi.w #0x198,(A0)
0001D1C8: bcc.w 0x0001d1ce
0001D1CC: addq.w #0x2,(A0)
0001D1CE: rts
0001D1D0: cmpi.w #0xa8,(A0)
0001D1D4: bcs.w 0x0001d1da
0001D1D8: subq.w #0x2,(A0)
0001D1DA: rts
0001D1DC: move.l (0x00ffdf7e).l,D0
0001D1E2: move.w (0x00ffdf76).l,D1w
0001D1E8: move.w (0x00ffe006).l,D2w
0001D1EE: jmp 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0001D1F4: subq.w #0x1,(0x00ffe008).l
0001D1FA: bpl.w 0x0001d216
0001D1FE: bclr.b #0x4,(0x12,A0)
0001D204: bsr.b 0x0001d1dc ; call 0001D1DC FUN_0001d1dc
0001D206: tst.b (0x00ffe16b).l
0001D20C: bne.w 0x0001d280
0001D210: jmp 0x0001a01e.l ; call 0001A01E FUN_0001a01e
0001D216: move.w (0x00ffdf88).l,D3w
0001D21C: andi.w #0x3,D3w
0001D220: bne.w 0x0001d226
0001D224: bsr.b 0x0001d1dc ; call 0001D1DC FUN_0001d1dc
0001D226: lea (0xc00000).l,A4 ; data 00c00000
0001D22C: move.w (0x00ffdf76).l,D0w
0001D232: movea.l (0x00ffdf7e).l,A1
0001D238: move.w #0x46,D6w
0001D23C: move.w (0x00ffe006).l,D1w
0001D242: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001D248: andi.w #-0x4,D0w
0001D24C: move.w D0w,D3w
0001D24E: add.w (0x00ffdf76).l,D0w
0001D254: move SR,-(SP) ; data Stack[-0x2]
0001D256: move #0x2700,SR
0001D25A: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0001D260: move (SP)+,SR
0001D262: move.w #0x5,D0w
0001D266: bsr.w 0x0001d3f4 ; call 0001D3F4 FUN_0001d3f4
0001D26A: move.l (0x0,A1,D3w*0x1),D1
0001D26E: nop
0001D270: nop
0001D272: nop
0001D274: nop
0001D276: and.l D1,D0
0001D278: move.l D0,(A4)
0001D27A: dbf D6w,0x0001d23c
0001D27E: rts
0001D280: clr.w (0x00ffe0ae).l
0001D286: move.w #0x4,(0x00ffe0b0).l
0001D28E: move.w #0x1,(0x00ffdf8c).l
0001D296: move.w #0x70,(0x00ffdf76).l
0001D29E: move.w #0xaf,(0x00ffdf78).l
0001D2A6: move.w #0xaf,(0x00ffdf72).l
0001D2AE: move.l #0x1d428,(0x00ffdf7a).l
0001D2B8: move.l #0xda038,(0x00ffdf7e).l
0001D2C2: move.w #-0x7ee5,(0x00ffdf74).l
0001D2CA: bra.w 0x0001d2ce
0001D2CE: move.w (0x00ffdf76).l,D0w
0001D2D4: subi.w #0x1f,D0w
0001D2D8: move.w D0w,(0x00ffdf70).l
0001D2DE: clr.w (0x00ffdf82).l
0001D2E4: rts
0001D2E6: movea.l (0x00ffdf7a).l,A2
0001D2EC: movea.l (0x00ffdf7e).l,A1
0001D2F2: move.w (A2),D0w
0001D2F4: cmpi.w #-0x8000,D0w
0001D2F8: beq.w 0x0001d306
0001D2FC: bsr.w 0x0001d33a ; call 0001D33A FUN_0001d33a
0001D300: adda.w #0x8,A2
0001D304: bra.b 0x0001d2f2
0001D306: subq.w #0x1,(0x00ffdf82).l
0001D30C: bpl.w 0x0001d31e
0001D310: move.w #0x0,(0x00ffdf82).l
0001D318: addq.w #0x1,(0x00ffdf70).l
0001D31E: move.w (0x00ffdf70).l,D0w
0001D324: cmp.w (0x00ffdf78).l,D0w
0001D32A: bgt.w 0x0001d334
0001D32E: andi #0x1e,CCR
0001D332: rts
0001D334: ori #0x1,CCR
0001D338: rts
0001D33A: move.w (0x2,A2),D5w
0001D33E: add.w (0x00ffdf72).l,D5w
0001D344: move.w (0x4,A2),D6w
0001D348: andi.w #0x3,D6w
0001D34C: addq.w #0x1,D6w
0001D34E: lsl.w #0x3,D6w
0001D350: move.w #0x1f,D7w
0001D354: move.w (0x00ffdf70).l,D2w
0001D35A: move.w D2w,D4w
0001D35C: sub.w D5w,D4w
0001D35E: bcs.w 0x0001d3ec
0001D362: cmp.w D4w,D6w
0001D364: beq.w 0x0001d3ec
0001D368: bcs.w 0x0001d3ec
0001D36C: move.w (0x6,A2),D0w
0001D370: andi.w #0x7ff,D0w
0001D374: move.w D0w,D3w
0001D376: lsl.w #0x5,D0w
0001D378: sub.w (0x00ffdf74).l,D3w
0001D37E: lsl.w #0x5,D3w
0001D380: movea.l A1,A0
0001D382: adda.w D3w,A0
0001D384: move.w (0x4,A2),D1w
0001D388: lsr.w #0x2,D1w
0001D38A: andi.w #0x3,D1w
0001D38E: move.w D1w,-(SP) ; data Stack[-0x2]
0001D390: addq.w #0x1,D1w
0001D392: lsl.w #0x2,D1w
0001D394: mulu.w D6w,D1
0001D396: movea.l A0,A6
0001D398: adda.w D1w,A6
0001D39A: move.w (SP)+,D1w
0001D39C: move.w D6w,D3w
0001D39E: lsl.w #0x2,D3w
0001D3A0: lsl.w #0x2,D4w
0001D3A2: adda.w D4w,A0
0001D3A4: add.w D4w,D0w
0001D3A6: lea (0xc00000).l,A4 ; data 00c00000
0001D3AC: move.w D0w,-(SP) ; data Stack[-0x2]
0001D3AE: move SR,-(SP) ; data Stack[-0x4]
0001D3B0: move #0x2700,SR
0001D3B4: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0001D3BA: move (SP)+,SR
0001D3BC: move.w D7w,D0w
0001D3BE: lsr.w #0x2,D0w
0001D3C0: subq.w #0x7,D0w
0001D3C2: neg.w D0w
0001D3C4: movem.w {  D7w D1w},-(SP)
0001D3C8: bsr.w 0x0001d3f4 ; call 0001D3F4 FUN_0001d3f4
0001D3CC: nop
0001D3CE: nop
0001D3D0: nop
0001D3D2: move.l (A0),D1
0001D3D4: and.l D1,D0
0001D3D6: move.l D0,(A4)
0001D3D8: adda.w D3w,A0
0001D3DA: movem.w (SP)+,{  D1 D7}
0001D3DE: move.w (SP)+,D0w
0001D3E0: add.w D3w,D0w
0001D3E2: cmpa.l A6,A0
0001D3E4: bcc.w 0x0001d3ec
0001D3E8: dbf D1w,0x0001d3ac
0001D3EC: addq.w #0x1,D2w
0001D3EE: dbf D7w,0x0001d35a
0001D3F2: rts
0001D3F4: lea (0xf1c5e).l,A5 ; data 000f1c5e
0001D3FA: lsl.w #0x2,D0w
0001D3FC: movea.l (0x0,A5,D0w*0x1),A5 ; data 000f1c5e
0001D400: clr.w D1w
0001D402: move.b (A5)+,D1b
0001D404: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001D40A: move.b (0x0,A5,D0w*0x1),D1b
0001D40E: clr.l D0
0001D410: move.w #0x7,D7w
0001D414: lsl.l #0x4,D0
0001D416: rol.b #0x1,D1b
0001D418: bcc.w 0x0001d422
0001D41C: ori.l #0xf,D0
0001D422: dbf D7w,0x0001d414
0001D426: rts
0001DBAA: cmpi.w #0x36,(0x00ffddc4).l
0001DBB2: bne.w 0x0001dbc2
0001DBB6: tst.b (0x00ffe177).l
0001DBBC: bne.w 0x0001dbc2
0001DBC0: rts
0001DBC2: tst.w (0x00ffdf84).l
0001DBC8: bne.w 0x0001dbd4
0001DBCC: move.w #-0x1,(0x00ffdf84).l
0001DBD4: move.w #0x78,(0x00ffdf86).l
0001DBDC: tst.l (0x00ffe162).l
0001DBE2: beq.w 0x0001dbee
0001DBE6: move.w #0x258,(0x00ffdf86).l
0001DBEE: rts
0001DBF0: clr.w (0x00ffdf84).l
0001DBF6: lea (0xff0100).l,A1 ; data 00ff0100
0001DBFC: move.w #0x9,D7w
0001DC00: clr.l (0xa2,A1)
0001DC04: clr.l (0xac,A1)
0001DC08: clr.b (0x43,A1)
0001DC0C: andi.b #-0x41,(0xb0,A1)
0001DC12: andi.b #-0x11,(0x11,A1)
0001DC18: adda.w #0x100,A1
0001DC1C: dbf D7w,0x0001dc00
0001DC20: rts
0001DC22: move.w #0x9,D7w
0001DC26: lea (0xff0100).l,A0 ; data 00ff0100
0001DC2C: btst.b #0x4,(0x11,A0)
0001DC32: bne.w 0x0001dc40
0001DC36: adda.w #0x100,A0
0001DC3A: dbf D7w,0x0001dc2c
0001DC3E: rts
0001DC40: movea.l (0x00fffc04).l,A0
0001DC46: move.l #0x678cc,D1
0001DC4C: tst.b (0x56,A0)
0001DC50: bpl.w 0x0001dc70
0001DC54: move.b (0x57,A0),D0b
0001DC58: cmpi.b #0xc,D0b
0001DC5C: beq.w 0x0001dd2e
0001DC60: cmpi.b #0x4,D0b
0001DC64: beq.w 0x0001dd2e
0001DC68: cmpi.b #0xb,D0b
0001DC6C: beq.w 0x0001dd2e
0001DC70: move.w #0x7,D7w
0001DC74: move.l (0xa2,A0),D0
0001DC78: bne.w 0x0001dc8e
0001DC7C: move.l (0xac,A0),D0
0001DC80: beq.w 0x0001dcd4
0001DC84: movea.l D0,A2
0001DC86: move.w (0x3e,A2),D0w
0001DC8A: bra.w 0x0001dcda
0001DC8E: movea.l D0,A2
0001DC90: bclr.b #0x4,(0x11,A2)
0001DC96: move.l #0x0,(0xa2,A0)
0001DC9E: move.l A2,D0
0001DCA0: cmpi.l #0xff0a00,D0
0001DCA6: bcc.w 0x0001dcd4
0001DCAA: cmpi.l #0x678cc,D1
0001DCB0: beq.w 0x0001dcb8
0001DCB4: adda.w #0x100,A2
0001DCB8: cmpi.w #0x3,(0x3e,A2)
0001DCBE: bge.w 0x0001dcc8
0001DCC2: lea (0xff0400).l,A2 ; data 00ff0400
0001DCC8: move.l A2,D0 ; data 00ff0400
0001DCCA: cmpi.l #0xff0b00,D0
0001DCD0: bcs.w 0x0001dcda
0001DCD4: lea (0xff0400).l,A2 ; data 00ff0400
0001DCDA: btst.b #0x0,(0x10,A2)
0001DCE0: beq.w 0x0001dd0a
0001DCE4: btst.b #0x6,(0x10,A2)
0001DCEA: bne.w 0x0001dd0a
0001DCEE: btst.b #0x4,(0x10,A2)
0001DCF4: bne.w 0x0001dd0a
0001DCF8: btst.b #0x7,(0xb0,A2)
0001DCFE: bne.w 0x0001dd0a
0001DD02: tst.w (0xc,A2)
0001DD06: bne.w 0x0001dd14
0001DD0A: adda.w #0x100,A2
0001DD0E: dbf D7w,0x0001dcb8
0001DD12: rts
0001DD14: clr.b (0x43,A0)
0001DD18: bset.b #0x4,(0x11,A2)
0001DD1E: move.l A2,(0xa2,A0) ; data 00ff0400
0001DD22: move.l A0,(0xac,A2)
0001DD26: move.w #-0x7ff7,(0xa6,A2)
0001DD2C: rts
0001DD2E: movea.l (0x00fffc04).l,A2
0001DD34: move.l (0xa2,A0),D0
0001DD38: beq.w 0x0001dd4c
0001DD3C: movea.l D0,A2
0001DD3E: bclr.b #0x4,(0x11,A2)
0001DD44: move.l #0x0,(0xa2,A0)
0001DD4C: cmpi.l #0x678cc,D1
0001DD52: bne.w 0x0001dd5c
0001DD56: clr.w D1w
0001DD58: bra.w 0x0001dd60
0001DD5C: adda.w #0x100,A2
0001DD60: move.w (0x3e,A2),D0w
0001DD64: cmpi.w #0x3,D0w
0001DD68: blt.w 0x0001dd72
0001DD6C: lea (0xff0100).l,A2 ; data 00ff0100
0001DD72: btst.b #0x0,(0x10,A2)
0001DD78: bne.w 0x0001dd82
0001DD7C: dbf D7w,0x0001dd4c
0001DD80: rts
0001DD82: bset.b #0x4,(0x11,A2)
0001DD88: move.l A2,(0xa2,A0) ; data 00ff0100
0001DD8C: move.w #-0x7ff7,(0xa6,A2)
0001DD92: rts
0001DD94: cmpi.b #0x2,(0x00ffe186).l
0001DD9C: bne.w 0x0001ddc6
0001DDA0: tst.w (0x00fff140).l
0001DDA6: bpl.w 0x0001ddc6
0001DDAA: move.b (0x00ff044c).l,D0b
0001DDB0: cmpi.b #0x9,D0b
0001DDB4: bgt.w 0x0001ddc6
0001DDB8: clr.w (0x00fff140).l
0001DDBE: move.b #0x3,(0x00ffe186).l
0001DDC6: tst.w (0x00ffe166).l
0001DDCC: beq.w 0x0001ddf0
0001DDD0: subq.w #0x1,(0x00ffe166).l
0001DDD6: bne.w 0x0001ddf0
0001DDDA: jsr 0x00053126.l ; call 00053126 FUN_00053126
0001DDE0: move.l #0xc64c,(0x00ffdd2c).l
0001DDEA: jsr 0x0001e2ec.l ; call 0001E2EC FUN_0001e2ec
0001DDF0: tst.w (0x00ffe11c).l
0001DDF6: beq.w 0x0001de00
0001DDFA: subq.w #0x1,(0x00ffe11c).l
0001DE00: tst.w (0x00ffdf84).l
0001DE06: beq.w 0x0001de7c
0001DE0A: tst.b (0x00ffe186).l
0001DE10: bne.w 0x0001de3e
0001DE14: tst.b (0x00ffe16f).l
0001DE1A: bne.w 0x0001de3e
0001DE1E: cmpi.w #0x14,(0x00ffddc4).l
0001DE26: beq.w 0x0001de3e
0001DE2A: subq.w #0x1,(0x00ffdf86).l
0001DE30: bpl.w 0x0001de3e
0001DE34: jsr 0x0001dbf0.l ; call 0001DBF0 FUN_0001dbf0
0001DE3A: bra.w 0x0001de7c
0001DE3E: movea.l (0x00fffc04).l,A0
0001DE44: move.l (0xa2,A0),D0
0001DE48: bne.w 0x0001de56
0001DE4C: jsr 0x0001dc4c.l ; call 0001DC4C FUN_0001dc4c
0001DE52: bra.w 0x0001de7c
0001DE56: movea.l D0,A2
0001DE58: btst.b #0x0,(0x10,A2)
0001DE5E: beq.b 0x0001de4c
0001DE60: tst.b (0x56,A0)
0001DE64: bpl.w 0x0001de74
0001DE68: move.b (0x57,A0),D0b
0001DE6C: cmpi.b #0xc,D0b
0001DE70: beq.w 0x0001de7c
0001DE74: btst.b #0x6,(0x10,A2)
0001DE7A: bne.b 0x0001de4c
0001DE7C: bsr.w 0x0001dea0 ; call 0001DEA0 FUN_0001dea0
0001DE80: cmpi.w #0x2,(0x00ffdf40).l
0001DE88: bne.w 0x0001de96
0001DE8C: jsr 0x00016eca.l ; call 00016ECA FUN_00016eca
0001DE92: bsr.w 0x0001dfc4 ; call 0001DFC4 FUN_0001dfc4
0001DE96: bsr.w 0x0001e166 ; call 0001E166 FUN_0001e166
0001DE9A: bsr.w 0x0001df1a ; call 0001DF1A FUN_0001df1a
0001DE9E: rts
0001DEA0: lea (0xff0100).l,A0 ; data 00ff0100
0001DEA6: move.w #0x9,D7w
0001DEAA: btst.b #0x6,(0x10,A0)
0001DEB0: beq.w 0x0001dece
0001DEB4: btst.b #0x3,(0x10,A0)
0001DEBA: beq.w 0x0001dec2
0001DEBE: bra.w 0x0001dece
0001DEC2: tst.w (0xc,A0)
0001DEC6: bne.w 0x0001dece
0001DECA: bsr.w 0x0001e514 ; call 0001E514 FUN_0001e514
0001DECE: adda.w #0x100,A0
0001DED2: dbf D7w,0x0001deaa
0001DED6: rts
0001DED8: lea (0xff0400).l,A0 ; data 00ff0400
0001DEDE: move.w #0x6,D7w
0001DEE2: clr.l (0x00fffc3c).l
0001DEE8: btst.b #0x7,(0xb0,A0)
0001DEEE: beq.w 0x0001def6
0001DEF2: bsr.w 0x0001e514 ; call 0001E514 FUN_0001e514
0001DEF6: adda.w #0x100,A0
0001DEFA: dbf D7w,0x0001dee8
0001DEFE: rts
0001DF00: lea (0xff0400).l,A0 ; data 00ff0400
0001DF06: move.w #0x6,D7w
0001DF0A: bclr.b #0x0,(0x9e,A0)
0001DF10: adda.w #0x100,A0
0001DF14: dbf D7w,0x0001df0a
0001DF18: rts
0001DF1A: movea.l #0xff0100,A0
0001DF20: bsr.w 0x0001df34 ; call 0001DF34 FUN_0001df34
0001DF24: movea.l #0xff0200,A0
0001DF2A: bsr.w 0x0001df34 ; call 0001DF34 FUN_0001df34
0001DF2E: movea.l #0xff0300,A0
0001DF34: move.w (A0),D0w
0001DF36: move.w (0x4,A0),D1w
0001DF3A: lea (0xff164c).l,A0 ; data 00ff164c
0001DF40: move.w #0x9,D6w
0001DF44: btst.b #0x0,(0x10,A0)
0001DF4A: beq.w 0x0001dfba
0001DF4E: move.w (A0),D2w
0001DF50: subi.w #0x100,D2w
0001DF54: cmp.w D0w,D2w
0001DF56: bcc.w 0x0001dfba
0001DF5A: addi.w #0x200,D2w
0001DF5E: cmp.w D0w,D2w
0001DF60: bcs.w 0x0001dfba
0001DF64: move.w (0x4,A0),D2w
0001DF68: subi.w #0x100,D2w
0001DF6C: cmp.w D1w,D2w
0001DF6E: bcc.w 0x0001dfba
0001DF72: addi.w #0x200,D2w
0001DF76: cmp.w D1w,D2w
0001DF78: bcs.w 0x0001dfba
0001DF7C: move.w (0x2,A0),D0w
0001DF80: jsr 0x000133a6.l ; call 000133A6 FUN_000133a6
0001DF86: bmi.w 0x0001dfba
0001DF8A: move.w #0x24,D0w
0001DF8E: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001DF94: bclr.b #0x0,(0x10,A0)
0001DF9A: cmpi.w #0x65,(0x2,A0)
0001DFA0: bne.w 0x0001dfc2
0001DFA4: move.l A0,-(SP) ; data Stack[-0x4]
0001DFA6: movea.w (0x6,A0),A0
0001DFAA: adda.l #0xff0000,A0
0001DFB0: jsr 0x0001e514.l ; call 0001E514 FUN_0001e514
0001DFB6: movea.l (SP)+,A0
0001DFB8: rts
0001DFBA: adda.w #0x12,A0
0001DFBE: dbf D6w,0x0001df44
0001DFC2: rts
0001DFC4: tst.b (0x00ffe16f).l
0001DFCA: beq.w 0x0001dfd8
0001DFCE: subq.w #0x1,(0x00ffe11e).l
0001DFD4: bmi.w 0x0001e09a
0001DFD8: tst.b (0x00ffe170).l
0001DFDE: beq.w 0x0001dfe4
0001DFE2: rts
0001DFE4: move.w (0x00ffdf88).l,D0w
0001DFEA: andi.w #0x3f,D0w
0001DFEE: cmpi.w #0xd,D0w
0001DFF2: beq.w 0x0001dff8
0001DFF6: rts
0001DFF8: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001DFFE: cmpi.w #-0x8000,D0w
0001E002: bcs.w 0x0001e008
0001E006: rts
0001E008: movea.l (0x00fffc04).l,A0
0001E00E: move.w (A0),D0w
0001E010: move.w (0x4,A0),D1w
0001E014: lsr.w #0x4,D0w
0001E016: lsr.w #0x4,D1w
0001E018: movea.l (0x00ffde1e).l,A1
0001E01E: movea.l (0xc,A1),A1
0001E022: move.w (A1)+,D2w
0001E024: bpl.w 0x0001e02a
0001E028: rts
0001E02A: move.w (A1)+,D3w
0001E02C: move.w (A1)+,D4w
0001E02E: move.w (A1)+,D5w
0001E030: cmp.w D0w,D2w
0001E032: bcc.w 0x0001e048
0001E036: cmp.w D0w,D3w
0001E038: bcs.w 0x0001e048
0001E03C: cmp.w D1w,D4w
0001E03E: bcc.w 0x0001e048
0001E042: cmp.w D1w,D5w
0001E044: bcc.w 0x0001e04a
0001E048: bra.b 0x0001e022
0001E04A: cmpi.w #0x14,(0x00ffddc4).l
0001E052: bne.w 0x0001e060
0001E056: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
0001E05C: bra.w 0x0001e098
0001E060: cmpi.w #0x2,(0x00ffdf40).l
0001E068: bne.w 0x0001e098
0001E06C: move.w #0x960,(0x00ffe11e).l
0001E074: tst.b (0x00ffe16f).l
0001E07A: bne.w 0x0001e098
0001E07E: move.b #-0x1,(0x00ffe16f).l
0001E086: move.w #0x30,D0w
0001E08A: jsr 0x000530de.l ; call 000530DE FUN_000530de
0001E090: bsr.w 0x0001df00 ; call 0001DF00 FUN_0001df00
0001E094: bsr.w 0x0001e0dc ; call 0001E0DC FUN_0001e0dc
0001E098: rts
0001E09A: tst.b (0x00ffe16f).l
0001E0A0: beq.w 0x0001e0ae
0001E0A4: clr.b (0x00ffe16f).l
0001E0AA: bsr.w 0x0001e0b8 ; call 0001E0B8 FUN_0001e0b8
0001E0AE: move.w #0x30,D0w
0001E0B2: jmp 0x000530d2.l ; call 000530D2 FUN_000530d2
0001E0B8: lea (0xff0400).l,A1 ; data 00ff0400
0001E0BE: move.w #0x6,D7w
0001E0C2: btst.b #0x1,(0x9e,A1)
0001E0C8: beq.w 0x0001e0d2
0001E0CC: bset.b #0x2,(0x9e,A1)
0001E0D2: adda.w #0x100,A1
0001E0D6: dbf D7w,0x0001e0c2
0001E0DA: rts
0001E0DC: movea.l (0x00ffde1e).l,A1
0001E0E2: movea.l (0x10,A1),A5
0001E0E6: tst.l (A5)
0001E0E8: beq.w 0x0001e164
0001E0EC: jsr 0x0001a342.l ; call 0001A342 FUN_0001a342
0001E0F2: bcs.w 0x0001e164
0001E0F6: move.w D0w,(0x00ffdd40).l
0001E0FC: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
0001E102: bcs.w 0x0001e164
0001E106: move.l (A5)+,D4
0001E108: movea.l (A5)+,A3
0001E10A: bclr.l #0x1f,D4
0001E10E: beq.w 0x0001e12a
0001E112: clr.w (0x00ffdd42).l
0001E118: movea.l A1,A6
0001E11A: jsr 0x000044bc.l ; call 000044BC FUN_000044bc
0001E120: bset.b #0x5,(0x10,A1)
0001E126: bra.w 0x0001e13c
0001E12A: addi.w #0xc0,(0x00ffdd42).l
0001E132: jsr 0x000044bc.l ; call 000044BC FUN_000044bc
0001E138: move.l A6,(0xa8,A1)
0001E13C: bset.b #0x1,(0x9e,A1)
0001E142: movea.l A1,A0
0001E144: jsr 0x00005a56.l ; call 00005A56 project_entity_screen_position
0001E14A: tst.w (0xc,A0)
0001E14E: beq.w 0x0001e162
0001E152: move.w #0x4,(0xe4,A0)
0001E158: move.w (0x2,A3),(A0)
0001E15C: move.w (0x4,A3),(0x4,A0)
0001E162: bra.b 0x0001e0e6
0001E164: rts
0001E166: tst.w (0x00ffdfe8).l
0001E16C: bne.w 0x0001e1ce
0001E170: movea.l (0x00fffc04).l,A0
0001E176: btst.b #0x7,(0x11,A0)
0001E17C: bne.w 0x0001e1ce
0001E180: tst.b (0x00ffe18b).l
0001E186: bne.w 0x0001e1ce
0001E18A: tst.b (0x00ffe16f).l
0001E190: bne.w 0x0001e19e
0001E194: tst.w (0x00ffdf84).l
0001E19A: bne.w 0x0001e1ce
0001E19E: tst.b (0x00ffe177).l
0001E1A4: bne.w 0x0001e1ce
0001E1A8: move.w (0x00ffdf88).l,D0w
0001E1AE: andi.w #0x3ff,D0w
0001E1B2: cmpi.w #0xa,D0w
0001E1B6: beq.w 0x0001e1bc
0001E1BA: rts
0001E1BC: move.w (0x00ffde18).l,D1w
0001E1C2: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0001E1C8: cmp.w D1w,D0w
0001E1CA: bcs.w 0x0001e1d0
0001E1CE: rts
0001E1D0: tst.b (0x00ffe16f).l
0001E1D6: beq.w 0x0001e1e6
0001E1DA: lea (0x1e20a).l,A6 ; data 0001e20a
0001E1E0: clr.w D0w
0001E1E2: bra.w 0x0001e1fc ; call 0001E1FC FUN_0001e1fc
0001E1E6: jsr 0x00059986.l ; call 00059986 FUN_00059986
0001E1EC: tst.w D0w
0001E1EE: beq.w 0x0001e1fc
0001E1F2: move.l A6,(0x00ffdd2c).l
0001E1F8: bra.w 0x0001e2e0 ; call 0001E2E0 FUN_0001e2e0
0001E1FC: movem.l {  A5},-(SP)
0001E200: bsr.w 0x0001e216 ; call 0001E216 FUN_0001e216
0001E204: movem.l (SP)+,{  A5}
0001E208: rts
0001E216: tst.w D0w
0001E218: beq.w 0x0001e224
0001E21C: bsr.w 0x0001e40c ; call 0001E40C FUN_0001e40c
0001E220: bra.w 0x0001e228
0001E224: bsr.w 0x0001e302 ; call 0001E302 FUN_0001e302
0001E228: lea (0xffdd50).l,A4 ; data 00ffdd50
0001E22E: tst.w (A4)
0001E230: beq.w 0x0001e258
0001E234: clr.w D0w
0001E236: movea.l A6,A5
0001E238: adda.w #0x4,A5
0001E23C: move.l (A6)+,D4
0001E23E: beq.w 0x0001e258
0001E242: movem.w {  D0w},-(SP)
0001E246: bsr.w 0x0001e25a ; call 0001E25A FUN_0001e25a
0001E24A: movem.w (SP)+,{ D0}
0001E24E: bcs.w 0x0001e258
0001E252: addq.w #0x1,D0w
0001E254: movea.l A5,A6
0001E256: bra.b 0x0001e236
0001E258: rts
0001E25A: move.l A4,(0x00ffdd4c).l
0001E260: jsr 0x0001a342.l ; call 0001A342 FUN_0001a342
0001E266: bcs.w 0x0001e2b8
0001E26A: move.w D0w,(0x00ffdd40).l
0001E270: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
0001E276: bcs.w 0x0001e2b8
0001E27A: movea.l #0x1e300,A3
0001E280: jsr 0x000044bc.l ; call 000044BC FUN_000044bc
0001E286: move.b #0x4,(0x4d,A1)
0001E28C: bset.b #0x5,(0x10,A1)
0001E292: movea.l (0x00ffdd4c).l,A4
0001E298: move.w (A4)+,D5w
0001E29A: bne.w 0x0001e2a6
0001E29E: lea (0xffdd50).l,A4 ; data 00ffdd50
0001E2A4: move.w (A4)+,D5w
0001E2A6: move.w (A4)+,D6w
0001E2A8: move.l A4,(0x00ffdd4c).l
0001E2AE: move.w D5w,(A1)
0001E2B0: move.w D6w,(0x4,A1)
0001E2B4: bra.w 0x0001e2be
0001E2B8: ori #0x1,CCR
0001E2BC: rts
0001E2BE: addq.b #0x1,(0x00ffe177).l
0001E2C4: bset.b #0x6,(0x9e,A1)
0001E2CA: move.w (A1),(0x96,A1)
0001E2CE: move.w (0x4,A1),(0x98,A1)
0001E2D4: jsr 0x0001dbaa.l ; call 0001DBAA FUN_0001dbaa
0001E2DA: andi #0x1e,CCR
0001E2DE: rts
0001E2E0: tst.w (0x00ffe11c).l
0001E2E6: beq.w 0x0001e2ec
0001E2EA: rts
0001E2EC: move.w #0x3c,(0x00ffe11c).l
0001E2F4: move.l (0x00ffdd2c).l,(0x00ffe104).l
0001E2FE: rts
0001E302: lea (0xffdd50).l,A4 ; data 00ffdd50
0001E308: move.w #0xbe0,D2w
0001E30C: move.w #0x8c0,D3w
0001E310: move.w (0x00ffddce).l,D0w
0001E316: lsr.w #0x1,D0w
0001E318: move.w (0x00ffddc6).l,D1w
0001E31E: addi.w #0xa00,D1w
0001E322: cmp.w D1w,D0w
0001E324: bcc.w 0x0001e32a
0001E328: neg.w D2w
0001E32A: move.w (0x00ffddd0).l,D0w
0001E330: lsr.w #0x1,D0w
0001E332: move.w (0x00ffddc8).l,D4w
0001E338: addi.w #0x700,D4w
0001E33C: cmp.w D4w,D0w
0001E33E: bcc.w 0x0001e344
0001E342: neg.w D3w
0001E344: move.w D1w,D5w
0001E346: move.w D4w,D6w
0001E348: add.w D2w,D5w
0001E34A: andi.w #-0x200,D5w
0001E34E: addi.w #0x100,D5w
0001E352: andi.w #-0x200,D6w
0001E356: addi.w #0x100,D6w
0001E35A: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E35E: bne.w 0x0001e366
0001E362: move.w D5w,(A4)+
0001E364: move.w D6w,(A4)+
0001E366: addi.w #0x200,D6w
0001E36A: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E36E: bne.w 0x0001e376
0001E372: move.w D5w,(A4)+
0001E374: move.w D6w,(A4)+
0001E376: addi.w #0x200,D6w
0001E37A: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E37E: bne.w 0x0001e386
0001E382: move.w D5w,(A4)+
0001E384: move.w D6w,(A4)+
0001E386: addi.w #-0x600,D6w
0001E38A: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E38E: bne.w 0x0001e396
0001E392: move.w D5w,(A4)+
0001E394: move.w D6w,(A4)+
0001E396: addi.w #-0x200,D6w
0001E39A: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E39E: bne.w 0x0001e3a6
0001E3A2: move.w D5w,(A4)+
0001E3A4: move.w D6w,(A4)+
0001E3A6: move.w D1w,D5w
0001E3A8: move.w D4w,D6w
0001E3AA: add.w D3w,D6w
0001E3AC: andi.w #-0x200,D5w
0001E3B0: addi.w #0x100,D5w
0001E3B4: andi.w #-0x200,D6w
0001E3B8: addi.w #0x100,D6w
0001E3BC: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E3C0: bne.w 0x0001e3c8
0001E3C4: move.w D5w,(A4)+
0001E3C6: move.w D6w,(A4)+
0001E3C8: addi.w #0x200,D5w
0001E3CC: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E3D0: bne.w 0x0001e3d8
0001E3D4: move.w D5w,(A4)+
0001E3D6: move.w D6w,(A4)+
0001E3D8: addi.w #0x200,D5w
0001E3DC: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E3E0: bne.w 0x0001e3e8
0001E3E4: move.w D5w,(A4)+
0001E3E6: move.w D6w,(A4)+
0001E3E8: addi.w #-0x600,D5w
0001E3EC: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E3F0: bne.w 0x0001e3f8
0001E3F4: move.w D5w,(A4)+
0001E3F6: move.w D6w,(A4)+
0001E3F8: addi.w #-0x200,D5w
0001E3FC: bsr.w 0x0001e41e ; call 0001E41E FUN_0001e41e
0001E400: bne.w 0x0001e408
0001E404: move.w D5w,(A4)+
0001E406: move.w D6w,(A4)+
0001E408: clr.w (A4)
0001E40A: rts
0001E40C: lea (0xffdd50).l,A4 ; data 00ffdd50
0001E412: move.w #0x500,D2w
0001E416: move.w #0x500,D3w
0001E41A: bra.w 0x0001e310
0001E41E: lea (0xff1700).l,A2 ; data 00ff1700
0001E424: move.w D5w,D0w
0001E426: move.w D6w,D7w
0001E428: lsr.w #0x4,D0w
0001E42A: lsr.w #0x4,D7w
0001E42C: lsl.w #0x1,D7w
0001E42E: andi.w #-0x40,D7w
0001E432: lsr.w #0x5,D0w
0001E434: add.w D0w,D7w
0001E436: move.b (0x0,A2,D7w*0x1),D0b ; data 00ff1700
0001E43A: rts
0001E43C: clr.b (0x00ffe180).l
0001E442: clr.b (0x00ffe182).l
0001E448: clr.b (0x00ffe184).l
0001E44E: clr.b (0x00ffe187).l
0001E454: tst.b (0x00ffe183).l
0001E45A: beq.w 0x0001e4aa
0001E45E: clr.b (0x00ffe183).l
0001E464: lea (0xff0400).l,A1 ; data 00ff0400
0001E46A: move.w #0xc,D7w
0001E46E: btst.b #0x0,(0x10,A1)
0001E474: beq.w 0x0001e482
0001E478: btst.b #0x7,(0xb0,A1)
0001E47E: bne.w 0x0001e496
0001E482: adda.w #0x100,A1
0001E486: dbf D7w,0x0001e46e
0001E48A: move.b #-0x1,(0x00ffe181).l
0001E492: bra.w 0x0001e49c
0001E496: clr.b (0x00ffe181).l
0001E49C: move.l #0x1f16a,(0x00ffdd2c).l
0001E4A6: bra.w 0x0001e2ec ; call 0001E2EC FUN_0001e2ec
0001E4AA: rts
0001E4AC: move.l A6,D4
0001E4AE: jsr 0x0001a342.l ; call 0001A342 FUN_0001a342
0001E4B4: bcs.w 0x0001e4fe
0001E4B8: move.w D0w,(0x00ffdd40).l
0001E4BE: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
0001E4C4: bcs.w 0x0001e4fe
0001E4C8: movea.l #0x1e300,A3
0001E4CE: jsr 0x000044bc.l ; call 000044BC FUN_000044bc
0001E4D4: bset.b #0x7,(0xb0,A1)
0001E4DA: move.l D4,(0x00fffc3c).l
0001E4E0: movea.l (0x00fffc04).l,A0
0001E4E6: move.w (A0),D0w
0001E4E8: addi.w #0x80,D0w
0001E4EC: move.w D0w,(A1)
0001E4EE: move.w (0x4,A0),(0x4,A1)
0001E4F4: move.w (A1),(0x96,A1)
0001E4F8: move.w (0x4,A1),(0x98,A1)
0001E4FE: rts
0001E514: bclr.b #0x7,(0xb0,A0)
0001E51A: beq.w 0x0001e524
0001E51E: clr.l (0x00fffc3c).l
0001E524: bclr.b #0x0,(0x10,A0)
0001E52A: btst.b #0x3,(0x10,A0)
0001E530: bne.w 0x0001e53e
0001E534: move.w (0x30,A0),D0w
0001E538: jsr 0x0001a304.l ; call 0001A304 FUN_0001a304
0001E53E: rts
0001E540: move.w #-0x1,(0x00ffdfda).l
0001E548: move.w #0x3,(0x00ffdfdc).l
0001E550: rts
0001E68C: move.w #-0x1,(0x00fffbec).l
0001E694: clr.b (0x00fffc1c).l
0001E69A: rts
0001E69C: clr.b (0x00fff0b6).l
0001E6A2: bsr.b 0x0001e68c ; call 0001E68C FUN_0001e68c
0001E6A4: lea (0xf3dc6).l,A0 ; data 000f3dc6
0001E6AA: lea (0xf3ebc).l,A1 ; data 000f3ebc
0001E6B0: move.b (0x00fff0ae).l,(0x00fffbec).l
0001E6BA: moveq #0x8,D1
0001E6BC: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001E6C2: move.b (0x00fff0ae).l,D1b
0001E6C8: mulu.w #0x30,D1
0001E6CC: mulu.w #0x6,D0
0001E6D0: clr.l D2
0001E6D2: move.w D0w,D2w
0001E6D4: add.w D1w,D2w
0001E6D6: move.b (0x0,A1,D2w*0x1),(0x00fffbed).l
0001E6DE: clr.l D6
0001E6E0: move.b (0x0,A1,D2w*0x1),D6b
0001E6E4: move.b (0x1,A1,D2w*0x1),(0x00fffbf3).l
0001E6EC: clr.l D3
0001E6EE: clr.l D4
0001E6F0: move.b (0x00ff018c).l,D4b
0001E6F6: move.w (0x4,A1,D2w*0x1),D1w
0001E6FA: mulu.w D4w,D1
0001E6FC: addq.w #0x1,D1w
0001E6FE: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0001E704: mulu.w #0x5,D0
0001E708: add.w (0x2,A1,D2w*0x1),D0w
0001E70C: move.w D0w,(0x00fffbf4).l
0001E712: move.w (0x2,A1,D2w*0x1),(0x00fff0c0).l
0001E71A: clr.l D1
0001E71C: move.b (0x00fffbed).l,D1b
0001E722: lsl.w #0x2,D1w
0001E724: adda.l D1,A0
0001E726: movea.l (A0),A0
0001E728: jsr (A0)
0001E72A: rts
0001E78C: lea (0xffb740).l,A5 ; data 00ffb740
0001E792: lea (0xf2520).l,A1 ; data 000f2520
0001E798: clr.w D1w
0001E79A: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001E7A0: clr.l D0
0001E7A2: move.b (0x00fffbee).l,D0b
0001E7A8: lsl.w #0x2,D0w
0001E7AA: lea (0xf1e58).l,A1 ; data 000f1e58
0001E7B0: movea.l (0x0,A1,D0w*0x1),A1 ; data 000f1e58
0001E7B4: moveq #0x1,D1
0001E7B6: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001E7BC: lea (0xf2549).l,A1 ; data 000f2549
0001E7C2: clr.w D1w
0001E7C4: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001E7CA: move.b #-0x1,(A5)
0001E7CE: rts
0001EEDC: clr.b (0x00fff0d5).l
0001EEE2: cmpi.w #-0x1,(0x00fffbec).l
0001EEEA: bne.w 0x0001eef0
0001EEEE: rts
0001EEF0: btst.b #0x7,(0x00fffc1c).l
0001EEF8: beq.w 0x0001eefe
0001EEFC: rts
0001EEFE: cmpi.b #0x6,(0x00fffbed).l
0001EF06: bmi.w 0x0001ef0c
0001EF0A: rts
0001EF0C: lea (0xf3e56).l,A1 ; data 000f3e56
0001EF12: clr.l D1
0001EF14: move.b (0x00fffbed).l,D1b
0001EF1A: lsl.w #0x2,D1w
0001EF1C: adda.l D1,A1
0001EF1E: movea.l (A1),A1 ; data 000f3e56
0001EF20: move.b #0x0,(0x00fff10c).l
0001EF28: jsr (A1)
0001EF2A: rts
0001F000: bclr.l #0x9,D0
0001F004: move.l D0,(0x00fffc22).l
0001F00A: clr.l D1
0001F00C: move.w (0x00fffbf4).l,D1w
0001F012: tst.b (0x00fffbed).l
0001F018: bne.w 0x0001f024
0001F01C: move.w (0x00fffbf0).l,D2w
0001F022: mulu.w D2w,D1
0001F024: add.l D1,(0x00fffbfe).l
0001F02A: move.b (0x00fffbf3).l,D1b
0001F030: add.b D1b,(0x00ff018d).l
0001F036: add.b D1b,(0x00ff028d).l
0001F03C: add.b D1b,(0x00ff038d).l
0001F042: bra.w 0x0001e68c ; call 0001E68C FUN_0001e68c
0001F1E0: addq.b #0x1,(0x00fffbfb).l
0001F1E6: cmpi.b #0x9,(0x00fffbfb).l
0001F232: lea (0xf359e).l,A0 ; data 000f359e
0001F238: lea (0x1220b4).l,A5 ; data 001220b4
0001F23E: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0001F244: rts
0001F246: move.l A0,-(SP) ; data Stack[-0x4]
0001F248: move.l A1,-(SP) ; data Stack[-0x8]
0001F24A: move.w #-0x6f00,(0x00c00004).l
0001F252: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
0001F258: jsr 0x00010984.l ; call 00010984 FUN_00010984
0001F25E: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
0001F262: movea.l (SP)+,A1
0001F264: tst.b (0x00fff10c).l
0001F26A: beq.w 0x0001f270
0001F26E: jsr (A1)
0001F270: movea.l (SP)+,A0
0001F272: clr.b (0x00fff10c).l
0001F278: bsr.w 0x0001f3c4 ; call 0001F3C4 FUN_0001f3c4
0001F27C: jsr 0x00010952.l ; call 00010952 FUN_00010952
0001F282: jsr 0x0005670e.l ; call 0005670E FUN_0005670e
0001F288: move.w #-0x6e6f,(0x00c00004).l
0001F290: rts
0001F292: move.l D6,-(SP) ; data Stack[-0x4]
0001F294: tst.b (0x00fff0b5).l
0001F29A: beq.w 0x0001f2a2
0001F29E: bsr.w 0x0001f32e ; call 0001F32E FUN_0001f32e
0001F2A2: lea (0x12cc30).l,A0 ; data 0012cc30
0001F2A8: jsr 0x000566e2.l ; call 000566E2 FUN_000566e2
0001F2AE: movea.l (A5)+,A0
0001F2B0: jsr 0x000566f6.l ; call 000566F6 FUN_000566f6
0001F2B6: lea (0xffbf40).l,A1 ; data 00ffbf40
0001F2BC: movea.l (A5)+,A3
0001F2BE: move.l #0x57a0,D0
0001F2C4: jsr 0x00001530.l ; call 00001530 FUN_00001530
0001F2CA: tst.b (0x00fff0af).l
0001F2D0: bmi.w 0x0001f2f6
0001F2D4: movea.l (A5)+,A1
0001F2D6: move.w #-0x1dfc,D4w
0001F2DA: move.w #0x6,D7w
0001F2DE: move.w #0x8,D1w
0001F2E2: move.w #0x6000,D2w
0001F2E6: clr.w D3w
0001F2E8: move.w D7w,D5w
0001F2EA: addq.w #0x1,D5w
0001F2EC: jsr 0x00001728.l ; call 00001728 FUN_00001728
0001F2F2: bra.w 0x0001f32a
0001F2F6: tst.b (0x00fff0ae).l
0001F2FC: bmi.w 0x0001f30a
0001F300: lea (0x72c1a).l,A0 ; data 00072c1a
0001F306: bra.w 0x0001f316
0001F30A: lea (0xff0100).l,A0 ; data 00ff0100
0001F310: clr.b (0x00fff0ae).l
0001F316: bsr.w 0x0001f38a ; call 0001F38A FUN_0001f38a
0001F31A: move.l #0xe202,D4
0001F320: bsr.w 0x0001f34a ; call 0001F34A FUN_0001f34a
0001F324: clr.b (0x00fff0af).l
0001F32A: move.l (SP)+,D6
0001F32C: rts
0001F32E: clr.l D3
0001F330: move.b (0x00fff0b4).l,D3b
0001F336: lsl.w #0x2,D3w
0001F338: lea (0x12d0ec).l,A5 ; data 0012d0ec
0001F33E: movea.l (0x0,A5,D3w*0x1),A5 ; data 0012d0ec
0001F342: clr.b (0x00fff0b5).l
0001F348: rts
0001F34A: clr.l D0
0001F34C: move.b (0x00fff0ae).l,D0b
0001F352: mulu.w #0x100,D0
0001F356: adda.l D0,A0
0001F358: move.w (0x2c,A0),D2w
0001F35C: divu.w #0x2000,D2
0001F360: subq.w #0x1,D2w
0001F362: lsl.w #0x2,D2w
0001F364: lea (0x18acb6).l,A0 ; data 0018acb6
0001F36A: movea.l (0x0,A0,D2w*0x1),A0 ; data 0018acb6
0001F36E: jsr 0x000566f6.l ; call 000566F6 FUN_000566f6
0001F374: move.w #0x6000,D2w
0001F378: moveq #0x1,D5
0001F37A: move.w #0x7,D7w
0001F37E: move.w #0x9,D1w
0001F382: jsr 0x00003648.l ; call 00003648 FUN_00003648
0001F388: rts
0001F38A: move.l A0,-(SP) ; data Stack[-0x4]
0001F38C: clr.l D0
0001F38E: move.b (0x00fff0ae).l,D0b
0001F394: mulu.w #0x100,D0
0001F398: adda.l D0,A0
0001F39A: moveq #0x20,D1
0001F39C: moveq #0x20,D2
0001F39E: move.l (0x50,A0),D6
0001F3A2: movea.l D6,A2
0001F3A4: moveq #0x4f,D6
0001F3A6: clr.l D0
0001F3A8: move.w (A2)+,D0w
0001F3AA: asl.l #0x5,D0
0001F3AC: addi.l #0x6acfa,D0
0001F3B2: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
0001F3B8: addi.w #0x20,D1w
0001F3BC: dbf D6w,0x0001f3a6
0001F3C0: movea.l (SP)+,A0
0001F3C2: rts
0001F3C4: tst.b (0x00fff10c).l
0001F3CA: beq.w 0x0001f3d8
0001F3CE: clr.b (0x00fff10c).l
0001F3D4: bra.w 0x0001f404
0001F3D8: bsr.w 0x0001f5a8 ; call 0001F5A8 FUN_0001f5a8
0001F3DC: lea (0xffb740).l,A5 ; data 00ffb740
0001F3E2: movea.l (A0)+,A1
0001F3E4: clr.w D1w
0001F3E6: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001F3EC: move.b #-0x1,(A5)
0001F3F0: movea.l #0xc116,A2
0001F3F6: move.w #0x1b,D1w
0001F3FA: move.w #0x2000,D2w
0001F3FE: jsr 0x0000c168.l ; call 0000C168 FUN_0000c168
0001F404: lea (0xffb740).l,A5 ; data 00ffb740
0001F40A: movea.l (A0)+,A1
0001F40C: move.l A1,D1
0001F40E: bmi.w 0x0001f592
0001F412: clr.w D1w
0001F414: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001F41A: move.b #-0x1,(A5)
0001F41E: movea.l #0xc98c,A2
0001F424: moveq #0x22,D1
0001F426: move.w #0x2000,D2w
0001F42A: jsr 0x0000c168.l ; call 0000C168 FUN_0000c168
0001F430: lea (0xcf11c).l,A1 ; data 000cf11c
0001F436: move.w #-0x367c,D4w
0001F43A: move.w #0x1,D7w
0001F43E: move.w #0x1,D1w
0001F442: move.w #0x0,D2w
0001F446: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001F44C: lea (0xffb740).l,A5 ; data 00ffb740
0001F452: movea.l (A0)+,A1
0001F454: move.l A1,D1
0001F456: bne.w 0x0001f468
0001F45A: move.w #-0x1,(0x00fff0d2).l
0001F462: addq.l #0x4,A0
0001F464: bra.w 0x0001f502
0001F468: clr.w (0x00fff0d2).l
0001F46E: clr.w D1w
0001F470: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001F476: move.b #-0x1,(A5)
0001F47A: movea.l #0xcb0c,A2
0001F480: moveq #0x22,D1
0001F482: move.w #0x2000,D2w
0001F486: jsr 0x0000c168.l ; call 0000C168 FUN_0000c168
0001F48C: lea (0xcf124).l,A1 ; data 000cf124
0001F492: move.w #-0x34fc,D4w
0001F496: move.w #0x1,D7w
0001F49A: move.w #0x1,D1w
0001F49E: move.w #0x0,D2w
0001F4A2: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001F4A8: lea (0xffb740).l,A5 ; data 00ffb740
0001F4AE: movea.l (A0)+,A1
0001F4B0: move.l A1,D1
0001F4B2: bne.w 0x0001f4c2
0001F4B6: move.b #-0x1,(0x00fff0d3).l
0001F4BE: bra.w 0x0001f502
0001F4C2: clr.b (0x00fff0d3).l
0001F4C8: clr.w D1w
0001F4CA: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0001F4D0: move.b #-0x1,(A5)
0001F4D4: movea.l #0xcc8c,A2
0001F4DA: moveq #0x22,D1
0001F4DC: move.w #0x2000,D2w
0001F4E0: jsr 0x0000c168.l ; call 0000C168 FUN_0000c168
0001F4E6: lea (0xcf12c).l,A1 ; data 000cf12c
0001F4EC: move.w #-0x337c,D4w
0001F4F0: move.w #0x1,D7w
0001F4F4: move.w #0x1,D1w
0001F4F8: move.w #0x0,D2w
0001F4FC: jsr 0x00001232.l ; call 00001232 FUN_00001232
0001F502: move.w #0xa,D5w
0001F506: jsr 0x00000c24.l ; call 00000C24 FUN_00000c24
0001F50C: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0001F512: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0001F518: move.l A0,-(SP) ; data Stack[-0x4]
0001F51A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0001F520: movea.l (SP)+,A0
0001F522: btst.b #0xe,(0x00ffdd22).l
0001F52A: beq.w 0x0001f540
0001F52E: movea.l (A0),A0
0001F530: move.l A0,D0
0001F532: cmpi.l #0xfa,D0
0001F538: bmi.w 0x0001f572
0001F53C: bra.w 0x0001f3c4
0001F540: btst.b #0xc,(0x00ffdd22).l
0001F548: beq.w 0x0001f55a
0001F54C: tst.b (0x00fff0d2).l
0001F552: bmi.b 0x0001f50c
0001F554: movea.l (0x4,A0),A0
0001F558: bra.b 0x0001f530
0001F55A: btst.b #0xd,(0x00ffdd22).l
0001F562: beq.b 0x0001f50c
0001F564: tst.b (0x00fff0d3).l
0001F56A: bmi.b 0x0001f50c
0001F56C: movea.l (0x8,A0),A0
0001F570: bra.b 0x0001f530
0001F572: cmpi.l #0xfa,D0
0001F578: bpl.w 0x0001f590
0001F57C: move.l A0,D0
0001F57E: lsl.w #0x2,D0w
0001F580: lea (0x12ccb4).l,A0 ; data 0012ccb4
0001F586: movea.l (0x0,A0,D0w*0x1),A0 ; data 0012ccb4
0001F58A: jsr (A0)
0001F58C: bra.w 0x0001f3c4
0001F590: rts
0001F592: move.w #0x1e,D5w
0001F596: jsr 0x00000c24.l ; call 00000C24 FUN_00000c24
0001F59C: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
0001F5A2: move.w #0xfb,D0w
0001F5A6: rts
0001F5A8: move.l #0xc116,D4
0001F5AE: moveq #0xe,D1
0001F5B0: moveq #0x1a,D7
0001F5B2: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0001F5B8: move.l #0xc984,D4
0001F5BE: moveq #0x8,D1
0001F5C0: moveq #0x24,D7
0001F5C2: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0001F5C8: rts
0001F5CA: btst.b #0x6,(0x00ff0110).l
0001F5D2: bne.w 0x0001f5e2
0001F5D6: btst.b #0x4,(0x00ff0110).l
0001F5DE: beq.w 0x0001f5e8
0001F5E2: jmp 0x0000c64c.l ; call 0000C64C FUN_0000c64c
0001F5E8: bsr.w 0x00020634 ; call 00020634 FUN_00020634
0001F5EC: move.b #0x0,(0x00fff10c).l
0001F5F4: clr.l D6
0001F5F6: clr.b (0x00fff0cc).l
0001F5FC: jsr 0x0001eedc.l ; call 0001EEDC FUN_0001eedc
0001F602: tst.w D6w
0001F604: bmi.w 0x0001f652
0001F608: tst.b (0x00fff0cc).l
0001F60E: bmi.w 0x0001f652
0001F612: jsr 0x00053a78.l ; call 00053A78 FUN_00053a78
0001F618: tst.w D6w
0001F61A: bmi.w 0x0001f652
0001F61E: tst.b (0x00fff0cc).l
0001F624: bmi.w 0x0001f652
0001F628: jsr 0x0001f65a.l ; call 0001F65A FUN_0001f65a
0001F62E: tst.w D6w
0001F630: bmi.w 0x0001f652
0001F634: jsr 0x0001f830.l ; call 0001F830 FUN_0001f830
0001F63A: tst.w D6w
0001F63C: bmi.w 0x0001f652
0001F640: jsr 0x0001fa78.l ; call 0001FA78 FUN_0001fa78
0001F646: tst.w D6w
0001F648: bmi.w 0x0001f652
0001F64C: jsr 0x0001fd80.l ; call 0001FD80 FUN_0001fd80
0001F652: clr.b (0x00fff0cc).l
0001F658: rts
0001F65A: cmpi.b #0x0,(0x00fffbf8).l
0001F662: bne.w 0x0001f6ac
0001F666: cmpi.b #0x3,(0x00fffbf9).l
0001F66E: bne.w 0x0001f6ac
0001F672: lea (0x121fc4).l,A5 ; data 00121fc4
0001F678: move.b #0x4b,(0x00fff10d).l
0001F680: tst.b (0x00fff0d5).l
0001F686: beq.w 0x0001f6a2
0001F68A: bmi.w 0x0001f698
0001F68E: lea (0xf495c).l,A0 ; data 000f495c
0001F694: bra.w 0x0001f816
0001F698: lea (0xf4954).l,A0 ; data 000f4954
0001F69E: bra.w 0x0001f816
0001F6A2: lea (0xf4910).l,A0 ; data 000f4910
0001F6A8: bra.w 0x0001f816
0001F6AC: cmpi.b #0x1,(0x00fffbf8).l
0001F6B4: bne.w 0x0001f71e
0001F6B8: cmpi.b #0x1,(0x00fffbf9).l
0001F6C0: bne.w 0x0001f71e
0001F6C4: lea (0x121fb8).l,A5 ; data 00121fb8
0001F6CA: move.b #0x1e,(0x00fff10d).l
0001F6D2: tst.b (0x00fff0d5).l
0001F6D8: beq.w 0x0001f6f4
0001F6DC: bmi.w 0x0001f6ea
0001F6E0: lea (0xf49e0).l,A0 ; data 000f49e0
0001F6E6: bra.w 0x0001f816
0001F6EA: lea (0xf49d8).l,A0 ; data 000f49d8
0001F6F0: bra.w 0x0001f816
0001F6F4: move.l (0x00fffc1e).l,D0
0001F6FA: btst.l #0xd,D0
0001F6FE: beq.w 0x0001f714
0001F702: btst.l #0x1e,D0
0001F706: bne.w 0x0001f714
0001F70A: lea (0xf4990).l,A0 ; data 000f4990
0001F710: bra.w 0x0001f816
0001F714: lea (0xf4964).l,A0 ; data 000f4964
0001F71A: bra.w 0x0001f816
0001F71E: cmpi.b #0x2,(0x00fffbf8).l
0001F726: bne.w 0x0001f770
0001F72A: cmpi.b #0x2,(0x00fffbf9).l
0001F732: bne.w 0x0001f770
0001F736: lea (0x121fa0).l,A5 ; data 00121fa0
0001F73C: move.b #0x14,(0x00fff10d).l
0001F744: tst.b (0x00fff0d5).l
0001F74A: beq.w 0x0001f766
0001F74E: bmi.w 0x0001f75c
0001F752: lea (0xf4af4).l,A0 ; data 000f4af4
0001F758: bra.w 0x0001f816
0001F75C: lea (0xf4aec).l,A0 ; data 000f4aec
0001F762: bra.w 0x0001f816
0001F766: lea (0xf4aa0).l,A0 ; data 000f4aa0
0001F76C: bra.w 0x0001f816
0001F770: cmpi.b #0x3,(0x00fffbf8).l
0001F778: bne.w 0x0001f7c2
0001F77C: cmpi.b #0x1,(0x00fffbf9).l
0001F784: bne.w 0x0001f7c2
0001F788: lea (0x121fac).l,A5 ; data 00121fac
0001F78E: move.b #0x32,(0x00fff10d).l
0001F796: tst.b (0x00fff0d5).l
0001F79C: beq.w 0x0001f7b8
0001F7A0: bmi.w 0x0001f7ae
0001F7A4: lea (0xf49e0).l,A0 ; data 000f49e0
0001F7AA: bra.w 0x0001f816
0001F7AE: lea (0xf4a50).l,A0 ; data 000f4a50
0001F7B4: bra.w 0x0001f816
0001F7B8: lea (0xf4a18).l,A0 ; data 000f4a18
0001F7BE: bra.w 0x0001f816
0001F7C2: cmpi.b #0x4,(0x00fffbf8).l
0001F7CA: bne.w 0x0001f82e
0001F7CE: cmpi.b #0x0,(0x00fffbf9).l
0001F7D6: bne.w 0x0001f82e
0001F7DA: lea (0x121fd0).l,A5 ; data 00121fd0
0001F7E0: move.b #0x28,(0x00fff10d).l
0001F7E8: tst.b (0x00fff0d5).l
0001F7EE: beq.w 0x0001f810
0001F7F2: bmi.w 0x0001f800
0001F7F6: lea (0xf4a98).l,A0 ; data 000f4a98
0001F7FC: bra.w 0x0001f816
0001F800: lea (0x122024).l,A5 ; data 00122024
0001F806: lea (0xf4a90).l,A0 ; data 000f4a90
0001F80C: bra.w 0x0001f816
0001F810: lea (0xf4a60).l,A0 ; data 000f4a60
0001F816: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001F81A: tst.b (0x00fff10d).l
0001F820: bpl.w 0x0001f82a
0001F824: jsr 0x0000cf38.l ; call 0000CF38 FUN_0000cf38
0001F82A: move.w #-0x1,D6w
0001F82E: rts
0001F830: cmpi.b #0x0,(0x00fffbf8).l
0001F838: bne.w 0x0001f95e
0001F83C: cmpi.b #0x0,(0x00fffbf9).l
0001F844: bne.w 0x0001f95e
0001F848: lea (0x121e68).l,A5 ; data 00121e68
0001F84E: move.b #0xb,(0x00fff0b6).l
0001F856: tst.b (0x00fff0d5).l
0001F85C: beq.w 0x0001f878
0001F860: bmi.w 0x0001f86e
0001F864: lea (0x110a8e).l,A0 ; data 00110a8e
0001F86A: bra.w 0x0001fa50
0001F86E: lea (0x110a86).l,A0 ; data 00110a86
0001F874: bra.w 0x0001fa50
0001F878: move.l (0x00fffc30).l,D3
0001F87E: btst.l #0x3,D3
0001F882: beq.w 0x0001f890
0001F886: lea (0x110a72).l,A0 ; data 00110a72
0001F88C: bra.w 0x0001fa50
0001F890: move.l (0x00fffc1e).l,D1
0001F896: btst.l #0x17,D1
0001F89A: beq.w 0x0001f8da
0001F89E: btst.l #0x18,D1
0001F8A2: beq.w 0x0001f8da
0001F8A6: btst.l #0x19,D1
0001F8AA: bne.w 0x0001f8e4
0001F8AE: move.l (0x00fffc34).l,D1
0001F8B4: bclr.l #0x12,D1
0001F8B8: bset.l #0x13,D1
0001F8BC: move.l D1,(0x00fffc34).l
0001F8C2: move.l #0x19,(0x00ffdd64).l
0001F8CC: bsr.w 0x0002196c ; call 0002196C FUN_0002196c
0001F8D0: lea (0x11105a).l,A0 ; data 0011105a
0001F8D6: bra.w 0x0001fa50
0001F8DA: lea (0x110a32).l,A0 ; data 00110a32
0001F8E0: bra.w 0x0001fa50
0001F8E4: btst.l #0x1d,D1
0001F8E8: bne.b 0x0001f8da
0001F8EA: move.l (0x00fffc22).l,D1
0001F8F0: btst.l #0x0,D1
0001F8F4: beq.w 0x0001f954
0001F8F8: bclr.l #0x0,D1
0001F8FC: move.l D1,(0x00fffc22).l
0001F902: move.w (0x00fffc2a).l,D0w
0001F908: bset.l #0x9,D0
0001F90C: move.w D0w,(0x00fffc2a).l
0001F912: move.l (0x00fffc34).l,D3
0001F918: bclr.l #0x14,D3
0001F91C: move.l D3,(0x00fffc34).l
0001F922: move.l (0x00fffc38).l,D3
0001F928: bclr.l #0x2,D3
0001F92C: bclr.l #0x3,D3
0001F930: bclr.l #0xb,D3
0001F934: move.l D3,(0x00fffc38).l
0001F93A: move.l (0x00fffc1e).l,D3
0001F940: bset.l #0x1d,D3
0001F944: move.l D3,(0x00fffc1e).l
0001F94A: lea (0x110a5e).l,A0 ; data 00110a5e
0001F950: bra.w 0x0001fa50
0001F954: lea (0x110a46).l,A0 ; data 00110a46
0001F95A: bra.w 0x0001fa50
0001F95E: cmpi.b #0x2,(0x00fffbf8).l
0001F966: bne.w 0x0001f9b0
0001F96A: cmpi.b #0x1,(0x00fffbf9).l
0001F972: bne.w 0x0001f9b0
0001F976: lea (0x12200c).l,A5 ; data 0012200c
0001F97C: move.b #0x8,(0x00fff0b6).l
0001F984: tst.b (0x00fff0d5).l
0001F98A: beq.w 0x0001f9a6
0001F98E: bmi.w 0x0001f99c
0001F992: lea (0x111482).l,A0 ; data 00111482
0001F998: bra.w 0x0001fa50
0001F99C: lea (0x11147a).l,A0 ; data 0011147a
0001F9A2: bra.w 0x0001fa50
0001F9A6: lea (0x110cf6).l,A0 ; data 00110cf6
0001F9AC: bra.w 0x0001fa50
0001F9B0: cmpi.b #0x1,(0x00fffbf8).l
0001F9B8: bne.w 0x0001fa02
0001F9BC: cmpi.b #0x6,(0x00fffbf9).l
0001F9C4: bne.w 0x0001fa02
0001F9C8: lea (0x121e8c).l,A5 ; data 00121e8c
0001F9CE: move.b #0x9,(0x00fff0b6).l
0001F9D6: tst.b (0x00fff0d5).l
0001F9DC: beq.w 0x0001f9f8
0001F9E0: bmi.w 0x0001f9ee
0001F9E4: lea (0x1116da).l,A0 ; data 001116da
0001F9EA: bra.w 0x0001fa50
0001F9EE: lea (0x1116d2).l,A0 ; data 001116d2
0001F9F4: bra.w 0x0001fa50
0001F9F8: lea (0x110e1e).l,A0 ; data 00110e1e
0001F9FE: bra.w 0x0001fa50
0001FA02: cmpi.b #0x4,(0x00fffbf8).l
0001FA0A: bne.w 0x0001fa76
0001FA0E: cmpi.b #0x4,(0x00fffbf9).l
0001FA16: bne.w 0x0001fa76
0001FA1A: lea (0x121ee0).l,A5 ; data 00121ee0
0001FA20: move.b #0xa,(0x00fff0b6).l
0001FA28: tst.b (0x00fff0d5).l
0001FA2E: beq.w 0x0001fa4a
0001FA32: bmi.w 0x0001fa40
0001FA36: lea (0x110eea).l,A0 ; data 00110eea
0001FA3C: bra.w 0x0001fa50
0001FA40: lea (0x110ee2).l,A0 ; data 00110ee2
0001FA46: bra.w 0x0001fa50
0001FA4A: lea (0x110eca).l,A0 ; data 00110eca
0001FA50: clr.b (0x00fff0d5).l
0001FA56: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001FA5A: tst.b (0x00fff0d5).l
0001FA60: bne.w 0x0001fa6a
0001FA64: move.w #-0x1,D6w
0001FA68: rts
0001FA6A: clr.b (0x00fff0d5).l
0001FA70: jsr 0x0000c928.l ; call 0000C928 FUN_0000c928
0001FA76: rts
0001FA78: cmpi.b #0x2,(0x00fffbf8).l
0001FA80: bne.w 0x0001fad6
0001FA84: cmpi.b #0xb,(0x00fffbf9).l
0001FA8C: bne.w 0x0001fad6
0001FA90: lea (0x1220c0).l,A5 ; data 001220c0
0001FA96: move.b #0x1,(0x00fff0b6).l
0001FA9E: move.b #-0x1,(0x00fff0b7).l
0001FAA6: tst.b (0x00fff0d5).l
0001FAAC: beq.w 0x0001fac8
0001FAB0: bmi.w 0x0001fabe
0001FAB4: lea (0x111676).l,A0 ; data 00111676
0001FABA: bra.w 0x0001fd7a
0001FABE: lea (0x11166e).l,A0 ; data 0011166e
0001FAC4: bra.w 0x0001fd7a
0001FAC8: lea (0x110dd2).l,A0 ; data 00110dd2
0001FACE: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001FAD2: bra.w 0x0001fd6e
0001FAD6: cmpi.b #0x1,(0x00fffbf8).l
0001FADE: bne.w 0x0001fba2
0001FAE2: cmpi.b #0x5,(0x00fffbf9).l
0001FAEA: bne.w 0x0001fb54
0001FAEE: lea (0x122018).l,A5 ; data 00122018
0001FAF4: move.b #0x2,(0x00fff0b6).l
0001FAFC: tst.b (0x00fff0d5).l
0001FB02: beq.w 0x0001fb1e
0001FB06: bmi.w 0x0001fb14
0001FB0A: lea (0x11868e).l,A0 ; data 0011868e
0001FB10: bra.w 0x0001fd7a
0001FB14: lea (0x118686).l,A0 ; data 00118686
0001FB1A: bra.w 0x0001fd7a
0001FB1E: clr.b (0x00fff112).l
0001FB24: lea (0x11864a).l,A0 ; data 0011864a
0001FB2A: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001FB2E: tst.b (0x00fff112).l
0001FB34: beq.w 0x0001fd74
0001FB38: bmi.w 0x0001fb48
0001FB3C: move.b #0x2,(0x00fff0b7).l
0001FB44: bra.w 0x0001fd6e
0001FB48: move.b #-0x1,(0x00fff0b7).l
0001FB50: bra.w 0x0001fd6e
0001FB54: cmpi.b #0x3,(0x00fffbf9).l
0001FB5C: bne.w 0x0001fba2
0001FB60: lea (0x121f04).l,A5 ; data 00121f04
0001FB66: move.b #0x4,(0x00fff0b6).l
0001FB6E: move.b #0x0,(0x00fff0b7).l
0001FB76: tst.b (0x00fff0d5).l
0001FB7C: beq.w 0x0001fb98
0001FB80: bmi.w 0x0001fb8e
0001FB84: lea (0x111792).l,A0 ; data 00111792
0001FB8A: bra.w 0x0001fd7a
0001FB8E: lea (0x11178a).l,A0 ; data 0011178a
0001FB94: bra.w 0x0001fd7a
0001FB98: lea (0x110e7a).l,A0 ; data 00110e7a
0001FB9E: bra.w 0x0001face
0001FBA2: cmpi.b #0x5,(0x00fffbf8).l
0001FBAA: bne.w 0x0001fce6
0001FBAE: cmpi.b #0x2,(0x00fffbf9).l
0001FBB6: bne.w 0x0001fbfc
0001FBBA: lea (0x1220a8).l,A5 ; data 001220a8
0001FBC0: move.b #0x7,(0x00fff0b6).l
0001FBC8: move.b #0x1,(0x00fff0b7).l
0001FBD0: tst.b (0x00fff0d5).l
0001FBD6: beq.w 0x0001fbf2
0001FBDA: bmi.w 0x0001fbe8
0001FBDE: lea (0x11183e).l,A0 ; data 0011183e
0001FBE4: bra.w 0x0001fd7a
0001FBE8: lea (0x111836).l,A0 ; data 00111836
0001FBEE: bra.w 0x0001fd7a
0001FBF2: lea (0x110eaa).l,A0 ; data 00110eaa
0001FBF8: bra.w 0x0001face
0001FBFC: cmpi.b #0x3,(0x00fffbf9).l
0001FC04: bne.w 0x0001fc4a
0001FC08: lea (0x1220a8).l,A5 ; data 001220a8
0001FC0E: move.b #0x5,(0x00fff0b6).l
0001FC16: move.b #0x0,(0x00fff0b7).l
0001FC1E: tst.b (0x00fff0d5).l
0001FC24: beq.w 0x0001fc40
0001FC28: bmi.w 0x0001fc36
0001FC2C: lea (0x11184e).l,A0 ; data 0011184e
0001FC32: bra.w 0x0001fd7a
0001FC36: lea (0x111846).l,A0 ; data 00111846
0001FC3C: bra.w 0x0001fd7a
0001FC40: lea (0x110eb2).l,A0 ; data 00110eb2
0001FC46: bra.w 0x0001face
0001FC4A: cmpi.b #0x4,(0x00fffbf9).l
0001FC52: bne.w 0x0001fc98
0001FC56: lea (0x1220a8).l,A5 ; data 001220a8
0001FC5C: move.b #0x6,(0x00fff0b6).l
0001FC64: move.b #0x2,(0x00fff0b7).l
0001FC6C: tst.b (0x00fff0d5).l
0001FC72: beq.w 0x0001fc8e
0001FC76: bmi.w 0x0001fc84
0001FC7A: lea (0x11185e).l,A0 ; data 0011185e
0001FC80: bra.w 0x0001fd7a
0001FC84: lea (0x111856).l,A0 ; data 00111856
0001FC8A: bra.w 0x0001fd7a
0001FC8E: lea (0x110eba).l,A0 ; data 00110eba
0001FC94: bra.w 0x0001face
0001FC98: cmpi.b #0x5,(0x00fffbf9).l
0001FCA0: bne.w 0x0001fce6
0001FCA4: move.b #0x0,(0x00fff0b6).l
0001FCAC: move.b #-0x1,(0x00fff0b7).l
0001FCB4: lea (0x1220c0).l,A5 ; data 001220c0
0001FCBA: tst.b (0x00fff0d5).l
0001FCC0: beq.w 0x0001fcdc
0001FCC4: bmi.w 0x0001fcd2
0001FCC8: lea (0x11186e).l,A0 ; data 0011186e
0001FCCE: bra.w 0x0001fd7a
0001FCD2: lea (0x111866).l,A0 ; data 00111866
0001FCD8: bra.w 0x0001fd7a
0001FCDC: lea (0x110ec2).l,A0 ; data 00110ec2
0001FCE2: bra.w 0x0001face
0001FCE6: cmpi.b #0x4,(0x00fffbf8).l
0001FCEE: bne.w 0x0001fd78
0001FCF2: cmpi.b #0x3,(0x00fffbf9).l
0001FCFA: bne.w 0x0001fd78
0001FCFE: lea (0x121e08).l,A5 ; data 00121e08
0001FD04: tst.b (0x00fff0d5).l
0001FD0A: beq.w 0x0001fd26
0001FD0E: bmi.w 0x0001fd1c
0001FD12: lea (0x1118ea).l,A0 ; data 001118ea
0001FD18: bra.w 0x0001fd7a
0001FD1C: lea (0x1118e2).l,A0 ; data 001118e2
0001FD22: bra.w 0x0001fd7a
0001FD26: clr.b (0x00fff112).l
0001FD2C: move.l (0x00fffc1e).l,D1
0001FD32: btst.l #0x4,D1
0001FD36: bne.w 0x0001fd44
0001FD3A: lea (0x111876).l,A0 ; data 00111876
0001FD40: bra.w 0x0001fd4a
0001FD44: lea (0x111892).l,A0 ; data 00111892
0001FD4A: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001FD4E: tst.b (0x00fff112).l
0001FD54: beq.w 0x0001fd74
0001FD58: clr.b (0x00fff112).l
0001FD5E: move.b #0x3,(0x00fff0b6).l
0001FD66: move.b #0x0,(0x00fff0b7).l
0001FD6E: jsr 0x00056ac0.l ; call 00056AC0 FUN_00056ac0
0001FD74: move.w #-0x1,D6w
0001FD78: rts
0001FD7A: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001FD7E: bra.b 0x0001fd74
0001FD80: cmpi.b #0x6,(0x00fffbf8).l
0001FD88: bne.w 0x0001fdbe
0001FD8C: cmpi.b #0x0,(0x00fffbf9).l
0001FD94: bne.w 0x0001fdbe
0001FD98: move.l (0x00fffc30).l,D3
0001FD9E: bclr.l #0x9,D3
0001FDA2: move.l D3,(0x00fffc30).l
0001FDA8: lea (0xf4b4c).l,A0 ; data 000f4b4c
0001FDAE: lea (0x121e14).l,A5 ; data 00121e14
0001FDB4: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
0001FDB8: move.w #-0x1,D6w
0001FDBC: rts
0001FDBE: cmpi.b #0x7,(0x00fffbf8).l
0001FDC6: bne.w 0x0001fe20
0001FDCA: cmpi.b #0x0,(0x00fffbf9).l
0001FDD2: bne.w 0x0001fe20
0001FDD6: lea (0x121e08).l,A5 ; data 00121e08
0001FDDC: move.l (0x00fffc1e).l,D1
0001FDE2: btst.l #0x18,D1
0001FDE6: bne.w 0x0001fdfe
0001FDEA: bset.l #0x18,D1
0001FDEE: move.l D1,(0x00fffc1e).l
0001FDF4: lea (0x11b714).l,A0 ; data 0011b714
0001FDFA: bra.w 0x0001fe7a
0001FDFE: btst.l #0x19,D1
0001FE02: bne.w 0x0001fe10
0001FE06: lea (0x11b7e4).l,A0 ; data 0011b7e4
0001FE0C: bra.w 0x0001fe7a
0001FE10: jsr 0x00059bcc.l ; call 00059BCC FUN_00059bcc
0001FE16: lea (0x11b7ec).l,A0 ; data 0011b7ec
0001FE1C: bra.w 0x0001fe7a
0001FE20: cmpi.b #0x9,(0x00fffbf8).l
0001FE28: bne.w 0x0001fe86
0001FE2C: cmpi.b #0x0,(0x00fffbf9).l
0001FE34: bne.w 0x0001fe86
0001FE38: move.l (0x00fffc1e).l,D1
0001FE3E: btst.l #0x12,D1
0001FE42: bne.w 0x0001fe50
0001FE46: lea (0x11b5a0).l,A0 ; data 0011b5a0
0001FE4C: bra.w 0x0001fe74
0001FE50: move.l (0x00fffc22).l,D3
0001FE56: btst.l #0x5,D3
0001FE5A: bne.b 0x0001fe46
0001FE5C: btst.l #0x14,D1
0001FE60: bne.w 0x0001fe6e
0001FE64: lea (0x11b5bc).l,A0 ; data 0011b5bc
0001FE6A: bra.w 0x0001fe74
0001FE6E: lea (0x11b5d8).l,A0 ; data 0011b5d8
0001FE74: lea (0x122024).l,A5 ; data 00122024
0001FE7A: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0001FE80: move.w #-0x1,D6w
0001FE84: rts
0001FE86: clr.b (0x00fff0d6).l
0001FE8C: cmpi.b #0x0,(0x00fffbf8).l
0001FE94: bne.w 0x00020054
0001FE98: cmpi.b #0x1,(0x00fffbf9).l
0001FEA0: bne.w 0x0001feee
0001FEA4: lea (0x122024).l,A5 ; data 00122024
0001FEAA: tst.b (0x00fff0d5).l
0001FEB0: beq.w 0x0001fecc
0001FEB4: bmi.w 0x0001fec2
0001FEB8: lea (0x110ac2).l,A0 ; data 00110ac2
0001FEBE: bra.w 0x0001fed2
0001FEC2: lea (0x110aba).l,A0 ; data 00110aba
0001FEC8: bra.w 0x0001fed2
0001FECC: lea (0x110a96).l,A0 ; data 00110a96
0001FED2: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0001FED8: clr.b (0x00fff0d5).l
0001FEDE: tst.b (0x00fff0d6).l
0001FEE4: bmi.w 0x0001feec
0001FEE8: move.w #-0x1,D6w
0001FEEC: rts
0001FEEE: cmpi.b #0x2,(0x00fffbf9).l
0001FEF6: bne.w 0x0001ff26
0001FEFA: lea (0x121f34).l,A5 ; data 00121f34
0001FF00: tst.b (0x00fff0d5).l
0001FF06: beq.w 0x0001ff1e
0001FF0A: bmi.w 0x0001ff16
0001FF0E: lea (0x110ad2).l,A0 ; data 00110ad2
0001FF14: bra.b 0x0001fed2
0001FF16: lea (0x110aca).l,A0 ; data 00110aca
0001FF1C: bra.b 0x0001fed2
0001FF1E: lea (0x110ada).l,A0 ; data 00110ada
0001FF24: bra.b 0x0001fed2
0001FF26: cmpi.b #0x4,(0x00fffbf9).l
0001FF2E: bne.w 0x0001ff62
0001FF32: lea (0x12209c).l,A5 ; data 0012209c
0001FF38: tst.b (0x00fff0d5).l
0001FF3E: beq.w 0x0001ff58
0001FF42: bmi.w 0x0001ff4e
0001FF46: lea (0x11139a).l,A0 ; data 0011139a
0001FF4C: bra.b 0x0001fed2
0001FF4E: lea (0x111392).l,A0 ; data 00111392
0001FF54: bra.w 0x0001fed2
0001FF58: lea (0x110cb2).l,A0 ; data 00110cb2
0001FF5E: bra.w 0x0001fed2
0001FF62: cmpi.b #0x5,(0x00fffbf9).l
0001FF6A: bne.w 0x0001ffa0
0001FF6E: lea (0x122090).l,A5 ; data 00122090
0001FF74: tst.b (0x00fff0d5).l
0001FF7A: beq.w 0x0001ff96
0001FF7E: bmi.w 0x0001ff8c
0001FF82: lea (0x110b7e).l,A0 ; data 00110b7e
0001FF88: bra.w 0x0001fed2
0001FF8C: lea (0x110b76).l,A0 ; data 00110b76
0001FF92: bra.w 0x0001fed2
0001FF96: lea (0x110b86).l,A0 ; data 00110b86
0001FF9C: bra.w 0x0001fed2
0001FFA0: cmpi.b #0x6,(0x00fffbf9).l
0001FFA8: bne.w 0x0001ffe6
0001FFAC: lea (0x1220b4).l,A5 ; data 001220b4
0001FFB2: tst.b (0x00fff0d5).l
0001FFB8: beq.w 0x0001ffd4
0001FFBC: bmi.w 0x0001ffca
0001FFC0: lea (0x110b6e).l,A0 ; data 00110b6e
0001FFC6: bra.w 0x0001fed2
0001FFCA: lea (0x110b66).l,A0 ; data 00110b66
0001FFD0: bra.w 0x0001fed2
0001FFD4: move.b #-0x1,(0x00fff0d6).l
0001FFDC: lea (0x110b5e).l,A0 ; data 00110b5e
0001FFE2: bra.w 0x0001fed2
0001FFE6: cmpi.b #0x7,(0x00fffbf9).l
0001FFEE: bne.w 0x00020024
0001FFF2: lea (0x121f70).l,A5 ; data 00121f70
0001FFF8: tst.b (0x00fff0d5).l
0001FFFE: beq.w 0x0002001a
00020002: bmi.w 0x00020010
00020006: lea (0x110efa).l,A0 ; data 00110efa
0002000C: bra.w 0x0001fed2
00020010: lea (0x110ef2).l,A0 ; data 00110ef2
00020016: bra.w 0x0001fed2
0002001A: lea (0x111934).l,A0 ; data 00111934
00020020: bra.w 0x0001fed2
00020024: cmpi.b #0x8,(0x00fffbf9).l
0002002C: beq.w 0x0002003c
00020030: cmpi.b #0x9,(0x00fffbf9).l
00020038: bne.w 0x00020054
0002003C: lea (0x121e20).l,A5 ; data 00121e20
00020042: lea (0x11c40a).l,A0 ; data 0011c40a
00020048: move.b #-0x1,(0x00fff0d6).l
00020050: bra.w 0x0001fed2
00020054: cmpi.b #0x4,(0x00fffbf8).l
0002005C: bne.w 0x00020122
00020060: cmpi.b #0x1,(0x00fffbf9).l
00020068: bne.w 0x000200a6
0002006C: lea (0x121eec).l,A5 ; data 00121eec
00020072: tst.b (0x00fff0d5).l
00020078: beq.w 0x00020094
0002007C: bmi.w 0x0002008a
00020080: lea (0x110f12).l,A0 ; data 00110f12
00020086: bra.w 0x0001fed2
0002008A: lea (0x110f0a).l,A0 ; data 00110f0a
00020090: bra.w 0x0001fed2
00020094: move.b #-0x1,(0x00fff0d6).l
0002009C: lea (0x110f02).l,A0 ; data 00110f02
000200A2: bra.w 0x0001fed2
000200A6: cmpi.b #0x2,(0x00fffbf9).l
000200AE: bne.w 0x000200e4
000200B2: lea (0x121f34).l,A5 ; data 00121f34
000200B8: tst.b (0x00fff0d5).l
000200BE: beq.w 0x000200da
000200C2: bmi.w 0x000200d0
000200C6: lea (0x110d3e).l,A0 ; data 00110d3e
000200CC: bra.w 0x0001fed2
000200D0: lea (0x110d36).l,A0 ; data 00110d36
000200D6: bra.w 0x0001fed2
000200DA: lea (0x110d46).l,A0 ; data 00110d46
000200E0: bra.w 0x0001fed2
000200E4: cmpi.b #0x5,(0x00fffbf9).l
000200EC: bne.w 0x00020122
000200F0: lea (0x122054).l,A5 ; data 00122054
000200F6: tst.b (0x00fff0d5).l
000200FC: beq.w 0x00020118
00020100: bmi.w 0x0002010e
00020104: lea (0x110f5e).l,A0 ; data 00110f5e
0002010A: bra.w 0x0001fed2
0002010E: lea (0x110f56).l,A0 ; data 00110f56
00020114: bra.w 0x0001fed2
00020118: lea (0x110f1a).l,A0 ; data 00110f1a
0002011E: bra.w 0x0001fed2
00020122: cmpi.b #0x3,(0x00fffbf8).l
0002012A: bne.w 0x000202ce
0002012E: cmpi.b #0x0,(0x00fffbf9).l
00020136: bne.w 0x0002016c
0002013A: lea (0x121f40).l,A5 ; data 00121f40
00020140: tst.b (0x00fff0d5).l
00020146: beq.w 0x00020162
0002014A: bmi.w 0x00020158
0002014E: lea (0x110c5e).l,A0 ; data 00110c5e
00020154: bra.w 0x0001fed2
00020158: lea (0x110c42).l,A0 ; data 00110c42
0002015E: bra.w 0x0001fed2
00020162: lea (0x110c7a).l,A0 ; data 00110c7a
00020168: bra.w 0x0001fed2
0002016C: cmpi.b #0x2,(0x00fffbf9).l
00020174: bne.w 0x000201aa
00020178: lea (0x122090).l,A5 ; data 00122090
0002017E: tst.b (0x00fff0d5).l
00020184: beq.w 0x000201a0
00020188: bmi.w 0x00020196
0002018C: lea (0x111382).l,A0 ; data 00111382
00020192: bra.w 0x0001fed2
00020196: lea (0x11137a).l,A0 ; data 0011137a
0002019C: bra.w 0x0001fed2
000201A0: lea (0x110c9e).l,A0 ; data 00110c9e
000201A6: bra.w 0x0001fed2
000201AA: cmpi.b #0x3,(0x00fffbf9).l
000201B2: bne.w 0x000201e8
000201B6: lea (0x12209c).l,A5 ; data 0012209c
000201BC: tst.b (0x00fff0d5).l
000201C2: beq.w 0x000201de
000201C6: bmi.w 0x000201d4
000201CA: lea (0x110bbe).l,A0 ; data 00110bbe
000201D0: bra.w 0x0001fed2
000201D4: lea (0x110ba2).l,A0 ; data 00110ba2
000201DA: bra.w 0x0001fed2
000201DE: lea (0x110bd2).l,A0 ; data 00110bd2
000201E4: bra.w 0x0001fed2
000201E8: cmpi.b #0x4,(0x00fffbf9).l
000201F0: bne.w 0x00020226
000201F4: lea (0x122018).l,A5 ; data 00122018
000201FA: tst.b (0x00fff0d5).l
00020200: beq.w 0x0002021c
00020204: bmi.w 0x00020212
00020208: lea (0x110c0a).l,A0 ; data 00110c0a
0002020E: bra.w 0x0001fed2
00020212: lea (0x110bee).l,A0 ; data 00110bee
00020218: bra.w 0x0001fed2
0002021C: lea (0x110c26).l,A0 ; data 00110c26
00020222: bra.w 0x0001fed2
00020226: cmpi.b #0x5,(0x00fffbf9).l
0002022E: bne.w 0x0002026c
00020232: lea (0x122060).l,A5 ; data 00122060
00020238: tst.b (0x00fff0d5).l
0002023E: beq.w 0x0002025a
00020242: bmi.w 0x00020250
00020246: lea (0x110ce2).l,A0 ; data 00110ce2
0002024C: bra.w 0x0001fed2
00020250: lea (0x110cce).l,A0 ; data 00110cce
00020256: bra.w 0x0001fed2
0002025A: move.b #-0x1,(0x00fff0d6).l
00020262: lea (0x110cc6).l,A0 ; data 00110cc6
00020268: bra.w 0x0001fed2
0002026C: cmpi.b #0x6,(0x00fffbf9).l
00020274: bne.w 0x000202aa
00020278: lea (0x122084).l,A5 ; data 00122084
0002027E: tst.b (0x00fff0d5).l
00020284: beq.w 0x000202a0
00020288: bmi.w 0x00020296
0002028C: lea (0x110f82).l,A0 ; data 00110f82
00020292: bra.w 0x0001fed2
00020296: lea (0x110f7a).l,A0 ; data 00110f7a
0002029C: bra.w 0x0001fed2
000202A0: lea (0x110f66).l,A0 ; data 00110f66
000202A6: bra.w 0x0001fed2
000202AA: cmpi.b #0x7,(0x00fffbf9).l
000202B2: beq.w 0x0002003c
000202B6: cmpi.b #0x8,(0x00fffbf9).l
000202BE: beq.w 0x0002003c
000202C2: cmpi.b #0x9,(0x00fffbf9).l
000202CA: beq.w 0x0002003c
000202CE: cmpi.b #0x1,(0x00fffbf8).l
000202D6: bne.w 0x0002041e
000202DA: cmpi.b #0x2,(0x00fffbf9).l
000202E2: bne.w 0x00020318
000202E6: lea (0x1220c0).l,A5 ; data 001220c0
000202EC: tst.b (0x00fff0d5).l
000202F2: beq.w 0x0002030e
000202F6: bmi.w 0x00020304
000202FA: lea (0x111676).l,A0 ; data 00111676
00020300: bra.w 0x0001fed2
00020304: lea (0x11166e).l,A0 ; data 0011166e
0002030A: bra.w 0x0001fed2
0002030E: lea (0x110dbe).l,A0 ; data 00110dbe
00020314: bra.w 0x0001fed2
00020318: cmpi.b #0x0,(0x00fffbf9).l
00020320: bne.w 0x0002038a
00020324: lea (0x121e50).l,A5 ; data 00121e50
0002032A: tst.b (0x00fff0d5).l
00020330: beq.w 0x0002034c
00020334: bmi.w 0x00020342
00020338: lea (0x11165e).l,A0 ; data 0011165e
0002033E: bra.w 0x0001fed2
00020342: lea (0x111656).l,A0 ; data 00111656
00020348: bra.w 0x0001fed2
0002034C: move.w #0x64,D1w
00020350: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00020356: cmpi.w #0x4b,D0w
0002035A: bmi.w 0x00020378
0002035E: lea (0x1d6ed8).l,A6 ; data 001d6ed8
00020364: move.w #0x1,D0w
00020368: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
0002036E: lea (0x110da2).l,A0 ; data 00110da2
00020374: bra.w 0x0001fed2
00020378: move.b #-0x1,(0x00fff0d6).l
00020380: lea (0x110db6).l,A0 ; data 00110db6
00020386: bra.w 0x0001fed2
0002038A: cmpi.b #0x4,(0x00fffbf9).l
00020392: bne.w 0x000203c8
00020396: lea (0x122060).l,A5 ; data 00122060
0002039C: tst.b (0x00fff0d5).l
000203A2: beq.w 0x000203be
000203A6: bmi.w 0x000203b4
000203AA: lea (0x110dee).l,A0 ; data 00110dee
000203B0: bra.w 0x0001fed2
000203B4: lea (0x110dda).l,A0 ; data 00110dda
000203BA: bra.w 0x0001fed2
000203BE: lea (0x110e02).l,A0 ; data 00110e02
000203C4: bra.w 0x0001fed2
000203C8: cmpi.b #0x9,(0x00fffbf9).l
000203D0: bne.w 0x00020406
000203D4: lea (0x122054).l,A5 ; data 00122054
000203DA: tst.b (0x00fff0d5).l
000203E0: beq.w 0x000203fc
000203E4: bmi.w 0x000203f2
000203E8: lea (0x110e4a).l,A0 ; data 00110e4a
000203EE: bra.w 0x0001fed2
000203F2: lea (0x110e36).l,A0 ; data 00110e36
000203F8: bra.w 0x0001fed2
000203FC: lea (0x110e5e).l,A0 ; data 00110e5e
00020402: bra.w 0x0001fed2
00020406: cmpi.b #0x8,(0x00fffbf9).l
0002040E: beq.w 0x0002003c
00020412: cmpi.b #0xa,(0x00fffbf9).l
0002041A: beq.w 0x0002003c
0002041E: cmpi.b #0x2,(0x00fffbf8).l
00020426: bne.w 0x000205aa
0002042A: cmpi.b #0x0,(0x00fffbf9).l
00020432: bne.w 0x00020446
00020436: lea (0x122030).l,A5 ; data 00122030
0002043C: lea (0x1113d2).l,A0 ; data 001113d2
00020442: bra.w 0x0001fed2
00020446: cmpi.b #0x3,(0x00fffbf9).l
0002044E: bne.w 0x00020484
00020452: lea (0x122030).l,A5 ; data 00122030
00020458: tst.b (0x00fff0d5).l
0002045E: beq.w 0x0002047a
00020462: bmi.w 0x00020470
00020466: lea (0x1114a2).l,A0 ; data 001114a2
0002046C: bra.w 0x0001fed2
00020470: lea (0x11149a).l,A0 ; data 0011149a
00020476: bra.w 0x0001fed2
0002047A: lea (0x110d0a).l,A0 ; data 00110d0a
00020480: bra.w 0x0001fed2
00020484: cmpi.b #0x4,(0x00fffbf9).l
0002048C: bne.w 0x000204ca
00020490: lea (0x121ebc).l,A5 ; data 00121ebc
00020496: tst.b (0x00fff0d5).l
0002049C: beq.w 0x000204b8
000204A0: bmi.w 0x000204ae
000204A4: lea (0x1114ba).l,A0 ; data 001114ba
000204AA: bra.w 0x0001fed2
000204AE: lea (0x1114b2).l,A0 ; data 001114b2
000204B4: bra.w 0x0001fed2
000204B8: move.b #-0x1,(0x00fff0d6).l
000204C0: lea (0x110d1e).l,A0 ; data 00110d1e
000204C6: bra.w 0x0001fed2
000204CA: cmpi.b #0x5,(0x00fffbf9).l
000204D2: bne.w 0x00020510
000204D6: lea (0x121e38).l,A5 ; data 00121e38
000204DC: tst.b (0x00fff0d5).l
000204E2: beq.w 0x000204fe
000204E6: bmi.w 0x000204f4
000204EA: lea (0x1114d2).l,A0 ; data 001114d2
000204F0: bra.w 0x0001fed2
000204F4: lea (0x1114ca).l,A0 ; data 001114ca
000204FA: bra.w 0x0001fed2
000204FE: move.b #-0x1,(0x00fff0d6).l
00020506: lea (0x110d26).l,A0 ; data 00110d26
0002050C: bra.w 0x0001fed2
00020510: cmpi.b #0x7,(0x00fffbf9).l
00020518: bne.w 0x00020554
0002051C: lea (0x122048).l,A5 ; data 00122048
00020522: tst.b (0x00fff0d5).l
00020528: beq.w 0x0002054a
0002052C: bmi.w 0x00020540
00020530: lea (0x122024).l,A5 ; data 00122024
00020536: lea (0x1114e2).l,A0 ; data 001114e2
0002053C: bra.w 0x0001fed2
00020540: lea (0x1114da).l,A0 ; data 001114da
00020546: bra.w 0x0001fed2
0002054A: lea (0x110d2e).l,A0 ; data 00110d2e
00020550: bra.w 0x0001fed2
00020554: cmpi.b #0x9,(0x00fffbf9).l
0002055C: bne.w 0x00020592
00020560: lea (0x121e38).l,A5 ; data 00121e38
00020566: tst.b (0x00fff0d5).l
0002056C: beq.w 0x00020588
00020570: bmi.w 0x0002057e
00020574: lea (0x110d72).l,A0 ; data 00110d72
0002057A: bra.w 0x0001fed2
0002057E: lea (0x11155e).l,A0 ; data 0011155e
00020584: bra.w 0x0001fed2
00020588: lea (0x110d86).l,A0 ; data 00110d86
0002058E: bra.w 0x0001fed2
00020592: cmpi.b #0x8,(0x00fffbf9).l
0002059A: beq.w 0x0002003c
0002059E: cmpi.b #0xa,(0x00fffbf9).l
000205A6: beq.w 0x0002003c
000205AA: cmpi.b #0x5,(0x00fffbf8).l
000205B2: bne.w 0x00020632
000205B6: cmpi.b #0x1,(0x00fffbf9).l
000205BE: bne.w 0x000205f4
000205C2: lea (0x121e8c).l,A5 ; data 00121e8c
000205C8: tst.b (0x00fff0d5).l
000205CE: beq.w 0x000205ea
000205D2: bmi.w 0x000205e0
000205D6: lea (0x1117fa).l,A0 ; data 001117fa
000205DC: bra.w 0x0001fed2
000205E0: lea (0x1117f2).l,A0 ; data 001117f2
000205E6: bra.w 0x0001fed2
000205EA: lea (0x110e96).l,A0 ; data 00110e96
000205F0: bra.w 0x0001fed2
000205F4: cmpi.b #0x6,(0x00fffbf9).l
000205FC: bne.w 0x00020632
00020600: lea (0x121f88).l,A5 ; data 00121f88
00020606: tst.b (0x00fff0d5).l
0002060C: beq.w 0x00020628
00020610: bmi.w 0x0002061e
00020614: lea (0x1117ce).l,A0 ; data 001117ce
0002061A: bra.w 0x0001fed2
0002061E: lea (0x1117b2).l,A0 ; data 001117b2
00020624: bra.w 0x0001fed2
00020628: lea (0x110e82).l,A0 ; data 00110e82
0002062E: bra.w 0x0001fed2
00020632: rts
00020634: cmpi.b #0x0,(0x00fffbf8).l
0002063C: bne.w 0x0002066c
00020640: cmpi.b #0x4,(0x00fffbf9).l
00020648: bne.w 0x00020656
0002064C: move.b #0x2,(0x00fff0b4).l
00020654: rts
00020656: cmpi.b #0x5,(0x00fffbf9).l
0002065E: bne.w 0x00020712
00020662: move.b #0x1,(0x00fff0b4).l
0002066A: rts
0002066C: cmpi.b #0x3,(0x00fffbf8).l
00020674: bne.w 0x000206ba
00020678: cmpi.b #0x2,(0x00fffbf9).l
00020680: bne.w 0x0002068e
00020684: move.b #0x1,(0x00fff0b4).l
0002068C: rts
0002068E: cmpi.b #0x3,(0x00fffbf9).l
00020696: bne.w 0x000206a4
0002069A: move.b #0x2,(0x00fff0b4).l
000206A2: rts
000206A4: cmpi.b #0x6,(0x00fffbf9).l
000206AC: bne.w 0x00020712
000206B0: move.b #0x0,(0x00fff0b4).l
000206B8: rts
000206BA: cmpi.b #0x1,(0x00fffbf8).l
000206C2: bne.w 0x000206dc
000206C6: cmpi.b #0x2,(0x00fffbf9).l
000206CE: bne.w 0x00020712
000206D2: move.b #0x4,(0x00fff0b4).l
000206DA: rts
000206DC: cmpi.b #0x2,(0x00fffbf8).l
000206E4: bne.w 0x000206fe
000206E8: cmpi.b #0x0,(0x00fffbf9).l
000206F0: bne.w 0x00020712
000206F4: move.b #0x0,(0x00fff0b4).l
000206FC: rts
000206FE: cmpi.b #0x5,(0x00fffbf8).l
00020706: bne.w 0x00020712
0002070A: move.b #0x3,(0x00fff0b4).l
00020712: rts
00020714: move.l (0x00fffc1e).l,D1
0002071A: btst.l #0x0,D1
0002071E: bne.w 0x0002072c
00020722: lea (0xfe620).l,A0 ; data 000fe620
00020728: bra.w 0x00020758
0002072C: clr.l D0
0002072E: move.b (0x00fff09c).l,D0b
00020734: lea (0xd445d).l,A1 ; data 000d445d
0002073A: move.b (0x0,A1,D0w*0x1),D0b ; data 000d445d
0002073E: cmp.b (0x00fffbf8).l,D0b
00020744: bne.w 0x00020752
00020748: lea (0xfe628).l,A0 ; data 000fe628
0002074E: bra.w 0x00020758
00020752: lea (0xfe5d4).l,A0 ; data 000fe5d4
00020758: lea (0x121f10).l,A5 ; data 00121f10
0002075E: move.b #0x0,(0x00fff10c).l
00020766: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020882: mulu.w (A2)+,D0
00020884: adda.l #0x22,A2
0002088A: clr.l D1
0002088C: move.b (0x00fff0e0).l,D1b
00020892: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00020898: lea (0xfe5ec).l,A0 ; data 000fe5ec
0002089E: move.b #-0x1,(0x00fff10c).l
000208A6: rts
00020A00: lsr.l #0x3,D4
00020A02: lea (0x121e20).l,A5 ; data 00121e20
00020A08: lea (0x105370).l,A0 ; data 00105370
00020A0E: bra.w 0x00020c8a
00020C8A: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020C8E: rts
00020D00: move.b #0x15,(0x0,A5,D0w*0x1)
00020D06: lea (0x12cc28).l,A0 ; data 0012cc28
00020D0C: bra.w 0x00020e32
00020E32: move.b #0x0,(0x00fff10c).l
00020E3A: move.b #-0x1,(0x00fff0b5).l
00020E42: bsr.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020E46: rts
00020EFA: lea (0x120e0c).l,A0 ; data 00120e0c
00020F00: lea (0x1220b4).l,A5 ; data 001220b4
00020F06: move.b #0x0,(0x00fff10c).l
00020F0E: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020F12: lea (0x120e54).l,A0 ; data 00120e54
00020F18: lea (0x122084).l,A5 ; data 00122084
00020F1E: move.b #0x0,(0x00fff10c).l
00020F26: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020F2A: lea (0x120f00).l,A0 ; data 00120f00
00020F30: lea (0x121df0).l,A5 ; data 00121df0
00020F36: move.b #0x0,(0x00fff10c).l
00020F3E: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020F42: lea (0x120f6c).l,A0 ; data 00120f6c
00020F48: lea (0x1220b4).l,A5 ; data 001220b4
00020F4E: move.b #0x0,(0x00fff10c).l
00020F56: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020F5A: lea (0x120fac).l,A0 ; data 00120fac
00020F60: lea (0x121f40).l,A5 ; data 00121f40
00020F66: move.b #0x0,(0x00fff10c).l
00020F6E: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020F72: lea (0x121004).l,A0 ; data 00121004
00020F78: lea (0x121f40).l,A5 ; data 00121f40
00020F7E: move.b #0x0,(0x00fff10c).l
00020F86: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020F8A: lea (0x121078).l,A0 ; data 00121078
00020F90: lea (0x12203c).l,A5 ; data 0012203c
00020F96: move.b #0x0,(0x00fff10c).l
00020F9E: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020FA2: lea (0x121108).l,A0 ; data 00121108
00020FA8: lea (0x12203c).l,A5 ; data 0012203c
00020FAE: move.b #0x0,(0x00fff10c).l
00020FB6: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020FBA: lea (0x1211bc).l,A0 ; data 001211bc
00020FC0: move.b #0x0,(0x00fff10c).l
00020FC8: move.b #-0x1,(0x00fff0b5).l
00020FD0: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020FD4: lea (0x1211f4).l,A0 ; data 001211f4
00020FDA: lea (0x121f34).l,A5 ; data 00121f34
00020FE0: move.b #0x0,(0x00fff10c).l
00020FE8: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00020FEC: lea (0x121238).l,A0 ; data 00121238
00020FF2: lea (0x122030).l,A5 ; data 00122030
00020FF8: move.b #0x0,(0x00fff10c).l
00021000: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00021004: lea (0x12130c).l,A0 ; data 0012130c
0002100A: move.b #0x0,(0x00fff10c).l
00021012: move.b #-0x1,(0x00fff0b5).l
0002101A: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
0002101E: lea (0x121330).l,A0 ; data 00121330
00021024: lea (0x121e5c).l,A5 ; data 00121e5c
0002102A: move.b #0x0,(0x00fff10c).l
00021032: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00021036: lea (0x121384).l,A0 ; data 00121384
0002103C: move.b #0x0,(0x00fff10c).l
00021044: move.b #-0x1,(0x00fff0b5).l
0002104C: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00021050: lea (0x1213a8).l,A0 ; data 001213a8
00021056: move.b #0x0,(0x00fff10c).l
0002105E: move.b #-0x1,(0x00fff0b5).l
00021066: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
0002106A: lea (0x1213cc).l,A0 ; data 001213cc
00021070: lea (0x122048).l,A5 ; data 00122048
00021076: move.b #0x0,(0x00fff10c).l
0002107E: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00021082: lea (0x121414).l,A0 ; data 00121414
00021088: lea (0x121f94).l,A5 ; data 00121f94
0002108E: move.b #0x0,(0x00fff10c).l
00021096: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
0002109A: lea (0x1214ac).l,A0 ; data 001214ac
000210A0: move.b #0x0,(0x00fff10c).l
000210A8: move.b #-0x1,(0x00fff0b5).l
000210B0: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
000210B4: lea (0x1214f4).l,A0 ; data 001214f4
000210BA: lea (0x121e2c).l,A5 ; data 00121e2c
000210C0: move.b #0x0,(0x00fff10c).l
000210C8: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
000210CC: lea (0x121558).l,A0 ; data 00121558
000210D2: lea (0x12206c).l,A5 ; data 0012206c
000210D8: move.b #0x0,(0x00fff10c).l
000210E0: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
000210E4: lea (0x121574).l,A0 ; data 00121574
000210EA: lea (0x12206c).l,A5 ; data 0012206c
000210F0: move.b #0x0,(0x00fff10c).l
000210F8: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00021112: roxl.b D0,D4b
00021114: lea (0x121588).l,A0 ; data 00121588
0002111A: lea (0x121e20).l,A5 ; data 00121e20
00021120: move.b #0x0,(0x00fff10c).l
00021128: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
0002112C: lea (0x121604).l,A0 ; data 00121604
00021132: lea (0x12203c).l,A5 ; data 0012203c
00021138: move.b #0x0,(0x00fff10c).l
00021140: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
00021144: lea (0x121688).l,A0 ; data 00121688
0002114A: lea (0x121e08).l,A5 ; data 00121e08
00021150: move.b #0x0,(0x00fff10c).l
00021158: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
0002115C: lea (0x120e30).l,A0 ; data 00120e30
00021162: lea (0x121e20).l,A5 ; data 00121e20
00021168: move.b #0x0,(0x00fff10c).l
00021170: bra.w 0x0001f246 ; call 0001F246 FUN_0001f246
000213CA: move.l (0x00fffc30).l,D1
000213D0: bset.l #0x1,D1
000213D4: move.l D1,(0x00fffc30).l
000213DA: lea (0xf4e8c).l,A0 ; data 000f4e8c
000213E0: rts
000213E2: move.b (0x00fffc27).l,D1b
000213E8: bset.l #0x0,D1
000213EC: move.b D1b,(0x00fffc27).l
000213F2: lea (0xf5ab0).l,A0 ; data 000f5ab0
000213F8: rts
00021458: move.l (0x00fffbfe).l,D0
0002145E: clr.l D1
00021460: move.b (0x00fff10d).l,D1b
00021466: sub.l D1,D0
00021468: bpl.w 0x00021474
0002146C: lea (0xf5af8).l,A0 ; data 000f5af8
00021472: rts
00021474: move.l D0,(0x00fffbfe).l
0002147A: move.b #-0x1,(0x00fff10d).l
00021482: lea (0xf5ab8).l,A0 ; data 000f5ab8
00021488: rts
0002148A: jsr 0x0001e69c.l ; call 0001E69C FUN_0001e69c
00021490: cmpi.w #0x6,D6w
00021494: bmi.w 0x000214b0
00021498: cmpi.w #0x9,D6w
0002149C: bpl.w 0x000214b0
000214A0: lea (0x1059fc).l,A0 ; data 001059fc
000214A6: subq.w #0x6,D6w
000214A8: lsl.w #0x2,D6w
000214AA: movea.l (0x0,A0,D6w*0x1),A0 ; data 001059fc
000214AE: rts
000214B0: lea (0xf3e1e).l,A0 ; data 000f3e1e
000214B6: clr.l D0
000214B8: move.b (0x00fffbed).l,D0b
000214BE: lsl.w #0x3,D0w
000214C0: movea.l (0x0,A0,D0w*0x1),A0 ; data 000f3e1e
000214C4: jsr (A0)
000214C6: lea (0x1058a4).l,A0 ; data 001058a4
000214CC: rts
0002155E: jsr 0x0001e68c.l ; call 0001E68C FUN_0001e68c
00021564: lea (0x1058e0).l,A0 ; data 001058e0
0002156A: rts
00021846: move.l #0xe204,D4
0002184C: move.w #0x6,D7w
00021850: move.w #0x8,D1w
00021854: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
0002185A: lea (0xf4eb8).l,A0 ; data 000f4eb8
00021860: rts
000218B2: lea (0xf5150).l,A0 ; data 000f5150
000218B8: bra.w 0x0002196c ; call 0002196C FUN_0002196c
000218BC: lea (0xf4ea4).l,A0 ; data 000f4ea4
000218C2: bra.w 0x0002196c ; call 0002196C FUN_0002196c
000218C6: move.w #0x1,D0w
000218CA: jsr 0x00059c34.l ; call 00059C34 FUN_00059c34
000218D0: lea (0xf5064).l,A0 ; data 000f5064
000218D6: bra.w 0x0002196c ; call 0002196C FUN_0002196c
000218DA: lea (0xf4bd0).l,A0 ; data 000f4bd0
000218E0: bra.w 0x0002196c ; call 0002196C FUN_0002196c
000218E4: lea (0x121e08).l,A5 ; data 00121e08
000218EA: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
000218EE: lea (0xf50e4).l,A0 ; data 000f50e4
000218F4: bra.w 0x0002196c ; call 0002196C FUN_0002196c
000218F8: lea (0xf51a8).l,A0 ; data 000f51a8
000218FE: bra.w 0x0002196c ; call 0002196C FUN_0002196c
0002196C: move.l (0x00ffdd64).l,D0
00021972: move.l (0x00fffc1e).l,D1
00021978: bset.l D0,D1
0002197A: move.l D1,(0x00fffc1e).l
00021980: rts
00021982: move.b #-0x1,(0x00fff112).l
0002198A: lea (0x1118ca).l,A0 ; data 001118ca
00021990: rts
000219A4: lea (0x121e14).l,A5 ; data 00121e14
000219AA: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
000219AE: lea (0xf4fc8).l,A0 ; data 000f4fc8
000219B4: rts
000219B6: lea (0x121e98).l,A5 ; data 00121e98
000219BC: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
000219C0: lea (0xf48f4).l,A0 ; data 000f48f4
000219C6: rts
000219C8: lea (0x121e68).l,A5 ; data 00121e68
000219CE: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
000219D2: lea (0xf48e0).l,A0 ; data 000f48e0
000219D8: rts
000219DA: move.b #-0x1,(0x00fff112).l
000219E2: lea (0x118666).l,A0 ; data 00118666
000219E8: rts
00021A4E: cmp.l (0x00fffbfe).l,D1
00021A54: bgt.w 0x00021a64
00021A58: sub.l D1,(0x00fffbfe).l
00021A5E: movea.l A1,A0
00021A60: move.w #-0x1,D1w
00021A64: rts
00021A66: moveq #0x64,D1
00021A68: lea (0xf5ad8).l,A0 ; data 000f5ad8
00021A6E: lea (0xf52d8).l,A1 ; data 000f52d8
00021A74: bsr.b 0x00021a4e ; call 00021A4E FUN_00021a4e
00021A76: bgt.w 0x00021a82
00021A7A: bset.b #0x0,(0x00fffc26).l
00021A82: rts
00021B6A: move.l (0x00fffc34).l,D1
00021B70: bset.l #0x0,D1
00021B74: move.l D1,(0x00fffc34).l
00021B7A: lea (0xf54c4).l,A0 ; data 000f54c4
00021B80: rts
00021BE4: jsr 0x0001f1e0.l ; call 0001F1E0 FUN_0001f1e0
00021BEA: lea (0x1d6fd0).l,A6 ; data 001d6fd0
00021BF0: move.w #0x1,D0w
00021BF4: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00021BFA: lea (0x12206c).l,A5 ; data 0012206c
00021C00: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00021C04: lea (0xf546c).l,A0 ; data 000f546c
00021C0A: rts
00021CAA: move.l (0x00fffc1e).l,D1
00021CB0: btst.l #0xd,D1
00021CB4: beq.w 0x00021cdc
00021CB8: btst.l #0x12,D1
00021CBC: bne.w 0x00021cc8
00021CC0: lea (0x11b610).l,A0 ; data 0011b610
00021CC6: rts
00021CC8: move.l (0x00fffc22).l,D0
00021CCE: btst.l #0x5,D0
00021CD2: bne.b 0x00021cc0
00021CD4: lea (0x11b62c).l,A0 ; data 0011b62c
00021CDA: rts
00021CDC: lea (0x11b680).l,A0 ; data 0011b680
00021CE2: rts
00021E08: move.w #0x2,D0w
00021E0C: jsr 0x00059c34.l ; call 00059C34 FUN_00059c34
00021E12: move.l (0x00fffc30).l,D1
00021E18: bclr.l #0x17,D1
00021E1C: bclr.l #0x1c,D1
00021E20: move.l D1,(0x00fffc30).l
00021E26: move.l (0x00fffc34).l,D1
00021E2C: bclr.l #0x1,D1
00021E30: bclr.l #0x3,D1
00021E34: bset.l #0x2,D1
00021E38: move.l D1,(0x00fffc34).l
00021E3E: move.l (0x00fffc38).l,D1
00021E44: bclr.l #0x12,D1
00021E48: bclr.l #0x14,D1
00021E4C: move.l D1,(0x00fffc38).l
00021E52: lea (0xf56cc).l,A0 ; data 000f56cc
00021E58: move.l #0x14,(0x00ffdd64).l
00021E62: bra.w 0x0002196c ; call 0002196C FUN_0002196c
00021E66: move.l #0x3e8,D1
00021E6C: lea (0x11b70c).l,A0 ; data 0011b70c
00021E72: lea (0x11b704).l,A1 ; data 0011b704
00021E78: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00021E7C: bgt.w 0x00021e8a
00021E80: move.w #0x6,D0w
00021E84: jsr 0x000136de.l ; call 000136DE FUN_000136de
00021E8A: rts
00021E8C: lea (0x122024).l,A5 ; data 00122024
00021E92: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00021E96: lea (0x11b6d4).l,A0 ; data 0011b6d4
00021E9C: rts
00021E9E: lea (0x122024).l,A5 ; data 00122024
00021EA4: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00021EA8: lea (0x11b6ec).l,A0 ; data 0011b6ec
00021EAE: rts
00021EE8: move.b #0x1,(0x00fff0d5).l
00021EF0: lea (0x111916).l,A0 ; data 00111916
00021EF6: rts
00021F6E: lea (0x121e14).l,A5 ; data 00121e14
00021F74: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00021F78: lea (0xf50f8).l,A0 ; data 000f50f8
00021F7E: rts
00021FB8: jsr 0x0001098e.l ; call 0001098E FUN_0001098e
00021FBE: lea (0x121e08).l,A5 ; data 00121e08
00021FC4: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00021FC8: lea (0x11b7b4).l,A0 ; data 0011b7b4
00021FCE: rts
00021FF2: move.l (0x00fffc34).l,D1
00021FF8: bclr.l #0x5,D1
00021FFC: bset.l #0x6,D1
00022000: move.l D1,(0x00fffc34).l
00022006: lea (0x121e74).l,A5 ; data 00121e74
0002200C: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00022010: lea (0xf58ac).l,A0 ; data 000f58ac
00022016: rts
00022100: ori.b #0x30,D5b
00022104: move.l (SP)+,D0
00022106: rts
00022108: jsr 0x0005d172.l ; call 0005D172 FUN_0005d172
0002210E: move.l (SP)+,D0
00022110: rts
00022112: lea (0x12206c).l,A5 ; data 0012206c
00022118: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
0002211C: lea (0xf4fa0).l,A0 ; data 000f4fa0
00022122: rts
00022126: ori.b #0x7c,(A2)
0002212A: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
0002212E: lea (0xf4fb4).l,A0 ; data 000f4fb4
00022134: rts
000221BA: move.l #0x5dc,D1
000221C0: lea (0x10560c).l,A0 ; data 0010560c
000221C6: lea (0x105640).l,A1 ; data 00105640
000221CC: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
000221D0: bgt.w 0x000221e4
000221D4: move.l (0x00fffc2c).l,D1
000221DA: bset.l #0x1,D1
000221DE: move.l D1,(0x00fffc2c).l
000221E4: rts
00022276: moveq #0x19,D1
00022278: lea (0x11181e).l,A0 ; data 0011181e
0002227E: lea (0x111826).l,A1 ; data 00111826
00022284: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00022288: bgt.w 0x00022294
0002228C: move.b #-0x1,(0x00fff0d6).l
00022294: rts
000222AA: move.b #-0x1,(0x00fff0d6).l
000222B2: lea (0x1116aa).l,A0 ; data 001116aa
000222B8: rts
000222BA: moveq #0x64,D1
000222BC: lea (0x1116ca).l,A0 ; data 001116ca
000222C2: lea (0x1116ba).l,A1 ; data 001116ba
000222C8: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
000222CC: bgt.w 0x000222d8
000222D0: move.b #-0x1,(0x00fff0d6).l
000222D8: rts
00022332: btst.b D1,(A0)
00022334: beq.w 0x00022354
00022338: cmpi.b #0x1,(0x00ff039a).l
00022340: bne.w 0x00022354
00022344: move.b #-0x1,(0x00fff0d6).l
0002234C: lea (0x1110ea).l,A0 ; data 001110ea
00022352: rts
00022354: lea (0x1110fa).l,A0 ; data 001110fa
0002235A: rts
00022452: cmpi.b #0x6,(0x00ff018b).l
0002245A: bmi.w 0x000224ac
0002245E: move.b #-0x1,(0x00fff0d6).l
00022466: move.l (0x00fffc1e).l,D1
0002246C: btst.l #0xf,D1
00022470: beq.w 0x000224a2
00022474: btst.l #0x10,D1
00022478: bne.w 0x000224a2
0002247C: move.l #0x1e5c4,(0x00ffe120).l
00022486: lea (0x121e2c).l,A5 ; data 00121e2c
0002248C: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00022490: lea (0x110b22).l,A0 ; data 00110b22
00022496: move.b #-0x1,(0x00fff0cc).l
0002249E: clr.w D6w
000224A0: rts
000224A2: lea (0x111176).l,A0 ; data 00111176
000224A8: clr.w D6w
000224AA: rts
000224AC: lea (0x11115a).l,A0 ; data 0011115a
000224B2: rts
000224B4: move.l #0x96,D1
000224BA: lea (0x111186).l,A0 ; data 00111186
000224C0: lea (0x111826).l,A1 ; data 00111826
000224C6: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
000224CA: bgt.w 0x00022510
000224CE: move.b #-0x1,(0x00fff0d6).l
000224D6: move.l (0x00fffc1e).l,D1
000224DC: btst.l #0xf,D1
000224E0: beq.w 0x00022510
000224E4: btst.l #0x10,D1
000224E8: bne.w 0x00022510
000224EC: move.l #0x1e5c4,(0x00ffe120).l
000224F6: lea (0x121e2c).l,A5 ; data 00121e2c
000224FC: bsr.w 0x0001f292 ; call 0001F292 FUN_0001f292
00022500: lea (0x110b22).l,A0 ; data 00110b22
00022506: move.b #-0x1,(0x00fff0cc).l
0002250E: clr.w D6w
00022510: rts
00022512: move.l #0x3e8,D1
00022518: lea (0x11127a).l,A0 ; data 0011127a
0002251E: lea (0x1112b2).l,A1 ; data 001112b2
00022524: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00022528: rts
0002252A: move.l #0x3e8,D1
00022530: lea (0x11127a).l,A0 ; data 0011127a
00022536: lea (0x111296).l,A1 ; data 00111296
0002253C: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00022540: rts
0002256A: cmpi.l #0x1f4,(0x00fffbfe).l
00022574: bpl.w 0x00022580
00022578: lea (0x111532).l,A0 ; data 00111532
0002257E: rts
00022580: cmpi.b #0x3,(0x00fffbfa).l
00022588: bmi.w 0x00022594
0002258C: lea (0x111516).l,A0 ; data 00111516
00022592: rts
00022594: move.l (0x00fffc22).l,D0
0002259A: bset.l #0x1,D0
0002259E: move.l D0,(0x00fffc22).l
000225A4: subi.l #0x1f4,(0x00fffbfe).l
000225AE: lea (0x11153a).l,A0 ; data 0011153a
000225B4: rts
00022688: move.l #0x2ee,D1
0002268E: lea (0x111722).l,A0 ; data 00111722
00022694: lea (0x111706).l,A1 ; data 00111706
0002269A: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
0002269E: rts
0002274A: movea.l (0x00fffc04).l,A0
00022750: jsr 0x00055ed6.l ; call 00055ED6 FUN_00055ed6
00022756: clr.w D7w
00022758: jsr 0x00055f20.l ; call 00055F20 FUN_00055f20
0002275E: jsr 0x00012c74.l ; call 00012C74 FUN_00012c74
00022764: move.w #0x6,D4w
00022768: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
0002276E: cmpi.b #0x3,(0x00fff0dd).l
00022776: bpl.w 0x000227a8
0002277A: lea (0x1d6fec).l,A6 ; data 001d6fec
00022780: move.w #0x1,D0w
00022784: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
0002278A: tst.w D0w
0002278C: beq.w 0x000227a8
00022790: jsr 0x0001dea0.l ; call 0001DEA0 FUN_0001dea0
00022796: clr.b (0x00ffe182).l
0002279C: move.w #-0x1,D6w
000227A0: lea (0x111402).l,A0 ; data 00111402
000227A6: rts
000227A8: clr.l D6
000227AA: move.b #-0x1,(0x00fff0d6).l
000227B2: lea (0x111412).l,A0 ; data 00111412
000227B8: cmpi.b #0x5,(0x00fff0dd).l
000227C0: bpl.w 0x000227ca
000227C4: lea (0x11140a).l,A0 ; data 0011140a
000227CA: move.l (0x00fffc2c).l,D1
000227D0: bset.l #0x1a,D1
000227D4: move.l D1,(0x00fffc2c).l
000227DA: rts
000227DC: cmpi.b #0x3,(0x00fffbfa).l
000227E4: bpl.w 0x000227f0
000227E8: lea (0x1118f2).l,A0 ; data 001118f2
000227EE: rts
000227F0: lea (0x1118fa).l,A0 ; data 001118fa
000227F6: rts
00022874: movea.l (0x00fffc04).l,A3
0002287A: adda.l #0x66,A3
00022880: move.w #-0x1,D6w
00022884: move.w #0x7,D7w
00022888: tst.b (0x0,A3,D7w*0x1)
0002288C: bne.w 0x00022892
00022890: move.w D7w,D6w
00022892: dbf D7w,0x00022888
00022896: rts
000229FA: move.l #0x1388,D1
00022A00: lea (0x10a834).l,A0 ; data 0010a834
00022A06: lea (0x10a8a0).l,A1 ; data 0010a8a0
00022A0C: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00022A10: bgt.w 0x00022a1a
00022A14: jsr 0x00002a94.l ; call 00002A94 FUN_00002a94
00022A1A: rts
00022C22: move.l #0xfa0,D1
00022C28: lea (0x10c854).l,A0 ; data 0010c854
00022C2E: lea (0x10c8d4).l,A1 ; data 0010c8d4
00022C34: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00022C38: bgt.w 0x00022c44
00022C3C: bset.b #0x7,(0x00ff018f).l
00022C44: rts
00022C98: btst.b #0x1,(0x00ff018f).l
00022CA0: beq.w 0x00022cac
00022CA4: lea (0x10d08a).l,A0 ; data 0010d08a
00022CAA: rts
00022CAC: cmpi.b #0x5,(0x00ff018b).l
00022CB4: bpl.w 0x00022cc0
00022CB8: lea (0x10d0ba).l,A0 ; data 0010d0ba
00022CBE: rts
00022CC0: lea (0x10d092).l,A0 ; data 0010d092
00022CC6: rts
00022EC6: move.l #0x1388,D1
00022ECC: lea (0x10ea4c).l,A0 ; data 0010ea4c
00022ED2: lea (0x10eab0).l,A1 ; data 0010eab0
00022ED8: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00022EDC: bgt.w 0x00022ee6
00022EE0: clr.b (0x00fffbfa).l
00022EE6: rts
000230EE: move.l (0x00fffc34).l,D1
000230F4: bset.l #0x0,D1
000230F8: move.l D1,(0x00fffc34).l
000230FE: lea (0x1053ac).l,A0 ; data 001053ac
00023104: rts
00023222: move.l (0x00fffc30).l,D1
00023228: bset.l #0x15,D1
0002322C: move.l D1,(0x00fffc30).l
00023232: lea (0x105674).l,A0 ; data 00105674
00023238: rts
0002323A: move.l (0x00fffc30).l,D1
00023240: bclr.l #0xb,D1
00023244: bset.l #0xc,D1
00023248: move.l D1,(0x00fffc30).l
0002324E: lea (0x10568c).l,A0 ; data 0010568c
00023254: rts
0002330A: move.l (0x00fffc34).l,D1
00023310: bset.l #0x0,D1
00023314: move.l D1,(0x00fffc34).l
0002331A: lea (0x105850).l,A0 ; data 00105850
00023320: rts
00023322: move.l (0x00fffc30).l,D1
00023328: bset.l #0x19,D1
0002332C: move.l D1,(0x00fffc30).l
00023332: lea (0x105838).l,A0 ; data 00105838
00023338: rts
00023750: clr.b (0x00fff0d6).l
00023756: clr.l D0
00023758: clr.l D6
0002375A: lea (0xff0100).l,A3 ; data 00ff0100
00023760: move.w #0x2,D7w
000238EA: move.w #0x64,D1w
000238EE: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000238F4: cmpi.w #0x32,D0w
000238F8: bpl.w 0x00023904
000238FC: lea (0x120fc8).l,A0 ; data 00120fc8
00023902: rts
00023904: lea (0x1d7074).l,A6 ; data 001d7074
0002390A: move.w #0x1,D0w
0002390E: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00023914: tst.w D0w
00023916: beq.b 0x000238fc
00023918: lea (0x120fe0).l,A0 ; data 00120fe0
0002391E: rts
00023A72: addi.l #0x96,(0x00fffbfe).l
00023A7C: lea (0x12103c).l,A0 ; data 0012103c
00023A82: rts
00023A84: lea (0x12104c).l,A0 ; data 0012104c
00023A8A: rts
00023A8C: lea (0x1d6d8a).l,A6 ; data 001d6d8a
00023A92: move.w #0x1,D0w
00023A96: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00023A9C: tst.w D0w
00023A9E: beq.b 0x00023a84
00023AA0: lea (0x121054).l,A0 ; data 00121054
00023AA6: rts
00023B0C: clr.l D6
00023B0E: move.b (0x00ff0177).l,D6b
00023B14: move.w #0x6,D4w
00023B18: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00023B1E: cmpi.b #0x2,(0x00fff0dd).l
00023B26: bpl.w 0x00023b48
00023B2A: lea (0x1d6db2).l,A6 ; data 001d6db2
00023B30: move.w #0x1,D0w
00023B34: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00023B3A: tst.w D0w
00023B3C: beq.w 0x00023b48
00023B40: lea (0x1210b0).l,A0 ; data 001210b0
00023B46: rts
00023B48: addi.l #0x64,(0x00fffbfe).l
00023B52: lea (0x1210b8).l,A0 ; data 001210b8
00023B58: rts
00023B88: clr.l D6
00023B8A: move.b (0x00ff0177).l,D6b
00023B90: move.w #0x6,D4w
00023B94: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00023B9A: tst.b (0x00fff0dd).l
00023BA0: beq.w 0x00023bac
00023BA4: lea (0x121100).l,A0 ; data 00121100
00023BAA: rts
00023BAC: lea (0x1210dc).l,A0 ; data 001210dc
00023BB2: rts
00023C38: move.w #0x64,D1w
00023C3C: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00023C42: cmpi.w #0x32,D0w
00023C46: bmi.w 0x00023ce0
00023C4A: lea (0x12115c).l,A0 ; data 0012115c
00023C50: rts
00023CE0: addi.l #0x32,(0x00fffbfe).l
00023CEA: lea (0x121178).l,A0 ; data 00121178
00023CF0: rts
00023F62: move.l #0x1f4,D1
00023F68: lea (0x10f948).l,A0 ; data 0010f948
00023F6E: lea (0x10f9d0).l,A1 ; data 0010f9d0
00023F74: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00023F78: rts
0002401C: move.w (0x00fffc2a).l,D0w
00024022: bset.l #0x0,D0
00024026: move.w D0w,(0x00fffc2a).l
0002402C: lea (0xfe848).l,A0 ; data 000fe848
00024032: rts
00024034: move.w (0x00fffc2a).l,D0w
0002403A: bset.l #0x1,D0
0002403E: move.w D0w,(0x00fffc2a).l
00024044: lea (0xfea5c).l,A0 ; data 000fea5c
0002404A: rts
00024094: move.w (0x00fffc2a).l,D0w
0002409A: bset.l #0x5,D0
0002409E: move.w D0w,(0x00fffc2a).l
000240A4: lea (0xff014).l,A0 ; data 000ff014
000240AA: rts
0002410C: move.l #0x1f4,D1
00024112: lea (0x1115e6).l,A0 ; data 001115e6
00024118: lea (0x111586).l,A1 ; data 00111586
0002411E: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00024122: rts
00024152: move.l #0x1f4,D1
00024158: lea (0x1115e6).l,A0 ; data 001115e6
0002415E: lea (0x1115ca).l,A1 ; data 001115ca
00024164: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00024168: rts
0002416A: bsr.w 0x00024252 ; call 00024252 FUN_00024252
0002416E: cmpi.b #0x1,(0x00fff0dd).l
00024176: bmi.w 0x0002418e
0002417A: beq.w 0x0002419c
0002417E: move.b #0x1,(0x00ffe170).l
00024186: lea (0x1211e4).l,A0 ; data 001211e4
0002418C: rts
0002418E: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
00024194: lea (0x1211d4).l,A0 ; data 001211d4
0002419A: rts
0002419C: lea (0x1211dc).l,A0 ; data 001211dc
000241A2: rts
00024252: clr.l D6
00024254: move.b (0x00ff018a).l,D6b
0002425A: btst.b #0x0,(0x00ff0210).l
00024392: move.w #0x64,D1w
00024396: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0002439C: cmpi.w #0x28,D0w
000243A0: bmi.w 0x000243ac
000243A4: lea (0x121268).l,A0 ; data 00121268
000243AA: rts
000243AC: lea (0x1d705c).l,A6 ; data 001d705c
000243B2: move.w #0x1,D0w
000243B6: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
000243BC: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
000243C2: lea (0x121270).l,A0 ; data 00121270
000243C8: rts
000243CA: move.w #0x64,D1w
000243CE: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000243D4: cmpi.w #0x28,D0w
000243D8: bmi.w 0x000243e4
000243DC: lea (0x121268).l,A0 ; data 00121268
000243E2: rts
000243E4: lea (0x1d7064).l,A6 ; data 001d7064
000243EA: move.w #0x1,D0w
000243EE: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
000243F4: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
000243FA: lea (0x121270).l,A0 ; data 00121270
00024400: rts
00024402: lea (0x1d705c).l,A6 ; data 001d705c
00024408: move.w #0x1,D0w
0002440C: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00024412: lea (0x121290).l,A0 ; data 00121290
00024418: rts
0002441A: lea (0x1d7064).l,A6 ; data 001d7064
00024420: move.w #0x1,D0w
00024424: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
0002442A: lea (0x121290).l,A0 ; data 00121290
00024430: rts
00024456: ori.b #-0x4c,(A2)
0002445A: rts
0002451C: move.w #0x64,D1w
00024520: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00024526: move.l (0x00fffc22).l,D3
0002452C: btst.l #0xa,D3
00024530: beq.w 0x0002454c
00024534: cmpi.w #0x5,D0w
00024538: bmi.w 0x0002455a
0002453C: cmpi.w #0x19,D0w
00024540: bmi.w 0x00024562
00024544: lea (0x121348).l,A0 ; data 00121348
0002454A: rts
0002454C: cmpi.w #0x5,D0w
00024550: bmi.w 0x00024562
00024554: cmpi.w #0x19,D0w
00024558: bpl.b 0x00024544
0002455A: lea (0x121358).l,A0 ; data 00121358
00024560: rts
00024562: move.l (0x00fffc1e).l,D1
00024568: btst.l #0x12,D1
0002456C: beq.b 0x00024544
0002456E: btst.l #0x13,D1
00024572: bne.b 0x00024544
00024574: lea (0x121350).l,A0 ; data 00121350
0002457A: rts
000245F4: bsr.w 0x00023750 ; call 00023750 FUN_00023750
000245F8: move.w #0x8,D4w
000245FC: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00024602: tst.b (0x00fff0dd).l
00024608: beq.w 0x00024614
0002460C: lea (0x1213c4).l,A0 ; data 001213c4
00024612: rts
00024614: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
0002461A: lea (0x1213bc).l,A0 ; data 001213bc
00024620: rts
00024724: clr.l D6
00024726: move.b (0x00ff017a).l,D6b
0002472C: move.w #0x6,D4w
00024730: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00024736: cmpi.b #0x1,(0x00fff0dd).l
0002473E: bmi.w 0x0002478c
00024742: beq.w 0x00024784
00024746: lea (0x11fa8d).l,A1 ; data 0011fa8d
0002474C: lea (0xffb740).l,A5 ; data 00ffb740
00024752: move.w #-0x1,D1w
00024756: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0002475C: lea (0x121494).l,A1 ; data 00121494
00024762: move.w (0x00ffe12c).l,D4w
00024768: lsl.w #0x2,D4w
0002476A: movea.l (0x0,A1,D4w*0x1),A1 ; data 00121494
0002476E: move.w #-0x1,D1w
00024772: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
00024778: move.b #-0x1,(A5)
0002477C: lea (0x11cd92).l,A0 ; data 0011cd92
00024782: rts
00024784: lea (0x121440).l,A0 ; data 00121440
0002478A: rts
0002478C: lea (0x11fa04).l,A1 ; data 0011fa04
00024792: lea (0xffb740).l,A5 ; data 00ffb740
00024798: move.w #-0x1,D1w
0002479C: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
000247A2: move.w #0x64,D1w
000247A6: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000247AC: cmpi.w #0x46,D0w
000247B0: bmi.w 0x000247c4
000247B4: lea (0x11fa04).l,A1 ; data 0011fa04
000247BA: move.w #-0x1,D1w
000247BE: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
000247C4: move.b #-0x1,(A5)
000247C8: lea (0x11cd92).l,A0 ; data 0011cd92
000247CE: rts
000247D0: clr.l D6
000247D2: move.b (0x00ff0188).l,D6b
000247D8: move.w #0x5,D4w
000247DC: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000247E2: cmpi.b #0x1,(0x00fff0dd).l
000247EA: bmi.w 0x00024838
000247EE: beq.w 0x00024830
000247F2: lea (0x11fdae).l,A1 ; data 0011fdae
000247F8: lea (0xffb740).l,A5 ; data 00ffb740
000247FE: move.w #-0x1,D1w
00024802: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
00024808: lea (0x1214dc).l,A1 ; data 001214dc
0002480E: move.w (0x00ffe12c).l,D4w
00024814: lsl.w #0x2,D4w
00024816: movea.l (0x0,A1,D4w*0x1),A1 ; data 001214dc
0002481A: move.w #-0x1,D1w
0002481E: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
00024824: move.b #-0x1,(A5)
00024828: lea (0x11cd92).l,A0 ; data 0011cd92
0002482E: rts
00024830: lea (0x1214cc).l,A0 ; data 001214cc
00024836: rts
00024838: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
0002483E: lea (0x1214c4).l,A0 ; data 001214c4
00024844: rts
00024918: lea (0x1d6fd0).l,A6 ; data 001d6fd0
0002491E: move.w #0x1,D0w
00024922: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00024928: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
0002492E: lea (0x12156c).l,A0 ; data 0012156c
00024934: rts
00024936: lea (0x1d705c).l,A6 ; data 001d705c
0002493C: move.w #0x1,D0w
00024940: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00024946: jsr 0x0001e04a.l ; call 0001E04A FUN_0001e04a
0002494C: lea (0x12156c).l,A0 ; data 0012156c
00024952: rts
00024954: lea (0xf5a78).l,A0 ; data 000f5a78
0002495A: move.b #0x1,(0x00fff0cc).l
00024962: rts
00024A1E: bsr.w 0x00022874 ; call 00022874 FUN_00022874
00024A22: tst.w D6w
00024A24: bpl.w 0x00024a30
00024A28: lea (0x10d738).l,A0 ; data 0010d738
00024A2E: rts
00024A30: move.l #0x7530,D1
00024A36: lea (0x10d730).l,A0 ; data 0010d730
00024A3C: lea (0x10d728).l,A1 ; data 0010d728
00024A42: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00024A46: bgt.w 0x00024a54
00024A4A: move.b #0x32,(0x0,A3,D6w*0x1)
00024A50: clr.b (0x8,A3,D6w*0x1)
00024A54: rts
00024CF4: lea (0x1d70c0).l,A6 ; data 001d70c0
00024CFA: move.w #0x0,D0w
00024CFE: jsr 0x0001e1fc.l ; call 0001E1FC FUN_0001e1fc
00024D04: lea (0x1216a4).l,A0 ; data 001216a4
00024D0A: rts
00024DBE: move.l #0xc8,D1
00024DC4: lea (0x12172c).l,A0 ; data 0012172c
00024DCA: lea (0x121734).l,A1 ; data 00121734
00024DD0: bsr.w 0x00021a4e ; call 00021A4E FUN_00021a4e
00024DD4: bgt.w 0x00024dde
00024DD8: jsr 0x00002a94.l ; call 00002A94 FUN_00002a94
00024DDE: rts
00024DFE: move.l (0x00fffc1e).l,D1
00024E04: btst.l #0x9,D1
00024E08: beq.w 0x00024e14
00024E0C: lea (0xf57bc).l,A0 ; data 000f57bc
00024E12: rts
00024E14: lea (0xf57d0).l,A0 ; data 000f57d0
00024E1A: rts
00024E20: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00024E24: lea (-0x6,PC),A0 ; data 00024e20
00024E28: movea.l (-0x4,A0),A6
00024E2C: add.w D0w,D0w
00024E2E: adda.w (0x24e3a,PC,D0w*0x1),A0 ; data 00024e3a
00024E32: jsr (A0)
00024E34: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00024E38: rts
0002509A: move.w D7w,D0w
0002509C: cmpi.w #0x20,D7w
000250A0: ble.b 0x000250a4
000250A2: moveq #0x20,D7
000250A4: sub.w D7w,D0w
000250A6: move.w D7w,(0x80,A6)
000250AA: move.w D0w,(0x7e,A6)
000250AE: subq.w #0x1,D7w
000250B0: swap D7
000250B2: move.l A0,D3
000250B4: move.l A1,D6
000250B6: tst.b (0x23,A6)
000250BA: beq.b 0x000250dc
000250BC: lea (0xc3c,A6),A2
000250C0: lea (0x115c,PC),A3 ; data 0002621e
000250C4: move.w (0x7a,A6),D7w
000250C8: subq.w #0x1,D7w
000250CA: moveq #0x0,D0
000250CC: move.b (A0)+,D0b
000250CE: add.w D0w,D0w
000250D0: move.w (0x0,A3,D0w*0x1),(A2)+ ; data 0002621e
000250D4: dbf D7w,0x000250ca
000250D8: lea (0xc3c,A6),A0
000250DC: lea (0xac,A6),A2
000250E0: subq.w #0x1,(0x7c,A6)
000250E4: bpl.b 0x000250ec
000250E6: lea (0x82c,A6),A3
000250EA: bra.b 0x000250f0
000250EC: lea (0x5ac,A6),A3
000250F0: move.w (0x10,A6),D7w
000250F4: move.w D7w,D2w
000250F6: lsr.w #0x1,D7w
000250F8: beq.w 0x00025c4e
000250FC: neg.w D7w
000250FE: addi.w #0xa0,D7w
00025102: add.w D7w,D7w
00025104: move.w D7w,D0w
00025106: lsl.w #0x3,D7w
00025108: add.w D0w,D7w
0002510A: jmp (0x2510e,PC,D7w*0x1)
00025228: or.b D1b,D0b
0002522A: move.b D0b,(A1)
0002522C: adda.w (A3)+,A1
0002522E: move.b (A0),D0b
00025230: adda.w (A2)+,A0
00025232: and.b D4b,D0b
00025234: move.b (A0),D1b
00025236: adda.w (A2)+,A0
00025238: and.b D5b,D1b
0002523A: or.b D1b,D0b
0002523C: move.b D0b,(A1)
0002523E: adda.w (A3)+,A1
00025240: move.b (A0),D0b
00025242: adda.w (A2)+,A0
00025244: and.b D4b,D0b
00025246: move.b (A0),D1b
00025248: adda.w (A2)+,A0
0002524A: and.b D5b,D1b
0002524C: or.b D1b,D0b
0002524E: move.b D0b,(A1)
00025250: adda.w (A3)+,A1
00025252: move.b (A0),D0b
00025254: adda.w (A2)+,A0
00025256: and.b D4b,D0b
00025258: move.b (A0),D1b
0002525A: adda.w (A2)+,A0
0002525C: and.b D5b,D1b
0002525E: or.b D1b,D0b
00025260: move.b D0b,(A1)
00025262: adda.w (A3)+,A1
00025264: move.b (A0),D0b
00025266: adda.w (A2)+,A0
00025268: and.b D4b,D0b
0002526A: move.b (A0),D1b
0002526C: adda.w (A2)+,A0
0002526E: and.b D5b,D1b
00025270: or.b D1b,D0b
00025272: move.b D0b,(A1)
00025274: adda.w (A3)+,A1
00025276: move.b (A0),D0b
00025278: adda.w (A2)+,A0
0002527A: and.b D4b,D0b
0002527C: move.b (A0),D1b
0002527E: adda.w (A2)+,A0
00025280: and.b D5b,D1b
00025282: or.b D1b,D0b
00025284: move.b D0b,(A1)
00025286: adda.w (A3)+,A1
00025288: move.b (A0),D0b
0002528A: adda.w (A2)+,A0
0002528C: and.b D4b,D0b
0002528E: move.b (A0),D1b
00025290: adda.w (A2)+,A0
00025292: and.b D5b,D1b
00025294: or.b D1b,D0b
00025296: move.b D0b,(A1)
00025298: adda.w (A3)+,A1
0002529A: move.b (A0),D0b
0002529C: adda.w (A2)+,A0
0002529E: and.b D4b,D0b
000252A0: move.b (A0),D1b
000252A2: adda.w (A2)+,A0
000252A4: and.b D5b,D1b
000252A6: or.b D1b,D0b
000252A8: move.b D0b,(A1)
000252AA: adda.w (A3)+,A1
000252AC: move.b (A0),D0b
000252AE: adda.w (A2)+,A0
000252B0: and.b D4b,D0b
000252B2: move.b (A0),D1b
000252B4: adda.w (A2)+,A0
000252B6: and.b D5b,D1b
000252B8: or.b D1b,D0b
000252BA: move.b D0b,(A1)
000252BC: adda.w (A3)+,A1
000252BE: move.b (A0),D0b
000252C0: adda.w (A2)+,A0
000252C2: and.b D4b,D0b
000252C4: move.b (A0),D1b
000252C6: adda.w (A2)+,A0
000252C8: and.b D5b,D1b
000252CA: or.b D1b,D0b
000252CC: move.b D0b,(A1)
000252CE: adda.w (A3)+,A1
000252D0: move.b (A0),D0b
000252D2: adda.w (A2)+,A0
000252D4: and.b D4b,D0b
000252D6: move.b (A0),D1b
000252D8: adda.w (A2)+,A0
000252DA: and.b D5b,D1b
000252DC: or.b D1b,D0b
000252DE: move.b D0b,(A1)
000252E0: adda.w (A3)+,A1
000252E2: move.b (A0),D0b
000252E4: adda.w (A2)+,A0
000252E6: and.b D4b,D0b
000252E8: move.b (A0),D1b
000252EA: adda.w (A2)+,A0
000252EC: and.b D5b,D1b
000252EE: or.b D1b,D0b
000252F0: move.b D0b,(A1)
000252F2: adda.w (A3)+,A1
000252F4: move.b (A0),D0b
000252F6: adda.w (A2)+,A0
000252F8: and.b D4b,D0b
000252FA: move.b (A0),D1b
000252FC: adda.w (A2)+,A0
000252FE: and.b D5b,D1b
00025300: or.b D1b,D0b
00025302: move.b D0b,(A1)
00025304: adda.w (A3)+,A1
00025306: move.b (A0),D0b
00025308: adda.w (A2)+,A0
0002530A: and.b D4b,D0b
0002530C: move.b (A0),D1b
0002530E: adda.w (A2)+,A0
00025310: and.b D5b,D1b
00025312: or.b D1b,D0b
00025314: move.b D0b,(A1)
00025316: adda.w (A3)+,A1
00025318: move.b (A0),D0b
0002531A: adda.w (A2)+,A0
0002531C: and.b D4b,D0b
0002531E: move.b (A0),D1b
00025320: adda.w (A2)+,A0
00025322: and.b D5b,D1b
00025324: or.b D1b,D0b
00025326: move.b D0b,(A1)
00025328: adda.w (A3)+,A1
0002532A: move.b (A0),D0b
0002532C: adda.w (A2)+,A0
0002532E: and.b D4b,D0b
00025330: move.b (A0),D1b
00025332: adda.w (A2)+,A0
00025334: and.b D5b,D1b
00025336: or.b D1b,D0b
00025338: move.b D0b,(A1)
0002533A: adda.w (A3)+,A1
0002533C: move.b (A0),D0b
0002533E: adda.w (A2)+,A0
00025340: and.b D4b,D0b
00025342: move.b (A0),D1b
00025344: adda.w (A2)+,A0
00025346: and.b D5b,D1b
00025348: or.b D1b,D0b
0002534A: move.b D0b,(A1)
0002534C: adda.w (A3)+,A1
0002534E: move.b (A0),D0b
00025350: adda.w (A2)+,A0
00025352: and.b D4b,D0b
00025354: move.b (A0),D1b
00025356: adda.w (A2)+,A0
00025358: and.b D5b,D1b
0002535A: or.b D1b,D0b
0002535C: move.b D0b,(A1)
0002535E: adda.w (A3)+,A1
00025360: move.b (A0),D0b
00025362: adda.w (A2)+,A0
00025364: and.b D4b,D0b
00025366: move.b (A0),D1b
00025368: adda.w (A2)+,A0
0002536A: and.b D5b,D1b
0002536C: or.b D1b,D0b
0002536E: move.b D0b,(A1)
00025370: adda.w (A3)+,A1
00025372: move.b (A0),D0b
00025374: adda.w (A2)+,A0
00025376: and.b D4b,D0b
00025378: move.b (A0),D1b
0002537A: adda.w (A2)+,A0
0002537C: and.b D5b,D1b
0002537E: or.b D1b,D0b
00025380: move.b D0b,(A1)
00025382: adda.w (A3)+,A1
00025384: move.b (A0),D0b
00025386: adda.w (A2)+,A0
00025388: and.b D4b,D0b
0002538A: move.b (A0),D1b
0002538C: adda.w (A2)+,A0
0002538E: and.b D5b,D1b
00025390: or.b D1b,D0b
00025392: move.b D0b,(A1)
00025394: adda.w (A3)+,A1
00025396: move.b (A0),D0b
00025398: adda.w (A2)+,A0
0002539A: and.b D4b,D0b
0002539C: move.b (A0),D1b
0002539E: adda.w (A2)+,A0
000253A0: and.b D5b,D1b
000253A2: or.b D1b,D0b
000253A4: move.b D0b,(A1)
000253A6: adda.w (A3)+,A1
000253A8: move.b (A0),D0b
000253AA: adda.w (A2)+,A0
000253AC: and.b D4b,D0b
000253AE: move.b (A0),D1b
000253B0: adda.w (A2)+,A0
000253B2: and.b D5b,D1b
000253B4: or.b D1b,D0b
000253B6: move.b D0b,(A1)
000253B8: adda.w (A3)+,A1
000253BA: move.b (A0),D0b
000253BC: adda.w (A2)+,A0
000253BE: and.b D4b,D0b
000253C0: move.b (A0),D1b
000253C2: adda.w (A2)+,A0
000253C4: and.b D5b,D1b
000253C6: or.b D1b,D0b
000253C8: move.b D0b,(A1)
000253CA: adda.w (A3)+,A1
000253CC: move.b (A0),D0b
000253CE: adda.w (A2)+,A0
000253D0: and.b D4b,D0b
000253D2: move.b (A0),D1b
000253D4: adda.w (A2)+,A0
000253D6: and.b D5b,D1b
000253D8: or.b D1b,D0b
000253DA: move.b D0b,(A1)
000253DC: adda.w (A3)+,A1
000253DE: move.b (A0),D0b
000253E0: adda.w (A2)+,A0
000253E2: and.b D4b,D0b
000253E4: move.b (A0),D1b
000253E6: adda.w (A2)+,A0
000253E8: and.b D5b,D1b
000253EA: or.b D1b,D0b
000253EC: move.b D0b,(A1)
000253EE: adda.w (A3)+,A1
000253F0: move.b (A0),D0b
000253F2: adda.w (A2)+,A0
000253F4: and.b D4b,D0b
000253F6: move.b (A0),D1b
000253F8: adda.w (A2)+,A0
000253FA: and.b D5b,D1b
000253FC: or.b D1b,D0b
000253FE: move.b D0b,(A1)
00025400: adda.w (A3)+,A1
00025402: move.b (A0),D0b
00025404: adda.w (A2)+,A0
00025406: and.b D4b,D0b
00025408: move.b (A0),D1b
0002540A: adda.w (A2)+,A0
0002540C: and.b D5b,D1b
0002540E: or.b D1b,D0b
00025410: move.b D0b,(A1)
00025412: adda.w (A3)+,A1
00025414: move.b (A0),D0b
00025416: adda.w (A2)+,A0
00025418: and.b D4b,D0b
0002541A: move.b (A0),D1b
0002541C: adda.w (A2)+,A0
0002541E: and.b D5b,D1b
00025420: or.b D1b,D0b
00025422: move.b D0b,(A1)
00025424: adda.w (A3)+,A1
00025426: move.b (A0),D0b
00025428: adda.w (A2)+,A0
0002542A: and.b D4b,D0b
0002542C: move.b (A0),D1b
0002542E: adda.w (A2)+,A0
00025430: and.b D5b,D1b
00025432: or.b D1b,D0b
00025434: move.b D0b,(A1)
00025436: adda.w (A3)+,A1
00025438: move.b (A0),D0b
0002543A: adda.w (A2)+,A0
0002543C: and.b D4b,D0b
0002543E: move.b (A0),D1b
00025440: adda.w (A2)+,A0
00025442: and.b D5b,D1b
00025444: or.b D1b,D0b
00025446: move.b D0b,(A1)
00025448: adda.w (A3)+,A1
0002544A: move.b (A0),D0b
0002544C: adda.w (A2)+,A0
0002544E: and.b D4b,D0b
00025450: move.b (A0),D1b
00025452: adda.w (A2)+,A0
00025454: and.b D5b,D1b
00025456: or.b D1b,D0b
00025458: move.b D0b,(A1)
0002545A: adda.w (A3)+,A1
0002545C: move.b (A0),D0b
0002545E: adda.w (A2)+,A0
00025460: and.b D4b,D0b
00025462: move.b (A0),D1b
00025464: adda.w (A2)+,A0
00025466: and.b D5b,D1b
00025468: or.b D1b,D0b
0002546A: move.b D0b,(A1)
0002546C: adda.w (A3)+,A1
0002546E: move.b (A0),D0b
00025470: adda.w (A2)+,A0
00025472: and.b D4b,D0b
00025474: move.b (A0),D1b
00025476: adda.w (A2)+,A0
00025478: and.b D5b,D1b
0002547A: or.b D1b,D0b
0002547C: move.b D0b,(A1)
0002547E: adda.w (A3)+,A1
00025480: move.b (A0),D0b
00025482: adda.w (A2)+,A0
00025484: and.b D4b,D0b
00025486: move.b (A0),D1b
00025488: adda.w (A2)+,A0
0002548A: and.b D5b,D1b
0002548C: or.b D1b,D0b
0002548E: move.b D0b,(A1)
00025490: adda.w (A3)+,A1
00025492: move.b (A0),D0b
00025494: adda.w (A2)+,A0
00025496: and.b D4b,D0b
00025498: move.b (A0),D1b
0002549A: adda.w (A2)+,A0
0002549C: and.b D5b,D1b
0002549E: or.b D1b,D0b
000254A0: move.b D0b,(A1)
000254A2: adda.w (A3)+,A1
000254A4: move.b (A0),D0b
000254A6: adda.w (A2)+,A0
000254A8: and.b D4b,D0b
000254AA: move.b (A0),D1b
000254AC: adda.w (A2)+,A0
000254AE: and.b D5b,D1b
000254B0: or.b D1b,D0b
000254B2: move.b D0b,(A1)
000254B4: adda.w (A3)+,A1
000254B6: move.b (A0),D0b
000254B8: adda.w (A2)+,A0
000254BA: and.b D4b,D0b
000254BC: move.b (A0),D1b
000254BE: adda.w (A2)+,A0
000254C0: and.b D5b,D1b
000254C2: or.b D1b,D0b
000254C4: move.b D0b,(A1)
000254C6: adda.w (A3)+,A1
000254C8: move.b (A0),D0b
000254CA: adda.w (A2)+,A0
000254CC: and.b D4b,D0b
000254CE: move.b (A0),D1b
000254D0: adda.w (A2)+,A0
000254D2: and.b D5b,D1b
000254D4: or.b D1b,D0b
000254D6: move.b D0b,(A1)
000254D8: adda.w (A3)+,A1
000254DA: move.b (A0),D0b
000254DC: adda.w (A2)+,A0
000254DE: and.b D4b,D0b
000254E0: move.b (A0),D1b
000254E2: adda.w (A2)+,A0
000254E4: and.b D5b,D1b
000254E6: or.b D1b,D0b
000254E8: move.b D0b,(A1)
000254EA: adda.w (A3)+,A1
000254EC: move.b (A0),D0b
000254EE: adda.w (A2)+,A0
000254F0: and.b D4b,D0b
000254F2: move.b (A0),D1b
000254F4: adda.w (A2)+,A0
000254F6: and.b D5b,D1b
000254F8: or.b D1b,D0b
000254FA: move.b D0b,(A1)
000254FC: adda.w (A3)+,A1
000254FE: move.b (A0),D0b
00025500: adda.w (A2)+,A0
00025502: and.b D4b,D0b
00025504: move.b (A0),D1b
00025506: adda.w (A2)+,A0
00025508: and.b D5b,D1b
0002550A: or.b D1b,D0b
0002550C: move.b D0b,(A1)
0002550E: adda.w (A3)+,A1
00025510: move.b (A0),D0b
00025512: adda.w (A2)+,A0
00025514: and.b D4b,D0b
00025516: move.b (A0),D1b
00025518: adda.w (A2)+,A0
0002551A: and.b D5b,D1b
0002551C: or.b D1b,D0b
0002551E: move.b D0b,(A1)
00025520: adda.w (A3)+,A1
00025522: move.b (A0),D0b
00025524: adda.w (A2)+,A0
00025526: and.b D4b,D0b
00025528: move.b (A0),D1b
0002552A: adda.w (A2)+,A0
0002552C: and.b D5b,D1b
0002552E: or.b D1b,D0b
00025530: move.b D0b,(A1)
00025532: adda.w (A3)+,A1
00025534: move.b (A0),D0b
00025536: adda.w (A2)+,A0
00025538: and.b D4b,D0b
0002553A: move.b (A0),D1b
0002553C: adda.w (A2)+,A0
0002553E: and.b D5b,D1b
00025540: or.b D1b,D0b
00025542: move.b D0b,(A1)
00025544: adda.w (A3)+,A1
00025546: move.b (A0),D0b
00025548: adda.w (A2)+,A0
0002554A: and.b D4b,D0b
0002554C: move.b (A0),D1b
0002554E: adda.w (A2)+,A0
00025550: and.b D5b,D1b
00025552: or.b D1b,D0b
00025554: move.b D0b,(A1)
00025556: adda.w (A3)+,A1
00025558: move.b (A0),D0b
0002555A: adda.w (A2)+,A0
0002555C: and.b D4b,D0b
0002555E: move.b (A0),D1b
00025560: adda.w (A2)+,A0
00025562: and.b D5b,D1b
00025564: or.b D1b,D0b
00025566: move.b D0b,(A1)
00025568: adda.w (A3)+,A1
0002556A: move.b (A0),D0b
0002556C: adda.w (A2)+,A0
0002556E: and.b D4b,D0b
00025570: move.b (A0),D1b
00025572: adda.w (A2)+,A0
00025574: and.b D5b,D1b
00025576: or.b D1b,D0b
00025578: move.b D0b,(A1)
0002557A: adda.w (A3)+,A1
0002557C: move.b (A0),D0b
0002557E: adda.w (A2)+,A0
00025580: and.b D4b,D0b
00025582: move.b (A0),D1b
00025584: adda.w (A2)+,A0
00025586: and.b D5b,D1b
00025588: or.b D1b,D0b
0002558A: move.b D0b,(A1)
0002558C: adda.w (A3)+,A1
0002558E: move.b (A0),D0b
00025590: adda.w (A2)+,A0
00025592: and.b D4b,D0b
00025594: move.b (A0),D1b
00025596: adda.w (A2)+,A0
00025598: and.b D5b,D1b
0002559A: or.b D1b,D0b
0002559C: move.b D0b,(A1)
0002559E: adda.w (A3)+,A1
000255A0: move.b (A0),D0b
000255A2: adda.w (A2)+,A0
000255A4: and.b D4b,D0b
000255A6: move.b (A0),D1b
000255A8: adda.w (A2)+,A0
000255AA: and.b D5b,D1b
000255AC: or.b D1b,D0b
000255AE: move.b D0b,(A1)
000255B0: adda.w (A3)+,A1
000255B2: move.b (A0),D0b
000255B4: adda.w (A2)+,A0
000255B6: and.b D4b,D0b
000255B8: move.b (A0),D1b
000255BA: adda.w (A2)+,A0
000255BC: and.b D5b,D1b
000255BE: or.b D1b,D0b
000255C0: move.b D0b,(A1)
000255C2: adda.w (A3)+,A1
000255C4: move.b (A0),D0b
000255C6: adda.w (A2)+,A0
000255C8: and.b D4b,D0b
000255CA: move.b (A0),D1b
000255CC: adda.w (A2)+,A0
000255CE: and.b D5b,D1b
000255D0: or.b D1b,D0b
000255D2: move.b D0b,(A1)
000255D4: adda.w (A3)+,A1
000255D6: move.b (A0),D0b
000255D8: adda.w (A2)+,A0
000255DA: and.b D4b,D0b
000255DC: move.b (A0),D1b
000255DE: adda.w (A2)+,A0
000255E0: and.b D5b,D1b
000255E2: or.b D1b,D0b
000255E4: move.b D0b,(A1)
000255E6: adda.w (A3)+,A1
000255E8: move.b (A0),D0b
000255EA: adda.w (A2)+,A0
000255EC: and.b D4b,D0b
000255EE: move.b (A0),D1b
000255F0: adda.w (A2)+,A0
000255F2: and.b D5b,D1b
000255F4: or.b D1b,D0b
000255F6: move.b D0b,(A1)
000255F8: adda.w (A3)+,A1
000255FA: move.b (A0),D0b
000255FC: adda.w (A2)+,A0
000255FE: and.b D4b,D0b
00025600: move.b (A0),D1b
00025602: adda.w (A2)+,A0
00025604: and.b D5b,D1b
00025606: or.b D1b,D0b
00025608: move.b D0b,(A1)
0002560A: adda.w (A3)+,A1
0002560C: move.b (A0),D0b
0002560E: adda.w (A2)+,A0
00025610: and.b D4b,D0b
00025612: move.b (A0),D1b
00025614: adda.w (A2)+,A0
00025616: and.b D5b,D1b
00025618: or.b D1b,D0b
0002561A: move.b D0b,(A1)
0002561C: adda.w (A3)+,A1
0002561E: move.b (A0),D0b
00025620: adda.w (A2)+,A0
00025622: and.b D4b,D0b
00025624: move.b (A0),D1b
00025626: adda.w (A2)+,A0
00025628: and.b D5b,D1b
0002562A: or.b D1b,D0b
0002562C: move.b D0b,(A1)
0002562E: adda.w (A3)+,A1
00025630: move.b (A0),D0b
00025632: adda.w (A2)+,A0
00025634: and.b D4b,D0b
00025636: move.b (A0),D1b
00025638: adda.w (A2)+,A0
0002563A: and.b D5b,D1b
0002563C: or.b D1b,D0b
0002563E: move.b D0b,(A1)
00025640: adda.w (A3)+,A1
00025642: move.b (A0),D0b
00025644: adda.w (A2)+,A0
00025646: and.b D4b,D0b
00025648: move.b (A0),D1b
0002564A: adda.w (A2)+,A0
0002564C: and.b D5b,D1b
0002564E: or.b D1b,D0b
00025650: move.b D0b,(A1)
00025652: adda.w (A3)+,A1
00025654: move.b (A0),D0b
00025656: adda.w (A2)+,A0
00025658: and.b D4b,D0b
0002565A: move.b (A0),D1b
0002565C: adda.w (A2)+,A0
0002565E: and.b D5b,D1b
00025660: or.b D1b,D0b
00025662: move.b D0b,(A1)
00025664: adda.w (A3)+,A1
00025666: move.b (A0),D0b
00025668: adda.w (A2)+,A0
0002566A: and.b D4b,D0b
0002566C: move.b (A0),D1b
0002566E: adda.w (A2)+,A0
00025670: and.b D5b,D1b
00025672: or.b D1b,D0b
00025674: move.b D0b,(A1)
00025676: adda.w (A3)+,A1
00025678: move.b (A0),D0b
0002567A: adda.w (A2)+,A0
0002567C: and.b D4b,D0b
0002567E: move.b (A0),D1b
00025680: adda.w (A2)+,A0
00025682: and.b D5b,D1b
00025684: or.b D1b,D0b
00025686: move.b D0b,(A1)
00025688: adda.w (A3)+,A1
0002568A: move.b (A0),D0b
0002568C: adda.w (A2)+,A0
0002568E: and.b D4b,D0b
00025690: move.b (A0),D1b
00025692: adda.w (A2)+,A0
00025694: and.b D5b,D1b
00025696: or.b D1b,D0b
00025698: move.b D0b,(A1)
0002569A: adda.w (A3)+,A1
0002569C: move.b (A0),D0b
0002569E: adda.w (A2)+,A0
000256A0: and.b D4b,D0b
000256A2: move.b (A0),D1b
000256A4: adda.w (A2)+,A0
000256A6: and.b D5b,D1b
000256A8: or.b D1b,D0b
000256AA: move.b D0b,(A1)
000256AC: adda.w (A3)+,A1
000256AE: move.b (A0),D0b
000256B0: adda.w (A2)+,A0
000256B2: and.b D4b,D0b
000256B4: move.b (A0),D1b
000256B6: adda.w (A2)+,A0
000256B8: and.b D5b,D1b
000256BA: or.b D1b,D0b
000256BC: move.b D0b,(A1)
000256BE: adda.w (A3)+,A1
000256C0: move.b (A0),D0b
000256C2: adda.w (A2)+,A0
000256C4: and.b D4b,D0b
000256C6: move.b (A0),D1b
000256C8: adda.w (A2)+,A0
000256CA: and.b D5b,D1b
000256CC: or.b D1b,D0b
000256CE: move.b D0b,(A1)
000256D0: adda.w (A3)+,A1
000256D2: move.b (A0),D0b
000256D4: adda.w (A2)+,A0
000256D6: and.b D4b,D0b
000256D8: move.b (A0),D1b
000256DA: adda.w (A2)+,A0
000256DC: and.b D5b,D1b
000256DE: or.b D1b,D0b
000256E0: move.b D0b,(A1)
000256E2: adda.w (A3)+,A1
000256E4: move.b (A0),D0b
000256E6: adda.w (A2)+,A0
000256E8: and.b D4b,D0b
000256EA: move.b (A0),D1b
000256EC: adda.w (A2)+,A0
000256EE: and.b D5b,D1b
000256F0: or.b D1b,D0b
000256F2: move.b D0b,(A1)
000256F4: adda.w (A3)+,A1
000256F6: move.b (A0),D0b
000256F8: adda.w (A2)+,A0
000256FA: and.b D4b,D0b
000256FC: move.b (A0),D1b
000256FE: adda.w (A2)+,A0
00025700: and.b D5b,D1b
00025702: or.b D1b,D0b
00025704: move.b D0b,(A1)
00025706: adda.w (A3)+,A1
00025708: move.b (A0),D0b
0002570A: adda.w (A2)+,A0
0002570C: and.b D4b,D0b
0002570E: move.b (A0),D1b
00025710: adda.w (A2)+,A0
00025712: and.b D5b,D1b
00025714: or.b D1b,D0b
00025716: move.b D0b,(A1)
00025718: adda.w (A3)+,A1
0002571A: move.b (A0),D0b
0002571C: adda.w (A2)+,A0
0002571E: and.b D4b,D0b
00025720: move.b (A0),D1b
00025722: adda.w (A2)+,A0
00025724: and.b D5b,D1b
00025726: or.b D1b,D0b
00025728: move.b D0b,(A1)
0002572A: adda.w (A3)+,A1
0002572C: move.b (A0),D0b
0002572E: adda.w (A2)+,A0
00025730: and.b D4b,D0b
00025732: move.b (A0),D1b
00025734: adda.w (A2)+,A0
00025736: and.b D5b,D1b
00025738: or.b D1b,D0b
0002573A: move.b D0b,(A1)
0002573C: adda.w (A3)+,A1
0002573E: move.b (A0),D0b
00025740: adda.w (A2)+,A0
00025742: and.b D4b,D0b
00025744: move.b (A0),D1b
00025746: adda.w (A2)+,A0
00025748: and.b D5b,D1b
0002574A: or.b D1b,D0b
0002574C: move.b D0b,(A1)
0002574E: adda.w (A3)+,A1
00025750: move.b (A0),D0b
00025752: adda.w (A2)+,A0
00025754: and.b D4b,D0b
00025756: move.b (A0),D1b
00025758: adda.w (A2)+,A0
0002575A: and.b D5b,D1b
0002575C: or.b D1b,D0b
0002575E: move.b D0b,(A1)
00025760: adda.w (A3)+,A1
00025762: move.b (A0),D0b
00025764: adda.w (A2)+,A0
00025766: and.b D4b,D0b
00025768: move.b (A0),D1b
0002576A: adda.w (A2)+,A0
0002576C: and.b D5b,D1b
0002576E: or.b D1b,D0b
00025770: move.b D0b,(A1)
00025772: adda.w (A3)+,A1
00025774: move.b (A0),D0b
00025776: adda.w (A2)+,A0
00025778: and.b D4b,D0b
0002577A: move.b (A0),D1b
0002577C: adda.w (A2)+,A0
0002577E: and.b D5b,D1b
00025780: or.b D1b,D0b
00025782: move.b D0b,(A1)
00025784: adda.w (A3)+,A1
00025786: move.b (A0),D0b
00025788: adda.w (A2)+,A0
0002578A: and.b D4b,D0b
0002578C: move.b (A0),D1b
0002578E: adda.w (A2)+,A0
00025790: and.b D5b,D1b
00025792: or.b D1b,D0b
00025794: move.b D0b,(A1)
00025796: adda.w (A3)+,A1
00025798: move.b (A0),D0b
0002579A: adda.w (A2)+,A0
0002579C: and.b D4b,D0b
0002579E: move.b (A0),D1b
000257A0: adda.w (A2)+,A0
000257A2: and.b D5b,D1b
000257A4: or.b D1b,D0b
000257A6: move.b D0b,(A1)
000257A8: adda.w (A3)+,A1
000257AA: move.b (A0),D0b
000257AC: adda.w (A2)+,A0
000257AE: and.b D4b,D0b
000257B0: move.b (A0),D1b
000257B2: adda.w (A2)+,A0
000257B4: and.b D5b,D1b
000257B6: or.b D1b,D0b
000257B8: move.b D0b,(A1)
000257BA: adda.w (A3)+,A1
000257BC: move.b (A0),D0b
000257BE: adda.w (A2)+,A0
000257C0: and.b D4b,D0b
000257C2: move.b (A0),D1b
000257C4: adda.w (A2)+,A0
000257C6: and.b D5b,D1b
000257C8: or.b D1b,D0b
000257CA: move.b D0b,(A1)
000257CC: adda.w (A3)+,A1
000257CE: move.b (A0),D0b
000257D0: adda.w (A2)+,A0
000257D2: and.b D4b,D0b
000257D4: move.b (A0),D1b
000257D6: adda.w (A2)+,A0
000257D8: and.b D5b,D1b
000257DA: or.b D1b,D0b
000257DC: move.b D0b,(A1)
000257DE: adda.w (A3)+,A1
000257E0: move.b (A0),D0b
000257E2: adda.w (A2)+,A0
000257E4: and.b D4b,D0b
000257E6: move.b (A0),D1b
000257E8: adda.w (A2)+,A0
000257EA: and.b D5b,D1b
000257EC: or.b D1b,D0b
000257EE: move.b D0b,(A1)
000257F0: adda.w (A3)+,A1
000257F2: move.b (A0),D0b
000257F4: adda.w (A2)+,A0
000257F6: and.b D4b,D0b
000257F8: move.b (A0),D1b
000257FA: adda.w (A2)+,A0
000257FC: and.b D5b,D1b
000257FE: or.b D1b,D0b
00025800: move.b D0b,(A1)
00025802: adda.w (A3)+,A1
00025804: move.b (A0),D0b
00025806: adda.w (A2)+,A0
00025808: and.b D4b,D0b
0002580A: move.b (A0),D1b
0002580C: adda.w (A2)+,A0
0002580E: and.b D5b,D1b
00025810: or.b D1b,D0b
00025812: move.b D0b,(A1)
00025814: adda.w (A3)+,A1
00025816: move.b (A0),D0b
00025818: adda.w (A2)+,A0
0002581A: and.b D4b,D0b
0002581C: move.b (A0),D1b
0002581E: adda.w (A2)+,A0
00025820: and.b D5b,D1b
00025822: or.b D1b,D0b
00025824: move.b D0b,(A1)
00025826: adda.w (A3)+,A1
00025828: move.b (A0),D0b
0002582A: adda.w (A2)+,A0
0002582C: and.b D4b,D0b
0002582E: move.b (A0),D1b
00025830: adda.w (A2)+,A0
00025832: and.b D5b,D1b
00025834: or.b D1b,D0b
00025836: move.b D0b,(A1)
00025838: adda.w (A3)+,A1
0002583A: move.b (A0),D0b
0002583C: adda.w (A2)+,A0
0002583E: and.b D4b,D0b
00025840: move.b (A0),D1b
00025842: adda.w (A2)+,A0
00025844: and.b D5b,D1b
00025846: or.b D1b,D0b
00025848: move.b D0b,(A1)
0002584A: adda.w (A3)+,A1
0002584C: move.b (A0),D0b
0002584E: adda.w (A2)+,A0
00025850: and.b D4b,D0b
00025852: move.b (A0),D1b
00025854: adda.w (A2)+,A0
00025856: and.b D5b,D1b
00025858: or.b D1b,D0b
0002585A: move.b D0b,(A1)
0002585C: adda.w (A3)+,A1
0002585E: move.b (A0),D0b
00025860: adda.w (A2)+,A0
00025862: and.b D4b,D0b
00025864: move.b (A0),D1b
00025866: adda.w (A2)+,A0
00025868: and.b D5b,D1b
0002586A: or.b D1b,D0b
0002586C: move.b D0b,(A1)
0002586E: adda.w (A3)+,A1
00025870: move.b (A0),D0b
00025872: adda.w (A2)+,A0
00025874: and.b D4b,D0b
00025876: move.b (A0),D1b
00025878: adda.w (A2)+,A0
0002587A: and.b D5b,D1b
0002587C: or.b D1b,D0b
0002587E: move.b D0b,(A1)
00025880: adda.w (A3)+,A1
00025882: move.b (A0),D0b
00025884: adda.w (A2)+,A0
00025886: and.b D4b,D0b
00025888: move.b (A0),D1b
0002588A: adda.w (A2)+,A0
0002588C: and.b D5b,D1b
0002588E: or.b D1b,D0b
00025890: move.b D0b,(A1)
00025892: adda.w (A3)+,A1
00025894: move.b (A0),D0b
00025896: adda.w (A2)+,A0
00025898: and.b D4b,D0b
0002589A: move.b (A0),D1b
0002589C: adda.w (A2)+,A0
0002589E: and.b D5b,D1b
000258A0: or.b D1b,D0b
000258A2: move.b D0b,(A1)
000258A4: adda.w (A3)+,A1
000258A6: move.b (A0),D0b
000258A8: adda.w (A2)+,A0
000258AA: and.b D4b,D0b
000258AC: move.b (A0),D1b
000258AE: adda.w (A2)+,A0
000258B0: and.b D5b,D1b
000258B2: or.b D1b,D0b
000258B4: move.b D0b,(A1)
000258B6: adda.w (A3)+,A1
000258B8: move.b (A0),D0b
000258BA: adda.w (A2)+,A0
000258BC: and.b D4b,D0b
000258BE: move.b (A0),D1b
000258C0: adda.w (A2)+,A0
000258C2: and.b D5b,D1b
000258C4: or.b D1b,D0b
000258C6: move.b D0b,(A1)
000258C8: adda.w (A3)+,A1
000258CA: move.b (A0),D0b
000258CC: adda.w (A2)+,A0
000258CE: and.b D4b,D0b
000258D0: move.b (A0),D1b
000258D2: adda.w (A2)+,A0
000258D4: and.b D5b,D1b
000258D6: or.b D1b,D0b
000258D8: move.b D0b,(A1)
000258DA: adda.w (A3)+,A1
000258DC: move.b (A0),D0b
000258DE: adda.w (A2)+,A0
000258E0: and.b D4b,D0b
000258E2: move.b (A0),D1b
000258E4: adda.w (A2)+,A0
000258E6: and.b D5b,D1b
000258E8: or.b D1b,D0b
000258EA: move.b D0b,(A1)
000258EC: adda.w (A3)+,A1
000258EE: move.b (A0),D0b
000258F0: adda.w (A2)+,A0
000258F2: and.b D4b,D0b
000258F4: move.b (A0),D1b
000258F6: adda.w (A2)+,A0
000258F8: and.b D5b,D1b
000258FA: or.b D1b,D0b
000258FC: move.b D0b,(A1)
000258FE: adda.w (A3)+,A1
00025900: move.b (A0),D0b
00025902: adda.w (A2)+,A0
00025904: and.b D4b,D0b
00025906: move.b (A0),D1b
00025908: adda.w (A2)+,A0
0002590A: and.b D5b,D1b
0002590C: or.b D1b,D0b
0002590E: move.b D0b,(A1)
00025910: adda.w (A3)+,A1
00025912: move.b (A0),D0b
00025914: adda.w (A2)+,A0
00025916: and.b D4b,D0b
00025918: move.b (A0),D1b
0002591A: adda.w (A2)+,A0
0002591C: and.b D5b,D1b
0002591E: or.b D1b,D0b
00025920: move.b D0b,(A1)
00025922: adda.w (A3)+,A1
00025924: move.b (A0),D0b
00025926: adda.w (A2)+,A0
00025928: and.b D4b,D0b
0002592A: move.b (A0),D1b
0002592C: adda.w (A2)+,A0
0002592E: and.b D5b,D1b
00025930: or.b D1b,D0b
00025932: move.b D0b,(A1)
00025934: adda.w (A3)+,A1
00025936: move.b (A0),D0b
00025938: adda.w (A2)+,A0
0002593A: and.b D4b,D0b
0002593C: move.b (A0),D1b
0002593E: adda.w (A2)+,A0
00025940: and.b D5b,D1b
00025942: or.b D1b,D0b
00025944: move.b D0b,(A1)
00025946: adda.w (A3)+,A1
00025948: move.b (A0),D0b
0002594A: adda.w (A2)+,A0
0002594C: and.b D4b,D0b
0002594E: move.b (A0),D1b
00025950: adda.w (A2)+,A0
00025952: and.b D5b,D1b
00025954: or.b D1b,D0b
00025956: move.b D0b,(A1)
00025958: adda.w (A3)+,A1
0002595A: move.b (A0),D0b
0002595C: adda.w (A2)+,A0
0002595E: and.b D4b,D0b
00025960: move.b (A0),D1b
00025962: adda.w (A2)+,A0
00025964: and.b D5b,D1b
00025966: or.b D1b,D0b
00025968: move.b D0b,(A1)
0002596A: adda.w (A3)+,A1
0002596C: move.b (A0),D0b
0002596E: adda.w (A2)+,A0
00025970: and.b D4b,D0b
00025972: move.b (A0),D1b
00025974: adda.w (A2)+,A0
00025976: and.b D5b,D1b
00025978: or.b D1b,D0b
0002597A: move.b D0b,(A1)
0002597C: adda.w (A3)+,A1
0002597E: move.b (A0),D0b
00025980: adda.w (A2)+,A0
00025982: and.b D4b,D0b
00025984: move.b (A0),D1b
00025986: adda.w (A2)+,A0
00025988: and.b D5b,D1b
0002598A: or.b D1b,D0b
0002598C: move.b D0b,(A1)
0002598E: adda.w (A3)+,A1
00025990: move.b (A0),D0b
00025992: adda.w (A2)+,A0
00025994: and.b D4b,D0b
00025996: move.b (A0),D1b
00025998: adda.w (A2)+,A0
0002599A: and.b D5b,D1b
0002599C: or.b D1b,D0b
0002599E: move.b D0b,(A1)
000259A0: adda.w (A3)+,A1
000259A2: move.b (A0),D0b
000259A4: adda.w (A2)+,A0
000259A6: and.b D4b,D0b
000259A8: move.b (A0),D1b
000259AA: adda.w (A2)+,A0
000259AC: and.b D5b,D1b
000259AE: or.b D1b,D0b
000259B0: move.b D0b,(A1)
000259B2: adda.w (A3)+,A1
000259B4: move.b (A0),D0b
000259B6: adda.w (A2)+,A0
000259B8: and.b D4b,D0b
000259BA: move.b (A0),D1b
000259BC: adda.w (A2)+,A0
000259BE: and.b D5b,D1b
000259C0: or.b D1b,D0b
000259C2: move.b D0b,(A1)
000259C4: adda.w (A3)+,A1
000259C6: move.b (A0),D0b
000259C8: adda.w (A2)+,A0
000259CA: and.b D4b,D0b
000259CC: move.b (A0),D1b
000259CE: adda.w (A2)+,A0
000259D0: and.b D5b,D1b
000259D2: or.b D1b,D0b
000259D4: move.b D0b,(A1)
000259D6: adda.w (A3)+,A1
000259D8: move.b (A0),D0b
000259DA: adda.w (A2)+,A0
000259DC: and.b D4b,D0b
000259DE: move.b (A0),D1b
000259E0: adda.w (A2)+,A0
000259E2: and.b D5b,D1b
000259E4: or.b D1b,D0b
000259E6: move.b D0b,(A1)
000259E8: adda.w (A3)+,A1
000259EA: move.b (A0),D0b
000259EC: adda.w (A2)+,A0
000259EE: and.b D4b,D0b
000259F0: move.b (A0),D1b
000259F2: adda.w (A2)+,A0
000259F4: and.b D5b,D1b
000259F6: or.b D1b,D0b
000259F8: move.b D0b,(A1)
000259FA: adda.w (A3)+,A1
000259FC: move.b (A0),D0b
000259FE: adda.w (A2)+,A0
00025A00: and.b D4b,D0b
00025A02: move.b (A0),D1b
00025A04: adda.w (A2)+,A0
00025A06: and.b D5b,D1b
00025A08: or.b D1b,D0b
00025A0A: move.b D0b,(A1)
00025A0C: adda.w (A3)+,A1
00025A0E: move.b (A0),D0b
00025A10: adda.w (A2)+,A0
00025A12: and.b D4b,D0b
00025A14: move.b (A0),D1b
00025A16: adda.w (A2)+,A0
00025A18: and.b D5b,D1b
00025A1A: or.b D1b,D0b
00025A1C: move.b D0b,(A1)
00025A1E: adda.w (A3)+,A1
00025A20: move.b (A0),D0b
00025A22: adda.w (A2)+,A0
00025A24: and.b D4b,D0b
00025A26: move.b (A0),D1b
00025A28: adda.w (A2)+,A0
00025A2A: and.b D5b,D1b
00025A2C: or.b D1b,D0b
00025A2E: move.b D0b,(A1)
00025A30: adda.w (A3)+,A1
00025A32: move.b (A0),D0b
00025A34: adda.w (A2)+,A0
00025A36: and.b D4b,D0b
00025A38: move.b (A0),D1b
00025A3A: adda.w (A2)+,A0
00025A3C: and.b D5b,D1b
00025A3E: or.b D1b,D0b
00025A40: move.b D0b,(A1)
00025A42: adda.w (A3)+,A1
00025A44: move.b (A0),D0b
00025A46: adda.w (A2)+,A0
00025A48: and.b D4b,D0b
00025A4A: move.b (A0),D1b
00025A4C: adda.w (A2)+,A0
00025A4E: and.b D5b,D1b
00025A50: or.b D1b,D0b
00025A52: move.b D0b,(A1)
00025A54: adda.w (A3)+,A1
00025A56: move.b (A0),D0b
00025A58: adda.w (A2)+,A0
00025A5A: and.b D4b,D0b
00025A5C: move.b (A0),D1b
00025A5E: adda.w (A2)+,A0
00025A60: and.b D5b,D1b
00025A62: or.b D1b,D0b
00025A64: move.b D0b,(A1)
00025A66: adda.w (A3)+,A1
00025A68: move.b (A0),D0b
00025A6A: adda.w (A2)+,A0
00025A6C: and.b D4b,D0b
00025A6E: move.b (A0),D1b
00025A70: adda.w (A2)+,A0
00025A72: and.b D5b,D1b
00025A74: or.b D1b,D0b
00025A76: move.b D0b,(A1)
00025A78: adda.w (A3)+,A1
00025A7A: move.b (A0),D0b
00025A7C: adda.w (A2)+,A0
00025A7E: and.b D4b,D0b
00025A80: move.b (A0),D1b
00025A82: adda.w (A2)+,A0
00025A84: and.b D5b,D1b
00025A86: or.b D1b,D0b
00025A88: move.b D0b,(A1)
00025A8A: adda.w (A3)+,A1
00025A8C: move.b (A0),D0b
00025A8E: adda.w (A2)+,A0
00025A90: and.b D4b,D0b
00025A92: move.b (A0),D1b
00025A94: adda.w (A2)+,A0
00025A96: and.b D5b,D1b
00025A98: or.b D1b,D0b
00025A9A: move.b D0b,(A1)
00025A9C: adda.w (A3)+,A1
00025A9E: move.b (A0),D0b
00025AA0: adda.w (A2)+,A0
00025AA2: and.b D4b,D0b
00025AA4: move.b (A0),D1b
00025AA6: adda.w (A2)+,A0
00025AA8: and.b D5b,D1b
00025AAA: or.b D1b,D0b
00025AAC: move.b D0b,(A1)
00025AAE: adda.w (A3)+,A1
00025AB0: move.b (A0),D0b
00025AB2: adda.w (A2)+,A0
00025AB4: and.b D4b,D0b
00025AB6: move.b (A0),D1b
00025AB8: adda.w (A2)+,A0
00025ABA: and.b D5b,D1b
00025ABC: or.b D1b,D0b
00025ABE: move.b D0b,(A1)
00025AC0: adda.w (A3)+,A1
00025AC2: move.b (A0),D0b
00025AC4: adda.w (A2)+,A0
00025AC6: and.b D4b,D0b
00025AC8: move.b (A0),D1b
00025ACA: adda.w (A2)+,A0
00025ACC: and.b D5b,D1b
00025ACE: or.b D1b,D0b
00025AD0: move.b D0b,(A1)
00025AD2: adda.w (A3)+,A1
00025AD4: move.b (A0),D0b
00025AD6: adda.w (A2)+,A0
00025AD8: and.b D4b,D0b
00025ADA: move.b (A0),D1b
00025ADC: adda.w (A2)+,A0
00025ADE: and.b D5b,D1b
00025AE0: or.b D1b,D0b
00025AE2: move.b D0b,(A1)
00025AE4: adda.w (A3)+,A1
00025AE6: move.b (A0),D0b
00025AE8: adda.w (A2)+,A0
00025AEA: and.b D4b,D0b
00025AEC: move.b (A0),D1b
00025AEE: adda.w (A2)+,A0
00025AF0: and.b D5b,D1b
00025AF2: or.b D1b,D0b
00025AF4: move.b D0b,(A1)
00025AF6: adda.w (A3)+,A1
00025AF8: move.b (A0),D0b
00025AFA: adda.w (A2)+,A0
00025AFC: and.b D4b,D0b
00025AFE: move.b (A0),D1b
00025B00: adda.w (A2)+,A0
00025B02: and.b D5b,D1b
00025B04: or.b D1b,D0b
00025B06: move.b D0b,(A1)
00025B08: adda.w (A3)+,A1
00025B0A: move.b (A0),D0b
00025B0C: adda.w (A2)+,A0
00025B0E: and.b D4b,D0b
00025B10: move.b (A0),D1b
00025B12: adda.w (A2)+,A0
00025B14: and.b D5b,D1b
00025B16: or.b D1b,D0b
00025B18: move.b D0b,(A1)
00025B1A: adda.w (A3)+,A1
00025B1C: move.b (A0),D0b
00025B1E: adda.w (A2)+,A0
00025B20: and.b D4b,D0b
00025B22: move.b (A0),D1b
00025B24: adda.w (A2)+,A0
00025B26: and.b D5b,D1b
00025B28: or.b D1b,D0b
00025B2A: move.b D0b,(A1)
00025B2C: adda.w (A3)+,A1
00025B2E: move.b (A0),D0b
00025B30: adda.w (A2)+,A0
00025B32: and.b D4b,D0b
00025B34: move.b (A0),D1b
00025B36: adda.w (A2)+,A0
00025B38: and.b D5b,D1b
00025B3A: or.b D1b,D0b
00025B3C: move.b D0b,(A1)
00025B3E: adda.w (A3)+,A1
00025B40: move.b (A0),D0b
00025B42: adda.w (A2)+,A0
00025B44: and.b D4b,D0b
00025B46: move.b (A0),D1b
00025B48: adda.w (A2)+,A0
00025B4A: and.b D5b,D1b
00025B4C: or.b D1b,D0b
00025B4E: move.b D0b,(A1)
00025B50: adda.w (A3)+,A1
00025B52: move.b (A0),D0b
00025B54: adda.w (A2)+,A0
00025B56: and.b D4b,D0b
00025B58: move.b (A0),D1b
00025B5A: adda.w (A2)+,A0
00025B5C: and.b D5b,D1b
00025B5E: or.b D1b,D0b
00025B60: move.b D0b,(A1)
00025B62: adda.w (A3)+,A1
00025B64: move.b (A0),D0b
00025B66: adda.w (A2)+,A0
00025B68: and.b D4b,D0b
00025B6A: move.b (A0),D1b
00025B6C: adda.w (A2)+,A0
00025B6E: and.b D5b,D1b
00025B70: or.b D1b,D0b
00025B72: move.b D0b,(A1)
00025B74: adda.w (A3)+,A1
00025B76: move.b (A0),D0b
00025B78: adda.w (A2)+,A0
00025B7A: and.b D4b,D0b
00025B7C: move.b (A0),D1b
00025B7E: adda.w (A2)+,A0
00025B80: and.b D5b,D1b
00025B82: or.b D1b,D0b
00025B84: move.b D0b,(A1)
00025B86: adda.w (A3)+,A1
00025B88: move.b (A0),D0b
00025B8A: adda.w (A2)+,A0
00025B8C: and.b D4b,D0b
00025B8E: move.b (A0),D1b
00025B90: adda.w (A2)+,A0
00025B92: and.b D5b,D1b
00025B94: or.b D1b,D0b
00025B96: move.b D0b,(A1)
00025B98: adda.w (A3)+,A1
00025B9A: move.b (A0),D0b
00025B9C: adda.w (A2)+,A0
00025B9E: and.b D4b,D0b
00025BA0: move.b (A0),D1b
00025BA2: adda.w (A2)+,A0
00025BA4: and.b D5b,D1b
00025BA6: or.b D1b,D0b
00025BA8: move.b D0b,(A1)
00025BAA: adda.w (A3)+,A1
00025BAC: move.b (A0),D0b
00025BAE: adda.w (A2)+,A0
00025BB0: and.b D4b,D0b
00025BB2: move.b (A0),D1b
00025BB4: adda.w (A2)+,A0
00025BB6: and.b D5b,D1b
00025BB8: or.b D1b,D0b
00025BBA: move.b D0b,(A1)
00025BBC: adda.w (A3)+,A1
00025BBE: move.b (A0),D0b
00025BC0: adda.w (A2)+,A0
00025BC2: and.b D4b,D0b
00025BC4: move.b (A0),D1b
00025BC6: adda.w (A2)+,A0
00025BC8: and.b D5b,D1b
00025BCA: or.b D1b,D0b
00025BCC: move.b D0b,(A1)
00025BCE: adda.w (A3)+,A1
00025BD0: move.b (A0),D0b
00025BD2: adda.w (A2)+,A0
00025BD4: and.b D4b,D0b
00025BD6: move.b (A0),D1b
00025BD8: adda.w (A2)+,A0
00025BDA: and.b D5b,D1b
00025BDC: or.b D1b,D0b
00025BDE: move.b D0b,(A1)
00025BE0: adda.w (A3)+,A1
00025BE2: move.b (A0),D0b
00025BE4: adda.w (A2)+,A0
00025BE6: and.b D4b,D0b
00025BE8: move.b (A0),D1b
00025BEA: adda.w (A2)+,A0
00025BEC: and.b D5b,D1b
00025BEE: or.b D1b,D0b
00025BF0: move.b D0b,(A1)
00025BF2: adda.w (A3)+,A1
00025BF4: move.b (A0),D0b
00025BF6: adda.w (A2)+,A0
00025BF8: and.b D4b,D0b
00025BFA: move.b (A0),D1b
00025BFC: adda.w (A2)+,A0
00025BFE: and.b D5b,D1b
00025C00: or.b D1b,D0b
00025C02: move.b D0b,(A1)
00025C04: adda.w (A3)+,A1
00025C06: move.b (A0),D0b
00025C08: adda.w (A2)+,A0
00025C0A: and.b D4b,D0b
00025C0C: move.b (A0),D1b
00025C0E: adda.w (A2)+,A0
00025C10: and.b D5b,D1b
00025C12: or.b D1b,D0b
00025C14: move.b D0b,(A1)
00025C16: adda.w (A3)+,A1
00025C18: move.b (A0),D0b
00025C1A: adda.w (A2)+,A0
00025C1C: and.b D4b,D0b
00025C1E: move.b (A0),D1b
00025C20: adda.w (A2)+,A0
00025C22: and.b D5b,D1b
00025C24: or.b D1b,D0b
00025C26: move.b D0b,(A1)
00025C28: adda.w (A3)+,A1
00025C2A: move.b (A0),D0b
00025C2C: adda.w (A2)+,A0
00025C2E: and.b D4b,D0b
00025C30: move.b (A0),D1b
00025C32: adda.w (A2)+,A0
00025C34: and.b D5b,D1b
00025C36: or.b D1b,D0b
00025C38: move.b D0b,(A1)
00025C3A: adda.w (A3)+,A1
00025C3C: move.b (A0),D0b
00025C3E: adda.w (A2)+,A0
00025C40: and.b D4b,D0b
00025C42: move.b (A0),D1b
00025C44: adda.w (A2)+,A0
00025C46: and.b D5b,D1b
00025C48: or.b D1b,D0b
00025C4A: move.b D0b,(A1)
00025C4C: adda.w (A3)+,A1
00025C4E: btst.l #0x0,D2
00025C52: beq.b 0x00025c5c
00025C54: move.b (A0),D0b
00025C56: and.b D4b,D0b
00025C58: move.b D0b,(A1)
00025C5A: adda.w (A3)+,A1
00025C5C: move.w (0x86,A6),D7w
00025C60: beq.b 0x00025c6e
00025C62: subq.w #0x1,D7w
00025C64: moveq #0x0,D0
00025C66: move.b D0b,(A1)
00025C68: adda.w (A3)+,A1
00025C6A: dbf D7w,0x00025c66
00025C6E: swap D7
00025C70: movea.l D3,A0
00025C72: adda.w (A4)+,A0
00025C74: movea.l D6,A1
00025C76: adda.w (A5)+,A1
00025C78: subq.w #0x1,(0x3c,A6)
00025C7C: beq.w 0x00025cfc
00025C80: dbf D7w,0x000250b0
00025C84: tst.b (0x24,A6)
00025C88: beq.b 0x00025cee
00025C8A: move.w (0x7e,A6),D7w
00025C8E: bne.b 0x00025cd2
00025C90: move.w (0x88,A6),D7w
00025C94: beq.w 0x00025cb2
00025C98: move.w D7w,D0w
00025C9A: moveq #0x20,D1
00025C9C: sub.w (0x80,A6),D1w
00025CA0: cmp.w D1w,D7w
00025CA2: ble.b 0x00025ca6
00025CA4: move.w D1w,D7w
00025CA6: sub.w D7w,(0x88,A6)
00025CAA: clr.w (0x80,A6)
00025CAE: bsr.w 0x00026074 ; call 00026074 FUN_00026074
00025CB2: move.l A1,D2
00025CB4: movea.l (0x1c,A6),A1
00025CB8: move.l A1,D3
00025CBA: sub.l D3,D2
00025CBC: move.w (0x84,A6),D0w
00025CC0: add.w D2w,(0x84,A6)
00025CC4: movea.l (0x28,A6),A2
00025CC8: jsr (A2)
00025CCA: move.w (0x88,A6),D7w
00025CCE: bne.b 0x00025c98
00025CD0: bra.b 0x00025cf8
00025CD2: move.l A1,D2
00025CD4: movea.l (0x1c,A6),A1
00025CD8: move.l A1,D3
00025CDA: sub.l D3,D2
00025CDC: move.w (0x84,A6),D0w
00025CE0: add.w D2w,(0x84,A6)
00025CE4: movea.l (0x28,A6),A2
00025CE8: jsr (A2)
00025CEA: bra.w 0x0002509a
00025CEE: move.w (0x88,A6),D7w
00025CF2: beq.b 0x00025cf8
00025CF4: bsr.w 0x00026074 ; call 00026074 FUN_00026074
00025CF8: moveq #0x0,D0
00025CFA: rts
00025CFC: movem.l { D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5},(0x3e,A6)
00025D02: lea (-0x84,PC),A0 ; data 00025c80
00025D06: move.l A0,(0x76,A6) ; data 00025c80
00025D0A: moveq #0x1,D0
00025D0C: bra.b 0x00025cfa
00026074: tst.w (0x7c,A6)
00026078: bgt.b 0x00026080
0002607A: lea (0x82c,A6),A3
0002607E: bra.b 0x00026084
00026080: lea (0x5ac,A6),A3
00026084: sub.w D7w,(0x7c,A6)
00026088: subq.w #0x1,D7w
0002608A: bmi.b 0x000260ac
0002608C: move.w (0x6,A3),D1w ; data 000005b2
00026090: addq.w #0x3,D1w
00026092: move.l A1,D6
00026094: move.w (0xc,A6),D5w
00026098: subq.w #0x1,D5w
0002609A: moveq #0x0,D0
0002609C: move.l D0,(A1)
0002609E: adda.w D1w,A1
000260A0: dbf D5w,0x0002609c
000260A4: movea.l D6,A1
000260A6: adda.w (A5)+,A1
000260A8: dbf D7w,0x0002608c
000260AC: rts
0002653E: clr.w D6w
00026540: move.b (A5)+,D6b
00026542: cmpi.b #-0x1,D6b
00026546: beq.w 0x0002660e
0002654A: moveq #0x2,D5
0002654C: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026552: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026558: jsr 0x00017cc8.l ; call 00017CC8 FUN_00017cc8
0002655E: jsr 0x00017fda.l ; call 00017FDA FUN_00017fda
00026564: dbf D5w,0x0002654c
00026568: clr.w D0w
0002656A: move.b (A5),D0b
0002656C: bpl.w 0x0002658e
00026570: lsl.b #0x1,D0b
00026572: bpl.w 0x00026584
00026576: addi.w #-0x1,(0x00ffdd08).l
0002657E: lsl.b #0x3,D0b
00026580: bpl.w 0x0002658a
00026584: addq.w #0x1,(0x00ffdd0a).l
0002658A: addq.l #0x1,A5
0002658C: bra.b 0x0002656a
0002658E: lsr.w #0x4,D0w
00026590: add.w D0w,(0x10,A6)
00026594: move.b (A5)+,D0b
00026596: bne.w 0x000265a0
0002659A: clr.w D5w
0002659C: move.b (A4)+,D5b
0002659E: bra.b 0x0002654c
000265A0: andi.b #0xf,D0b
000265A4: add.w D0w,(0x12,A6)
000265A8: moveq #0x3,D0
000265AA: jsr 0x00024e20.l ; call 00024E20 FUN_00024e20
000265B0: move SR,-(SP)
000265B2: move #0x2700,SR
000265B6: move.l #0x7260,D1
000265BC: lea (0xff1100).l,A1 ; data 00ff1100
000265C2: move.w (0x00fff118).l,D0w
000265C8: mulu.w #0x14,D0
000265CC: tst.b (0x0,A1,D0w*0x1) ; data 00ff1100
000265D0: bne.w 0x000265de
000265D4: move.l #0xe00,D2
000265DA: bra.w 0x000265e4
000265DE: move.l #0xf00,D2
000265E4: move.l #0xff8700,D0
000265EA: jsr 0x00000f98.l ; call 00000F98 FUN_00000f98
000265F0: move (SP)+,SR
000265F2: bsr.w 0x00026610 ; call 00026610 FUN_00026610
000265F6: bset.b #0x2,(0x00ffef14).l
00026600: ori.b #-0x1,D3b
00026604: roxl.b #0x7,D4b
00026606: dbf D6w,0x0002654a
0002660A: bra.w 0x0002653e
0002660E: rts
00026610: move SR,-(SP) ; data Stack[-0x2]
00026696: clr.l D0
00026698: movea.l (A5)+,A0
0002669A: lea (0xff8700).l,A1 ; data 00ff8700
000266A0: move.w (A6),D0w
000266A2: lsr.w #0x3,D0w
000266A4: move.w (0x2,A6),D1w
000266A8: lsr.w #0x3,D1w
000266AA: clr.l D3
000266AC: move.l D0,D3
000266AE: lsl.w #0x2,D3w
000266B0: subq.w #0x1,D1w
000266B2: move.l D0,D5
000266B4: subq.w #0x1,D5w
000266B6: move.l A1,-(SP) ; data 00ff8700
000266B8: clr.l D2
000266BA: move.w (A0)+,D2w
000266BC: lsl.w #0x5,D2w
000266BE: movea.l (A5),A2
000266C0: adda.l D2,A2
000266C2: move.l A1,-(SP) ; data 00ff8700
000266C4: moveq #0x7,D2
000266C6: move.l (A2)+,(A1)
000266C8: adda.l D3,A1
000266CA: dbf D2w,0x000266c6
000266CE: movea.l (SP)+,A1
000266D0: addq.l #0x4,A1
000266D2: dbf D5w,0x000266b8
000266D6: movea.l (SP)+,A1
000266D8: move.w (A6),D5w
000266DA: lsl.w #0x2,D5w
000266DC: adda.l D5,A1
000266DE: dbf D1w,0x000266b2
000266E2: rts
000266E4: moveq #0x0,D0
000266E6: jsr 0x00024e20.l ; call 00024E20 FUN_00024e20
000266EC: lea (0xffe192).l,A6 ; data 00ffe192
000266F2: move.w (A5),(A6)
000266F4: move.w (A5)+,(0x8,A6)
000266F8: move.w (A5),(0x2,A6)
000266FC: move.w (A5)+,(0xa,A6)
00026700: lea (0xff1100).l,A1 ; data 00ff1100
00026706: move.w (0x00fff118).l,D0w
0002670C: mulu.w #0x14,D0
00026710: tst.b (0x0,A1,D0w*0x1) ; data 00ff1100
00026714: bne.w 0x00026728
00026718: move.w #0xe,(0xc,A6)
0002671E: move.w #0x8,(0xe,A6)
00026724: bra.w 0x00026734
00026728: move.w #0xc,(0xc,A6)
0002672E: move.w #0xa,(0xe,A6)
00026734: move.w #0x0,(0x4,A6)
0002673A: move.w #0x0,(0x6,A6)
00026740: move.w #0x0,(0x10,A6)
00026746: move.w #0x0,(0x12,A6)
0002674C: move.w #0x1,(0x16,A6)
00026752: move.w #0x1,(0x14,A6)
00026758: bsr.w 0x00026696 ; call 00026696 FUN_00026696
0002675C: move.l #0xff8700,(0x18,A6)
00026764: move.l #0xff9700,(0x1c,A6)
0002676C: moveq #0x1,D0
0002676E: jsr 0x00024e20.l ; call 00024E20 FUN_00024e20
00026774: move.l #0xffb740,(0x1c,A6)
0002677C: moveq #0x2,D0
0002677E: jsr 0x00024e20.l ; call 00024E20 FUN_00024e20
00026784: lea (0xffb740).l,A1 ; data 00ffb740
0002678A: subq.w #0x1,(A1)
0002678C: move.w #0x11e,(0x00ffdd28).l
00026794: move.w #0xe4,(0x00ffdd2a).l
0002679C: rts
0002679E: move.b #-0x1,(0x00fff0e1).l
000267A6: lea (0xff1100).l,A1 ; data 00ff1100
000267AC: move.w (0x00fff118).l,D0w
000267B2: mulu.w #0x14,D0
000267B6: tst.b (0x0,A1,D0w*0x1) ; data 00ff1100
000267BA: bne.w 0x000267c8
000267BE: lea (0x14c11e).l,A5 ; data 0014c11e
000267C4: bra.w 0x000267ce
000267C8: lea (0x14c186).l,A5 ; data 0014c186
000267CE: lea (0xffe192).l,A6 ; data 00ffe192
000267D4: move.l #0xff9700,(0x18,A6)
000267DC: move.l #0xff8700,(0x1c,A6)
000267E4: lea (0x14c1eb).l,A4 ; data 0014c1eb
000267EA: clr.w D6w
000267EC: move.b (A5)+,D6b
000267EE: moveq #0x2,D5
000267F0: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000267F6: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000267FC: jsr 0x00017cc8.l ; call 00017CC8 FUN_00017cc8
00026802: jsr 0x00017fda.l ; call 00017FDA FUN_00017fda
00026808: dbf D5w,0x000267f0
0002680C: clr.l D0
0002680E: move.b (A5),D0b
00026810: bpl.w 0x0002683c
00026814: lsr.b #0x4,D0b
00026816: andi.b #0x7,D0b
0002681A: beq.w 0x00026824
0002681E: sub.w D0w,(0x00ffdd28).l
00026824: move.b (A5),D0b
00026826: andi.b #0x7,D0b
0002682A: beq.w 0x00026834
0002682E: sub.w D0w,(0x00ffdd2a).l
00026834: addq.l #0x1,A5
00026836: clr.w D5w
00026838: bra.b 0x000267f0
0002683C: lsr.w #0x4,D0w
0002683E: add.w D0w,(0x10,A6)
00026842: move.b (A5)+,D0b
00026844: bne.w 0x0002684e
00026848: clr.w D5w
0002684A: move.b (A4)+,D5b
0002684C: bra.b 0x000267f0
0002684E: andi.b #0xf,D0b
00026852: add.w D0w,(0x12,A6)
00026856: move.l #0xff8700,(0x1c,A6)
0002685E: moveq #0x3,D0
00026860: jsr 0x00024e20.l ; call 00024E20 FUN_00024e20
00026866: move SR,-(SP) ; data Stack[-0x2]
00026868: move #0x2700,SR
0002686C: move.l #0x7260,D1
00026872: lea (0xff1100).l,A1 ; data 00ff1100
00026878: move.w (0x00fff118).l,D0w
0002687E: mulu.w #0x14,D0
00026882: tst.b (0x0,A1,D0w*0x1) ; data 00ff1100
00026886: bne.w 0x00026894
0002688A: move.l #0xe00,D2
00026890: bra.w 0x0002689a
00026894: move.l #0xf00,D2
0002689A: move.l #0xff8700,D0
000268A0: jsr 0x00000f98.l ; call 00000F98 FUN_00000f98
000268A6: move (SP)+,SR
000268A8: movem.l {  A4 D6},-(SP) ; data 0014c1eb
000268AC: move.w #0x1,(0x00ffdd02).l
000268B4: lea (0xffd986).l,A3 ; data 00ffd986
000268BA: moveq #0x2,D5
000268BC: jsr 0x0001c3fa.l ; call 0001C3FA FUN_0001c3fa
000268C2: bsr.w 0x000268e6 ; call 000268E6 FUN_000268e6
000268C6: subq.w #0x8,A3
000268C8: clr.b (0x3,A3)
000268CC: move.w #0x1,(0x00ffd980).l
000268D4: bset.b #0x1,(0x00ffef14).l
000268DC: movem.l (SP)+,{  D6 A4}
000268E0: dbf D6w,0x000267ee
000268E4: rts
000268E6: lea (0xffb740).l,A2 ; data 00ffb740
000268EC: move.w (A2)+,D7w
000268EE: move.w (A2)+,D0w
000268F0: move.b D0b,(0x2,A3)
000268F4: move.w (A2)+,D0w
000268F6: addi.w #0x393,D0w
000268FA: move.w D0w,(0x4,A3)
000268FE: move.w (A2)+,D0w
00026900: add.w (0x00ffdd2a).l,D0w
00026906: move.w D0w,(A3)
00026908: move.w (A2)+,D0w
0002690A: add.w (0x00ffdd28).l,D0w
00026910: move.w D0w,(0x6,A3)
00026914: move.b D5b,(0x3,A3)
00026918: addq.w #0x1,D5w
0002691A: addq.w #0x1,(0x00ffdd02).l
00026920: addq.l #0x8,A3
00026922: move.l A3,(0x00ffdd18).l
00026928: move.w (0x00ffdd02).l,(0x00ffdd1c).l
00026932: dbf D7w,0x000268ee
00026936: rts
00026938: lea (0xfffbbe).l,A1 ; data 00fffbbe
0002693E: lea (0x14c1f4).l,A0 ; data 0014c1f4
00026944: move.w #0x0,D6w
00026948: move.w #0x16,D7w
0002694C: move.w (A0)+,(A1)+
0002694E: dbf D7w,0x0002694c
00026952: dbf D6w,0x00026948
00026956: rts
00026958: lea (0xffbf40).l,A1 ; data 00ffbf40
0002695E: movea.l #0x14c222,A3
00026964: move.l #0x8c40,D0
0002696A: jsr 0x00001530.l ; call 00001530 FUN_00001530
00026970: rts
00026972: tst.b (0x00fff0d2).l
00026978: beq.w 0x000269fe
0002697C: tst.w (0x00ffe0ee).l
00026982: beq.w 0x000269ae
00026986: subq.w #0x1,(0x00ffe0f0).l
0002698C: bpl.w 0x0002699a
00026990: clr.w (0x00ffe0ee).l
00026996: bra.w 0x000269ae
0002699A: move.w (0x00ffdf88).l,D0w
000269A0: andi.w #0x3,D0w
000269A4: cmpi.w #0x1,D0w
000269A8: ble.w 0x000269ae
000269AC: rts
000269AE: lea (0xff1580).l,A0 ; data 00ff1580
000269B4: cmpi.l #0x1c31e,(0x4,A0)
000269BC: bne.w 0x000269ca
000269C0: lea (0x14cd0e).l,A2 ; data 0014cd0e
000269C6: bra.w 0x000269e6
000269CA: cmpi.l #0x1c32a,(0x4,A0)
000269D2: bne.w 0x000269e0
000269D6: lea (0x14cd30).l,A2 ; data 0014cd30
000269DC: bra.w 0x000269e6
000269E0: lea (0x14ccec).l,A2 ; data 0014ccec
000269E6: lea (0xff1580).l,A1 ; data 00ff1580
000269EC: move.w (0xe,A1),D1w
000269F0: move.w (0x10,A1),D2w
000269F4: move.w #0x6000,D6w
000269F8: jsr 0x00001378.l ; call 00001378 FUN_00001378
000269FE: rts
00026A00: move.b #-0x1,(0x00fff0b1).l
00026A08: move.w #0x29,D0w
00026A0C: jsr 0x000530de.l ; call 000530DE FUN_000530de
00026A12: move.l #0x15be02,D0
00026A18: move.l #0x9620,D1
00026A1E: move.l #0xa0,D2
00026A24: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00026A2A: jsr 0x00010952.l ; call 00010952 FUN_00010952
00026A30: lea (0xe2e3a).l,A0 ; data 000e2e3a
00026A36: jsr 0x00001358.l ; call 00001358 copy_source_word_block
00026A3C: bset.b #0x0,(0x00ffef14).l
00026A44: lea (0x15bea6).l,A1 ; data 0015bea6
00026A4A: move.l #0xe700,D0
00026A50: move.w #0x27,D3w
00026A54: move.w #0x6000,D2w
00026A58: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026A5E: lea (0x15bea8).l,A1 ; data 0015bea8
00026A64: move.l #0xe028,D0
00026A6A: move.w #0x27,D3w
00026A6E: jsr 0x00026c26.l ; call 00026C26 FUN_00026c26
00026A74: lea (0x15bea2).l,A1 ; data 0015bea2
00026A7A: move.l #0xe728,D0
00026A80: move.w #0x0,D3w
00026A84: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026A8A: moveq #0xd,D1
00026A8C: move.l #0xe6fe,D4
00026A92: move.l #0xe752,D5
00026A98: move.l #0xdfa8,D6
00026A9E: move.l #0xeea8,D7
00026AA4: movem.l {  D7 D6 D5 D4 D3 D2 D1},-(SP) ; data 0000dfa8
00026AA8: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026AAE: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026AB4: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00026ABA: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7}
00026ABE: lea (0x15beaa).l,A1 ; data 0015beaa
00026AC4: addq.l #0x2,D4
00026AC6: move.l D4,D0
00026AC8: move.w #0x0,D3w
00026ACC: move.w #0x6000,D2w
00026AD0: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026AD6: subq.l #0x2,D5
00026AD8: move.l D5,D0
00026ADA: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026AE0: addi.l #0x80,D6
00026AE6: move.l D6,D0
00026AE8: jsr 0x00026c26.l ; call 00026C26 FUN_00026c26
00026AEE: subi.l #0x80,D7
00026AF4: move.l D7,D0
00026AF6: jsr 0x00026c26.l ; call 00026C26 FUN_00026c26
00026AFC: dbf D1w,0x00026aa4
00026B00: moveq #0x5,D1
00026B02: movem.l {  D5 D4 D3 D2 D1},-(SP)
00026B06: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026B0C: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026B12: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00026B18: movem.l (SP)+,{  D1 D2 D3 D4 D5}
00026B1C: lea (0x15beaa).l,A1 ; data 0015beaa
00026B22: addq.l #0x2,D4
00026B24: move.l D4,D0
00026B26: move.w #0x0,D3w
00026B2A: move.w #0x6000,D2w
00026B2E: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026B34: subq.l #0x2,D5
00026B36: move.l D5,D0
00026B38: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026B3E: dbf D1w,0x00026b02
00026B42: lea (0x15bea4).l,A1 ; data 0015bea4
00026B48: move.l #0xe728,D0
00026B4E: move.w #0x0,D3w
00026B52: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026B58: moveq #0xa,D5
00026B5A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026B60: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026B66: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00026B6C: dbf D5w,0x00026b5a
00026B70: lea (0x15bea2).l,A1 ; data 0015bea2
00026B76: move.l #0xe728,D0
00026B7C: move.w #0x0,D3w
00026B80: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026B86: moveq #0x5,D5
00026B88: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026B8E: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026B94: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00026B9A: dbf D5w,0x00026b88
00026B9E: lea (0x15bea4).l,A1 ; data 0015bea4
00026BA4: move.l #0xe728,D0
00026BAA: move.w #0x0,D3w
00026BAE: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026BB4: moveq #0x5,D5
00026BB6: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026BBC: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026BC2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00026BC8: dbf D5w,0x00026bb6
00026BCC: lea (0x15beaa).l,A1 ; data 0015beaa
00026BD2: move.l #0xe728,D0
00026BD8: move.w #0x0,D3w
00026BDC: jsr 0x00026bfe.l ; call 00026BFE FUN_00026bfe
00026BE2: moveq #0xa,D5
00026BE4: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00026BEA: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00026BF0: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00026BF6: dbf D5w,0x00026be4
00026BFA: clr.w D7w
00026BFC: rts
00026BFE: movem.l {  D3 D2 D1 D0},-(SP)
00026C02: move SR,-(SP) ; data Stack[-0x12]
00026C04: move #0x2700,SR
00026C08: lea (0xc00000).l,A0 ; data 00c00000
00026C0E: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00026C14: move.w (A1),D0w
00026C16: or.w D2w,D0w
00026C18: move.w D0w,(A0)
00026C1A: dbf D3w,0x00026c14
00026C1E: move (SP)+,SR
00026C20: movem.l (SP)+,{ D0 D1 D2 D3}
00026C24: rts
00026C26: movem.l {  D3 D2 D1 D0},-(SP)
00026C2A: move SR,-(SP) ; data Stack[-0x12]
00026C2C: move #0x2700,SR
00026C30: move.w #-0x7080,(0x00c00004).l
00026C38: lea (0xc00000).l,A0 ; data 00c00000
00026C3E: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
00026C44: move.w (A1),D0w
00026C46: or.w D2w,D0w
00026C48: move.w D0w,(A0)
00026C4A: dbf D3w,0x00026c44
00026C4E: move.w #-0x70fe,(0x00c00004).l
00026C56: move (SP)+,SR
00026C58: movem.l (SP)+,{ D0 D1 D2 D3}
00026C5C: rts
00052DC2: move.w #0x100,(0x00a11100).l
00052DCA: btst.b #0x0,(0x00a11100).l
00052DD2: bne.b 0x00052dca
00052DD4: rts
00052DD6: move.w #0x0,(0x00a11100).l
00052DDE: rts
00052DE0: move.l A1,-(SP) ; data Stack[-0x4]
00052DE2: move SR,-(SP) ; data Stack[-0x6]
00052DE4: ori #0x700,SR
00052DE8: move.w #0x100,(0x00a11200).l
00052DF0: jsr 0x00052dc2.l ; call 00052DC2 FUN_00052dc2
00052DF6: lea (0x26c5e).l,A0 ; data 00026c5e
00052DFC: lea (0x284e4).l,A1 ; data 000284e4
00052E02: move.l A1,D0
00052E04: sub.l A0,D0
00052E06: subq.w #0x1,D0w
00052E08: lea (0xa00000).l,A1 ; data 00a00000
00052E0E: move.b (A0)+,(A1)+
00052E10: dbf D0w,0x00052e0e
00052E14: move.b #0x0,(A1)+
00052E18: cmpa.l #0xa02000,A1
00052E1E: bne.b 0x00052e14
00052E20: move (SP)+,SR
00052E22: movea.l (SP)+,A1
00052E24: rts
00052E26: move SR,-(SP) ; data Stack[-0x2]
00052E28: ori #0x700,SR
00052E2C: move.w #0x0,(0x00a11200).l
00052E34: moveq #0xf,D0
00052E36: subq.l #0x1,D0
00052E38: bne.b 0x00052e36
00052E3A: move.w #0x0,(0x00a11100).l
00052E42: move.w #0x100,(0x00a11200).l
00052E4A: move (SP)+,SR
00052E4C: rts
00052E4E: movea.l (SP)+,A0
00052E50: link.w A6,0x0 ; data Stack[0x0]
00052E54: movem.l {  A1 D1},-(SP)
00052E58: move SR,-(SP) ; data Stack[-0xa]
00052E5A: move.l A0,-(SP) ; data Stack[-0xe]
00052E5C: lea (0xa00036).l,A0 ; data 00a00036
00052E62: lea (0xa01b40).l,A1 ; data 00a01b40
00052E68: ori #0x700,SR
00052E6C: move.w #0x100,(0x00a11100).l
00052E74: btst.b #0x0,(0x00a11100).l
00052E7C: bne.b 0x00052e74
00052E7E: move.b (A0),D1b
00052E80: ext.w D1w
00052E82: rts
00052E84: move.w #0x0,(0x00a11100).l
00052E8C: move (SP)+,SR
00052E8E: movem.l (SP)+,{  D1 A1}
00052E92: unlk A6
00052E94: rts
00052E96: move.b #-0x1,(0x0,A1,D1w*0x1)
00052E9C: addq.b #0x1,D1b
00052E9E: andi.b #0x3f,D1b
00052EA2: move.b D0b,(0x0,A1,D1w*0x1)
00052EA6: addq.b #0x1,D1b
00052EA8: andi.b #0x3f,D1b
00052EAC: move.b D1b,(A0)
00052EAE: rts
00052EB0: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
00052EB6: move.l (0x8,A6),D0
00052EBA: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052EC0: jmp 0x00052e84.l ; call 00052E84 FUN_00052e84
00052EC6: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
00052ECC: move.l (0x8,A6),D0
00052ED0: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052ED6: asr.l #0x8,D0
00052ED8: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052EDE: asr.l #0x8,D0
00052EE0: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052EE6: jmp 0x00052e84.l ; call 00052E84 FUN_00052e84
00052EEC: link.w A6,0x0
00052EF0: jsr 0x00052de0.l ; call 00052DE0 FUN_00052de0
00052EF6: jsr 0x00052e26.l ; call 00052E26 FUN_00052e26
00052EFC: moveq #-0x1,D0
00052EFE: move.l D0,-(SP) ; data Stack[-0x8]
00052F00: jsr 0x00052eb0.l ; call 00052EB0 FUN_00052eb0
00052F06: moveq #0xb,D0
00052F08: move.l D0,-(SP) ; data Stack[-0xc]
00052F0A: jsr 0x00052eb0.l ; call 00052EB0 FUN_00052eb0
00052F10: move.l (0x8,A6),-(SP)
00052F14: jsr 0x00052ec6.l ; call 00052EC6 FUN_00052ec6
00052F1A: move.l (0xc,A6),-(SP)
00052F1E: jsr 0x00052ec6.l ; call 00052EC6 FUN_00052ec6
00052F24: move.l (0x10,A6),-(SP)
00052F28: jsr 0x00052ec6.l ; call 00052EC6 FUN_00052ec6
00052F2E: move.l (0x14,A6),-(SP)
00052F32: jsr 0x00052ec6.l ; call 00052EC6 FUN_00052ec6
00052F38: unlk A6
00052F3A: rts
00052F3C: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
00052F42: moveq #0x10,D0
00052F44: jsr 0x00052e96.l ; call 00052E96 FUN_00052e96
00052F4A: move.l (0x8,A6),D0
00052F4E: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052F54: jmp 0x00052e84.l ; call 00052E84 FUN_00052e84
00052F5A: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
00052F60: moveq #0x12,D0
00052F62: bra.b 0x00052f44
00052F6E: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
00052F74: moveq #0x20,D0
00052F76: bra.b 0x00052f44
00052FA0: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
00052FA6: moveq #0x16,D0
00052FA8: jsr 0x00052e96.l ; call 00052E96 FUN_00052e96
00052FAE: jmp 0x00052e84.l ; call 00052E84 FUN_00052e84
00052FD2: jsr 0x00052e96.l ; call 00052E96 FUN_00052e96
00052FD8: move.l (0x8,A6),D0
00052FDC: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052FE2: move.l (0xc,A6),D0
00052FE6: jsr 0x00052ea2.l ; call 00052EA2 FUN_00052ea2
00052FEC: jmp 0x00052e84.l ; call 00052E84 FUN_00052e84
00053094: jsr 0x00052e4e.l ; call 00052E4E FUN_00052e4e
0005309A: moveq #0x1b,D0
0005309C: bra.w 0x00052fd2 ; call 00052FD2 FUN_00052fd2
000530A0: move SR,-(SP) ; data Stack[-0x2]
000530A2: ori #0x700,SR
000530A6: jsr 0x00052dc2.l ; call 00052DC2 FUN_00052dc2
000530AC: moveq #0x0,D0
000530AE: move.b (0x9,SP),D0b
000530B2: lea (0xa01b22).l,A0 ; data 00a01b22
000530B8: move.b (0x0,A0,D0w*0x1),D0b ; data 00a01b22
000530BC: jsr 0x00052dd6.l ; call 00052DD6 FUN_00052dd6
000530C2: move (SP)+,SR
000530C4: rts
000530D2: tst.w (0x00fffc0a).l
000530D8: bne.w 0x00053112
000530DC: rts
000530DE: tst.w (0x00fffc0a).l
000530E4: bne.w 0x000530fe
000530E8: rts
000530EA: tst.w (0x00fffc08).l
000530F0: beq.w 0x000530fc
000530F4: bsr.w 0x00053112 ; call 00053112 FUN_00053112
000530F8: bra.w 0x000530fe
000530FC: rts
000530FE: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00053102: move.l D0,-(SP) ; data Stack[-0x40]
00053104: jsr 0x00052f3c.l ; call 00052F3C FUN_00052f3c
0005310A: addq.w #0x4,SP
0005310C: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00053110: rts
00053112: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00053116: move.l D0,-(SP) ; data Stack[-0x40]
00053118: jsr 0x00052f5a.l ; call 00052F5A FUN_00052f5a
0005311E: addq.w #0x4,SP
00053120: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00053124: rts
00053126: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0005312A: moveq #0x0,D0
0005312C: move.w #0x5,D6w
00053130: addi.b #0x14,D0b
00053134: bsr.w 0x0005315e ; call 0005315E FUN_0005315e
00053138: move.w #0x2,D5w
0005313C: jsr 0x00000c24.l ; call 00000C24 FUN_00000c24
00053142: dbf D6w,0x00053130
00053146: jsr 0x00052fa0.l ; call 00052FA0 FUN_00052fa0
0005314C: bsr.w 0x00053156 ; call 00053156 FUN_00053156
00053150: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00053154: rts
00053156: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0005315A: clr.l D0
0005315C: bra.b 0x00053162
0005315E: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00053162: move.l D0,-(SP) ; data Stack[-0x40]
00053164: jsr 0x00052f6e.l ; call 00052F6E FUN_00052f6e
0005316A: addq.w #0x4,SP
0005316C: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00053170: rts
00053172: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
00053176: moveq #0x0,D0
00053178: bra.b 0x00053180
00053180: move.l D0,-(SP) ; data Stack[-0x40]
00053182: moveq #0x1,D0
00053184: move.l D0,-(SP) ; data Stack[-0x44]
00053186: jsr 0x00053094.l ; call 00053094 FUN_00053094
0005318C: addq.w #0x8,SP
0005318E: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
00053192: rts
00053194: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1},-(SP)
00053198: moveq #0x1,D0
0005319A: move.l D0,-(SP) ; data Stack[-0x3c]
0005319C: jsr 0x000530a0.l ; call 000530A0 FUN_000530a0
000531A2: addq.w #0x4,SP
000531A4: movem.l (SP)+,{  D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
000531A8: rts
000531AA: move.l #0x311bf,-(SP) ; data 000311bf
000531B0: move.l #0x29609,-(SP) ; data 00029609
000531B6: move.l #0x29595,-(SP) ; data 00029595
000531BC: move.l #0x284e4,-(SP) ; data 000284e4
000531C2: jsr 0x00052eec.l ; call 00052EEC FUN_00052eec
000531C8: adda.w #0x10,SP
000531CC: jmp 0x00052fa0.l ; call 00052FA0 FUN_00052fa0
000531EA: bsr.w 0x00053216 ; call 00053216 FUN_00053216
000531EE: lea (0xff1100).l,A1 ; data 00ff1100
000531F4: move.w #0x9f,D7w
000531F8: move.l (A0)+,(A1)+
000531FA: dbf D7w,0x000531f8
000531FE: rts
00053200: clr.l D0
00053202: move.b (0x00fffbbc).l,D0b
00053208: lea (0x15db48).l,A0 ; data 0015db48
0005320E: lsl.w #0x3,D0w
00053210: movea.l (0x0,A0,D0w*0x1),A0 ; data 0015db48
00053214: rts
00053216: clr.l D0
00053218: move.b (0x00fffbbc).l,D0b
0005321E: lea (0x15db48).l,A0 ; data 0015db48
00053224: lsl.w #0x3,D0w
00053226: movea.l (0x4,A0,D0w*0x1),A0 ; data 0015db4c
0005322A: rts
0005322C: bsr.w 0x00053248 ; call 00053248 FUN_00053248
00053230: subq.w #0x8,A3
00053232: clr.b (0x3,A3) ; data fffffffb
0005323C: addx.l D0,D4
0005323E: bset.b #0x1,(0x00ffef14).l
00053246: rts
00053248: lea (0x15fbfa).l,A2 ; data 0015fbfa
0005324E: lea (0xffd986).l,A3 ; data 00ffd986
00053254: move.w #0x1,(0x00ffdd02).l
0005325C: move.w #0x2,D5w
00053260: clr.l D1
00053262: clr.l D2
00053264: lea (0xff1100).l,A1 ; data 00ff1100
0005326A: move.w (0x00fff118).l,D0w
00053270: mulu.w #0x14,D0
00053274: move.b (0x3,A1,D0w*0x1),D1b ; data 00ff1103
00053278: move.b (0x4,A1,D0w*0x1),D2b ; data 00ff1104
0005327C: mulu.w #0x10,D1
00053280: mulu.w #0x10,D2
00053284: move.w #0x2000,D6w
00053288: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005328E: rts
00053290: lea (0xfffaa4).l,A3 ; data 00fffaa4
00053296: lea (0xfffb30).l,A4 ; data 00fffb30
0005329C: moveq #0x22,D7
0005329E: clr.l (A3)+
000532A0: clr.l (A4)+
000532A2: dbf D7w,0x0005329e
000532A6: rts
000532A8: lea (0xfffaa4).l,A0 ; data 00fffaa4
000532AE: moveq #0x22,D7
000532B0: move.l #-0x1,(A0)+
000532B6: dbf D7w,0x000532b0
000532BA: rts
000532BC: move.w #-0x7ecc,(0x00c00004).l
000532C4: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
000532CA: lea (0x15fcb6).l,A0 ; data 0015fcb6
000532D0: jsr 0x00001358.l ; call 00001358 copy_source_word_block
000532D6: bset.b #0x0,(0x00ffef14).l
000532DE: move.l #0x15beac,D0
000532E4: move.l #0x17e0,D1
000532EA: move.l #0x520,D2
000532F0: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
000532F6: lea (0x15c4dc).l,A1 ; data 0015c4dc
000532FC: move.l #0xe000,D4
00053302: move.w #0x27,D7w
00053306: move.w #0x1b,D1w
0005330A: move.w #0x0,D2w
0005330E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00053314: lea (0x15fd36).l,A1 ; data 0015fd36
0005331A: movea.l #0xc204,A2
00053320: move.w #0x2000,D1w
00053324: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005332A: bsr.w 0x000533ee ; call 000533EE FUN_000533ee
0005332E: jsr 0x0005322c.l ; call 0005322C FUN_0005322c
00053334: bsr.w 0x00053200 ; call 00053200 FUN_00053200
00053338: lea (0xfffaa4).l,A3 ; data 00fffaa4
0005333E: lea (0xfffb30).l,A4 ; data 00fffb30
00053344: move.l #0xc190,D4
0005334A: move.w #0x9,D6w
0005334E: move.w #0x2000,D2w
00053352: move.w #0xd,D5w
00053356: move.l D4,-(SP) ; data Stack[-0x4]
00053358: tst.b (A3)+
0005335A: bmi.w 0x0005336c
0005335E: lea (0x15c3cc).l,A1 ; data 0015c3cc
00053364: addq.l #0x1,A0
00053366: addq.l #0x1,A4
00053368: bra.w 0x000533c8
0005336C: move.b (A4)+,D0b
0005336E: andi.b #0xf,D0b
00053372: bne.w 0x0005337e
00053376: move.w #0x2000,D2w
0005337A: bra.w 0x000533b8
0005337E: cmpi.b #0x1,D0b
00053382: beq.w 0x000533b4
00053386: cmpi.b #0x2,D0b
0005338A: beq.w 0x000533ac
0005338E: cmpi.b #0x3,D0b
00053392: beq.w 0x0005339e
00053396: move.w #0x6000,D2w
0005339A: bra.w 0x000533a2
0005339E: move.w #0x4000,D2w
000533A2: lea (0x15fc6a).l,A1 ; data 0015fc6a
000533A8: bra.w 0x000533be
000533AC: move.w #0x6000,D2w
000533B0: bra.w 0x000533b8
000533B4: move.w #0x4000,D2w
000533B8: lea (0x15fc0e).l,A1 ; data 0015fc0e
000533BE: clr.w D3w
000533C0: move.b (A0)+,D3b
000533C2: lsl.w #0x2,D3w
000533C4: movea.l (0x0,A1,D3w*0x1),A1 ; data 0015fc0e
000533C8: moveq #0x1,D1
000533CA: moveq #0x1,D7
000533CC: jsr 0x00001232.l ; call 00001232 FUN_00001232
000533D2: addq.l #0x4,D4
000533D4: dbf D5w,0x00053358
000533D8: move.l (SP)+,D4
000533DA: addi.l #0x100,D4
000533E0: dbf D6w,0x00053352
000533E4: move.w #-0x7e8c,(0x00c00004).l
000533EC: rts
000533EE: bsr.w 0x00053200 ; call 00053200 FUN_00053200
000533F2: lea (0xff1100).l,A1 ; data 00ff1100
000533F8: clr.l D0
000533FA: move.w (0x00fff118).l,D0w
00053400: mulu.w #0x14,D0
00053404: clr.l D1
00053406: move.b (0x4,A1,D0w*0x1),D1b ; data 00ff1104
0005340A: mulu.w #0xe,D1
0005340E: add.b (0x3,A1,D0w*0x1),D1b ; data 00ff1103
00053412: move.w D1w,(0x00fff102).l
00053418: lea (0xfffaa4).l,A1 ; data 00fffaa4
0005341E: move.b #-0x1,(0x0,A1,D1w*0x1) ; data 00fffaa4
00053424: move.w (0x00fff102).l,(0x00fff104).l
0005342E: clr.w (0x00fffbfc).l
00053434: btst.b #0x1,(0x00fffbfc).l
0005343C: bne.w 0x000534e8
00053440: subi.w #0xe,(0x00fff104).l
00053448: bpl.w 0x00053458
0005344C: addi.w #0xe,(0x00fff104).l
00053454: bra.w 0x000534e8
00053458: move.w (0x00fff104).l,D0w
0005345E: move.w #0xb,D1w
00053462: cmp.b (0x0,A0,D0w*0x1),D1b
00053466: bpl.w 0x000534b0
0005346A: lea (0x15fcaf).l,A2 ; data 0015fcaf
00053470: moveq #0x6,D7
00053472: move.b (A2)+,D1b
00053474: cmp.b (0x0,A0,D0w*0x1),D1b
00053478: beq.w 0x00053482
0005347C: dbf D7w,0x00053472
00053480: bra.b 0x0005344c
00053482: move.b #-0x1,(0x0,A1,D0w*0x1)
00053488: move.w (0x00fffbfc).l,-(SP)
0005348E: clr.w (0x00fffbfc).l
00053494: bset.b #0x3,(0x00fffbfc).l
0005349C: bsr.b 0x00053434 ; call 00053434 FUN_00053434
0005349E: move.w (SP)+,(0x00fffbfc).l
000534A4: addi.w #0xe,(0x00fff104).l
000534AC: bra.w 0x000534e8
000534B0: lea (0x15fca8).l,A2 ; data 0015fca8
000534B6: addi.w #0xe,D0w
000534BA: moveq #0x6,D7
000534BC: move.b (A2)+,D1b
000534BE: cmp.b (0x0,A0,D0w*0x1),D1b
000534C2: beq.w 0x000534d6
000534C6: dbf D7w,0x000534bc
000534CA: addi.w #0xe,(0x00fff104).l
000534D2: bra.w 0x000534e8
000534D6: subi.w #0xe,D0w
000534DA: move.b #-0x1,(0x0,A1,D0w*0x1)
000534E0: addi.w #0xe,(0x00fff104).l
000534E8: btst.b #0x2,(0x00fffbfc).l
000534F0: bne.w 0x0005359c
000534F4: addq.w #0x1,(0x00fff104).l
000534FA: move.l #0x8b,D0
00053500: cmp.w (0x00fff104).l,D0w
00053506: bpl.w 0x00053514
0005350A: subq.w #0x1,(0x00fff104).l
00053510: bra.w 0x0005359c
00053514: move.w (0x00fff104).l,D0w
0005351A: move.w #0xb,D1w
0005351E: cmp.b (0x0,A0,D0w*0x1),D1b
00053522: bpl.w 0x0005356c
00053526: lea (0x15fc9a).l,A2 ; data 0015fc9a
0005352C: moveq #0x6,D7
0005352E: move.b (A2)+,D1b
00053530: cmp.b (0x0,A0,D0w*0x1),D1b
00053534: beq.w 0x0005353e
00053538: dbf D7w,0x0005352e
0005353C: bra.b 0x0005350a
0005353E: move.b #-0x1,(0x0,A1,D0w*0x1)
00053544: move.w (0x00fffbfc).l,-(SP)
0005354A: clr.w (0x00fffbfc).l
00053550: bset.b #0x4,(0x00fffbfc).l
00053558: bsr.w 0x00053434 ; call 00053434 FUN_00053434
0005355C: move.w (SP)+,(0x00fffbfc).l
00053562: subq.w #0x1,(0x00fff104).l
00053568: bra.w 0x0005359c
0005356C: lea (0x15fca1).l,A2 ; data 0015fca1
00053572: subq.w #0x1,D0w
00053574: moveq #0x6,D7
00053576: move.b (A2)+,D1b
00053578: cmp.b (0x0,A0,D0w*0x1),D1b
0005357C: beq.w 0x0005358e
00053580: dbf D7w,0x00053576
00053584: subq.w #0x1,(0x00fff104).l
0005358A: bra.w 0x0005359c
0005358E: addq.w #0x1,D0w
00053590: move.b #-0x1,(0x0,A1,D0w*0x1)
00053596: subq.w #0x1,(0x00fff104).l
0005359C: btst.b #0x3,(0x00fffbfc).l
000535A4: bne.w 0x0005365e
000535A8: addi.w #0xe,(0x00fff104).l
000535B0: move.l #0x8b,D0
000535B6: cmp.w (0x00fff104).l,D0w
000535BC: bpl.w 0x000535cc
000535C0: subi.w #0xe,(0x00fff104).l
000535C8: bra.w 0x0005365e
000535CC: move.w (0x00fff104).l,D0w
000535D2: move.w #0xb,D1w
000535D6: cmp.b (0x0,A0,D0w*0x1),D1b
000535DA: bpl.w 0x00053626
000535DE: lea (0x15fca8).l,A2 ; data 0015fca8
000535E4: moveq #0x6,D7
000535E6: move.b (A2)+,D1b
000535E8: cmp.b (0x0,A0,D0w*0x1),D1b
000535EC: beq.w 0x000535f6
000535F0: dbf D7w,0x000535e6
000535F4: bra.b 0x000535c0
000535F6: move.b #-0x1,(0x0,A1,D0w*0x1)
000535FC: move.w (0x00fffbfc).l,-(SP)
00053602: clr.w (0x00fffbfc).l
00053608: bset.b #0x1,(0x00fffbfc).l
00053610: bsr.w 0x00053434 ; call 00053434 FUN_00053434
00053614: move.w (SP)+,(0x00fffbfc).l
0005361A: subi.w #0xe,(0x00fff104).l
00053622: bra.w 0x0005365e
00053626: lea (0x15fcaf).l,A2 ; data 0015fcaf
0005362C: subi.w #0xe,D0w
00053630: moveq #0x6,D7
00053632: move.b (A2)+,D1b
00053634: cmp.b (0x0,A0,D0w*0x1),D1b
00053638: beq.w 0x0005364c
0005363C: dbf D7w,0x00053632
00053640: subi.w #0xe,(0x00fff104).l
00053648: bra.w 0x0005365e
0005364C: addi.w #0xe,D0w
00053650: move.b #-0x1,(0x0,A1,D0w*0x1)
00053656: subi.w #0xe,(0x00fff104).l
0005365E: btst.b #0x4,(0x00fffbfc).l
00053666: beq.w 0x0005366c
0005366A: rts
0005366C: subq.w #0x1,(0x00fff104).l
00053672: bpl.w 0x0005367e
00053676: addq.w #0x1,(0x00fff104).l
0005367C: rts
0005367E: move.w (0x00fff104).l,D0w
00053684: move.w #0xb,D1w
00053688: cmp.b (0x0,A0,D0w*0x1),D1b
0005368C: bpl.w 0x000536d4
00053690: lea (0x15fca1).l,A2 ; data 0015fca1
00053696: moveq #0x6,D7
00053698: move.b (A2)+,D1b
0005369A: cmp.b (0x0,A0,D0w*0x1),D1b
0005369E: beq.w 0x000536a8
000536A2: dbf D7w,0x00053698
000536A6: bra.b 0x00053676
000536A8: move.b #-0x1,(0x0,A1,D0w*0x1)
000536AE: move.w (0x00fffbfc).l,-(SP)
000536B4: clr.w (0x00fffbfc).l
000536BA: bset.b #0x2,(0x00fffbfc).l
000536C2: bsr.w 0x00053434 ; call 00053434 FUN_00053434
000536C6: move.w (SP)+,(0x00fffbfc).l
000536CC: addq.w #0x1,(0x00fff104).l
000536D2: rts
000536D4: lea (0x15fc9a).l,A2 ; data 0015fc9a
000536DA: addq.w #0x1,D0w
000536DC: moveq #0x6,D7
000536DE: move.b (A2)+,D1b
000536E0: cmp.b (0x0,A0,D0w*0x1),D1b
000536E4: beq.w 0x000536f4
000536E8: dbf D7w,0x000536de
000536EC: addq.w #0x1,(0x00fff104).l
000536F2: rts
000536F4: subq.w #0x1,D0w
000536F6: move.b #-0x1,(0x0,A1,D0w*0x1)
000536FC: addq.w #0x1,(0x00fff104).l
00053702: rts
00053704: bsr.w 0x000532bc ; call 000532BC FUN_000532bc
00053708: lea (0xff1100).l,A1 ; data 00ff1100
0005370E: bsr.w 0x000537fa ; call 000537FA FUN_000537fa
00053712: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00053718: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0005371E: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00053724: btst.b #0xa,(0x00ffdd22).l
0005372C: beq.w 0x0005375e
00053730: subq.l #0x1,A1
00053732: cmpa.l #0xff1100,A1
00053738: bpl.w 0x000537c6
0005373C: lea (0xff1107).l,A1 ; data 00ff1107
00053742: moveq #0x6,D7
00053744: move.w (0x00fff118).l,D0w
0005374A: mulu.w #0x14,D0
0005374E: tst.b (0x7,A1,D0w*0x1) ; data 00ff110e
00053752: bpl.w 0x000537c6
00053756: subq.l #0x1,A1
00053758: dbf D7w,0x0005374e
0005375C: bra.b 0x00053712
0005375E: btst.b #0xe,(0x00ffdd22).l
00053766: bne.w 0x00053782
0005376A: btst.b #0xc,(0x00ffdd22).l
00053772: bne.w 0x00053782
00053776: btst.b #0xd,(0x00ffdd22).l
0005377E: beq.w 0x000537b8
00053782: move.w (0x00fff118).l,(0x00fff11a).l
0005378C: move.w (0x00fff118).l,D0w
00053792: mulu.w #0x14,D0
00053796: move.b (0x7,A1,D0w*0x1),D0b
0005379A: ext.w D0w
0005379C: move.w D0w,(0x00fff118).l
000537A2: tst.b (0x00fff0a9).l
000537A8: bne.w 0x000537ae
000537AC: move.l (SP)+,D0
000537AE: jsr 0x00017708.l ; call 00017708 FUN_00017708
000537B4: moveq #-0x1,D0
000537B6: rts
000537B8: btst.b #0xb,(0x00ffdd22).l
000537C0: beq.w 0x000537ce
000537C4: addq.l #0x1,A1
000537C6: bsr.w 0x000537fa ; call 000537FA FUN_000537fa
000537CA: bra.w 0x00053712
000537CE: btst.b #0xf,(0x00ffdd22).l
000537D6: beq.w 0x00053712
000537DA: tst.b (0x00fff0a9).l
000537E0: bne.w 0x00053712
000537E4: move.w #-0x7ecc,(0x00c00004).l
000537EC: jsr 0x000177cc.l ; call 000177CC FUN_000177cc
000537F2: jsr 0x000176ea.l ; call 000176EA FUN_000176ea
000537F8: rts
000537FA: move.l A1,-(SP) ; data Stack[-0x4]
000537FC: bsr.w 0x00053248 ; call 00053248 FUN_00053248
00053800: movea.l (SP)+,A1
00053802: lea (0x15fc04).l,A2 ; data 0015fc04
00053808: clr.l D1
0005380A: clr.l D2
0005380C: move.w (0x00fff118).l,D0w
00053812: mulu.w #0x14,D0
00053816: move.b (0x7,A1,D0w*0x1),D0b ; data 00ff1107
0005381A: bpl.w 0x00053826
0005381E: lea (0xff1100).l,A1 ; data 00ff1100
00053824: bra.b 0x0005380c
00053826: ext.w D0w
00053828: mulu.w #0x14,D0
0005382C: lea (0xff1100).l,A4 ; data 00ff1100
00053832: move.b (0x3,A4,D0w*0x1),D1b ; data 00ff1103
00053836: move.b (0x4,A4,D0w*0x1),D2b ; data 00ff1104
0005383A: mulu.w #0x10,D1
0005383E: mulu.w #0x10,D2
00053842: move.w #0x2000,D6w
00053846: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005384C: subq.w #0x8,A3
0005384E: clr.b (0x3,A3) ; data fffffffb
00053852: move.w #0x1,(0x00ffd980).l
0005385A: bset.b #0x1,(0x00ffef14).l
00053862: rts
00053A24: move.l A1,-(SP) ; data Stack[-0x4]
00053A26: bsr.w 0x00053248 ; call 00053248 FUN_00053248
00053A2A: movea.l (SP)+,A1
00053A2C: lea (0x15fc04).l,A2 ; data 0015fc04
00053A32: clr.l D1
00053A34: clr.l D2
00053A36: move.w (0x00fff11a).l,D0w
00053A3C: mulu.w #0x14,D0
00053A40: lea (0xff1100).l,A4 ; data 00ff1100
00053A46: move.b (0x3,A4,D0w*0x1),D1b ; data 00ff1103
00053A4A: move.b (0x4,A4,D0w*0x1),D2b ; data 00ff1104
00053A4E: mulu.w #0x10,D1
00053A52: mulu.w #0x10,D2
00053A56: move.w #0x2000,D6w
00053A5A: jsr 0x00001378.l ; call 00001378 FUN_00001378
00053A60: subq.w #0x8,A3
00053A62: clr.b (0x3,A3) ; data fffffffb
00053A66: move.w #0x1,(0x00ffd980).l
00053A6E: bset.b #0x1,(0x00ffef14).l
00053A76: rts
00053A78: move.b #0x0,(0x00fff10c).l
00053A80: moveq #-0x1,D0
00053A82: move.l (0x00fffc1e).l,D1
00053A88: addq.l #0x1,D0
00053A8A: cmpi.w #0x16,D0w
00053A8E: bgt.w 0x00053ab4
00053A92: lea (0x15fd5c).l,A1 ; data 0015fd5c
00053A98: move.l D0,(0x00ffdd64).l
00053A9E: lsl.w #0x2,D0w
00053AA0: adda.l D0,A1
00053AA2: movea.l (A1),A1 ; data 0015fd5c
00053AA4: jsr (A1)
00053AA6: tst.w D6w
00053AA8: bmi.w 0x00053ab4
00053AAC: move.l (0x00ffdd64).l,D0
00053AB2: bra.b 0x00053a82
00053AB4: rts
00053B16: btst.l #0x0,D1
00053B1A: beq.w 0x00053b76
00053B1E: btst.l #0x1,D1
00053B22: bne.w 0x00053b76
00053B26: cmpi.b #0x1,(0x00fffbf8).l
00053B2E: bne.w 0x00053b76
00053B32: cmpi.b #0x1,(0x00fffbf9).l
00053B3A: bne.w 0x00053b76
00053B3E: tst.b (0x00fff0d5).l
00053B44: bne.w 0x00053b76
00053B48: lea (0xf49e8).l,A0 ; data 000f49e8
00053B4E: lea (0x121fb8).l,A5 ; data 00121fb8
00053B54: move.b #0x1e,(0x00fff10d).l
00053B5C: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
00053B62: tst.b (0x00fff10d).l
00053B68: bpl.w 0x00053b72
00053B6C: jsr 0x0000cf38.l ; call 0000CF38 FUN_0000cf38
00053B72: move.w #-0x1,D6w
00053B76: rts
00054076: btst.l #0x10,D1
0005407A: beq.w 0x000540d4
0005407E: btst.l #0xc,D1
00054082: bne.w 0x000540d4
00054086: cmpi.b #0x5,(0x00fffbf8).l
0005408E: bne.w 0x000540d4
00054092: cmpi.b #0x6,(0x00fffbf9).l
0005409A: bne.w 0x000540d4
0005409E: tst.b (0x00fff0d5).l
000540A4: bne.w 0x000540d4
000540A8: move.b #0x3,(0x00fff0b4).l
000540B0: move.b #-0x1,(0x00ffe184).l
000540B8: lea (0x110a2a).l,A0 ; data 00110a2a
000540BE: lea (0x121e2c).l,A5 ; data 00121e2c
000540C4: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
000540CA: move.b #-0x1,(0x00fff0cc).l
000540D2: clr.w D6w
000540D4: rts
00054152: btst.l #0x19,D1
00054156: beq.w 0x000541b0
0005415A: btst.l #0xe,D1
0005415E: bne.w 0x000541b0
00054162: cmpi.b #0x3,(0x00fffbf8).l
0005416A: bne.w 0x000541b0
0005416E: cmpi.b #0x2,(0x00fffbf9).l
00054176: bne.w 0x000541b0
0005417A: tst.b (0x00fff0d5).l
00054180: bne.w 0x000541b0
00054184: move.b #0x1,(0x00fff0b4).l
0005418C: move.b #-0x1,(0x00ffe187).l
00054194: lea (0x11b7f4).l,A0 ; data 0011b7f4
0005419A: lea (0x122090).l,A5 ; data 00122090
000541A0: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
000541A6: move.b #-0x1,(0x00fff0cc).l
000541AE: clr.w D6w
000541B0: rts
00054222: btst.l #0xb,D1
00054226: beq.w 0x00054284
0005422A: btst.l #0xf,D1
0005422E: bne.w 0x00054284
00054232: cmpi.b #0x0,(0x00fffbf8).l
0005423A: bne.w 0x00054284
0005423E: cmpi.b #0x3,(0x00fffbf9).l
00054246: bne.w 0x00054284
0005424A: tst.b (0x00fff0d5).l
00054250: bne.w 0x00054284
00054254: lea (0x1111de).l,A0 ; data 001111de
0005425A: lea (0x121fc4).l,A5 ; data 00121fc4
00054260: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
00054266: move.l (0x00fffc30).l,D3
0005426C: bclr.l #0x1a,D3
00054270: bset.l #0x1b,D3
00054274: move.l D3,(0x00fffc30).l
0005427A: jsr 0x0002196c.l ; call 0002196C FUN_0002196c
00054280: move.w #-0x1,D6w
00054284: rts
00054300: bclr.l #0x0,D3
00054304: move.l D3,(0x00fffc38).l
0005430A: jsr 0x0005d1b4.l ; call 0005D1B4 FUN_0005d1b4
00054310: jsr 0x0001f1e0.l ; call 0001F1E0 FUN_0001f1e0
00054316: jsr 0x0001f1e0.l ; call 0001F1E0 FUN_0001f1e0
0005431C: move.w #-0x1,D6w
00054320: rts
00054322: cmpi.b #0x6,(0x00fffbf8).l
0005432A: bne.w 0x000543a0
0005432E: cmpi.b #0x1,(0x00fffbf9).l
00054336: bne.w 0x000543a0
0005433A: lea (0x121f04).l,A5 ; data 00121f04
00054340: btst.l #0x11,D1
00054344: beq.w 0x00054390
00054348: btst.l #0x12,D1
0005434C: bne.w 0x00054390
00054350: lea (0xf4db8).l,A0 ; data 000f4db8
00054356: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0005435C: jsr 0x0002196c.l ; call 0002196C FUN_0002196c
00054362: move.l (0x00fffc22).l,D0
00054368: bclr.l #0x6,D0
0005436C: bset.l #0x5,D0
00054370: move.l D0,(0x00fffc22).l
00054376: move.l (0x00fffc30).l,D1
0005437C: bclr.l #0x1d,D1
00054380: bset.l #0x1e,D1
00054384: move.l D1,(0x00fffc30).l
0005438A: move.w #-0x1,D6w
0005438E: rts
00054390: lea (0xf4dcc).l,A0 ; data 000f4dcc
00054396: jsr 0x0001f246.l ; call 0001F246 FUN_0001f246
0005439C: move.w #-0x1,D6w
000543A0: rts
00054662: move.b #-0x1,(0x00fff0b7).l
0005466A: clr.b (0x00fff0d4).l
00054670: move.w #0x4,D4w
00054674: lsl.w #0x4,D4w
00054676: lea (0xfffb30).l,A5 ; data 00fffb30
0005467C: move.w (0x00fff102).l,D3w
00054682: andi.b #0xf,(0x0,A5,D3w*0x1) ; data 00fffb30
00054688: or.b D4b,(0x0,A5,D3w*0x1) ; data 00fffb30
0005468C: clr.l D1
0005468E: move.w (0x00fff118).l,D1w
00054694: mulu.w #0x14,D1
00054698: lea (0xff1100).l,A1 ; data 00ff1100
0005469E: move.b (0x5,A1,D1w*0x1),D0b ; data 00ff1105
000546A2: or.b D0b,(0x0,A5,D3w*0x1) ; data 00fffb30
000546A6: cmpi.b #0x1,(0x0,A1,D1w*0x1) ; data 00ff1100
000546AC: bne.w 0x000546b6
000546B0: jsr 0x000532a8.l ; call 000532A8 FUN_000532a8
000546B6: move.b #-0x1,(0x00fff0a6).l
000546BE: clr.b (0x00fff0a8).l
000546C4: move.w #-0x7500,(0x00c00004).l
000546CC: move.w #-0x7ecc,(0x00c00004).l
000546D4: jsr 0x00010952.l ; call 00010952 FUN_00010952
000546DA: lea (0x166330).l,A5 ; data 00166330
000546E0: lea (0xff1100).l,A6 ; data 00ff1100
000546E6: move.w (0x00fff118).l,D0w
000546EC: mulu.w #0x14,D0
000546F0: clr.l D2
000546F2: move.b (0x0,A6,D0w*0x1),D2b ; data 00ff1100
000546F6: cmpi.w #0x2,D2w
000546FA: bmi.w 0x00054700
000546FE: subq.w #0x1,D2w
00054700: mulu.w #0x38,D2
00054704: adda.l D2,A5
00054706: clr.l D2
00054708: move.b (0x5,A6,D0w*0x1),D2b ; data 00ff1105
0005470C: subq.w #0x1,D2w
0005470E: lsl.w #0x2,D2w
00054710: lea (0x166448).l,A0 ; data 00166448
00054716: adda.l D2,A0
00054718: movea.l (A0),A0 ; data 00166448
0005471A: jsr 0x000566f6.l ; call 000566F6 FUN_000566f6
00054720: move.l #0x15ff30,D0
00054726: move.l #0x1c80,D1
0005472C: move.l #0x5a0,D2
00054732: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00054738: move.l (A5)+,D0 ; data 00166330
0005473A: move.l #0x2220,D1
00054740: move.l (A5)+,D2 ; data 00166334
00054742: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00054748: clr.l D1
0005474A: clr.l D7
0005474C: move.w #0x5,D6w
00054750: movea.l (A5)+,A1 ; data 00166338
00054752: move.w (A5)+,D4w ; data 0016633c
00054754: move.b (A5)+,D7b ; data 0016633e
00054756: move.b (A5)+,D1b ; data 0016633f
00054758: move.w #0x6000,D2w
0005475C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00054762: dbf D6w,0x00054750
00054766: lea (0x1604d0).l,A1 ; data 001604d0
0005476C: move.w #-0x3df2,D4w
00054770: move.w #0x19,D7w
00054774: move.w #0x1,D1w
00054778: move.w #0x6000,D2w
0005477C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00054782: lea (0x160538).l,A1 ; data 00160538
00054788: move.w #-0x3cf0,D4w
0005478C: move.w #0x0,D7w
00054790: move.w #0xf,D1w
00054794: jsr 0x00001232.l ; call 00001232 FUN_00001232
0005479A: lea (0x160558).l,A1 ; data 00160558
000547A0: move.w #-0x3cc2,D4w
000547A4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000547AA: lea (0x160578).l,A1 ; data 00160578
000547B0: move.w #-0x34f0,D4w
000547B4: move.w #0x17,D7w
000547B8: move.w #0x1,D1w
000547BC: jsr 0x00001232.l ; call 00001232 FUN_00001232
000547C2: bsr.w 0x000549f6 ; call 000549F6 FUN_000549f6
000547C6: jsr 0x00017f80.l ; call 00017F80 FUN_00017f80
000547CC: move.w #-0x7e8c,(0x00c00004).l
000547D4: bsr.w 0x00054a8e ; call 00054A8E FUN_00054a8e
000547D8: tst.b (0x00fff0b1).l
000547DE: bmi.w 0x00054bcc
000547E2: lea (0xff1100).l,A1 ; data 00ff1100
000547E8: move.w (0x00fff118).l,D0w
000547EE: mulu.w #0x14,D0
000547F2: clr.l D5
000547F4: move.b (0x0,A1,D0w*0x1),D5b ; data 00ff1100
000547F8: mulu.w #0xa,D5
000547FC: lea (0x1664d8).l,A0 ; data 001664d8
00054802: movea.l (0x0,A0,D5w*0x1),A1 ; data 001664d8
00054806: movea.l #0xc418,A2
0005480C: move.w #0x0,D1w
00054810: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054816: lea (0x166696).l,A1 ; data 00166696
0005481C: movea.l #0xc596,A2
00054822: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054828: bsr.w 0x00054a28 ; call 00054A28 FUN_00054a28
0005482C: clr.b (0x00fff0dc).l
00054832: clr.w D2w
00054834: bra.w 0x0005488c
00054838: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005483E: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00054844: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005484A: btst.b #0x8,(0x00ffdd22).l
00054852: beq.w 0x0005485c
00054856: moveq #-0x1,D2
00054858: bra.w 0x0005486a
0005485C: btst.b #0x9,(0x00ffdd22).l
00054864: beq.w 0x000548a0
00054868: moveq #0x1,D2
0005486A: lea (0x1664d8).l,A0 ; data 001664d8
00054870: clr.l D0
00054872: lea (0xff1100).l,A1 ; data 00ff1100
00054878: move.w (0x00fff118).l,D0w
0005487E: mulu.w #0x14,D0
00054882: clr.l D5
00054884: move.b (0x0,A1,D0w*0x1),D5b ; data 00ff1100
00054888: mulu.w #0xa,D5
0005488C: move.w (0x4,A0,D5w*0x1),D3w ; data 001664dc
00054890: move.w #-0x3968,D7w
00054894: move.w #0x0,D4w
00054898: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
0005489E: bra.b 0x00054838
000548A0: btst.b #0xe,(0x00ffdd22).l
000548A8: beq.w 0x0005496c
000548AC: clr.l D1
000548AE: bsr.w 0x00055f44 ; call 00055F44 FUN_00055f44
000548B2: movea.l (0x00fffc04).l,A0
000548B8: clr.l D7
000548BA: move.b (0x88,A0),D7b
000548BE: cmp.w D6w,D7w
000548C0: bpl.w 0x000548c6
000548C4: move.w D7w,D6w
000548C6: add.w D7w,D6w
000548C8: bne.w 0x000548d0
000548CC: move.w #0x1,D6w
000548D0: clr.l D4
000548D2: lea (0xff1100).l,A1 ; data 00ff1100
000548D8: move.w (0x00fff118).l,D1w
000548DE: mulu.w #0x14,D1
000548E2: move.b (0x5,A1,D1w*0x1),D4b ; data 00ff1105
000548E6: subq.b #0x1,D4b
000548E8: sub.b D4b,D6b
000548EA: ble.w 0x00054902
000548EE: move.b (0x6,A1,D1w*0x1),D4b ; data 00ff1106
000548F2: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000548F8: tst.b (0x00fff0dd).l
000548FE: bgt.w 0x0005492e
00054902: jsr 0x000549e4.l ; call 000549E4 FUN_000549e4
00054908: lea (0x166aa5).l,A1 ; data 00166aa5
0005490E: movea.l #0xc598,A2
00054914: move.w #0x0,D1w
00054918: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005491E: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
00054924: addq.b #0x1,(0x00fff0a8).l
0005492A: bra.w 0x000546cc
0005492E: lea (0x1664d8).l,A0 ; data 001664d8
00054934: clr.l D0
00054936: lea (0xff1100).l,A1 ; data 00ff1100
0005493C: move.w (0x00fff118).l,D0w
00054942: mulu.w #0x14,D0
00054946: move.b (0x0,A1,D0w*0x1),D0b ; data 00ff1100
0005494A: ext.w D0w
0005494C: mulu.w #0xa,D0
00054950: movea.l (0x6,A0,D0w*0x1),A0 ; data 001664de
00054954: move.b (0x00fff0dc).l,D0b
0005495A: lsl.w #0x2,D0w
0005495C: movea.l (0x0,A0,D0w*0x1),A0
00054960: jsr (A0)
00054962: addq.b #0x1,(0x00fff0a8).l
00054968: bra.w 0x000546cc
0005496C: btst.b #0xc,(0x00ffdd22).l
00054974: beq.w 0x0005497c
00054978: bra.w 0x000548ac
0005497C: btst.b #0xd,(0x00ffdd22).l
00054984: beq.w 0x0005498c
00054988: bra.w 0x000548ac
0005498C: btst.b #0xf,(0x00ffdd22).l
00054994: beq.w 0x00054838
00054998: move.w #-0x7ecc,(0x00c00004).l
000549A0: clr.b (0x00fff0b7).l
000549A6: jsr 0x000176ea.l ; call 000176EA FUN_000176ea
000549AC: jsr 0x000177cc.l ; call 000177CC FUN_000177cc
000549B2: move.w #-0x7e8c,(0x00c00004).l
000549BA: jsr 0x0001822c.l ; call 0001822C FUN_0001822c
000549C0: lea (0xfff0ed).l,A4 ; data 00fff0ed
000549C6: lea (0xfff0e8).l,A5 ; data 00fff0e8
000549CC: move.w #0x4,D6w
000549D0: move.b #0x6,(0x0,A4,D6w*0x1)
000549D6: move.b #0x1,(0x0,A5,D6w*0x1)
000549DC: dbf D6w,0x000549d0
000549E0: bra.w 0x000546cc
000549E4: move.l #0xc312,D4
000549EA: moveq #0xf,D1
000549EC: moveq #0x15,D7
000549EE: jsr 0x00012c5a.l ; call 00012C5A FUN_00012c5a
000549F4: rts
000549F6: lea (0xffd8fc).l,A1 ; data 00ffd8fc
000549FC: tst.b (0x00fff0a0).l
00054A02: beq.w 0x00054a12
00054A06: bgt.w 0x00054a1a
00054A0A: move.w #0xc0,(A1)
00054A0E: bra.w 0x00054a1e
00054A12: move.w #0xcc,(A1)
00054A16: bra.w 0x00054a1e
00054A1A: move.w #0xc,(A1)
00054A1E: bset.b #0x0,(0x00ffef14).l
00054A26: rts
00054A28: clr.l D2
00054A2A: lea (0xff1100).l,A6 ; data 00ff1100
00054A30: move.w (0x00fff118).l,D0w
00054A36: mulu.w #0x14,D0
00054A3A: move.l #0x8000,D1
00054A40: lea (0xe30b6).l,A1 ; data 000e30b6
00054A46: movea.l #0xc51e,A2
00054A4C: move.b (0x5,A6,D0w*0x1),D2b ; data 00ff1105
00054A50: lsl.w #0x3,D2w
00054A52: adda.l D2,A1
00054A54: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054A5A: lea (0xe4060).l,A1 ; data 000e4060
00054A60: clr.l D2
00054A62: move.b (0x1,A6,D0w*0x1),D2b ; data 00ff1101
00054A66: lsl.w #0x4,D2w
00054A68: adda.l D2,A1
00054A6A: movea.l #0xc39a,A2
00054A70: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054A76: clr.l D1
00054A78: move.b (0x6,A6,D0w*0x1),D1b ; data 00ff1106
00054A7C: movea.l #0xc52c,A2
00054A82: move.w #-0x8000,D2w
00054A86: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00054A8C: rts
00054A8E: moveq #0xa,D1
00054A90: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00054A96: cmp.b (0x00fff0a8).l,D0b
00054A9C: blt.w 0x00054aa2
00054AA0: rts
00054AA2: clr.b (0x00fff0a8).l
00054AA8: moveq #0x1,D1
00054AAA: jsr 0x0001989a.l ; call 0001989A FUN_0001989a
00054AB0: tst.b (0x00fff0b1).l
00054AB6: bne.w 0x00054ae4
00054ABA: bsr.w 0x000549f6 ; call 000549F6 FUN_000549f6
00054ABE: move.w #-0x8000,D1w
00054AC2: movea.l #0xc496,A2
00054AC8: lea (0x1669e8).l,A1 ; data 001669e8
00054ACE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054AD4: move.w #0x19,D5w
00054AD8: jsr 0x00000c24.l ; call 00000C24 FUN_00000c24
00054ADE: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
00054AE4: jmp 0x000549e4.l ; call 000549E4 FUN_000549e4
00054AEA: jsr 0x000549e4.l ; call 000549E4 FUN_000549e4
00054AF0: movea.l #0xc518,A2
00054AF6: move.w #0x0,D1w
00054AFA: tst.b (0x00fff0a0).l
00054B00: bmi.w 0x00054b22
00054B04: move.b #-0x1,(0x00fff0a0).l
00054B0C: lea (0x1666a8).l,A1 ; data 001666a8
00054B12: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054B18: bsr.w 0x000549f6 ; call 000549F6 FUN_000549f6
00054B1C: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00054B22: lea (0x1666dd).l,A1 ; data 001666dd
00054B28: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00054B2E: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00054BCC: move.l (SP)+,D0
00054BCE: clr.l D0
00054BD0: rts
00055000: beq.w 0x00055020
00055004: clr.b (0x00fff0ae).l
0005500A: jsr 0x000549e4.l ; call 000549E4 FUN_000549e4
00055010: lea (0x1667dd).l,A1 ; data 001667dd
00055016: movea.l #0xc59a,A2
0005501C: bra.w 0x0005502c
00055020: lea (0x1667f8).l,A1 ; data 001667f8
00055026: movea.l #0xc89a,A2
0005502C: move.w #0x0,D1w
00055030: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00055036: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00055640: move.w D0w,-(SP) ; data Stack[-0x2]
00055642: bsr.w 0x00055ed6 ; call 00055ED6 FUN_00055ed6
00055646: clr.w D7w
00055648: bsr.w 0x00055f20 ; call 00055F20 FUN_00055f20
0005564C: jsr 0x00012c74.l ; call 00012C74 FUN_00012c74
00055652: tst.b (0xb4,A0)
00055656: beq.w 0x00055680
0005565A: cmpi.b #0x2,(0xb4,A0)
00055660: bmi.w 0x00055670
00055664: beq.w 0x00055678
00055668: move.w #0x8,D4w
0005566C: bra.w 0x00055684
00055670: move.w #0x5,D4w
00055674: bra.w 0x00055684
00055678: move.w #0x6,D4w
0005567C: bra.w 0x00055684
00055680: move.w #0x4,D4w
00055684: bsr.w 0x00055740 ; call 00055740 FUN_00055740
00055688: bsr.w 0x00055786 ; call 00055786 FUN_00055786
0005568C: jsr 0x000556c6.l ; call 000556C6 FUN_000556c6
00055692: jsr 0x00000d5a.l ; call 00000D5A FUN_00000d5a
00055698: cmpi.w #0x2,D4w
0005569C: bpl.w 0x000556a4
000556A0: move.w #0x2,D4w
000556A4: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000556AA: lea (0x9f).w,A4
000556AE: adda.l A0,A4
000556B0: move.b (0x00fff0dd).l,(A4)
000556B6: lea (0x9f).w,A4
000556BA: adda.l A1,A4
000556BC: move.b (0x00fff0dd).l,(A4)
000556C2: move.w (SP)+,D0w
000556C4: rts
000556C6: clr.l D0
000556C8: move.b (0x57,A0),D0b
000556CC: cmpi.b #0x8,D0b
000556D0: bmi.w 0x000556fc
000556D4: cmpi.b #0xb,D0b
000556D8: bpl.w 0x000556fc
000556DC: addq.w #0x3,D4w
000556DE: subq.w #0x1,D0w
000556E0: btst.b #0x4,(0x6e,A0,D0w*0x1)
000556E6: beq.w 0x000556f0
000556EA: subq.w #0x2,D4w
000556EC: bra.w 0x000556fc
000556F0: btst.b #0x5,(0x6e,A0,D0w*0x1)
000556F6: beq.w 0x000556fc
000556FA: subq.w #0x3,D4w
000556FC: move.b (0x57,A0),D0b
00055700: subq.w #0x1,D0w
00055702: btst.b #0x3,(0x6e,A0,D0w*0x1)
00055708: beq.w 0x0005570e
0005570C: subq.w #0x1,D4w
0005570E: btst.b #0x0,(0x6e,A0,D0w*0x1)
00055714: beq.w 0x0005573e
00055718: move.w (0x90,A0),D7w
0005571C: btst.l #0x4,D7
00055720: beq.w 0x00055728
00055724: subq.w #0x2,D4w
00055726: rts
00055728: move.w #0x7,D7w
0005572C: cmpi.b #0x16,(0x66,A0,D7w*0x1)
00055732: bne.w 0x0005573a
00055736: subq.w #0x1,D4w
00055738: rts
0005573A: dbf D7w,0x0005572c
0005573E: rts
00055740: move.w (0x90,A0),D7w
00055744: btst.l #0x1,D7
00055748: beq.w 0x0005574e
0005574C: subq.w #0x1,D4w
0005574E: cmpi.b #0x16,(0x9b,A1)
00055754: beq.w 0x00055782
00055758: btst.l #0x1,D7
0005575C: bne.w 0x00055784
00055760: cmpi.b #0x4,(0x9a,A0)
00055766: beq.w 0x00055784
0005576A: cmpi.b #0x2,(0x9a,A0)
00055770: beq.w 0x00055784
00055774: btst.b #0x7,(0x11,A1)
0005577A: beq.w 0x00055784
0005577E: addq.w #0x4,D4w
00055780: rts
00055782: addq.w #0x2,D4w
00055784: rts
00055786: btst.b #0x0,(0xdf,A0)
0005578C: beq.w 0x00055804
00055790: movem.l {  D6 D4},-(SP)
00055794: bsr.w 0x00055828 ; call 00055828 FUN_00055828
00055798: clr.l D0
0005579A: clr.l D4
0005579C: move.b (0x1e,A5),D0b
000557A0: move.b (0x1f,A5),D4b
000557A4: moveq #0x5,D7
000557A6: clr.l D2
000557A8: lea (0xd4ea0).l,A4 ; data 000d4ea0
000557AE: mulu.w #0x30,D7
000557B2: clr.l D6
000557B4: cmpi.w #0x3,(0x3e,A0)
000557BA: bpl.w 0x000557c4
000557BE: jsr 0x0001047e.l ; call 0001047E FUN_0001047e
000557C4: tst.b (0x27,A4,D7w*0x1)
000557C8: bmi.w 0x000557d4
000557CC: add.b (0x77,A0),D6b
000557D0: bra.w 0x000557d8
000557D4: add.b (0x7c,A0),D6b
000557D8: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000557DE: sub.b (0x00fff0dd).l,D0b
000557E4: bgt.w 0x000557ee
000557E8: clr.l D0
000557EA: bra.w 0x000557fa
000557EE: btst.b #0x1,(0xdf,A0)
000557F4: beq.w 0x000557fa
000557F8: lsr.w #0x1,D0w
000557FA: movem.l (SP)+,{  D4 D6}
000557FE: add.w D0w,D4w
00055800: bra.w 0x00055826
00055804: btst.b #0x1,(0xdf,A0)
0005580A: beq.w 0x00055826
0005580E: movem.l {  D6 D4},-(SP)
00055812: bsr.w 0x00055828 ; call 00055828 FUN_00055828
00055816: clr.l D0
00055818: clr.l D4
0005581A: move.b (0x20,A5),D0b
0005581E: move.b (0x21,A5),D4b
00055822: moveq #0xc,D7
00055824: bra.b 0x000557a6
00055826: rts
00055828: clr.l D3
0005582A: lea (0xd8).w,A5
0005582E: adda.l A0,A5
00055830: movea.l (A5),A5
00055832: adda.l #0xa0,A5
00055838: move.w (A5),D3w
0005583A: mulu.w #0x22,D3
0005583E: lea (0xfff9fa).l,A5 ; data 00fff9fa
00055844: adda.l D3,A5
00055846: rts
00055848: bsr.w 0x00055ed6 ; call 00055ED6 FUN_00055ed6
0005584C: clr.w D7w
0005584E: bsr.w 0x00055f20 ; call 00055F20 FUN_00055f20
00055852: move.b (0x87,A0),D7b
00055856: cmp.w D6w,D7w
00055858: bpl.w 0x0005585e
0005585C: move.w D7w,D6w
0005585E: add.w D7w,D6w
00055860: bne.w 0x00055868
00055864: move.w #0x1,D6w
00055868: tst.b (0x87,A0)
0005586C: bgt.w 0x00055876
00055870: moveq #0x8,D4
00055872: bra.w 0x00055878
00055876: moveq #0x5,D4
00055878: bsr.w 0x00055740 ; call 00055740 FUN_00055740
0005587C: bsr.w 0x00055786 ; call 00055786 FUN_00055786
00055880: cmpi.w #0x2,D4w
00055884: bpl.w 0x0005588c
00055888: move.w #0x2,D4w
0005588C: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00055892: clr.l D4
00055894: clr.l D5
00055896: clr.l D6
00055898: move.b (0x00fff0dd).l,D4b
0005589E: lsr.w #0x1,D4w
000558A0: moveq #0x6,D1
000558A2: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000558A8: addq.w #0x1,D0w
000558AA: sub.w D4w,D0w
000558AC: ble.w 0x000558fe
000558B0: move.l D0,D4
000558B2: mulu.w #0xc,D4
000558B6: moveq #0x3,D1
000558B8: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000558BE: subq.w #0x1,D0w
000558C0: bmi.w 0x000558ce
000558C4: beq.w 0x000558d0
000558C8: move.w D4w,D5w
000558CA: bra.w 0x000558d0
000558CE: sub.w D4w,D5w
000558D0: moveq #0x3,D1
000558D2: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000558D8: subq.w #0x1,D0w
000558DA: bmi.w 0x000558fc
000558DE: bgt.w 0x000558f6
000558E2: tst.w D5w
000558E4: bne.w 0x000558fe
000558E8: moveq #0x2,D1
000558EA: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
000558F0: tst.w D0w
000558F2: beq.w 0x000558fc
000558F6: move.w D4w,D6w
000558F8: bra.w 0x000558fe
000558FC: sub.w D4w,D6w
000558FE: rts
00055900: cmpi.b #0xf,(0x57,A0)
00055906: bne.w 0x0005591e
0005590A: move.w #0x3,D5w
0005590E: move.w #0xa,D4w
00055912: move.b #0x1,(0x00fff0cc).l
0005591A: bra.w 0x0005593c
0005591E: cmpi.b #0xe,(0x57,A0)
00055924: bne.w 0x00055934
00055928: move.w #0x3,D5w
0005592C: move.w #0x6,D4w
00055930: bra.w 0x0005593c
00055934: move.w #0x6,D5w
00055938: move.w #0x8,D4w
0005593C: move.w #-0x1,(0x00fff112).l
00055944: bra.w 0x00055980
00055948: clr.b (0x00fff0cc).l
0005594E: clr.l D3
00055950: clr.l D4
00055952: clr.l D6
00055954: move.b (0x57,A0),D6b
00055958: subq.w #0x1,D6w
0005595A: mulu.w #0x30,D6
0005595E: lea (0xd4540).l,A4 ; data 000d4540
00055964: move.b (0x25,A4,D6w*0x1),D3b ; data 000d4565
00055968: move.b (0x26,A4,D6w*0x1),D4b ; data 000d4566
0005596C: lea (0x166ad4).l,A2 ; data 00166ad4
00055972: clr.l D5
00055974: move.b (0x0,A2,D3w*0x1),D5b ; data 00166ad4
00055978: neg.b D5b
0005597A: clr.w (0x00fff112).l
00055980: clr.l D1
00055982: exg A0,A1
00055984: bsr.w 0x00055ed6 ; call 00055ED6 FUN_00055ed6
00055988: move.w #-0x1,D7w
0005598C: bsr.w 0x00055f20 ; call 00055F20 FUN_00055f20
00055990: bsr.w 0x00055786 ; call 00055786 FUN_00055786
00055994: exg A0,A1
00055996: add.b (0x77,A1),D6b
0005599A: movea.l A1,A5
0005599C: adda.l #0x90,A5
000559A2: move.w (A5),D0w
000559A4: moveq #0x2,D7
000559A6: moveq #0x9,D1
000559A8: btst.l D1,D0
000559AA: beq.w 0x000559b6
000559AE: subq.w #0x8,D1w
000559B0: add.w D1w,D6w
000559B2: bra.w 0x000559bc ; call 000559BC FUN_000559bc
000559B6: addq.w #0x1,D1w
000559B8: dbf D7w,0x000559a8
000559BC: cmpi.w #0x2,D4w
000559C0: bpl.w 0x000559c8
000559C4: move.w #0x2,D4w
000559C8: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000559CE: clr.l D6
000559D0: move.b (0x9f,A0),D6b
000559D4: sub.b (0x00fff0dd).l,D6b
000559DA: bpl.w 0x000559e6
000559DE: add.b D6b,D5b
000559E0: bgt.w 0x000559f4
000559E4: rts
000559E6: add.w D6w,D5w
000559E8: cmpi.w #0xb,D5w
000559EC: bmi.w 0x000559f4
000559F0: move.w #0xa,D5w
000559F4: tst.w (0x00fff112).l
000559FA: beq.w 0x00055a0e
000559FE: bgt.w 0x00055a16
00055A02: bsr.w 0x00055eb4 ; call 00055EB4 FUN_00055eb4
00055A06: ble.w 0x00055b14
00055A0A: bra.w 0x00055a16
00055A0E: bsr.w 0x00055e92 ; call 00055E92 FUN_00055e92
00055A12: ble.w 0x00055b14
00055A16: bsr.w 0x00055e62 ; call 00055E62 FUN_00055e62
00055A1A: ble.w 0x00055b14
00055A1E: move.w D5w,D1w
00055A20: neg.w D1w
00055A22: move.w (0x3e,A1),D6w
00055A26: cmpi.w #0x3,D6w
00055A2A: bpl.w 0x00055a62
00055A2E: cmpi.b #0x3,(0x00ffe186).l
00055A36: beq.w 0x00055b14
00055A3A: tst.b (0x00fff0cc).l
00055A40: beq.w 0x00055a56
00055A44: clr.b (0x00fff0cc).l
00055A4A: exg A0,A1
00055A4C: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
00055A52: exg A0,A1
00055A54: rts
00055A56: exg A0,A1
00055A58: jsr 0x00003674.l ; call 00003674 FUN_00003674
00055A5E: exg A0,A1
00055A60: rts
00055A62: tst.b (0x9a,A1)
00055A66: bpl.w 0x00055a6e
00055A6A: move.w #-0x1,D1w
00055A6E: tst.b (0x00fff0cc).l
00055A74: beq.w 0x00055a90
00055A78: clr.b (0x00fff0cc).l
00055A7E: add.b D1b,(0x9c,A1)
00055A82: bgt.w 0x00055b14
00055A86: move.b #0x0,(0x9c,A1)
00055A8C: bra.w 0x00055a9e
00055A90: add.b D1b,(0x4c,A1)
00055A94: bgt.w 0x00055b14
00055A98: move.b #0x0,(0x4c,A1)
00055A9E: tst.b (0x00fffbed).l
00055AA4: bne.w 0x00055acc
00055AA8: cmpi.b #0x42,(0x9b,A1)
00055AAE: bne.w 0x00055acc
00055AB2: addq.w #0x1,(0x00fffbf0).l
00055AB8: cmpi.w #0x15,(0x00fffbf0).l
00055AC0: bmi.w 0x00055acc
00055AC4: move.w #0x14,(0x00fffbf0).l
00055ACC: subq.b #0x1,(0x00fff0ce).l
00055AD2: bpl.w 0x00055af0
00055AD6: move.b #0xf,(0x00fff0ce).l
00055ADE: addq.b #0x1,(0x00ff018d).l
00055AE4: addq.b #0x1,(0x00ff028d).l
00055AEA: addq.b #0x1,(0x00ff038d).l
00055AF0: cmpi.b #0x13,(0x9b,A1)
00055AF6: bne.w 0x00055b14
00055AFA: addq.b #0x1,(0x00fffbfa).l
00055B00: cmpi.b #0x9,(0x00fffbfa).l
00055B08: bmi.w 0x00055b14
00055B0C: move.b #0x8,(0x00fffbfa).l
00055B14: rts
00055B16: move.b #0x6,D6b
00055B1A: moveq #0x4,D4
00055B1C: jsr 0x00055740.l ; call 00055740 FUN_00055740
00055B22: jsr 0x00055786.l ; call 00055786 FUN_00055786
00055B28: jsr 0x00000d5a.l ; call 00000D5A FUN_00000d5a
00055B2E: cmpi.w #0x2,D4w
00055B32: bpl.w 0x00055b3a
00055B36: move.w #0x2,D4w
00055B3A: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00055B40: move.b (0x00fff0dd).l,(0x9f,A0)
00055B48: move.b #0x6,(0x9d,A0)
00055B4E: move.w #0x2,D5w
00055B52: clr.l D6
00055B54: cmpi.w #0x3,(0x3e,A1)
00055B5A: bpl.w 0x00055b64
00055B5E: jsr 0x0001047e.l ; call 0001047E FUN_0001047e
00055B64: add.b (0x77,A1),D6b
00055B68: move.b #0x6,D4b
00055B6C: move.w #0x1,(0x00fff112).l
00055B74: bra.w 0x000559bc ; call 000559BC FUN_000559bc
00055C76: bsr.w 0x00055ed6 ; call 00055ED6 FUN_00055ed6
00055C7A: clr.w D7w
00055C7C: bsr.w 0x00055f20 ; call 00055F20 FUN_00055f20
00055C80: move.b (0x85,A0),D7b
00055C84: cmp.w D6w,D7w
00055C86: bpl.w 0x00055c8c
00055C8A: move.w D7w,D6w
00055C8C: add.w D7w,D6w
00055C8E: bne.w 0x00055c96
00055C92: move.w #0x1,D6w
00055C96: move.w #0x4,D4w
00055C9A: bsr.w 0x00055740 ; call 00055740 FUN_00055740
00055C9E: bsr.w 0x00055786 ; call 00055786 FUN_00055786
00055CA2: cmpi.w #0x2,D4w
00055CA6: bpl.w 0x00055cae
00055CAA: move.w #0x2,D4w
00055CAE: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00055CB4: move.b (0x00fff0dd).l,D5b
00055CBA: beq.w 0x00055e46
00055CBE: addq.w #0x3,D5w
00055CC0: bsr.w 0x00055eb4 ; call 00055EB4 FUN_00055eb4
00055CC4: ble.w 0x00055e46
00055CC8: bsr.w 0x00055e62 ; call 00055E62 FUN_00055e62
00055CCC: ble.w 0x00055e46
00055CD0: exg A0,A1
00055CD2: bsr.w 0x00055ed6 ; call 00055ED6 FUN_00055ed6
00055CD6: clr.w D7w
00055CD8: bsr.w 0x00055f20 ; call 00055F20 FUN_00055f20
00055CDC: move.w #0x4,D4w
00055CE0: bsr.w 0x00055740 ; call 00055740 FUN_00055740
00055CE4: bsr.w 0x00055786 ; call 00055786 FUN_00055786
00055CE8: exg A0,A1
00055CEA: move.b (0x85,A1),D7b
00055CEE: cmp.w D6w,D7w
00055CF0: bpl.w 0x00055cf6
00055CF4: move.w D7w,D6w
00055CF6: tst.b (0x57,A1)
00055CFA: bne.w 0x00055d00
00055CFE: add.w D7w,D6w
00055D00: tst.w D6w
00055D02: bne.w 0x00055d0a
00055D06: move.w #0x1,D6w
00055D0A: cmpi.w #0x2,D4w
00055D0E: bpl.w 0x00055d16
00055D12: move.w #0x2,D4w
00055D16: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00055D1C: sub.b (0x00fff0dd).l,D5b
00055D22: ble.w 0x00055e46
00055D26: move.b (0x79,A0),D4b
00055D2A: movea.l A0,A3
00055D2C: adda.l #0x90,A3
00055D32: move.w (A3),D0w
00055D34: moveq #0x3,D7
00055D36: moveq #0x5,D6
00055D38: btst.l D6,D0
00055D3A: beq.w 0x00055d46
00055D3E: subq.w #0x4,D6w
00055D40: add.w D6w,D4w
00055D42: bra.w 0x00055d4c
00055D46: addq.w #0x1,D6w
00055D48: dbf D7w,0x00055d38
00055D4C: exg A0,A1
00055D4E: bsr.w 0x00055ed6 ; call 00055ED6 FUN_00055ed6
00055D52: move.w #-0x1,D7w
00055D56: bsr.w 0x00055f20 ; call 00055F20 FUN_00055f20
00055D5A: bsr.w 0x00055786 ; call 00055786 FUN_00055786
00055D5E: exg A0,A1
00055D60: add.b (0x77,A1),D6b
00055D64: movea.l A1,A5
00055D66: adda.l #0x90,A5
00055D6C: move.w (A5),D0w
00055D6E: moveq #0x2,D7
00055D70: moveq #0x9,D1
00055D72: btst.l D1,D0
00055D74: beq.w 0x00055d80
00055D78: subq.w #0x8,D1w
00055D7A: add.w D1w,D6w
00055D7C: bra.w 0x00055d86
00055D80: addq.w #0x1,D1w
00055D82: dbf D7w,0x00055d72
00055D86: cmpi.w #0x2,D4w
00055D8A: bpl.w 0x00055d92
00055D8E: move.w #0x2,D4w
00055D92: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00055D98: sub.b (0x00fff0dd).l,D5b
00055D9E: ble.w 0x00055e46
00055DA2: move.w (0x90,A0),D0w
00055DA6: btst.l #0x3,D0
00055DAA: bne.w 0x00055dbc
00055DAE: btst.l #0x2,D0
00055DB2: beq.w 0x00055e22
00055DB6: subq.b #0x2,D5b
00055DB8: ble.w 0x00055e46
00055DBC: move.w D5w,D1w
00055DBE: neg.w D1w
00055DC0: cmpi.w #0x3,(0x3e,A1)
00055DC6: bpl.w 0x00055de2
00055DCA: cmpi.b #0x3,(0x00ffe186).l
00055DD2: beq.w 0x00055e46
00055DD6: exg A0,A1
00055DD8: jsr 0x00003674.l ; call 00003674 FUN_00003674
00055DDE: exg A0,A1
00055DE0: rts
00055DE2: tst.b (0x9a,A1)
00055DE6: bpl.w 0x00055dee
00055DEA: move.w #-0x1,D1w
00055DEE: add.b D1b,(0x4c,A1)
00055DF2: bpl.w 0x00055e46
00055DF6: move.b #0x0,(0x4c,A1)
00055DFC: subq.b #0x1,(0x00fff0ce).l
00055E02: bpl.w 0x00055e46
00055E06: move.b #0xf,(0x00fff0ce).l
00055E0E: addq.b #0x1,(0x00ff018d).l
00055E14: addq.b #0x1,(0x00ff028d).l
00055E1A: addq.b #0x1,(0x00ff038d).l
00055E20: rts
00055E22: move.w D5w,D1w
00055E24: neg.w D1w
00055E26: cmpi.w #0x3,(0x3e,A1)
00055E2C: bpl.w 0x00055e48
00055E30: cmpi.b #0x3,(0x00ffe186).l
00055E38: beq.w 0x00055e46
00055E3C: exg A0,A1
00055E3E: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
00055E44: exg A0,A1
00055E46: rts
00055E48: tst.b (0x9a,A1)
00055E4C: bpl.w 0x00055e54
00055E50: move.w #-0x1,D1w
00055E54: add.b D1b,(0x9c,A1)
00055E58: bpl.b 0x00055dfc
00055E5A: move.b #0x0,(0x9c,A1)
00055E60: bra.b 0x00055dfc
00055E62: btst.b #0x5,(0x11,A0)
00055E68: beq.w 0x00055e70
00055E6C: subq.w #0x2,D5w
00055E6E: rts
00055E70: movea.l A0,A2
00055E72: moveq #0x7,D7
00055E74: cmpi.b #0x27,(0x66,A2)
00055E7A: bne.w 0x00055e88
00055E7E: move.b (0x6e,A2),D7b
00055E82: lsr.w #0x1,D7w
00055E84: sub.w D7w,D5w
00055E86: rts
00055E88: addq.l #0x1,A2
00055E8A: dbf D7w,0x00055e74
00055E8E: tst.w D5w
00055E90: rts
00055E92: clr.l D0
00055E94: clr.l D1
00055E96: move.b (0x76,A1),D1b
00055E9A: beq.w 0x00055eb0
00055E9E: subq.w #0x1,D1w
00055EA0: lea (0xd4d50).l,A4 ; data 000d4d50
00055EA6: mulu.w #0x30,D1
00055EAA: move.b (0x25,A4,D1w*0x1),D0b ; data 000d4d75
00055EAE: sub.b D0b,D5b
00055EB0: tst.b D5b
00055EB2: rts
00055EB4: clr.l D0
00055EB6: clr.l D1
00055EB8: move.b (0x76,A1),D1b
00055EBC: beq.w 0x00055ed2
00055EC0: subq.w #0x1,D1w
00055EC2: lea (0xd4d50).l,A4 ; data 000d4d50
00055EC8: mulu.w #0x30,D1
00055ECC: move.b (0x26,A4,D1w*0x1),D0b ; data 000d4d76
00055ED0: sub.b D0b,D5b
00055ED2: tst.b D5b
00055ED4: rts
00055ED6: clr.l D1
00055ED8: move.b (0x78,A0),D1b
00055EDC: add.b (0x7b,A0),D1b
00055EE0: add.b (0x7c,A0),D1b
00055EE4: lsr.w #0x1,D1w
00055EE6: movea.l A0,A2
00055EE8: moveq #0x7,D7
00055EEA: cmpi.b #0x28,(0x66,A2)
00055EF0: bne.w 0x00055efa
00055EF4: add.b (0x6e,A2),D1b
00055EF8: rts
00055EFA: addq.l #0x1,A2
00055EFC: dbf D7w,0x00055eea
00055F00: rts
00055F02: clr.l D1
00055F04: move.b (0x80,A0),D1b
00055F08: moveq #0x6,D7
00055F0A: cmpi.b #0x18,(0x66,A0,D7w*0x1)
00055F10: bne.w 0x00055f1a
00055F14: add.b (0x6e,A0,D7w*0x1),D1b
00055F18: rts
00055F1A: dbf D7w,0x00055f0a
00055F1E: rts
00055F20: clr.l D6
00055F22: move.l D1,D2
00055F24: clr.l D3
00055F26: move.b (0x4d,A0),D3b
00055F2A: mulu.w #0x19,D3
00055F2E: mulu.w D3w,D2
00055F30: divu.w #0x64,D2
00055F34: sub.w D2w,D1w
00055F36: tst.w D7w
00055F38: bmi.w 0x00055f40
00055F3C: move.w D2w,D6w
00055F3E: rts
00055F40: move.w D1w,D6w
00055F42: rts
00055F44: movem.l {  D7 D5 D4 D3 D2 D1 D0},-(SP)
00055F48: movea.l (0x00fffc04).l,A0
00055F4E: move.l D1,-(SP) ; data Stack[-0x20]
00055F50: jsr 0x0000d81c.l ; call 0000D81C average_entity_delay_attributes
00055F56: move.l D1,D6
00055F58: move.l (SP)+,D1
00055F5A: add.b (0x88,A0),D6b
00055F5E: lea (0xfffbbe).l,A0 ; data 00fffbbe
00055F64: add.b (0x7,A0),D6b
00055F68: add.b (0x7,A0),D6b
00055F6C: tst.b (0x00fff0b7).l
00055F72: bgt.w 0x00055fa8
00055F76: bmi.w 0x00055fc0
00055F7A: lsr.w #0x1,D6w
00055F7C: bcc.w 0x00055f88
00055F80: tst.w D1w
00055F82: beq.w 0x00055f88
00055F86: addq.w #0x1,D6w
00055F88: tst.w D1w
00055F8A: bmi.w 0x00055f92
00055F8E: bra.w 0x00055fa2
00055F92: clr.l D7
00055F94: move.b (0x00fffbbe).l,D7b
00055F9A: cmp.w D6w,D7w
00055F9C: bpl.w 0x00055fa2
00055FA0: move.w D7w,D6w
00055FA2: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D7}
00055FA6: rts
00055FA8: move.l D6,D5
00055FAA: move.l D6,D7
00055FAC: divu.w #0x3,D7
00055FB0: sub.w D7w,D6w
00055FB2: sub.w D6w,D5w
00055FB4: tst.w D1w
00055FB6: beq.w 0x00055fbc
00055FBA: bra.b 0x00055f92
00055FBC: move.l D5,D6
00055FBE: bra.b 0x00055fa2
00055FC0: move.l D6,D5
00055FC2: move.l D6,D7
00055FC4: divu.w #0x3,D7
00055FC8: sub.w D7w,D6w
00055FCA: sub.w D6w,D5w
00055FCC: tst.w D1w
00055FCE: bmi.w 0x00055fd4
00055FD2: bra.b 0x00055fa2
00055FD4: move.l D5,D6
00055FD6: bra.b 0x00055f92
00055FD8: clr.b (0x00fff0dd).l
00055FDE: clr.l D1
00055FE0: bsr.w 0x00055f44 ; call 00055F44 FUN_00055f44
00055FE4: jsr 0x00019c54.l ; call 00019C54 FUN_00019c54
00055FEA: cmp.w D6w,D7w
00055FEC: bpl.w 0x00055ff2
00055FF0: move.w D7w,D6w
00055FF2: add.w D7w,D6w
00055FF4: bne.w 0x00055ffc
00055FF8: move.w #0x1,D6w
00055FFC: clr.l D4
00056050: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
00056056: lea (0xfffbbe).l,A2 ; data 00fffbbe
0005605C: move.b (0xa,A2),D4b
00056060: bra.w 0x00056088
00056078: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
0005607E: lea (0xfffbbe).l,A2 ; data 00fffbbe
00056084: move.b (0x9,A2),D4b
00056088: cmpi.w #0x2,D4w
0005608C: bpl.w 0x00056092
00056090: moveq #0x2,D4
00056092: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00056098: move.b (A4),D4b
0005609A: sub.b (0x00fff0dd).l,D4b
000560A0: move.b D4b,(A4)
000560A2: bpl.w 0x000560a8
000560A6: clr.b (A4)
000560A8: rts
000560AA: tst.b (0x00fff0e6).l
000560B0: bmi.w 0x00056108
000560B4: lea (0xff1100).l,A1 ; data 00ff1100
000560BA: move.w (0x00fff118).l,D1w
000560C0: mulu.w #0x14,D1
000560C4: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
000560CA: move.b (0x9,A5),D4b
000560CE: bsr.w 0x000562d4 ; call 000562D4 FUN_000562d4
000560D2: cmpi.w #0x2,D4w
000560D6: bpl.w 0x000560de
000560DA: move.w #0x2,D4w
000560DE: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000560E4: subq.b #0x1,(0x00fff0dd).l
000560EA: bmi.w 0x00056112
000560EE: moveq #0xa,D0
000560F0: sub.b (0x00fff0dd).l,D0b
000560F6: move.b D0b,(0x00fff0e7).l
000560FC: move.b #-0x1,(0x00fff0e6).l
00056104: moveq #0x3,D0
00056106: rts
00056108: subq.b #0x1,(0x00fff0e7).l
0005610E: bne.w 0x00056112
00056112: moveq #-0x1,D0
00056114: rts
00056116: tst.b (0x00fff0a3).l
0005611C: bmi.w 0x0005613c
00056120: bgt.w 0x000561dc
00056124: cmpi.b #0x1,(0x00fff0a0).l
0005612C: beq.w 0x000561e2
00056130: move.b #-0x1,(0x00fff0a3).l
00056138: moveq #0x2,D0
0005613A: rts
0005613C: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
00056142: lea (0xfffbbe).l,A0 ; data 00fffbbe
00056148: move.b (0xa,A0),D4b
0005614C: bsr.w 0x000562de ; call 000562DE FUN_000562de
00056150: cmpi.w #0x2,D4w
00056154: bpl.w 0x0005615c
00056158: move.w #0x2,D4w
0005615C: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
00056162: move.b (0x00fff0dd).l,D5b
00056168: move.w #-0x1,D1w
0005616C: bsr.w 0x00055f44 ; call 00055F44 FUN_00055f44
00056170: movea.l (0x00fffc04).l,A0
00056176: add.b (0x88,A0),D6b
0005617A: clr.l D4
0005617C: move.w (0x00fff118).l,D1w
00056182: mulu.w #0x14,D1
00056186: move.b (0x6,A1,D1w*0x1),D4b
0005618A: bsr.w 0x000562de ; call 000562DE FUN_000562de
0005618E: cmpi.w #0x2,D4w
00056192: bpl.w 0x0005619a
00056196: move.w #0x2,D4w
0005619A: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000561A0: sub.b (0x00fff0dd).l,D5b
000561A6: ble.w 0x000561bc
000561AA: moveq #0x1,D1
000561AC: jsr 0x0001989a.l ; call 0001989A FUN_0001989a
000561B2: tst.b (0x00fff0b1).l
000561B8: bmi.w 0x000561e2
000561BC: jsr 0x00017a5e.l ; call 00017A5E FUN_00017a5e
000561C2: tst.b (0x00fff0a0).l
000561C8: ble.w 0x000561d6
000561CC: jsr 0x0001a908.l ; call 0001A908 FUN_0001a908
000561D2: bra.w 0x000561dc
000561D6: clr.b (0x00ffdf84).l
000561DC: clr.b (0x00fff0a3).l
000561E2: move.w #-0x1,D0w
000561E6: rts
000561E8: jsr 0x00053216.l ; call 00053216 FUN_00053216
000561EE: lea (0xfffbbe).l,A5 ; data 00fffbbe
000561F4: lea (0xff1100).l,A1 ; data 00ff1100
000561FA: move.w (0x00fff118).l,D1w
00056200: mulu.w #0x14,D1
00056204: clr.l D4
00056206: move.b (0x10,A0,D1w*0x1),D4b
0005620A: cmp.b (0x10,A1,D1w*0x1),D4b ; data 00ff1110
0005620E: beq.w 0x00056216
00056212: addq.b #0x1,(0x10,A1,D1w*0x1) ; data 00ff1110
00056216: clr.l D6
00056218: move.b (0x13,A1,D1w*0x1),D4b ; data 00ff1113
0005621C: cmpi.b #0x2,D4b
00056220: bmi.w 0x00056116
00056224: cmpi.b #0x5,D4b
00056228: bmi.w 0x00056268
0005622C: cmpi.b #0x7,D4b
00056230: bmi.w 0x000560aa
00056234: cmpi.b #0xb,D4b
00056238: bpl.w 0x0005624a
0005623C: tst.b (0x00fff0a0).l
00056242: bgt.w 0x000560aa
00056246: bra.w 0x00056116 ; call 00056116 FUN_00056116
0005624A: cmpi.b #0x11,D4b
0005624E: bpl.w 0x00056260
00056252: tst.b (0x00fff0a0).l
00056258: bgt.w 0x00056268
0005625C: bra.w 0x00056116 ; call 00056116 FUN_00056116
00056260: cmpi.b #0x15,D4b
00056264: bmi.w 0x00056116
00056268: lea (0xff1100).l,A1 ; data 00ff1100
0005626E: move.w (0x00fff118).l,D1w
00056274: mulu.w #0x14,D1
00056278: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
0005627E: clr.l D4
00056280: move.b (0x8,A5),D4b
00056284: bsr.w 0x000562d4 ; call 000562D4 FUN_000562d4
00056288: cmpi.w #0x2,D4w
0005628C: bpl.w 0x00056294
00056290: move.w #0x2,D4w
00056294: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
0005629A: jsr 0x00019b40.l ; call 00019B40 FUN_00019b40
000562A0: cmpi.w #0x8,D0w
000562A4: bne.w 0x000562b6
000562A8: jsr 0x00017d72.l ; call 00017D72 FUN_00017d72
000562AE: clr.l D1
000562B0: jsr 0x00017d90.l ; call 00017D90 FUN_00017d90
000562B6: clr.l D0
000562B8: move.b (0x00fff0dd).l,(0x00fff0a4).l
000562C2: bgt.w 0x000562d2
000562C6: move.w #-0x1,D0w
000562CA: move.b #0x1,(0x00fff0a4).l
000562D2: rts
000562D4: clr.l D7
000562D6: move.b (0x00ffe169).l,D7b
000562DC: add.w D7w,D4w
000562DE: clr.l D7
000562E0: move.b (0x00ffe16a).l,D7b
000562E6: lsl.w #0x1,D7w
000562E8: add.w D7w,D4w
000562EA: rts
000562EC: lea (0xfffbbe).l,A5 ; data 00fffbbe
000562F2: lea (0xff1100).l,A6 ; data 00ff1100
000562F8: move.w (0x00fff118).l,D0w
000562FE: mulu.w #0x14,D0
00056302: clr.l D4
00056304: move.w #-0x1,D1w
00056308: bsr.w 0x00055f44 ; call 00055F44 FUN_00055f44
0005630C: add.b (0x00fffbbe).l,D6b
00056312: move.b (0x6,A6,D0w*0x1),D4b ; data 00ff1106
00056316: bsr.b 0x000562de ; call 000562DE FUN_000562de
00056318: cmpi.w #0x2,D4w
0005631C: bpl.w 0x00056324
00056320: move.w #0x2,D4w
00056324: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
0005632A: move.b (0x00fff0a4).l,D4b
00056330: sub.b (0x00fff0dd).l,D4b
00056336: beq.w 0x000563c4
0005633A: bgt.w 0x0005634a
0005633E: move.w #0x1,(0x00ffe07e).l
00056346: bra.w 0x000563c4
0005634A: clr.l D1
0005634C: sub.b D4b,D1b
0005634E: lea (0xfffbbe).l,A0 ; data 00fffbbe
00056354: add.b (0x1,A0),D1b
00056358: bpl.w 0x000563c4
0005635C: jsr 0x00019b40.l ; call 00019B40 FUN_00019b40
00056362: cmpi.w #0x8,D0w
00056366: bne.w 0x00056376
0005636A: jsr 0x00017d90.l ; call 00017D90 FUN_00017d90
00056370: move.w #-0x1,D1w
00056374: rts
00056376: jsr 0x0001a12e.l ; call 0001A12E FUN_0001a12e
0005637C: tst.b D1b
0005637E: bpl.w 0x000563c4
00056382: jsr 0x00017db8.l ; call 00017DB8 FUN_00017db8
00056388: move.w #-0x1,D1w
0005638C: tst.b (0x00fffbca).l
00056392: bne.w 0x000563c4
00056396: jsr 0x00019b40.l ; call 00019B40 FUN_00019b40
0005639C: cmpi.w #0x3,D0w
000563A0: beq.w 0x000563cc
000563A4: cmpi.w #0xc,D0w
000563A8: beq.w 0x000563cc
000563AC: cmpi.w #0xf,D0w
000563B0: beq.w 0x000563cc
000563B4: cmpi.w #0x17,D0w
000563B8: beq.w 0x000563cc
000563BC: cmpi.w #0x18,D0w
000563C0: beq.w 0x000563cc
000563C4: jsr 0x0001792e.l ; call 0001792E FUN_0001792e
000563CA: rts
000563CC: lea (0xfffbbe).l,A0 ; data 00fffbbe
000563D2: lea (0xff1100).l,A1 ; data 00ff1100
000563D8: move.w (0x00fff118).l,D1w
000563DE: mulu.w #0x14,D1
000563E2: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
000563E8: clr.l D4
000563EA: move.b (A0),D4b
000563EC: cmpi.w #0x2,D4w
000563F0: bpl.w 0x000563f8
000563F4: move.w #0x2,D4w
000563F8: jsr 0x00000dba.l ; call 00000DBA FUN_00000dba
000563FE: move.b (0x00fff0dd).l,D4b
00056404: beq.w 0x0005642e
00056408: sub.b (0x1,A0),D4b
0005640C: ble.w 0x0005642e
00056410: sub.b D4b,(A0)
00056412: bpl.w 0x0005641c
00056416: clr.b (A0)
00056418: clr.b (0x7,A0)
0005641C: tst.b (0x7,A0)
00056420: beq.w 0x00056428
00056424: subq.b #0x1,(0x7,A0)
00056428: jsr 0x0001b1fc.l ; call 0001B1FC FUN_0001b1fc
0005642E: move.w #-0x1,D1w
00056432: rts
00056434: jsr 0x0001a468.l ; call 0001A468 FUN_0001a468
0005643A: clr.b (0x00fff0a3).l
00056440: lea (0xff1100).l,A0 ; data 00ff1100
00056446: move.w (0x00fff118).l,D0w
0005644C: mulu.w #0x14,D0
00056450: sub.b D1b,(0xf,A0,D0w*0x1) ; data 00ff110f
00056454: bgt.w 0x00056480
00056458: clr.b (0xf,A0,D0w*0x1) ; data 00ff110f
0005645C: subq.b #0x2,(0x00fff0ce).l
00056462: bpl.w 0x00056480
00056466: move.b #0xf,(0x00fff0ce).l
0005646E: addq.b #0x1,(0x00ff018d).l
00056474: addq.b #0x1,(0x00ff028d).l
0005647A: addq.b #0x1,(0x00ff038d).l
00056480: rts
00056482: jsr 0x0001a468.l ; call 0001A468 FUN_0001a468
00056488: clr.b (0x00fff0a3).l
0005648E: lea (0xff1100).l,A0 ; data 00ff1100
00056494: move.w (0x00fff118).l,D0w
0005649A: mulu.w #0x14,D0
0005649E: sub.b D1b,(0x10,A0,D0w*0x1) ; data 00ff1110
000564A2: bpl.w 0x000564d4
000564A6: clr.b (0x10,A0,D0w*0x1) ; data 00ff1110
000564AA: jsr 0x0001a154.l ; call 0001A154 FUN_0001a154
000564B0: subq.b #0x2,(0x00fff0ce).l
000564B6: bpl.w 0x000564d4
000564BA: move.b #0xf,(0x00fff0ce).l
000564C2: addq.b #0x1,(0x00ff018d).l
000564C8: addq.b #0x1,(0x00ff028d).l
000564CE: addq.b #0x1,(0x00ff038d).l
000564D4: rts
000564D6: lea (0xff1100).l,A0 ; data 00ff1100
000564DC: move.w (0x00fff118).l,D0w
000564E2: mulu.w #0x14,D0
000564E6: lsr.w #0x1,D1w
000564E8: bgt.w 0x000564f0
000564EC: move.w #0x1,D1w
000564F0: sub.b D1b,(0x6,A0,D0w*0x1) ; data 00ff1106
000564F4: cmpi.b #0x2,(0x6,A0,D0w*0x1) ; data 00ff1106
000564FA: bpl.w 0x00056504
000564FE: move.b #0x2,(0x6,A0,D0w*0x1) ; data 00ff1106
00056504: rts
00056506: cmpi.b #0x4,(0x1a,A0,D3w*0x1)
0005650C: bmi.w 0x00056528
00056510: beq.w 0x000565fa
00056514: cmpi.b #0x8,(0x1a,A0,D3w*0x1)
0005651A: bpl.w 0x00056528
0005651E: jsr 0x00019c5c.l ; call 00019C5C FUN_00019c5c
00056524: move.w D7w,D6w
00056526: rts
00056528: clr.l D1
0005652A: bsr.w 0x00055f44 ; call 00055F44 FUN_00055f44
0005652E: add.b (0x00fffbbe).l,D6b
00056534: jsr 0x00019c5c.l ; call 00019C5C FUN_00019c5c
0005653A: cmp.w D6w,D7w
0005653C: bpl.w 0x00056542
00056540: move.w D7w,D6w
00056542: add.w D7w,D6w
00056544: bne.w 0x0005654c
00056548: move.w #0x1,D6w
0005654C: clr.l D4
0005654E: lea (0xff1100).l,A1 ; data 00ff1100
00056554: move.w (0x00fff118).l,D1w
0005655A: mulu.w #0x14,D1
0005655E: move.b (0x5,A1,D1w*0x1),D4b ; data 00ff1105
00056562: subq.b #0x1,D4b
00056564: sub.b D4b,D6b
00056566: ble.w 0x000565e8
0005656A: move.b (0x6,A1,D1w*0x1),D4b ; data 00ff1106
0005656E: bsr.w 0x000562de ; call 000562DE FUN_000562de
00056572: bsr.w 0x00056632 ; call 00056632 FUN_00056632
00056576: lea (0xfffbbe).l,A2 ; data 00fffbbe
0005657C: cmpi.b #0xb,(0x1a,A0,D3w*0x1)
00056582: beq.w 0x000565a4
00056586: move.l D6,-(SP) ; data Stack[-0x4]
00056588: jsr 0x00019bf6.l ; call 00019BF6 FUN_00019bf6
0005658E: move.l D6,D5
00056590: move.l (SP)+,D6
00056592: cmpi.b #0x4,(0x1a,A0,D3w*0x1)
00056598: bmi.w 0x000565b2
0005659C: move.b (0xa,A2),D4b
000565A0: bra.w 0x000565b6
000565A4: clr.l D5
000565A6: move.b (0x6,A1,D1w*0x1),D5b
000565AA: move.b (0xb,A2),D4b
000565AE: bra.w 0x000565b6
000565B2: move.b (0x9,A2),D4b
000565B6: bsr.w 0x000562de ; call 000562DE FUN_000562de
000565BA: cmpi.w #0x2,D4w
000565BE: bpl.w 0x000565c4
000565C2: moveq #0x2,D4
000565C4: moveq #0x7,D0
000565C6: sub.w D4w,D0w
000565C8: bgt.w 0x000565d2
000565CC: moveq #0xa,D0
000565CE: bra.w 0x000565d6
000565D2: mulu.w #0x14,D0
000565D6: mulu.w D0w,D5
000565D8: divu.w #0x64,D5
000565DC: sub.w D5w,D6w
000565DE: bpl.w 0x000565ec
000565E2: move.w #0x1,D6w
000565E6: rts
000565E8: clr.l D6
000565EA: rts
000565EC: cmpi.w #0x9,D6w
000565F0: bmi.w 0x000565f8
000565F4: move.w #0x8,D6w
000565F8: rts
000565FA: jsr 0x00019c5c.l ; call 00019C5C FUN_00019c5c
00056600: move.w D7w,D6w
00056602: cmpi.b #0x8,(0xc,A0)
00056608: blt.w 0x00056612
0005660C: moveq #0x4,D4
0005660E: bra.w 0x00056624
00056612: cmpi.b #0x5,(0xc,A0)
00056618: blt.w 0x00056622
0005661C: moveq #0x5,D4
0005661E: bra.w 0x00056624
00056622: moveq #0x6,D4
00056624: bsr.w 0x00056632 ; call 00056632 FUN_00056632
00056628: tst.w D6w
0005662A: bgt.b 0x000565ec
0005662C: move.w #0x1,D6w
00056630: bra.b 0x000565ec
00056632: cmpi.w #0x2,D4w
00056636: bpl.w 0x0005663e
0005663A: move.w #0x2,D4w
0005663E: moveq #0x7,D0
00056640: sub.w D4w,D0w
00056642: bgt.w 0x0005664c
00056646: moveq #0xa,D0
00056648: bra.w 0x00056650
0005664C: mulu.w #0x14,D0
00056650: mulu.w D0w,D6
00056652: divu.w #0x64,D6
00056656: rts
0005668C: clr.l D1
0005668E: move.b (0x00fff0a4).l,D1b
00056694: lsr.w #0x1,D1w
00056696: bsr.w 0x00056434 ; call 00056434 FUN_00056434
0005669A: jsr 0x00017e04.l ; call 00017E04 FUN_00017e04
000566A0: rts
000566A2: lea (0xfff9fa).l,A1 ; data 00fff9fa
000566A8: lea (0x166ad8).l,A0 ; data 00166ad8
000566AE: move.w #0x4,D6w
000566B2: move.w #0x10,D7w
000566B6: move.w (A0)+,(A1)+
000566B8: dbf D7w,0x000566b6
000566BC: dbf D6w,0x000566b2
000566C0: rts
000566C2: lea (0x18ac56).l,A0 ; data 0018ac56
000566C8: lea (0xffd91e).l,A1 ; data 00ffd91e
000566CE: moveq #0x17,D0
000566D0: move.l (A0)+,(A1)+
000566D2: dbf D0w,0x000566d0
000566D6: rts
000566D8: lea (0xffd87e).l,A1 ; data 00ffd87e
000566DE: bra.w 0x000566fc
000566E2: lea (0xffd89e).l,A1 ; data 00ffd89e
000566E8: bra.w 0x000566fc
000566EC: lea (0xffd8be).l,A1 ; data 00ffd8be
000566F2: bra.w 0x000566fc
000566F6: lea (0xffd8de).l,A1 ; data 00ffd8de
000566FC: moveq #0x7,D0
000566FE: move.l (A0)+,(A1)+
00056700: dbf D0w,0x000566fe
00056704: bset.b #0x0,(0x00ffef14).l
0005670C: rts
0005670E: lea (0x18ac56).l,A0 ; data 0018ac56
00056714: lea (0xffd89e).l,A1 ; data 00ffd89e
0005671A: moveq #0x17,D0
0005671C: move.l (A0)+,(A1)+
0005671E: dbf D0w,0x0005671c
00056722: bset.b #0x0,(0x00ffef14).l
0005672A: rts
0005672C: tst.b (0x00ffe18a).l
00056732: beq.w 0x00056738
00056736: rts
00056738: cmpi.w #0x14,(0x00ffddc4).l
00056740: beq.w 0x0005696a
00056744: cmpi.w #0x1,(0x00ffdf40).l
0005674C: beq.w 0x0005689a
00056750: cmpi.w #0x2,(0x00ffdf40).l
00056758: beq.w 0x0005696c
0005675C: tst.w (0x00ffdf40).l
00056762: bne.w 0x0005696a
00056766: cmpi.w #0x6,(0x00ffddc4).l
0005676E: beq.w 0x00056908
00056772: cmpi.w #0x7,(0x00ffddc4).l
0005677A: beq.w 0x00056908
0005677E: cmpi.w #0x8,(0x00ffddc4).l
00056786: beq.w 0x00056908
0005678A: cmpi.w #0x9,(0x00ffddc4).l
00056792: beq.w 0x00056908
00056796: cmpi.w #0x4,(0x00ffddc4).l
0005679E: beq.w 0x000569f8
000567A2: cmpi.w #0x5,(0x00ffddc4).l
000567AA: beq.w 0x00056a5c
000567AE: cmpi.w #0x0,(0x00ffddc4).l
000567B6: beq.w 0x000567d6
000567BA: cmpi.w #0x32,(0x00ffddc4).l
000567C2: bge.w 0x0005696a
000567C6: cmpi.w #0x2c,(0x00ffddc4).l
000567CE: bge.w 0x0005682c
000567D2: bra.w 0x0005696a
000567D6: lea (0x18ad42).l,A0 ; data 0018ad42
000567DC: subq.w #0x1,(0x00ffdf90).l
000567E2: bpl.w 0x0005696a
000567E6: move.w #0x6,(0x00ffdf90).l
000567EE: addq.w #0x1,(0x00ffdf92).l
000567F4: cmpi.w #0x6,(0x00ffdf92).l
000567FC: bcs.w 0x00056806
00056800: clr.w (0x00ffdf92).l
00056806: move.w (0x00ffdf92).l,D1w
0005680C: lsl.w #0x1,D1w
0005680E: move.w #-0x3fec,D0w
00056812: move.w D0w,(0x00c00004).l
00056818: move.w #0x0,(0x00c00004).l
00056820: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
00056828: bra.w 0x0005696a
0005682C: lea (0x18ad5a).l,A0 ; data 0018ad5a
00056832: subq.w #0x1,(0x00ffdf90).l
00056838: bpl.w 0x0005696a
0005683C: move.w #0x9,(0x00ffdf90).l
00056844: addq.w #0x1,(0x00ffdf92).l
0005684A: cmpi.w #0x4,(0x00ffdf92).l
00056852: bcs.w 0x0005685c
00056856: clr.w (0x00ffdf92).l
0005685C: move.w (0x00ffdf92).l,D1w
00056862: lsl.w #0x1,D1w
00056864: move.w #-0x3fe8,D0w
00056868: move.w D0w,(0x00c00004).l
0005686E: move.w #0x0,(0x00c00004).l
00056876: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
0005687E: move.w (0x2,A0,D1w*0x1),(0x00c00000).l
00056886: move.w (0x4,A0,D1w*0x1),(0x00c00000).l
0005688E: move.w (0x6,A0,D1w*0x1),(0x00c00000).l
00056896: bra.w 0x0005696a
0005689A: lea (0x18ad6a).l,A0 ; data 0018ad6a
000568A0: subq.w #0x1,(0x00ffdf90).l
000568A6: bpl.w 0x0005696a
000568AA: move.w #0x6,(0x00ffdf90).l
000568B2: addq.w #0x1,(0x00ffdf92).l
000568B8: cmpi.w #0x4,(0x00ffdf92).l
000568C0: bcs.w 0x000568ca
000568C4: clr.w (0x00ffdf92).l
000568CA: move.w (0x00ffdf92).l,D1w
000568D0: lsl.w #0x1,D1w
000568D2: move.w #-0x3fe8,D0w
000568D6: move.w D0w,(0x00c00004).l
000568DC: move.w #0x0,(0x00c00004).l
000568E4: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
000568EC: move.w (0x2,A0,D1w*0x1),(0x00c00000).l
000568F4: move.w (0x4,A0,D1w*0x1),(0x00c00000).l
000568FC: move.w (0x6,A0,D1w*0x1),(0x00c00000).l
00056904: bra.w 0x0005696a
00056908: move.w #-0x3fe6,D0w
0005690C: lea (0x18ad7a).l,A0 ; data 0018ad7a
00056912: subq.w #0x1,(0x00ffdf90).l
00056918: bpl.w 0x0005696a
0005691C: move.w #0xc,(0x00ffdf90).l
00056924: addq.w #0x1,(0x00ffdf92).l
0005692A: cmpi.w #0x3,(0x00ffdf92).l
00056932: bcs.w 0x0005693c
00056936: clr.w (0x00ffdf92).l
0005693C: move.w (0x00ffdf92).l,D1w
00056942: lsl.w #0x3,D1w
00056944: move.w D0w,(0x00c00004).l
0005694A: move.w #0x0,(0x00c00004).l
00056952: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
0005695A: move.w (0x2,A0,D1w*0x1),(0x00c00000).l
00056962: move.w (0x4,A0,D1w*0x1),(0x00c00000).l
0005696A: rts
0005696C: move.w (0x00ffdf88).l,D0w
00056972: andi.w #0xf,D0w
00056976: cmpi.w #0xf,D0w
0005697A: bne.b 0x0005696a
0005697C: tst.w (0x00ffdf90).l
00056982: bpl.w 0x000569b0
00056986: subq.w #0x1,(0x00ffdf92).l
0005698C: andi.w #0x7,(0x00ffdf92).l
00056994: cmpi.w #0x7,(0x00ffdf92).l
0005699C: bne.w 0x000569d2
000569A0: clr.w (0x00ffdf90).l
000569A6: clr.w (0x00ffdf92).l
000569AC: bra.w 0x000569d2
000569B0: addq.w #0x1,(0x00ffdf92).l
000569B6: andi.w #0x7,(0x00ffdf92).l
000569BE: bne.w 0x000569d2
000569C2: move.w #-0x1,(0x00ffdf90).l
000569CA: move.w #0x7,(0x00ffdf92).l
000569D2: move.w (0x00ffdf92).l,D1w
000569D8: lsl.w #0x1,D1w
000569DA: lsl.w #0x8,D1w
000569DC: move.w #-0x3fe2,D0w
000569E0: move.w D0w,(0x00c00004).l
000569E6: move.w #0x0,(0x00c00004).l
000569EE: move.w D1w,(0x00c00000).l
000569F4: bra.w 0x0005696a
000569F8: move.w #-0x3ff4,D0w
000569FC: lea (0x18ad92).l,A0 ; data 0018ad92
00056A02: subq.w #0x1,(0x00ffdf90).l
00056A08: bpl.w 0x0005696a
00056A0C: move.w #0xa,(0x00ffdf90).l
00056A14: addq.w #0x1,(0x00ffdf92).l
00056A1A: cmpi.w #0x3,(0x00ffdf92).l
00056A22: bcs.w 0x00056a2c
00056A26: clr.w (0x00ffdf92).l
00056A2C: move.w (0x00ffdf92).l,D1w
00056A32: lsl.w #0x3,D1w
00056A34: move.w D0w,(0x00c00004).l
00056A3A: move.w #0x0,(0x00c00004).l
00056A42: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
00056A4A: move.w (0x2,A0,D1w*0x1),(0x00c00000).l
00056A52: move.w (0x4,A0,D1w*0x1),(0x00c00000).l
00056A5A: rts
00056A5C: move.w #-0x3fec,D0w
00056A60: lea (0x18ad4e).l,A0 ; data 0018ad4e
00056A66: subq.w #0x1,(0x00ffdf90).l
00056A6C: bpl.w 0x0005696a
00056A70: move.w #0xa,(0x00ffdf90).l
00056A78: addq.w #0x1,(0x00ffdf92).l
00056A7E: cmpi.w #0x3,(0x00ffdf92).l
00056A86: bcs.w 0x00056a90
00056A8A: clr.w (0x00ffdf92).l
00056A90: move.w (0x00ffdf92).l,D1w
00056A96: lsl.w #0x1,D1w
00056A98: move.w D0w,(0x00c00004).l
00056A9E: move.w #0x0,(0x00c00004).l
00056AA6: move.w (0x0,A0,D1w*0x1),(0x00c00000).l
00056AAE: move.w (0x2,A0,D1w*0x1),(0x00c00000).l
00056AB6: move.w (0x4,A0,D1w*0x1),(0x00c00000).l
00056ABE: rts
00056AC0: clr.b (0x00fff0b1).l
00056AC6: move.w #-0x6f00,(0x00c00004).l
00056ACE: jsr 0x00010bc4.l ; call 00010BC4 FUN_00010bc4
00056AD4: jsr 0x00011048.l ; call 00011048 FUN_00011048
00056ADA: jsr 0x0005670e.l ; call 0005670E FUN_0005670e
00056AE0: cmpi.b #0x1,(0x00fff0b7).l
00056AE8: bne.w 0x00056b04
00056AEC: move.l #0xcf97c,D0
00056AF2: move.l #0x89a0,D1
00056AF8: move.l #0x5e0,D2
00056AFE: jsr 0x00000f5c.l ; call 00000F5C FUN_00000f5c
00056B04: cmpi.b #0x2,(0x00fff0b7).l
00056B0C: bne.w 0x00056b40
00056B10: lea (0xe2e9a).l,A0 ; data 000e2e9a
00056B16: jsr 0x000566e2.l ; call 000566E2 FUN_000566e2
00056B1C: lea (0x1ac8ec).l,A0 ; data 001ac8ec
00056B22: jsr 0x000566f6.l ; call 000566F6 FUN_000566f6
00056B28: lea (0xffbf40).l,A1 ; data 00ffbf40
00056B2E: movea.l #0xd86e0,A3
00056B34: move.l #0x89a0,D0
00056B3A: jsr 0x00001530.l ; call 00001530 FUN_00001530
00056B40: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
00056B46: clr.l D0
00056B48: lea (0x1aab50).l,A1 ; data 001aab50
00056B4E: move.b (0x00fff0b6).l,D0b
00056B54: lsl.w #0x2,D0w
00056B56: adda.l D0,A1
00056B58: movea.l (A1)+,A1 ; data 001aab50
00056B5A: movea.l (A1)+,A2
00056B5C: move.w #0x6000,D1w
00056B60: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056B66: tst.b (0x00fff0b7).l
00056B6C: bne.w 0x00056b90
00056B70: movea.l (0x00fffc04).l,A1
00056B76: adda.l #0xa0,A1
00056B7C: move.w (A1),D1w
00056B7E: mulu.w #0x22,D1
00056B82: lea (0xfff9fa).l,A1 ; data 00fff9fa
00056B88: adda.l D1,A1
00056B8A: move.l A1,(0x00fff10e).l
00056B90: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00056B96: clr.b (0x00fff112).l
00056B9C: clr.b (0x00fff0d0).l
00056BA2: clr.b (0x00fff0b8).l
00056BA8: move.b #-0x1,(0x00fff0c2).l
00056BB0: clr.l D2
00056BB2: movea.l (0x00fffc04).l,A0
00056BB8: move.w (0x2c,A0),D2w
00056BBC: divu.w #0x2000,D2
00056BC0: subq.w #0x1,D2w
00056BC2: lsl.w #0x2,D2w
00056BC4: lea (0x18acb6).l,A0 ; data 0018acb6
00056BCA: movea.l (0x0,A0,D2w*0x1),A0 ; data 0018acb6
00056BCE: jsr 0x000566ec.l ; call 000566EC FUN_000566ec
00056BD4: movea.l (0x00fffc04).l,A0
00056BDA: move.w #-0x1cba,D4w
00056BDE: move.w #0x4000,D2w
00056BE2: jsr 0x000110ec.l ; call 000110EC FUN_000110ec
00056BE8: clr.b (0x00fff0d1).l
00056BEE: jsr 0x0000c406.l ; call 0000C406 FUN_0000c406
00056BF4: cmpi.b #0x2,(0x00fff0b7).l
00056BFC: bne.w 0x00056c2c
00056C00: btst.b #0x0,(0x00ff018f).l
00056C08: bne.w 0x00056c2c
00056C0C: movea.l #0xc316,A2
00056C12: lea (0x1aa3b4).l,A1 ; data 001aa3b4
00056C18: move.w #0x6000,D1w
00056C1C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056C22: jsr 0x00000d94.l ; call 00000D94 FUN_00000d94
00056C28: bra.w 0x00056da8
00056C2C: cmpi.b #0x1,(0x00fff0b7).l
00056C34: bne.w 0x00056c48
00056C38: move.b #0x1,(0x00fff0d1).l
00056C40: bsr.w 0x00058d32 ; call 00058D32 FUN_00058d32
00056C44: bra.w 0x00056c50
00056C48: bsr.w 0x00058b5e ; call 00058B5E FUN_00058b5e
00056C4C: bsr.w 0x00058bb6 ; call 00058BB6 FUN_00058bb6
00056C50: bsr.w 0x000587da ; call 000587DA FUN_000587da
00056C54: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00056C5A: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00056C60: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00056C66: btst.b #0xa,(0x00ffdd22).l
00056C6E: beq.w 0x00056c82
00056C72: move.w #0x25,D0w
00056C76: jsr 0x000530de.l ; call 000530DE FUN_000530de
00056C7C: moveq #-0x1,D0
00056C7E: bra.w 0x00056cb6
00056C82: btst.b #0x8,(0x00ffdd22).l
00056C8A: beq.w 0x00056c9e
00056C8E: move.w #0x25,D0w
00056C92: jsr 0x000530de.l ; call 000530DE FUN_000530de
00056C98: moveq #-0x1,D0
00056C9A: bra.w 0x00056cd4
00056C9E: btst.b #0xb,(0x00ffdd22).l
00056CA6: beq.w 0x00056cbc
00056CAA: move.w #0x25,D0w
00056CAE: jsr 0x000530de.l ; call 000530DE FUN_000530de
00056CB4: moveq #0x1,D0
00056CB6: bsr.w 0x00058c54 ; call 00058C54 FUN_00058c54
00056CBA: bra.b 0x00056c54
00056CBC: btst.b #0x9,(0x00ffdd22).l
00056CC4: beq.w 0x00056ce8
00056CC8: move.w #0x25,D0w
00056CCC: jsr 0x000530de.l ; call 000530DE FUN_000530de
00056CD2: moveq #0x1,D0
00056CD4: cmpi.b #0x1,(0x00fff0b7).l
00056CDC: beq.w 0x00056c54
00056CE0: bsr.w 0x00058cde ; call 00058CDE FUN_00058cde
00056CE4: bra.w 0x00056c54
00056CE8: btst.b #0xe,(0x00ffdd22).l
00056CF0: beq.w 0x00056d6c
00056CF4: move.w #0x24,D0w
00056CF8: jsr 0x000530de.l ; call 000530DE FUN_000530de
00056CFE: tst.b (0x00fff0d1).l
00056D04: beq.w 0x00056d2a
00056D08: bsr.w 0x00056dc4 ; call 00056DC4 FUN_00056dc4
00056D0C: tst.b (0x00fff112).l
00056D12: bmi.w 0x00056c54
00056D16: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00056D1C: move.b (0x00fff0b8).l,(0x00fff0d0).l
00056D26: bra.w 0x00056be8
00056D2A: cmpi.b #0x3,(0x00fff0d0).l
00056D32: bne.w 0x00056d64
00056D36: tst.b (0x00fff0b7).l
00056D3C: bpl.w 0x00056d64
00056D40: tst.b (0x00fff0c2).l
00056D46: bpl.w 0x00056d64
00056D4A: movea.l #0xc58e,A2
00056D50: lea (0x1aa5c8).l,A1 ; data 001aa5c8
00056D56: move.w #0x6000,D1w
00056D5A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056D60: bra.w 0x00056c54
00056D64: bsr.w 0x000587da ; call 000587DA FUN_000587da
00056D68: bra.w 0x00056c54
00056D6C: btst.b #0xc,(0x00ffdd22).l
00056D74: beq.w 0x00056d7c
00056D78: bra.w 0x00056cf4
00056D7C: btst.b #0xd,(0x00ffdd22).l
00056D84: beq.w 0x00056d9c
00056D88: move.w #0x24,D0w
00056D8C: jsr 0x000530de.l ; call 000530DE FUN_000530de
00056D92: jsr 0x0000d4c8.l ; call 0000D4C8 FUN_0000d4c8
00056D98: bra.w 0x00056b66
00056D9C: btst.b #0xf,(0x00ffdd22).l
00056DA4: beq.w 0x00056c54
00056DA8: jsr 0x0000334c.l ; call 0000334C FUN_0000334c
00056DAE: jsr 0x000039f2.l ; call 000039F2 FUN_000039f2
00056DB4: jsr 0x00003404.l ; call 00003404 FUN_00003404
00056DBA: move.w #-0x6e6f,(0x00c00004).l
00056DC2: rts
00056DC4: clr.b (0x00fff112).l
00056DCA: clr.l D0
00056DCC: move.b (0x00fff0b6).l,D0b
00056DD2: tst.b (0x00fff0b7).l
00056DD8: bmi.w 0x00056e0c
00056DDC: beq.w 0x00056e5a
00056DE0: cmpi.b #0x1,(0x00fff0b7).l
00056DE8: beq.w 0x000571b2
00056DEC: cmpi.b #0x3,(0x00fff0b8).l
00056DF4: bmi.w 0x0005751a
00056DF8: beq.w 0x000573fc
00056DFC: cmpi.b #0x5,(0x00fff0b8).l
00056E04: beq.w 0x000577ba
00056E08: bra.w 0x0005766a ; call 0005766A FUN_0005766a
00056E0C: tst.b (0x00fff0b8).l
00056E12: beq.w 0x00056e3a
00056E16: cmpi.b #0x2,(0x00fff0b8).l
00056E1E: bmi.w 0x00056e4e
00056E22: beq.w 0x00056e44
00056E26: cmpi.b #0x4,(0x00fff0b8).l
00056E2E: bmi.w 0x00057026
00056E32: beq.w 0x000578c4
00056E36: bra.w 0x00056f4a
00056E3A: lea (0x1aac86).l,A0 ; data 001aac86
00056E40: bra.w 0x00056e54
00056E44: lea (0x1aad77).l,A0 ; data 001aad77
00056E4A: bra.w 0x00056e54
00056E4E: lea (0x1aadbf).l,A0 ; data 001aadbf
00056E54: bsr.w 0x0005794e ; call 0005794E FUN_0005794e
00056E58: rts
00056E5A: cmpi.b #0x3,(0x00fff0b8).l
00056E62: beq.w 0x00056f4a
00056E66: movea.l (0x00fffc04).l,A0
00056E6C: cmpi.b #0x2,(0x9b,A0)
00056E72: bmi.w 0x00056e96
00056E76: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
00056E7A: movea.l #0xc590,A2
00056E80: lea (0x1aa42c).l,A1 ; data 001aa42c
00056E86: move.w #0x6000,D1w
00056E8A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056E90: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00056E96: subq.w #0x3,D0w
00056E98: cmpi.b #0x1,(0x00fff0b8).l
00056EA0: beq.w 0x00056eb0
00056EA4: bmi.w 0x00056eb8
00056EA8: lea (0x1aaeb0).l,A0 ; data 001aaeb0
00056EAE: bra.b 0x00056e54
00056EB0: lea (0x1aaef8).l,A0 ; data 001aaef8
00056EB6: bra.b 0x00056e54
00056EB8: lea (0x1aaf40).l,A0 ; data 001aaf40
00056EBE: mulu.w #0x20,D0
00056EC2: adda.l D0,A0
00056EC4: clr.l D0
00056EC6: move.b (0x00fff0d0).l,D0b
00056ECC: mulu.w #0x4,D0
00056ED0: adda.l D0,A0
00056ED2: tst.b (A0)
00056ED4: bpl.w 0x00056ee2
00056ED8: move.b #-0x1,(0x00fff112).l
00056EE0: rts
00056EE2: movea.l (0x00fff0c2).l,A1
00056EE8: cmpi.b #0x6,(0x1,A1)
00056EEE: beq.w 0x00056f48
00056EF2: clr.l D1
00056EF4: move.l (0x00fff0be).l,D1
00056EFA: cmp.l (0x00fffbfe).l,D1
00056F00: ble.w 0x00056f24
00056F04: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
00056F08: movea.l #0xc58e,A2
00056F0E: lea (0x1aa587).l,A1 ; data 001aa587
00056F14: move.w #0x6000,D1w
00056F18: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056F1E: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00056F24: sub.l D1,(0x00fffbfe).l
00056F2A: movea.l (0x00fff0c2).l,A1
00056F30: tst.b (0x1,A1)
00056F34: bgt.w 0x00056f40
00056F38: movea.l (0x00fff10e).l,A0
00056F3E: addq.w #0x1,(A0)
00056F40: move.b (0x00fff0ae).l,(A1)+
00056F46: addq.b #0x1,(A1)
00056F48: rts
00056F4A: bsr.w 0x00057c5e ; call 00057C5E FUN_00057c5e
00056F4E: clr.l D7
00056F50: move.b (0x00fff0d0).l,D7b
00056F56: movea.l (0x00fffc04).l,A0
00056F5C: move.b (0x66,A0,D7w*0x1),D0b
00056F60: bgt.w 0x00056f6e
00056F64: move.b #-0x1,(0x00fff112).l
00056F6C: rts
00056F6E: move.w #0x6000,D1w
00056F72: cmpi.b #0x29,D0b
00056F76: beq.w 0x00056f8a
00056F7A: cmpi.b #0x2a,D0b
00056F7E: beq.w 0x00056f8a
00056F82: cmpi.b #0x2b,D0b
00056F86: bne.w 0x00056fa2
00056F8A: movea.l #0xc60c,A2
00056F90: lea (0x1aa3dd).l,A1 ; data 001aa3dd
00056F96: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056F9C: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00056FA2: lea (0x1aa72b).l,A1 ; data 001aa72b
00056FA8: movea.l #0xc58e,A2
00056FAE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00056FB4: move.l (0x00fff0be).l,D1
00056FBA: movea.l #0xc5b6,A2
00056FC0: move.w #0x6000,D2w
00056FC4: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00056FCA: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
00056FCE: tst.b (0x00fff0dc).l
00056FD4: bne.w 0x00057024
00056FD8: move.l (0x00fff0be).l,D1
00056FDE: add.l D1,(0x00fffbfe).l
00056FE4: movea.l (0x00fffc04).l,A0
00056FEA: clr.l D7
00056FEC: move.b (0x00fff0d0).l,D7b
00056FF2: movea.l (0x00fffc04).l,A0
00056FF8: move.b (0x66,A0,D7w*0x1),D0b
00056FFC: move.b (0x76,A0),D6b
00057000: addi.b #0x2b,D6b
00057004: cmp.b D6b,D0b
00057006: bne.w 0x0005700e
0005700A: clr.b (0x76,A0)
0005700E: cmp.b (0x56,A0),D7b
00057012: bne.w 0x0005701e
00057016: move.w #-0x1,(0x00fff098).l
0005701E: jsr 0x0001275c.l ; call 0001275C FUN_0001275c
00057024: rts
00057026: move.l #0xc384,D4
0005702C: moveq #0x7,D1
0005702E: bsr.w 0x00057c66 ; call 00057C66 FUN_00057c66
00057032: clr.l D0
00057034: move.b (0x00fff0b6).l,D0b
0005703A: mulu.w #0x10,D0
0005703E: lea (0x1aae8c).l,A0 ; data 001aae8c
00057044: adda.l D0,A0
00057046: cmpi.b #0x6,(0x00fff0d0).l
0005704E: bmi.w 0x0005705c
00057052: move.b #-0x1,(0x00fff112).l
0005705A: rts
0005705C: clr.l D1
0005705E: move.b (0x00fff0d0).l,D1b
00057064: move.b (0x00fff0c2).l,D0b
0005706A: movea.l (0x00fffc04).l,A1
00057070: btst.b D1,(0x6e,A1,D0w*0x1)
00057074: beq.w 0x00057088
00057078: movea.l #0xc58c,A2
0005707E: lea (0x1aa6e4).l,A1 ; data 001aa6e4
00057084: bra.w 0x0005710e
00057088: tst.b (0x00fff0d0).l
0005708E: beq.w 0x000570ec
00057092: move.b (0x00fff0c2).l,D0b
00057098: movea.l (0x00fffc04).l,A1
0005709E: cmpi.b #0x8,(0x66,A1,D0w*0x1)
000570A4: bpl.w 0x000570d0
000570A8: cmpi.b #0x1,(0x00fff0d0).l
000570B0: beq.w 0x000570ec
000570B4: cmpi.b #0x3,(0x00fff0d0).l
000570BC: beq.w 0x000570ec
000570C0: movea.l #0xc58e,A2
000570C6: lea (0x1aa65c).l,A1 ; data 001aa65c
000570CC: bra.w 0x0005710e
000570D0: cmpi.b #0x1,(0x00fff0d0).l
000570D8: bne.w 0x000570ec
000570DC: movea.l #0xc58a,A2
000570E2: lea (0x1aa683).l,A1 ; data 001aa683
000570E8: bra.w 0x0005710e
000570EC: cmpi.b #0x3,(0x00fff0d0).l
000570F4: bne.w 0x0005711e
000570F8: btst.b #0x0,(0x6e,A1,D0w*0x1)
000570FE: beq.w 0x0005711e
00057102: movea.l #0xc58a,A2
00057108: lea (0x1aa6b2).l,A1 ; data 001aa6b2
0005710E: move.w #0x6000,D1w
00057112: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057118: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
0005711E: movea.l #0xc58e,A2
00057124: move.l (0x00fff0be).l,D1
0005712A: cmp.l (0x00fffbfe).l,D1
00057130: ble.w 0x0005714a
00057134: lea (0x1aa587).l,A1 ; data 001aa587
0005713A: move.w #0x6000,D1w
0005713E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057144: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
0005714A: lea (0x1aa758).l,A1 ; data 001aa758
00057150: move.w #0x6000,D1w
00057154: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005715A: move.l (0x00fff0be).l,D1
00057160: movea.l #0xc5b6,A2
00057166: move.w #0x6000,D2w
0005716A: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057170: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
00057174: tst.b (0x00fff0dc).l
0005717A: bne.w 0x000571b0
0005717E: sub.l D1,(0x00fffbfe).l
00057184: moveq #0x1,D1
00057186: clr.l D0
00057188: move.b (0x00fff0d0).l,D0b
0005718E: lsl.w D0,D1w
00057190: move.b (0x00fff0c2).l,D0b
00057196: movea.l (0x00fffc04).l,A1
0005719C: or.b D1b,(0x6e,A1,D0w*0x1)
000571A0: tst.b (0x00fff0d0).l
000571A6: bne.w 0x000571b0
000571AA: bclr.b #0x3,(0x6e,A1,D0w*0x1)
000571B0: rts
000571B2: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
000571B6: clr.w (0x00ffef96).l
000571BC: movea.l (0x00fffc04).l,A5
000571C2: move.w (0x90,A5),D0w
000571C6: move.b (0x00fff0d0).l,(0x00fff0d5).l
000571D0: cmpi.b #0x2,(0x00fff0d5).l
000571D8: bne.w 0x000571f4
000571DC: btst.l #0x3,D0
000571E0: beq.w 0x000571f4
000571E4: movea.l #0xc592,A2
000571EA: lea (0x1aa7ad).l,A1 ; data 001aa7ad
000571F0: bra.w 0x000572f8
000571F4: cmpi.b #0x3,(0x00fff0d5).l
000571FC: bne.w 0x00057208
00057200: move.w #0x1,(0x00ffef96).l
00057208: cmpi.b #0x5,(0x00fff0d5).l
00057210: bmi.w 0x000572be
00057214: beq.w 0x00057292
00057218: cmpi.b #0x6,(0x00fff0d5).l
00057220: beq.w 0x0005725c
00057224: addq.b #0x5,(0x00fff0d5).l
0005722A: moveq #0x2,D7
0005722C: moveq #0xc,D1
0005722E: btst.l D1,D0
00057230: beq.w 0x00057252
00057234: subi.w #0xb,D1w
00057238: cmpi.w #0x3,D1w
0005723C: beq.w 0x000572cc
00057240: add.b D1b,(0x00fff0d5).l
00057246: move.w #0x1,(0x00ffef96).l
0005724E: bra.w 0x000572be
00057252: addq.w #0x1,D1w
00057254: dbf D7w,0x0005722e
00057258: bra.w 0x000572be
0005725C: addq.b #0x3,(0x00fff0d5).l
00057262: moveq #0x2,D7
00057264: moveq #0x9,D1
00057266: btst.l D1,D0
00057268: beq.w 0x00057288
0005726C: subq.w #0x8,D1w
0005726E: cmpi.w #0x3,D1w
00057272: beq.w 0x000572cc
00057276: add.b D1b,(0x00fff0d5).l
0005727C: move.w #0x1,(0x00ffef96).l
00057284: bra.w 0x000572be
00057288: addq.w #0x1,D1w
0005728A: dbf D7w,0x00057266
0005728E: bra.w 0x000572be
00057292: moveq #0x3,D7
00057294: moveq #0x5,D1
00057296: btst.l D1,D0
00057298: beq.w 0x000572b8
0005729C: subq.w #0x4,D1w
0005729E: cmpi.w #0x4,D1w
000572A2: beq.w 0x000572cc
000572A6: add.b D1b,(0x00fff0d5).l
000572AC: move.w #0x1,(0x00ffef96).l
000572B4: bra.w 0x000572be
000572B8: addq.w #0x1,D1w
000572BA: dbf D7w,0x00057296
000572BE: clr.l D1
000572C0: move.b (0x00fff0d5).l,D1b
000572C6: btst.l D1,D0
000572C8: beq.w 0x000572dc
000572CC: movea.l #0xc590,A2
000572D2: lea (0x1aa787).l,A1 ; data 001aa787
000572D8: bra.w 0x000572f8
000572DC: move.l (0x00fff0be).l,D1
000572E2: cmp.l (0x00fffbfe).l,D1
000572E8: ble.w 0x00057308
000572EC: movea.l #0xc58e,A2
000572F2: lea (0x1aa587).l,A1 ; data 001aa587
000572F8: move.w #0x6000,D1w
000572FC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057302: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00057308: movea.l #0xc58c,A2
0005730E: lea (0x1aa76f).l,A1 ; data 001aa76f
00057314: move.w #0x6000,D1w
00057318: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005731E: move.l (0x00fff0be).l,D1
00057324: movea.l #0xc5b6,A2
0005732A: move.w #0x6000,D2w
0005732E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057334: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
00057338: tst.b (0x00fff0dc).l
0005733E: bne.w 0x000573fa
00057342: move.l D1,-(SP) ; data Stack[-0x4]
00057344: clr.l D1
00057346: move.b (0x00fff0d5).l,D1b
0005734C: movea.l (0x00fffc04).l,A1
00057352: lea (0x1aa8a0).l,A4 ; data 001aa8a0
00057358: clr.l D0
0005735A: move.b (0x7e,A1),D0b
0005735E: mulu.w #0xa,D0
00057362: add.b (0x8e,A1),D0b
00057366: sub.b (0x0,A4,D1w*0x1),D0b ; data 001aa8a0
0005736A: move.w (0x90,A1),D7w
0005736E: cmpi.b #0x3,D1b
00057372: bne.w 0x00057380
00057376: btst.l #0x2,D7
0005737A: beq.w 0x00057380
0005737E: addq.w #0x1,D0w
00057380: tst.b D0b
00057382: bgt.w 0x0005739c
00057386: move.l (SP)+,D1
00057388: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
0005738C: movea.l #0xc590,A2
00057392: lea (0x1aa7e2).l,A1 ; data 001aa7e2
00057398: bra.w 0x000572f8
0005739C: divu.w #0xa,D0
000573A0: move.b D0b,(0x7e,A1)
000573A4: swap D0
000573A6: move.b D0b,(0x8e,A1)
000573AA: move.w (0x90,A1),D0w
000573AE: bset.l D1,D0
000573B0: tst.w (0x00ffef96).l
000573B6: beq.w 0x000573be
000573BA: subq.w #0x1,D1w
000573BC: bclr.l D1,D0
000573BE: move.w D0w,(0x90,A1)
000573C2: move.l (SP)+,D1
000573C4: sub.l D1,(0x00fffbfe).l
000573CA: cmpi.b #0x2,(0x9b,A1)
000573D0: bpl.w 0x000573fa
000573D4: move.b (0x7e,A1),(0x7f,A1)
000573DA: movea.l (0x00fff10e).l,A2
000573E0: move.w (A2)+,D7w
000573E2: move.b (0x1,A2),D6b
000573E6: cmp.b (0x7e,A1),D6b
000573EA: ble.w 0x000573f4
000573EE: move.b (0x7e,A1),(0x1,A2)
000573F4: addq.l #0x2,A2
000573F6: dbf D7w,0x000573e2
000573FA: rts
000573FC: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
00057400: clr.l D0
00057402: move.b (0x00fff0b6).l,D0b
00057408: subq.w #0x6,D0w
0005740A: beq.w 0x00057412
0005740E: move.w #0x1,D0w
00057412: mulu.w #0x70,D0
00057416: lea (0x1ab196).l,A0 ; data 001ab196
0005741C: adda.l D0,A0
0005741E: clr.l D0
00057420: move.b (0x00fff0d0).l,D0b
00057426: mulu.w #0xe,D0
0005742A: tst.b (0x0,A0,D0w*0x1) ; data 001ab206
0005742E: bpl.w 0x0005743c
00057432: move.b #-0x1,(0x00fff112).l
0005743A: rts
0005743C: move.b (0x00fffbcb).l,D1b
00057442: cmp.b (0x0,A0,D0w*0x1),D1b ; data 001ab206
00057446: bmi.w 0x00057466
0005744A: movea.l #0xc410,A2
00057450: lea (0x1aa836).l,A1 ; data 001aa836
00057456: move.w #0x6000,D1w
0005745A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057460: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00057466: move.l (0x00fff0be).l,D1
0005746C: cmp.l (0x00fffbfe).l,D1
00057472: ble.w 0x00057492
00057476: movea.l #0xc40e,A2
0005747C: lea (0x1aa587).l,A1 ; data 001aa587
00057482: move.w #0x6000,D1w
00057486: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005748C: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00057492: movea.l #0xc58a,A2
00057498: lea (0x1aa81b).l,A1 ; data 001aa81b
0005749E: move.w #0x6000,D1w
000574A2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000574A8: move.l (0x00fff0be).l,D1
000574AE: movea.l #0xc5ba,A2
000574B4: move.w #0x6000,D2w
000574B8: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000574BE: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
000574C2: tst.b (0x00fff0dc).l
000574C8: bne.w 0x00057518
000574CC: sub.l D1,(0x00fffbfe).l
000574D2: lea (0xfffbbe).l,A1 ; data 00fffbbe
000574D8: move.b (0x6,A0,D0w*0x1),(A1)
000574DC: move.b (0x7,A0,D0w*0x1),(0x1,A1)
000574E2: move.w (0x8,A0,D0w*0x1),D1w
000574E6: cmp.w (0x2,A1),D1w
000574EA: bmi.w 0x000574f4
000574EE: move.w (0x8,A0,D0w*0x1),(0x2,A1)
000574F4: move.w (0xa,A0,D0w*0x1),D1w
000574F8: cmp.w (0x4,A1),D1w
000574FC: bmi.w 0x00057506
00057500: move.w (0xa,A0,D0w*0x1),(0x4,A1)
00057506: move.b (0xc,A0,D0w*0x1),(0x6,A1)
0005750C: move.b (0xd,A0,D0w*0x1),(0x7,A1)
00057512: move.b (0x0,A0,D0w*0x1),(0xd,A1)
00057518: rts
0005751A: cmpi.b #0x9,(0x00fff0d6).l
00057522: bpl.w 0x00057530
00057526: bsr.w 0x00057628 ; call 00057628 FUN_00057628
0005752A: tst.b (A0)
0005752C: bpl.w 0x0005753a
00057530: move.b #-0x1,(0x00fff112).l
00057538: rts
0005753A: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
0005753E: movea.l #0xc28c,A2
00057544: lea (0x1aa468).l,A1 ; data 001aa468
0005754A: move.w #0x6000,D1w
0005754E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057554: clr.l D1
00057556: move.b (0x00fff0d6).l,D1b
0005755C: movea.l #0xc2a4,A2
00057562: move.w #0x6000,D2w
00057566: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005756C: move.l (0x00fff0be).l,D1
00057572: movea.l #0xc32a,A2
00057578: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005757E: cmp.l (0x00fffbfe).l,D1
00057584: ble.w 0x000575a4
00057588: movea.l #0xc58e,A2
0005758E: lea (0x1aa587).l,A1 ; data 001aa587
00057594: move.w #0x6000,D1w
00057598: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005759E: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
000575A4: bsr.w 0x000575f8 ; call 000575F8 FUN_000575f8
000575A8: bpl.w 0x000575c8
000575AC: movea.l #0xc588,A2
000575B2: lea (0x1aa3f5).l,A1 ; data 001aa3f5
000575B8: move.w #0x6000,D1w
000575BC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000575C2: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
000575C8: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
000575CC: tst.b (0x00fff0dc).l
000575D2: bne.w 0x000575f6
000575D6: move.l (0x00fff0be).l,D1
000575DC: sub.l D1,(0x00fffbfe).l
000575E2: bsr.w 0x00057628 ; call 00057628 FUN_00057628
000575E6: clr.l D7
000575E8: move.b (A0),D7b
000575EA: bsr.w 0x00058d7e ; call 00058D7E FUN_00058d7e
000575EE: move.b (0x00fff0d6).l,(0xe,A3,D7w*0x1)
000575F6: rts
000575F8: jsr 0x00019558.l ; call 00019558 FUN_00019558
000575FE: bsr.w 0x00057610 ; call 00057610 FUN_00057610
00057602: move.l D1,D2
00057604: jsr 0x0001952c.l ; call 0001952C FUN_0001952c
0005760A: add.w D1w,D6w
0005760C: sub.w D2w,D6w
0005760E: rts
00057610: bsr.w 0x00057628 ; call 00057628 FUN_00057628
00057614: clr.l D7
00057616: move.b (A0),D7b
00057618: clr.l D1
0005761A: move.b (0x00fff0d6).l,D1b
00057620: jsr 0x00019538.l ; call 00019538 FUN_00019538
00057626: rts
00057628: bsr.w 0x0005763c ; call 0005763C FUN_0005763c
0005762C: clr.l D0
0005762E: move.b (0x00fff0d0).l,D0b
00057634: mulu.w #0x2,D0
00057638: adda.l D0,A0
0005763A: rts
0005763C: clr.l D0
0005763E: move.b (0x00fff0b6).l,D0b
00057644: subq.w #0x6,D0w
00057646: beq.w 0x0005764e
0005764A: move.w #0x1,D0w
0005764E: mulu.w #0x30,D0
00057652: lea (0x1ab276).l,A0 ; data 001ab276
00057658: adda.l D0,A0
0005765A: clr.l D0
0005765C: move.b (0x00fff0b8).l,D0b
00057662: mulu.w #0x10,D0
00057666: adda.l D0,A0
00057668: rts
0005766A: cmpi.b #0x4,(0x00fff0d0).l
00057672: bmi.w 0x00057680
00057676: move.b #-0x1,(0x00fff112).l
0005767E: rts
00057680: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
00057684: clr.l D1
00057686: lea (0x1ac8e0).l,A1 ; data 001ac8e0
0005768C: move.b (0x00fff0d0).l,D1b
00057692: lea (0xfffbbe).l,A0 ; data 00fffbbe
00057698: clr.l D0
0005769A: move.b (A0),D0b
0005769C: cmpi.w #0x2,D1w
000576A0: beq.w 0x00057702
000576A4: bgt.w 0x000576ce
000576A8: tst.w D1w
000576AA: beq.w 0x000576f4
000576AE: mulu.w #0x50,D0
000576B2: cmp.w (0x4,A0),D0w
000576B6: ble.w 0x000576be
000576BA: bra.w 0x0005770c
000576BE: movea.l #0xc40c,A2
000576C4: lea (0x1aa50e).l,A1 ; data 001aa50e
000576CA: bra.w 0x000576e4
000576CE: cmpi.b #0x3,(0x7,A0)
000576D4: bmi.w 0x0005770c
000576D8: movea.l #0xc40a,A2
000576DE: lea (0x1aa4dd).l,A1 ; data 001aa4dd
000576E4: move.w #0x6000,D1w
000576E8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000576EE: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
000576F4: mulu.w #0x28,D0
000576F8: cmp.w (0x2,A0),D0w
000576FC: ble.b 0x000576be
000576FE: bra.w 0x0005770c
00057702: mulu.w #0xa,D0
00057706: cmp.b (0x6,A0),D0b
0005770A: ble.b 0x000576be
0005770C: lsl.w #0x1,D1w
0005770E: adda.l D1,A1
00057710: move.w (A1),D1w ; data 001ac8e0
00057712: cmp.l (0x00fffbfe).l,D1
00057718: ble.w 0x00057738
0005771C: movea.l #0xc58e,A2
00057722: lea (0x1aa587).l,A1 ; data 001aa587
00057728: move.w #0x6000,D1w
0005772C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057732: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00057738: move.l D1,(0x00fff0be).l
0005773E: movea.l #0xc58e,A2
00057744: lea (0x1aa742).l,A1 ; data 001aa742
0005774A: move.w #0x6000,D1w
0005774E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057754: move.l (0x00fff0be).l,D1
0005775A: movea.l #0xc5b4,A2
00057760: move.w #0x6000,D2w
00057764: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005776A: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
0005776E: tst.b (0x00fff0dc).l
00057774: bne.w 0x000577b8
00057778: sub.l D1,(0x00fffbfe).l
0005777E: lea (0xfffbbe).l,A1 ; data 00fffbbe
00057784: tst.b (0x00fff0d0).l
0005778A: beq.w 0x000577a4
0005778E: cmpi.b #0x2,(0x00fff0d0).l
00057796: bmi.w 0x000577ac
0005779A: beq.w 0x000577b4
0005779E: addq.b #0x1,(0x7,A1)
000577A2: rts
000577A4: addi.w #0xa,(0x2,A1)
000577AA: rts
000577AC: addi.w #0x19,(0x4,A1)
000577B2: rts
000577B4: addq.b #0x5,(0x6,A1)
000577B8: rts
000577BA: cmpi.b #0x5,(0x00fff0d0).l
000577C2: bmi.w 0x000577d0
000577C6: move.b #-0x1,(0x00fff112).l
000577CE: rts
000577D0: tst.b (0x00fff0b3).l
000577D6: bmi.w 0x00057874
000577DA: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
000577DE: movea.l #0xc28c,A2
000577E4: lea (0x1aa4b3).l,A1 ; data 001aa4b3
000577EA: move.w #0x6000,D1w
000577EE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000577F4: clr.l D1
000577F6: move.b (0x00fff0d6).l,D1b
000577FC: movea.l #0xc2a6,A2
00057802: move.w #0x6000,D2w
00057806: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005780C: move.l (0x00fff0be).l,D1
00057812: movea.l #0xc32c,A2
00057818: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005781E: cmp.l (0x00fffbfe).l,D1
00057824: ble.w 0x00057844
00057828: movea.l #0xc58e,A2
0005782E: lea (0x1aa587).l,A1 ; data 001aa587
00057834: move.w #0x6000,D1w
00057838: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005783E: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00057844: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
00057848: tst.b (0x00fff0dc).l
0005784E: bne.w 0x00057874
00057852: sub.l D1,(0x00fffbfe).l
00057858: lea (0xfffbbe).l,A1 ; data 00fffbbe
0005785E: clr.l D0
00057860: move.b (0x00fff0d0).l,D0b
00057866: beq.w 0x0005786e
0005786A: addq.w #0x7,D0w
0005786C: adda.l D0,A1
0005786E: move.b (0x00fff0d6).l,(A1)
00057874: clr.b (0x00fff0b3).l
0005787A: rts
0005787C: clr.l D1
0005787E: move.b (0x00fff0d6).l,D1b
00057884: move.l D1,D2
00057886: mulu.w D1w,D2
00057888: tst.b (0x00fff0d0).l
0005788E: bgt.w 0x0005789e
00057892: mulu.w D1w,D2
00057894: exg D1,D2
00057896: mulu.w #0x19,D1
0005789A: bra.w 0x000578bc
0005789E: cmpi.b #0x3,(0x00fff0d0).l
000578A6: bpl.w 0x000578b2
000578AA: mulu.w #0x3,D2
000578AE: bra.w 0x000578b6
000578B2: mulu.w #0x2,D2
000578B6: exg D1,D2
000578B8: mulu.w #0x64,D1
000578BC: move.l D1,(0x00fff0be).l
000578C2: rts
000578C4: bsr.w 0x00057c5e ; call 00057C5E FUN_00057c5e
000578C8: clr.l D0
000578CA: move.b (0x00fff0d0).l,D0b
000578D0: movea.l (0x00fffc04).l,A1
000578D6: move.b (0x66,A1,D0w*0x1),D0b
000578DA: bgt.w 0x000578e8
000578DE: move.b #-0x1,(0x00fff112).l
000578E6: rts
000578E8: cmpi.b #0xb,D0b
000578EC: bmi.w 0x0005790c
000578F0: movea.l #0xc590,A2
000578F6: lea (0x1aa612).l,A1 ; data 001aa612
000578FC: move.w #0x6000,D1w
00057900: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057906: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
0005790C: move.b (0x00fff0d0).l,(0x00fff0c2).l
00057916: lea (0xd4540).l,A4 ; data 000d4540
0005791C: subq.w #0x1,D0w
0005791E: mulu.w #0x30,D0
00057922: movea.l #0xc516,A2
00057928: lea (0x0,A4,D0w*0x1),A1
0005792C: move.w #0x6000,D1w
00057930: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057936: movea.l #0xc614,A2
0005793C: lea (0x1aa63d).l,A1 ; data 001aa63d
00057942: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057948: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
0005794E: mulu.w #0x18,D0
00057952: adda.l D0,A0
00057954: clr.l D0
00057956: move.b (0x00fff0d0).l,D0b
0005795C: mulu.w #0x3,D0
00057960: adda.l D0,A0
00057962: tst.b (A0)
00057964: bpl.w 0x00057972
00057968: move.b #-0x1,(0x00fff112).l
00057970: rts
00057972: bsr.w 0x00057c5e ; call 00057C5E FUN_00057c5e
00057976: cmpi.b #0x45,(A0)
0005797A: bne.w 0x0005798c
0005797E: clr.l D1
00057980: move.b (0x2,A0),D1b
00057984: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
00057988: bra.w 0x000579c6
0005798C: movea.l (0x00fffc04).l,A1
00057992: moveq #0x7,D6
00057994: move.b (0x66,A1),D5b
00057998: beq.w 0x000579be
0005799C: addq.l #0x1,A1
0005799E: dbf D6w,0x00057994
000579A2: movea.l #0xc598,A2
000579A8: lea (0x1aa3c2).l,A1 ; data 001aa3c2
000579AE: move.w #0x6000,D1w
000579B2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000579B8: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
000579BE: subq.w #0x7,D6w
000579C0: neg.w D6w
000579C2: bsr.w 0x00057a44 ; call 00057A44 FUN_00057a44
000579C6: cmp.l (0x00fffbfe).l,D1
000579CC: ble.w 0x000579ec
000579D0: movea.l #0xc58e,A2
000579D6: lea (0x1aa587).l,A1 ; data 001aa587
000579DC: move.w #0x6000,D1w
000579E0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000579E6: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
000579EC: bsr.w 0x00057b8c ; call 00057B8C FUN_00057b8c
000579F0: tst.b (0x00fff0dc).l
000579F6: bne.w 0x00057a42
000579FA: movea.l (0x00fffc04).l,A1
00057A00: cmpi.b #0x45,(A0)
00057A04: bne.w 0x00057a2e
00057A08: addq.b #0x1,(0x4f,A1)
00057A0C: cmpi.b #0x15,(0x4f,A1)
00057A12: bmi.w 0x00057a3c
00057A16: move.b #0x14,(0x4f,A1)
00057A1C: bsr.w 0x00057c5e ; call 00057C5E FUN_00057c5e
00057A20: movea.l #0xc58e,A2
00057A26: lea (0x1aa54d).l,A1 ; data 001aa54d
00057A2C: bra.b 0x000579dc
00057A2E: move.b (A0),(0x66,A1,D6w*0x1)
00057A32: addq.b #0x1,(0x66,A1,D6w*0x1)
00057A36: move.b (0x1,A0),(0x6e,A1,D6w*0x1)
00057A3C: sub.l D1,(0x00fffbfe).l
00057A42: rts
00057A44: clr.l D0
00057A46: move.b (A0),D0b
00057A48: lea (0xd4540).l,A1 ; data 000d4540
00057A4E: mulu.w #0x30,D0
00057A52: clr.l D1
00057A54: move.w (0x22,A1,D0w*0x1),D1w ; data 000d4562
00057A58: clr.l D2
00057A5A: move.b (0x2,A0),D2b
00057A5E: beq.w 0x00057a6e
00057A62: mulu.w D2w,D1
00057A64: divu.w #0xa,D1
00057A68: andi.l #0xffff,D1
00057A6E: move.l D1,D7
00057A70: divu.w #0x64,D7
00057A74: cmpi.b #0x4,(0x00fff0b6).l
00057A7C: bne.w 0x00057a96
00057A80: btst.b #0x6,(0x00ff018f).l
00057A88: beq.w 0x00057ae8
00057A8C: mulu.w #0xa,D7
00057A90: sub.l D7,D1
00057A92: bra.w 0x00057ae8
00057A96: cmpi.b #0x5,(0x00fff0b6).l
00057A9E: bne.w 0x00057ab4
00057AA2: btst.b #0x7,(0x00ff018f).l
00057AAA: beq.w 0x00057ae8
00057AAE: mulu.w #0x19,D7
00057AB2: sub.l D7,D1
00057AB4: cmpi.b #0x2,(0x00fff0b6).l
00057ABC: bne.w 0x00057ace
00057AC0: btst.b #0x2,(0x00ff018f).l
00057AC8: beq.w 0x00057ae8
00057ACC: lsr.l #0x1,D1
00057ACE: cmpi.b #0x8,(0x00fff0b6).l
00057AD6: bne.w 0x00057ae8
00057ADA: btst.b #0x1,(0x00ff018f).l
00057AE2: beq.w 0x00057ae8
00057AE6: lsr.l #0x1,D1
00057AE8: bsr.w 0x00057b04 ; call 00057B04 FUN_00057b04
00057AEC: sub.l D3,D1
00057AEE: move.l D1,(0x00fff0be).l
00057AF4: rts
00057AF6: bsr.w 0x00057b04 ; call 00057B04 FUN_00057b04
00057AFA: add.l D3,D1
00057AFC: move.l D1,(0x00fff0be).l
00057B02: rts
00057B04: movea.l (0x00fffc04).l,A1
00057B0A: lea (0x1aac6e).l,A2 ; data 001aac6e
00057B10: clr.l D2
00057B12: move.b (0x00fff0b6).l,D2b
00057B18: mulu.w #0x2,D2
00057B1C: adda.l D2,A2
00057B1E: move.b (0x8c,A1),D2b
00057B22: sub.b (A2),D2b ; data 001aac6e
00057B24: ble.w 0x00057b3e
00057B28: cmpi.w #0xb,D2w
00057B2C: bmi.w 0x00057b34
00057B30: move.w #0xa,D2w
00057B34: move.l D1,D3
00057B36: lsr.l #0x5,D3
00057B38: mulu.w D2w,D3
00057B3A: ext.l D3
00057B3C: rts
00057B3E: clr.l D3
00057B40: rts
00057B42: movea.l (0x00fffc04).l,A1
00057B48: lea (0x1aac6e).l,A2 ; data 001aac6e
00057B4E: clr.l D2
00057B50: move.b (0x00fff0b6).l,D2b
00057B56: mulu.w #0x2,D2
00057B5A: adda.l D2,A2
00057B5C: move.b (0x8c,A1),D2b
00057B60: sub.b (A2),D2b ; data 001aac6e
00057B62: ble.w 0x00057b84
00057B66: cmpi.w #0xb,D2w
00057B6A: bmi.w 0x00057b72
00057B6E: move.w #0xa,D2w
00057B72: move.l D1,D3
00057B74: lsr.l #0x8,D3
00057B76: mulu.w D2w,D3
00057B78: ext.l D3
00057B7A: move.w #0x9,D7w
00057B7E: sub.l D3,D1
00057B80: dbf D7w,0x00057b7e
00057B84: move.l D1,(0x00fff0be).l
00057B8A: rts
00057B8C: movem.l {  A0 D6 D1 D0},-(SP)
00057B90: movea.l #0xc50e,A2
00057B96: lea (0x1aa715).l,A1 ; data 001aa715
00057B9C: move.w #0x6000,D1w
00057BA0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057BA6: movea.l #0xc620,A2
00057BAC: lea (0xe387e).l,A1 ; data 000e387e
00057BB2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057BB8: clr.w D2w
00057BBA: move.b #0x1,(0x00fff0dc).l
00057BC2: bra.w 0x00057be6
00057BC6: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00057BCC: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00057BD2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00057BD8: btst.b #0x8,(0x00ffdd22).l
00057BE0: beq.w 0x00057c04
00057BE4: moveq #-0x1,D2
00057BE6: move.w #0x25,D0w
00057BEA: jsr 0x000530de.l ; call 000530DE FUN_000530de
00057BF0: moveq #0x1,D3
00057BF2: move.l #0xc61c,D7
00057BF8: move.w #0x6000,D4w
00057BFC: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
00057C02: bra.b 0x00057bc6
00057C04: btst.b #0x9,(0x00ffdd22).l
00057C0C: beq.w 0x00057c14
00057C10: moveq #0x1,D2
00057C12: bra.b 0x00057be6
00057C14: btst.b #0xe,(0x00ffdd22).l
00057C1C: bne.w 0x00057c2a
00057C20: btst.b #0xc,(0x00ffdd22).l
00057C28: beq.b 0x00057bc6
00057C2A: move.w #0x24,D0w
00057C2E: jsr 0x000530de.l ; call 000530DE FUN_000530de
00057C34: movem.l (SP)+,{ D0 D1 D6 A0}
00057C38: rts
00057C3A: move.l #0xc604,D4
00057C40: moveq #0x2,D1
00057C42: bra.w 0x00057c66 ; call 00057C66 FUN_00057c66
00057C46: move.l #0xc184,D4
00057C4C: moveq #0xb,D1
00057C4E: bra.w 0x00057c66 ; call 00057C66 FUN_00057c66
00057C52: move.l #0xc904,D4
00057C58: moveq #0x1,D1
00057C5A: bra.w 0x00057c66 ; call 00057C66 FUN_00057c66
00057C5E: move.l #0xc484,D4
00057C64: moveq #0x5,D1
00057C66: lea (0xc1320).l,A1 ; data 000c1320
00057C6C: moveq #0x1e,D7
00057C6E: move.w #0x0,D2w
00057C72: jsr 0x000011c0.l ; call 000011C0 FUN_000011c0
00057C78: rts
00057C7A: bsr.b 0x00057c46 ; call 00057C46 FUN_00057c46
00057C7C: bsr.b 0x00057c52 ; call 00057C52 FUN_00057c52
00057C7E: clr.l D0
00057C80: move.b (0x00fff0b6).l,D0b
00057C86: tst.b (0x00fff0b7).l
00057C8C: bmi.w 0x00057cc0
00057C90: beq.w 0x00057d0e
00057C94: cmpi.b #0x1,(0x00fff0b7).l
00057C9C: beq.w 0x00057eee
00057CA0: cmpi.b #0x3,(0x00fff0b8).l
00057CA8: bmi.w 0x0005805c
00057CAC: beq.w 0x00058206
00057CB0: cmpi.b #0x5,(0x00fff0b8).l
00057CB8: beq.w 0x00058394
00057CBC: bra.w 0x00058316 ; call 00058316 FUN_00058316
00057CC0: tst.b (0x00fff0b8).l
00057CC6: beq.w 0x00057cee
00057CCA: cmpi.b #0x2,(0x00fff0b8).l
00057CD2: bmi.w 0x00057d02
00057CD6: beq.w 0x00057cf8
00057CDA: cmpi.b #0x4,(0x00fff0b8).l
00057CE2: bmi.w 0x000585fa
00057CE6: beq.w 0x00058540
00057CEA: bra.w 0x00058548
00057CEE: lea (0x1aac86).l,A0 ; data 001aac86
00057CF4: bra.w 0x00057d08
00057CF8: lea (0x1aad77).l,A0 ; data 001aad77
00057CFE: bra.w 0x00057d08
00057D02: lea (0x1aadbf).l,A0 ; data 001aadbf
00057D08: bsr.w 0x000586ca ; call 000586CA FUN_000586ca
00057D0C: rts
00057D0E: subq.w #0x3,D0w
00057D10: cmpi.b #0x1,(0x00fff0b8).l
00057D18: beq.w 0x00057d34
00057D1C: bmi.w 0x00057d3c
00057D20: cmpi.b #0x3,(0x00fff0b8).l
00057D28: beq.w 0x00058548
00057D2C: lea (0x1aaeb0).l,A0 ; data 001aaeb0
00057D32: bra.b 0x00057d08
00057D34: lea (0x1aaef8).l,A0 ; data 001aaef8
00057D3A: bra.b 0x00057d08
00057D3C: lea (0x1aaf40).l,A0 ; data 001aaf40
00057D42: mulu.w #0x20,D0
00057D46: adda.l D0,A0
00057D48: clr.l D0
00057D4A: move.b (0x00fff0d0).l,D0b
00057D50: mulu.w #0x4,D0
00057D54: adda.l D0,A0
00057D56: clr.l D7
00057D58: move.b (A0),D7b
00057D5A: bpl.w 0x00057d68
00057D5E: lea (0x1aa5b2).l,A1 ; data 001aa5b2
00057D64: bra.w 0x0005804a
00057D68: bsr.w 0x00057d7e ; call 00057D7E FUN_00057d7e
00057D6C: lea (0xd4ea0).l,A4 ; data 000d4ea0
00057D72: mulu.w #0x30,D7
00057D76: lea (0x0,A4,D7w*0x1),A1
00057D7A: bra.w 0x000585e8
00057D7E: move.l D7,-(SP) ; data Stack[-0x4]
00057D80: movea.l (0x00fffc04).l,A4
00057D86: cmpi.b #0x2,(0x9b,A4)
00057D8C: bmi.w 0x00057daa
00057D90: movea.l #0xc590,A2
00057D96: lea (0x1aa42c).l,A1 ; data 001aa42c
00057D9C: move.w #0x6000,D1w
00057DA0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057DA6: bra.w 0x00057eea
00057DAA: lea (0xd4ea0).l,A4 ; data 000d4ea0
00057DB0: mulu.w #0x30,D7
00057DB4: movea.l (0x1e,A4,D7w*0x1),A1 ; data 000d4ebe
00057DB8: movea.l #0xc508,A2
00057DBE: move.w #0x6000,D1w
00057DC2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057DC8: movea.l A4,A1
00057DCA: adda.l D7,A1
00057DCC: movea.l #0xc396,A2
00057DD2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057DD8: move.l (SP),D7 ; data Stack[-0x4]
00057DDA: move.b D7b,(0x00fff0ae).l
00057DE0: movea.l (0x00fff10e).l,A3
00057DE6: clr.l D6
00057DE8: move.w (A3)+,D3w
00057DEA: cmp.b (A3),D7b
00057DEC: bne.w 0x00057e1e
00057DF0: move.l A3,(0x00fff0c2).l
00057DF6: move.b (0x1,A3),D6b
00057DFA: movea.l (0x00fffc04).l,A1
00057E00: cmp.b (0x7f,A1),D6b
00057E04: bmi.w 0x00057e32
00057E08: lea (0x1aa85f).l,A1 ; data 001aa85f
00057E0E: movea.l #0xc690,A2
00057E14: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057E1A: bra.w 0x00057e9a
00057E1E: addq.l #0x2,A3
00057E20: dbf D3w,0x00057dea
00057E24: move.l A3,(0x00fff0c2).l
00057E2A: move.w (0x2,A0),D6w
00057E2E: bra.w 0x00057e38
00057E32: addq.w #0x1,D6w
00057E34: mulu.w (0x2,A0),D6
00057E38: move.l D6,(0x00fff0be).l
00057E3E: lea (0x1aa884).l,A1 ; data 001aa884
00057E44: movea.l #0xc70e,A2
00057E4A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057E50: lea (0x1aa89a).l,A1 ; data 001aa89a
00057E56: movea.l #0xc41c,A2
00057E5C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057E62: clr.l D1
00057E64: move.b (0x1,A3),D1b
00057E68: addq.w #0x1,D1w
00057E6A: movea.l #0xc428,A2
00057E70: move.w #0x6000,D2w
00057E74: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057E7A: clr.l D1
00057E7C: move.l (0x00fff0be).l,D1
00057E82: move.l A2,-(SP) ; data Stack[-0x8]
00057E84: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
00057E88: movea.l (SP)+,A2
00057E8A: move.w #0x6000,D2w
00057E8E: movea.l #0xc734,A2
00057E94: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057E9A: lea (0x1aaf40).l,A0 ; data 001aaf40
00057EA0: clr.l D0
00057EA2: move.b (0x00fff0b6).l,D0b
00057EA8: subq.w #0x3,D0w
00057EAA: mulu.w #0x20,D0
00057EAE: adda.l D0,A0
00057EB0: move.b (0x00fff0d0).l,D0b
00057EB6: mulu.w #0x4,D0
00057EBA: adda.l D0,A0
00057EBC: lea (0xd4ea0).l,A4 ; data 000d4ea0
00057EC2: move.w #-0x3de0,D4w
00057EC6: move.b (A0),D0b
00057EC8: cmpi.b #0x6,D0b
00057ECC: bpl.w 0x00057ed8
00057ED0: move.w #0x2000,D6w
00057ED4: bra.w 0x00057edc
00057ED8: move.w #0x6000,D6w
00057EDC: mulu.w #0x30,D0
00057EE0: movea.l (0x1a,A4,D0w*0x1),A1 ; data 000d4eba
00057EE4: jsr 0x00011190.l ; call 00011190 FUN_00011190
00057EEA: move.l (SP)+,D7
00057EEC: rts
00057EEE: lea (0xd177c).l,A1 ; data 000d177c
00057EF4: clr.l D0
00057EF6: move.b (0x00fff0d0).l,D0b
00057EFC: mulu.w #0xc,D0
00057F00: adda.l D0,A1
00057F02: move.w #-0x1dec,D4w
00057F06: move.w #0x2,D7w
00057F0A: move.w #0x1,D1w
00057F0E: move.w #0x2000,D2w
00057F12: jsr 0x00001232.l ; call 00001232 FUN_00001232
00057F18: lea (0x1ab4d8).l,A1 ; data 001ab4d8
00057F1E: movea.l #0xc714,A2
00057F24: move.w #0x6000,D1w
00057F28: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057F2E: lea (0x1ab517).l,A1 ; data 001ab517
00057F34: movea.l #0xc592,A2
00057F3A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00057F40: lea (0x1ab060).l,A0 ; data 001ab060
00057F46: clr.l D0
00057F48: move.b (0x00fff0b6).l,D0b
00057F4E: subq.w #0x7,D0w
00057F50: mulu.w #0x20,D0
00057F54: adda.l D0,A0
00057F56: move.b (0x00fff0d0).l,D0b
00057F5C: mulu.w #0x4,D0
00057F60: move.l (0x0,A0,D0w*0x1),D1
00057F64: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
00057F68: movea.l #0xc72c,A2
00057F6E: move.w #0x6000,D2w
00057F72: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057F78: movea.l (0x00fffc04).l,A0
00057F7E: clr.l D1
00057F80: move.b (0x7e,A0),D1b
00057F84: movea.l #0xc5ae,A2
00057F8A: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057F90: move.b (0x8e,A0),D1b
00057F94: movea.l #0xc5b2,A2
00057F9A: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057FA0: move.w (0x90,A0),D7w
00057FA4: lea (0x1aa8a0).l,A1 ; data 001aa8a0
00057FAA: clr.l D0
00057FAC: move.b (0x00fff0d0).l,D0b
00057FB2: cmpi.w #0x6,D0w
00057FB6: bmi.w 0x00057fce
00057FBA: addq.w #0x5,D0w
00057FBC: cmpi.w #0xc,D0w
00057FC0: bmi.w 0x00057fce
00057FC4: btst.l #0xc,D7
00057FC8: beq.w 0x00057fce
00057FCC: addq.w #0x1,D0w
00057FCE: adda.l D0,A1
00057FD0: move.b (A1),D0b ; data 001aa8a0
00057FD2: move.b (0x00fff0d0).l,D6b
00057FD8: cmpi.b #0x3,D6b
00057FDC: bne.w 0x00057fea
00057FE0: btst.l #0x2,D7
00057FE4: beq.w 0x00057fea
00057FE8: subq.w #0x1,D0w
00057FEA: divu.w #0xa,D0
00057FEE: move.w D0w,D1w
00057FF0: movea.l #0xc62e,A2
00057FF6: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00057FFC: swap D0
00057FFE: move.w D0w,D1w
00058000: movea.l #0xc632,A2
00058006: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005800C: lea (0x1ac8c0).l,A1 ; data 001ac8c0
00058012: clr.l D0
00058014: move.b (0x00fff0d0).l,D0b
0005801A: mulu.w #0x4,D0
0005801E: adda.l D0,A1
00058020: movea.l (A1),A1 ; data 001ac8c0
00058022: movea.l #0xc386,A2
00058028: move.w #0x6000,D1w
0005802C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058032: lea (0x1aafa0).l,A1 ; data 001aafa0
00058038: mulu.w #0x6,D0
0005803C: adda.l D0,A1
0005803E: movea.l #0xc220,A2
00058044: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005804A: move.w #0x6000,D1w
0005804E: movea.l #0xc918,A2
00058054: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005805A: rts
0005805C: bsr.w 0x00057628 ; call 00057628 FUN_00057628
00058060: clr.l D0
00058062: move.b (A0),D0b
00058064: lea (0xfffbbe).l,A5 ; data 00fffbbe
0005806A: move.b (0xe,A5,D0w*0x1),(0x00fff0d6).l
00058072: addq.b #0x1,(0x00fff0d6).l
00058078: bsr.w 0x00057610 ; call 00057610 FUN_00057610
0005807C: clr.l D2
0005807E: move.b (0x00fff0d6).l,D2b
00058084: mulu.w D2w,D1
00058086: mulu.w #0x1e,D1
0005808A: cmpi.l #0x10000,D1
00058090: bpl.w 0x0005809c
00058094: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
00058098: bra.w 0x000580a0
0005809C: bsr.w 0x00057b42 ; call 00057B42 FUN_00057b42
000580A0: lea (0x1ab3f0).l,A1 ; data 001ab3f0
000580A6: lea (0x1ab378).l,A3 ; data 001ab378
000580AC: clr.l D0
000580AE: move.b (A0),D0b
000580B0: bpl.w 0x000580be
000580B4: lea (0x1aa5b2).l,A1 ; data 001aa5b2
000580BA: bra.w 0x000580ec
000580BE: move.l D0,D7
000580C0: mulu.w #0xa,D0
000580C4: adda.l D0,A3
000580C6: move.w #0x6000,D1w
000580CA: movea.l #0xc99a,A2
000580D0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000580D6: movea.l A3,A1
000580D8: movea.l #0xc214,A2
000580DE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000580E4: move.l A1,-(SP) ; data Stack[-0x4]
000580E6: bsr.w 0x000580fe ; call 000580FE FUN_000580fe
000580EA: movea.l (SP)+,A1
000580EC: move.w #0x6000,D1w
000580F0: movea.l #0xc91a,A2
000580F6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000580FC: rts
000580FE: move.l D7,D0
00058100: lea (0xe31f2).l,A5 ; data 000e31f2
00058106: lsl.w #0x2,D7w
00058108: movea.l (0x0,A5,D7w*0x1),A1 ; data 000e31f2
0005810C: lea (0xffb740).l,A5 ; data 00ffb740
00058112: clr.w D1w
00058114: jsr 0x0000c134.l ; call 0000C134 normalize_source_text_line
0005811A: move.b #-0x1,(A5)
0005811E: move.w #0x1e,D1w
00058122: move.w #0x6000,D2w
00058126: movea.l #0xc306,A2
0005812C: jsr 0x0000c168.l ; call 0000C168 FUN_0000c168
00058132: move.w #0x6000,D1w
00058136: cmpi.b #0x9,(0x00fff0d6).l
0005813E: bmi.w 0x00058158
00058142: lea (0x1ab4e7).l,A1 ; data 001ab4e7
00058148: movea.l #0xc68a,A2
0005814E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058154: bra.w 0x000581e0
00058158: lea (0x1ab4b4).l,A1 ; data 001ab4b4
0005815E: movea.l #0xc22a,A2
00058164: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005816A: lea (0x1ab4ba).l,A1 ; data 001ab4ba
00058170: movea.l #0xc686,A2
00058176: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005817C: lea (0x1ab4d8).l,A1 ; data 001ab4d8
00058182: movea.l #0xc714,A2
00058188: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005818E: clr.l D1
00058190: move.b (0x00fff0d6).l,D1b
00058196: movea.l #0xc236,A2
0005819C: move.w #0x6000,D2w
000581A0: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000581A6: move.l D0,D7
000581A8: bsr.w 0x000575f8 ; call 000575F8 FUN_000575f8
000581AC: add.w D2w,D6w
000581AE: move.l D2,D1
000581B0: movea.l #0xc692,A2
000581B6: move.w #0x6000,D2w
000581BA: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000581C0: move.l D6,D1
000581C2: movea.l #0xc6b8,A2
000581C8: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000581CE: move.l (0x00fff0be).l,D1
000581D4: movea.l #0xc72c,A2
000581DA: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000581E0: lea (0x1ac90c).l,A1 ; data 001ac90c
000581E6: mulu.w #0xc,D7
000581EA: adda.l D7,A1
000581EC: move.l #0xc18a,D4
000581F2: move.w #0x2,D7w
000581F6: move.w #0x1,D1w
000581FA: move.w #0x2000,D2w
000581FE: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058204: rts
00058206: clr.l D0
00058208: move.b (0x00fff0b6).l,D0b
0005820E: subq.w #0x6,D0w
00058210: beq.w 0x00058218
00058214: move.w #0x1,D0w
00058218: mulu.w #0x70,D0
0005821C: lea (0x1ab196).l,A0 ; data 001ab196
00058222: adda.l D0,A0
00058224: clr.l D0
00058226: move.b (0x00fff0d0).l,D0b
0005822C: mulu.w #0xe,D0
00058230: adda.l D0,A0
00058232: lea (0x1ab118).l,A1 ; data 001ab118
00058238: clr.l D0
0005823A: move.b (A0),D0b ; data 001ab206
0005823C: bpl.w 0x0005824a
00058240: lea (0x1aa5b2).l,A1 ; data 001aa5b2
00058246: bra.w 0x0005804a
0005824A: mulu.w #0x18,D0
0005824E: adda.l D0,A1
00058250: move.l A1,-(SP) ; data Stack[-0x4]
00058252: bsr.w 0x0005825c ; call 0005825C FUN_0005825c
00058256: movea.l (SP)+,A1
00058258: bra.w 0x0005804a
0005825C: lea (0x1ab3fa).l,A1 ; data 001ab3fa
00058262: movea.l #0xc206,A2
00058268: move.w #0x6000,D1w
0005826C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058272: clr.l D1
00058274: move.w #0x6000,D2w
00058278: move.b (0x6,A0),D1b
0005827C: movea.l #0xc52a,A2
00058282: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00058288: move.b (0x7,A0),D1b
0005828C: movea.l #0xc420,A2
00058292: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00058298: move.b (0xc,A0),D1b
0005829C: movea.l #0xc330,A2
000582A2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000582A8: move.b (0xd,A0),D1b
000582AC: movea.l #0xc43e,A2
000582B2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000582B8: move.w (0x8,A0),D1w
000582BC: movea.l #0xc230,A2
000582C2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000582C8: move.w (0xa,A0),D1w
000582CC: movea.l #0xc2b0,A2
000582D2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000582D8: move.b (0x00fffbcb).l,D1b
000582DE: cmp.b (A0),D1b
000582E0: bmi.w 0x000582ea
000582E4: bsr.w 0x00057c3a ; call 00057C3A FUN_00057c3a
000582E8: rts
000582EA: move.l (0x2,A0),D1
000582EE: cmpi.l #0x10000,D1
000582F4: bpl.w 0x00058300
000582F8: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
000582FC: bra.w 0x00058304
00058300: bsr.w 0x00057b42 ; call 00057B42 FUN_00057b42
00058304: move.w #0x6000,D2w
00058308: movea.l #0xc738,A2
0005830E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00058314: rts
00058316: clr.l D0
00058318: move.b (0x00fff0d0).l,D0b
0005831E: cmpi.w #0x4,D0w
00058322: bmi.w 0x00058330
00058326: lea (0x1aa5b2).l,A1 ; data 001aa5b2
0005832C: bra.w 0x000580ec
00058330: lea (0x1ac89c).l,A1 ; data 001ac89c
00058336: mulu.w #0x4,D0
0005833A: adda.l D0,A1
0005833C: movea.l (A1),A1 ; data 001ac89c
0005833E: movea.l #0xc386,A2
00058344: move.w #0x6000,D1w
00058348: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005834E: lea (0x1ab2d6).l,A1 ; data 001ab2d6
00058354: mulu.w #0x5,D0
00058358: adda.l D0,A1
0005835A: movea.l #0xc222,A2
00058360: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058366: move.l A1,-(SP) ; data Stack[-0x4]
00058368: lea (0x1ac9f0).l,A1 ; data 001ac9f0
0005836E: divu.w #0x14,D0
00058372: mulu.w #0xc,D0
00058376: adda.l D0,A1
00058378: move.w #-0x3dea,D4w
0005837C: move.w #0x2,D7w
00058380: move.w #0x1,D1w
00058384: move.w #0x2000,D2w
00058388: jsr 0x00001232.l ; call 00001232 FUN_00001232
0005838E: movea.l (SP)+,A1
00058390: bra.w 0x000580ec
00058394: cmpi.b #0x5,(0x00fff0d0).l
0005839C: bpl.w 0x0005853e
000583A0: clr.b (0x00fff0b3).l
000583A6: move.w #0x6000,D1w
000583AA: lea (0xfffbbe).l,A1 ; data 00fffbbe
000583B0: clr.l D0
000583B2: move.b (0x00fff0d0).l,D0b
000583B8: beq.w 0x000583e4
000583BC: subq.w #0x1,D0w
000583BE: move.b (0x8,A1,D0w*0x1),(0x00fff0d6).l
000583C6: clr.l D6
000583C8: move.b (A1),D6b
000583CA: cmp.b (0x00fff0d6).l,D6b
000583D0: bgt.w 0x00058424
000583D4: lea (0x1ab918).l,A1 ; data 001ab918
000583DA: movea.l #0xc60c,A2
000583E0: bra.w 0x00058412
000583E4: clr.l D0
000583E6: move.b (0xd,A1),D0b
000583EA: addq.b #0x1,D0b
000583EC: lea (0x1ab190).l,A0 ; data 001ab190
000583F2: move.b (0x0,A0,D0w*0x1),D0b ; data 001ab190
000583F6: cmp.b (A1),D0b
000583F8: beq.w 0x00058406
000583FC: move.b D0b,(0x00fff0d6).l
00058402: bra.w 0x0005842a
00058406: lea (0x1ab906).l,A1 ; data 001ab906
0005840C: movea.l #0xc612,A2
00058412: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058418: move.b #-0x1,(0x00fff0b3).l
00058420: bra.w 0x0005842e
00058424: addq.b #0x1,(0x00fff0d6).l
0005842A: bsr.w 0x0005787c ; call 0005787C FUN_0005787c
0005842E: move.w #0x6000,D1w
00058432: move.b (0x00fff0d0).l,D0b
00058438: bne.w 0x00058464
0005843C: lea (0x1ab8f9).l,A1 ; data 001ab8f9
00058442: movea.l #0xc222,A2
00058448: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005844E: lea (0x1ab326).l,A1 ; data 001ab326
00058454: movea.l #0xc914,A2
0005845A: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058460: bra.w 0x000584a2
00058464: cmpi.w #0x5,D0w
00058468: bmi.w 0x00058476
0005846C: lea (0x1aa5b2).l,A1 ; data 001aa5b2
00058472: bra.w 0x000580ec
00058476: lea (0x1ab36e).l,A1 ; data 001ab36e
0005847C: movea.l #0xc99a,A2
00058482: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058488: lea (0x1ab346).l,A1 ; data 001ab346
0005848E: subq.w #0x1,D0w
00058490: mulu.w #0xa,D0
00058494: adda.l D0,A1
00058496: movea.l #0xc222,A2
0005849C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000584A2: move.l A1,-(SP) ; data Stack[-0x4]
000584A4: lea (0x1ac8ac).l,A1 ; data 001ac8ac
000584AA: move.b (0x00fff0d0).l,D0b
000584B0: mulu.w #0x4,D0
000584B4: adda.l D0,A1
000584B6: movea.l (A1),A1 ; data 001ac8ac
000584B8: movea.l #0xc386,A2
000584BE: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000584C4: tst.b (0x00fff0b3).l
000584CA: bmi.w 0x0005850a
000584CE: lea (0x1aa4b3).l,A1 ; data 001aa4b3
000584D4: movea.l #0xc60c,A2
000584DA: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000584E0: clr.l D1
000584E2: move.b (0x00fff0d6).l,D1b
000584E8: movea.l #0xc626,A2
000584EE: move.w #0x6000,D2w
000584F2: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000584F8: move.l (0x00fff0be).l,D1
000584FE: movea.l #0xc6ac,A2
00058504: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005850A: lea (0x1ac9b4).l,A1 ; data 001ac9b4
00058510: move.b (0x00fff0d0).l,D0b
00058516: mulu.w #0xc,D0
0005851A: adda.l D0,A1
0005851C: move.w #-0x3dea,D4w
00058520: move.w #0x2,D7w
00058524: move.w #0x1,D1w
00058528: move.w #0x2000,D2w
0005852C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058532: movea.l (SP)+,A1
00058534: tst.b (0x00fff0d0).l
0005853A: bne.w 0x000580ec
0005853E: rts
00058540: move.b #-0x1,(0x00fff0b1).l
00058548: movea.l (0x00fffc04).l,A0
0005854E: clr.l D0
00058550: move.b (0x00fff0d0).l,D0b
00058556: adda.l D0,A0
00058558: clr.l D0
0005855A: move.b (0x66,A0),D0b
0005855E: subq.w #0x1,D0w
00058560: bmi.w 0x00058574
00058564: cmpi.w #0x28,D0w
00058568: bmi.w 0x0005857e
0005856C: cmpi.w #0x2b,D0w
00058570: bpl.w 0x0005857e
00058574: lea (0x1aa5b2).l,A1 ; data 001aa5b2
0005857A: bra.w 0x0005804a
0005857E: jsr 0x00013228.l ; call 00013228 FUN_00013228
00058584: move.l D0,D7
00058586: bsr.w 0x0005878e ; call 0005878E FUN_0005878e
0005858A: move.l D7,D0
0005858C: tst.b (0x00fff0b1).l
00058592: bne.w 0x000585ac
00058596: lea (0x1aa72b).l,A1 ; data 001aa72b
0005859C: movea.l #0xc70e,A2
000585A2: move.w #0x6000,D1w
000585A6: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000585AC: lea (0xd4540).l,A1 ; data 000d4540
000585B2: mulu.w #0x30,D0
000585B6: adda.l D0,A1
000585B8: move.l A1,-(SP) ; data Stack[-0x4]
000585BA: tst.b (0x00fff0b1).l
000585C0: bne.w 0x000585e0
000585C4: clr.l D1
000585C6: move.w (0x22,A1),D1w ; data 000d4562
000585CA: lsr.w #0x1,D1w
000585CC: bsr.w 0x00057af6 ; call 00057AF6 FUN_00057af6
000585D0: movea.l #0xc736,A2
000585D6: move.w #0x6000,D2w
000585DA: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
000585E0: clr.b (0x00fff0b1).l
000585E6: movea.l (SP)+,A1
000585E8: move.w #0x6000,D1w
000585EC: movea.l #0xc916,A2
000585F2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000585F8: rts
000585FA: cmpi.b #0x6,(0x00fff0d0).l
00058602: bmi.w 0x00058610
00058606: lea (0x1aa5b2).l,A1 ; data 001aa5b2
0005860C: bra.w 0x0005804a
00058610: lea (0x1ab4d8).l,A1 ; data 001ab4d8
00058616: move.w #0x6000,D1w
0005861A: movea.l #0xc714,A2
00058620: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058626: clr.l D0
00058628: move.b (0x00fff0b6).l,D0b
0005862E: mulu.w #0xc,D0
00058632: lea (0x1aae8c).l,A0 ; data 001aae8c
00058638: adda.l D0,A0
0005863A: move.b (0x00fff0d0).l,D0b
00058640: mulu.w #0x2,D0
00058644: clr.l D1
00058646: move.w (0x0,A0,D0w*0x1),D1w
0005864A: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
0005864E: movea.l #0xc72c,A2
00058654: move.w #0x6000,D2w
00058658: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
0005865E: lea (0xd25dc).l,A1 ; data 000d25dc
00058664: move.b (0x00fff0d0).l,D0b
0005866A: mulu.w #0x10,D0
0005866E: adda.l D0,A1
00058670: move.w #-0x3dec,D4w
00058674: move.w #0x3,D7w
00058678: move.w #0x1,D1w
0005867C: move.w #0x2000,D2w
00058680: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058686: lea (0x1ac884).l,A1 ; data 001ac884
0005868C: move.b (0x00fff0d0).l,D0b
00058692: lsl.w #0x2,D0w
00058694: adda.l D0,A1
00058696: movea.l (A1),A1 ; data 001ac884
00058698: move.w #0x6000,D1w
0005869C: movea.l #0xc386,A2
000586A2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000586A8: lea (0x1aae07).l,A1 ; data 001aae07
000586AE: move.b (0x00fff0d0).l,D0b
000586B4: mulu.w #0x16,D0
000586B8: adda.l D0,A1
000586BA: movea.l #0xc220,A2
000586C0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000586C6: bra.w 0x0005804a
000586CA: mulu.w #0x18,D0
000586CE: adda.l D0,A0
000586D0: clr.l D0
000586D2: move.b (0x00fff0d0).l,D0b
000586D8: mulu.w #0x3,D0
000586DC: adda.l D0,A0
000586DE: tst.b (A0)
000586E0: bpl.w 0x000586ee
000586E4: lea (0x1aa5b2).l,A1 ; data 001aa5b2
000586EA: bra.w 0x000585e8
000586EE: cmpi.b #0x45,(A0)
000586F2: bne.w 0x0005874e
000586F6: lea (0xce28e).l,A1 ; data 000ce28e
000586FC: move.w #0x2,D7w
00058700: move.w #0x1,D1w
00058704: move.w #0x2000,D2w
00058708: move.l #0xc220,D4
0005870E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058714: lea (0x1aace2).l,A1 ; data 001aace2
0005871A: move.w #0x6000,D1w
0005871E: movea.l #0xc386,A2
00058724: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005872A: clr.l D1
0005872C: move.b (0x2,A0),D1b
00058730: bsr.w 0x00057a6e ; call 00057A6E FUN_00057a6e
00058734: movea.l #0xc730,A2
0005873A: move.w #0x6000,D2w
0005873E: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00058744: lea (0x1aacce).l,A1 ; data 001aacce
0005874A: bra.w 0x000580ec
0005874E: jsr 0x000130c0.l ; call 000130C0 FUN_000130c0
00058754: clr.l D0
00058756: move.b (A0),D0b
00058758: bsr.w 0x0005878e ; call 0005878E FUN_0005878e
0005875C: lea (0x1ab4d8).l,A1 ; data 001ab4d8
00058762: movea.l #0xc714,A2
00058768: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005876E: bsr.w 0x00057a44 ; call 00057A44 FUN_00057a44
00058772: movea.l #0xc72c,A2
00058778: move.w #0x6000,D2w
0005877C: jsr 0x0000c216.l ; call 0000C216 FUN_0000c216
00058782: lea (0xd4540).l,A1 ; data 000d4540
00058788: adda.l D0,A1
0005878A: bra.w 0x000585e8
0005878E: lea (0x1ac7fc).l,A1 ; data 001ac7fc
00058794: cmpi.w #0x19,D0w
00058798: bmi.w 0x000587c2
0005879C: cmpi.w #0x25,D0w
000587A0: bmi.w 0x000587c0
000587A4: subi.w #0xd,D0w
000587A8: cmpi.w #0x1b,D0w
000587AC: bmi.w 0x000587c2
000587B0: cmpi.w #0x1e,D0w
000587B4: bpl.w 0x000587ba
000587B8: rts
000587BA: subq.w #0x3,D0w
000587BC: bra.w 0x000587c2
000587C0: moveq #0x18,D0
000587C2: lsl.w #0x2,D0w
000587C4: adda.l D0,A1
000587C6: movea.l (A1),A1 ; data 001ac7fc
000587C8: move.w #0x6000,D1w
000587CC: movea.l #0xc506,A2
000587D2: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000587D8: rts
000587DA: move.b (0x00fff0d0).l,(0x00fff0b8).l
000587E4: clr.l D0
000587E6: move.b (0x00fff0b6).l,D0b
000587EC: tst.b (0x00fff0b7).l
000587F2: bmi.w 0x00058826
000587F6: beq.w 0x0005887e
000587FA: cmpi.b #0x1,(0x00fff0b7).l
00058802: beq.w 0x0005895c
00058806: cmpi.b #0x3,(0x00fff0d0).l
0005880E: bmi.w 0x000589e8
00058812: beq.w 0x0005898a
00058816: cmpi.b #0x5,(0x00fff0d0).l
0005881E: beq.w 0x00058918
00058822: bra.w 0x00058906
00058826: tst.b (0x00fff0d0).l
0005882C: beq.w 0x00058852
00058830: cmpi.b #0x2,(0x00fff0d0).l
00058838: bmi.w 0x0005886c
0005883C: beq.w 0x00058862
00058840: cmpi.b #0x3,(0x00fff0d0).l
00058848: beq.w 0x0005885c
0005884C: bsr.w 0x00058aa4 ; call 00058AA4 FUN_00058aa4
00058850: rts
00058852: lea (0x1aac86).l,A0 ; data 001aac86
00058858: bra.w 0x00058872
0005885C: bsr.w 0x00058b1e ; call 00058B1E FUN_00058b1e
00058860: rts
00058862: lea (0x1aad77).l,A0 ; data 001aad77
00058868: bra.w 0x00058872
0005886C: lea (0x1aadbf).l,A0 ; data 001aadbf
00058872: mulu.w #0x18,D0
00058876: adda.l D0,A0
00058878: bsr.w 0x00058a2e ; call 00058A2E FUN_00058a2e
0005887C: rts
0005887E: subq.w #0x3,D0w
00058880: cmpi.b #0x1,(0x00fff0b8).l
00058888: beq.w 0x000588a2
0005888C: bmi.w 0x000588aa
00058890: cmpi.b #0x3,(0x00fff0b8).l
00058898: beq.b 0x0005884c
0005889A: lea (0x1aaeb0).l,A0 ; data 001aaeb0
000588A0: bra.b 0x00058872
000588A2: lea (0x1aaef8).l,A0 ; data 001aaef8
000588A8: bra.b 0x00058872
000588AA: lea (0x1aaf40).l,A0 ; data 001aaf40
000588B0: mulu.w #0x20,D0
000588B4: adda.l D0,A0
000588B6: lea (0xd4ea0).l,A4 ; data 000d4ea0
000588BC: move.w #-0x157c,D4w
000588C0: move.w #0x7,D3w
000588C4: clr.l D0
000588C6: move.b (A0),D0b ; data 001aaf40
000588C8: bpl.w 0x000588d6
000588CC: lea (0x34f8).l,A1 ; data 000034f8
000588D2: bra.w 0x000588f2
000588D6: cmpi.b #0x6,D0b
000588DA: bpl.w 0x000588e6
000588DE: move.w #0x2000,D6w
000588E2: bra.w 0x000588ea
000588E6: move.w #0x6000,D6w
000588EA: mulu.w #0x30,D0
000588EE: movea.l (0x1a,A4,D0w*0x1),A1 ; data 000d4eba
000588F2: jsr 0x00011190.l ; call 00011190 FUN_00011190
000588F8: addq.l #0x8,D4
000588FA: adda.l #0x4,A0
00058900: dbf D3w,0x000588c4
00058904: rts
00058906: lea (0x1ac9f0).l,A1 ; data 001ac9f0
0005890C: move.w #0x3,D3w
00058910: move.w #0x3,D5w
00058914: bra.w 0x00058926
00058918: lea (0x1ac9b4).l,A1 ; data 001ac9b4
0005891E: move.w #0x4,D3w
00058922: move.w #0x2,D5w
00058926: move.w #-0x157c,D4w
0005892A: move.w #0x2,D7w
0005892E: move.w #0x1,D1w
00058932: move.w #0x2000,D2w
00058936: jsr 0x00001232.l ; call 00001232 FUN_00001232
0005893C: addq.l #0x8,D4
0005893E: adda.l #0xc,A1
00058944: dbf D3w,0x00058936
00058948: lea (0xcd2ba).l,A1 ; data 000cd2ba
0005894E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058954: addq.l #0x8,D4
00058956: dbf D5w,0x0005894e
0005895A: rts
0005895C: move.w #-0x157c,D4w
00058960: move.w #0x2,D7w
00058964: move.w #0x1,D1w
00058968: move.w #0x2000,D2w
0005896C: lea (0xd177c).l,A1 ; data 000d177c
00058972: move.w #0x7,D3w
00058976: jsr 0x00001232.l ; call 00001232 FUN_00001232
0005897C: addq.l #0x8,D4
0005897E: adda.l #0xc,A1
00058984: dbf D3w,0x00058976
00058988: rts
0005898A: clr.l D0
0005898C: move.b (0x00fff0b6).l,D0b
00058992: subq.w #0x6,D0w
00058994: beq.w 0x0005899c
00058998: move.w #0x1,D0w
0005899C: mulu.w #0x70,D0
000589A0: lea (0x1ab196).l,A0 ; data 001ab196
000589A6: adda.l D0,A0
000589A8: move.w #-0x157c,D4w
000589AC: move.w #0x2,D7w
000589B0: move.w #0x1,D1w
000589B4: move.w #0x6000,D2w
000589B8: move.w #0x7,D3w
000589BC: clr.l D0
000589BE: move.b (A0),D0b
000589C0: bpl.w 0x000589ce
000589C4: lea (0xcd2ba).l,A1 ; data 000cd2ba
000589CA: bra.w 0x000589d4
000589CE: lea (0xce29a).l,A1 ; data 000ce29a
000589D4: jsr 0x00001232.l ; call 00001232 FUN_00001232
000589DA: addq.l #0x8,D4
000589DC: adda.l #0xe,A0
000589E2: dbf D3w,0x000589bc
000589E6: rts
000589E8: bsr.w 0x0005763c ; call 0005763C FUN_0005763c
000589EC: move.w #-0x157c,D4w
000589F0: move.w #0x2,D7w
000589F4: move.w #0x1,D1w
000589F8: move.w #0x2000,D2w
000589FC: move.w #0x7,D3w
00058A00: clr.l D0
00058A02: move.b (A0),D0b
00058A04: bpl.w 0x00058a12
00058A08: lea (0xcd2ba).l,A1 ; data 000cd2ba
00058A0E: bra.w 0x00058a1e
00058A12: lea (0x1ac90c).l,A1 ; data 001ac90c
00058A18: mulu.w #0xc,D0
00058A1C: adda.l D0,A1
00058A1E: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058A24: addq.l #0x8,D4
00058A26: addq.l #0x2,A0
00058A28: dbf D3w,0x00058a00
00058A2C: rts
00058A2E: move.w #-0x157c,D4w
00058A32: moveq #0x7,D3
00058A34: lea (0xd4540).l,A4 ; data 000d4540
00058A3A: clr.l D0
00058A3C: move.b (A0),D0b
00058A3E: bpl.w 0x00058a4c
00058A42: lea (0x34f8).l,A1 ; data 000034f8
00058A48: bra.w 0x00058a8c
00058A4C: cmpi.b #0xf,D0b
00058A50: bpl.w 0x00058a5c
00058A54: move.w #0x2000,D6w
00058A58: bra.w 0x00058a60
00058A5C: move.w #0x6000,D6w
00058A60: cmpi.b #0x45,D0b
00058A64: bne.w 0x00058a84
00058A68: lea (0xce28e).l,A1 ; data 000ce28e
00058A6E: move.w #0x2,D7w
00058A72: move.w #0x1,D1w
00058A76: move.w #0x2000,D2w
00058A7A: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058A80: bra.w 0x00058a96
00058A84: mulu.w #0x30,D0
00058A88: movea.l (0x1a,A4,D0w*0x1),A1 ; data 000d455a
00058A8C: move.w #0x6000,D6w
00058A90: jsr 0x00011190.l ; call 00011190 FUN_00011190
00058A96: addq.l #0x8,D4
00058A98: adda.l #0x3,A0
00058A9E: dbf D3w,0x00058a3a
00058AA2: rts
00058AA4: move.w #-0x157c,D4w
00058AA8: moveq #0x7,D3
00058AAA: lea (0xd4540).l,A4 ; data 000d4540
00058AB0: movea.l (0x00fffc04).l,A0
00058AB6: clr.l D0
00058AB8: move.b (0x66,A0),D0b
00058ABC: bne.w 0x00058aca
00058AC0: lea (0x34f8).l,A1 ; data 000034f8
00058AC6: bra.w 0x00058ae8
00058ACA: subq.w #0x1,D0w
00058ACC: cmpi.b #0xf,D0b
00058AD0: bpl.w 0x00058adc
00058AD4: move.w #0x2000,D6w
00058AD8: bra.w 0x00058ae0
00058ADC: move.w #0x6000,D6w
00058AE0: mulu.w #0x30,D0
00058AE4: movea.l (0x1a,A4,D0w*0x1),A1 ; data 000d455a
00058AE8: move.w #0x6000,D6w
00058AEC: jsr 0x00011190.l ; call 00011190 FUN_00011190
00058AF2: addq.l #0x8,D4
00058AF4: addq.l #0x1,A0
00058AF6: dbf D3w,0x00058ab6
00058AFA: cmpi.b #0x4,(0x00fff0b8).l
00058B02: bne.w 0x00058b1c
00058B06: movea.l #0xc594,A2
00058B0C: lea (0x1aa5f4).l,A1 ; data 001aa5f4
00058B12: move.w #0x6000,D1w
00058B16: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058B1C: rts
00058B1E: move.w #-0x157c,D4w
00058B22: lea (0xd297c).l,A1 ; data 000d297c
00058B28: move.w #0x2,D7w
00058B2C: move.w #0x1,D1w
00058B30: move.w #0x4000,D2w
00058B34: moveq #0x5,D3
00058B36: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058B3C: addq.l #0x8,D4
00058B3E: adda.l #0xc,A1
00058B44: dbf D3w,0x00058b36
00058B48: lea (0xcd2ba).l,A1 ; data 000cd2ba
00058B4E: moveq #0x1,D3
00058B50: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058B56: addq.l #0x8,D4
00058B58: dbf D3w,0x00058b50
00058B5C: rts
00058B5E: bsr.w 0x00057c46 ; call 00057C46 FUN_00057c46
00058B62: tst.b (0x00fff0b7).l
00058B68: bmi.w 0x00058b86
00058B6C: beq.w 0x00058b98
00058B70: cmpi.b #0x1,(0x00fff0b7).l
00058B78: beq.w 0x00058bb4
00058B7C: lea (0xce1c6).l,A1 ; data 000ce1c6
00058B82: bra.w 0x00058b8c
00058B86: lea (0xce17e).l,A1 ; data 000ce17e
00058B8C: move.w #-0x17f0,D4w
00058B90: move.w #0x11,D7w
00058B94: bra.w 0x00058ba6
00058B98: lea (0xce14e).l,A1 ; data 000ce14e
00058B9E: move.w #-0x17ea,D4w
00058BA2: move.w #0xb,D7w
00058BA6: move.w #0x1,D1w
00058BAA: move.w #0x0,D2w
00058BAE: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058BB4: rts
00058BB6: tst.b (0x00fff0b7).l
00058BBC: bmi.w 0x00058be0
00058BC0: beq.w 0x00058bf4
00058BC4: cmpi.b #0x1,(0x00fff0b7).l
00058BCC: beq.w 0x00058c52
00058BD0: lea (0x1aa9ab).l,A3 ; data 001aa9ab
00058BD6: lea (0xce25e).l,A1 ; data 000ce25e
00058BDC: bra.w 0x00058bec
00058BE0: lea (0x1aa8af).l,A3 ; data 001aa8af
00058BE6: lea (0xce22e).l,A1 ; data 000ce22e
00058BEC: move.w #-0x17ee,D4w
00058BF0: bra.w 0x00058c04
00058BF4: lea (0x1aaaa7).l,A3 ; data 001aaaa7
00058BFA: lea (0xce20e).l,A1 ; data 000ce20e
00058C00: move.w #-0x17e8,D4w
00058C04: clr.l D0
00058C06: move.b (0x00fff0d0).l,D0b
00058C0C: mulu.w #0x8,D0
00058C10: adda.l D0,A1
00058C12: clr.l D0
00058C14: move.b (0x00fff0d0).l,D0b
00058C1A: mulu.w #0x6,D0
00058C1E: add.l D0,D4
00058C20: move.w #0x1,D7w
00058C24: move.w #0x1,D1w
00058C28: move.w #0x0,D2w
00058C2C: jsr 0x00001232.l ; call 00001232 FUN_00001232
00058C32: movea.l A3,A1
00058C34: clr.l D0
00058C36: move.b (0x00fff0d0).l,D0b
00058C3C: mulu.w #0x2a,D0
00058C40: adda.l D0,A1
00058C42: movea.l #0xc910,A2
00058C48: move.w #0x6000,D1w
00058C4C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00058C52: rts
00058C54: tst.b (0x00fff0d1).l
00058C5A: beq.w 0x00058c88
00058C5E: add.b D0b,(0x00fff0d0).l
00058C64: bpl.w 0x00058c72
00058C68: move.b #0x0,(0x00fff0d0).l
00058C70: rts
00058C72: cmpi.b #0x8,(0x00fff0d0).l
00058C7A: bmi.w 0x00058cc4
00058C7E: move.b #0x7,(0x00fff0d0).l
00058C86: rts
00058C88: tst.b (0x00fff0b7).l
00058C8E: beq.w 0x00058c9a
00058C92: move.w #0x5,D1w
00058C96: bra.w 0x00058c9e
00058C9A: move.w #0x3,D1w
00058C9E: add.b D0b,(0x00fff0d0).l
00058CA4: bpl.w 0x00058cb2
00058CA8: move.b #0x0,(0x00fff0d0).l
00058CB0: rts
00058CB2: cmp.b (0x00fff0d0).l,D1b
00058CB8: bpl.w 0x00058cc4
00058CBC: move.b D1b,(0x00fff0d0).l
00058CC2: rts
00058CC4: tst.b (0x00fff0d1).l
00058CCA: beq.w 0x00058cd4
00058CCE: bsr.w 0x00058d32 ; call 00058D32 FUN_00058d32
00058CD2: rts
00058CD4: bsr.w 0x00058b5e ; call 00058B5E FUN_00058b5e
00058CD8: bsr.w 0x00058bb6 ; call 00058BB6 FUN_00058bb6
00058CDC: rts
00058CDE: add.b D0b,(0x00fff0d1).l
00058CE4: bpl.w 0x00058cf2
00058CE8: move.b #0x0,(0x00fff0d1).l
00058CF0: rts
00058CF2: cmpi.b #0x2,(0x00fff0d1).l
00058CFA: bmi.w 0x00058d08
00058CFE: move.b #0x1,(0x00fff0d1).l
00058D06: rts
00058D08: clr.b (0x00fff0d0).l
00058D0E: tst.b (0x00fff0d1).l
00058D14: beq.w 0x00058d22
00058D18: bsr.w 0x00058b5e ; call 00058B5E FUN_00058b5e
00058D1C: bsr.w 0x00058d32 ; call 00058D32 FUN_00058d32
00058D20: rts
00058D22: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
00058D28: bsr.w 0x00058b5e ; call 00058B5E FUN_00058b5e
00058D2C: bsr.w 0x00058bb6 ; call 00058BB6 FUN_00058bb6
00058D30: rts
00058D32: clr.l D1
00058D34: move.b (0x00fff0d0).l,D1b
00058D3A: mulu.w #0x20,D1
00058D3E: clr.l D2
00058D40: move.w #0x2,D5w
00058D44: move.w #0x1,(0x00ffdd02).l
00058D4C: lea (0xffd986).l,A3 ; data 00ffd986
00058D52: lea (0xd2a00).l,A2 ; data 000d2a00
00058D58: move.w #0x4000,D6w
00058D5C: jsr 0x00001378.l ; call 00001378 FUN_00001378
00058D62: subq.w #0x8,A3
00058D64: clr.b (0x3,A3)
00058D68: move.w #0x1,(0x00ffd980).l
00058D70: bset.b #0x1,(0x00ffef14).l
00058D78: bsr.w 0x00057c7a ; call 00057C7A FUN_00057c7a
00058D7C: rts
00058D7E: lea (0xfffbbe).l,A3 ; data 00fffbbe
00058D84: move.l #-0x1,(0x1a,A3)
00058D8C: move.b #-0x1,(0x1e,A3)
00058D92: rts
00059092: lea (0xd29d6).l,A0 ; data 000d29d6
00059098: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
0005909E: lea (0x18ac96).l,A0 ; data 0018ac96
000590A4: jsr 0x000566e2.l ; call 000566E2 FUN_000566e2
000590AA: jsr 0x00010b5a.l ; call 00010B5A FUN_00010b5a
000590B0: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
000590B6: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
000590BC: lea (0x1b6498).l,A1 ; data 001b6498
000590C2: movea.l #0xc39a,A2
000590C8: move.w #0x2000,D1w
000590CC: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000590D2: clr.b (0x00fff0dc).l
000590D8: clr.w D2w
000590DA: moveq #0x1,D3
000590DC: move.w #-0x3ae6,D7w
000590E0: move.w #0x2000,D4w
000590E4: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
000590EA: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
000590F0: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000590F6: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000590FC: btst.b #0x8,(0x00ffdd22).l
00059104: beq.w 0x0005910e
00059108: moveq #-0x1,D2
0005910A: bra.w 0x0005911c
0005910E: btst.b #0x9,(0x00ffdd22).l
00059116: beq.w 0x0005912c
0005911A: moveq #0x1,D2
0005911C: moveq #0x1,D3
0005911E: move.w #-0x3ae6,D7w
00059122: move.w #0x2000,D4w
00059126: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
0005912C: btst.b #0xe,(0x00ffdd22).l
00059134: beq.w 0x00059154
00059138: tst.b (0x00fff0dc).l
0005913E: beq.w 0x00059248
00059142: bsr.w 0x00059172 ; call 00059172 FUN_00059172
00059146: tst.b (0x00fff0dc).l
0005914C: bpl.w 0x00059170
00059150: bra.w 0x000590b6
00059154: btst.b #0xc,(0x00ffdd22).l
0005915C: beq.w 0x00059162
00059160: bra.b 0x00059138
00059162: btst.b #0xd,(0x00ffdd22).l
0005916A: beq.w 0x000590ea
0005916E: bra.b 0x00059138
00059170: rts
00059172: lea (0x1b6518).l,A1 ; data 001b6518
00059178: movea.l #0xc726,A2
0005917E: move.w #0x2000,D1w
00059182: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00059188: clr.b (0x00fff0dc).l
0005918E: clr.w D2w
00059190: bra.w 0x000591c6
00059194: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005919A: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
000591A0: jsr 0x0000195c.l ; call 0000195C sample_controller_input
000591A6: btst.b #0x8,(0x00ffdd22).l
000591AE: beq.w 0x000591b8
000591B2: moveq #-0x1,D2
000591B4: bra.w 0x000591c6
000591B8: btst.b #0x9,(0x00ffdd22).l
000591C0: beq.w 0x000591d8
000591C4: moveq #0x1,D2
000591C6: moveq #0x1,D3
000591C8: move.w #-0x38de,D7w
000591CC: move.w #0x2000,D4w
000591D0: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
000591D6: bra.b 0x00059194
000591D8: btst.b #0xe,(0x00ffdd22).l
000591E0: beq.w 0x000591e8
000591E4: bra.w 0x00059202
000591E8: btst.b #0xc,(0x00ffdd22).l
000591F0: beq.w 0x000591f8
000591F4: bra.w 0x00059202
000591F8: btst.b #0xd,(0x00ffdd22).l
00059200: beq.b 0x00059194
00059202: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
00059208: jsr 0x00059360.l ; call 00059360 FUN_00059360
0005920E: bcs.w 0x00059224
00059212: move.w #-0x1,(0x00ffe108).l
0005921A: lea (0x1b6577).l,A1 ; data 001b6577
00059220: bra.w 0x00059232
00059224: move.b #-0x1,(0x00fff0dc).l
0005922C: lea (0x1b6527).l,A1 ; data 001b6527
00059232: movea.l #0xc492,A2
00059238: move.w #0x2000,D1w
0005923C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00059242: jmp 0x00000d94.l ; call 00000D94 FUN_00000d94
00059248: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
0005924E: lea (0x1b64cf).l,A1 ; data 001b64cf
00059254: movea.l #0xc3a6,A2
0005925A: move.w #0x2000,D1w
0005925E: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
00059264: clr.b (0x00fff0dc).l
0005926A: clr.w D2w
0005926C: bra.w 0x000592a2
00059270: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00059276: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0005927C: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059282: btst.b #0x8,(0x00ffdd22).l
0005928A: beq.w 0x00059294
0005928E: moveq #-0x1,D2
00059290: bra.w 0x000592a2
00059294: btst.b #0x9,(0x00ffdd22).l
0005929C: beq.w 0x000592dc
000592A0: moveq #0x1,D2
000592A2: moveq #0x2,D3
000592A4: move.w #-0x3a5a,D7w
000592A8: move.w #0x2000,D4w
000592AC: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
000592B2: move.b (0x00fff0dc).l,(0x00fff0ae).l
000592BC: lea (0x7291a).l,A0 ; data 0007291a
000592C2: jsr 0x0001f38a.l ; call 0001F38A FUN_0001f38a
000592C8: lea (0x7291a).l,A0 ; data 0007291a
000592CE: move.l #0xc410,D4
000592D4: jsr 0x0001f34a.l ; call 0001F34A FUN_0001f34a
000592DA: bra.b 0x00059270
000592DC: btst.b #0xe,(0x00ffdd22).l
000592E4: beq.w 0x000592ec
000592E8: bra.w 0x00059308
000592EC: btst.b #0xc,(0x00ffdd22).l
000592F4: beq.w 0x000592fc
000592F8: bra.w 0x00059308
000592FC: btst.b #0xd,(0x00ffdd22).l
00059304: beq.w 0x00059270
00059308: clr.l D0
0005930A: move.b (0x00fff0dc).l,D0b
00059310: jsr 0x000042b0.l ; call 000042B0 FUN_000042b0
00059316: rts
00059318: lea (0xfff1e8).l,A1 ; data 00fff1e8
0005931E: lea (0x59972).l,A0 ; data 00059972
00059324: move.w #0x13,D7w
00059328: move.b (A0)+,(A1)+
0005932A: dbf D7w,0x00059328
0005932E: lea (0x7291a).l,A0 ; data 0007291a
00059334: bsr.w 0x000597ee ; call 000597EE FUN_000597ee
00059338: lea (0x72c1a).l,A0 ; data 00072c1a
0005933E: move.w #0xb,D7w
00059342: bsr.w 0x000597ee ; call 000597EE FUN_000597ee
00059346: adda.w #0x100,A0
0005934A: dbf D7w,0x00059342
0005934E: move.b #-0x1,(0x00fffc0c).l
00059356: move.b #-0x1,(0x00fffc0d).l
0005935E: rts
00059360: bsr.w 0x00059456 ; call 00059456 FUN_00059456
00059364: bcs.w 0x00059370
00059368: bsr.w 0x00059372 ; call 00059372 FUN_00059372
0005936C: andi #0x1e,CCR
00059370: rts
00059372: move.l #0x8000000,D5
00059378: tst.b (0x00fff0dc).l
0005937E: beq.w 0x00059388
00059382: move.l #0x18001000,D5
00059388: lea (0xfff1e8).l,A1 ; data 00fff1e8
0005938E: move.w D5w,D0w
00059390: move.w #0x7ff,D7w
00059394: bsr.w 0x000593fc ; call 000593FC FUN_000593fc
00059398: bcc.w 0x000593cc
0005939C: lea (0xfff1e8).l,A1 ; data 00fff1e8
000593A2: move.l D5,D0
000593A4: swap D0
000593A6: move.w #0x7ff,D7w
000593AA: bsr.w 0x000593fc ; call 000593FC FUN_000593fc
000593AE: bcs.w 0x000593c6
000593B2: move.l #0x8000000,D0
000593B8: move.w #0x7ff,D7w
000593BC: jsr 0x00001502.l ; call 00001502 copy_even_bytes
000593C2: bra.w 0x000593f6
000593C6: ori #0x1,CCR
000593CA: rts
000593CC: lea (0xfff1e8).l,A1 ; data 00fff1e8
000593D2: move.l D5,D0
000593D4: swap D0
000593D6: move.w #0x7ff,D7w
000593DA: bsr.w 0x0005961c ; call 0005961C FUN_0005961c
000593DE: bcc.w 0x000593f6
000593E2: move.l #0x800,D0
000593E8: move.w #0x7ff,D7w
000593EC: jsr 0x00001502.l ; call 00001502 copy_even_bytes
000593F2: bra.w 0x000593f6
000593F6: andi #0x1e,CCR
000593FA: rts
000593FC: jsr 0x000014ce.l ; call 000014CE extract_even_bytes
00059402: movea.l #0xfff1ec,A1
00059408: move.b (A1)+,D0b
0005940A: cmpi.b #0x47,D0b
0005940E: bne.w 0x00059442
00059412: move.b (A1)+,D0b
00059414: cmpi.b #0x41,D0b
00059418: bne.w 0x00059442
0005941C: move.b (A1)+,D0b
0005941E: cmpi.b #0x4d,D0b
00059422: bne.w 0x00059442
00059426: move.b (A1)+,D0b
00059428: cmpi.b #0x45,D0b
0005942C: bne.w 0x00059442
00059430: movea.l #0xfff1fc,A1
00059436: bsr.w 0x00059786 ; call 00059786 FUN_00059786
0005943A: move.l (A1),D0
0005943C: cmp.l D6,D0
0005943E: beq.w 0x00059450
00059442: nop
00059444: nop
00059446: nop
00059448: nop
0005944A: ori #0x1,CCR
0005944E: rts
00059450: andi #0x1e,CCR
00059454: rts
00059456: move.l #0x8000000,D5
0005945C: tst.b (0x00fff0dc).l
00059462: beq.w 0x0005946c
00059466: move.l #0x18001000,D5
0005946C: lea (0xffb740).l,A1 ; data 00ffb740
00059472: move.w D5w,D0w
00059474: move.w #0x7ff,D7w
00059478: bsr.w 0x0005961c ; call 0005961C FUN_0005961c
0005947C: bcc.w 0x000594b0
00059480: lea (0xffb740).l,A1 ; data 00ffb740
00059486: move.l D5,D0
00059488: swap D0
0005948A: move.w #0x7ff,D7w
0005948E: bsr.w 0x0005961c ; call 0005961C FUN_0005961c
00059492: bcs.w 0x000594aa
00059496: move.l #0x8000000,D0
0005949C: move.w #0x7ff,D7w
000594A0: jsr 0x00001502.l ; call 00001502 copy_even_bytes
000594A6: bra.w 0x000594da
000594AA: ori #0x1,CCR
000594AE: rts
000594B0: lea (0xffb740).l,A1 ; data 00ffb740
000594B6: move.l D5,D0
000594B8: swap D0
000594BA: move.w #0x7ff,D7w
000594BE: bsr.w 0x0005961c ; call 0005961C FUN_0005961c
000594C2: bcc.w 0x000594da
000594C6: move.l #0x800,D0
000594CC: move.w #0x7ff,D7w
000594D0: jsr 0x00001502.l ; call 00001502 copy_even_bytes
000594D6: bra.w 0x000594da
000594DA: andi #0x1e,CCR
000594DE: rts
000594E0: clr.w (0x00ffdf40).l
000594E6: lea (0xfff9fa).l,A0 ; data 00fff9fa
000594EC: lea (0xfff1e8).l,A1 ; data 00fff1e8
000594F2: move.w #0x3d6,D0w
000594F6: adda.w D0w,A1
000594F8: move.w #0x247,D7w
000594FC: move.b (A1)+,(A0)+
000594FE: dbf D7w,0x000594fc
00059502: move.w (A1)+,(0x00ffddc4).l
00059508: move.l A1,-(SP) ; data 00fff5c2
0005950A: jsr 0x00003074.l ; call 00003074 FUN_00003074
00059510: move.b #0xf,(0x00ff0110).l
00059518: move.b (0x00fffc0c).l,D0b
0005951E: bmi.w 0x0005952e
00059522: clr.w D7w
00059524: ext.w D0w
00059526: subq.w #0x1,D0w
00059528: jsr 0x000042f2.l ; call 000042F2 FUN_000042f2
0005952E: move.b (0x00fffc0d).l,D0b
00059534: bmi.w 0x00059544
00059538: clr.w D7w
0005953A: ext.w D0w
0005953C: subq.w #0x1,D0w
0005953E: jsr 0x0000431c.l ; call 0000431C FUN_0000431c
00059544: movea.l (SP)+,A1
00059546: move.w (A1)+,(0x00ffddc6).l
0005954C: move.w (A1)+,(0x00ffddc8).l
00059552: move.w (A1)+,(0x00ff0100).l
00059558: move.w (A1)+,(0x00ff0104).l
0005955E: move.w (A1)+,(0x00ff0200).l
00059564: move.w (A1)+,(0x00ff0204).l
0005956A: move.w (A1)+,(0x00ff0300).l
00059570: move.w (A1)+,(0x00ff0304).l
00059576: bsr.w 0x0005966e ; call 0005966E FUN_0005966e
0005957A: move.b (0x00fffc0c).l,D0b
00059580: bmi.w 0x00059590
00059584: move.b #0xd,(0x00ff0210).l
0005958C: bra.w 0x00059596
00059590: clr.b (0x00ff0210).l
00059596: move.b (0x00fffc0d).l,D0b
0005959C: bmi.w 0x000595ac
000595A0: move.b #0xd,(0x00ff0310).l
000595A8: bra.w 0x000595b2
000595AC: clr.b (0x00ff0310).l
000595B2: bset.b #0x1,(0x00ff0110).l
000595BA: bclr.b #0x1,(0x00ff0210).l
000595C2: bclr.b #0x1,(0x00ff0310).l
000595CA: move.l #0xff0100,(0x00fffc04).l
000595D4: move.l #0x720fa,D0
000595DA: cmpi.b #0x2,(0x00ff019b).l
000595E2: beq.w 0x000595fe
000595E6: move.l #0x7223a,D0
000595EC: cmpi.b #0x3,(0x00ff019b).l
000595F4: beq.w 0x000595fe
000595F8: move.l #0x7219a,D0 ; data 0007219a
000595FE: move.l D0,(0x00ff0150).l
00059604: tst.l (0x00fffc3c).l
0005960A: beq.w 0x0005961a
0005960E: lea (0x1d657c).l,A6 ; data 001d657c
00059614: jsr 0x0001e4ac.l ; call 0001E4AC FUN_0001e4ac
0005961A: rts
0005961C: jsr 0x000014ce.l ; call 000014CE extract_even_bytes
00059622: movea.l #0xffb744,A1
00059628: move.b (A1)+,D0b
0005962A: cmpi.b #0x47,D0b
0005962E: bne.w 0x00059662
00059632: move.b (A1)+,D0b
00059634: cmpi.b #0x41,D0b
00059638: bne.w 0x00059662
0005963C: move.b (A1)+,D0b
0005963E: cmpi.b #0x4d,D0b
00059642: bne.w 0x00059662
00059646: move.b (A1)+,D0b
00059648: cmpi.b #0x45,D0b
0005964C: bne.w 0x00059662
00059650: movea.l #0xffb754,A1
00059656: bsr.w 0x00059786 ; call 00059786 FUN_00059786
0005965A: move.l (A1),D0
0005965C: cmp.l D6,D0
0005965E: beq.w 0x00059668
00059662: ori #0x1,CCR
00059666: rts
00059668: andi #0x1e,CCR
0005966C: rts
0005966E: movea.l #0xfff1fc,A0
00059674: lea (0xff0100).l,A1 ; data 00ff0100
0005967A: bsr.w 0x000598b0 ; call 000598B0 FUN_000598b0
0005967E: clr.l D0
00059680: move.b (0x00fffc0c).l,D0b
00059686: bmi.w 0x000596a0
0005968A: movea.l #0xfff1fc,A0
00059690: mulu.w #0x4a,D0
00059694: adda.w D0w,A0
00059696: movea.l #0xff0200,A1
0005969C: bsr.w 0x000598b0 ; call 000598B0 FUN_000598b0
000596A0: clr.l D0
000596A2: move.b (0x00fffc0d).l,D0b
000596A8: bmi.w 0x000596c2
000596AC: movea.l #0xfff1fc,A0
000596B2: mulu.w #0x4a,D0
000596B6: adda.w D0w,A0
000596B8: movea.l #0xff0300,A1
000596BE: bsr.w 0x000598b0 ; call 000598B0 FUN_000598b0
000596C2: rts
00059786: clr.l D6
00059788: move.w #0x61b,D7w
0005978C: clr.l D0
0005978E: move.b (A1)+,D0b
00059790: add.l D0,D6
00059792: dbf D7w,0x0005978c
00059796: rts
00059798: movea.l #0xfff1fc,A1
0005979E: lea (0xff0100).l,A0 ; data 00ff0100
000597A4: bsr.w 0x000597ee ; call 000597EE FUN_000597ee
000597A8: clr.l D0
000597AA: move.b (0x00fffc0c).l,D0b
000597B0: bmi.w 0x000597ca
000597B4: movea.l #0xfff1fc,A1
000597BA: mulu.w #0x4a,D0
000597BE: adda.w D0w,A1
000597C0: movea.l #0xff0200,A0
000597C6: bsr.w 0x000597ee ; call 000597EE FUN_000597ee
000597CA: clr.l D0
000597CC: move.b (0x00fffc0d).l,D0b
000597D2: bmi.w 0x000597ec
000597D6: movea.l #0xfff1fc,A1
000597DC: mulu.w #0x4a,D0
000597E0: adda.w D0w,A1
000597E2: movea.l #0xff0300,A0
000597E8: bsr.w 0x000597ee ; call 000597EE FUN_000597ee
000597EC: rts
000597EE: move.w (0xe,A0),(A1)+
000597F2: move.w (0x2c,A0),(A1)+
000597F6: move.b (0x4c,A0),(A1)+
000597FA: move.b (0x4d,A0),(A1)+
000597FE: move.b (0x4e,A0),(A1)+
00059802: move.b (0x4f,A0),(A1)+
00059806: move.b (0x56,A0),(A1)+
0005980A: move.b (0x57,A0),(A1)+
0005980E: move.l (0x58,A0),(A1)+
00059812: move.l (0x5c,A0),(A1)+
00059816: move.l (0x60,A0),(A1)+
0005981A: move.w (0x64,A0),(A1)+
0005981E: move.l (0x66,A0),(A1)+
00059822: move.l (0x6a,A0),(A1)+
00059826: move.l (0x6e,A0),(A1)+
0005982A: move.l (0x72,A0),(A1)+
0005982E: move.b (0x76,A0),(A1)+
00059832: move.b (0x77,A0),(A1)+
00059836: move.b (0x78,A0),(A1)+
0005983A: move.b (0x79,A0),(A1)+
0005983E: move.b (0x7a,A0),(A1)+
00059842: move.b (0x7b,A0),(A1)+
00059846: move.b (0x7c,A0),(A1)+
0005984A: move.b (0x7d,A0),(A1)+
0005984E: move.b (0x7e,A0),(A1)+
00059852: move.b (0x7f,A0),(A1)+
00059856: move.b (0x80,A0),(A1)+
0005985A: move.b (0x81,A0),(A1)+
0005985E: move.b (0x82,A0),(A1)+
00059862: move.b (0x83,A0),(A1)+
00059866: move.b (0x84,A0),(A1)+
0005986A: move.b (0x85,A0),(A1)+
0005986E: move.b (0x86,A0),(A1)+
00059872: move.b (0x87,A0),(A1)+
00059876: move.b (0x88,A0),(A1)+
0005987A: move.b (0x89,A0),(A1)+
0005987E: move.b (0x8a,A0),(A1)+
00059882: move.b (0x8b,A0),(A1)+
00059886: move.b (0x8c,A0),(A1)+
0005988A: move.b (0x8d,A0),(A1)+
0005988E: move.b (0x8e,A0),(A1)+
00059892: move.b (0x9a,A0),(A1)+
00059896: move.b (0x9b,A0),(A1)+
0005989A: move.b (0x9c,A0),(A1)+
0005989E: move.w (0x90,A0),(A1)+
000598A2: move.w (0xa0,A0),(A1)+
000598A6: move.b (0x8f,A0),(A1)+
000598AA: move.b (0x9d,A0),(A1)+
000598AE: rts
000598B0: move.w (A0)+,(0xe,A1)
000598B4: move.w (A0)+,(0x2c,A1)
000598B8: move.b (A0)+,(0x4c,A1)
000598BC: move.b (A0)+,(0x4d,A1)
000598C0: move.b (A0)+,(0x4e,A1)
000598C4: move.b (A0)+,(0x4f,A1)
000598C8: move.b (A0)+,(0x56,A1)
000598CC: move.b (A0)+,(0x57,A1)
000598D0: move.l (A0)+,(0x58,A1)
000598D4: move.l (A0)+,(0x5c,A1)
000598D8: move.l (A0)+,(0x60,A1)
000598DC: move.w (A0)+,(0x64,A1)
000598E0: move.l (A0)+,(0x66,A1)
000598E4: move.l (A0)+,(0x6a,A1)
000598E8: move.l (A0)+,(0x6e,A1)
000598EC: move.l (A0)+,(0x72,A1)
000598F0: move.b (A0)+,(0x76,A1)
000598F4: move.b (A0)+,(0x77,A1)
000598F8: move.b (A0)+,(0x78,A1)
000598FC: move.b (A0)+,(0x79,A1)
00059900: move.b (A0)+,(0x7a,A1)
00059904: move.b (A0)+,(0x7b,A1)
00059908: move.b (A0)+,(0x7c,A1)
0005990C: move.b (A0)+,(0x7d,A1)
00059910: move.b (A0)+,(0x7e,A1)
00059914: move.b (A0)+,(0x7f,A1)
00059918: move.b (A0)+,(0x80,A1)
0005991C: move.b (A0)+,(0x81,A1)
00059920: move.b (A0)+,(0x82,A1)
00059924: move.b (A0)+,(0x83,A1)
00059928: move.b (A0)+,(0x84,A1)
0005992C: move.b (A0)+,(0x85,A1)
00059930: move.b (A0)+,(0x86,A1)
00059934: move.b (A0)+,(0x87,A1)
00059938: move.b (A0)+,(0x88,A1)
0005993C: move.b (A0)+,(0x89,A1)
00059940: move.b (A0)+,(0x8a,A1)
00059944: move.b (A0)+,(0x8b,A1)
00059948: move.b (A0)+,(0x8c,A1)
0005994C: move.b (A0)+,(0x8d,A1)
00059950: move.b (A0)+,(0x8e,A1)
00059954: move.b (A0)+,(0x9a,A1)
00059958: move.b (A0)+,(0x9b,A1)
0005995C: move.b (A0)+,(0x9c,A1)
00059960: move.w (A0)+,(0x90,A1)
00059964: move.w (A0)+,(0xa0,A1)
00059968: move.b (A0)+,(0x8f,A1)
0005996C: move.b (A0)+,(0x9d,A1)
00059970: rts
00059986: moveq #0x5,D1
00059988: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
0005998E: move.l D0,D2
00059990: moveq #0x5,D1
00059992: jsr 0x00000d3a.l ; call 00000D3A FUN_00000d3a
00059998: add.l D2,D0
0005999A: lea (0x3312).l,A5 ; data 00003312
000599A0: move.b (0x00fffbf8).l,D1b
000599A6: move.b (0x0,A5,D1w*0x1),D1b ; data 00003312
000599AA: move.l D1,D2
000599AC: mulu.w #0x9,D2
000599B0: add.w D0w,D2w
000599B2: lea (0x1d7174).l,A5 ; data 001d7174
000599B8: mulu.w #0x4,D0
000599BC: mulu.w #0x24,D1
000599C0: adda.l D0,A5
000599C2: adda.l D1,A5
000599C4: movea.l (A5),A6
000599C6: lea (0x1d70e4).l,A5 ; data 001d70e4
000599CC: clr.l D0
000599CE: move.b (0x0,A5,D2w*0x1),D0b ; data 001d70e4
000599D2: beq.w 0x000599d8
000599D6: movea.l (A6),A6
000599D8: rts
000599DA: jsr 0x0000c3e6.l ; call 0000C3E6 FUN_0000c3e6
000599E0: lea (0x1d90e0).l,A1 ; data 001d90e0
000599E6: movea.l #0xc216,A2
000599EC: move.w #0x6000,D1w
000599F0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
000599F6: clr.b (0x00fff0dc).l
000599FC: clr.w D2w
000599FE: moveq #0x7,D3
00059A00: move.w #-0x3c6e,D7w
00059A04: move.w #0x6000,D4w
00059A08: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
00059A0E: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00059A14: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00059A1A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059A20: jsr 0x0000e660.l ; call 0000E660 FUN_0000e660
00059A26: btst.b #0x8,(0x00ffdd22).l
00059A2E: beq.w 0x00059a38
00059A32: moveq #-0x1,D2
00059A34: bra.w 0x00059a46
00059A38: btst.b #0x9,(0x00ffdd22).l
00059A40: beq.w 0x00059a58
00059A44: moveq #0x1,D2
00059A46: moveq #0x7,D3
00059A48: move.w #-0x3c6e,D7w
00059A4C: move.w #0x6000,D4w
00059A50: jsr 0x0000c376.l ; call 0000C376 move_wrapped_menu_cursor
00059A56: bra.b 0x00059a0e
00059A58: btst.b #0xe,(0x00ffdd22).l
00059A60: beq.w 0x00059a78
00059A64: clr.b (0x00fff101).l
00059A6A: lea (0x1d90c0).l,A0 ; data 001d90c0
00059A70: jsr 0x0000c436.l ; call 0000C436 FUN_0000c436
00059A76: rts
00059A78: btst.b #0xd,(0x00ffdd22).l
00059A80: beq.w 0x00059a8c
00059A84: jsr 0x0000391a.l ; call 0000391A FUN_0000391a
00059A8A: bra.b 0x00059a0e
00059A8C: btst.b #0xf,(0x00ffdd22).l
00059A94: beq.w 0x00059a0e
00059A98: rts
00059B28: move.l #0x3d090,(0x00fffbfe).l
00059B32: rts
00059BCC: lea (0xff0100).l,A0 ; data 00ff0100
00059BD2: moveq #0x2,D5
00059BD4: bclr.b #0x6,(0x10,A0)
00059BDA: bclr.b #0x4,(0x10,A0)
00059BE0: moveq #0xa,D1
00059BE2: jsr 0x00003674.l ; call 00003674 FUN_00003674
00059BE8: jsr 0x000037ec.l ; call 000037EC FUN_000037ec
00059BEE: adda.l #0x100,A0
00059BF4: dbf D5w,0x00059bd4
00059BF8: move.b #0xa,(0x00ff014c).l
00059C00: move.b #0xa,(0x00ff019c).l
00059C08: move.b #0xa,(0x00ff024c).l
00059C10: move.b #0xa,(0x00ff029c).l
00059C18: move.b #0xa,(0x00ff034c).l
00059C20: move.b #0xa,(0x00ff039c).l
00059C28: rts
00059C34: add.b D0b,(0x00ff018d).l
00059C3A: add.b D0b,(0x00ff028d).l
00059C40: add.b D0b,(0x00ff038d).l
00059C46: rts
00059C48: move.l #0x1c7f,(0x00fffc2c).l
00059C52: rts
00059C54: move.b #-0x1,(0x00ffe168).l
00059C5C: rts
00059C5E: move.b #-0x1,(0x00ffe18b).l
00059C66: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059C6C: clr.w (0x00ffddc2).l
00059C72: move.w #0x10,D0w
00059C76: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
00059C7C: jsr 0x0005af62.l ; call 0005AF62 FUN_0005af62
00059C82: tst.w (0x00ffddc2).l
00059C88: bne.w 0x00059cfe
00059C8C: jsr 0x00053126.l ; call 00053126 FUN_00053126
00059C92: move.w #0x11,D0w
00059C96: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
00059C9C: clr.w (0x00ffddc2).l
00059CA2: jsr 0x00059d7c.l ; call 00059D7C FUN_00059d7c
00059CA8: tst.w (0x00ffddc2).l
00059CAE: bne.w 0x00059cfe
00059CB2: jsr 0x00053126.l ; call 00053126 FUN_00053126
00059CB8: move.w #0x12,D0w
00059CBC: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
00059CC2: clr.w (0x00ffddc2).l
00059CC8: jsr 0x0005aa96.l ; call 0005AA96 FUN_0005aa96
00059CCE: tst.w (0x00ffddc2).l
00059CD4: bne.w 0x00059cfe
00059CD8: clr.w (0x00ffddc2).l
00059CDE: jsr 0x0005ad1a.l ; call 0005AD1A FUN_0005ad1a
00059CE4: clr.w (0x00ffddc2).l
00059CEA: jsr 0x0000131c.l ; call 0000131C step_palette_transition
00059CF0: move.b #0x2,(0x00fffbf8).l
00059CF8: jsr 0x00059d12.l ; call 00059D12 FUN_00059d12
00059CFE: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059D04: jsr 0x0000131c.l ; call 0000131C step_palette_transition
00059D0A: jsr 0x00053126.l ; call 00053126 FUN_00053126
00059D10: rts
00059D12: lea (0x59d3e).l,A6 ; data 00059d3e
00059D18: jsr 0x00001406.l ; call 00001406 FUN_00001406
00059D1E: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
00059D24: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
00059D2A: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
00059D30: move.b #0x1,(0x00fff09e).l
00059D38: jmp 0x000134d2.l ; call 000134D2 FUN_000134d2
00059D7C: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
00059D82: move.l #0x5adf0,(0x00ffdd32).l
00059D8C: move.w (0x0005adf0).l,(0x00ffdd36).l
00059D96: lea (0x5b4a8).l,A6 ; data 0005b4a8
00059D9C: jsr 0x00001406.l ; call 00001406 FUN_00001406
00059DA2: move.w #-0x40,(0x00ffdd08).l
00059DAA: move.w #0x0,(0x00ffdd0c).l
00059DB2: bset.b #0x2,(0x00ffef14).l
00059DBA: move.w #-0x20,(0x00ffdd0a).l
00059DC2: move.w #0x0,(0x00ffdd0e).l
00059DCA: bset.b #0x3,(0x00ffef14).l
00059DD2: move.w #0x138,(0x00ffdd46).l
00059DDA: move.w #0xf1,(0x00ffdd48).l
00059DE2: move.w #0x1,(0x00ffdd02).l
00059DEA: lea (0xffd986).l,A3 ; data 00ffd986
00059DF0: move.w #0x2,D5w
00059DF4: bsr.w 0x0005a37c ; call 0005A37C FUN_0005a37c
00059DF8: subq.w #0x8,A3
00059DFA: move.w #0x1,(0x00ffd980).l
00059E02: clr.b (0x3,A3)
00059E06: bset.b #0x1,(0x00ffef14).l
00059E0E: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
00059E14: move.w #0xa0,D7w
00059E18: move.w D7w,-(SP) ; data Stack[-0x2]
00059E1A: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00059E20: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059E26: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00059E2C: bsr.w 0x0005af2a ; call 0005AF2A FUN_0005af2a
00059E30: tst.w (0x00ffdd08).l
00059E36: beq.w 0x00059e5a
00059E3A: cmpi.w #0x178,(0x00ffdd46).l
00059E42: beq.w 0x00059e4c
00059E46: addq.w #0x1,(0x00ffdd46).l
00059E4C: addq.w #0x1,(0x00ffdd08).l
00059E52: bset.b #0x2,(0x00ffef14).l
00059E5A: tst.w (0x00ffdd0a).l
00059E60: beq.w 0x00059e92
00059E64: move.w (0x00ffdf88).l,D0w
00059E6A: andi.w #0x1,D0w
00059E6E: bne.w 0x00059e92
00059E72: cmpi.w #0xd1,(0x00ffdd48).l
00059E7A: beq.w 0x00059e84
00059E7E: subq.w #0x1,(0x00ffdd48).l
00059E84: addq.w #0x1,(0x00ffdd0a).l
00059E8A: bset.b #0x3,(0x00ffef14).l
00059E92: move.w #0x1,(0x00ffdd02).l
00059E9A: lea (0xffd986).l,A3 ; data 00ffd986
00059EA0: move.w #0x2,D5w
00059EA4: bsr.w 0x0005a37c ; call 0005A37C FUN_0005a37c
00059EA8: subq.w #0x8,A3
00059EAA: move.w #0x1,(0x00ffd980).l
00059EB2: clr.b (0x3,A3)
00059EB6: bset.b #0x1,(0x00ffef14).l
00059EBE: move.w (SP)+,D7w
00059EC0: tst.w (0x00ffddc2).l
00059EC6: bne.w 0x00059ece
00059ECA: dbf D7w,0x00059e18
00059ECE: move.w (0x00ffdf88).l,(0x00fff11e).l
00059ED8: jsr 0x00059f7a.l ; call 00059F7A FUN_00059f7a
00059EDE: jsr 0x0005a0a4.l ; call 0005A0A4 FUN_0005a0a4
00059EE4: jsr 0x0005a244.l ; call 0005A244 FUN_0005a244
00059EEA: jsr 0x0005a3b0.l ; call 0005A3B0 FUN_0005a3b0
00059EF0: jsr 0x0005a4b4.l ; call 0005A4B4 FUN_0005a4b4
00059EF6: jsr 0x0005a572.l ; call 0005A572 FUN_0005a572
00059EFC: jsr 0x0005a61a.l ; call 0005A61A FUN_0005a61a
00059F02: jsr 0x0005a70e.l ; call 0005A70E FUN_0005a70e
00059F08: jsr 0x0005a7ea.l ; call 0005A7EA FUN_0005a7ea
00059F0E: jsr 0x0005a8c8.l ; call 0005A8C8 FUN_0005a8c8
00059F14: bsr.w 0x0005a1ea ; call 0005A1EA FUN_0005a1ea
00059F18: move.w (0x00ffdf88).l,-(SP)
00059F1E: move.w (SP)+,(0x00ffdf88).l
00059F24: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059F2A: move.w #0x1f0,D7w
00059F2E: move.w D7w,-(SP) ; data Stack[-0x2]
00059F30: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
00059F36: jsr 0x00059f8e.l ; call 00059F8E FUN_00059f8e
00059F3C: jsr 0x0005a0b2.l ; call 0005A0B2 FUN_0005a0b2
00059F42: jsr 0x0000195c.l ; call 0000195C sample_controller_input
00059F48: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
00059F4E: bsr.w 0x0005af2a ; call 0005AF2A FUN_0005af2a
00059F52: jsr 0x0005a1ea.l ; call 0005A1EA FUN_0005a1ea
00059F58: move.w (SP)+,D7w
00059F5A: cmpi.w #0x164,D7w
00059F5E: bne.w 0x00059f6a
00059F62: bsr.w 0x0005ae3a ; call 0005AE3A FUN_0005ae3a
00059F66: bsr.w 0x0005adfe ; call 0005ADFE FUN_0005adfe
00059F6A: tst.w (0x00ffddc2).l
00059F70: bne.w 0x00059f78
00059F74: dbf D7w,0x00059f2e
00059F78: rts
00059F7A: clr.w (0x00fff122).l
00059F80: clr.w (0x00fff120).l
00059F86: clr.w (0x00fff124).l
00059F8C: rts
00059F8E: move.w (0x00ffdf88).l,D0w
00059F94: move.w D0w,D1w
00059F96: sub.w (0x00fff11e).l,D1w
00059F9C: cmpi.w #0x168,D1w
00059FA0: bhi.w 0x0005a07e
00059FA4: bne.b 0x00059ff6
00059FA6: move.w #0x2e,D0w
00059FAA: jsr 0x000530de.l ; call 000530DE FUN_000530de
00059FB0: move.w #0x0,(0x00ffdd08).l
00059FB8: jsr 0x00000f1c.l ; call 00000F1C FUN_00000f1c
00059FBE: move.w #0x600,D4w
00059FC2: move.w #0x10,D7w
00059FC6: move.w #0xf,D1w
00059FCA: jsr 0x0005b32c.l ; call 0005B32C FUN_0005b32c
00059FD0: lea (0x1e2e3e).l,A1 ; data 001e2e3e
00059FD6: move.w #0x786,D4w
00059FDA: move.w #0x13,D7w
00059FDE: move.w #0xc,D1w
00059FE2: move.w #0x4000,D2w
00059FE6: clr.w D3w
00059FE8: move.w D7w,D5w
00059FEA: addq.w #0x1,D5w
00059FEC: jsr 0x00001728.l ; call 00001728 FUN_00001728
00059FF2: bra.w 0x0005a07e
00059FF6: cmpi.w #0x120,D1w
00059FFA: bcs.b 0x0005a01a
00059FFC: addi.w #0xaa,(0x00fff124).l
0005A004: move.w (0x00fff124).l,D1w
0005A00A: lsr.w #0x8,D1w
0005A00C: move.w D1w,(0x00ffdd08).l
0005A012: bset.b #0x2,(0x00ffef14).l
0005A01A: cmp.w (0x00fff122).l,D0w
0005A020: bcs.w 0x0005a07e
0005A024: lea (0x5a080).l,A0 ; data 0005a080
0005A02A: move.w (0x00fff120).l,D0w
0005A030: add.w D0w,D0w
0005A032: move.w D0w,D1w
0005A034: add.w D0w,D0w
0005A036: add.w D1w,D0w
0005A038: adda.w D0w,A0
0005A03A: movea.l (A0)+,A1 ; data 0005a080
0005A03C: move.w (A0),D0w ; data 0005a084
0005A03E: add.w (0x00ffdf88).l,D0w
0005A044: move.w D0w,(0x00fff122).l
0005A04A: move.w #0x600,D4w
0005A04E: move.w #0x10,D7w
0005A052: move.w #0xf,D1w
0005A056: move.w #0x4000,D2w
0005A05A: clr.w D3w
0005A05C: move.w D7w,D5w
0005A05E: addq.w #0x1,D5w
0005A060: jsr 0x00001728.l ; call 00001728 FUN_00001728
0005A066: move.w (0x00fff120).l,D0w
0005A06C: addq.w #0x1,D0w
0005A06E: cmpi.w #0x6,D0w
0005A072: bcs.w 0x0005a078
0005A076: clr.w D0w
0005A078: move.w D0w,(0x00fff120).l
0005A07E: rts
0005A0A4: clr.w (0x00fff128).l
0005A0AA: clr.w (0x00fff126).l
0005A0B0: rts
0005A0B2: cmpi.w #0xc,(0x00fff126).l
0005A0BA: bcc.w 0x0005a1d0
0005A0BE: move.w (0x00ffdf88).l,D0w
0005A0C4: cmp.w (0x00fff128).l,D0w
0005A0CA: bcs.w 0x0005a1d0
0005A0CE: cmpi.w #0x4,(0x00fff126).l
0005A0D6: bcc.b 0x0005a124
0005A0D8: lea (0x5a1d2).l,A0 ; data 0005a1d2
0005A0DE: move.w (0x00fff126).l,D0w
0005A0E4: add.w D0w,D0w
0005A0E6: move.w D0w,D1w
0005A0E8: add.w D0w,D0w
0005A0EA: add.w D1w,D0w
0005A0EC: adda.w D0w,A0
0005A0EE: movea.l (A0)+,A1 ; data 0005a1d2
0005A0F0: move.w (A0),D0w ; data 0005a1d6
0005A0F2: add.w (0x00ffdf88).l,D0w
0005A0F8: move.w D0w,(0x00fff128).l
0005A0FE: move.w #0x826,D4w
0005A102: move.w #0xc,D7w
0005A106: move.w #0xb,D1w
0005A10A: move.w #0x0,D2w
0005A10E: clr.w D3w
0005A110: move.w D7w,D5w
0005A112: addq.w #0x1,D5w
0005A114: jsr 0x00001728.l ; call 00001728 FUN_00001728
0005A11A: addq.w #0x1,(0x00fff126).l
0005A120: bra.w 0x0005a1d0
0005A124: cmpi.w #0x4,(0x00fff126).l
0005A12C: bne.w 0x0005a13a
0005A130: move.w #0x46,D0w
0005A134: jsr 0x000530de.l ; call 000530DE FUN_000530de
0005A13A: cmpi.w #0x4,(0x00fff126).l
0005A142: bne.w 0x0005a16e
0005A146: move.w #0x0,D0w
0005A14A: move.w #0xeee,D1w
0005A14E: jsr 0x0005b2c2.l ; call 0005B2C2 FUN_0005b2c2
0005A154: addq.w #0x1,(0x00fff126).l
0005A15A: move.w (0x00ffdf88).l,D0w
0005A160: addi.w #0x9,D0w
0005A164: move.w D0w,(0x00fff128).l
0005A16A: bra.w 0x0005a1d0
0005A16E: move.w (0x00ffdf88).l,D0w
0005A174: cmp.w (0x00fff128).l,D0w
0005A17A: bcs.w 0x0005a1d0
0005A17E: move.w #0x0,D0w
0005A182: bsr.w 0x0005b2e2 ; call 0005B2E2 FUN_0005b2e2
0005A186: move.w (0x00ffdf88).l,D0w
0005A18C: addi.w #0x9,D0w
0005A190: move.w D0w,(0x00fff128).l
0005A196: addq.w #0x1,(0x00fff126).l
0005A19C: cmpi.w #0x9,(0x00fff126).l
0005A1A4: bne.b 0x0005a1b4
0005A1A6: move.w #0x13d,D0w
0005A1AA: move.w #0x139,D1w
0005A1AE: jsr 0x0005a582.l ; call 0005A582 FUN_0005a582
0005A1B4: cmpi.w #0xc,(0x00fff126).l
0005A1BC: bcs.b 0x0005a1d0
0005A1BE: move.w #0x826,D4w
0005A1C2: move.w #0xc,D7w
0005A1C6: move.w #0xb,D1w
0005A1CA: jsr 0x0005b32c.l ; call 0005B32C FUN_0005b32c
0005A1D0: rts
0005A1EA: move.w #0x1,(0x00ffdd02).l
0005A1F2: lea (0xffd986).l,A3 ; data 00ffd986
0005A1F8: move.w #0x2,D5w
0005A1FC: jsr 0x0005a252.l ; call 0005A252 FUN_0005a252
0005A202: jsr 0x0005a3be.l ; call 0005A3BE FUN_0005a3be
0005A208: jsr 0x0005a4c2.l ; call 0005A4C2 FUN_0005a4c2
0005A20E: jsr 0x0005a59c.l ; call 0005A59C FUN_0005a59c
0005A214: jsr 0x0005a628.l ; call 0005A628 FUN_0005a628
0005A21A: jsr 0x0005a71c.l ; call 0005A71C FUN_0005a71c
0005A220: jsr 0x0005a7f8.l ; call 0005A7F8 FUN_0005a7f8
0005A226: jsr 0x0005a8de.l ; call 0005A8DE FUN_0005a8de
0005A22C: subq.w #0x8,A3
0005A22E: move.w #0x1,(0x00ffd980).l
0005A236: clr.b (0x3,A3)
0005A23A: bset.b #0x1,(0x00ffef14).l
0005A242: rts
0005A244: clr.w (0x00fff130).l
0005A24A: clr.w (0x00fff12e).l
0005A250: rts
0005A252: cmpi.w #0xc,(0x00fff12e).l
0005A25A: bcc.w 0x0005a37a
0005A25E: move.w (0x00ffdf88).l,D0w
0005A264: cmp.w (0x00fff130).l,D0w
0005A26A: bcs.w 0x0005a362
0005A26E: cmpi.w #0x4,(0x00fff12e).l
0005A276: bcc.b 0x0005a2ae
0005A278: lea (0x5a398).l,A0 ; data 0005a398
0005A27E: move.w (0x00fff12e).l,D0w
0005A284: add.w D0w,D0w
0005A286: move.w D0w,D1w
0005A288: add.w D0w,D0w
0005A28A: add.w D1w,D0w
0005A28C: adda.w D0w,A0
0005A28E: movea.l (A0)+,A1 ; data 0005a398
0005A290: move.w (A0),D0w ; data 0005a39c
0005A292: add.w (0x00ffdf88).l,D0w
0005A298: move.w D0w,(0x00fff130).l
0005A29E: move.l A1,(0x00fff12a).l
0005A2A4: addq.w #0x1,(0x00fff12e).l
0005A2AA: bra.w 0x0005a362
0005A2AE: cmpi.w #0x4,(0x00fff12e).l
0005A2B6: bne.w 0x0005a2c4
0005A2BA: move.w #0x2f,D0w
0005A2BE: jsr 0x000530de.l ; call 000530DE FUN_000530de
0005A2C4: cmpi.w #0x4,(0x00fff12e).l
0005A2CC: bne.w 0x0005a300
0005A2D0: move.w #0x4,(0x00fff146).l
0005A2D8: move.w #0x1,D0w
0005A2DC: move.w #0xeee,D1w
0005A2E0: jsr 0x0005b2c2.l ; call 0005B2C2 FUN_0005b2c2
0005A2E6: addq.w #0x1,(0x00fff12e).l
0005A2EC: move.w (0x00ffdf88).l,D0w
0005A2F2: addi.w #0x9,D0w
0005A2F6: move.w D0w,(0x00fff130).l
0005A2FC: bra.w 0x0005a362
0005A300: move.w (0x00ffdf88).l,D0w
0005A306: cmp.w (0x00fff130).l,D0w
0005A30C: bcs.w 0x0005a362
0005A310: move.w #0x1,D0w
0005A314: bsr.w 0x0005b2e2 ; call 0005B2E2 FUN_0005b2e2
0005A318: move.w (0x00ffdf88).l,D0w
0005A31E: addi.w #0x9,D0w
0005A322: move.w D0w,(0x00fff130).l
0005A328: addq.w #0x1,(0x00fff12e).l
0005A32E: cmpi.w #0x9,(0x00fff12e).l
0005A336: bne.b 0x0005a346
0005A338: move.w #0x182,D0w
0005A33C: move.w #0xe2,D1w
0005A340: jsr 0x0005a582.l ; call 0005A582 FUN_0005a582
0005A346: cmpi.w #0xc,(0x00fff12e).l
0005A34E: bcs.b 0x0005a362
0005A350: move.w #0x3b8,D4w
0005A354: move.w #0x9,D7w
0005A358: move.w #0x8,D1w
0005A35C: jsr 0x0005b32c.l ; call 0005B32C FUN_0005b32c
0005A362: movea.l (0x00fff12a).l,A2
0005A368: move.w #0x178,D1w
0005A36C: move.w #0xd1,D2w
0005A370: move.w #0x2000,D6w
0005A374: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A37A: rts
0005A37C: movea.l #0x1df1b2,A2
0005A382: move.w (0x00ffdd46).l,D1w
0005A388: move.w (0x00ffdd48).l,D2w
0005A38E: move.w #0x2000,D6w
0005A392: jmp 0x00001378.l ; call 00001378 FUN_00001378
0005A3B0: clr.w (0x00fff138).l
0005A3B6: clr.w (0x00fff136).l
0005A3BC: rts
0005A3BE: move.w (0x00ffdf88).l,D0w
0005A3C4: cmp.w (0x00fff138).l,D0w
0005A3CA: bcs.w 0x0005a450
0005A3CE: cmpi.w #0x5,(0x00fff136).l
0005A3D6: bcc.w 0x0005a480
0005A3DA: lea (0x5a482).l,A0 ; data 0005a482
0005A3E0: move.w (0x00fff136).l,D0w
0005A3E6: add.w D0w,D0w
0005A3E8: move.w D0w,D1w
0005A3EA: add.w D0w,D0w
0005A3EC: add.w D0w,D0w
0005A3EE: add.w D1w,D0w
0005A3F0: adda.w D0w,A0
0005A3F2: move.l (A0)+,(0x00fff132).l
0005A3F8: move.w (A0)+,D0w ; data 0005a486
0005A3FA: add.w (0x00ffdf88).l,D0w
0005A400: move.w D0w,(0x00fff138).l
0005A406: move.w (A0)+,D0w ; data 0005a488
0005A408: beq.b 0x0005a410
0005A40A: move.w D0w,(0x00fff13a).l
0005A410: move.w (A0),(0x00fff13c).l
0005A416: move.w (0x00fff136).l,D0w
0005A41C: addq.w #0x1,D0w
0005A41E: cmpi.w #0x5,D0w
0005A422: bcs.w 0x0005a44a
0005A426: move.w #0x1,D0w
0005A42A: move.w (0x00ffdf88).l,D1w
0005A430: sub.w (0x00fff11e).l,D1w
0005A436: cmpi.w #0x168,D1w
0005A43A: bcs.w 0x0005a44a
0005A43E: move.w #0x5,(0x00fff136).l
0005A446: bra.w 0x0005a480
0005A44A: move.w D0w,(0x00fff136).l
0005A450: move.w (0x00fff13c).l,D1w
0005A456: add.w D1w,(0x00fff13a).l
0005A45C: move.w (0x00fff124).l,D3w
0005A462: lsr.w #0x8,D3w
0005A464: movea.l (0x00fff132).l,A2
0005A46A: move.w (0x00fff13a).l,D1w
0005A470: add.w D3w,D1w
0005A472: move.w #0x133,D2w
0005A476: move.w #0x6000,D6w
0005A47A: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A480: rts
0005A4B4: clr.w (0x00fff148).l
0005A4BA: clr.w (0x00fff146).l
0005A4C0: rts
0005A4C2: cmpi.w #0x4,(0x00fff146).l
0005A4CA: bcc.w 0x0005a560
0005A4CE: move.w (0x00ffdf88).l,D0w
0005A4D4: cmp.w (0x00fff148).l,D0w
0005A4DA: bcs.w 0x0005a530
0005A4DE: lea (0x5a562).l,A0 ; data 0005a562
0005A4E4: move.w (0x00fff146).l,D0w
0005A4EA: add.w D0w,D0w
0005A4EC: add.w D0w,D0w
0005A4EE: move.l (0x0,A0,D0w*0x1),(0x00fff13e).l
0005A4F6: addq.w #0x4,D0w
0005A4F8: cmpi.w #0x10,D0w
0005A4FC: bcs.w 0x0005a502
0005A500: clr.w D0w
0005A502: move.l (0x0,A0,D0w*0x1),(0x00fff142).l
0005A50A: move.w (0x00ffdf88).l,D0w
0005A510: addq.w #0x6,D0w
0005A512: move.w D0w,(0x00fff148).l
0005A518: move.w (0x00fff146).l,D0w
0005A51E: addq.w #0x1,D0w
0005A520: cmpi.w #0x4,D0w
0005A524: bcs.w 0x0005a52a
0005A528: clr.w D0w
0005A52A: move.w D0w,(0x00fff146).l
0005A530: movea.l (0x00fff13e).l,A2
0005A536: move.w #0x183,D1w
0005A53A: move.w #0xbe,D2w
0005A53E: move.w #0x6000,D6w
0005A542: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A548: movea.l (0x00fff142).l,A2
0005A54E: move.w #0x1ab,D1w
0005A552: move.w #0xd7,D2w
0005A556: move.w #0x6000,D6w
0005A55A: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A560: rts
0005A572: clr.w (0x00fff150).l
0005A578: move.w #0x6,(0x00fff14e).l
0005A580: rts
0005A582: clr.w (0x00fff150).l
0005A588: clr.w (0x00fff14e).l
0005A58E: move.w D0w,(0x00fff152).l
0005A594: move.w D1w,(0x00fff154).l
0005A59A: rts
0005A59C: move.w (0x00ffdf88).l,D0w
0005A5A2: cmp.w (0x00fff150).l,D0w
0005A5A8: bcs.w 0x0005a5e4
0005A5AC: cmpi.w #0x6,(0x00fff14e).l
0005A5B4: bcc.w 0x0005a600
0005A5B8: lea (0x5a602).l,A0 ; data 0005a602
0005A5BE: move.w (0x00fff14e).l,D0w
0005A5C4: add.w D0w,D0w
0005A5C6: add.w D0w,D0w
0005A5C8: move.l (0x0,A0,D0w*0x1),(0x00fff14a).l
0005A5D0: move.w (0x00ffdf88).l,D0w
0005A5D6: addq.w #0x8,D0w
0005A5D8: move.w D0w,(0x00fff150).l
0005A5DE: addq.w #0x1,(0x00fff14e).l
0005A5E4: movea.l (0x00fff14a).l,A2
0005A5EA: move.w (0x00fff152).l,D1w
0005A5F0: move.w (0x00fff154).l,D2w
0005A5F6: move.w #0x6000,D6w
0005A5FA: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A600: rts
0005A61A: clr.w (0x00fff15c).l
0005A620: clr.w (0x00fff15a).l
0005A626: rts
0005A628: move.w (0x00ffdf88).l,D0w
0005A62E: cmp.w (0x00fff15c).l,D0w
0005A634: bcc.w 0x0005a64c
0005A638: move.w (0x00fff15e).l,D0w
0005A63E: cmp.w (0x00fff162).l,D0w
0005A644: bcc.w 0x0005a69e
0005A648: bra.w 0x0005a6c6
0005A64C: cmpi.w #0x7,(0x00fff15a).l
0005A654: bcc.w 0x0005a6c6
0005A658: lea (0x5a6c8).l,A0 ; data 0005a6c8
0005A65E: move.w (0x00fff15a).l,D0w
0005A664: add.w D0w,D0w
0005A666: move.w D0w,D1w
0005A668: add.w D0w,D0w
0005A66A: add.w D0w,D0w
0005A66C: add.w D1w,D0w
0005A66E: adda.w D0w,A0
0005A670: move.l (A0)+,(0x00fff156).l
0005A676: move.w (A0)+,(0x00fff160).l
0005A67C: move.w (A0)+,(0x00fff162).l
0005A682: move.w (A0),D0w ; data 0005a6d0
0005A684: add.w (0x00ffdf88).l,D0w
0005A68A: move.w D0w,(0x00fff15c).l
0005A690: move.w #0x200,(0x00fff15e).l
0005A698: addq.w #0x1,(0x00fff15a).l
0005A69E: subi.w #0x9,(0x00fff15e).l
0005A6A6: movea.l (0x00fff156).l,A2
0005A6AC: move.w (0x00fff15e).l,D1w
0005A6B2: move.w (0x00fff160).l,D2w
0005A6B8: addi.w #0x18,D2w
0005A6BC: move.w #0x6000,D6w
0005A6C0: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A6C6: rts
0005A70E: clr.w (0x00fff16a).l
0005A714: clr.w (0x00fff168).l
0005A71A: rts
0005A71C: move.w (0x00ffdf88).l,D0w
0005A722: cmp.w (0x00fff16a).l,D0w
0005A728: bcc.w 0x0005a73e
0005A72C: move.w (0x00fff16c).l,D0w
0005A732: cmpi.w #0x40,D0w
0005A736: bcc.w 0x0005a782
0005A73A: bra.w 0x0005a7a8
0005A73E: cmpi.w #0x8,(0x00fff168).l
0005A746: bcc.w 0x0005a7a8
0005A74A: lea (0x5a7aa).l,A0 ; data 0005a7aa
0005A750: move.w (0x00fff168).l,D0w
0005A756: lsl.w #0x3,D0w
0005A758: adda.w D0w,A0
0005A75A: move.l (A0)+,(0x00fff164).l
0005A760: move.w (A0)+,(0x00fff16e).l
0005A766: move.w (A0),D0w ; data 0005a7b0
0005A768: add.w (0x00ffdf88).l,D0w
0005A76E: move.w D0w,(0x00fff16a).l
0005A774: move.w #0x200,(0x00fff16c).l
0005A77C: addq.w #0x1,(0x00fff168).l
0005A782: subq.w #0x8,(0x00fff16c).l
0005A788: movea.l (0x00fff164).l,A2
0005A78E: move.w (0x00fff16c).l,D1w
0005A794: move.w (0x00fff16e).l,D2w
0005A79A: addi.w #0x18,D2w
0005A79E: move.w #0x6000,D6w
0005A7A2: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A7A8: rts
0005A7EA: clr.w (0x00fff176).l
0005A7F0: clr.w (0x00fff174).l
0005A7F6: rts
0005A7F8: move.w (0x00ffdf88).l,D0w
0005A7FE: cmp.w (0x00fff176).l,D0w
0005A804: bcc.w 0x0005a81a
0005A808: move.w (0x00fff178).l,D0w
0005A80E: cmpi.w #0x40,D0w
0005A812: bcc.w 0x0005a85e
0005A816: bra.w 0x0005a886
0005A81A: cmpi.w #0x8,(0x00fff174).l
0005A822: bcc.w 0x0005a886
0005A826: lea (0x5a888).l,A0 ; data 0005a888
0005A82C: move.w (0x00fff174).l,D0w
0005A832: lsl.w #0x3,D0w
0005A834: adda.w D0w,A0
0005A836: move.l (A0)+,(0x00fff170).l
0005A83C: move.w (A0)+,(0x00fff17a).l
0005A842: move.w (A0),D0w ; data 0005a88e
0005A844: add.w (0x00ffdf88).l,D0w
0005A84A: move.w D0w,(0x00fff176).l
0005A850: move.w #0x200,(0x00fff178).l
0005A858: addq.w #0x1,(0x00fff174).l
0005A85E: subi.w #0xb,(0x00fff178).l
0005A866: movea.l (0x00fff170).l,A2
0005A86C: move.w (0x00fff178).l,D1w
0005A872: move.w (0x00fff17a).l,D2w
0005A878: addi.w #0x18,D2w
0005A87C: move.w #0x6000,D6w
0005A880: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A886: rts
0005A8C8: clr.w (0x00fff182).l
0005A8CE: clr.w (0x00fff180).l
0005A8D4: move.w #0x193,(0x00fff184).l
0005A8DC: rts
0005A8DE: move.w (0x00ffdf88).l,D0w
0005A8E4: cmp.w (0x00fff182).l,D0w
0005A8EA: bcs.w 0x0005a936
0005A8EE: sub.w (0x00fff11e).l,D0w
0005A8F4: cmpi.w #0x6,D0w
0005A8F8: bcs.w 0x0005a956
0005A8FC: cmpi.w #0x3,(0x00fff180).l
0005A904: bcc.b 0x0005a956
0005A906: lea (0x5a958).l,A0 ; data 0005a958
0005A90C: move.w (0x00fff180).l,D0w
0005A912: add.w D0w,D0w
0005A914: move.w D0w,D1w
0005A916: add.w D0w,D0w
0005A918: add.w D1w,D0w
0005A91A: adda.w D0w,A0
0005A91C: move.l (A0)+,(0x00fff17c).l
0005A922: move.w (A0),D0w ; data 0005a95c
0005A924: add.w (0x00ffdf88).l,D0w
0005A92A: move.w D0w,(0x00fff182).l
0005A930: addq.w #0x1,(0x00fff180).l
0005A936: addq.w #0x4,(0x00fff184).l
0005A93C: movea.l (0x00fff17c).l,A2
0005A942: move.w (0x00fff184).l,D1w
0005A948: move.w #0x13c,D2w
0005A94C: move.w #0x6000,D6w
0005A950: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005A956: rts
0005A96A: lea (0x5b9b4).l,A1 ; data 0005b9b4
0005A970: movea.w #0x2906,A2
0005A974: move.w #-0x2000,D1w
0005A978: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A97E: lea (0x5b9d0).l,A1 ; data 0005b9d0
0005A984: movea.w #0x2986,A2
0005A988: move.w #-0x2000,D1w
0005A98C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A992: lea (0x5b9ef).l,A1 ; data 0005b9ef
0005A998: movea.w #0x2a06,A2
0005A99C: move.w #-0x2000,D1w
0005A9A0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A9A6: lea (0x5ba08).l,A1 ; data 0005ba08
0005A9AC: movea.w #0x2a86,A2
0005A9B0: move.w #-0x2000,D1w
0005A9B4: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A9BA: lea (0x5ba15).l,A1 ; data 0005ba15
0005A9C0: movea.w #0x2b86,A2
0005A9C4: move.w #-0x2000,D1w
0005A9C8: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A9CE: lea (0x5ba34).l,A1 ; data 0005ba34
0005A9D4: movea.w #0x2c06,A2
0005A9D8: move.w #-0x2000,D1w
0005A9DC: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A9E2: lea (0x5b961).l,A1 ; data 0005b961
0005A9E8: movea.w #0x2186,A2
0005A9EC: move.w #-0x2000,D1w
0005A9F0: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005A9F6: lea (0x5b983).l,A1 ; data 0005b983
0005A9FC: movea.w #0x2286,A2
0005AA00: move.w #-0x2000,D1w
0005AA04: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA0A: lea (0x5b9a3).l,A1 ; data 0005b9a3
0005AA10: movea.w #0x2386,A2
0005AA14: move.w #-0x2000,D1w
0005AA18: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA1E: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AA24: movea.w #0x2186,A2
0005AA28: move.w #-0x2000,D1w
0005AA2C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA32: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AA38: movea.w #0x2286,A2
0005AA3C: move.w #-0x2000,D1w
0005AA40: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA46: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AA4C: movea.w #0x2386,A2
0005AA50: move.w #-0x2000,D1w
0005AA54: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA5A: lea (0x5b902).l,A1 ; data 0005b902
0005AA60: movea.w #0x2186,A2
0005AA64: move.w #-0x8000,D1w
0005AA68: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA6E: lea (0x5b925).l,A1 ; data 0005b925
0005AA74: movea.w #0x2286,A2
0005AA78: move.w #-0x8000,D1w
0005AA7C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA82: lea (0x5b943).l,A1 ; data 0005b943
0005AA88: movea.w #0x2386,A2
0005AA8C: move.w #-0x8000,D1w
0005AA90: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AA96: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
0005AA9C: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0005AAA2: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0005AAA8: lea (0xffbf40).l,A1 ; data 00ffbf40
0005AAAE: movea.l #0x1e3f38,A3
0005AAB4: move.l #0x3020,D0
0005AABA: jsr 0x00001530.l ; call 00001530 FUN_00001530
0005AAC0: lea (0xffbf40).l,A1 ; data 00ffbf40
0005AAC6: movea.l #0x1e62be,A3
0005AACC: move.l #0xc000,D0
0005AAD2: jsr 0x00001530.l ; call 00001530 FUN_00001530
0005AAD8: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
0005AADE: bsr.w 0x0005aa5a ; call 0005AA5A FUN_0005aa5a
0005AAE2: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005AAE8: lea (0x5b632).l,A0 ; data 0005b632
0005AAEE: jsr 0x00001368.l ; call 00001368 copy_target_palette
0005AAF4: move.w #0x786,D4w
0005AAF8: move.w #0x13,D7w
0005AAFC: move.w #0xc,D1w
0005AB00: jsr 0x0005b32c.l ; call 0005B32C FUN_0005b32c
0005AB06: move.w (0x00ffdf88).l,(0x00fff11e).l
0005AB10: jsr 0x0005ac28.l ; call 0005AC28 FUN_0005ac28
0005AB16: jsr 0x0005ac70.l ; call 0005AC70 FUN_0005ac70
0005AB1C: bsr.w 0x0005abd2 ; call 0005ABD2 FUN_0005abd2
0005AB20: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005AB26: lea (0x1e620e).l,A1 ; data 001e620e
0005AB2C: move.w #0x700,D4w
0005AB30: move.w #0x27,D7w
0005AB34: move.w #0xd,D1w
0005AB38: move.w #-0x8000,D2w
0005AB3C: clr.w D3w
0005AB3E: move.w D7w,D5w
0005AB40: addq.w #0x1,D5w
0005AB42: jsr 0x00001728.l ; call 00001728 FUN_00001728
0005AB48: move.w (0x00ffdf88).l,-(SP)
0005AB4E: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
0005AB54: move.w (SP)+,(0x00ffdf88).l
0005AB5A: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005AB60: move.w #0x114,D7w
0005AB64: move.w D7w,-(SP) ; data Stack[-0x2]
0005AB66: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005AB6C: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005AB72: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0005AB78: jsr 0x0005abd2.l ; call 0005ABD2 FUN_0005abd2
0005AB7E: jsr 0x0005ab96.l ; call 0005AB96 FUN_0005ab96
0005AB84: move.w (SP)+,D7w
0005AB86: tst.w (0x00ffddc2).l
0005AB8C: bne.w 0x0005ab94
0005AB90: dbf D7w,0x0005ab64
0005AB94: rts
0005AB96: move.w (0x00ffdf88).l,D0w
0005AB9C: sub.w (0x00fff11e).l,D0w
0005ABA2: cmpi.w #0xb4,D0w
0005ABA6: bcs.w 0x0005abd0
0005ABAA: andi.w #0xb,D0w
0005ABAE: bne.w 0x0005abd0
0005ABB2: clr.w D0w
0005ABB4: bsr.w 0x0005b2e2 ; call 0005B2E2 FUN_0005b2e2
0005ABB8: move.w #0x1,D0w
0005ABBC: bsr.w 0x0005b2e2 ; call 0005B2E2 FUN_0005b2e2
0005ABC0: move.w #0x2,D0w
0005ABC4: bsr.w 0x0005b2e2 ; call 0005B2E2 FUN_0005b2e2
0005ABC8: move.w #0x3,D0w
0005ABCC: bsr.w 0x0005b2e2 ; call 0005B2E2 FUN_0005b2e2
0005ABD0: rts
0005ABD2: move.w #0x1,(0x00ffdd02).l
0005ABDA: lea (0xffd986).l,A3 ; data 00ffd986
0005ABE0: move.w #0x2,D5w
0005ABE4: jsr 0x0005ac7e.l ; call 0005AC7E FUN_0005ac7e
0005ABEA: jsr 0x0005ac0e.l ; call 0005AC0E FUN_0005ac0e
0005ABF0: jsr 0x0005ac36.l ; call 0005AC36 FUN_0005ac36
0005ABF6: subq.w #0x8,A3
0005ABF8: move.w #0x1,(0x00ffd980).l
0005AC00: clr.b (0x3,A3)
0005AC04: bset.b #0x1,(0x00ffef14).l
0005AC0C: rts
0005AC0E: lea (0x1df1d6).l,A2 ; data 001df1d6
0005AC14: move.w #0xc2,D1w
0005AC18: move.w #0x13c,D2w
0005AC1C: move.w #-0x4000,D6w
0005AC20: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005AC26: rts
0005AC28: clr.w (0x00fff186).l
0005AC2E: clr.w (0x00fff188).l
0005AC34: rts
0005AC36: lea (0x1df238).l,A2 ; data 001df238
0005AC3C: move.w (0x00fff186).l,D1w
0005AC42: lsr.w #0x8,D1w
0005AC44: addi.w #0x60,D1w
0005AC48: move.w (0x00fff188).l,D2w
0005AC4E: lsr.w #0x8,D2w
0005AC50: addi.w #0x123,D2w
0005AC54: move.w #0x6000,D6w
0005AC58: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005AC5E: addi.w #0x40,(0x00fff186).l
0005AC66: addi.w #0x18,(0x00fff188).l
0005AC6E: rts
0005AC70: clr.w (0x00fff1a4).l
0005AC76: clr.w (0x00fff1a6).l
0005AC7C: rts
0005AC7E: move.w (0x00ffdf88).l,D0w
0005AC84: sub.w (0x00fff11e).l,D0w
0005AC8A: cmpi.w #0x3c,D0w
0005AC8E: bcs.w 0x0005ad00
0005AC92: move.w (0x00ffdf88).l,D0w
0005AC98: cmp.w (0x00fff1a6).l,D0w
0005AC9E: bcs.w 0x0005ace4
0005ACA2: cmpi.w #0x3,(0x00fff1a4).l
0005ACAA: bcc.w 0x0005ace4
0005ACAE: lea (0x5ad02).l,A0 ; data 0005ad02
0005ACB4: move.w (0x00fff1a4).l,D0w
0005ACBA: lsl.w #0x3,D0w
0005ACBC: adda.w D0w,A0
0005ACBE: move.l (A0)+,(0x00fff1a0).l
0005ACC4: move.w (A0)+,(0x00fff1a8).l
0005ACCA: move.w (A0),(0x00fff1aa).l
0005ACD0: move.w (0x00ffdf88).l,D0w
0005ACD6: addq.w #0x6,D0w
0005ACD8: move.w D0w,(0x00fff1a6).l
0005ACDE: addq.w #0x1,(0x00fff1a4).l
0005ACE4: movea.l (0x00fff1a0).l,A2
0005ACEA: move.w (0x00fff1a8).l,D1w
0005ACF0: move.w (0x00fff1aa).l,D2w
0005ACF6: move.w #-0x2000,D6w
0005ACFA: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005AD00: rts
0005AD1A: jsr 0x0005b29a.l ; call 0005B29A FUN_0005b29a
0005AD20: lea (0x5b528).l,A6 ; data 0005b528
0005AD26: jsr 0x00001406.l ; call 00001406 FUN_00001406
0005AD2C: move.w (0x00ffdf88).l,(0x00fff11e).l
0005AD36: jsr 0x0005b002.l ; call 0005B002 FUN_0005b002
0005AD3C: jsr 0x0005b198.l ; call 0005B198 FUN_0005b198
0005AD42: bsr.w 0x0005b0bc ; call 0005B0BC FUN_0005b0bc
0005AD46: bsr.w 0x0005a9e2 ; call 0005A9E2 FUN_0005a9e2
0005AD4A: move.w (0x00ffdf88).l,-(SP)
0005AD50: lea (0xffd87e).l,A1 ; data 00ffd87e
0005AD56: lea (0xffd8fe).l,A0 ; data 00ffd8fe
0005AD5C: moveq #0x1f,D0
0005AD5E: move.l (A0)+,(A1)+
0005AD60: dbf D0w,0x0005ad5e
0005AD64: bset.b #0x0,(0x00ffef14).l
0005AD6C: move.w (SP)+,(0x00ffdf88).l
0005AD72: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005AD78: move.w #0x528,D7w
0005AD7C: move.w D7w,-(SP) ; data 00000528
0005AD7E: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005AD84: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005AD8A: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0005AD90: move.w (SP)+,D7w
0005AD92: cmpi.w #0x1e0,D7w
0005AD96: bne.w 0x0005ada4
0005AD9A: move.w D7w,-(SP) ; data 00000528
0005AD9C: jsr 0x0005a96a.l ; call 0005A96A FUN_0005a96a
0005ADA2: move.w (SP)+,D7w
0005ADA4: move.w D7w,-(SP) ; data 00000528
0005ADA6: jsr 0x0005b08e.l ; call 0005B08E FUN_0005b08e
0005ADAC: jsr 0x0005b00a.l ; call 0005B00A FUN_0005b00a
0005ADB2: jsr 0x0005b056.l ; call 0005B056 FUN_0005b056
0005ADB8: jsr 0x0005b0bc.l ; call 0005B0BC FUN_0005b0bc
0005ADBE: move.w (SP)+,D7w
0005ADC0: tst.w (0x00ffddc2).l
0005ADC6: bne.w 0x0005adce
0005ADCA: dbf D7w,0x0005ad7c
0005ADCE: rts
0005ADFE: lea (0x5b8a4).l,A1 ; data 0005b8a4
0005AE04: movea.w #0x2106,A2
0005AE08: move.w #-0x2000,D1w
0005AE0C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE12: lea (0x5b8c6).l,A1 ; data 0005b8c6
0005AE18: movea.w #0x2206,A2
0005AE1C: move.w #-0x2000,D1w
0005AE20: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE26: lea (0x5b8e8).l,A1 ; data 0005b8e8
0005AE2C: movea.w #0x2306,A2
0005AE30: move.w #-0x2000,D1w
0005AE34: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE3A: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AE40: movea.w #0x2106,A2
0005AE44: move.w #-0x6000,D1w
0005AE48: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE4E: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AE54: movea.w #0x2186,A2
0005AE58: move.w #-0x6000,D1w
0005AE5C: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE62: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AE68: movea.w #0x2206,A2
0005AE6C: move.w #-0x6000,D1w
0005AE70: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE76: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AE7C: movea.w #0x2286,A2
0005AE80: move.w #-0x6000,D1w
0005AE84: jsr 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AE8A: lea (0x5b7b2).l,A1 ; data 0005b7b2
0005AE90: movea.w #0x2306,A2
0005AE94: move.w #-0x6000,D1w
0005AE98: jmp 0x0000c0da.l ; call 0000C0DA FUN_0000c0da
0005AF2A: subq.w #0x1,(0x00ffdd36).l
0005AF30: bpl.w 0x0005af5c
0005AF34: movea.l (0x00ffdd32).l,A0
0005AF3A: adda.w #0x2,A0
0005AF3E: movea.l (A0)+,A1
0005AF40: move.l A0,(0x00ffdd32).l
0005AF46: move.l A0,-(SP) ; data Stack[-0x4]
0005AF48: jsr (A1)
0005AF4A: movea.l (SP)+,A0
0005AF4C: move.w (A0),(0x00ffdd36).l
0005AF52: bne.w 0x0005af5c
0005AF56: ori #0x1,CCR
0005AF5A: rts
0005AF5C: andi #0x1e,CCR
0005AF60: rts
0005AF62: clr.w (0x00ffdd08).l
0005AF68: clr.w (0x00ffdd0c).l
0005AF6E: clr.w (0x00ffdd0a).l
0005AF74: clr.w (0x00ffdd0e).l
0005AF7A: bset.b #0x2,(0x00ffef14).l
0005AF82: bset.b #0x3,(0x00ffef14).l
0005AF8A: jsr 0x0005b29a.l ; call 0005B29A FUN_0005b29a
0005AF90: lea (0x5b574).l,A6 ; data 0005b574
0005AF96: jsr 0x00001406.l ; call 00001406 FUN_00001406
0005AF9C: jsr 0x0000c0c2.l ; call 0000C0C2 FUN_0000c0c2
0005AFA2: move.l #0x5add0,(0x00ffdd32).l
0005AFAC: move.w (0x0005add0).l,(0x00ffdd36).l
0005AFB6: lea (0xffd87e).l,A1 ; data 00ffd87e
0005AFBC: lea (0xffd8fe).l,A0 ; data 00ffd8fe
0005AFC2: moveq #0x1f,D0
0005AFC4: move.l (A0)+,(A1)+
0005AFC6: dbf D0w,0x0005afc4
0005AFCA: bset.b #0x0,(0x00ffef14).l
0005AFD2: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005AFD8: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005AFDE: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005AFE4: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0005AFEA: bsr.w 0x0005af2a ; call 0005AF2A FUN_0005af2a
0005AFEE: bcs.w 0x0005b000
0005AFF2: tst.w (0x00ffddc2).l
0005AFF8: bne.w 0x0005b000
0005AFFC: dbf D7w,0x0005afd8
0005B000: rts
0005B002: clr.w (0x00fff18a).l
0005B008: rts
0005B00A: move.w (0x00ffdd08).l,D0w
0005B010: cmpi.w #-0x200,D0w
0005B014: blt.w 0x0005b054
0005B018: cmpi.w #-0x190,D0w
0005B01C: bgt.w 0x0005b044
0005B020: tst.w (0x00fff18a).l
0005B026: bne.w 0x0005b044
0005B02A: move.w #0x74a,D4w
0005B02E: move.w #0x4,D7w
0005B032: move.w #0x7,D1w
0005B036: jsr 0x0005b32c.l ; call 0005B32C FUN_0005b32c
0005B03C: move.w #0x1,(0x00fff18a).l
0005B044: subq.w #0x2,D0w
0005B046: move.w D0w,(0x00ffdd08).l
0005B04C: bset.b #0x2,(0x00ffef14).l
0005B054: rts
0005B056: move.w (0x00ffdd08).l,D0w
0005B05C: cmpi.w #-0x1b0,D0w
0005B060: bgt.w 0x0005b08c
0005B064: bne.w 0x0005b06e
0005B068: jsr 0x0005aa1e.l ; call 0005AA1E FUN_0005aa1e
0005B06E: move.w (0x00ffdd0e).l,D0w
0005B074: cmpi.w #0x78,D0w
0005B078: bge.w 0x0005b08c
0005B07C: addq.w #0x1,D0w
0005B07E: move.w D0w,(0x00ffdd0e).l
0005B084: bset.b #0x3,(0x00ffef14).l
0005B08C: rts
0005B08E: move.w (0x00ffdf88).l,D0w
0005B094: andi.w #0x7,D0w
0005B098: bne.w 0x0005b0ba
0005B09C: lea (0xffd8a0).l,A0 ; data 00ffd8a0
0005B0A2: move.w (A0),D0w
0005B0A4: move.w (0x2,A0),(A0)
0005B0A8: move.w (0x4,A0),(0x2,A0)
0005B0AE: move.w D0w,(0x4,A0)
0005B0B2: bset.b #0x0,(0x00ffef14).l
0005B0BA: rts
0005B0BC: move.w #0x1,(0x00ffdd02).l
0005B0C4: lea (0xffd986).l,A3 ; data 00ffd986
0005B0CA: move.w #0x2,D5w
0005B0CE: jsr 0x0005b100.l ; call 0005B100 FUN_0005b100
0005B0D4: jsr 0x0005b1b6.l ; call 0005B1B6 FUN_0005b1b6
0005B0DA: subq.w #0x8,A3
0005B0DC: move.w #0x1,(0x00ffd980).l
0005B0E4: clr.b (0x3,A3)
0005B0E8: bset.b #0x1,(0x00ffef14).l
0005B0F0: rts
0005B100: move.w (0x00ffdd08).l,D0w
0005B106: cmpi.w #-0x1b8,D0w
0005B10A: blt.w 0x0005b17e
0005B10E: addi.w #0x1fc,D0w
0005B112: move.w (0x00ffdf88).l,D0w
0005B118: cmp.w (0x00fff192).l,D0w
0005B11E: bcs.w 0x0005b160
0005B122: lea (0x5b180).l,A0 ; data 0005b180
0005B128: move.w (0x00fff190).l,D0w
0005B12E: add.w D0w,D0w
0005B130: add.w D0w,D0w
0005B132: adda.w D0w,A0
0005B134: move.l (A0)+,(0x00fff18c).l
0005B13A: move.w (0x00ffdf88).l,D0w
0005B140: addq.w #0x6,D0w
0005B142: move.w D0w,(0x00fff192).l
0005B148: move.w (0x00fff190).l,D0w
0005B14E: addq.w #0x1,D0w
0005B150: cmpi.w #0x6,D0w
0005B154: bcs.w 0x0005b15a
0005B158: clr.w D0w
0005B15A: move.w D0w,(0x00fff190).l
0005B160: movea.l (0x00fff18c).l,A2
0005B166: move.w (0x00ffdd08).l,D1w
0005B16C: addi.w #0x1fc,D1w
0005B170: move.w #0xef,D2w
0005B174: move.w #0x2000,D6w
0005B178: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005B17E: rts
0005B198: clr.w (0x00fff194).l
0005B19E: clr.w (0x00fff196).l
0005B1A4: move.w #0x7c00,(0x00fff198).l
0005B1AC: move.w #-0x4900,(0x00fff19a).l
0005B1B4: rts
0005B1B6: move.w (0x00ffdf88).l,D0w
0005B1BC: sub.w (0x00fff11e).l,D0w
0005B1C2: cmpi.w #0x226,D0w
0005B1C6: bcs.w 0x0005b26e
0005B1CA: move.w (0x00ffdf88).l,D0w
0005B1D0: cmp.w (0x00fff196).l,D0w
0005B1D6: bcs.w 0x0005b21c
0005B1DA: cmpi.w #0x7,(0x00fff194).l
0005B1E2: bcc.w 0x0005b21c
0005B1E6: lea (0x5b270).l,A0 ; data 0005b270
0005B1EC: move.w (0x00fff194).l,D0w
0005B1F2: add.w D0w,D0w
0005B1F4: move.w D0w,D1w
0005B1F6: add.w D0w,D0w
0005B1F8: add.w D1w,D0w
0005B1FA: adda.w D0w,A0
0005B1FC: move.w (A0)+,(0x00fff19c).l
0005B202: move.w (A0)+,(0x00fff19e).l
0005B208: move.w (A0),D0w ; data 0005b274
0005B20A: add.w (0x00ffdf88).l,D0w
0005B210: move.w D0w,(0x00fff196).l
0005B216: addq.w #0x1,(0x00fff194).l
0005B21C: move.w (0x00fff198).l,D1w
0005B222: add.w (0x00fff19c).l,D1w
0005B228: move.w D1w,(0x00fff198).l
0005B22E: lsr.w #0x8,D1w
0005B230: move.w (0x00fff19a).l,D2w
0005B236: add.w (0x00fff19e).l,D2w
0005B23C: move.w D2w,(0x00fff19a).l
0005B242: lsr.w #0x8,D2w
0005B244: addi.w #0x28,D2w
0005B248: movea.l #0x1df3f4,A2
0005B24E: cmpi.w #0x110,D2w
0005B252: bcs.w 0x0005b264
0005B256: cmpi.w #0x11f,D2w
0005B25A: bcc.w 0x0005b264
0005B25E: movea.l #0x1df3fe,A2
0005B264: move.w #0x2000,D6w
0005B268: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005B26E: rts
0005B29A: move SR,-(SP) ; data Stack[-0x2]
0005B29C: move #0x2700,SR
0005B2A0: move.w #0x3000,D0w
0005B2A4: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005B2AA: move.l #0x7ff07ff,D0
0005B2B0: move.w #0x3ff,D1w
0005B2B4: move.l D0,(0x00c00000).l
0005B2BA: dbf D1w,0x0005b2b4
0005B2BE: move (SP)+,SR
0005B2C0: rts
0005B2C2: lsl.w #0x5,D0w
0005B2C4: addq.w #0x2,D0w
0005B2C6: lea (0xffd87e).l,A0 ; data 00ffd87e
0005B2CC: adda.w D0w,A0
0005B2CE: move.w #0xe,D2w
0005B2D2: move.w D1w,(A0)+ ; data 00ffd87e
0005B2D4: dbf D2w,0x0005b2d2
0005B2D8: bset.b #0x0,(0x00ffef14).l
0005B2E0: rts
0005B2E2: lsl.w #0x5,D0w
0005B2E4: addq.w #0x2,D0w
0005B2E6: lea (0xffd87e).l,A0 ; data 00ffd87e
0005B2EC: adda.w D0w,A0
0005B2EE: move.w #0xe,D2w
0005B2F2: move.w (A0),D0w ; data 00ffd87e
0005B2F4: move.w D0w,D1w
0005B2F6: andi.w #0xe,D1w
0005B2FA: beq.w 0x0005b300
0005B2FE: subq.w #0x2,D0w
0005B300: move.w D0w,D1w
0005B302: andi.w #0xe0,D1w
0005B306: beq.w 0x0005b30e
0005B30A: subi.w #0x20,D0w
0005B30E: move.w D0w,D1w
0005B310: andi.w #0xe00,D1w
0005B314: beq.w 0x0005b31c
0005B318: subi.w #0x200,D0w
0005B31C: move.w D0w,(A0)+ ; data 00ffd87e
0005B31E: dbf D2w,0x0005b2f2
0005B322: bset.b #0x0,(0x00ffef14).l
0005B32A: rts
0005B32C: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0005B330: move SR,-(SP) ; data Stack[-0x3e]
0005B332: move #0x2700,SR
0005B336: lea (0xc00000).l,A2 ; data 00c00000
0005B33C: move.l D4,D0
0005B33E: move.w D7w,D3w
0005B340: move.w #0x7ff,D2w
0005B344: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005B34A: move.w D2w,(A2)
0005B34C: dbf D7w,0x0005b34a
0005B350: addi.w #0x80,D4w
0005B354: move.l D4,D0
0005B356: move.w D3w,D7w
0005B358: dbf D1w,0x0005b344
0005B35C: move (SP)+,SR
0005B35E: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0005B362: rts
0005B378: lea (0x5b36e).l,A0 ; data 0005b36e
0005B37E: lea (0xc00000).l,A2 ; data 00c00000
0005B384: lea (0x5b364).l,A3 ; data 0005b364
0005B38A: moveq #0x4,D6
0005B38C: move SR,-(SP)
0005B38E: move #0x2700,SR
0005B392: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0005B398: lea (0x1df468).l,A1 ; data 001df468
0005B39E: move.l #0x40e,D4
0005B3A4: clr.l D1
0005B3A6: clr.l D2
0005B3A8: clr.l D5
0005B3AA: move.w (A0)+,D5w
0005B3AC: move.b (A3)+,D1b
0005B3AE: move.b (A3)+,D2b
0005B3B0: move.l D4,D0
0005B3B2: add.l D5,D0
0005B3B4: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005B3BA: move.w (A1)+,D0w
0005B3BC: ori.w #0x0,D0w
0005B3C0: move.w D0w,(A2)
0005B3C2: move.w (A1)+,D0w
0005B3C4: ori.w #0x0,D0w
0005B3C8: move.w D1w,D7w
0005B3CA: move.w D0w,(A2)
0005B3CC: dbf D7w,0x0005b3ca
0005B3D0: move.w (A1)+,D0w
0005B3D2: ori.w #0x0,D0w
0005B3D6: move.w D0w,(A2)
0005B3D8: move.w #-0x7080,(0x00c00004).l
0005B3E0: move.l D4,D0
0005B3E2: add.l D5,D0
0005B3E4: addi.l #0x80,D0
0005B3EA: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005B3F0: move.w (A1)+,D0w
0005B3F2: ori.w #0x0,D0w
0005B3F6: move.w D2w,D7w
0005B3F8: move.w D0w,(A2)
0005B3FA: dbf D7w,0x0005b3f8
0005B3FE: move.l D4,D0
0005B400: add.l D5,D0
0005B402: addi.l #0x80,D0
0005B408: move.l D1,D3
0005B40A: addq.l #0x2,D3
0005B40C: mulu.w #0x2,D3
0005B410: add.l D3,D0
0005B412: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005B418: move.w (A1)+,D0w
0005B41A: ori.w #0x0,D0w
0005B41E: move.w D2w,D7w
0005B420: move.w D0w,(A2)
0005B422: dbf D7w,0x0005b420
0005B426: move.w #-0x70fe,(0x00c00004).l
0005B42E: move.l D4,D0
0005B430: add.l D5,D0
0005B432: move.l D2,D3
0005B434: addq.l #0x2,D3
0005B436: mulu.w #0x80,D3
0005B43A: add.l D3,D0
0005B43C: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005B442: move.w (A1)+,D0w
0005B444: ori.w #0x0,D0w
0005B448: move.w D0w,(A2)
0005B44A: move.w (A1)+,D0w
0005B44C: ori.w #0x0,D0w
0005B450: move.w D1w,D7w
0005B452: move.w D0w,(A2)
0005B454: dbf D7w,0x0005b452
0005B458: move.w (A1)+,D0w
0005B45A: ori.w #0x0,D0w
0005B45E: move.w D0w,(A2)
0005B460: move (SP)+,SR
0005B462: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP) ; data 0005b366
0005B466: moveq #0x1,D6
0005B468: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005B46E: jsr 0x00000bf2.l ; call 00000BF2 latch_controller_events
0005B474: dbf D6w,0x0005b468
0005B478: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0005B47C: dbf D6w,0x0005b38c
0005B480: rts
0005BA4C: bra.w 0x0005ba50
0005BA50: lea (0xffbf40).l,A1 ; data 00ffbf40
0005BA56: movea.l #0x1ebdee,A3
0005BA5C: move.w #0x5920,D0w
0005BA60: jmp 0x00001530.l ; call 00001530 FUN_00001530
0005BA66: move.w (0x00ffddea).l,D0w
0005BA6C: lea (0xff2710).l,A1 ; data 00ff2710
0005BA72: lea (0x1eb3e2).l,A2 ; data 001eb3e2
0005BA78: move.w (A2)+,(A1)
0005BA7A: move.w (A2)+,(0x2,A1)
0005BA7E: adda.w D0w,A1
0005BA80: move.w (A2)+,(A1)
0005BA82: move.w (A2)+,(0x2,A1)
0005BA86: adda.w D0w,A1
0005BA88: move.w (A2)+,(A1)
0005BA8A: move.w (A2)+,(0x2,A1)
0005BA8E: move.w #0x1,(0x00ffe100).l
0005BA96: rts
0005BA98: move.w (0x00ffddc4).l,D5w
0005BA9E: subi.w #0x15,D5w
0005BAA2: add.w D5w,D5w
0005BAA4: add.w D5w,D5w
0005BAA6: lea (0x1edb88).l,A3 ; data 001edb88
0005BAAC: movea.l (0x0,A3,D5w*0x1),A3 ; data 001edb88
0005BAB0: move.b (0x00ffe183).l,D0b
0005BAB6: ext.w D0w
0005BAB8: subq.w #0x1,D0w
0005BABA: add.w D0w,D0w
0005BABC: add.w D0w,D0w
0005BABE: movea.l (0x0,A3,D0w*0x1),A3
0005BAC2: rts
0005C8C6: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0005C8CA: move SR,-(SP) ; data Stack[-0x3e]
0005C8CC: move #0x2700,SR
0005C8D0: lea (0xc00000).l,A2 ; data 00c00000
0005C8D6: move.l D4,D0
0005C8D8: lsl.w #0x1,D7w
0005C8DA: jsr 0x00000e9c.l ; call 00000E9C FUN_00000e9c
0005C8E0: move.w (A1)+,D0w
0005C8E2: or.w D2w,D0w
0005C8E4: move.w D0w,(A2)
0005C8E6: adda.w D7w,A1
0005C8E8: addi.w #0x80,D4w
0005C8EC: move.l D4,D0
0005C8EE: dbf D1w,0x0005c8da
0005C8F2: move (SP)+,SR
0005C8F4: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0005C8F8: rts
0005D172: jsr 0x00053126.l ; call 00053126 FUN_00053126
0005D178: move.w #0x0,D0w
0005D17C: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0005D1B4: jsr 0x00053126.l ; call 00053126 FUN_00053126
0005D1BA: move.w #0x0,D0w
0005D1BE: jsr 0x000530ea.l ; call 000530EA FUN_000530ea
0005D1C4: move.w #0x3,D0w
0005D1C8: jsr 0x00059c34.l ; call 00059C34 FUN_00059c34
0005D1CE: move.w #0xc,(0x00fff0be).l
0005D1D6: lea (0x110b3e).l,A5 ; data 00110b3e
0005D1DC: lea (0x1fe6f4).l,A6 ; data 001fe6f4
0005D260: lea (0xfff11e).l,A0 ; data 00fff11e
0005D266: move.w #0x10,D7w
0005D26A: clr.l (A0)+
0005D26C: dbf D7w,0x0005d26a
0005D270: lea (0xfff11e).l,A0 ; data 00fff11e
0005D276: move.w #0x11c0,(A0)
0005D27A: move.w #-0x180,(0x4,A0)
0005D280: move.w #-0x1,(0x22,A0)
0005D286: move.w #0xb4,(0x24,A0)
0005D28C: rts
0005D29C: cmpi.b #0x2,(0x00ffe186).l
0005D2A4: beq.w 0x0005d2b6
0005D2A8: cmpi.b #0x3,(0x00ffe186).l
0005D2B0: beq.w 0x0005d2b6
0005D2B4: rts
0005D2B6: lea (0xfff11e).l,A0 ; data 00fff11e
0005D2BC: tst.w (0x22,A0)
0005D2C0: bmi.b 0x0005d2b4
0005D2C2: cmpi.w #0x2,(0x22,A0)
0005D2C8: beq.w 0x0005d330
0005D2CC: tst.w (0x22,A0)
0005D2D0: bne.w 0x0005d2f0
0005D2D4: subq.w #0x1,(0x24,A0)
0005D2D8: bpl.w 0x0005d408
0005D2DC: jsr 0x0005ba66.l ; call 0005BA66 FUN_0005ba66
0005D2E2: move.w #0x1,(0x22,A0)
0005D2E8: move.w #0x100,(0x24,A0)
0005D2EE: bra.b 0x0005d2b4
0005D2F0: tst.w (0x4,A0)
0005D2F4: bmi.w 0x0005d302
0005D2F8: cmpi.w #0x248,(0x4,A0)
0005D2FE: bcc.w 0x0005d306
0005D302: addq.w #0x8,(0x4,A0)
0005D306: jsr 0x00005a56.l ; call 00005A56 project_entity_screen_position
0005D30C: bsr.w 0x0005d55c ; call 0005D55C FUN_0005d55c
0005D310: bsr.w 0x0005d5fc ; call 0005D5FC FUN_0005d5fc
0005D314: bsr.w 0x0005d5a2 ; call 0005D5A2 FUN_0005d5a2
0005D318: bsr.w 0x0005d57e ; call 0005D57E FUN_0005d57e
0005D31C: subq.w #0x1,(0x24,A0)
0005D320: bpl.b 0x0005d2b4
0005D322: move.w #0x2,(0x22,A0)
0005D328: move.w #0x28,(0x24,A0)
0005D32E: rts
0005D330: jsr 0x00005a56.l ; call 00005A56 project_entity_screen_position
0005D336: bsr.w 0x0005d55c ; call 0005D55C FUN_0005d55c
0005D33A: bsr.w 0x0005d65a ; call 0005D65A FUN_0005d65a
0005D33E: bsr.w 0x0005d5fc ; call 0005D5FC FUN_0005d5fc
0005D342: bsr.w 0x0005d5a2 ; call 0005D5A2 FUN_0005d5a2
0005D346: bsr.w 0x0005d57e ; call 0005D57E FUN_0005d57e
0005D34A: cmpi.w #0x3,(0x18,A0)
0005D350: bne.w 0x0005d2b4
0005D354: movea.l #0xff0700,A1
0005D35A: lea (0xff0500).l,A1 ; data 00ff0500
0005D360: move.w #0x5,D7w
0005D364: btst.b #0x0,(0x10,A1)
0005D36A: beq.w 0x0005d38e
0005D36E: btst.b #0x4,(0x10,A1)
0005D374: bne.w 0x0005d38e
0005D378: btst.b #0x6,(0x10,A1)
0005D37E: bne.w 0x0005d38e
0005D382: tst.b (0x4c,A1)
0005D386: beq.w 0x0005d38e
0005D38A: bra.w 0x0005d3e6 ; call 0005D3E6 FUN_0005d3e6
0005D38E: adda.w #0x100,A1
0005D392: dbf D7w,0x0005d364
0005D396: subq.w #0x1,(0x24,A0)
0005D39A: bmi.w 0x0005d3d4
0005D39E: movea.l #0xff0400,A1
0005D3A4: move.b #0x1,(0x9d,A0)
0005D3AA: move.b #0x20,(0x77,A1)
0005D3B0: move.b #0x20,(0x7c,A1)
0005D3B6: move.b #0x20,(0x80,A1)
0005D3BC: bsr.w 0x0005d3e6 ; call 0005D3E6 FUN_0005d3e6
0005D3C0: move.b #0x4,(0x4c,A1)
0005D3C6: move.b #0x4,(0x9c,A1)
0005D3CC: move.b #0x21,(0x10,A1)
0005D3D2: rts
0005D3D4: move.l #0x5bac4,(0x00ffdd2c).l
0005D3DE: jsr 0x0001e2e0.l ; call 0001E2E0 FUN_0001e2e0
0005D3E4: rts
0005D3E6: bset.b #0x0,(0x11,A1)
0005D3EC: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP)
0005D3F0: jsr 0x00015930.l ; call 00015930 FUN_00015930
0005D3F6: lea (0xff0400).l,A0 ; data 00ff0400
0005D3FC: jsr 0x0000aff2.l ; call 0000AFF2 prepare_entity_auxiliary_behavior
0005D402: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0005D406: rts
0005D408: movem.l {  A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0},-(SP) ; data 00fff11e
0005D40C: bsr.w 0x0005d416 ; call 0005D416 FUN_0005d416
0005D410: movem.l (SP)+,{ D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6}
0005D414: rts
0005D416: cmpi.w #0x32,(0x24,A0)
0005D41C: bgt.w 0x0005d53a
0005D420: cmpi.w #0x14,(0x24,A0)
0005D426: beq.w 0x0005d4da
0005D42A: cmpi.w #0x16,(0x24,A0)
0005D430: beq.w 0x0005d516
0005D434: cmpi.w #0x18,(0x24,A0)
0005D43A: beq.w 0x0005d506
0005D43E: cmpi.w #0x19,(0x24,A0)
0005D444: beq.w 0x0005d4ec
0005D448: cmpi.w #0x1a,(0x24,A0)
0005D44E: beq.w 0x0005d506
0005D452: cmpi.w #0x1b,(0x24,A0)
0005D458: beq.w 0x0005d4ec
0005D45C: cmpi.w #0x1d,(0x24,A0)
0005D462: beq.w 0x0005d506
0005D466: cmpi.w #0x1e,(0x24,A0)
0005D46C: beq.w 0x0005d4ec
0005D470: cmpi.w #0x20,(0x24,A0)
0005D476: beq.w 0x0005d516
0005D47A: cmpi.w #0x21,(0x24,A0)
0005D480: beq.w 0x0005d516
0005D484: cmpi.w #0x23,(0x24,A0)
0005D48A: beq.w 0x0005d516
0005D48E: cmpi.w #0x24,(0x24,A0)
0005D494: beq.w 0x0005d506
0005D498: cmpi.w #0x25,(0x24,A0)
0005D49E: beq.w 0x0005d4ec
0005D4A2: cmpi.w #0x26,(0x24,A0)
0005D4A8: beq.w 0x0005d4ec
0005D4AC: cmpi.w #0x27,(0x24,A0)
0005D4B2: beq.w 0x0005d506
0005D4B6: cmpi.w #0x28,(0x24,A0)
0005D4BC: beq.w 0x0005d4ec
0005D4C0: cmpi.w #0x29,(0x24,A0)
0005D4C6: beq.w 0x0005d4ec
0005D4CA: lea (0x1d494e).l,A0 ; data 001d494e
0005D4D0: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
0005D4D6: bra.w 0x0005d522
0005D4DA: move.w #0x2c,D0w
0005D4DE: jsr 0x000530de.l ; call 000530DE FUN_000530de
0005D4E4: bsr.w 0x0005d6bc ; call 0005D6BC FUN_0005d6bc
0005D4E8: bra.w 0x0005d522
0005D4EC: move.w #0x39,D0w
0005D4F0: jsr 0x000530de.l ; call 000530DE FUN_000530de
0005D4F6: addq.w #0x8,(0x00ffdd08).l
0005D4FC: addq.w #0x8,(0x00ffdd0c).l
0005D502: bra.w 0x0005d522
0005D506: subq.w #0x8,(0x00ffdd08).l
0005D50C: subq.w #0x8,(0x00ffdd0c).l
0005D512: bra.w 0x0005d522
0005D516: lea (0x5d53c).l,A0 ; data 0005d53c
0005D51C: jsr 0x000566d8.l ; call 000566D8 FUN_000566d8
0005D522: bset.b #0x2,(0x00ffef14).l
0005D52A: bset.b #0x3,(0x00ffef14).l
0005D532: bset.b #0x0,(0x00ffef14).l
0005D53A: rts
0005D55C: subq.w #0x1,(0x2,A0)
0005D560: bpl.w 0x0005d57c
0005D564: move.w #0xa,(0x2,A0)
0005D56A: addq.w #0x1,(0x18,A0)
0005D56E: cmpi.w #0x8,(0x18,A0)
0005D574: blt.w 0x0005d57c
0005D578: clr.w (0x18,A0)
0005D57C: rts
0005D57E: move.w (0xa,A0),D1w
0005D582: move.w (0xc,A0),D2w
0005D586: move.w (0x18,A0),D0w
0005D58A: add.w D0w,D0w
0005D58C: add.w D0w,D0w
0005D58E: lea (0x1fea0e).l,A2 ; data 001fea0e
0005D594: movea.l (0x0,A2,D0w*0x1),A2 ; data 001fea0e
0005D598: move.w #0x4000,D6w
0005D59C: jmp 0x00001378.l ; call 00001378 FUN_00001378
0005D5A2: subq.w #0x1,(0x6,A0)
0005D5A6: bpl.w 0x0005d5c2
0005D5AA: move.w #0x14,(0x6,A0)
0005D5B0: addq.w #0x1,(0x16,A0)
0005D5B4: cmpi.w #0x9,(0x16,A0)
0005D5BA: blt.w 0x0005d5c2
0005D5BE: clr.w (0x16,A0)
0005D5C2: move.w (0x18,A0),D0w
0005D5C6: add.w D0w,D0w
0005D5C8: add.w D0w,D0w
0005D5CA: lea (0x1fe926).l,A2 ; data 001fe926
0005D5D0: move.w (0x0,A2,D0w*0x1),D1w ; data 001fe926
0005D5D4: add.w (0xa,A0),D1w
0005D5D8: move.w (0x2,A2,D0w*0x1),D2w ; data 001fe928
0005D5DC: add.w (0xc,A0),D2w
0005D5E0: move.w (0x16,A0),D0w
0005D5E4: add.w D0w,D0w
0005D5E6: add.w D0w,D0w
0005D5E8: lea (0x1fea2e).l,A2 ; data 001fea2e
0005D5EE: movea.l (0x0,A2,D0w*0x1),A2 ; data 001fea2e
0005D5F2: move.w #0x4000,D6w
0005D5F6: jmp 0x00001378.l ; call 00001378 FUN_00001378
0005D5FC: subq.w #0x1,(0x1a,A0)
0005D600: bpl.w 0x0005d61c
0005D604: move.w #0xa,(0x1a,A0)
0005D60A: addq.w #0x1,(0x1c,A0)
0005D60E: cmpi.w #0xa,(0x1c,A0)
0005D614: blt.w 0x0005d61c
0005D618: clr.w (0x1c,A0)
0005D61C: move.w (0x18,A0),D0w
0005D620: add.w D0w,D0w
0005D622: add.w D0w,D0w
0005D624: lea (0x1fe926).l,A2 ; data 001fe926
0005D62A: move.w (0x0,A2,D0w*0x1),D1w ; data 001fe926
0005D62E: add.w (0xa,A0),D1w
0005D632: move.w (0x2,A2,D0w*0x1),D2w ; data 001fe928
0005D636: add.w (0xc,A0),D2w
0005D63A: subi.w #0x18,D2w
0005D63E: move.w (0x1c,A0),D0w
0005D642: add.w D0w,D0w
0005D644: add.w D0w,D0w
0005D646: lea (0x1fe9e6).l,A2 ; data 001fe9e6
0005D64C: movea.l (0x0,A2,D0w*0x1),A2 ; data 001fe9e6
0005D650: move.w #0x4000,D6w
0005D654: jmp 0x00001378.l ; call 00001378 FUN_00001378
0005D65A: subq.w #0x1,(0x20,A0)
0005D65E: bpl.w 0x0005d67a
0005D662: move.w #0x4,(0x20,A0)
0005D668: addq.w #0x1,(0x1e,A0)
0005D66C: cmpi.w #0x28,(0x1e,A0)
0005D672: blt.w 0x0005d67a
0005D676: clr.w (0x1e,A0)
0005D67A: move.w (0x18,A0),D0w
0005D67E: add.w D0w,D0w
0005D680: add.w D0w,D0w
0005D682: lea (0x1fe926).l,A2 ; data 001fe926
0005D688: move.w (0x0,A2,D0w*0x1),D1w ; data 001fe926
0005D68C: add.w (0xa,A0),D1w
0005D690: move.w (0x2,A2,D0w*0x1),D2w ; data 001fe928
0005D694: add.w (0xc,A0),D2w
0005D698: addi.w #0x30,D2w
0005D69C: addi.w #0x12,D1w
0005D6A0: move.w (0x1e,A0),D0w
0005D6A4: add.w D0w,D0w
0005D6A6: add.w D0w,D0w
0005D6A8: lea (0x1fe946).l,A2 ; data 001fe946
0005D6AE: movea.l (0x0,A2,D0w*0x1),A2 ; data 001fe946
0005D6B2: move.w #0x4000,D6w
0005D6B6: jmp 0x00001378.l ; call 00001378 FUN_00001378
0005D6BC: jsr 0x00015a8c.l ; call 00015A8C find_first_available_entity_slot
0005D6C2: bcs.w 0x0005d744
0005D6C6: lea (0x7361a).l,A0 ; data 0007361a
0005D6CC: move.w #0x7f,D7w
0005D6D0: move.l A1,-(SP) ; data Stack[-0x4]
0005D6D2: move.w (A0)+,(A1)+
0005D6D4: dbf D7w,0x0005d6d2
0005D6D8: movea.l (SP)+,A1
0005D6DA: move.l #0x15390,(0x3a,A1) ; data 00015390
0005D6E2: move.w #0x2000,(0x2c,A1) ; data 00002000
0005D6E8: move.w #0x1200,(A1) ; data 00001200
0005D6EC: move.w #0x2c0,(0x4,A1)
0005D6F2: move.w D0w,(0x3e,A1)
0005D6F6: move.w #0x2,(0xe,A1)
0005D6FC: bset.b #0x7,(0x10,A1)
0005D702: bset.b #0x0,(0x10,A1)
0005D708: move.w #0xc,(0x16,A1)
0005D70E: move.b #0x3,(0x41,A1)
0005D714: clr.w (0x44,A1)
0005D718: move.w #-0x1,(0x46,A1)
0005D71E: jsr 0x0001a2cc.l ; call 0001A2CC allocate_first_free_effect_slot
0005D724: bcc.w 0x0005d732
0005D728: bclr.b #0x0,(0x10,A1)
0005D72E: bra.w 0x0005d744
0005D732: move.w D0w,(0x30,A1)
0005D736: move.w D1w,(0x20,A1)
0005D73A: move.w A2w,(0x26,A1)
0005D73E: move.w #0x2a,(0x38,A1)
0005D744: rts
0005D746: move.w #0x1,(0x00fffc0a).l
0005D74E: bsr.w 0x0005da50 ; call 0005DA50 setup_pre_title_resources
0005D752: move.b #0x1,(0x00ffe18a).l
0005D75A: move.w #0x41,(0x00ffdd2a).l
0005D762: move.w #0x119,(0x00ffdd2c).l
0005D76A: move.w #0x1a8,(0x00ffdd2e).l
0005D772: clr.w (0x00ffdd3c).l
0005D778: move.w #0x150,D7w
0005D77C: move.w #0x9,(0x00ffdf92).l
0005D784: move.w D7w,-(SP) ; data Stack[-0x2]
0005D786: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005D78C: tst.w (0x00ffdd3c).l
0005D792: beq.w 0x0005d79a
0005D796: bsr.w 0x0005d8ba ; call 0005D8BA step_pre_title_logo_strobe
0005D79A: bsr.w 0x0005dac4 ; call 0005DAC4 fill_boot_horizontal_scroll
0005D79E: bsr.w 0x0005d938 ; call 0005D938 advance_pre_title_animation
0005D7A2: cmpi.w #0x160,(0x00ffdd2e).l
0005D7AA: beq.w 0x0005d7c8
0005D7AE: subq.w #0x1,(0x00ffdd2e).l
0005D7B4: cmpi.w #0x160,(0x00ffdd2e).l
0005D7BC: bne.w 0x0005d838
0005D7C0: move.w #0x14,(0x00ffdd30).l
0005D7C8: subq.w #0x1,(0x00ffdd30).l
0005D7CE: bpl.w 0x0005d838
0005D7D2: addq.w #0x1,(0x00ffdd28).l
0005D7D8: move.w (0x00ffdd28).l,D0w
0005D7DE: andi.w #0x1,D0w
0005D7E2: bne.w 0x0005d838
0005D7E6: cmpi.w #0x5,(0x00ffdd2a).l
0005D7EE: bgt.w 0x0005d7fc
0005D7F2: move.w #0x16,D0w
0005D7F6: jsr 0x000530d2.l ; call 000530D2 FUN_000530d2
0005D7FC: subq.w #0x1,(0x00ffdd2a).l
0005D802: bmi.w 0x0005d838
0005D806: bne.w 0x0005d81c
0005D80A: move.w #0x16,D0w
0005D80E: jsr 0x000530d2.l ; call 000530D2 FUN_000530d2
0005D814: move.w #0x1,(0x00ffdd3c).l
0005D81C: subq.w #0x1,(0x00ffdd0a).l
0005D822: cmpi.w #0x5,(0x00ffdd2a).l
0005D82A: blt.w 0x0005d838
0005D82E: move.w #0x5c,D0w
0005D832: jsr 0x000530de.l ; call 000530DE FUN_000530de
0005D838: bset.b #0x1,(0x00ffef14).l
0005D840: bset.b #0x3,(0x00ffef14).l
0005D848: bset.b #0x2,(0x00ffef14).l
0005D850: move.w (SP)+,D7w
0005D852: dbf D7w,0x0005d784
0005D856: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005D85C: jsr 0x0000131c.l ; call 0000131C step_palette_transition
0005D862: bsr.w 0x0005d886 ; call 0005D886 FUN_0005d886
0005D866: move.w #0x78,D7w
0005D86A: move.w D7w,-(SP) ; data Stack[-0x2]
0005D86C: jsr 0x00000bd2.l ; call 00000BD2 wait_for_vblank
0005D872: move.w (SP)+,D7w
0005D874: dbf D7w,0x0005d86a
0005D878: jsr 0x0000195c.l ; call 0000195C sample_controller_input
0005D87E: jsr 0x0000131c.l ; call 0000131C step_palette_transition
0005D884: rts
0005D886: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0005D88C: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0005D892: jsr 0x00000e34.l ; call 00000E34 FUN_00000e34
0005D898: moveq #0xf,D7
0005D89A: moveq #0x4,D1
0005D89C: move.w #0x4000,D2w
0005D8A0: move.l #0xe518,D4
0005D8A6: lea (0x1ffac6).l,A1 ; data 001ffac6
0005D8AC: jsr 0x00001232.l ; call 00001232 FUN_00001232
0005D8B2: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
0005D8B8: rts
0005D8BA: lea (0x5dce6).l,A0 ; data 0005dce6
0005D8C0: subq.w #0x1,(0x00ffdf90).l
0005D8C6: bmi.w 0x0005d8e0
0005D8CA: cmpi.w #0x2,(0x00ffdf90).l
0005D8D2: bne.w 0x0005d936
0005D8D6: move.w #0x3,D0w
0005D8DA: jmp 0x0005b2e2.l ; call 0005B2E2 FUN_0005b2e2
0005D8E0: move.w #0x4,(0x00ffdf90).l
0005D8E8: addq.w #0x1,(0x00ffdf92).l
0005D8EE: cmpi.w #0x8,(0x00ffdf92).l
0005D8F6: bcs.w 0x0005d900
0005D8FA: clr.w (0x00ffdf92).l
0005D900: move.w (0x00ffdf92).l,D1w
0005D906: lsl.w #0x1,D1w
0005D908: lea (0xffd884).l,A1 ; data 00ffd884
0005D90E: move.w (0x0,A0,D1w*0x1),(A1)+
0005D912: move.w (0x2,A0,D1w*0x1),(A1)+
0005D916: move.w (0x4,A0,D1w*0x1),(A1)+
0005D91A: move.w (0x6,A0,D1w*0x1),(A1)+
0005D91E: move.w (0x8,A0,D1w*0x1),(A1)+
0005D922: move.w (0xa,A0,D1w*0x1),(A1)+
0005D926: move.w (0xc,A0,D1w*0x1),(A1)+
0005D92A: move.w (0xe,A0,D1w*0x1),(A1)
0005D92E: bset.b #0x0,(0x00ffef14).l
0005D936: rts
0005D938: move.w #0x1,(0x00ffdd02).l
0005D940: lea (0xffd986).l,A3 ; data 00ffd986
0005D946: move.w #0x2,D5w
0005D94A: lea (0x1d4e4).l,A2 ; data 0001d4e4
0005D950: move.w (0x00ffdd2c).l,D1w
0005D956: move.w (0x00ffdd2e).l,D2w
0005D95C: move.w #0x6000,D6w
0005D960: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005D966: lea (0x1d50e).l,A2 ; data 0001d50e
0005D96C: move.w (0x00ffdd2c).l,D1w
0005D972: move.w (0x00ffdd2e).l,D2w
0005D978: move.w #0x6000,D6w
0005D97C: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005D982: bsr.w 0x0005d996 ; call 0005D996 advance_pre_title_logo_phase
0005D986: subq.w #0x8,A3
0005D988: clr.b (0x3,A3)
0005D98C: move.w #0x1,(0x00ffd980).l
0005D994: rts
0005D996: tst.w (0x00ffdd30).l
0005D99C: bmi.w 0x0005d9a2
0005D9A0: rts
0005D9A2: tst.w (0x00ffdd3c).l
0005D9A8: beq.w 0x0005d9ae
0005D9AC: rts
0005D9AE: subq.w #0x1,(0x00ffe066).l
0005D9B4: bpl.w 0x0005d9d8
0005D9B8: move.w #0x3,(0x00ffe066).l
0005D9C0: addq.w #0x1,(0x00ffe064).l
0005D9C6: cmpi.w #0x6,(0x00ffe064).l
0005D9CE: blt.w 0x0005d9d8
0005D9D2: clr.w (0x00ffe064).l
0005D9D8: move.w (0x00ffe064).l,D0w
0005D9DE: lea (0x5da20).l,A2 ; data 0005da20
0005D9E4: lsl.w #0x2,D0w
0005D9E6: movea.l (0x0,A2,D0w*0x1),A2
0005D9EA: move.w #0x158,D1w
0005D9EE: move.w #0x120,D2w
0005D9F2: move.w #0x2000,D6w
0005D9F6: jsr 0x00001378.l ; call 00001378 FUN_00001378
0005D9FC: lea (0x5da38).l,A2 ; data 0005da38
0005DA02: move.w (0x00ffe064).l,D0w
0005DA08: lsl.w #0x2,D0w
0005DA0A: movea.l (0x0,A2,D0w*0x1),A2
0005DA0E: move.w #0xd8,D1w
0005DA12: move.w #0x120,D2w
0005DA16: move.w #0x2000,D6w
0005DA1A: jmp 0x00001378.l ; call 00001378 FUN_00001378
0005DA50: jsr 0x00000ffe.l ; call 00000FFE FUN_00000ffe
0005DA56: jsr 0x0000102a.l ; call 0000102A FUN_0000102a
0005DA5C: lea (0x5db10).l,A6 ; data 0005db10
0005DA62: jsr 0x00001406.l ; call 00001406 FUN_00001406
0005DA68: bsr.w 0x0005dae8 ; call 0005DAE8 FUN_0005dae8
0005DA6C: jsr 0x000176ba.l ; call 000176BA FUN_000176ba
0005DA72: lea (0xffbf40).l,A1 ; data 00ffbf40
0005DA78: movea.l #0x14c222,A3
0005DA7E: move.l #0x400,D0
0005DA84: jsr 0x00001530.l ; call 00001530 FUN_00001530
0005DA8A: bset.b #0x2,(0x00ffef14).l
0005DA92: bset.b #0x3,(0x00ffef14).l
0005DA9A: bset.b #0x0,(0x00ffef14).l
0005DAA2: jsr 0x000012f6.l ; call 000012F6 FUN_000012f6
0005DAA8: moveq #0xb,D7
0005DAAA: moveq #0x3,D1
0005DAAC: move.w #0x0,D2w
0005DAB0: move.l #0xe51a,D4
0005DAB6: lea (0x1ffa66).l,A1 ; data 001ffa66
0005DABC: jsr 0x00001232.l ; call 00001232 FUN_00001232
0005DAC2: rts
0005DAC4: lea (0xffb740).l,A0 ; data 00ffb740
0005DACA: move.w #0xdf,D7w
0005DACE: jsr 0x00000d44.l ; call 00000D44 advance_source_prng
0005DAD4: lsr.w #0x5,D0w
0005DAD6: andi.w #0x1f,D0w
0005DADA: move.w D0w,(A0)
0005DADC: adda.w #0x4,A0
0005DAE0: dbf D7w,0x0005dace
0005DAE4: bra.w 0x0005dafc ; call 0005DAFC FUN_0005dafc
0005DAE8: lea (0xffb740).l,A0 ; data 00ffb740
0005DAEE: move.w #0x1bf,D7w
0005DAF2: clr.w (A0)+
0005DAF4: dbf D7w,0x0005daf2
0005DAF8: bra.w 0x0005dafc ; call 0005DAFC FUN_0005dafc
0005DAFC: lea (0xffb740).l,A0 ; data 00ffb740
0005DB02: move.w #0x1bf,D7w
0005DB06: moveq #0x0,D0
0005DB08: jsr 0x00000e18.l ; call 00000E18 FUN_00000e18
0005DB0E: rts
000FF990: rts
00103C22: rts
