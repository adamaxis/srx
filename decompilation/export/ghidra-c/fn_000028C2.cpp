// Ghidra headless export
// Entry: 0x000028C2
// Name: FUN_000028c2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000286E FUN_0000286e
//   0x000028AA FUN_000028aa
//   0x00002900 FUN_00002900
//   0x00002C6C FUN_00002c6c
//   0x0000301E upload_scene_profile_block


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000028c2(void)

{
  _DAT_00ffdf8a = 0xffff;
  FUN_00002900();
  FUN_0000286e();
  FUN_000028aa();
  DAT_00ffddc6 = DAT_00ffddc6 & 0xff80;
  DAT_00ffddc8 = DAT_00ffddc8 & 0xff80;
  DAT_00ffdf36 = 0xffff;
  DAT_00ffdf38 = 0xffff;
  FUN_00002c6c();
  upload_scene_profile_block();
  return;
}


