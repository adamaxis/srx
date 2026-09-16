// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000AC84


void FUN_0000ac84(void)

{
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x10) & 8) == 0) {
    func_0x00013904();
  }
  else {
    func_0x000138b8();
  }
  if (DAT_00ffe18a != '\0') {
    func_0x0000ae5e();
  }
  return;
}


