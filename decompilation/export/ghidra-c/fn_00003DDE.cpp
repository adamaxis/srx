// Ghidra headless export
// Entry: 0x00003DDE
// Name: emit_field_tile_row_words
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003E14 upload_field_tile_row
//   0x00003FD8 calculate_field_tile_source_layout


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void emit_field_tile_row_words(void)

{
  short sVar1;
  int iVar2;
  int extraout_D1;
  int iVar3;
  short unaff_D3w;
  ushort unaff_D4w;
  ushort uVar4;
  short sVar5;
  int extraout_A0;
  int extraout_A1;
  
  calculate_field_tile_source_layout();
  uVar4 = (ushort)-((unaff_D4w & 0xf80) - 0x1000) >> 7;
  if ((short)(uVar4 - 1) < 0x1f) {
    upload_field_tile_row();
    sVar5 = -(uVar4 - 0x1f);
    VDP_CTRL = 0x8f80;
    make_vdp_transfer_command();
    iVar2 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
    iVar3 = extraout_D1;
    do {
      uVar4 = *(ushort *)(extraout_A0 + iVar3 + iVar2);
      if ((short)_DAT_00ffdf58 < 0) {
        uVar4 = uVar4 ^ 0x8000;
      }
      else {
        uVar4 = _DAT_00ffdf58 | uVar4;
      }
      VDP_DATA = uVar4;
      sVar1 = (short)iVar3 + 8;
      if (0x1e < sVar1) {
        sVar1 = (short)iVar3 + -0x18;
        unaff_D3w = DAT_00ffddea + unaff_D3w;
        iVar2 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
      }
      iVar3 = CONCAT22((short)((uint)iVar3 >> 0x10),sVar1);
      sVar5 = sVar5 + -1;
    } while (sVar5 != -1);
    VDP_CTRL = 0x8f02;
    return;
  }
  upload_field_tile_row();
  return;
}

