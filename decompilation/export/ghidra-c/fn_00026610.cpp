// Ghidra headless export
// Entry: 0x00026610
// Name: write_sequential_asset_grid
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void write_sequential_asset_grid(void)

{
  short extraout_D1w;
  word wVar1;
  short sVar2;
  
  VDP_CTRL = 0x8f80;
  if (*(char *)((int)&DAT_00ff1100 + (int)(short)(DAT_00fff118 * 0x14)) == '\0') {
    sVar2 = 7;
  }
  else {
    sVar2 = 9;
  }
  wVar1 = 0x393;
  do {
    make_vdp_transfer_command();
    do {
      VDP_DATA = wVar1;
      wVar1 = wVar1 + 1;
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    if (*(char *)((int)&DAT_00ff1100 + (int)(short)(DAT_00fff118 * 0x14)) == '\0') {
      sVar2 = 7;
    }
    else {
      sVar2 = 9;
    }
  } while (extraout_D1w != 0);
  VDP_CTRL = 0x8f02;
  return;
}


