// Ghidra headless export
// Entry: 0x000114A0
// Name: FUN_000114a0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x00011442 FUN_00011442
//   0x00011454 reset_menu_upload_cursor_and_upload


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000114a0(void)

{
  char cVar1;
  undefined1 uVar2;
  
  cVar1 = FUN_00011442();
  DAT_00fff0d0 = cVar1 + DAT_00fff0d0;
  if (DAT_00fff0d0 < '\0') {
    DAT_00fff112 = 0xff;
    DAT_00fff0d0 = '\x06';
  }
  if (DAT_00fff0d0 == '\a') {
    DAT_00fff0d0 = '\0';
  }
  if ((DAT_00fff0d0 == '\x02') && (-1 < cVar1)) {
    DAT_00fff112 = 0xff;
  }
  if (-1 < (char)(DAT_00fff0d0 + -2)) {
    reset_vdp_upload_state();
    reset_menu_upload_cursor_and_upload();
    return;
  }
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  _DAT_00ffd980 = 1;
  uVar2 = (DAT_00ffef14 & 2) == 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  FUN_00012c36();
  read_entity_action_code();
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


