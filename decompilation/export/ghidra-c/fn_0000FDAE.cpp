// Ghidra headless export
// Entry: 0x0000FDAE
// Name: FUN_0000fdae
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block


void FUN_0000fdae(void)

{
  short sVar1;
  
  sVar1 = 1;
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


