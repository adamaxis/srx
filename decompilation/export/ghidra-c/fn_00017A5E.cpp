// Ghidra headless export
// Entry: 0x00017A5E
// Name: FUN_00017a5e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00017C3C FUN_00017c3c
//   0x00017E04 refresh_hud_grid_if_ready
//   0x00019DAC merge_menu_pending_flags


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
    merge_menu_pending_flags();
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
        refresh_hud_grid_if_ready();
      }
      iVar3 = (uint)bVar1 * 2;
      if (-1 < *(char *)((int)&PTR_DAT_000e31a7 + iVar3)) {
        render_encoded_title_text();
        if ((short)(unaff_D3w + -2) < 0) {
          render_encoded_title_text();
        }
        else {
          build_grouped_decimal_glyphs();
        }
        if (-1 < *(char *)((int)&PTR_DAT_000e31a7 + iVar3 + 1)) {
          render_encoded_title_text();
          if (-1 < (short)(unaff_D3w + -2)) {
            build_grouped_decimal_glyphs();
            return;
          }
          render_encoded_title_text();
        }
      }
      return;
    }
  }
  render_encoded_title_text();
  return;
}


