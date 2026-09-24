// Ghidra headless export
// Entry: 0x0005D8BA
// Name: step_pre_title_logo_strobe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005B2E2 step_title_intro_entity_color_words


void step_pre_title_logo_strobe(void)

{
  short sVar1;
  
  DAT_00ffdf90 = DAT_00ffdf90 + -1;
  if (DAT_00ffdf90 < 0) {
    DAT_00ffdf90 = 4;
    DAT_00ffdf92 = DAT_00ffdf92 + 1;
    if (7 < DAT_00ffdf92) {
      DAT_00ffdf92 = 0;
    }
    sVar1 = DAT_00ffdf92 << 1;
    DAT_00ffd882._2_2_ = *(undefined2 *)((int)&DAT_0005dce6 + (int)sVar1);
    DAT_00ffd886 = *(undefined2 *)((int)&DAT_0005dce8 + (int)sVar1);
    DAT_00ffd888 = *(undefined2 *)((int)&DAT_0005dcea + (int)sVar1);
    DAT_00ffd88a = *(undefined2 *)((int)&DAT_0005dcec + (int)sVar1);
    DAT_00ffd88c = *(undefined2 *)((int)&DAT_0005dcee + (int)sVar1);
    DAT_00ffd88e = *(undefined2 *)((int)&DAT_0005dcf0 + (int)sVar1);
    DAT_00ffd890 = *(undefined2 *)((int)&DAT_0005dcf2 + (int)sVar1);
    DAT_00ffd892 = *(undefined2 *)((int)&DAT_0005dcf4 + (int)sVar1);
    DAT_00ffef14 = DAT_00ffef14 | 1;
  }
  else if (DAT_00ffdf90 == 2) {
    step_title_intro_entity_color_words();
    return;
  }
  return;
}


