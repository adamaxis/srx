// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001882E


void FUN_0001882e(void)

{
  short unaff_D7w;
  
  FUN_0001897c();
  FUN_00018a92();
  if ((unaff_D7w < 0xc) && ((char)(&DAT_00fffbcc)[unaff_D7w] < '\x01')) {
                    /* WARNING: Subroutine does not return */
    FUN_0000c0da();
  }
  FUN_00001232();
  FUN_00001232();
  if ((short)(unaff_D7w + -0xc) < 0) {
    FUN_0001a16a();
    FUN_0000c216();
  }
  FUN_0000c134();
  DAT_00ffef96 = 0xff;
  if ((short)(unaff_D7w + -0xc) < 0) {
    func_0x0000c160();
    FUN_0000c134();
  }
  DAT_00ffef96 = 0xff;
  func_0x0000c160();
  if ((short)(unaff_D7w + -0xc) < 0) {
    FUN_000195a6();
    FUN_0000c216();
    FUN_0001952c();
    FUN_0000c216();
    FUN_0001957e();
    FUN_0000c216();
  }
  return;
}


