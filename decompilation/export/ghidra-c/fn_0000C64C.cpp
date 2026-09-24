// Ghidra headless export
// Entry: 0x0000C64C
// Name: FUN_0000c64c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00002A32 initialize_field_mode_profile_a
//   0x00002A44 initialize_field_mode_profile_b
//   0x000038FA FUN_000038fa
//   0x0000391A FUN_0000391a
//   0x0000C802 FUN_0000c802
//   0x0001DED8 FUN_0001ded8
//   0x0001E68C FUN_0001e68c
//   0x0001F246 FUN_0001f246


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000c64c(void)

{
  uint uVar1;
  undefined *puVar2;
  
  DAT_00ffe186 = 0;
  DAT_00ffe182 = 0;
  DAT_00fff10c = 0;
  if ((char)DAT_00fffbec == '\x05') {
    if (((DAT_00fffc1c & 1) == 0) || ((DAT_00fffc1c & 0x80) != 0)) goto LAB_0000c716;
    DAT_00ffe183 = 0;
    puVar2 = &DAT_000f3d5c;
LAB_0000c690:
    FUN_0001ded8(puVar2);
  }
  else {
    if ((char)DAT_00fffbec == '\x01') {
      if ((DAT_00fffc1c & 1) == 0) goto LAB_0000c716;
      puVar2 = &DAT_000f359e;
      goto LAB_0000c690;
    }
    if ((((char)DAT_00fffbec != '\x02') && ((char)DAT_00fffbec != '\x04')) ||
       ((DAT_00fffc1c & 1) == 0)) goto LAB_0000c716;
    DAT_00fffc22 = DAT_00fffc22 & 0xfffffdff;
  }
  FUN_0001e68c();
  FUN_0001f246();
LAB_0000c716:
  DAT_00ff024c = 0;
  DAT_00ff034c = 0;
  FUN_0000c802();
  DAT_00ff0110 = DAT_00ff0110 & 0xaf;
  DAT_00ff0110 = DAT_00ff0110 | 1;
  DAT_00ff0111 = DAT_00ff0111 & 0xbf;
  _DAT_00ff013a = 0xb45c;
  if ((char)(DAT_00ff014c + -6) < '\0') {
    DAT_00ff014c = '\x06';
  }
  if ((char)(DAT_00ff019c + -8) < '\0') {
    DAT_00ff019c = '\b';
  }
  uVar1 = DAT_00fffbfe >> 3;
  if (-1 < (int)(uVar1 - 5000)) {
    uVar1 = 5000;
  }
  DAT_00fffbfe = DAT_00fffbfe - uVar1;
  FUN_0001f246();
  FUN_000038fa();
  FUN_0000391a();
  if ((DAT_00fffc1e & 1) == 0) {
    initialize_field_mode_profile_b();
    return;
  }
  initialize_field_mode_profile_a();
  return;
}


