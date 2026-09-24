// Ghidra headless export
// Entry: 0x0001A12E
// Name: advance_menu_countdown
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019F72 finalize_menu_slot_code_five


void advance_menu_countdown(void)

{
  bool bVar1;
  char cVar2;
  
  if (DAT_00fff0d2 != '\0') {
    cVar2 = DAT_00fff0d2 + -1;
    bVar1 = DAT_00fff0d2 < '\x01';
    DAT_00fff0d2 = cVar2;
    if (cVar2 == '\0' || bVar1) {
      finalize_menu_slot_code_five();
    }
  }
  return;
}


