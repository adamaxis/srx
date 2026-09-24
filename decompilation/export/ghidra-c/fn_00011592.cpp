// Ghidra headless export
// Entry: 0x00011592
// Name: FUN_00011592
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x0000C0DA render_title_text_source
//   0x00011ABE FUN_00011abe
//   0x00012148 FUN_00012148
//   0x00012A8C FUN_00012a8c
//   0x00012C36 FUN_00012c36
//   0x00012E9E FUN_00012e9e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00011592(void)

{
  char cVar1;
  undefined1 uVar2;
  
  DAT_00ffdd02 = 1;
  FUN_00001378();
  _DAT_00ffd980 = 1;
  uVar2 = (DAT_00ffef14 & 2) == 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  FUN_00012c36();
  FUN_00012148();
  if (!(bool)uVar2) {
    cVar1 = FUN_00012a8c();
    if ((char)(cVar1 + -0x10) < '\0') {
      FUN_00012e9e();
    }
    FUN_00011abe();
    return;
  }
  render_title_text_source();
  return;
}


