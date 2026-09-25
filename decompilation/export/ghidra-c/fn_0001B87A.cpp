// Ghidra headless export
// Entry: 0x0001B87A
// Name: dispatch_title_phase_handler
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void dispatch_title_phase_handler(void)

{
  (**(code **)((int)&PTR_DAT_0001bae4 + (int)(short)(DAT_00ffe04e << 2)))();
  build_title_sprite_records();
  return;
}


