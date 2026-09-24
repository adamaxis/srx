// Ghidra headless export
// Entry: 0x0000FDFE
// Name: FUN_0000fdfe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0000FDAE write_menu_grid_quadruple


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000fdfe(void)

{
  char cVar1;
  ushort uVar2;
  undefined2 *puVar3;
  short *psVar4;
  
  DAT_00fff0da = DAT_00fff0da + -1;
  if (DAT_00fff0da < '\0') {
    DAT_00fff0da = '\x06';
    cVar1 = DAT_00fff0db + -3;
    DAT_00fff0db = DAT_00fff0db + '\x01';
    if (-1 < cVar1) {
      DAT_00fff0db = '\0';
    }
    DAT_00ffdd02 = 1;
    build_title_sprite_records();
    DAT_00ffd989 = 0;
    uVar2 = (ushort)DAT_00fff0d0;
    for (puVar3 = &DAT_00ffd986; puVar3[3] = uVar2 * 0x20 + puVar3[3],
        *(char *)((int)puVar3 + 3) != '\0'; puVar3 = puVar3 + 4) {
    }
    uVar2 = (ushort)DAT_00fff0d1;
    for (psVar4 = &DAT_00ffd986; *psVar4 = uVar2 * 0x18 + *psVar4,
        *(char *)((int)psVar4 + 3) != '\0'; psVar4 = psVar4 + 4) {
    }
    _DAT_00ffd980 = 1;
    DAT_00ffef14 = DAT_00ffef14 | 2;
  }
  write_menu_grid_quadruple();
  return;
}


