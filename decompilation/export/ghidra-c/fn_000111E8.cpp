// Ghidra headless export
// Entry: 0x000111E8
// Name: FUN_000111e8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA FUN_0000c0da
//   0x0000C216 FUN_0000c216
//   0x00011352 FUN_00011352
//   0x00011928 FUN_00011928
//   0x00011ABE FUN_00011abe
//   0x00012C36 FUN_00012c36
//   0x00012EAE FUN_00012eae


void FUN_000111e8(void)

{
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int iVar1;
  
  if (DAT_00fff112 == '\0') {
    FUN_00011928();
    return;
  }
  DAT_00fff112 = 0;
  FUN_00012c36();
  FUN_0000c0da();
  FUN_0000c0da();
  FUN_0000c0da();
  if (*(char *)(DAT_00fffc04 + 0x57) == '\0') {
    if ((*(ushort *)(DAT_00fffc04 + 0x90) & 8) == 0) {
      if ((*(ushort *)(DAT_00fffc04 + 0x90) & 4) == 0) {
        FUN_0000c0da();
        iVar1 = extraout_A0_01;
      }
      else {
        FUN_0000c0da();
        iVar1 = extraout_A0_00;
      }
    }
    else {
      FUN_0000c0da();
      iVar1 = extraout_A0;
    }
LAB_00011304:
    if (-1 < (char)(*(char *)(iVar1 + 0x57) + -0x10)) goto LAB_00011312;
  }
  else {
    if (-1 < *(char *)(DAT_00fffc04 + 0x56)) {
      FUN_0000c0da();
      iVar1 = extraout_A0_02;
      goto LAB_00011304;
    }
    FUN_0000c0da();
  }
  FUN_00012eae();
LAB_00011312:
  FUN_0000c216();
  FUN_0000c216();
  FUN_00011352();
  FUN_00011abe();
  return;
}


