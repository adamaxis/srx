// Ghidra headless export
// Entry: 0x0005D746
// Name: run_pre_title_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x000530D2 FUN_000530d2
//   0x000530DE FUN_000530de
//   0x0005D886 FUN_0005d886
//   0x0005D8BA step_pre_title_logo_strobe
//   0x0005D938 advance_pre_title_animation
//   0x0005DA50 setup_pre_title_resources
//   0x0005DAC4 fill_boot_horizontal_scroll


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_pre_title_sequence(void)

{
  short sVar1;
  
  _DAT_00fffc0a = 1;
  setup_pre_title_resources();
  DAT_00ffe18a = 1;
  DAT_00ffdd2a = 0x41;
  _DAT_00ffdd2c = 0x119;
  DAT_00ffdd2e = 0x1a8;
  DAT_00ffdd3c = 0;
  sVar1 = 0x150;
  DAT_00ffdf92 = 9;
  do {
    wait_for_vblank();
    if (DAT_00ffdd3c != 0) {
      step_pre_title_logo_strobe();
    }
    fill_boot_horizontal_scroll();
    advance_pre_title_animation();
    if (DAT_00ffdd2e == 0x160) {
LAB_0005d7c8:
      DAT_00ffdd30 = DAT_00ffdd30 + -1;
      if ((DAT_00ffdd30 < 0) && (DAT_00ffdd28 = DAT_00ffdd28 + 1, (DAT_00ffdd28 & 1) == 0)) {
        if (DAT_00ffdd2a < 6) {
          FUN_000530d2();
        }
        DAT_00ffdd2a = DAT_00ffdd2a + -1;
        if (-1 < DAT_00ffdd2a) {
          if (DAT_00ffdd2a == 0) {
            FUN_000530d2();
            DAT_00ffdd3c = 1;
          }
          DAT_00ffdd0a = DAT_00ffdd0a + -1;
          if (4 < DAT_00ffdd2a) {
            FUN_000530de();
          }
        }
      }
    }
    else {
      DAT_00ffdd2e = DAT_00ffdd2e + -1;
      if (DAT_00ffdd2e == 0x160) {
        DAT_00ffdd30 = 0x14;
        goto LAB_0005d7c8;
      }
    }
    DAT_00ffef14 = DAT_00ffef14 | 0xe;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      sample_controller_input();
      step_palette_transition();
      FUN_0005d886();
      sVar1 = 0x78;
      do {
        wait_for_vblank();
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
      sample_controller_input();
      step_palette_transition();
      return;
    }
  } while( true );
}


