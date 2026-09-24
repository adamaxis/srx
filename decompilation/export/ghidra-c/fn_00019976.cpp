// Ghidra headless export
// Entry: 0x00019976
// Name: FUN_00019976
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001792E render_menu_hud_rows
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x00017C8E hud_selection_value_matches
//   0x00017EE4 reset_menu_overlay_and_upload_two
//   0x0001989A FUN_0001989a
//   0x0001B1C4 FUN_0001b1c4


void FUN_00019976(void)

{
  undefined4 uVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  bool bVar4;
  
  if (((&DAT_00fffbd8)[(short)(ushort)DAT_00fff0d4] != -1) &&
     (-1 < (char)((&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] + -6))) {
    bVar4 = false;
    hud_selection_value_matches();
    if (!bVar4) {
      bVar4 = false;
      hud_selection_value_matches();
      if (!bVar4) {
        bVar4 = false;
        hud_selection_value_matches();
        if (!bVar4) {
          bVar4 = false;
          uVar1 = hud_selection_value_matches();
          if ((!bVar4) && (DAT_00fff0e4 == '\f')) {
            if ((DAT_00ffdd22._0_1_ & 0x40) == 0) {
              if ((DAT_00ffdd22._0_1_ & 0x10) == 0) {
                DAT_00fff0b7 = 1;
              }
              else {
                DAT_00fff0b7 = 0;
              }
            }
            else {
              DAT_00fff0b7 = 0xff;
            }
            (**(code **)(&DAT_000e2e0a + (short)((short)uVar1 << 2)))();
            reset_menu_overlay_and_upload_two();
            if (DAT_00fff0b6 == '\0') {
              bVar4 = false;
              uVar1 = hud_selection_value_matches();
              if (!bVar4) {
                bVar4 = false;
                hud_selection_value_matches();
                if (!bVar4) {
                  bVar4 = false;
                  hud_selection_value_matches();
                  if (!bVar4) {
                    set_hud_selection_value();
                    render_menu_text_followup();
                  }
                }
                goto LAB_00019a8a;
              }
            }
            else {
              DAT_00fff0b6 = '\0';
            }
            if (-1 < (char)((&DAT_00ff1113)[(short)((uint)DAT_00fff118 * 0x14)] - 0x11)) {
              uVar2 = (undefined2)((uint)DAT_00fff118 * 0x14 >> 0x10);
              if (((&DAT_00ff1113)[(short)((uint)DAT_00fff118 * 0x14)] & 1) == 0) {
                uVar3 = CONCAT22(uVar2,5);
                (&DAT_00fffbcc)[(short)uVar1] = 0;
              }
              else {
                uVar3 = CONCAT22(uVar2,4);
              }
              DAT_00fff0a0 = 1;
              FUN_0001989a(uVar1,uVar3);
              FUN_0001b1c4();
              return;
            }
          }
        }
      }
    }
  }
LAB_00019a8a:
  render_menu_hud_rows();
  return;
}


