// Ghidra headless export
// Entry: 0x0000C376
// Name: move_wrapped_menu_cursor
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C3B2 FUN_0000c3b2


void move_wrapped_menu_cursor(void)

{
  char cVar1;
  char unaff_D2b;
  char unaff_D3b;
  bool bVar2;
  
  FUN_0000c3b2();
  bVar2 = SCARRY1(unaff_D2b,DAT_00fff0dc);
  cVar1 = unaff_D2b + DAT_00fff0dc;
  DAT_00fff0dc = unaff_D3b;
  if ((bVar2 == cVar1 < '\0') && (DAT_00fff0dc = cVar1, (char)(unaff_D3b - cVar1) < '\0')) {
    DAT_00fff0dc = '\0';
  }
  FUN_0000c3b2();
  DAT_00fff0fc = 0;
  return;
}


