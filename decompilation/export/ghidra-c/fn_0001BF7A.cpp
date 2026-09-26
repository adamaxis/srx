// Ghidra headless export
// Entry: 0x0001BF7A
// Name: advance_profile_copy_timer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C3C read_menu_entry_profile_eligibility
//   0x0001C012 copy_seven_words


void advance_profile_copy_timer(void)

{
  short sVar1;
  
  sVar1 = DAT_00fff11e + -1;
  if (sVar1 < 0) {
    if ((short)(DAT_00fff11e + 7) < 0) {
      sVar1 = *(short *)(&DAT_0001bffc +
                        (short)(*(char *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 2) * 2
                               ));
    }
    else {
      DAT_00fff11e = sVar1;
      read_menu_entry_profile_eligibility();
      copy_seven_words();
      DAT_00ffef14 = DAT_00ffef14 | 1;
      sVar1 = DAT_00fff11e;
    }
  }
  DAT_00fff11e = sVar1;
  return;
}

