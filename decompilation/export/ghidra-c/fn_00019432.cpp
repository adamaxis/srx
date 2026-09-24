// Ghidra headless export
// Entry: 0x00019432
// Name: FUN_00019432
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017BC0 FUN_00017bc0
//   0x00017C84 set_hud_selection_value
//   0x000194AC FUN_000194ac


void FUN_00019432(void)

{
  ushort uVar1;
  
  if (((DAT_00ffdd22._0_1_ & 4) != 0) || ((DAT_00ffdd22._0_1_ & 8) != 0)) {
    uVar1 = (ushort)DAT_00fff0d4;
    if ((((&DAT_00fffbd8)[(short)uVar1] != '\x03') &&
        ((((&DAT_00fffbd8)[(short)uVar1] != '\x05' && ((&DAT_00fffbd8)[(short)uVar1] != '\x06')) &&
         ((&DAT_00fffbd8)[(short)uVar1] != '\a')))) &&
       (-1 < (char)((&DAT_00fff0e8)[(short)uVar1] + -6))) {
      set_hud_selection_value();
    }
    FUN_000194ac();
    FUN_00017bc0();
  }
  return;
}


