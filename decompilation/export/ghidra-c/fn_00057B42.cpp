// Ghidra headless export
// Entry: 0x00057B42
// Name: convert_intro_routine_0x00057b42
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_intro_routine_0x00057b42(void)

{
  byte bVar1;
  uint uVar2;
  int in_D1;
  short sVar3;
  short sVar4;
  
  bVar1 = *(char *)(DAT_00fffc04 + 0x8c) - (&DAT_001aac6e)[(uint)DAT_00fff0b6 * 2];
  sVar3 = (short)CONCAT31((int3)((uint)DAT_00fff0b6 * 2 >> 8),bVar1);
  if (bVar1 != 0 &&
      SBORROW1(*(char *)(DAT_00fffc04 + 0x8c),(&DAT_001aac6e)[(uint)DAT_00fff0b6 * 2]) ==
      (int)((uint)bVar1 << 0x18) < 0) {
    if (-1 < (short)(sVar3 + -0xb)) {
      sVar3 = 10;
    }
    uVar2 = (uint)in_D1 >> 8;
    sVar4 = 9;
    do {
      in_D1 = in_D1 - (short)((short)uVar2 * sVar3);
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
  }
  _DAT_00fff0be = in_D1;
  return;
}


