// Ghidra headless export
// Entry: 0x0000022E
// Name: VBLANK
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined8 VBLANK(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  service_display_frame();
  return CONCAT44(in_D0,in_D1);
}


