// Ghidra headless export
// Entry: 0x000044BC
// Name: FUN_000044bc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng


void FUN_000044bc(void)

{
  undefined2 in_D0w;
  ushort uVar1;
  undefined2 in_D1w;
  undefined2 *unaff_D4;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *in_A1;
  undefined2 *puVar4;
  int extraout_A1;
  undefined2 unaff_A2w;
  undefined2 *unaff_A3;
  int *unaff_A5;
  byte *pbVar5;
  
  sVar2 = 0x7f;
  puVar3 = &DAT_0007361a;
  puVar4 = in_A1;
  do {
    *puVar4 = *puVar3;
    sVar2 = sVar2 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar2 != -1);
  in_A1[0x1f] = DAT_00ffdd40;
  *(undefined1 *)(in_A1 + 8) = 5;
  in_A1[0x10] = in_D1w;
  in_A1[0x13] = unaff_A2w;
  *(undefined2 **)(in_A1 + 0x70) = unaff_A3;
  *in_A1 = *unaff_A3;
  in_A1[0x4b] = *in_A1;
  sVar2 = DAT_00ffdd42 + unaff_A3[1];
  in_A1[2] = sVar2;
  in_A1[0x4c] = sVar2;
  in_A1[7] = *unaff_D4;
  in_A1[0x16] = unaff_D4[1];
  *(undefined1 *)(in_A1 + 0x26) = *(undefined1 *)(unaff_D4 + 2);
  *(undefined1 *)((int)in_A1 + 0x4d) = *(undefined1 *)((int)unaff_D4 + 5);
  *(undefined1 *)(in_A1 + 0x27) = *(undefined1 *)(unaff_D4 + 3);
  *(undefined1 *)((int)in_A1 + 0x4f) = *(undefined1 *)((int)unaff_D4 + 7);
  *(undefined1 *)(in_A1 + 0x2b) = *(undefined1 *)(unaff_D4 + 4);
  *(undefined1 *)((int)in_A1 + 0x57) = *(undefined1 *)((int)unaff_D4 + 9);
  *(undefined4 *)(in_A1 + 0x2c) = *(undefined4 *)(unaff_D4 + 5);
  *(undefined4 *)(in_A1 + 0x2e) = *(undefined4 *)(unaff_D4 + 7);
  *(undefined4 *)(in_A1 + 0x30) = *(undefined4 *)(unaff_D4 + 9);
  in_A1[0x32] = unaff_D4[0xb];
  *(undefined4 *)(in_A1 + 0x33) = *(undefined4 *)(unaff_D4 + 0xc);
  *(undefined4 *)(in_A1 + 0x35) = *(undefined4 *)(unaff_D4 + 0xe);
  *(undefined4 *)(in_A1 + 0x37) = *(undefined4 *)(unaff_D4 + 0x10);
  *(undefined4 *)(in_A1 + 0x39) = *(undefined4 *)(unaff_D4 + 0x12);
  *(undefined1 *)(in_A1 + 0x3b) = *(undefined1 *)(unaff_D4 + 0x14);
  *(undefined1 *)((int)in_A1 + 0x77) = *(undefined1 *)((int)unaff_D4 + 0x29);
  *(undefined1 *)(in_A1 + 0x3c) = *(undefined1 *)(unaff_D4 + 0x15);
  *(undefined1 *)((int)in_A1 + 0x79) = *(undefined1 *)((int)unaff_D4 + 0x2b);
  *(undefined1 *)(in_A1 + 0x3d) = *(undefined1 *)(unaff_D4 + 0x16);
  *(undefined1 *)((int)in_A1 + 0x7b) = *(undefined1 *)((int)unaff_D4 + 0x2d);
  *(undefined1 *)(in_A1 + 0x3e) = *(undefined1 *)(unaff_D4 + 0x17);
  *(undefined1 *)((int)in_A1 + 0x7d) = *(undefined1 *)((int)unaff_D4 + 0x2f);
  *(undefined1 *)(in_A1 + 0x3f) = *(undefined1 *)(unaff_D4 + 0x18);
  *(undefined1 *)((int)in_A1 + 0x7f) = *(undefined1 *)((int)unaff_D4 + 0x31);
  *(undefined1 *)(in_A1 + 0x40) = *(undefined1 *)(unaff_D4 + 0x19);
  *(undefined1 *)((int)in_A1 + 0x81) = *(undefined1 *)((int)unaff_D4 + 0x33);
  *(undefined1 *)(in_A1 + 0x41) = *(undefined1 *)(unaff_D4 + 0x1a);
  *(undefined1 *)((int)in_A1 + 0x83) = *(undefined1 *)((int)unaff_D4 + 0x35);
  *(undefined1 *)(in_A1 + 0x42) = *(undefined1 *)(unaff_D4 + 0x1b);
  *(undefined1 *)((int)in_A1 + 0x85) = *(undefined1 *)((int)unaff_D4 + 0x37);
  *(undefined1 *)(in_A1 + 0x43) = *(undefined1 *)(unaff_D4 + 0x1c);
  *(undefined1 *)((int)in_A1 + 0x87) = *(undefined1 *)((int)unaff_D4 + 0x39);
  *(undefined1 *)(in_A1 + 0x44) = *(undefined1 *)(unaff_D4 + 0x1d);
  *(undefined1 *)((int)in_A1 + 0x89) = *(undefined1 *)((int)unaff_D4 + 0x3b);
  *(undefined1 *)(in_A1 + 0x45) = *(undefined1 *)(unaff_D4 + 0x1e);
  *(undefined1 *)((int)in_A1 + 0x8b) = *(undefined1 *)((int)unaff_D4 + 0x3d);
  *(undefined1 *)(in_A1 + 0x46) = *(undefined1 *)(unaff_D4 + 0x1f);
  *(undefined1 *)((int)in_A1 + 0x8d) = *(undefined1 *)((int)unaff_D4 + 0x3f);
  *(undefined1 *)(in_A1 + 0x47) = *(undefined1 *)(unaff_D4 + 0x20);
  *(undefined1 *)(in_A1 + 0x4d) = *(undefined1 *)((int)unaff_D4 + 0x41);
  *(undefined1 *)((int)in_A1 + 0x9b) = *(undefined1 *)(unaff_D4 + 0x21);
  *(undefined1 *)(in_A1 + 0x4e) = *(undefined1 *)((int)unaff_D4 + 0x43);
  in_A1[0x48] = unaff_D4[0x22];
  in_A1[0x50] = unaff_D4[0x23];
  *(undefined1 *)((int)in_A1 + 0x8f) = *(undefined1 *)(unaff_D4 + 0x24);
  *(undefined1 *)((int)in_A1 + 0x9d) = *(undefined1 *)((int)unaff_D4 + 0x49);
  in_A1[0x18] = in_D0w;
  uVar1 = advance_source_prng();
  uVar1 = uVar1 & 0xf;
  if (DAT_00ffddc4 == 0x37) {
    uVar1 = 0x28;
  }
  *(ushort *)(extraout_A1 + 0xb6) = uVar1;
  if (DAT_00ffe162._0_1_ != '\0') {
    *(byte *)(extraout_A1 + 0x9e) = *(byte *)(extraout_A1 + 0x9e) | 0x40;
  }
  if (*unaff_A5 == 2) {
    for (pbVar5 = (byte *)(unaff_A5 + 1); *(short *)pbVar5 != 0; pbVar5 = pbVar5 + 2) {
      *(byte *)(extraout_A1 + (short)(ushort)*pbVar5) = pbVar5[1];
    }
  }
  return;
}


