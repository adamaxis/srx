// Ghidra headless export
// Entry: 0x00003D80
// Name: emit_field_tile_column_words
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void emit_field_tile_column_words(void)

{
  int iVar1;
  int extraout_D1;
  int iVar2;
  short unaff_D3w;
  ushort uVar3;
  short unaff_D7w;
  int extraout_A0;
  int extraout_A1;
  
  make_vdp_transfer_command();
  iVar1 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
  iVar2 = extraout_D1;
  do {
    uVar3 = *(ushort *)(extraout_A0 + iVar2 + iVar1);
    if ((short)_DAT_00ffdf58 < 0) {
      uVar3 = uVar3 ^ 0x8000;
    }
    else {
      uVar3 = _DAT_00ffdf58 | uVar3;
    }
    VDP_DATA = uVar3;
    uVar3 = (short)iVar2 + 2;
    if ((uVar3 & 7) == 0) {
      uVar3 = (short)iVar2 - 6;
      unaff_D3w = unaff_D3w + 2;
      iVar1 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
    }
    iVar2 = CONCAT22((short)((uint)iVar2 >> 0x10),uVar3);
    unaff_D7w = unaff_D7w + -1;
  } while (unaff_D7w != -1);
  return;
}


