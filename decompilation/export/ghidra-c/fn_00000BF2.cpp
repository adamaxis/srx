// Ghidra headless export
// Entry: 0x00000BF2
// Name: latch_controller_events
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort latch_controller_events(void)

{
  ushort uVar1;
  
  uVar1 = (_DAT_00ffdd26 | DAT_00ffdd22) & 0xff00;
  if ((uVar1 != 0) && (_DAT_00ffddc0 = 0xffff, (DAT_00ffdd22 & 0x8000) != 0)) {
    _DAT_00ffddc2 = 0xffff;
  }
  return uVar1;
}


