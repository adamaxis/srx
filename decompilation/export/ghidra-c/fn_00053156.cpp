// Ghidra headless export
// Entry: 0x00053156
// Name: service_audio_tick_wrapper
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined8 service_audio_tick_wrapper(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  run_audio_transaction_wrapper(0);
  return CONCAT44(in_D0,in_D1);
}


