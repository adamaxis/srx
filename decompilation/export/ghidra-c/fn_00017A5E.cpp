// Ghidra headless export
// Entry: 0x00017A5E
// Name: FUN_00017a5e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00017C3C FUN_00017c3c
//   0x00017E04 FUN_00017e04
//   0x00019DAC FUN_00019dac


void FUN_00017a5e(void)

{
  byte bVar1;
  char cVar2;
  short unaff_D3w;
  int iVar3;
  char cVar4;
  bool bVar5;
  char cVar6;
  
  bVar1 = (&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)];
  if ((char)bVar1 < '\0') {
    FUN_00019dac();
    *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) =
         *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) | 0x40;
  }
  else {
    cVar6 = '\0';
    cVar4 = (char)(&DAT_00ff110f)[(short)(DAT_00fff118 * 0x14)] < '\0';
    cVar2 = (&DAT_00ff110f)[(short)(DAT_00fff118 * 0x14)];
    bVar5 = cVar2 == '\0';
    if (('\0' < cVar2) && (FUN_00017c3c(), !bVar5 && cVar6 == cVar4)) {
      if (DAT_00ffdf84 != '\0') {
        FUN_00017e04();
      }
      iVar3 = (uint)bVar1 * 2;
      if (-1 < *(char *)((int)&PTR_DAT_000e31a7 + iVar3)) {
        render_title_text_source();
        if ((short)(unaff_D3w + -2) < 0) {
          render_title_text_source();
        }
        else {
          build_grouped_decimal_glyphs();
        }
        if (-1 < *(char *)((int)&PTR_DAT_000e31a7 + iVar3 + 1)) {
          render_title_text_source();
          if (-1 < (short)(unaff_D3w + -2)) {
            build_grouped_decimal_glyphs();
            return;
          }
          render_title_text_source();
        }
      }
      return;
    }
  }
  render_title_text_source();
  return;
}


