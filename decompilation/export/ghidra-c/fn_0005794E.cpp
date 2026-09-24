// Ghidra headless export
// Entry: 0x0005794E
// Name: FUN_0005794e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D94 FUN_00000d94
//   0x0000C0DA render_title_text_source
//   0x00057A44 FUN_00057a44
//   0x00057A6E FUN_00057a6e
//   0x00057B8C FUN_00057b8c
//   0x00057C5E FUN_00057c5e


void FUN_0005794e(void)

{
  char *pcVar1;
  uint in_D0;
  int extraout_D1;
  int extraout_D1_00;
  int extraout_D1_01;
  short unaff_D6w;
  short sVar2;
  int in_A0;
  char *extraout_A0;
  char *extraout_A0_00;
  int iVar3;
  
  if (*(char *)((uint)DAT_00fff0d0 * 3 + (in_D0 & 0xffff) * 0x18 + in_A0) < '\0') {
    DAT_00fff112 = 0xff;
    return;
  }
  FUN_00057c5e();
  if (*extraout_A0 == 'E') {
    FUN_00057a6e();
    iVar3 = extraout_D1;
  }
  else {
    sVar2 = 7;
    iVar3 = DAT_00fffc04;
    while (*(char *)(iVar3 + 0x66) != '\0') {
      iVar3 = iVar3 + 1;
      sVar2 = sVar2 + -1;
      if (sVar2 == -1) {
        render_title_text_source();
        FUN_00000d94();
        return;
      }
    }
    unaff_D6w = -(sVar2 + -7);
    FUN_00057a44();
    iVar3 = extraout_D1_00;
  }
  if (DAT_00fffbfe < iVar3) {
LAB_000579dc:
    render_title_text_source();
    FUN_00000d94();
    return;
  }
  FUN_00057b8c();
  iVar3 = DAT_00fffc04;
  if (DAT_00fff0dc == '\0') {
    if (*extraout_A0_00 == 'E') {
      *(char *)(DAT_00fffc04 + 0x4f) = *(char *)(DAT_00fffc04 + 0x4f) + '\x01';
      if (-1 < (char)(*(char *)(iVar3 + 0x4f) + -0x15)) {
        *(undefined1 *)(iVar3 + 0x4f) = 0x14;
        FUN_00057c5e();
        goto LAB_000579dc;
      }
    }
    else {
      *(char *)(DAT_00fffc04 + 0x66 + (int)unaff_D6w) = *extraout_A0_00;
      pcVar1 = (char *)(iVar3 + 0x66 + (int)unaff_D6w);
      *pcVar1 = *pcVar1 + '\x01';
      *(char *)(iVar3 + 0x6e + (int)unaff_D6w) = extraout_A0_00[1];
    }
    DAT_00fffbfe = DAT_00fffbfe - extraout_D1_01;
  }
  return;
}


