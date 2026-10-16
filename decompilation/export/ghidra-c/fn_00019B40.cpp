// Ghidra headless export
// Entry: 0x00019B40
// Name: select_run_hud_mode
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C3C read_menu_entry_profile_eligibility


uint select_run_hud_mode(void)

{
  uint uVar1;
  short sVar2;
  char cVar3;
  bool bVar4;
  char cVar5;
  
  if (('\0' < (char)(&DAT_00ff110f)[(short)(DAT_00fff118 * 0x14)]) &&
     (uVar1 = (uint)(byte)(&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)],
     -1 < (char)(&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)])) {
    cVar5 = '\0';
    cVar3 = DAT_00ffdf84 < '\0';
    bVar4 = DAT_00ffdf84 == '\0';
    if (!(bool)cVar3) {
      uVar1 = read_menu_entry_profile_eligibility();
      if (bVar4 || cVar5 != cVar3) {
        return 0xffffffff;
      }
    }
    if (((DAT_00fff0a0 < '\x01') || ((char)((char)uVar1 + -7) < '\0')) ||
       (-1 < (char)((char)uVar1 + -0x11))) {
      sVar2 = (short)uVar1 << 1;
    }
    else {
      sVar2 = (short)uVar1 * 2 + 1;
    }
    return (uint)*(byte *)((int)&PTR_DAT_000e31a7 + CONCAT22((short)(uVar1 >> 0x10),sVar2));
  }
  return 0x45;
}


