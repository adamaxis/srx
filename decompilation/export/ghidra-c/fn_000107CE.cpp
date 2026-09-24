// Ghidra headless export
// Entry: 0x000107CE
// Name: FUN_000107ce
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA render_title_text_source
//   0x0001055E FUN_0001055e
//   0x000105E6 FUN_000105e6
//   0x00010854 FUN_00010854


void FUN_000107ce(void)

{
  char cVar1;
  int iVar2;
  char in_D0b;
  
  iVar2 = DAT_00fffc04;
  cVar1 = in_D0b + *(char *)(DAT_00fffc04 + 0x4d);
  *(char *)(DAT_00fffc04 + 0x4d) = cVar1;
  if (cVar1 < '\0') {
    *(undefined1 *)(iVar2 + 0x4d) = 0;
  }
  else if ('\x04' < *(char *)(iVar2 + 0x4d)) {
    *(undefined1 *)(iVar2 + 0x4d) = 4;
  }
  FUN_00001232();
  render_title_text_source();
  FUN_00010854();
  FUN_000105e6();
  FUN_0001055e();
  FUN_0001055e();
  return;
}


