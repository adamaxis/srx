// Ghidra headless export
// Entry: 0x000587DA
// Name: convert_intro_routine_0x000587da
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00011190 transfer_menu_glyph_triple
//   0x0005763C convert_intro_routine_0x0005763c
//   0x00058A2E convert_intro_routine_0x00058a2e
//   0x00058AA4 convert_intro_routine_0x00058aa4
//   0x00058B1E convert_intro_routine_0x00058b1e


void convert_intro_routine_0x000587da(void)

{
  short sVar1;
  short sVar2;
  
  DAT_00fff0b8 = DAT_00fff0d0;
  if (DAT_00fff0b7 < '\0') {
    if (((DAT_00fff0d0 != '\0') && (-1 < (char)(DAT_00fff0d0 + -2))) && (DAT_00fff0d0 != '\x02')) {
      if (DAT_00fff0d0 == '\x03') {
        convert_intro_routine_0x00058b1e();
        return;
      }
LAB_0005884c:
      convert_intro_routine_0x00058aa4();
      return;
    }
  }
  else {
    if (DAT_00fff0b7 != '\0') {
      if (DAT_00fff0b7 == '\x01') {
        sVar1 = 7;
        do {
          write_vdp_grid();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        return;
      }
      if (-1 < (char)(DAT_00fff0d0 + -3)) {
        if (DAT_00fff0d0 == '\x03') {
          sVar1 = 7;
          do {
            write_vdp_grid();
            sVar1 = sVar1 + -1;
          } while (sVar1 != -1);
          return;
        }
        if (DAT_00fff0d0 == '\x05') {
          sVar1 = 4;
          sVar2 = 2;
        }
        else {
          sVar1 = 3;
          sVar2 = 3;
        }
        do {
          write_vdp_grid();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        do {
          write_vdp_grid();
          sVar2 = sVar2 + -1;
        } while (sVar2 != -1);
        return;
      }
      convert_intro_routine_0x0005763c();
      sVar1 = 7;
      do {
        write_vdp_grid();
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
      return;
    }
    if (DAT_00fff0d0 != '\x01') {
      if ((char)(DAT_00fff0d0 + -1) < '\0') {
        sVar1 = 7;
        do {
          transfer_menu_glyph_triple();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        return;
      }
      if (DAT_00fff0d0 == '\x03') goto LAB_0005884c;
    }
  }
  convert_intro_routine_0x00058a2e();
  return;
}


