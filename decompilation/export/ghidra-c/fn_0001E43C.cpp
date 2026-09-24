// Ghidra headless export
// Entry: 0x0001E43C
// Name: FUN_0001e43c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E2EC begin_world_activity


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e43c(void)

{
  short sVar1;
  undefined *puVar2;
  
  DAT_00ffe180 = 0;
  DAT_00ffe182 = 0;
  DAT_00ffe184 = 0;
  DAT_00ffe187 = 0;
  if (DAT_00ffe183 == '\0') {
    return;
  }
  DAT_00ffe183 = 0;
  puVar2 = &DAT_00ff0400;
  sVar1 = 0xc;
  while (((puVar2[0x10] & 1) == 0 || ((puVar2[0xb0] & 0x80) == 0))) {
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      DAT_00ffe181 = 0xff;
LAB_0001e49c:
      _DAT_00ffdd2c = &UNK_0001f16a;
      begin_world_activity();
      return;
    }
  }
  DAT_00ffe181 = 0;
  goto LAB_0001e49c;
}


