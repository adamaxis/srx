// Ghidra headless export
// Entry: 0x0001CCDC
// Name: FUN_0001ccdc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019E32 FUN_00019e32
//   0x0001D048 FUN_0001d048
//   0x0001D0DE FUN_0001d0de


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ccdc(void)

{
  int extraout_A0;
  
  if (DAT_00ff15a4 == '\0') {
    return;
  }
  FUN_0001d048();
  FUN_0001d0de();
  build_title_sprite_records();
  if (*(char *)(extraout_A0 + 6) == '\0') {
    _DAT_00ffe0d6 = 0;
    FUN_00019e32();
  }
  return;
}


