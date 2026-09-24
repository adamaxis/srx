// Ghidra headless export
// Entry: 0x0001F246
// Name: FUN_0001f246
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0C2 FUN_0000c0c2
//   0x00010952 FUN_00010952
//   0x00010984 FUN_00010984
//   0x0001F292 FUN_0001f292
//   0x0001F3C4 FUN_0001f3c4
//   0x0005670E FUN_0005670e


void FUN_0001f246(void)

{
  code *in_A1;
  
  VDP_CTRL = 0x9100;
  FUN_0000c0c2();
  FUN_00010984();
  FUN_0001f292();
  if (DAT_00fff10c != '\0') {
    (*in_A1)();
  }
  DAT_00fff10c = 0;
  FUN_0001f3c4();
  FUN_00010952();
  FUN_0005670e();
  VDP_CTRL = 0x9191;
  return;
}


