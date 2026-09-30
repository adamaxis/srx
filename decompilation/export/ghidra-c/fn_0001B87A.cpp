// Ghidra headless export
// Entry: 0x0001B87A
// Name: dispatch_title_world_phase_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records

// The indirect handler table at 0x1BAE4 has five entries (0x1B92C, 0x1B95A,
// 0x1B986, 0x1B9AE, and 0x1B9CA). After that handler runs, FFE050 selects one
// of four source sprite pointers at 0x1BAD4; FFE04A/FFE04C supply D1/D2 and
// D6 is fixed at 0x6000 for build_title_sprite_records. The native conversion
// names the handlers by behavior: gated +1 to 0xD0, gated -1 below 0xB0,
// +3 to 0x120 with audio 0x2A, -3 below 0xB0, and gated sprite-bank advance
// wrapping at four. It keeps both tables and all typed position fields explicit.


void dispatch_title_world_phase_service(void)

{
  (**(code **)((int)&PTR_DAT_0001bae4 + (int)(short)(DAT_00ffe04e << 2)))();
  build_title_sprite_records();
  return;
}


