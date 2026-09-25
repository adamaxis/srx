// Ghidra headless export
// Entry: 0x0001CD5A
// Name: convert_world_routine_0x0001cd5a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019F18 run_menu_progression_variant
//   0x0001CEFA advance_title_effect_position
//   0x0001CF4A advance_title_effect_sprites


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_world_routine_0x0001cd5a(void)

{
  undefined2 *extraout_A0;
  undefined **ppuVar1;
  
  if (DAT_00ff15cc == '\0') {
    return;
  }
  DAT_00ff15d1 = DAT_00ff15d1 + -1;
  if ((DAT_00ff15d1 < '\0') && (DAT_00ff15cc = '\0', _DAT_00ffe0a8 != 0)) {
    DAT_00ffe0aa = 10;
  }
  DAT_00ff15ca = DAT_00ff15ca + -1;
  if (DAT_00ff15ca < '\0') {
    DAT_00ff15ca = '\x14';
    if ((char)(DAT_00ff15cb + '\x01') < '\x03') {
      DAT_00ff15cb = DAT_00ff15cb + '\x01';
    }
  }
  DAT_00ff15d0 = DAT_00ff15d0 + -1;
  if (DAT_00ff15d0 < '\0') {
    DAT_00ff15d0 = '\x02';
    DAT_00ff15cf = DAT_00ff15cf + '\x01';
    if ('\x02' < DAT_00ff15cf) {
      DAT_00ff15cf = '\0';
    }
  }
  ppuVar1 = &PTR_DAT_0001d158;
  if ((DAT_00ff15cb != '\0') && (ppuVar1 = &PTR_DAT_0001d140, DAT_00ff15cb != '\x01')) {
    ppuVar1 = &PTR_DAT_0001d14c;
  }
  DAT_00ff15c6 = DAT_00ff15c6 + -8;
  DAT_00ff15c8 = DAT_00ff15c8 + -0xe;
  build_title_sprite_records(*(undefined4 *)((int)ppuVar1 + (int)(short)((short)DAT_00ff15cf << 2)))
  ;
  if (*(char *)(extraout_A0 + 3) == '\0') {
    DAT_00ff1592 = DAT_00ff1592 & 0xbf;
    _DAT_00ffe0d6 = 0;
    run_menu_progression_variant();
    return;
  }
  if (*(char *)(extraout_A0 + 2) == '\0') {
    if (*(char *)((int)extraout_A0 + 5) == '\0') {
      DAT_00ffe088 = *extraout_A0;
      DAT_00ffe08a = *extraout_A0;
      DAT_00ffe08c = extraout_A0[1];
      DAT_00ffe08e = extraout_A0[1];
    }
    else if (*(char *)((int)extraout_A0 + 5) == '\x01') {
      DAT_00ffe090 = *extraout_A0;
      DAT_00ffe092 = extraout_A0[1];
      DAT_00ffe094 = *extraout_A0;
      DAT_00ffe096 = extraout_A0[1];
      DAT_00ffe098 = DAT_00ffe088;
      DAT_00ffe09a = DAT_00ffe08c;
      DAT_00ffe09c = DAT_00ffe088;
      DAT_00ffe09e = DAT_00ffe08c;
      DAT_00ffe0a0 = DAT_00ffe08a;
      DAT_00ffe0a2 = DAT_00ffe08e;
      DAT_00ffe0a4 = DAT_00ffe08a;
      DAT_00ffe0a6 = DAT_00ffe08e;
    }
  }
  if (*(char *)((int)extraout_A0 + 5) != '\0') {
    if (*(char *)((int)extraout_A0 + 5) != '\x01') {
      advance_title_effect_sprites();
    }
    advance_title_effect_position();
  }
  return;
}

