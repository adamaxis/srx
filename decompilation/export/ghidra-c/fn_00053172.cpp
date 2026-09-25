// Ghidra headless export
// Entry: 0x00053172
// Name: run_fixed_audio_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053094 begin_bus_then_flush_append_pair


undefined8 run_fixed_audio_pair(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  begin_bus_then_flush_append_pair(1,0);
  return CONCAT44(in_D0,in_D1);
}


