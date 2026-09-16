// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000033BE


undefined8 FUN_000033be(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short sVar1;
  short sVar2;
  
  sVar2 = 3;
  do {
    sVar1 = 3;
    do {
      FUN_00000f5c();
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return CONCAT44(in_D0,in_D1);
}


