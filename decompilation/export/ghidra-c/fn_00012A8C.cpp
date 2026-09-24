// Ghidra headless export
// Entry: 0x00012A8C
// Name: FUN_00012a8c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA render_title_text_source
//   0x00011352 FUN_00011352
//   0x00012148 FUN_00012148
//   0x00012A54 FUN_00012a54


undefined4 FUN_00012a8c(void)

{
  undefined4 in_D0;
  short sVar1;
  char cVar2;
  short unaff_D7w;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int iVar3;
  
  sVar1 = FUN_00012a54();
  if (-1 < *(int *)((int)&PTR_DAT_000d455e + (int)(short)((sVar1 + -1) * 0x30))) {
    unaff_D7w = 8;
    FUN_00001232();
  }
  render_title_text_source();
  cVar2 = FUN_00012148();
  if ((char)(cVar2 + -0xd) < '\0') {
    iVar3 = extraout_A0;
    if ((*(byte *)(extraout_A0 + 0x6e + (int)unaff_D7w) & 2) != 0) {
      cVar2 = FUN_00001232();
      iVar3 = extraout_A0_00;
    }
    if ((*(byte *)(iVar3 + 0x6e + (int)unaff_D7w) & 4) != 0) {
      cVar2 = FUN_00001232();
      iVar3 = extraout_A0_01;
    }
    if ((*(byte *)(iVar3 + 0x6e + (int)unaff_D7w) & 1) != 0) {
      cVar2 = FUN_00001232();
      iVar3 = extraout_A0_02;
    }
    if ((*(byte *)(iVar3 + 0x6e + (int)unaff_D7w) & 8) != 0) {
      cVar2 = FUN_00001232();
      iVar3 = extraout_A0_03;
    }
    if ((*(byte *)(iVar3 + 0x6e + (int)unaff_D7w) & 0x10) != 0) {
      cVar2 = FUN_00001232();
      iVar3 = extraout_A0_04;
    }
    if ((*(byte *)(iVar3 + 0x6e + (int)unaff_D7w) & 0x20) != 0) {
      cVar2 = FUN_00001232();
    }
  }
  if (-1 < (char)(cVar2 + -0xd)) {
    if ((-1 < (char)(cVar2 + -0x14)) && (cVar2 != '\x14')) {
      if ((char)(cVar2 + -0x18) < '\0') {
        return in_D0;
      }
      if (-1 < (char)(cVar2 + -0x29)) {
        if ((char)(cVar2 + -0x2c) < '\0') {
          return in_D0;
        }
        DAT_00fff0b1 = 0xff;
        FUN_00011352();
        return in_D0;
      }
    }
    FUN_00012148();
  }
  render_title_text_source();
  FUN_00001232();
  return in_D0;
}


