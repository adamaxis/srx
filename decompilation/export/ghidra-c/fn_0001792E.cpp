// Ghidra headless export
// Entry: 0x0001792E
// Name: FUN_0001792e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x00017C3C FUN_00017c3c
//   0x00056506 FUN_00056506


void FUN_0001792e(void)

{
  short sVar1;
  short sVar2;
  int iVar3;
  
  sVar2 = 4;
  iVar3 = 0xebc0;
  do {
    sVar1 = sVar2;
    if (((DAT_00fff0b0 != '\0') && (FUN_00017c3c(sVar2,iVar3), sVar2 != 0)) &&
       (-1 < (char)(&DAT_00fffbd8)[sVar1])) {
      FUN_00056506();
    }
    upload_hud_tile_block();
    iVar3 = iVar3 + -0xc;
    sVar2 = sVar1 + -1;
  } while (sVar2 != -1);
  return;
}


