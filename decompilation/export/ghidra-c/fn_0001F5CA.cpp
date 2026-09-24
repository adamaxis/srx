// Ghidra headless export
// Entry: 0x0001F5CA
// Name: FUN_0001f5ca
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C64C FUN_0000c64c
//   0x0001EEDC dispatch_world_effect
//   0x0001F65A FUN_0001f65a
//   0x0001F830 FUN_0001f830
//   0x0001FA78 FUN_0001fa78
//   0x0001FD80 FUN_0001fd80
//   0x00020634 FUN_00020634
//   0x00053A78 FUN_00053a78


void FUN_0001f5ca(void)

{
  if (((DAT_00ff0110 & 0x40) == 0) && ((DAT_00ff0110 & 0x10) == 0)) {
    FUN_00020634();
    DAT_00fff10c = 0;
    DAT_00fff0cc = '\0';
    dispatch_world_effect();
    if ((-1 < DAT_00fff0cc) && (FUN_00053a78(), -1 < DAT_00fff0cc)) {
      FUN_0001f65a();
      FUN_0001f830();
      FUN_0001fa78();
      FUN_0001fd80();
    }
    DAT_00fff0cc = 0;
    return;
  }
  FUN_0000c64c();
  return;
}


