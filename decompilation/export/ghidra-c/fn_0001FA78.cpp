// Ghidra headless export
// Entry: 0x0001FA78
// Name: convert_world_routine_0x0001fa78
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001F246 run_mode_exit_services
//   0x00056AC0 convert_intro_routine_0x00056ac0


/* WARNING: Type propagation algorithm not settling */

void convert_world_routine_0x0001fa78(void)

{
  if ((DAT_00fffbf8 == '\x02') && (DAT_00fffbf9 == '\v')) {
    DAT_00fff0b6 = 1;
    DAT_00fff0b7 = 0xff;
  }
  else {
    if (DAT_00fffbf8 == '\x01') {
      if (DAT_00fffbf9 == '\x05') {
        DAT_00fff0b6 = 2;
        if (DAT_00fff0d5 != '\0') goto LAB_0001fd7a;
        DAT_00fff112 = '\0';
        run_mode_exit_services();
        if (DAT_00fff112 == '\0') {
          return;
        }
        if (DAT_00fff112 < '\0') {
          DAT_00fff0b7 = 0xff;
        }
        else {
          DAT_00fff0b7 = 2;
        }
        goto LAB_0001fd6e;
      }
      if (DAT_00fffbf9 == '\x03') {
        DAT_00fff0b6 = 4;
        DAT_00fff0b7 = 0;
        goto joined_r0x0001fb7c;
      }
    }
    if (DAT_00fffbf8 != '\x05') {
LAB_0001fce6:
      if (DAT_00fffbf8 != '\x04') {
        return;
      }
      if (DAT_00fffbf9 != '\x03') {
        return;
      }
      if (DAT_00fff0d5 != '\0') goto LAB_0001fd7a;
      DAT_00fff112 = '\0';
      run_mode_exit_services();
      if (DAT_00fff112 == '\0') {
        return;
      }
      DAT_00fff112 = '\0';
      DAT_00fff0b6 = 3;
      DAT_00fff0b7 = 0;
      goto LAB_0001fd6e;
    }
    if (DAT_00fffbf9 == '\x02') {
      DAT_00fff0b6 = 7;
      DAT_00fff0b7 = 1;
    }
    else if (DAT_00fffbf9 == '\x03') {
      DAT_00fff0b6 = 5;
      DAT_00fff0b7 = 0;
    }
    else if (DAT_00fffbf9 == '\x04') {
      DAT_00fff0b6 = 6;
      DAT_00fff0b7 = 2;
    }
    else {
      if (DAT_00fffbf9 != '\x05') goto LAB_0001fce6;
      DAT_00fff0b6 = 0;
      DAT_00fff0b7 = 0xff;
    }
  }
joined_r0x0001fb7c:
  if (DAT_00fff0d5 != '\0') {
LAB_0001fd7a:
    run_mode_exit_services();
    return;
  }
  run_mode_exit_services();
LAB_0001fd6e:
  convert_intro_routine_0x00056ac0();
  return;
}


