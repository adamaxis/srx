// Ghidra headless export
// Entry: 0x0000EA36
// Name: FUN_0000ea36
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000EAD2 FUN_0000ead2


void FUN_0000ea36(void)

{
  ushort unaff_D5w;
  undefined2 unaff_D6w;
  bool bVar1;
  undefined2 uStack_6;
  undefined1 auStack_4 [4];
  
  if ((DAT_00ffef10._0_1_ & 0x20) != 0) {
    bVar1 = unaff_D5w < 0xc;
    FUN_0000ead2();
    register0x0000003c = (BADSPACEBASE *)auStack_4;
    unaff_D6w = uStack_6;
    if (bVar1) {
      return;
    }
  }
  if ((DAT_00ffef10._1_1_ & 0x20) != 0) {
    *(undefined2 *)((int)register0x0000003c + -4) = unaff_D6w;
    *(ushort *)((int)register0x0000003c + -8) = unaff_D5w;
    bVar1 = 0xfff3 < unaff_D5w;
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0xea76;
    FUN_0000ead2();
    unaff_D5w = *(ushort *)((int)register0x0000003c + -8);
    unaff_D6w = *(undefined2 *)((int)register0x0000003c + -6);
    register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -4);
    if (bVar1) {
      return;
    }
  }
  if ((DAT_00ffef10._2_1_ & 0x20) != 0) {
    *(undefined2 *)((int)register0x0000003c + -4) = unaff_D6w;
    *(ushort *)((int)register0x0000003c + -8) = unaff_D5w;
    bVar1 = unaff_D5w < 0xc;
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0xea9a;
    FUN_0000ead2();
    unaff_D5w = *(ushort *)((int)register0x0000003c + -8);
    unaff_D6w = *(undefined2 *)((int)register0x0000003c + -6);
    register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -4);
    if (bVar1) {
      return;
    }
  }
  if (((byte)DAT_00ffef10 & 0x20) != 0) {
    *(undefined2 *)((int)register0x0000003c + -4) = unaff_D6w;
    *(ushort *)((int)register0x0000003c + -8) = unaff_D5w;
    bVar1 = 0xfff3 < unaff_D5w;
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0xeabe;
    FUN_0000ead2();
    if (bVar1) {
      return;
    }
  }
  return;
}


