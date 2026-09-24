// Ghidra headless export
// Entry: 0x00001232
// Name: upload_hud_tile_block
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined8 upload_hud_tile_block(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D2w;
  short unaff_D7w;
  short sVar1;
  ushort *extraout_A1;
  ushort *puVar2;
  
  do {
    FUN_00000e9c();
    puVar2 = extraout_A1;
    sVar1 = unaff_D7w;
    do {
      VDP_DATA = unaff_D2w | *puVar2;
      sVar1 = sVar1 + -1;
      puVar2 = puVar2 + 1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


