// Ghidra headless export
// Entry: 0x00058B1E
// Name: FUN_00058b1e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block


void FUN_00058b1e(void)

{
  short sVar1;
  
  sVar1 = 5;
  do {
    upload_hud_tile_block();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 1;
  do {
    upload_hud_tile_block();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


