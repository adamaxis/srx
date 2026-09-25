// Ghidra headless export
// Entry: 0x0005B1B6
// Name: advance_title_intro_landing
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_title_intro_landing(void)

{
  short sVar1;
  
  if (0x225 < (ushort)(DAT_00ffdf88 - DAT_00fff11e)) {
    if ((DAT_00fff196 <= DAT_00ffdf88) && (DAT_00fff194 < 7)) {
      sVar1 = DAT_00fff194 * 6;
      DAT_00fff19c = *(short *)(&DAT_0005b270 + sVar1);
      DAT_00fff19e = *(short *)(&DAT_0005b272 + sVar1);
      DAT_00fff196 = DAT_00ffdf88 + *(short *)(&DAT_0005b274 + sVar1);
      DAT_00fff194 = DAT_00fff194 + 1;
    }
    DAT_00fff198 = DAT_00fff19c + DAT_00fff198;
    DAT_00fff19a = DAT_00fff19e + DAT_00fff19a;
    build_title_sprite_records();
  }
  return;
}


