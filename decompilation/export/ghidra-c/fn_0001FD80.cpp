// Ghidra headless export
// Entry: 0x0001FD80
// Name: FUN_0001fd80
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x0001E1FC FUN_0001e1fc
//   0x0001F246 run_mode_exit_services
//   0x00059BCC reset_runner_status_triplet


void FUN_0001fd80(void)

{
  short sVar1;
  
  if ((DAT_00fffbf8 == '\x06') && (DAT_00fffbf9 == '\0')) {
    DAT_00fffc30 = DAT_00fffc30 & 0xfffffdff;
    run_mode_exit_services();
    return;
  }
  if ((DAT_00fffbf8 == '\a') && (DAT_00fffbf9 == '\0')) {
    if ((DAT_00fffc1e & 0x1000000) == 0) {
      DAT_00fffc1e = DAT_00fffc1e | 0x1000000;
    }
    else if ((DAT_00fffc1e & 0x2000000) != 0) {
      reset_runner_status_triplet();
    }
LAB_0001fe7a:
    run_mode_exit_services();
    return;
  }
  if ((DAT_00fffbf8 == '\t') && (DAT_00fffbf9 == '\0')) goto LAB_0001fe7a;
  DAT_00fff0d6 = 0;
  if (DAT_00fffbf8 == '\0') {
    if ((((DAT_00fffbf9 == '\x01') || (DAT_00fffbf9 == '\x02')) || (DAT_00fffbf9 == '\x04')) ||
       (DAT_00fffbf9 == '\x05')) goto LAB_0001fed2;
    if (DAT_00fffbf9 == '\x06') {
      if (DAT_00fff0d5 == '\0') {
        DAT_00fff0d6 = 0xff;
      }
      goto LAB_0001fed2;
    }
    if (DAT_00fffbf9 == '\a') goto LAB_0001fed2;
    if ((DAT_00fffbf9 != '\b') && (DAT_00fffbf9 != '\t')) goto LAB_00020054;
LAB_0002003c:
    DAT_00fff0d6 = 0xff;
  }
  else {
LAB_00020054:
    if (DAT_00fffbf8 == '\x04') {
      if (DAT_00fffbf9 == '\x01') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if ((DAT_00fffbf9 == '\x02') || (DAT_00fffbf9 == '\x05')) goto LAB_0001fed2;
    }
    if (DAT_00fffbf8 == '\x03') {
      if ((((DAT_00fffbf9 == '\0') || (DAT_00fffbf9 == '\x02')) || (DAT_00fffbf9 == '\x03')) ||
         (DAT_00fffbf9 == '\x04')) goto LAB_0001fed2;
      if (DAT_00fffbf9 == '\x05') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if (DAT_00fffbf9 == '\x06') goto LAB_0001fed2;
      if (((DAT_00fffbf9 == '\a') || (DAT_00fffbf9 == '\b')) || (DAT_00fffbf9 == '\t'))
      goto LAB_0002003c;
    }
    if (DAT_00fffbf8 == '\x01') {
      if (DAT_00fffbf9 == '\x02') goto LAB_0001fed2;
      if (DAT_00fffbf9 == '\0') {
        if (DAT_00fff0d5 == '\0') {
          sVar1 = sample_source_random_bound();
          if ((short)(sVar1 + -0x4b) < 0) {
            DAT_00fff0d6 = 0xff;
          }
          else {
            FUN_0001e1fc();
          }
        }
        goto LAB_0001fed2;
      }
      if ((DAT_00fffbf9 == '\x04') || (DAT_00fffbf9 == '\t')) goto LAB_0001fed2;
      if ((DAT_00fffbf9 == '\b') || (DAT_00fffbf9 == '\n')) goto LAB_0002003c;
    }
    if (DAT_00fffbf8 == '\x02') {
      if ((DAT_00fffbf9 == '\0') || (DAT_00fffbf9 == '\x03')) goto LAB_0001fed2;
      if (DAT_00fffbf9 == '\x04') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if (DAT_00fffbf9 == '\x05') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if ((DAT_00fffbf9 == '\a') || (DAT_00fffbf9 == '\t')) goto LAB_0001fed2;
      if ((DAT_00fffbf9 == '\b') || (DAT_00fffbf9 == '\n')) goto LAB_0002003c;
    }
    if ((DAT_00fffbf8 != '\x05') || ((DAT_00fffbf9 != '\x01' && (DAT_00fffbf9 != '\x06')))) {
      return;
    }
  }
LAB_0001fed2:
  run_mode_exit_services();
  DAT_00fff0d5 = 0;
  return;
}


