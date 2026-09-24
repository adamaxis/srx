// Ghidra headless export
// Entry: 0x00003D18
// Name: render_field_tile_rows
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003D4A render_field_tile_column
//   0x00004000 FUN_00004000
//   0x000041B6 fold_field_coordinate


void render_field_tile_rows(void)

{
  short sVar1;
  short sVar2;
  ushort unaff_D3w;
  short unaff_D4w;
  ushort uVar3;
  short unaff_D5w;
  
  fold_field_coordinate();
  sVar1 = FUN_00004000();
  sVar2 = 0x1e;
  uVar3 = unaff_D5w + unaff_D4w;
  do {
    render_field_tile_column(sVar1,sVar2,unaff_D3w);
    unaff_D3w = uVar3 & 0xf000;
    sVar1 = sVar1 + 8;
    sVar2 = sVar2 + -1;
    uVar3 = uVar3 & 0xf000;
  } while (sVar2 != -1);
  return;
}


