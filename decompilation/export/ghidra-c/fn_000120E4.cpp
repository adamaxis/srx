// Ghidra headless export
// Entry: 0x000120E4
// Name: FUN_000120e4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command
//   0x0000C216 build_grouped_decimal_glyphs


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000120e4(void)

{
  short sVar1;
  short sVar2;
  short unaff_D2w;
  
  make_vdp_transfer_command();
  VDP_DATA = DAT_000c1320 | 0x6000;
  sVar2 = (short)_DAT_00ffdd2a;
  sVar1 = unaff_D2w + sVar2;
  _DAT_00ffdd2a = CONCAT22((short)((uint)_DAT_00ffdd2a >> 0x10),sVar1);
  if (sVar1 == 0 || SCARRY2(unaff_D2w,sVar2) != sVar1 < 0) {
    _DAT_00ffdd2a = CONCAT31((int3)((uint)_DAT_00ffdd2a >> 8),*(undefined1 *)(DAT_00fffc04 + 0x4f));
  }
  if (*(char *)(DAT_00fffc04 + 0x4f) < (char)_DAT_00ffdd2a) {
    _DAT_00ffdd2a = 1;
  }
  build_grouped_decimal_glyphs();
  return;
}


