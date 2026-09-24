// Ghidra headless export
// Entry: 0x000059EA
// Name: FUN_000059ea
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void FUN_000059ea(void)

{
  char cVar1;
  int in_A0;
  
  cVar1 = *(char *)(in_A0 + 6) + -1;
  *(char *)(in_A0 + 6) = cVar1;
  if (cVar1 < '\0') {
    *(undefined1 *)(in_A0 + 6) = 3;
    *(char *)(in_A0 + 7) = *(char *)(in_A0 + 7) + '\x01';
    if ('\x03' < *(char *)(in_A0 + 7)) {
      *(undefined1 *)(in_A0 + 7) = 0;
    }
  }
  build_title_sprite_records();
  return;
}


