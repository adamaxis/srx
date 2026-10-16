// Ghidra headless export
// Entry: 0x0001BF7A
// Name: advance_profile_copy_timer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C3C read_menu_entry_profile_eligibility
//   0x0001C012 copy_seven_words
// Reload table at ROM 0x001BFFC for timing indexes 0..10:
//   [0x00F0, 0x00D2, 0x00B4, 0x0078, 0x005A, 0x002D,
//    0x001E, 0x0014, 0x000A, 0x0008, 0x0005]
// The eligibility result is tested with BLE; zero and negative results skip
// the copy. Positive results use (countdown + 8) * 2 as the source byte
// offset into the profile source records.


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

