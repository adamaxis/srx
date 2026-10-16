// Ghidra headless export
// Entry: 0x0001275C
// Name: clear_menu_slot
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 upload_field_asset_setup


void clear_menu_slot(void)

{
  char in_D0b;
  short unaff_D7w;
  int in_A0;
  int extraout_A0;
  
  if ((char)(in_D0b + -0x2c) < '\0') {
    if (DAT_00fff098 == -1) {
      *(undefined1 *)(in_A0 + 0x56) = 0x7f;
      *(undefined1 *)(in_A0 + 0x57) = 0;
      *(undefined1 *)(in_A0 + 0x4e) = 0;
      upload_field_asset_setup();
      in_A0 = extraout_A0;
    }
  }
  else if (((char)(in_D0b + -0x2b) == *(char *)(in_A0 + 0x76)) &&
          (*(char *)(in_A0 + 0x6e + (int)unaff_D7w) != '\0')) {
    *(undefined1 *)(in_A0 + 0x76) = 0;
  }
  *(undefined1 *)(in_A0 + 0x66 + (int)unaff_D7w) = 0;
  *(undefined1 *)(in_A0 + 0x6e + (int)unaff_D7w) = 0;
  return;
}


