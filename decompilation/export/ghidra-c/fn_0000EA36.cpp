// Ghidra headless export
// Entry: 0x0000EA36
// Name: FUN_0000ea36
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000EAD2 evaluate_terrain_direction_boundary


void FUN_0000ea36(void)

{
  ushort unaff_D5w;
  undefined2 unaff_D6w;
  bool bVar1;
  undefined2 uStack_6;
  undefined1 auStack_4 [4];
  
  if ((DAT_00ffef10 & 0x20) != 0) {
    bVar1 = unaff_D5w < 0xc;
    evaluate_terrain_direction_boundary();
    register0x0000003c = (BADSPACEBASE *)auStack_4;
    unaff_D6w = uStack_6;
    if (bVar1) {
      return;
    }
  }
  if ((DAT_00ffef11 & 0x20) != 0) {
    *(undefined2 *)((int)register0x0000003c + -4) = unaff_D6w;
    *(ushort *)((int)register0x0000003c + -8) = unaff_D5w;
    bVar1 = 0xfff3 < unaff_D5w;
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0xea76;
    evaluate_terrain_direction_boundary();
    unaff_D5w = *(ushort *)((int)register0x0000003c + -8);
    unaff_D6w = *(undefined2 *)((int)register0x0000003c + -6);
    register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -4);
    if (bVar1) {
      return;
    }
  }
  if ((DAT_00ffef12 & 0x20) != 0) {
    *(undefined2 *)((int)register0x0000003c + -4) = unaff_D6w;
    *(ushort *)((int)register0x0000003c + -8) = unaff_D5w;
    bVar1 = unaff_D5w < 0xc;
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0xea9a;
    evaluate_terrain_direction_boundary();
    unaff_D5w = *(ushort *)((int)register0x0000003c + -8);
    unaff_D6w = *(undefined2 *)((int)register0x0000003c + -6);
    register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -4);
    if (bVar1) {
      return;
    }
  }
  if ((DAT_00ffef13 & 0x20) != 0) {
    *(undefined2 *)((int)register0x0000003c + -4) = unaff_D6w;
    *(ushort *)((int)register0x0000003c + -8) = unaff_D5w;
    bVar1 = 0xfff3 < unaff_D5w;
    *(undefined4 *)((int)register0x0000003c + -0xc) = 0xeabe;
    evaluate_terrain_direction_boundary();
    if (bVar1) {
      return;
    }
  }
  return;
}


