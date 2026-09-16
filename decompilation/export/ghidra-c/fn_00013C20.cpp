// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013C20


void FUN_00013c20(void)

{
  undefined1 uVar1;
  
  uVar1 = 0;
  if (((3 < DAT_00ff014c) && ((uVar1 = 1, DAT_00fffc0c < '\0' || (3 < DAT_00ff024c)))) &&
     ((DAT_00fffc0d < '\0' || (uVar1 = 2, 3 < DAT_00ff034c)))) {
    return;
  }
  DAT_00ffe190 = uVar1;
  return;
}


