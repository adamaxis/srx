// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017DB8


void FUN_00017db8(void)

{
  char in_D1b;
  bool bVar1;
  
  bVar1 = SCARRY1(in_D1b,DAT_00fffbca);
  DAT_00fffbca = in_D1b + DAT_00fffbca;
  if (DAT_00fffbca == '\0' || bVar1 != DAT_00fffbca < '\0') {
    DAT_00fffbca = '\0';
  }
  else if ('\n' < DAT_00fffbca) {
    DAT_00fffbca = '\n';
  }
  FUN_00001232();
  FUN_0001792e();
  return;
}


