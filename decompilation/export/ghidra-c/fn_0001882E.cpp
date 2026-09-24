// Ghidra headless export
// Entry: 0x0001882E
// Name: FUN_0001882e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA FUN_0000c0da
//   0x0000C134 FUN_0000c134
//   0x0000C160 FUN_0000c160
//   0x0000C216 FUN_0000c216
//   0x0001897C FUN_0001897c
//   0x00018A92 FUN_00018a92
//   0x0001952C FUN_0001952c
//   0x0001957E FUN_0001957e
//   0x000195A6 FUN_000195a6
//   0x0001A16A FUN_0001a16a


void FUN_0001882e(void)

{
  short unaff_D7w;
  
  FUN_0001897c();
  FUN_00018a92();
  if ((unaff_D7w < 0xc) && ((char)(&DAT_00fffbcc)[unaff_D7w] < '\x01')) {
    FUN_0000c0da();
    return;
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
    FUN_0000c160();
    FUN_0000c134();
  }
  DAT_00ffef96 = 0xff;
  FUN_0000c160();
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


