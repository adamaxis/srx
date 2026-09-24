// Ghidra headless export
// Entry: 0x0000C7F8
// Name: FUN_0000c7f8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000c7f8(void)

{
  uint uVar1;
  undefined *puVar2;
  int extraout_A0;
  int extraout_A0_00;
  
  if (DAT_00ff014c != '\0') {
    FUN_00059798();
    if (((DAT_00ff0210 & 1) != 0) && ((DAT_00ff024c == '\0' || (DAT_00ff029c == '\0')))) {
      DAT_00fffc0c = 0xff;
      FUN_0000c8b0();
      FUN_0001e514();
      if (extraout_A0 == DAT_00fffc04) {
        FUN_0000391a();
      }
    }
    if (((DAT_00ff0310 & 1) != 0) && ((DAT_00ff034c == '\0' || (DAT_00ff039c == '\0')))) {
      DAT_00fffc0d = 0xff;
      FUN_0000c8b0();
      FUN_0001e514();
      if (extraout_A0_00 == DAT_00fffc04) {
        FUN_0000391a();
      }
    }
    FUN_00003404();
    FUN_000038fa();
    return;
  }
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
    FUN_00002a44();
    return;
  }
  FUN_00002a32();
  return;
}


