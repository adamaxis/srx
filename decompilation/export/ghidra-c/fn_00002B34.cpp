// Ghidra headless export
// Entry: 0x00002B34
// Name: apply_scene_profile_selection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void apply_scene_profile_selection(void)

{
  ushort *puVar1;
  
  puVar1 = DAT_00ffe114;
  if (DAT_00ffe114 == (ushort *)0x0) {
    DAT_00ff0100 = DAT_00ffde08;
    DAT_00ff0104 = DAT_00ffde0a;
    DAT_00ff0200 = DAT_00ffde0c;
    DAT_00ff0204 = DAT_00ffde0e;
    DAT_00ff0300 = DAT_00ffde10;
    DAT_00ff0304 = DAT_00ffde12;
    return;
  }
  DAT_00ffe114 = (ushort *)0x0;
  DAT_00ffddc6 = *puVar1 & 0xff80;
  DAT_00ffddc8 = puVar1[1] & 0xff80;
  DAT_00ffdf36 = 0xffff;
  DAT_00ffdf38 = 0xffff;
  DAT_00ff0100 = puVar1[2];
  DAT_00ff0104 = puVar1[3];
  DAT_00ff0200 = puVar1[4];
  DAT_00ff0204 = puVar1[5];
  DAT_00ff0300 = puVar1[6];
  DAT_00ff0304 = puVar1[7];
  return;
}


