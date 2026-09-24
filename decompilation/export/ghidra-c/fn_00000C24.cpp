// Ghidra headless export
// Entry: 0x00000C24
// Name: FUN_00000c24
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank


void FUN_00000c24(void)

{
  short unaff_D5w;
  
  do {
    wait_for_vblank();
    unaff_D5w = unaff_D5w + -1;
  } while (unaff_D5w != -1);
  return;
}


